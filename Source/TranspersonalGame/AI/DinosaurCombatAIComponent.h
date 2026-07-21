#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatAIComponent.generated.h"

// ============================================================
// Combat & Enemy AI Agent #12 — DinosaurCombatAIComponent
// Tactical combat AI for dinosaur enemies in MinPlayableMap
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoAggression : uint8
{
    Passive       UMETA(DisplayName = "Passive"),       // Ignores player unless attacked
    Territorial   UMETA(DisplayName = "Territorial"),   // Attacks if player enters zone
    Predatory     UMETA(DisplayName = "Predatory"),     // Actively hunts player
    Frenzied      UMETA(DisplayName = "Frenzied"),      // Wounded — attacks everything
    Fleeing       UMETA(DisplayName = "Fleeing")        // Overwhelmed — retreats
};

UENUM(BlueprintType)
enum class ECombat_DinoAttackType : uint8
{
    Bite          UMETA(DisplayName = "Bite"),
    Charge        UMETA(DisplayName = "Charge"),
    TailSwipe     UMETA(DisplayName = "TailSwipe"),
    Stomp         UMETA(DisplayName = "Stomp"),
    Pounce        UMETA(DisplayName = "Pounce")         // Raptor-specific
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TyrannosaurusRex  UMETA(DisplayName = "TyrannosaurusRex"),
    Velociraptor      UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus     UMETA(DisplayName = "Brachiosaurus"),
    Triceratops       UMETA(DisplayName = "Triceratops"),
    Pterodactyl       UMETA(DisplayName = "Pterodactyl")
};

USTRUCT(BlueprintType)
struct FCombat_AttackRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float Timestamp = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_DinoAttackType AttackType = ECombat_DinoAttackType::Bite;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float DamageDealt = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bHitPlayer = false;
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float BaseDamage = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggroRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed_Patrol = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed_Chase = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MoveSpeed_Flee = 600.0f;
};

/**
 * UCombat_DinosaurCombatAIComponent
 * Drives tactical combat behaviour for dinosaur enemies.
 * Attach to any APawn subclass representing a dinosaur.
 *
 * Features:
 * - Species-specific stat presets (TRex, Raptor, Brach, Trike, Ptero)
 * - Aggression state machine (Passive → Territorial → Predatory → Frenzied → Fleeing)
 * - Pack coordination: raptors share aggro target via nearest pack member
 * - Wound response: health < 25% triggers Frenzied or Fleeing based on species courage
 * - Attack cooldown with per-attack-type damage multipliers
 * - Player proximity damage (no animation required — pure proximity for prototype)
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent), DisplayName="Dinosaur Combat AI")
class TRANSPERSONALGAME_API UCombat_DinosaurCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatAIComponent();

    // ── Species & Stats ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Velociraptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    FCombat_DinoStats Stats;

    // ── Aggression State ─────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Combat")
    ECombat_DinoAggression AggressionState = ECombat_DinoAggression::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float CourageRating = 0.7f;  // 0=coward, 1=fearless. Affects flee threshold.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsPackAnimal = false;  // Raptors share aggro; TRex is solitary

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float PackCoordinationRadius = 1200.0f;  // Range to alert pack members

    // ── Runtime State ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Runtime")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Runtime")
    float LastAttackTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Runtime")
    TArray<FCombat_AttackRecord> AttackHistory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Runtime")
    bool bIsInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Runtime")
    float ThreatAccumulator = 0.0f;  // Builds up from player actions; drives aggro escalation

    // ── Public API ───────────────────────────────────────────────────────────

    /** Apply damage to this dinosaur. Returns true if dinosaur dies. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool ReceiveDamage(float DamageAmount, AActor* DamageSource);

    /** Force aggression state change (e.g., from quest trigger). */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void SetAggressionState(ECombat_DinoAggression NewState);

    /** Apply species-specific stat preset. Call after setting Species. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Dinosaur|Setup")
    void ApplySpeciesPreset();

    /** Returns true if player is within aggro radius. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    bool IsPlayerInAggroRange() const;

    /** Returns current health as 0-1 fraction. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthFraction() const;

    /** Alert nearby pack members of the current target. */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void AlertPackMembers();

    /** Called when this dinosaur kills the player. */
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Combat")
    void OnPlayerKilled();
    virtual void OnPlayerKilled_Implementation();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void TickCombatAI(float DeltaTime);
    void TickPassive(float DeltaTime);
    void TickTerritorial(float DeltaTime);
    void TickPredatory(float DeltaTime);
    void TickFrenzied(float DeltaTime);
    void TickFleeing(float DeltaTime);

    void TryAttackTarget(float CurrentTime);
    float CalculateAttackDamage(ECombat_DinoAttackType AttackType) const;
    ECombat_DinoAttackType SelectAttackType() const;
    void EvaluateWoundResponse();
    void ScanForPlayer();

    float TickInterval = 0.1f;
    float TimeSinceLastTick = 0.0f;
};

// DinosaurBase.h
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260620_003
// Base class for all dinosaur species in TranspersonalGame.
// Inherits ACharacter for movement/capsule/mesh; adds species stats,
// AI state machine, sensory system, and combat interface.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ── Enums (global scope — UHT requirement) ────────────────────

UENUM(BlueprintType)
enum class EDinoAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Wandering       UMETA(DisplayName = "Wandering"),
    Grazing         UMETA(DisplayName = "Grazing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Alerted         UMETA(DisplayName = "Alerted"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Chasing         UMETA(DisplayName = "Chasing"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Resting         UMETA(DisplayName = "Resting"),
    Dead            UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EDinoDiet : uint8
{
    Carnivore       UMETA(DisplayName = "Carnivore"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Omnivore        UMETA(DisplayName = "Omnivore")
};

UENUM(BlueprintType)
enum class EDinoSize : uint8
{
    Tiny            UMETA(DisplayName = "Tiny"),      // < 1m — Compsognathus
    Small           UMETA(DisplayName = "Small"),     // 1-3m — Velociraptor
    Medium          UMETA(DisplayName = "Medium"),    // 3-6m — Dilophosaurus
    Large           UMETA(DisplayName = "Large"),     // 6-10m — Allosaurus
    Huge            UMETA(DisplayName = "Huge"),      // 10-15m — T-Rex
    Colossal        UMETA(DisplayName = "Colossal")   // >15m — Brachiosaurus
};

// ── ADinosaurBase ─────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable, ClassGroup = (Dinosaurs),
    meta = (DisplayName = "Dinosaur Base"))
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Damage Override ───────────────────────────────────────
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

    // ── Species Identity ──────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoDiet DietType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSize SizeClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    bool bIsPackAnimal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity",
        meta = (EditCondition = "bIsPackAnimal"))
    float PackRadius;

    // ── Health & Survival Stats ───────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stats")
    float CurrentStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stats")
    float AttackCooldown;

    // ── Movement Speeds ───────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float TurnSpeed;

    // ── Sensory Radii ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Senses")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Senses")
    float HearingRadius;

    // ── AI State ──────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoAIState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAlerted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsAttacking;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsFleeing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bCanAttack;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    AActor* ThreatTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector LastKnownThreatLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float AlertCooldown;

    // ── Public Interface ──────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EDinoAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void AlertToThreat(AActor* Threat, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack();

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stats")
    float GetStaminaPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    bool IsAlive() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|AI")
    EDinoAIState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Senses")
    bool CanSeeTarget(AActor* Target) const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Senses")
    bool CanHearTarget(AActor* Target) const;

protected:
    // ── Blueprint Events (overridable) ────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDamageReceived(float DamageAmount, AActor* DamageCauser);
    virtual void OnDamageReceived_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnBehaviorStateChanged(EDinoAIState PreviousState, EDinoAIState NewState);
    virtual void OnBehaviorStateChanged_Implementation(EDinoAIState PreviousState, EDinoAIState NewState);

    // ── Internal ──────────────────────────────────────────────
    void Die();
    void UpdateBehaviorState();
    void RecoverStamina();

    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle StaminaRecoveryTimer;
};

// DinosaurBase.h
// Engine Architect #02 — Transpersonal Game Studio
// Base class for all dinosaur pawns in the game.
// All species (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "DinosaurBase.generated.h"

// ─── Enums (global scope — RULE 1) ────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Raptor          UMETA(DisplayName = "Raptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Spinosaurus     UMETA(DisplayName = "Spinosaurus"),
};

UENUM(BlueprintType)
enum class EDinoBehaviourState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Returning   UMETA(DisplayName = "Returning to Territory"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead"),
};

// ─── DinosaurBase ─────────────────────────────────────────────────────────────

UCLASS(Abstract, BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBase : public ACharacter
{
    GENERATED_BODY()

public:
    ADinosaurBase();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

    // ─── Survival Stats ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Hunger")
    float MaxHunger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Hunger")
    float CurrentHunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Stamina")
    float MaxStamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Stamina")
    float CurrentStamina;

    // ─── Movement Speeds ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ─── Combat ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    // ─── AI / Territory ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float TerritoryRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector TerritoryCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    FVector LastKnownThreatLocation;

    // ─── Species & State ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Identity")
    EDinoSpecies Species;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinoBehaviourState CurrentBehaviourState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAlive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsAggressive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|State")
    bool bIsHunting;

    // ─── Rates ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float HungerDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Survival")
    float StaminaRecoveryRate;

    // ─── Actions ──────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Combat")
    void PerformAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur|State")
    void Die();

    // ─── Stat Getters ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Hunger")
    float GetHungerPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Stamina")
    float GetStaminaPercent() const;

    // ─── Blueprint Events (overridable) ───────────────────────────────────────

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDamageTaken(float DamageAmount, AActor* DamageCauser);
    virtual void OnDamageTaken_Implementation(float DamageAmount, AActor* DamageCauser);

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDeath();
    virtual void OnDeath_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnAttack(AActor* Target);
    virtual void OnAttack_Implementation(AActor* Target);

private:
    FTimerHandle HungerTimerHandle;
    FTimerHandle BehaviourTimerHandle;

    void TickHunger();
    void EvaluateBehaviour();
};

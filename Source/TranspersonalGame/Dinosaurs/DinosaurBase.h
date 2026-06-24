// DinosaurBase.h
// Engine Architect #02 — PROD_CYCLE_AUTO_20260624_001
// Base class for ALL dinosaur species in the prehistoric survival game.
// Provides: species stats, behavior state machine, survival stats, damage system.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "DinosaurBase.generated.h"

// ── Enums (global scope — RULE 1) ─────────────────────────────────────────────

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    TyrannosaurusRex    UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor              UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus         UMETA(DisplayName = "Stegosaurus"),
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Unknown             UMETA(DisplayName = "Unknown")
};

UENUM(BlueprintType)
enum class EDinosaurBehavior : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Running     UMETA(DisplayName = "Running"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Dead        UMETA(DisplayName = "Dead")
};

// ── Forward declarations ──────────────────────────────────────────────────────
class UAIPerceptionStimuliSourceComponent;

// ── DinosaurBase ──────────────────────────────────────────────────────────────

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

    // ── Species ───────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Species")
    EDinosaurSpecies DinosaurSpecies;

    // ── Health ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Health")
    float MaxHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    float CurrentHealth;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Health")
    bool bIsDead;

    // ── Combat ────────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    bool bIsAggressive;

    // ── Perception ────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|AI")
    float ChaseRange;

    // ── Movement speeds ───────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Movement")
    float RunSpeed;

    // ── Behavior ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    EDinosaurBehavior CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsHunting;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|AI")
    bool bIsFleeing;

    // ── Territory ─────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Territory")
    float TerritoryRadius;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Territory")
    FVector TerritoryCenter;

    // ── Survival stats ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float HungerLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Survival")
    float ThirstLevel;

    // ── Components ────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dinosaur|Components",
        meta = (AllowPrivateAccess = "true"))
    UAIPerceptionStimuliSourceComponent* PerceptionStimuliSource;

    // ── Blueprint-callable methods ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|AI")
    void SetBehaviorState(EDinosaurBehavior NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur|Health")
    bool IsAlive() const;

    UFUNCTION(BlueprintNativeEvent, Category = "Dinosaur|Events")
    void OnDinosaurDied();
    virtual void OnDinosaurDied_Implementation();

protected:
    // ── Internal methods ──────────────────────────────────────────────────────
    void ApplySpeciesStats();
    void Die();

    UFUNCTION()
    void UpdateBehaviorState();

    UFUNCTION()
    void UpdateSurvivalStats();

    FTimerHandle SurvivalTimerHandle;
    FTimerHandle BehaviorTimerHandle;
};

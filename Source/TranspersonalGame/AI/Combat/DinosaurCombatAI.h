#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "DinosaurCombatAI.generated.h"

// ============================================================
// DinosaurCombatAI — Agent #12 Combat & Enemy AI
// Tactical combat AI for dinosaur encounters.
// Implements flanking, pack coordination, charge/retreat logic.
// ============================================================

UENUM(BlueprintType)
enum class ECombat_DinoState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Alerted     UMETA(DisplayName = "Alerted"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Charging    UMETA(DisplayName = "Charging"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Flanking    UMETA(DisplayName = "Flanking"),
    Retreating  UMETA(DisplayName = "Retreating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECombat_DinoSpecies : uint8
{
    TRex            UMETA(DisplayName = "T-Rex"),
    Velociraptor    UMETA(DisplayName = "Velociraptor"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus    UMETA(DisplayName = "Ankylosaurus"),
    Generic         UMETA(DisplayName = "Generic")
};

USTRUCT(BlueprintType)
struct FCombat_DinoStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float CurrentHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackDamage = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float ChargeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    float FleeHealthThreshold = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Stats")
    int32 PackSize = 1;
};

USTRUCT(BlueprintType)
struct FCombat_TacticalMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    float TimeSinceLastSighting = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    bool bPlayerDetected = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    bool bIsFlankingAssigned = false;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    FVector AssignedFlankPosition = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Combat|Memory")
    int32 AttackAttemptCount = 0;
};

/**
 * UCombat_DinosaurCombatComponent
 * Actor component that drives dinosaur tactical combat behaviour.
 * Attach to any SkeletalMeshActor or Pawn to give it combat AI.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Dinosaur Combat AI")
class TRANSPERSONALGAME_API UCombat_DinosaurCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurCombatComponent();

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    ECombat_DinoSpecies Species = ECombat_DinoSpecies::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    FCombat_DinoStats Stats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    bool bEnablePackCoordination = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Config")
    float FlankAngleDegrees = 90.0f;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Combat|State", meta = (AllowPrivateAccess = "true"))
    ECombat_DinoState CurrentState = ECombat_DinoState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    FCombat_TacticalMemory TacticalMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
    TWeakObjectPtr<AActor> CurrentTarget;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnTakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    ECombat_DinoState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsHostile() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    FVector CalculateFlankPosition(const FVector& TargetLocation, float FlankSide) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void TransitionToState(ECombat_DinoState NewState);

    // ---- Species presets ----
    UFUNCTION(BlueprintCallable, Category = "Combat|Config")
    void ApplySpeciesPreset(ECombat_DinoSpecies InSpecies);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdateIdleState(float DeltaTime);
    void UpdateAlertedState(float DeltaTime);
    void UpdateChargingState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdateRetreatingState(float DeltaTime);

    void ScanForPlayer();
    bool HasLineOfSightToTarget() const;
    float GetDistanceToTarget() const;

    float StateTimer = 0.0f;
    float ScanInterval = 0.5f;
    float ScanTimer = 0.0f;
};

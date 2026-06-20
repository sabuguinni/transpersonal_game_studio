#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrolling   UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing      UMETA(DisplayName = "Chasing"),
    Attacking    UMETA(DisplayName = "Attacking"),
    Resting      UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_TRexMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Memory")
    float TimeSinceLastSighting;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Memory")
    bool bHasSeenPlayer;

    UPROPERTY(BlueprintReadWrite, Category = "TRex|Memory")
    bool bIsAlerted;

    FNPC_TRexMemory()
        : LastKnownPlayerLocation(FVector::ZeroVector)
        , TimeSinceLastSighting(0.f)
        , bHasSeenPlayer(false)
        , bIsAlerted(false)
    {}
};

/**
 * TRexBehaviorComponent — drives T-Rex AI state machine
 * Patrol radius: 5000 units
 * Chase range:   3000 units
 * Attack range:  300 units
 * Memory: remembers last player position for 8 seconds after losing sight
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float PatrolRadius = 5000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float ChaseRange = 3000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackRange = 300.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float PatrolSpeed = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float ChaseSpeed = 600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackDamage = 80.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float AttackCooldown = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float MemoryDuration = 8.f;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FNPC_TRexMemory Memory;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    FVector PatrolTarget;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastAttack;

    // --- Queries ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool HasLineOfSightToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void ForceState(ENPC_TRexState NewState);

private:
    void UpdatePatrol(float DeltaTime);
    void UpdateChase(float DeltaTime);
    void UpdateAttack(float DeltaTime);
    void UpdateInvestigate(float DeltaTime);
    void UpdateResting(float DeltaTime);

    void TransitionTo(ENPC_TRexState NewState);
    void PickNewPatrolTarget();
    void MoveToward(const FVector& Target, float Speed, float DeltaTime);
    void PerformAttack();

    APawn* GetPlayerPawn() const;

    FVector InvestigateTarget;
    float RestTimer;
    float InvestigateTimer;
};

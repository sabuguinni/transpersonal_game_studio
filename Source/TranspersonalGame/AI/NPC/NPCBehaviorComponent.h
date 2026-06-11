#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FNPC_PatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WaitTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsResting = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FleeRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float MovementSpeed = 300.0f;

    // Patrol System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FNPC_PatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "Patrol")
    int32 CurrentPatrolIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    bool bLoopPatrol = true;

    // Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDuration = 10.0f;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InvestigateLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void FleeFromThreat(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanSeePlayer();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    APawn* GetNearestPlayer();

private:
    FTimerHandle BehaviorTimer;
    FTimerHandle MemoryTimer;
    
    void UpdateBehavior();
    void ProcessIdleState();
    void ProcessPatrollingState();
    void ProcessInvestigatingState();
    void ProcessFleeingState();
    void MoveToNextPatrolPoint();
    void ClearPlayerMemory();
    
    float StateTimer = 0.0f;
    FVector TargetLocation = FVector::ZeroVector;
    bool bHasValidTarget = false;
};
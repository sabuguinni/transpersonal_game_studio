#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TRexBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Investigating UMETA(DisplayName = "Investigating"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
};

USTRUCT(BlueprintType)
struct FNPC_TRexSenses
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SightAngleDegrees = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float HearingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float SmellRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float AttackRadius = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float ChaseBreakRadius = 5000.0f;
};

USTRUCT(BlueprintType)
struct FNPC_TRexMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    float TimeSinceLastPlayerSight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    bool bHasSeenPlayer = false;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    bool bIsHungry = false;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    float HungerLevel = 0.5f;
};

/**
 * TRexBehaviorComponent — drives T-Rex NPC state machine.
 * Patrol → Investigate → Chase → Attack loop.
 * Attach to any SkeletalMeshActor or Pawn representing the T-Rex.
 */
UCLASS(ClassGroup=(NPC), meta=(BlueprintSpawnableComponent), DisplayName="TRex Behavior Component")
class TRANSPERSONALGAME_API UNPCTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCTRexBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    ENPC_TRexState CurrentState = ENPC_TRexState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    FNPC_TRexSenses Senses;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Memory")
    FNPC_TRexMemory Memory;

    // --- Patrol ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    TArray<AActor*> PatrolWaypoints;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Patrol")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float PatrolAcceptanceRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    float WaypointWaitTime = 3.0f;

    // --- Combat ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown = 2.5f;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Combat")
    float TimeSinceLastAttack = 0.0f;

    // --- Blueprint Events ---
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnStateChanged(ENPC_TRexState NewState, ENPC_TRexState OldState);

    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnPlayerDetected(AActor* Player, float Distance);

    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnAttackPlayer(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnLostPlayer();

    // --- Callable ---
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void ForceState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool CanSeePlayer() const;

private:
    void UpdateStateMachine(float DeltaTime);
    void UpdateIdle(float DeltaTime);
    void UpdatePatrolling(float DeltaTime);
    void UpdateInvestigating(float DeltaTime);
    void UpdateChasing(float DeltaTime);
    void UpdateAttacking(float DeltaTime);
    void SetState(ENPC_TRexState NewState);
    AActor* FindPlayer() const;

    UPROPERTY()
    AActor* CachedPlayer = nullptr;

    float WaypointWaitTimer = 0.0f;
    float StateTimer = 0.0f;
};

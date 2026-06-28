#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AI/NPCBehavior/NPCBehaviorComponent.h"
#include "TRexBehaviorComponent.generated.h"

// T-Rex patrol and combat state
UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Patrolling       UMETA(DisplayName = "Patrolling"),
    Investigating    UMETA(DisplayName = "Investigating"),
    Chasing          UMETA(DisplayName = "Chasing"),
    Attacking        UMETA(DisplayName = "Attacking"),
    Resting          UMETA(DisplayName = "Resting"),
    Feeding          UMETA(DisplayName = "Feeding")
};

// T-Rex patrol point
USTRUCT(BlueprintType)
struct FNPC_TRexPatrolPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float WaitDuration;

    FNPC_TRexPatrolPoint()
        : Location(FVector::ZeroVector)
        , WaitDuration(5.0f)
    {}
};

/**
 * UTRexBehaviorComponent
 * Drives T-Rex patrol, detection, chase, and attack logic.
 * - Patrols a 5000-unit radius territory
 * - Chases player when within 3000 units
 * - Attacks when within 300 units
 * - Uses memory: remembers last known player position
 */
UCLASS(ClassGroup = (AI), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTRexBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTRexBehaviorComponent();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Detection")
    float FieldOfViewDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float AttackCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Memory")
    float MemoryDuration;

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    ENPC_TRexState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float TimeSincePlayerSeen;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    bool bPlayerInMemory;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastAttack;

    // --- Patrol Points ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Patrol")
    TArray<FNPC_TRexPatrolPoint> PatrolPoints;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|Patrol", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPatrolIndex;

    // --- Interface ---

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void SetTerritoryCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void GeneratePatrolPoints(int32 NumPoints = 6);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void OnPlayerDetected(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void OnPlayerLost();

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    ENPC_TRexState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInMemory() const { return bPlayerInMemory; }

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool CanHearPlayer() const;

    // --- UActorComponent overrides ---
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void UpdatePatrolState(float DeltaTime);
    void UpdateInvestigateState(float DeltaTime);
    void UpdateChaseState(float DeltaTime);
    void UpdateAttackState(float DeltaTime);
    void UpdateRestingState(float DeltaTime);
    void UpdateMemory(float DeltaTime);
    void TransitionToState(ENPC_TRexState NewState);
    void ScanForPlayer();
    FVector GetNextPatrolPoint() const;
    void MoveToLocation(const FVector& Target, float Speed);

    UPROPERTY()
    AActor* TrackedPlayer;

    float PatrolWaitTimer;
    bool bWaitingAtPatrolPoint;
};

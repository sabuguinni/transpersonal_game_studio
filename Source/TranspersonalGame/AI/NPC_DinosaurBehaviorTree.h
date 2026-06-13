#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Chasing UMETA(DisplayName = "Chasing"),
    Attacking UMETA(DisplayName = "Attacking"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Feeding UMETA(DisplayName = "Feeding"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Territorial UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
    Protoceratops UMETA(DisplayName = "Protoceratops"),
    Tsintaosaurus UMETA(DisplayName = "Tsintaosaurus")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float TerritorialRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float ChaseSpeed = 600.0f;
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerSightingTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> ThreatLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector FeedingLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bHasSeenPlayer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsInCombat = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurBehaviorState CurrentState = ENPC_DinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPC_DinosaurMemory DinosaurMemory;

    // Behavior timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StateChangeInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float PatrolPointReachDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float PlayerDetectionInterval = 0.5f;

    // References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APawn* OwnerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class APawn* PlayerPawn;

    // Behavior functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializePatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetHomeLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Detection")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void AttackPlayer();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat(const FVector& ThreatLocation);

private:
    // Internal behavior functions
    void UpdateBehaviorState(float DeltaTime);
    void ExecuteIdleBehavior(float DeltaTime);
    void ExecutePatrolBehavior(float DeltaTime);
    void ExecuteHuntingBehavior(float DeltaTime);
    void ExecuteChasingBehavior(float DeltaTime);
    void ExecuteAttackingBehavior(float DeltaTime);
    void ExecuteFleeingBehavior(float DeltaTime);
    void ExecuteFeedingBehavior(float DeltaTime);
    void ExecuteSleepingBehavior(float DeltaTime);
    void ExecuteTerritorialBehavior(float DeltaTime);

    // Utility functions
    void DetectPlayer();
    void UpdateMemory();
    FVector GetNextPatrolPoint();
    bool IsInAttackRange() const;
    bool IsPlayerInTerritory() const;
    void GenerateRandomPatrolPoints();

    // Timers
    FTimerHandle StateUpdateTimer;
    FTimerHandle PlayerDetectionTimer;

    // Internal state
    int32 CurrentPatrolIndex = 0;
    float StateTimer = 0.0f;
    float LastPlayerDistance = 0.0f;
    bool bIsMovingToTarget = false;
    FVector CurrentTargetLocation = FVector::ZeroVector;
};
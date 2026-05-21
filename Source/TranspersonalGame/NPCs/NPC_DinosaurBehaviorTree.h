#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Shared/SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_DinosaurState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Sleeping    UMETA(DisplayName = "Sleeping")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FVector LastKnownPlayerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float TimeSincePlayerSeen;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    int32 CurrentPatrolIndex;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float HungerLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    float FearLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    bool bHasSeenPlayer;

    FNPC_DinosaurMemory()
    {
        LastKnownPlayerLocation = FVector::ZeroVector;
        TimeSincePlayerSeen = 0.0f;
        CurrentPatrolIndex = 0;
        HungerLevel = 50.0f;
        FearLevel = 0.0f;
        bHasSeenPlayer = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float Health;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float MaxHealth;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float AttackDamage;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float MovementSpeed;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float DetectionRange;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float AttackRange;

    UPROPERTY(BlueprintReadWrite, Category = "Stats")
    float PatrolRadius;

    FNPC_DinosaurStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        MovementSpeed = 300.0f;
        DetectionRange = 1500.0f;
        AttackRange = 200.0f;
        PatrolRadius = 2000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void InitializeBehaviorTree(APawn* DinosaurPawn, AAIController* AIController);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void UpdateBehaviorState(ENPC_DinosaurState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    ENPC_DinosaurState GetCurrentState() const { return CurrentState; }

    // Memory system
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerMemory(FVector PlayerLocation, bool bPlayerVisible);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetLastKnownPlayerLocation() const { return DinosaurMemory.LastKnownPlayerLocation; }

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasSeenPlayer() const { return DinosaurMemory.bHasSeenPlayer; }

    // Patrol system
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetupPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void AdvancePatrolPoint();

    // Decision making
    UFUNCTION(BlueprintCallable, Category = "Decision")
    bool ShouldAttackPlayer(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Decision")
    bool ShouldFleeFromPlayer(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Decision")
    bool ShouldHuntPlayer(FVector PlayerLocation) const;

    // Blackboard integration
    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void UpdateBlackboard(UBlackboardComponent* BlackboardComp);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardValue(UBlackboardComponent* BlackboardComp, FName KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardBool(UBlackboardComponent* BlackboardComp, FName KeyName, bool Value);

    // Stats management
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void SetDinosaurStats(const FNPC_DinosaurStats& NewStats) { DinosaurStats = NewStats; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    FNPC_DinosaurStats GetDinosaurStats() const { return DinosaurStats; }

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    bool IsAlive() const { return DinosaurStats.Health > 0.0f; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_DinosaurState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    FNPC_DinosaurMemory DinosaurMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FNPC_DinosaurStats DinosaurStats;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    APawn* OwnerPawn;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AAIController* OwnerController;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior Tree")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(BlueprintReadOnly, Category = "Behavior Tree")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Blackboard")
    UBlackboardComponent* BlackboardComponent;

private:
    // Internal helper functions
    void InitializeDefaultStats();
    void GeneratePatrolPoints(FVector CenterLocation, float Radius);
    float CalculateDistanceToPlayer(FVector PlayerLocation) const;
    bool IsPlayerInRange(FVector PlayerLocation, float Range) const;
    void UpdateMemoryTimers(float DeltaTime);
};
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "SharedTypes.h"
#include "NPC_TRexBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_TRexState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Chasing     UMETA(DisplayName = "Chasing"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Territorial UMETA(DisplayName = "Territorial")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TRexStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Health = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Hunger = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float Aggression = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float TerritorialRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float ChaseRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float AttackRadius = 300.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TRexBehaviorTree : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_TRexBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core behavior tree components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // T-Rex behavior tree asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* TRexBehaviorTree;

    // T-Rex blackboard asset
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardData* TRexBlackboard;

    // T-Rex stats and state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex")
    FNPC_TRexStats TRexStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "T-Rex")
    ENPC_TRexState CurrentState;

    // Patrol system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolWaitTime = 5.0f;

    // Territory center
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    // Current target
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Target")
    APawn* CurrentTarget;

    // Timers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timers")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timers")
    float AttackCooldown = 3.0f;

public:
    // Behavior tree functions
    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "T-Rex Behavior")
    void SetTRexState(ENPC_TRexState NewState);

    UFUNCTION(BlueprintPure, Category = "T-Rex Behavior")
    ENPC_TRexState GetTRexState() const { return CurrentState; }

    // Patrol functions
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetupPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void AdvancePatrolPoint();

    // Target management
    UFUNCTION(BlueprintCallable, Category = "Target")
    void SetTarget(APawn* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Target")
    APawn* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintPure, Category = "Target")
    float GetDistanceToTarget() const;

    // Combat functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    // Territory functions
    UFUNCTION(BlueprintPure, Category = "Territory")
    bool IsInTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Territory")
    float GetDistanceFromTerritoryCenter(const FVector& Location) const;

    // Stats management
    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateHunger(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateStamina(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Stats")
    bool IsHungry() const { return TRexStats.Hunger > 70.0f; }

    UFUNCTION(BlueprintPure, Category = "Stats")
    bool IsTired() const { return TRexStats.Stamina < 30.0f; }

protected:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal state management
    void UpdateTRexBehavior(float DeltaTime);
    void HandleIdleState(float DeltaTime);
    void HandlePatrollingState(float DeltaTime);
    void HandleHuntingState(float DeltaTime);
    void HandleChasingState(float DeltaTime);
    void HandleAttackingState(float DeltaTime);

    // Utility functions
    APawn* FindNearestPrey() const;
    bool IsValidTarget(APawn* Target) const;
    void SetupAIPerception();
    void GeneratePatrolPoints();
};
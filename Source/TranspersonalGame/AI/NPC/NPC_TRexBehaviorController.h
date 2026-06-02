#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "NPC_TRexBehaviorController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;

/**
 * T-Rex AI Controller with territorial patrol and aggressive hunting behavior
 * Manages apex predator behavior patterns including territory defense and prey stalking
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_TRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_TRexBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior Tree and Blackboard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBehaviorTree* TRexBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    UBlackboardAsset* TRexBlackboard;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAISightConfig* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIHearingConfig* HearingConfig;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolWaitTime;

    // Combat and Hunting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float DetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    // Behavioral States
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPC_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector CurrentPatrolPoint;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastAttackTime;

public:
    // Behavior Control Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorState(ENPC_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartHunting(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartAttacking(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ReturnToTerritory();

    // Territory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(const FVector& NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsWithinTerritory(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetRandomPatrolPoint() const;

    // Target Detection and Evaluation
    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    AActor* FindBestTarget() const;

    UFUNCTION(BlueprintCallable, Category = "AI Perception")
    float EvaluateTargetThreat(AActor* Target) const;

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnAttackComplete();

    // Blackboard Key Management
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void UpdateBlackboardKeys();

protected:
    // Internal behavior functions
    void InitializeAIPerception();
    void SetupBehaviorTree();
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateHuntingBehavior(float DeltaTime);
    void UpdateCombatBehavior(float DeltaTime);
    
    // Utility functions
    bool IsTargetValid(AActor* Target) const;
    float GetDistanceToTarget(AActor* Target) const;
    FVector CalculateInterceptPoint(AActor* Target) const;

private:
    // Internal state tracking
    float PatrolTimer;
    float StateChangeTime;
    FVector LastKnownTargetLocation;
    bool bIsAttacking;
    
    // Blackboard key names
    static const FName BB_CurrentTarget;
    static const FName BB_PatrolPoint;
    static const FName BB_BehaviorState;
    static const FName BB_TerritoryCenter;
    static const FName BB_IsInTerritory;
};
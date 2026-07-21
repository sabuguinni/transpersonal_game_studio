#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "SharedTypes.h"
#include "TRexBehaviorController.generated.h"

class UBehaviorTree;
class UBlackboardComponent;
class APawn;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ATRexBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Sight Configuration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAISightConfig* SightConfig;

    // Patrol Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseRange = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed = 600.0f;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FVector HomeLocation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    APawn* TargetPlayer;

    // Blackboard Keys
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PlayerActorKey = TEXT("PlayerActor");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName PatrolLocationKey = TEXT("PatrolLocation");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName BehaviorStateKey = TEXT("BehaviorState");

    UPROPERTY(EditDefaultsOnly, Category = "AI")
    FName HomeLocationKey = TEXT("HomeLocation");

public:
    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartPatrolling();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartChasing(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartAttacking(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void ReturnToPatrol();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    FVector GetRandomPatrolLocation();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    bool IsPlayerInRange(float Range) const;

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Internal Functions
    void UpdateBehaviorState();
    void SetBlackboardBehaviorState(ENPCBehaviorState NewState);
    void InitializeBlackboard();
};
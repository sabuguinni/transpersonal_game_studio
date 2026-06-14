#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "CombatAIController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat AI behavior
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StartCombatBehavior(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void StopCombatBehavior();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState() const { return CurrentCombatState; }

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // Combat state
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ChaseRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float HearingRange;

    // Combat timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    float LastAttackTime;

    // AI decision making
    void UpdateCombatDecisions();
    void HandlePatrolState();
    void HandleChaseState();
    void HandleAttackState();
    void HandleFleeState();

    // Utility functions
    bool CanSeeTarget(AActor* Target) const;
    bool IsInAttackRange(AActor* Target) const;
    float GetDistanceToTarget(AActor* Target) const;
};
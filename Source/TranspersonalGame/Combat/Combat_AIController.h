#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../SharedTypes.h"
#include "Combat_AIController.generated.h"

class UBehaviorTree;
class APawn;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

    // AI Behavior Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Trees for different dinosaur types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* CarnivoreBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* HerbivoreBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* ApexPredatorBehaviorTree;

    // Combat State
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    TArray<AActor*> PackMembers;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float FleeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    float AggressionLevel;

public:
    // Combat AI Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddPackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemovePackMember(AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInAttackRange(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteFleeResponse();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

protected:
    // AI Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Combat Logic
    void EvaluateThreat(AActor* ThreatActor);
    void UpdateCombatBehavior();
    void SelectBehaviorTree();
    
    // Pack Coordination
    void BroadcastPackSignal(ECombat_PackSignal Signal);
    void RespondToPackSignal(ECombat_PackSignal Signal, AActor* Sender);

private:
    float LastAttackTime;
    float AttackCooldown;
    bool bIsInCombat;
    bool bIsPackLeader;
};
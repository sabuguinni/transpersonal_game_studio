#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../SharedTypes.h"
#include "Combat_DinosaurAIController.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogCombatAI, Log, All);

/**
 * Advanced AI Controller for dinosaur combat behavior
 * Handles tactical decision making, threat assessment, and pack coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_DinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_DinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Combat AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UAIPerceptionComponent* PerceptionComponent;

    // Combat Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBehaviorTree* CombatBehaviorTree;

    // Combat States
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombat_ThreatLevel ThreatLevel;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float CombatAggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float PackCoordinationRange;

    // Target Management
    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    TArray<AActor*> ThreatActors;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    TArray<ACombat_DinosaurAIController*> PackAllies;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* Target);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterPackAlly(ACombat_DinosaurAIController* Ally);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SendPackSignal(ECombat_PackSignal Signal);

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AttackType SelectAttackType();

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Logic
    void UpdateCombatBehavior(float DeltaTime);
    void EvaluateThreats();
    void UpdatePackCoordination();
    void ExecuteCombatDecision();

    // Timers
    float LastThreatEvaluation;
    float LastPackUpdate;
    float CombatStateTimer;

    // Combat Memory
    TMap<AActor*, float> ThreatMemory;
    TMap<AActor*, FVector> LastKnownPositions;
    
    // Pack Communication
    float LastPackSignal;
    ECombat_PackSignal LastReceivedSignal;
};
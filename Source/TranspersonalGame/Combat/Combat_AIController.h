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

class UCombat_ThreatAssessment;
class UCombat_TacticalAI;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI") 
    class UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UAIPerceptionComponent* PerceptionComp;

    // Combat Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_ThreatAssessment* ThreatAssessment;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_TacticalAI* TacticalAI;

    // Behavior Tree Asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_DinosaurSpecies DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIState CurrentAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TerritorialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsApexPredator;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetAIState() const { return CurrentAIState; }

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreat(AActor* PotentialThreat);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetThreatLevel(AActor* Target);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ACombat_AIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsInPack() const;

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FlankTarget(AActor* Target, FVector FlankPosition);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RetreatFromCombat();

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal State Management
    void UpdateAIBehavior(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteTacticalDecision();

    // Pack Management
    UPROPERTY()
    TArray<ACombat_AIController*> PackMembers;

    UPROPERTY()
    ACombat_AIController* PackLeader;

    UPROPERTY()
    bool bIsPackLeader;

    // Combat Timers
    float LastAttackTime;
    float LastThreatAssessmentTime;
    float CombatCooldownTime;

    // Blackboard Keys
    FName TargetActorKey;
    FName AIStateKey;
    FName ThreatLevelKey;
    FName PackPositionKey;
};
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Combat/CombatTypes.h"
#include "CombatAIController.generated.h"

class APawn;
class UBehaviorTree;
class UBlackboardData;

/**
 * Advanced AI Controller for combat scenarios
 * Handles tactical decision making, pack coordination, and threat assessment
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;
    virtual void UnPossess() override;

    // Combat behavior tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    // Blackboard for combat decisions
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI")
    UBlackboardData* CombatBlackboard;

    // AI Perception component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Sight configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Perception")
    float SightRadius = 2000.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Perception")
    float LoseSightRadius = 2500.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Perception")
    float FieldOfView = 120.0f;

    // Hearing configuration
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Perception")
    float HearingRange = 1500.0f;

    // Combat behavior parameters
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Behavior")
    ECombat_AggressionLevel AggressionLevel = ECombat_AggressionLevel::Moderate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Behavior")
    ECombat_TacticalRole TacticalRole = ECombat_TacticalRole::Flanker;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat AI|Behavior")
    float FleeHealthThreshold = 0.3f;

    // Pack coordination
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    ACombatAIController* PackLeader;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI|Pack")
    bool bIsPackLeader = false;

public:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat decision making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* GetCombatTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFleeFromCombat() const;

    // Pack coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Pack")
    void CoordinatePackAttack(AActor* Target);

    // Tactical positioning
    UFUNCTION(BlueprintCallable, Category = "Combat AI|Tactics")
    FVector GetFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Tactics")
    FVector GetAmbushPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI|Tactics")
    bool IsInOptimalAttackPosition(AActor* Target) const;

    // Combat state queries
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_CombatState GetCombatState() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombat_CombatState NewState);

protected:
    // Internal combat state
    ECombat_CombatState CurrentCombatState = ECombat_CombatState::Idle;
    
    // Current combat target
    UPROPERTY()
    AActor* CombatTarget;

    // Threat assessment
    float AssessThreatLevel(AActor* Target) const;
    bool IsTargetTooStrong(AActor* Target) const;
    bool HasAdvantageousPosition(AActor* Target) const;

    // Pack behavior helpers
    void UpdatePackFormation();
    void SendPackCommand(ECombat_PackCommand Command, AActor* Target = nullptr);
    void ReceivePackCommand(ECombat_PackCommand Command, AActor* Target);

private:
    // Perception configuration
    void SetupPerception();
    void ConfigureSight();
    void ConfigureHearing();

    // Combat timer handles
    FTimerHandle CombatUpdateTimer;
    FTimerHandle ThreatAssessmentTimer;

    // Combat update frequency
    void UpdateCombatBehavior();
    void AssessThreats();
};
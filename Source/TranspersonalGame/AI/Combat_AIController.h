#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "Combat_AIController.generated.h"

class UCombat_BehaviorTreeManager;
class ACombat_DinosaurPawn;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float Distance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bIsPlayerTarget = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIBehaviorState CurrentBehaviorState = ECombat_AIBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_FormationRole FormationRole = ECombat_FormationRole::Solo;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatTimer = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombat_AIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_AIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Combat AI Core Systems
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombat_BehaviorTreeManager* BehaviorTreeManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UBlackboardComponent* BlackboardComponent;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Config")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Config")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Config")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Config")
    float FlankingDistance = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Config")
    float RetreatThreshold = 0.3f;

    // Combat State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_ThreatAssessment CurrentThreat;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    FCombat_TacticalState TacticalState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> AlliedUnits;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    AActor* PrimaryTarget;

public:
    // Combat AI Interface
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetBehaviorState(ECombat_AIBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssignFormationRole(ECombat_FormationRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatAssessment(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanExecuteAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterAlliedUnit(AActor* AllyActor);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterAlliedUnit(AActor* AllyActor);

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat Logic
    void UpdateCombatState(float DeltaTime);
    void ProcessThreatAssessment();
    void ExecuteCombatBehavior();
    void UpdateFormationPosition();
    void HandleCombatCommunication();

    // Tactical Decision Making
    ECombat_TacticalManeuver SelectOptimalManeuver() const;
    bool ShouldEngageTarget() const;
    bool ShouldRetreat() const;
    FVector FindOptimalCoverPosition() const;

    // Formation Management
    void UpdateFormationRole();
    FVector CalculateFormationPosition() const;
    void CoordinateWithAllies();

private:
    float LastThreatUpdateTime = 0.0f;
    float LastCommunicationTime = 0.0f;
    float CombatStateUpdateInterval = 0.1f;
    float CommunicationInterval = 2.0f;
};
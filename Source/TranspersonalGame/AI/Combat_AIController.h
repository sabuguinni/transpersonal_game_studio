#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Combat_AIController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_TacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    ECombat_TacticalRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tactics")
    bool bIsOccupied;

    FCombat_TacticalPosition()
    {
        Position = FVector::ZeroVector;
        Role = ECombat_TacticalRole::Flanker;
        Priority = 1.0f;
        bIsOccupied = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_PackCoordination
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<ACombat_AIController*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ACombat_AIController* AlphaLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    ECombat_PackFormation CurrentFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* PrimaryTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<FCombat_TacticalPosition> TacticalPositions;

    FCombat_PackCoordination()
    {
        AlphaLeader = nullptr;
        CurrentFormation = ECombat_PackFormation::Scatter;
        PrimaryTarget = nullptr;
    }
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

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComp;

    // Combat AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    class UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_AIPersonality AIPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    ECombat_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CautiousLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float PackLoyalty;

    // Tactical Combat System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    FCombat_PackCoordination PackData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    ECombat_TacticalRole AssignedRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Combat")
    TArray<AActor*> KnownAllies;

    // Combat State Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_AIState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    float TimeLastSeenTarget;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRadius;

public:
    // Combat AI Interface
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatAI();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAIPersonality(ECombat_AIPersonality NewPersonality);

    // Pack Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void JoinPack(ACombat_AIController* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AssignTacticalRole(ECombat_TacticalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void CoordinatePackAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void SetPackFormation(ECombat_PackFormation Formation);

    // Tactical Combat
    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    FVector CalculateFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    bool CanExecuteAmbush();

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void ExecuteTacticalManeuver(ECombat_TacticalManeuver Maneuver);

    UFUNCTION(BlueprintCallable, Category = "Tactical Combat")
    void UpdateTacticalPositions();

    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void EnterCombatState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    void ExitCombatState();

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool IsInCombat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat State")
    bool HasValidTarget() const;

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal Combat Logic
    void ProcessCombatLogic(float DeltaTime);
    void UpdatePackCoordination();
    void EvaluateThreatLevel();
    void SelectOptimalTarget();
    void ExecuteCombatBehavior();

    // Tactical Analysis
    void AnalyzeTacticalSituation();
    void CalculateOptimalPositioning();
    void AssessPackFormationEffectiveness();

    // Combat Decision Making
    bool ShouldEngageTarget(AActor* Target);
    bool ShouldRetreat();
    bool ShouldCallForBackup();
    ECombat_TacticalManeuver SelectBestManeuver();

private:
    // Combat Timers
    float CombatUpdateTimer;
    float TacticalAnalysisTimer;
    float PackCoordinationTimer;

    // Combat Flags
    bool bIsPackLeader;
    bool bInTacticalFormation;
    bool bExecutingManeuver;
};
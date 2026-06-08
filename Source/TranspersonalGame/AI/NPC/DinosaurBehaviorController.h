#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "DinosaurBehaviorController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurBehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurBehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* DefaultBlackboard;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 2000.0f;

    // Behavior States
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    ENPCBehaviorState CurrentBehaviorState = ENPCBehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.3f;

    // Territory and Patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<FVector> PatrolPoints;

    UPROPERTY(BlueprintReadWrite, Category = "Territory")
    int32 CurrentPatrolIndex = 0;

    // Pack Behavior
    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    TArray<ADinosaurBehaviorController*> PackMembers;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    bool bIsPackLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    ADinosaurBehaviorController* PackLeader = nullptr;

    // Combat and Threat Response
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float AttackRange = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float ChaseRange = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    float FleeThreshold = 0.8f;

public:
    // Behavior Control Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateEmotionalState(float DeltaAggression, float DeltaFear, float DeltaHunger);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritoryCenter(FVector NewCenter);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void AddPatrolPoint(FVector PatrolPoint);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(FVector Location) const;

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack")
    void JoinPack(ADinosaurBehaviorController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void AddPackMember(ADinosaurBehaviorController* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void RemovePackMember(ADinosaurBehaviorController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack")
    void BroadcastToPackMembers(ENPCBehaviorState NewState);

    // Combat Functions
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ClearTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInAttackRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool IsTargetInChaseRange() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void InitiateCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void FleeFromThreat(AActor* Threat);

protected:
    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Behavior Logic
    void InitializePerception();
    void SetupBlackboardKeys();
    void UpdateBehaviorTree();
    void ProcessThreatAssessment();
    void UpdatePackCommunication();

    // Blackboard Keys
    FName TargetActorKey = TEXT("TargetActor");
    FName BehaviorStateKey = TEXT("BehaviorState");
    FName PatrolPointKey = TEXT("PatrolPoint");
    FName TerritoryLocationKey = TEXT("TerritoryLocation");
    FName AggressionLevelKey = TEXT("AggressionLevel");
    FName FearLevelKey = TEXT("FearLevel");
    FName HungerLevelKey = TEXT("HungerLevel");
    FName PackLeaderKey = TEXT("PackLeader");
    FName IsInCombatKey = TEXT("IsInCombat");
};
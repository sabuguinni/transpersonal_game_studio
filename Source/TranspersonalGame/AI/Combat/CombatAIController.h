// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "GameplayTags.h"
#include "Engine/World.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class UCombatTacticsComponent;
class UThreatAssessmentComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Engaging,
    Retreating,
    Coordinating,
    Ambushing
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Critical
};

USTRUCT(BlueprintType)
struct FCombatTarget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* TargetActor = nullptr;

    UPROPERTY(BlueprintReadWrite)
    EThreatLevel ThreatLevel = EThreatLevel::None;

    UPROPERTY(BlueprintReadWrite)
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float DistanceToTarget = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsVisible = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsInAttackRange = false;

    FCombatTarget()
    {
        TargetActor = nullptr;
        ThreatLevel = EThreatLevel::None;
        LastSeenTime = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        DistanceToTarget = 0.0f;
        bIsVisible = false;
        bIsInAttackRange = false;
    }
};

/**
 * Advanced Combat AI Controller for dinosaur predators and enemies
 * Implements tactical combat behaviors, threat assessment, and coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombatTacticsComponent* CombatTacticsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UThreatAssessmentComponent* ThreatAssessmentComponent;

    // Behavior Tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    // Combat State
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FCombatTarget PrimaryTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FCombatTarget> SecondaryTargets;

    // Combat Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float HearingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float FearThreshold = 0.3f;

    // Coordination
    UPROPERTY(BlueprintReadOnly, Category = "Combat Coordination")
    TArray<ACombatAIController*> AlliedControllers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Coordination")
    float CoordinationRadius = 2000.0f;

public:
    // Combat State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    AActor* GetPrimaryTarget() const { return PrimaryTarget.TargetActor; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AddSecondaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RemoveTarget(AActor* Target);

    // Combat Actions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInSight(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target) const;

    // Coordination
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterAlliedController(ACombatAIController* AllyController);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterAlliedController(ACombatAIController* AllyController);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void BroadcastThreatToAllies(AActor* ThreatActor, EThreatLevel Level);

    // Blackboard Keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName TargetActorKey = TEXT("TargetActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName CombatStateKey = TEXT("CombatState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName ThreatLevelKey = TEXT("ThreatLevel");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName LastKnownLocationKey = TEXT("LastKnownLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName AttackRangeKey = TEXT("AttackRange");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName AggressionKey = TEXT("Aggression");

protected:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal Functions
    void UpdateCombatState();
    void UpdateTargets();
    void UpdateBlackboard();
    void FindNearbyAllies();
    EThreatLevel CalculateThreatLevel(AActor* Target) const;
    bool ShouldEngageTarget(AActor* Target) const;
    bool ShouldRetreat() const;

    // Timers
    FTimerHandle CombatUpdateTimer;
    FTimerHandle CoordinationTimer;

    void OnCombatUpdate();
    void OnCoordinationUpdate();
};
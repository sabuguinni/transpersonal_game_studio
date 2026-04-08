#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "CombatAITypes.h"
#include "CombatAIBlackboard.h"
#include "CombatAIController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class ACombatAIPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatAIState, OldState, ECombatAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerLost);

/**
 * Main AI Controller for Combat AI entities
 * Manages behavior trees, perception, and combat decision making
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Combat AI Events")
    FOnCombatStateChanged OnCombatStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI Events")
    FOnThreatDetected OnThreatDetected;

    UPROPERTY(BlueprintAssignable, Category = "Combat AI Events")
    FOnPlayerLost OnPlayerLost;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBlackboardAsset* BlackboardAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    EDinosaurArchetype DinosaurArchetype = EDinosaurArchetype::PackHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatAIStats CombatStats;

    // Pack behavior
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ACombatAIController*> PackMembers;

    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    ACombatAIController* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCoordinationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize = 6;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Combat AI Blackboard
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombatAIBlackboard* CombatBlackboard;

    // Behavior Tree
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    // Current state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombatAIState CurrentCombatState = ECombatAIState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombatMemory CombatMemory;

    // Timers
    FTimerHandle StateUpdateTimer;
    FTimerHandle PerceptionUpdateTimer;
    FTimerHandle PackCoordinationTimer;

public:
    // State Management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatAIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombatAIState GetCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetThreatLevel(ECombatThreatLevel NewThreatLevel);

    // Pack Management
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(ACombatAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(ACombatAIController* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(ACombatAIController* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool IsPackLeader() const { return PackLeader == nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    int32 GetPackSize() const { return PackMembers.Num() + 1; } // +1 for self

    // Combat Queries
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetLastKnownPlayerLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngagePlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFleeFromPlayer() const;

    // Utility
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ACombatAIPawn* GetCombatPawn() const;

protected:
    // AI Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Internal updates
    void UpdateCombatState();
    void UpdatePackCoordination();
    void UpdateThreatAssessment();
    void UpdateMemory(float DeltaTime);

    // Behavior Tree management
    void StartBehaviorTree();
    void StopBehaviorTree();

    // Pack coordination
    void CoordinatePackBehavior();
    void BroadcastPackCommand(ECombatAIState NewState);

private:
    float LastStateUpdateTime = 0.0f;
    float StateUpdateInterval = 0.1f; // Update state 10 times per second
};
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "NPC_DinosaurBehaviorTree.generated.h"

/**
 * Behavior Tree system for dinosaur NPCs
 * Manages state transitions between: Idle, Patrol, Hunt, Flee, Feed, Sleep
 * Handles pack coordination for social species like Velociraptors
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI))
class TRANSPERSONALGAME_API UNPC_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void RestartBehaviorTree();

    // State management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetCurrentState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    ENPCBehaviorState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void TransitionToState(ENPCBehaviorState NewState, float TransitionDelay = 0.0f);

    // Blackboard management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBlackboardValue(const FString& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBlackboardVector(const FString& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Behavior")
    void SetBlackboardObject(const FString& KeyName, UObject* Value);

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    float GetBlackboardValue(const FString& KeyName) const;

    // Behavior queries
    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsInCombat() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsHunting() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsFleeing() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsFeeding() const;

    UFUNCTION(BlueprintPure, Category = "Dinosaur Behavior")
    bool IsPatrolling() const;

    // Pack behavior (for social species)
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(UNPC_DinosaurBehaviorTree* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsInPack() const { return PackLeader != nullptr; }

    UFUNCTION(BlueprintPure, Category = "Pack Behavior")
    bool IsPackLeader() const { return bIsPackLeader; }

    // Threat detection
    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void DetectThreats();

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void AddThreat(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Threat Detection")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintPure, Category = "Threat Detection")
    AActor* GetHighestThreat() const;

protected:
    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree", meta = (AllowPrivateAccess = "true"))
    UBlackboardComponent* BlackboardComponent;

    // Current state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ENPCBehaviorState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    ENPCBehaviorState PreviousState;

    // State timing
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float StateTimer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State", meta = (AllowPrivateAccess = "true"))
    float StateTransitionDelay;

    // Pack behavior
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    UNPC_DinosaurBehaviorTree* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    TArray<UNPC_DinosaurBehaviorTree*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    int32 MaxPackSize;

    // Threat tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Threats", meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, float> ThreatMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threats", meta = (AllowPrivateAccess = "true"))
    float ThreatDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Threats", meta = (AllowPrivateAccess = "true"))
    float ThreatDecayRate;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float HuntRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (AllowPrivateAccess = "true"))
    float FearLevel;

private:
    // Internal state management
    void UpdateStateTimer(float DeltaTime);
    void ProcessStateTransition();
    void UpdateThreatDecay(float DeltaTime);
    void UpdatePackBehavior();

    // Blackboard key names
    static const FString BB_CurrentState;
    static const FString BB_TargetLocation;
    static const FString BB_TargetActor;
    static const FString BB_ThreatLevel;
    static const FString BB_PackLeader;
    static const FString BB_IsInPack;
};
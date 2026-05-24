#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "SharedTypes.h"
#include "NPC_BehaviorTreeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_DinosaurState CurrentState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HungerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float LastPlayerInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPackLeader = false;

    FNPC_BehaviorContext()
    {
        Species = ENPC_DinosaurSpecies::TRex;
        CurrentState = ENPC_DinosaurState::Idle;
        AggressionLevel = 0.5f;
        FearLevel = 0.0f;
        HungerLevel = 0.3f;
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 5000.0f;
        CurrentTarget = nullptr;
        PackMembers.Empty();
        LastPlayerInteractionTime = 0.0f;
        bIsPackLeader = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_BehaviorRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    ENPC_DinosaurState TriggerState = ENPC_DinosaurState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    ENPC_DinosaurState TargetState = ENPC_DinosaurState::Patrolling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float MinAggressionLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float MaxAggressionLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float PlayerDistanceThreshold = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rule")
    float CooldownTime = 5.0f;

    FNPC_BehaviorRule()
    {
        TriggerState = ENPC_DinosaurState::Idle;
        TargetState = ENPC_DinosaurState::Patrolling;
        MinAggressionLevel = 0.0f;
        MaxAggressionLevel = 1.0f;
        PlayerDistanceThreshold = 3000.0f;
        Priority = 1.0f;
        bRequiresLineOfSight = true;
        CooldownTime = 5.0f;
    }
};

/**
 * Manages behavior trees and AI decision making for NPCs
 * Handles state transitions, pack behavior, and territorial AI
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UObject
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

    // Core behavior management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void InitializeBehaviorTree(AAIController* AIController, ENPC_DinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorContext(AAIController* AIController, const FNPC_BehaviorContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_DinosaurState EvaluateStateTransition(const FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ExecuteStateChange(AAIController* AIController, ENPC_DinosaurState NewState);

    // Pack behavior
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RegisterPackMember(AActor* PackLeader, AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(AActor* PackLeader, AActor* PackMember);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    TArray<AActor*> GetPackMembers(AActor* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackBehavior(AActor* PackLeader, ENPC_DinosaurState TargetState);

    // Territory management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsWithinTerritory(const FVector& Position, const FVector& TerritoryCenter, float TerritoryRadius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    FVector GetRandomPatrolPoint(const FVector& TerritoryCenter, float TerritoryRadius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(AAIController* AIController, AActor* Intruder);

    // Player interaction
    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void OnPlayerDetected(AAIController* AIController, AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    void OnPlayerLost(AAIController* AIController, AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Player Interaction")
    bool ShouldAttackPlayer(const FNPC_BehaviorContext& Context, float PlayerDistance);

    // Behavior rules
    UFUNCTION(BlueprintCallable, Category = "Behavior Rules")
    void AddBehaviorRule(const FNPC_BehaviorRule& Rule);

    UFUNCTION(BlueprintCallable, Category = "Behavior Rules")
    void RemoveBehaviorRule(ENPC_DinosaurState TriggerState, ENPC_DinosaurState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Behavior Rules")
    FNPC_BehaviorRule GetBestMatchingRule(const FNPC_BehaviorContext& Context);

    // Debug and utilities
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBehaviorInfo(AActor* DinosaurActor, const FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetBehaviorStateString(ENPC_DinosaurState State);

protected:
    // Behavior trees for different species
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    TMap<ENPC_DinosaurSpecies, UBehaviorTree*> SpeciesBehaviorTrees;

    // Behavior rules
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Rules")
    TArray<FNPC_BehaviorRule> BehaviorRules;

    // Pack tracking
    UPROPERTY()
    TMap<AActor*, TArray<AActor*>> PackMemberships;

    // Context tracking
    UPROPERTY()
    TMap<AAIController*, FNPC_BehaviorContext> BehaviorContexts;

    // Helper functions
    void SetupDefaultBehaviorRules();
    void InitializeSpeciesBehaviorTrees();
    bool CheckLineOfSight(AActor* Observer, AActor* Target);
    float CalculateAggressionModifier(ENPC_DinosaurSpecies Species, const FNPC_BehaviorContext& Context);
    void UpdatePackCoordination(AActor* PackLeader);
};
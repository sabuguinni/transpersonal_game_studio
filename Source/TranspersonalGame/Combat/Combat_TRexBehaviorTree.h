#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat_TRexBehaviorTree.generated.h"

/**
 * T-Rex Combat Behavior Tree Data Asset
 * Defines the AI behavior patterns for T-Rex combat encounters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombat_TRexBehaviorTree : public UDataAsset
{
    GENERATED_BODY()

public:
    UCombat_TRexBehaviorTree();

    // Main behavior tree for T-Rex combat AI
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBehaviorTree* TRexCombatBehaviorTree;

    // Blackboard for T-Rex AI state management
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat AI")
    class UBlackboardData* TRexBlackboard;

    // Combat behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float DetectionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float AttackRange = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float ChaseSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float AttackCooldown = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Parameters")
    float RoarCooldown = 8.0f;

    // Territory and hunting behavior
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Territory")
    float TerritoryRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunting")
    float HuntingPersistence = 15.0f; // seconds to chase before giving up

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hunting")
    float LostTargetSearchTime = 5.0f;

    // Combat state thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat States")
    float AggroThreshold = 0.7f; // Health percentage to become aggressive

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat States")
    float FleeThreshold = 0.2f; // Health percentage to flee

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat States")
    float EnrageThreshold = 0.3f; // Health percentage to enter rage mode

    // Initialize behavior tree data
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeBehaviorTree();

    // Get behavior tree for AI controller
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    UBehaviorTree* GetTRexBehaviorTree() const { return TRexCombatBehaviorTree; }

    // Get blackboard for AI controller
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    UBlackboardData* GetTRexBlackboard() const { return TRexBlackboard; }
};
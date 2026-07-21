#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "CombatBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ECombat_AIState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Defending   UMETA(DisplayName = "Defending")
};

UENUM(BlueprintType)
enum class ECombat_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState = ECombat_AIState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FearLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AlertRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeRadius = 200.0f;

    FCombat_BehaviorData()
    {
        CurrentState = ECombat_AIState::Idle;
        ThreatLevel = ECombat_ThreatLevel::None;
        AggressionLevel = 0.5f;
        FearLevel = 0.3f;
        AlertRadius = 1000.0f;
        AttackRadius = 500.0f;
        FleeRadius = 200.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatBehaviorTree : public UObject
{
    GENERATED_BODY()

public:
    UCombatBehaviorTree();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* DinosaurBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBlackboardAsset* DinosaurBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombat_BehaviorData DefaultBehaviorData;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitializeCombatBehavior(class AAIController* AIController);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel(ECombat_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TransitionToState(ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanTransitionToState(ECombat_AIState TargetState) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateAggressionModifier(float DistanceToTarget) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetBlackboardValues(UBlackboardComponent* BlackboardComp);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    FCombat_BehaviorData CurrentBehaviorData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TMap<ECombat_AIState, TArray<ECombat_AIState>> ValidStateTransitions;

    void InitializeStateTransitions();
    bool IsValidTransition(ECombat_AIState FromState, ECombat_AIState ToState) const;
};
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NPCBehaviorSystem.h"
#include "BTTask_UpdateConsciousness.generated.h"

UCLASS()
class TRANSPERSONALGAME_API UBTTask_UpdateConsciousness : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_UpdateConsciousness();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Consciousness")
    FBlackboardKeySelector PlayerConsciousnessKey;

    UPROPERTY(EditAnywhere, Category = "Consciousness")
    FBlackboardKeySelector NPCConsciousnessKey;

    UPROPERTY(EditAnywhere, Category = "Consciousness")
    FBlackboardKeySelector EmotionalResonanceKey;

    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float ConsciousnessUpdateRate = 0.1f;

    UPROPERTY(EditAnywhere, Category = "Consciousness")
    float MaxInfluenceDistance = 500.0f;

private:
    float CalculateConsciousnessInfluence(class UNPCBehaviorComponent* NPCBehavior, class ACharacter* Player);
    void UpdateBlackboardValues(UBehaviorTreeComponent& OwnerComp, float PlayerConsciousness, float NPCConsciousness, float Resonance);
};

UCLASS()
class TRANSPERSONALGAME_API UBTTask_TriggerTransformation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_TriggerTransformation();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Transformation")
    FBlackboardKeySelector TransformationStateKey;

    UPROPERTY(EditAnywhere, Category = "Transformation")
    float TransformationDuration = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Transformation")
    bool bRequirePlayerPresence = true;
};

UCLASS()
class TRANSPERSONALGAME_API UBTTask_ArchetypeBehavior : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ArchetypeBehavior();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Archetype")
    ENPCArchetype TargetArchetype = ENPCArchetype::ShadowSelf;

    UPROPERTY(EditAnywhere, Category = "Archetype")
    FBlackboardKeySelector ArchetypeKey;

    UPROPERTY(EditAnywhere, Category = "Archetype")
    TArray<FString> ArchetypeAnimations;

    UPROPERTY(EditAnywhere, Category = "Archetype")
    float BehaviorDuration = 2.0f;

private:
    void ExecuteShadowBehavior(class UNPCBehaviorComponent* NPCBehavior);
    void ExecuteWiseGuideBehavior(class UNPCBehaviorComponent* NPCBehavior);
    void ExecuteInnerChildBehavior(class UNPCBehaviorComponent* NPCBehavior);
    void ExecuteAnimaBehavior(class UNPCBehaviorComponent* NPCBehavior);
};
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"
#include "ConsciousnessAIComponent.h"
#include "TranspersonalBehaviorTree.generated.h"

// Custom Task: Meditate
UCLASS()
class TRANSPERSONALGAME_API UBTTask_Meditate : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_Meditate();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float MeditationDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float ConsciousnessGain = 0.1f;

private:
    float StartTime;
};

// Custom Task: Seek Wisdom
UCLASS()
class TRANSPERSONALGAME_API UBTTask_SeekWisdom : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_SeekWisdom();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    float SearchRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wisdom")
    TArray<FString> WisdomSources;
};

// Custom Task: Share Knowledge
UCLASS()
class TRANSPERSONALGAME_API UBTTask_ShareKnowledge : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_ShareKnowledge();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    float TeachingRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    float KnowledgeTransferRate = 0.05f;
};

// Custom Task: Respond to Player
UCLASS()
class TRANSPERSONALGAME_API UBTTask_RespondToPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_RespondToPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float ResponseDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bUseConsciousnessBasedResponse = true;
};

// Service: Update Consciousness
UCLASS()
class TRANSPERSONALGAME_API UBTService_UpdateConsciousness : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_UpdateConsciousness();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FBlackboardKeySelector ConsciousnessLevelKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FBlackboardKeySelector EmotionalStateKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FBlackboardKeySelector PlayerRelationshipKey;
};

// Service: Detect Spiritual Events
UCLASS()
class TRANSPERSONALGAME_API UBTService_DetectSpiritualEvents : public UBTService
{
    GENERATED_BODY()

public:
    UBTService_DetectSpiritualEvents();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    FBlackboardKeySelector SpiritualEventKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    FBlackboardKeySelector EventIntensityKey;
};

// Decorator: Consciousness Level Check
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_ConsciousnessLevel : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_ConsciousnessLevel();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    EConsciousnessState RequiredState = EConsciousnessState::Aware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    bool bCheckMinimumLevel = true;
};

// Decorator: Player Relationship Check
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_PlayerRelationship : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_PlayerRelationship();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float MinimumRelationship = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationship")
    float MaximumRelationship = 1.0f;
};

// Decorator: Spiritual Resonance Check
UCLASS()
class TRANSPERSONALGAME_API UBTDecorator_SpiritualResonance : public UBTDecorator
{
    GENERATED_BODY()

public:
    UBTDecorator_SpiritualResonance();

protected:
    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance")
    float MinimumResonance = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resonance")
    bool bRequireEnvironmentalHarmony = false;
};
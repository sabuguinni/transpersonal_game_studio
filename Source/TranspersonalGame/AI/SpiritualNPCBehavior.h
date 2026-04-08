// SpiritualNPCBehavior.h
// Sistema de comportamento para NPCs espirituais que respondem aos estados de consciência

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Core/ConsciousnessSystem.h"
#include "SpiritualNPCBehavior.generated.h"

UENUM(BlueprintType)
enum class ESpiritualNPCState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"), 
    Aware           UMETA(DisplayName = "Aware"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class ESpiritualNPCType : uint8
{
    Guide           UMETA(DisplayName = "Spiritual Guide"),
    Guardian        UMETA(DisplayName = "Consciousness Guardian"),
    Seeker          UMETA(DisplayName = "Fellow Seeker"),
    Shadow          UMETA(DisplayName = "Shadow Aspect"),
    Archetype       UMETA(DisplayName = "Archetypal Being")
};

USTRUCT(BlueprintType)
struct FSpiritualDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState RequiredPlayerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESpiritualNPCState RequiredNPCState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessImpact;

    FSpiritualDialogue()
    {
        DialogueText = "";
        RequiredPlayerState = EConsciousnessState::Ordinary;
        RequiredNPCState = ESpiritualNPCState::Dormant;
        ConsciousnessImpact = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FSpiritualBehaviorPattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESpiritualNPCState TriggerState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BehaviorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> MovementPattern;

    FSpiritualBehaviorPattern()
    {
        TriggerState = ESpiritualNPCState::Dormant;
        BehaviorName = "Idle";
        Duration = 5.0f;
        bLooping = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API USpiritualNPCBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    USpiritualNPCBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual NPC")
    ESpiritualNPCType NPCType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual NPC")
    ESpiritualNPCState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual NPC")
    float ConsciousnessResonance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual NPC")
    float AwarenessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spiritual NPC")
    bool bCanTranscend;

    // Dialogue System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FSpiritualDialogue> DialogueOptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString CurrentDialogue;

    // Behavior Patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FSpiritualBehaviorPattern> BehaviorPatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FSpiritualBehaviorPattern CurrentBehavior;

    // Consciousness Interaction
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    class UConsciousnessSystem* PlayerConsciousness;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    EConsciousnessState LastPlayerState;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Spiritual NPC")
    void UpdateNPCState(EConsciousnessState PlayerState);

    UFUNCTION(BlueprintCallable, Category = "Spiritual NPC")
    void RespondToPlayerConsciousness(float PlayerConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Spiritual NPC")
    FString GetContextualDialogue(EConsciousnessState PlayerState);

    UFUNCTION(BlueprintCallable, Category = "Spiritual NPC")
    void TriggerSpiritualInteraction();

    UFUNCTION(BlueprintCallable, Category = "Spiritual NPC")
    void ExecuteBehaviorPattern(ESpiritualNPCState TargetState);

    // Advanced Behaviors
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior")
    void EnterMeditativeState();

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior")
    void ChannelArchetypalEnergy();

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior")
    void GuidePlayerConsciousness(EConsciousnessState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior")
    void ManifestSpiritualVision();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnStateChanged(ESpiritualNPCState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnPlayerConsciousnessDetected(EConsciousnessState PlayerState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnSpiritualResonance(float ResonanceLevel);

private:
    float StateTransitionTimer;
    float BehaviorTimer;
    bool bPlayerInRange;
    
    void CheckPlayerProximity();
    void ProcessConsciousnessResonance();
    void UpdateBehaviorPattern(float DeltaTime);
    ESpiritualNPCState CalculateOptimalState(EConsciousnessState PlayerState);
};
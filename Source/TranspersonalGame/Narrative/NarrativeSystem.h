#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Sound/SoundWave.h"
#include "NarrativeSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueTriggered, FName, DialogueID, AActor*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeEvent, FGameplayTag, EventTag);

/**
 * Dialogue Voice Types for different character archetypes
 */
UENUM(BlueprintType)
enum class EDialogueVoiceType : uint8
{
    Protagonist     UMETA(DisplayName = "Dr. Protagonist"),
    DinosaurAlpha   UMETA(DisplayName = "Dinosaur Alpha"),
    DinosaurBeta    UMETA(DisplayName = "Dinosaur Beta"),
    Environment     UMETA(DisplayName = "Environment"),
    Narrator        UMETA(DisplayName = "Narrator")
};

/**
 * Emotional states for contextual dialogue
 */
UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Anxious     UMETA(DisplayName = "Anxious"),
    Terrified   UMETA(DisplayName = "Terrified"),
    Curious     UMETA(DisplayName = "Curious"),
    Determined  UMETA(DisplayName = "Determined"),
    Exhausted   UMETA(DisplayName = "Exhausted"),
    Hopeful     UMETA(DisplayName = "Hopeful")
};

/**
 * Data structure for dialogue context
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueContext : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FName DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueVoiceType SpeakerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEmotionalState EmotionalContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText SubtitleOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundWave> AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsInternalMonologue = false;

    FDialogueContext()
    {
        DialogueID = NAME_None;
        SpeakerType = EDialogueVoiceType::Protagonist;
        EmotionalContext = EEmotionalState::Calm;
        Priority = 1.0f;
        bIsInternalMonologue = false;
    }
};

/**
 * Narrative beats for story progression
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeBeat : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FName BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTagContainer CompletionTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FName> DialogueSequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 StoryWeight = 1;

    FNarrativeBeat()
    {
        BeatID = NAME_None;
        StoryWeight = 1;
    }
};

/**
 * Main Narrative System - manages story progression, dialogue, and character development
 */
UCLASS()
class TRANSPERSONALGAME_API UNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerDialogue(FName DialogueID, AActor* Speaker, EEmotionalState EmotionalContext = EEmotionalState::Calm);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTriggerDialogue(FName DialogueID, const FGameplayTagContainer& CurrentTags) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayInternalMonologue(FName DialogueID, EEmotionalState EmotionalContext);

    // Story Progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceNarrativeBeat(FName BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsNarrativeBeatActive(FName BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CheckNarrativeProgression(const FGameplayTagContainer& CurrentGameState);

    // Character Development
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateProtagonistState(EEmotionalState NewState, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    // Discovery System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDiscovery(FGameplayTag DiscoveryTag, const FString& DiscoveryDescription);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetDiscoveryLog() const { return DiscoveryLog; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueTriggered OnDialogueTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnNarrativeEvent OnNarrativeEvent;

protected:
    // Data Tables
    UPROPERTY(EditDefaultsOnly, Category = "Narrative")
    TSoftObjectPtr<UDataTable> DialogueDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "Narrative")
    TSoftObjectPtr<UDataTable> NarrativeBeatsDataTable;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FName> ActiveNarrativeBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FName> CompletedNarrativeBeats;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FString> DiscoveryLog;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FGameplayTagContainer CurrentStoryTags;

private:
    void LoadNarrativeData();
    FDialogueContext* FindDialogueContext(FName DialogueID, EEmotionalState EmotionalContext) const;
    void ProcessNarrativeBeatCompletion(const FNarrativeBeat& Beat);
};

/**
 * Component for actors that can participate in dialogue
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDialogueComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDialogueComponent();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void InitiateDialogue(FName DialogueID, AActor* Target = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetEmotionalState(EEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddDialogueTag(FGameplayTag Tag);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void RemoveDialogueTag(FGameplayTag Tag);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EDialogueVoiceType VoiceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer DialogueTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FName> AvailableDialogues;
};
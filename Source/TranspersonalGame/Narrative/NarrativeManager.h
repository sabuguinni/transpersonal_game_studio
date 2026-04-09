#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Sound/DialogueWave.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryEventTriggered, FGameplayTag, EventTag, const FString&, EventData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDialogueStarted, class UDialogueWave*, DialogueWave, class AActor*, Speaker, class AActor*, Listener);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FGameplayTag EventTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FGameplayTag> PrerequisiteEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FGameplayTag> UnlockEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsRepeatable = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 Priority = 0;

    FStoryEvent()
    {
        Priority = 0;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueWave* DialogueWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    class UDialogueVoice* SpeakerVoice;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FGameplayTag> RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bAutoAdvance = true;

    FDialogueEntry()
    {
        DisplayDuration = 3.0f;
        bAutoAdvance = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarrativeState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FGameplayTag> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FGameplayTag> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FGameplayTag CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 StoryProgress = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FString> CharacterRelationships;

    FNarrativeState()
    {
        StoryProgress = 0;
    }
};

/**
 * Narrative Manager - Core system for story progression and dialogue
 * Manages the overarching story of the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story Event Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(FGameplayTag EventTag, const FString& EventData = "");

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsEventCompleted(FGameplayTag EventTag) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool CanTriggerEvent(FGameplayTag EventTag) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterStoryEvent(const FStoryEvent& NewEvent);

    // Dialogue System
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(class UDialogueWave* DialogueWave, class AActor* Speaker, class AActor* Listener = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bIsDialogueActive; }

    // Chapter and Progress Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetCurrentChapter(FGameplayTag ChapterTag);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FGameplayTag GetCurrentChapter() const { return NarrativeState.CurrentChapter; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryProgress(int32 ProgressAmount = 1);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetStoryProgress() const { return NarrativeState.StoryProgress; }

    // Character Relationships
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetCharacterRelationship(const FString& CharacterName, const FString& RelationshipStatus);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetCharacterRelationship(const FString& CharacterName) const;

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarrativeState GetNarrativeState() const { return NarrativeState; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetNarrativeState(const FNarrativeState& NewState) { NarrativeState = NewState; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnStoryEventTriggered OnStoryEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueEnded OnDialogueEnded;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarrativeState NarrativeState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FGameplayTag, FStoryEvent> RegisteredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    class UDataTable* StoryEventsDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    class UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsDialogueActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    class UAudioComponent* CurrentDialogueAudio;

private:
    void LoadStoryEventsFromDataTable();
    void ProcessEventPrerequisites(const FStoryEvent& Event);
    void ProcessEventUnlocks(const FStoryEvent& Event);
    
    UFUNCTION()
    void OnDialogueAudioFinished();
};
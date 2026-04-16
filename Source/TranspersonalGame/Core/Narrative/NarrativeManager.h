#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

class UDialogueComponent;
class ANarrativeActor;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TSoftObjectPtr<USoundCue> VoiceLine;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> RequiredFlags;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        Duration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    bool bIsMainQuest;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("");
        bIsMainQuest = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> CompletionFlags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 Priority;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        NarrativeText = FText::GetEmpty();
        Priority = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceDialogue();

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const FNarr_QuestDialogue& QuestDialogue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayQuestDialogue(const FString& QuestID);

    // Character voices
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayCharacterVoiceLine(const FString& CharacterName, const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterCharacterVoice(const FString& CharacterName, USoundCue* VoiceCue);

protected:
    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentListener;

    // Story state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, bool> StoryFlags;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FString> CompletedStoryBeats;

    // Data storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TMap<FString, FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Data")
    TMap<FString, TSoftObjectPtr<USoundCue>> CharacterVoices;

    // Internal methods
    void ProcessDialogueLine(const FNarr_DialogueLine& DialogueLine);
    bool CheckConditions(const TArray<FString>& Conditions) const;
    void ApplyCompletionFlags(const TArray<FString>& Flags);

    // Audio playback
    UPROPERTY()
    class UAudioComponent* VoiceAudioComponent;

private:
    void InitializeDefaultStoryBeats();
    void InitializeDefaultDialogues();
};
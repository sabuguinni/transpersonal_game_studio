#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

class UDialogueComponent;
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> VoiceClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::GetEmpty();
        Duration = 3.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueLine> IntroLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueLine> ProgressLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueLine> CompletionLines;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("DefaultQuest");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryCheckpoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString CheckpointID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TSoftObjectPtr<USoundBase> NarrationClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bTriggerAutomatically;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float TriggerRadius;

    FNarr_StoryCheckpoint()
    {
        CheckpointID = TEXT("Checkpoint_001");
        NarrativeText = FText::GetEmpty();
        bTriggerAutomatically = true;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID, ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarration(const FString& CheckpointID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterQuestDialogue(const FString& QuestID, const FNarr_QuestDialogue& DialogueData);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryCheckpointReached(const FString& CheckpointID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void MarkCheckpointReached(const FString& CheckpointID);

    // Dialogue system
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void PlayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndCurrentDialogue();

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void CheckLocationTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetStoryProgress() const;

protected:
    // Narrative data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryCheckpoint> StoryCheckpoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TSet<FString> ReachedCheckpoints;

    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    int32 CurrentLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
    TArray<FNarr_DialogueLine> CurrentDialogueLines;

    // Audio management
    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    class UAudioComponent* NarrationAudioComponent;

    // Timing
    UPROPERTY(BlueprintReadOnly, Category = "Timing")
    float DialogueTimer;

private:
    void InitializeStoryCheckpoints();
    void LoadQuestDialogues();
    void UpdateDialogueTimer(float DeltaTime);
    void ProcessNextDialogueLine();
    
    // Story checkpoint helpers
    bool IsPlayerNearCheckpoint(const FNarr_StoryCheckpoint& Checkpoint, const FVector& PlayerLocation) const;
    void TriggerCheckpointNarration(const FNarr_StoryCheckpoint& Checkpoint);
};
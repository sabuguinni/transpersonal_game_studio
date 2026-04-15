#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("");
        DialogueText = FText::GetEmpty();
        AudioAssetPath = TEXT("");
        Duration = 3.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueSequence
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SequenceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresQuestProgress;

    FNarr_DialogueSequence()
    {
        SequenceID = TEXT("");
        bIsRepeatable = true;
        bRequiresQuestProgress = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredBeats;

    FNarr_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = FText::GetEmpty();
        BeatDescription = FText::GetEmpty();
        bIsCompleted = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueCompleted, const FString&, DialogueID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryBeatCompleted, const FString&, BeatID);

/**
 * Core narrative system that manages story progression, dialogue sequences, and character interactions
 * Handles the main story arc of survival in a prehistoric world with realistic dialogue
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueSequence GetDialogueSequence(const FString& DialogueID) const;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryBeatCompleted(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_StoryBeat> GetAvailableStoryBeats() const;

    // Character relationship tracking
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ModifyCharacterRelationship(const FString& CharacterID, int32 RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCharacterRelationship(const FString& CharacterID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnDialogueCompleted OnDialogueCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Narrative")
    FOnStoryBeatCompleted OnStoryBeatCompleted;

protected:
    // Current dialogue state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentDialogueLineIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    // Story data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_DialogueSequence> DialogueDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, FNarr_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<FString, int32> CharacterRelationships;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float DefaultDialogueLineDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bAutoAdvanceDialogue;

private:
    void LoadDialogueData();
    void LoadStoryData();
    void InitializeCharacterRelationships();
};
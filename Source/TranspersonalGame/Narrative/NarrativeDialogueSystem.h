#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeDialogueSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FText DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = FText::FromString(TEXT("..."));
        Duration = 3.0f;
        AudioPath = TEXT("");
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueTree : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> NextDialogueIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    FNarr_DialogueTree()
    {
        DialogueID = TEXT("default_dialogue");
        bIsQuestDialogue = false;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeDialogueSystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool StartDialogue(const FString& DialogueID, AActor* Speaker, AActor* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetCurrentDialogueEntry() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterDialogueTable(UDataTable* DialogueTable);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectChoice(int32 ChoiceIndex);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentDialogueID;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    int32 CurrentEntryIndex;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueTree CurrentDialogueTree;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TWeakObjectPtr<AActor> CurrentListener;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    UDataTable* DialogueDataTable;

private:
    bool LoadDialogueTree(const FString& DialogueID);
    void PlayDialogueAudio(const FString& AudioPath);
    void TriggerDialogueEvent(const FString& EventName);
};
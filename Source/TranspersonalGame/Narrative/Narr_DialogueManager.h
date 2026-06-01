#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_DialogueManager.generated.h"

class UNarr_DialogueNode;
class UNarr_DialogueChoice;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDialogueStarted, FString, SpeakerName, FString, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChoicePresented, TArray<FString>, ChoiceTexts, TArray<int32>, ChoiceIDs);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FString> PlayerChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<int32> NextDialogueIDs;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        DisplayDuration = 3.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_DialogueEntry> DialogueEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FNarr_QuestDialogue()
    {
        QuestID = TEXT("");
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_DialogueManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_DialogueManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void StartDialogue(const FString& QuestID, const FString& SpeakerName);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void AddQuestDialogue(const FString& QuestID, const TArray<FNarr_DialogueEntry>& Entries);

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentSpeaker() const;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    FString GetCurrentDialogueText() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueStarted OnDialogueStarted;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDialogueEnded OnDialogueEnded;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnChoicePresented OnChoicePresented;

    // Tutorial dialogue
    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void StartTutorialDialogue(ETutorialStep TutorialStep);

    UFUNCTION(BlueprintCallable, Category = "Tutorial")
    void RegisterTutorialDialogue();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentQuestID;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 CurrentDialogueIndex;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentSpeaker;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentDialogueText;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<FString, FNarr_QuestDialogue> QuestDialogues;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<ETutorialStep, FNarr_DialogueEntry> TutorialDialogues;

private:
    void ProcessCurrentDialogue();
    void ShowChoices(const TArray<FString>& Choices, const TArray<int32>& NextIDs);
    FTimerHandle DialogueTimerHandle;
};

#include "Narr_DialogueManager.generated.h"
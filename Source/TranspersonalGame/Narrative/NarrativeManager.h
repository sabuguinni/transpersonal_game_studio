#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsPlayerChoice;

    FNarr_DialogueLine()
    {
        SpeakerName = TEXT("Unknown");
        DialogueText = TEXT("");
        DisplayDuration = 3.0f;
        bIsPlayerChoice = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FNarr_QuestObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        RequiredCount = 1;
        CurrentCount = 0;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void DisplayDialogueLine(const FNarr_DialogueLine& DialogueLine);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteQuestObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsQuestObjectiveComplete(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerNarration(const FString& NarrationText, float Duration = 5.0f);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_DialogueLine> CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_QuestObjective> ActiveObjectives;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FString CurrentNarrationText;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bIsDialogueActive;

private:
    void LoadDialogueData();
    void ProcessDialogueChoice(int32 ChoiceIndex);
};
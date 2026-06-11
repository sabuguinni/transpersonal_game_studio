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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    float AudioDuration;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
    TArray<FString> ResponseOptions;

    FNarr_DialogueEntry()
    {
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        AudioDuration = 0.0f;
        AudioURL = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
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
class TRANSPERSONALGAME_API UNarrativeDialogueSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID, AActor* Speaker);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueOption(int32 OptionIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsDialogueActive() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateQuestObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsQuestCompleted(const FString& QuestID) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, FNarr_DialogueEntry> DialogueDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, TArray<FNarr_QuestObjective>> QuestDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bDialogueActive;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    AActor* CurrentSpeaker;

private:
    void LoadDialogueData();
    void LoadQuestData();
    void BroadcastDialogueUpdate();
};
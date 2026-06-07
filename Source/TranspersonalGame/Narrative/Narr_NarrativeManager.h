#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "Narr_NarrativeManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_QuestState : uint8
{
    Inactive,
    Active,
    Completed,
    Failed
};

UENUM(BlueprintType)
enum class ENarr_DialogueType : uint8
{
    Info,
    Warning,
    Quest,
    Trade,
    Combat
};

USTRUCT(BlueprintType)
struct FNarr_DialogueChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString NextDialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bRequiresItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString RequiredItemID;

    FNarr_DialogueChoice()
    {
        ChoiceText = TEXT("");
        NextDialogueID = TEXT("");
        bRequiresItem = false;
        RequiredItemID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNarr_DialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    ENarr_DialogueType DialogueType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNarr_DialogueChoice> Choices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DisplayDuration;

    FNarr_DialogueEntry()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        DialogueType = ENarr_DialogueType::Info;
        AudioAssetPath = TEXT("");
        DisplayDuration = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct FNarr_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ENarr_QuestState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 TargetProgress;

    FNarr_QuestObjective()
    {
        ObjectiveID = TEXT("");
        Description = TEXT("");
        State = ENarr_QuestState::Inactive;
        CurrentProgress = 0;
        TargetProgress = 1;
    }
};

USTRUCT(BlueprintType)
struct FNarr_Quest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    ENarr_QuestState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FNarr_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    FNarr_Quest()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        Description = TEXT("");
        State = ENarr_QuestState::Inactive;
        RewardItemID = TEXT("");
        ExperienceReward = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_NarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_NarrativeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateQuestObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_Quest> GetActiveQuests() const;

    // Dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SelectDialogueChoice(int32 ChoiceIndex);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_DialogueEntry GetCurrentDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsInDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void EndDialogue();

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryFlag(const FString& FlagName, bool bValue);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool GetStoryFlag(const FString& FlagName) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    UDataTable* QuestDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Narrative")
    UDataTable* DialogueDataTable;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_Quest> ActiveQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_Quest> CompletedQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_DialogueEntry CurrentDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    bool bInDialogue;

    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TMap<FString, bool> StoryFlags;

private:
    void LoadQuestData();
    void LoadDialogueData();
    void InitializeDefaultQuests();
    void InitializeDefaultDialogues();

    FNarr_Quest* FindQuest(const FString& QuestID);
    const FNarr_Quest* FindQuest(const FString& QuestID) const;
    FNarr_DialogueEntry* FindDialogue(const FString& DialogueID);
};
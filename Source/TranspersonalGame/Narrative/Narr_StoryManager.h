#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

// Story chapter data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> QuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 ChapterOrder;

    FNarr_StoryChapter()
    {
        ChapterName = TEXT("");
        ChapterDescription = TEXT("");
        bIsCompleted = false;
        ChapterOrder = 0;
    }
};

// Story progression state
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 TribeMembersRescued;

    FNarr_StoryState()
    {
        CurrentChapter = 0;
        SurvivalDays = 0.0f;
        DinosaurKills = 0;
        TribeMembersRescued = 0;
    }
};

// Narrative event for story progression
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockQuests;

    FNarr_StoryEvent()
    {
        EventName = TEXT("");
        EventDescription = TEXT("");
        TriggerCondition = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StoryManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Story progression data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_StoryChapter> StoryChapters;

    UPROPERTY(BlueprintReadOnly, Category = "Story", meta = (AllowPrivateAccess = "true"))
    FNarr_StoryState CurrentStoryState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_StoryEvent> StoryEvents;

    // Survival tracking
    UPROPERTY(BlueprintReadOnly, Category = "Story", meta = (AllowPrivateAccess = "true"))
    float DayStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Story", meta = (AllowPrivateAccess = "true"))
    float CurrentDayTime;

public:
    // Story progression functions
    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceChapter();

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsChapterCompleted(int32 ChapterIndex);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryChapter GetCurrentChapter();

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetCurrentChapterIndex() const { return CurrentStoryState.CurrentChapter; }

    // Quest integration
    UFUNCTION(BlueprintCallable, Category = "Story")
    void OnQuestCompleted(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsQuestCompleted(const FString& QuestID);

    // Story events
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CheckEventCondition(const FString& Condition);

    // Survival statistics
    UFUNCTION(BlueprintCallable, Category = "Story")
    void OnDinosaurKilled();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void OnTribeMemberRescued();

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetSurvivalDays() const { return CurrentStoryState.SurvivalDays; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetDinosaurKills() const { return CurrentStoryState.DinosaurKills; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetTribeMembersRescued() const { return CurrentStoryState.TribeMembersRescued; }

    // Dialogue unlocking
    UFUNCTION(BlueprintCallable, Category = "Story")
    void UnlockDialogue(const FString& DialogueID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsDialogueUnlocked(const FString& DialogueID);

    // Initialize story chapters
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Story")
    void InitializeStoryChapters();

private:
    void UpdateDayTime(float DeltaTime);
    void CreateSurvivalChapters();
    void CreateStoryEvents();
};
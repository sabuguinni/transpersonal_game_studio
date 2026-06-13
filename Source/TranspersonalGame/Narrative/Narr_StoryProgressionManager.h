#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 ChapterOrder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    FNarr_StoryChapter()
    {
        ChapterID = "";
        ChapterTitle = "";
        ChapterDescription = "";
        bIsCompleted = false;
        ChapterOrder = 0;
        NarrativeText = "";
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FString ChoiceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    FString ConsequenceDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    TArray<FString> AffectedNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    float MoralityImpact;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Choice")
    bool bWasChosen;

    FNarr_PlayerChoice()
    {
        ChoiceID = "";
        ChoiceText = "";
        ConsequenceDescription = "";
        MoralityImpact = 0.0f;
        bWasChosen = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FNarr_StoryChapter> StoryChapters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    FString CurrentChapterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FNarr_PlayerChoice> PlayerChoiceHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    float PlayerMorality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 TotalQuestsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<FString> UnlockedRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TMap<FString, int32> NPCRelationshipLevels;

public:
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void InitializeStoryChapters();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool AdvanceToNextChapter();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    FNarr_StoryChapter GetCurrentChapter();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsChapterUnlocked(const FString& ChapterID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void CompleteChapter(const FString& ChapterID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void RecordPlayerChoice(const FNarr_PlayerChoice& Choice);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    float GetPlayerMorality();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void ModifyPlayerMorality(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetUnlockedQuests();

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void UnlockRegion(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsRegionUnlocked(const FString& RegionName);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void UpdateNPCRelationship(const FString& NPCID, int32 Delta);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    int32 GetNPCRelationshipLevel(const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetStoryMilestones();

protected:
    UFUNCTION()
    void SetupDefaultChapters();

    UFUNCTION()
    bool CheckChapterRequirements(const FNarr_StoryChapter& Chapter);

    UFUNCTION()
    void TriggerChapterEvents(const FString& ChapterID);
};

#include "Narr_StoryProgressionManager.generated.h"
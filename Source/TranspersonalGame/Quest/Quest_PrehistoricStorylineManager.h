#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Quest_PrehistoricStorylineManager.generated.h"

class ATranspersonalCharacter;
class UQuest_SurvivalMissionManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString BeatTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FString BeatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> RequiredCompletedBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FString> UnlockedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    int32 RequiredSurvivalLevel;

    FQuest_StoryBeat()
    {
        BeatID = TEXT("");
        BeatTitle = TEXT("");
        BeatDescription = TEXT("");
        bIsCompleted = false;
        RequiredSurvivalLevel = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PrehistoricChapter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    FString ChapterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    FString ChapterTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    FString ChapterDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    TArray<FQuest_StoryBeat> StoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    bool bIsUnlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Chapter")
    bool bIsCompleted;

    FQuest_PrehistoricChapter()
    {
        ChapterID = TEXT("");
        ChapterTitle = TEXT("");
        ChapterDescription = TEXT("");
        bIsUnlocked = false;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_PrehistoricStorylineManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_PrehistoricStorylineManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    void InitializeStoryline();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    void UpdateStorylineProgress();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    bool CompleteStoryBeat(const FString& BeatID);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    TArray<FQuest_StoryBeat> GetAvailableStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    FQuest_PrehistoricChapter GetCurrentChapter() const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    bool IsStoryBeatUnlocked(const FString& BeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    void UnlockNextChapter();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    float GetOverallStoryProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Storyline")
    TArray<FString> GetUnlockedMissionsForBeat(const FString& BeatID) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Data")
    TArray<FQuest_PrehistoricChapter> StorylineChapters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Data")
    int32 CurrentChapterIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Data")
    TArray<FString> CompletedStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Data")
    TArray<FString> UnlockedMissions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline Data")
    UQuest_SurvivalMissionManager* SurvivalMissionManager;

private:
    void CreatePrehistoricStoryline();
    void CreateChapter1_FirstSteps();
    void CreateChapter2_TribalFormation();
    void CreateChapter3_TerritoryExpansion();
    void CreateChapter4_DinosaurMastery();
    void CreateChapter5_CivilizationDawn();

    bool CheckStoryBeatRequirements(const FQuest_StoryBeat& StoryBeat) const;
    void ProcessStoryBeatCompletion(const FQuest_StoryBeat& CompletedBeat);
    ATranspersonalCharacter* GetPlayerCharacter() const;
};
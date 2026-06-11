#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 StoryChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedEvents;

    FNarr_StoryEvent()
    {
        EventID = TEXT("DefaultEvent");
        EventTitle = FText::FromString(TEXT("Unknown Event"));
        EventDescription = FText::FromString(TEXT("No description available"));
        bIsCompleted = false;
        StoryChapter = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    int32 CurrentChapter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    TArray<FString> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    float SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    bool bFirstHuntCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progress")
    bool bSafeHavenFound;

    FNarr_StoryProgress()
    {
        CurrentChapter = 1;
        SurvivalDays = 0.0f;
        DinosaurEncounters = 0;
        bFirstHuntCompleted = false;
        bSafeHavenFound = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    FNarr_StoryProgress CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    bool bStorySystemActive;

public:
    UFUNCTION(BlueprintCallable, Category = "Story")
    bool TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CompleteStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsEventActive(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FString> GetActiveEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FString> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryProgress GetStoryProgress() const { return CurrentProgress; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceChapter();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdateSurvivalStats(float DaysElapsed, int32 NewEncounters);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetMilestone(const FString& MilestoneName, bool bCompleted);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool GetMilestone(const FString& MilestoneName) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AddStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FText GetChapterTitle() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FText GetChapterDescription() const;

private:
    FNarr_StoryEvent* FindStoryEvent(const FString& EventID);
    bool CheckEventPrerequisites(const FNarr_StoryEvent& Event) const;
    void UnlockFollowupEvents(const FNarr_StoryEvent& CompletedEvent);
    void InitializeDefaultStoryEvents();
};
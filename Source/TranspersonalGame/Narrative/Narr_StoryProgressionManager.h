#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString MilestoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 RequiredSurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<ENarr_SurvivalEvent> RequiredEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PrerequisiteMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 StoryWeight;

    FNarr_StoryMilestone()
    {
        MilestoneID = TEXT("");
        MilestoneName = TEXT("");
        Description = TEXT("");
        bIsCompleted = false;
        bIsActive = false;
        RequiredSurvivalDays = 0;
        StoryWeight = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryArc
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ArcID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString ArcName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryTheme Theme;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryMilestone> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsMainArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CompletionPercentage;

    FNarr_StoryArc()
    {
        ArcID = TEXT("");
        ArcName = TEXT("");
        Theme = ENarr_StoryTheme::Survival;
        bIsMainArc = false;
        CompletionPercentage = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TArray<FNarr_StoryArc> StoryArcs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    FString CurrentActiveArc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    int32 PlayerSurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TArray<ENarr_SurvivalEvent> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    float OverallStoryProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    bool bStorySystemEnabled;

public:
    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterSurvivalEvent(ENarr_SurvivalEvent EventType);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void IncrementSurvivalDays();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CheckMilestoneCompletion();

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsMilestoneCompleted(const FString& MilestoneID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void ActivateMilestone(const FString& MilestoneID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteMilestone(const FString& MilestoneID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryArc* GetCurrentStoryArc();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void SetActiveStoryArc(const FString& ArcID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryMilestone> GetActiveMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    float GetStoryProgress() const { return OverallStoryProgress; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    int32 GetSurvivalDays() const { return PlayerSurvivalDays; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    void ResetStoryProgress();

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AddCustomStoryArc(const FNarr_StoryArc& NewArc);

    UFUNCTION(BlueprintPure, Category = "Story")
    bool HasCompletedEvent(ENarr_SurvivalEvent EventType) const;

private:
    void InitializeDefaultStoryArcs();
    void UpdateStoryProgress();
    FNarr_StoryMilestone* FindMilestone(const FString& MilestoneID);
    bool AreMilestonePrerequisitesMet(const FNarr_StoryMilestone& Milestone) const;
    void OnMilestoneCompleted(const FNarr_StoryMilestone& Milestone);
    void BroadcastStoryEvent(const FString& EventDescription);
};
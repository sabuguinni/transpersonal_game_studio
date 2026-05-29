#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "StoryProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FString MilestoneID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    ENarr_StoryAct RequiredAct;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    int32 RequiredSurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> RequiredQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    TArray<FString> UnlockedDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story")
    bool bIsCompleted;

    FNarr_StoryMilestone()
        : RequiredAct(ENarr_StoryAct::Survival)
        , RequiredSurvivalDays(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    ENarr_StoryAct CurrentAct;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    TArray<FString> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    TArray<ENarr_BiomeType> DiscoveredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Progress")
    float MaxThreatSurvived;

    FNarr_PlayerProgress()
        : CurrentAct(ENarr_StoryAct::Survival)
        , SurvivalDays(0)
        , DinosaurEncounters(0)
        , MaxThreatSurvived(0.0f)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryMilestoneReached, const FNarr_StoryMilestone&, Milestone);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryActChanged, ENarr_StoryAct, NewAct);

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UStoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStoryProgressionManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdatePlayerProgress(const FNarr_PlayerProgress& NewProgress);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void DiscoverBiome(ENarr_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RecordDinosaurEncounter(const FString& DinosaurType, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceSurvivalDay();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryAct GetCurrentAct() const { return PlayerProgress.CurrentAct; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_PlayerProgress GetPlayerProgress() const { return PlayerProgress; }

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryMilestone> GetAvailableMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsMilestoneCompleted(const FString& MilestoneID) const;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStoryMilestoneReached OnMilestoneReached;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStoryActChanged OnActChanged;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FNarr_PlayerProgress PlayerProgress;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TArray<FNarr_StoryMilestone> StoryMilestones;

private:
    void InitializeStoryMilestones();
    void CheckMilestoneCompletion();
    void UpdateStoryAct();
    bool AreMilestoneRequirementsMet(const FNarr_StoryMilestone& Milestone) const;
};
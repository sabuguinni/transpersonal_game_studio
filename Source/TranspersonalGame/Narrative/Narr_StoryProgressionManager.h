#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString MilestoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> RequiredConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedContent;

    FNarr_StoryMilestone()
    {
        MilestoneID = TEXT("DefaultMilestone");
        MilestoneName = FText::FromString(TEXT("Unknown Milestone"));
        Description = FText::FromString(TEXT(""));
        bCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DaysAlive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DinosaursSeen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DinosaursSurvived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 ToolsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 SheltersBuilt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> BiomesExplored;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedMilestones;

    FNarr_PlayerProgress()
    {
        DaysAlive = 0;
        DinosaursSeen = 0;
        DinosaursSurvived = 0;
        ToolsCrafted = 0;
        SheltersBuilt = 0;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void UpdatePlayerProgress(const FString& ProgressType, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CheckMilestoneConditions(const FString& MilestoneID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteMilestone(const FString& MilestoneID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_PlayerProgress GetPlayerProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    TArray<FNarr_StoryMilestone> GetAvailableMilestones() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID, const FVector& Location);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Story")
    FNarr_PlayerProgress CurrentProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Story")
    TMap<FString, FNarr_StoryMilestone> StoryMilestones;

    UFUNCTION()
    void InitializeStoryMilestones();

    UFUNCTION()
    void CheckAllMilestones();

    UFUNCTION()
    void UnlockContent(const TArray<FString>& ContentIDs);
};
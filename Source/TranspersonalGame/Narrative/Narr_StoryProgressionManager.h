#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryProgressionManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening       UMETA(DisplayName = "Awakening"),
    FirstHunt       UMETA(DisplayName = "First Hunt"),
    TerritoryWars   UMETA(DisplayName = "Territory Wars"),
    AlphaChallenge  UMETA(DisplayName = "Alpha Challenge"),
    Mastery         UMETA(DisplayName = "Mastery")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString MilestoneID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockDialogues;

    FNarr_StoryMilestone()
    {
        MilestoneID = TEXT("");
        Description = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Awakening;
        bIsCompleted = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UNarr_StoryProgressionManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryProgressionManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintCallable, Category = "Story")
    ENarr_StoryPhase GetCurrentStoryPhase() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void CompleteMilestone(const FString& MilestoneID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool IsMilestoneCompleted(const FString& MilestoneID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventName);

    UFUNCTION(BlueprintCallable, Category = "Story")
    FString GetCurrentPhaseNarration() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FNarr_StoryMilestone> StoryMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TMap<FString, bool> StoryFlags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 DinosaurKillCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 Dayssurvived;

private:
    void InitializeStoryMilestones();
    void CheckPhaseProgression();
    void UnlockPhaseContent(ENarr_StoryPhase Phase);
};
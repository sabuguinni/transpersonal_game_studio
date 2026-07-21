#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Narr_SurvivalStoryManager.generated.h"

UENUM(BlueprintType)
enum class ENarr_SurvivalStoryPhase : uint8
{
    Awakening,      // Player wakes up alone, learns basic survival
    FirstHunt,      // Player kills first small prey
    FirstCraft,     // Player crafts first tool/weapon
    FirstShelter,   // Player builds first shelter
    DinosaurThreat, // First major dinosaur encounter
    TribeContact,   // First contact with other survivors
    Leadership,     // Player becomes tribal leader
    Mastery         // Player masters the prehistoric world
};

UENUM(BlueprintType)
enum class ENarr_SurvivalMilestone : uint8
{
    FirstFire,
    FirstKill,
    FirstTool,
    FirstShelter,
    FirstTaming,
    FirstTrade,
    FirstBattle,
    FirstVictory
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgressionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_SurvivalStoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<ENarr_SurvivalMilestone> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    int32 SurvivalDays;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float CharacterGrowthScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedStoryBeats;

    FNarr_StoryProgressionData()
    {
        CurrentPhase = ENarr_SurvivalStoryPhase::Awakening;
        SurvivalDays = 0;
        CharacterGrowthScore = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_SurvivalStoryManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_SurvivalStoryManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    FNarr_StoryProgressionData StoryData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TMap<ENarr_SurvivalStoryPhase, FString> PhaseNarratives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TMap<ENarr_SurvivalMilestone, FString> MilestoneNarratives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    float StoryUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    bool bAutoProgressStory;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void AdvanceStoryPhase(ENarr_SurvivalStoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void CompleteMilestone(ENarr_SurvivalMilestone Milestone);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void UpdateSurvivalProgress(int32 Days, float GrowthScore);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    FString GetCurrentPhaseNarrative() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    FString GetMilestoneNarrative(ENarr_SurvivalMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool HasCompletedMilestone(ENarr_SurvivalMilestone Milestone) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    float GetStoryCompletionPercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    TArray<FString> GetAvailableStoryBeats() const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void TriggerStoryEvent(const FString& EventName);

private:
    void InitializeStoryNarratives();
    void CheckForPhaseTransitions();
    void BroadcastStoryUpdate();

    FTimerHandle StoryUpdateTimer;
    float LastUpdateTime;
};
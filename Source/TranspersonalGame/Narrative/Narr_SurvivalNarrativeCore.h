#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Narr_SurvivalNarrativeCore.generated.h"

UENUM(BlueprintType)
enum class ENarr_SurvivalPhase : uint8
{
    Awakening       UMETA(DisplayName = "First Awakening"),
    Discovery       UMETA(DisplayName = "World Discovery"),
    Adaptation      UMETA(DisplayName = "Survival Adaptation"),
    Mastery         UMETA(DisplayName = "Environmental Mastery"),
    Leadership      UMETA(DisplayName = "Tribal Leadership"),
    Legacy          UMETA(DisplayName = "Ancestral Legacy")
};

UENUM(BlueprintType)
enum class ENarr_ThreatLevel : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Dangerous       UMETA(DisplayName = "Dangerous"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Lethal          UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalMilestone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString MilestoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    ENarr_SurvivalPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Milestone")
    FString UnlockNarrative;

    FNarr_SurvivalMilestone()
    {
        MilestoneName = TEXT("");
        Description = TEXT("");
        RequiredPhase = ENarr_SurvivalPhase::Awakening;
        ExperienceReward = 0;
        bIsCompleted = false;
        UnlockNarrative = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EnvironmentalStory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString LocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString StoryFragment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> DiscoveryClues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsDiscovered;

    FNarr_EnvironmentalStory()
    {
        LocationName = TEXT("");
        StoryFragment = TEXT("");
        ThreatLevel = ENarr_ThreatLevel::Peaceful;
        bIsDiscovered = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_SurvivalNarrativeCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_SurvivalNarrativeCore();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    ENarr_SurvivalPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    int32 SurvivalExperience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    TArray<FNarr_SurvivalMilestone> CompletedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    TArray<FNarr_EnvironmentalStory> DiscoveredStories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    float NarrativeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Core")
    FString CurrentNarrativeContext;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void AdvanceNarrativePhase();

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void CompleteMilestone(const FString& MilestoneName);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void DiscoverEnvironmentalStory(const FString& LocationName);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    FString GetContextualNarrative(ENarr_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void UpdateNarrativeIntensity(float DeltaIntensity);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    bool CanProgressToPhase(ENarr_SurvivalPhase TargetPhase);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    TArray<FString> GetAvailableMilestones();

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    void InitializeSurvivalNarrative();

private:
    void LoadMilestoneData();
    void LoadEnvironmentalStories();
    FString GeneratePhaseNarrative(ENarr_SurvivalPhase Phase);
    void TriggerNarrativeEvent(const FString& EventName);
};
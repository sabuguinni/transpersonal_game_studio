#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Narr_StoryManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryEventTriggered, FString, EventID, FString, EventDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryMilestoneReached, EStoryMilestone, Milestone, FString, Description);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    EStoryMilestone RequiredMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PrerequisiteEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsRepeatable;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventName = TEXT("");
        EventDescription = TEXT("");
        RequiredMilestone = EStoryMilestone::None;
        bIsTriggered = false;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    EStoryMilestone CurrentMilestone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    TArray<FString> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 DinosaurEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 ResourcesGathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    int32 ToolsCrafted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Progress")
    float SurvivalTime;

    FNarr_StoryProgress()
    {
        CurrentMilestone = EStoryMilestone::None;
        DinosaurEncounters = 0;
        ResourcesGathered = 0;
        ToolsCrafted = 0;
        SurvivalTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void AdvanceStoryMilestone(EStoryMilestone NewMilestone);

    UFUNCTION(BlueprintCallable, Category = "Story")
    bool CanTriggerEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    EStoryMilestone GetCurrentMilestone() const;

    UFUNCTION(BlueprintCallable, Category = "Story")
    FNarr_StoryProgress GetStoryProgress() const;

    // Event registration
    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterStoryEvent(const FNarr_StoryEvent& StoryEvent);

    UFUNCTION(BlueprintCallable, Category = "Story")
    void RegisterDefaultStoryEvents();

    // Progress tracking
    UFUNCTION(BlueprintCallable, Category = "Progress")
    void IncrementDinosaurEncounters();

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void IncrementResourcesGathered();

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void IncrementToolsCrafted();

    UFUNCTION(BlueprintCallable, Category = "Progress")
    void UpdateSurvivalTime(float DeltaTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStoryEventTriggered OnStoryEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnStoryMilestoneReached OnStoryMilestoneReached;

    // Contextual narration
    UFUNCTION(BlueprintCallable, Category = "Narration")
    FString GetContextualNarration(const FString& Context) const;

    UFUNCTION(BlueprintCallable, Category = "Narration")
    void TriggerEnvironmentalNarration(const FString& BiomeName, const FString& WeatherCondition);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Progress")
    FNarr_StoryProgress CurrentProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Data")
    TMap<FString, FString> ContextualNarrations;

private:
    void CheckMilestoneProgression();
    void RegisterContextualNarrations();
    bool ArePrerequisitesMet(const FNarr_StoryEvent& Event) const;
};

#include "Narr_StoryManager.generated.h"
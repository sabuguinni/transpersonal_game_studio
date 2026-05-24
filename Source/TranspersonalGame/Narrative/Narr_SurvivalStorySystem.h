#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "NarrativeTypes.h"
#include "Narr_SurvivalStorySystem.generated.h"

class UNarr_VoicelineDatabase;
class UNarr_StoryController;
class ANarr_SurvivalNarrator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryEventTriggered, FString, EventID, FString, EventDescription);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSurvivalMilestone, ENarr_SurvivalMilestone, Milestone, int32, DaysSurvived, FString, Description);

UENUM(BlueprintType)
enum class ENarr_SurvivalPhase : uint8
{
    FirstDays       UMETA(DisplayName = "First Days (1-7)"),
    Adaptation      UMETA(DisplayName = "Adaptation (8-30)"),
    Establishment   UMETA(DisplayName = "Establishment (31-60)"),
    Mastery         UMETA(DisplayName = "Mastery (61-100)"),
    Legend          UMETA(DisplayName = "Legend (100+)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    ENarr_SurvivalPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    TArray<FString> VoicelineIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    bool bIsRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Event")
    float CooldownHours;

    FNarr_StoryEvent()
    {
        EventID = "";
        EventTitle = "";
        EventDescription = "";
        RequiredPhase = ENarr_SurvivalPhase::FirstDays;
        bIsRepeatable = false;
        CooldownHours = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    int32 DaysSurvived;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    ENarr_SurvivalPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    TArray<FString> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    TMap<FString, float> EventCooldowns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    TArray<ENarr_SurvivalMilestone> AchievedMilestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Progress")
    float TotalPlayTime;

    FNarr_SurvivalProgress()
    {
        DaysSurvived = 0;
        CurrentPhase = ENarr_SurvivalPhase::FirstDays;
        TotalPlayTime = 0.0f;
    }
};

/**
 * Survival-focused story system that tracks player progress and triggers contextual narrative events
 * Manages the progression from helpless survivor to apex predator through realistic milestones
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_SurvivalStorySystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_SurvivalStorySystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void AdvanceDay();

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    bool CanTriggerEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void CheckSurvivalMilestones();

    // Progress tracking
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    ENarr_SurvivalPhase GetCurrentPhase() const { return SurvivalProgress.CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    int32 GetDaysSurvived() const { return SurvivalProgress.DaysSurvived; }

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    float GetPhaseProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    TArray<FString> GetAvailableEvents() const;

    // Event management
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void RegisterStoryEvent(const FNarr_StoryEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    FNarr_StoryEvent GetStoryEvent(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void SetEventCooldown(const FString& EventID, float CooldownHours);

    // Condition checking
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    bool EvaluateCondition(const FString& Condition) const;

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void UpdateGameState(const FString& StateKey, const FString& StateValue);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void SaveProgress();

    UFUNCTION(BlueprintCallable, Category = "Survival Story")
    void LoadProgress();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Survival Story")
    FOnStoryEventTriggered OnStoryEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Survival Story")
    FOnSurvivalMilestone OnSurvivalMilestone;

protected:
    // Core components
    UPROPERTY()
    UNarr_VoicelineDatabase* VoicelineDatabase;

    UPROPERTY()
    UNarr_StoryController* StoryController;

    // Story data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story System")
    FNarr_SurvivalProgress SurvivalProgress;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story System")
    TMap<FString, FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Story System")
    TMap<FString, FString> GameStateVariables;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float DayDurationHours;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    bool bAutoAdvanceDays;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float EventCheckInterval;

    // Internal state
    UPROPERTY()
    float LastDayAdvanceTime;

    UPROPERTY()
    float LastEventCheckTime;

    UPROPERTY()
    bool bSystemInitialized;

private:
    void InitializeDefaultEvents();
    void InitializePhaseEvents();
    void UpdateSurvivalPhase();
    void ProcessEventCooldowns();
    void TriggerPhaseTransition(ENarr_SurvivalPhase NewPhase);
    bool CheckEventConditions(const FNarr_StoryEvent& Event) const;
    void ExecuteStoryEvent(const FNarr_StoryEvent& Event);
    FString GetPhaseDescription(ENarr_SurvivalPhase Phase) const;
    TArray<FString> GetPhaseSpecificEvents(ENarr_SurvivalPhase Phase) const;
};
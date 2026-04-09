#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "EmotionalStates.h"
#include "StorylineManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStoryBeatTriggered, FName, StoryBeatID, const FText&, Description);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStorylineProgressed, FName, StorylineID, int32, CurrentBeat, int32, TotalBeats);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStorylineCompleted, FName, StorylineID, bool, bSuccessful);

/**
 * Story beat types for different narrative moments
 */
UENUM(BlueprintType)
enum class EStoryBeatType : uint8
{
    Introduction    UMETA(DisplayName = "Introduction - Character/World Setup"),
    Discovery       UMETA(DisplayName = "Discovery - Finding Something New"),
    Conflict        UMETA(DisplayName = "Conflict - Tension/Challenge"),
    Resolution      UMETA(DisplayName = "Resolution - Problem Solved"),
    Revelation      UMETA(DisplayName = "Revelation - Truth Uncovered"),
    Transformation  UMETA(DisplayName = "Transformation - Character Growth"),
    Climax          UMETA(DisplayName = "Climax - Peak Dramatic Moment"),
    Denouement      UMETA(DisplayName = "Denouement - Final Aftermath")
};

/**
 * Storyline priority for managing multiple concurrent narratives
 */
UENUM(BlueprintType)
enum class EStorylinePriority : uint8
{
    Background      UMETA(DisplayName = "Background - Environmental Lore"),
    Subplot         UMETA(DisplayName = "Subplot - Secondary Story"),
    MainQuest       UMETA(DisplayName = "Main Quest - Primary Narrative"),
    Critical        UMETA(DisplayName = "Critical - Essential Story Moment"),
    Ending          UMETA(DisplayName = "Ending - Final Story Resolution")
};

/**
 * Individual story beat within a larger storyline
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryBeat : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FName BeatID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    EStoryBeatType BeatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FEmotionalContext EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FGameplayTagContainer TriggerConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    FGameplayTagContainer CompletionTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FName> DialogueSequences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FName> QuestObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    TArray<FName> UnlockedStoryBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    float MinimumDuration; // Minimum time before beat can complete

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bCanSkip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Beat")
    bool bIsRepeatable;

    FStoryBeat()
    {
        BeatID = NAME_None;
        BeatType = EStoryBeatType::Introduction;
        MinimumDuration = 0.0f;
        bIsOptional = false;
        bCanSkip = false;
        bIsRepeatable = false;
    }
};

/**
 * Complete storyline containing multiple story beats
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryline : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    FName StorylineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    FText Summary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    EStorylinePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    TArray<FName> StoryBeats; // Ordered sequence of story beats

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    FGameplayTagContainer PrerequisiteTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    FGameplayTagContainer ConflictingStorylines; // Mutually exclusive storylines

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    int32 CurrentBeatIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    bool bCanBranch; // Allow player choices to affect storyline

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storyline")
    TArray<FName> BranchingStorylines; // Alternative storylines based on choices

    FStoryline()
    {
        StorylineID = NAME_None;
        Priority = EStorylinePriority::Subplot;
        CurrentBeatIndex = 0;
        bIsActive = false;
        bIsCompleted = false;
        bCanBranch = false;
    }
};

/**
 * Story progress tracking for save/load system
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FStoryProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    TMap<FName, int32> StorylineProgress; // StorylineID -> Current Beat Index

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    TArray<FName> CompletedStoryBeats;

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    TArray<FName> CompletedStorylines;

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    TArray<FName> UnlockedStorylines;

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    FGameplayTagContainer StoryTags; // Current story state tags

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    float TotalPlayTime;

    UPROPERTY(BlueprintReadWrite, Category = "Story Progress")
    FDateTime LastSaveTime;

    FStoryProgress()
    {
        TotalPlayTime = 0.0f;
        LastSaveTime = FDateTime::Now();
    }
};

/**
 * Storyline Manager - Controls narrative progression and story beats
 * Manages the overall narrative flow and story state in the prehistoric world
 */
UCLASS()
class TRANSPERSONALGAME_API UStorylineManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UStorylineManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Core Storyline Functions
    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool StartStoryline(FName StorylineID);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool AdvanceStoryline(FName StorylineID);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool TriggerStoryBeat(FName StoryBeatID, bool bForceActivation = false);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void CompleteStoryBeat(FName StoryBeatID);

    // Story State Queries
    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool IsStorylineActive(FName StorylineID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool IsStoryBeatCompleted(FName StoryBeatID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool CanStartStoryline(FName StorylineID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    int32 GetStorylineProgress(FName StorylineID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    FStoryBeat GetCurrentStoryBeat(FName StorylineID) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    TArray<FName> GetActiveStorylines() const;

    // Story Branching and Choices
    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool MakeStoryChoice(FName ChoiceID, FName SelectedOption);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    TArray<FName> GetAvailableStoryChoices() const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void UnlockStoryline(FName StorylineID);

    // Progress Management
    UFUNCTION(BlueprintCallable, Category = "Storyline")
    FStoryProgress GetStoryProgress() const { return CurrentProgress; }

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void LoadStoryProgress(const FStoryProgress& Progress);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void ResetStoryProgress();

    // Tag Integration
    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void AddStoryTag(FGameplayTag Tag);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    void RemoveStoryTag(FGameplayTag Tag);

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    bool HasStoryTag(FGameplayTag Tag) const;

    UFUNCTION(BlueprintCallable, Category = "Storyline")
    FGameplayTagContainer GetCurrentStoryTags() const { return CurrentProgress.StoryTags; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Storyline")
    FOnStoryBeatTriggered OnStoryBeatTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Storyline")
    FOnStorylineProgressed OnStorylineProgressed;

    UPROPERTY(BlueprintAssignable, Category = "Storyline")
    FOnStorylineCompleted OnStorylineCompleted;

protected:
    // Data Tables
    UPROPERTY(EditDefaultsOnly, Category = "Storyline")
    TSoftObjectPtr<UDataTable> StorylineDataTable;

    UPROPERTY(EditDefaultsOnly, Category = "Storyline")
    TSoftObjectPtr<UDataTable> StoryBeatDataTable;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Storyline")
    FStoryProgress CurrentProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Storyline")
    TArray<FName> ActiveStorylines;

    UPROPERTY(BlueprintReadOnly, Category = "Storyline")
    TMap<FName, float> StoryBeatTimers; // Track minimum duration requirements

    // Settings
    UPROPERTY(EditDefaultsOnly, Category = "Storyline")
    bool bAutoSaveProgress;

    UPROPERTY(EditDefaultsOnly, Category = "Storyline")
    float AutoSaveInterval;

    UPROPERTY(EditDefaultsOnly, Category = "Storyline")
    int32 MaxConcurrentStorylines;

private:
    // Internal Functions
    void LoadStorylineData();
    FStoryline* FindStoryline(FName StorylineID) const;
    FStoryBeat* FindStoryBeat(FName StoryBeatID) const;
    bool ValidateStorylineConditions(const FStoryline& Storyline) const;
    bool ValidateStoryBeatConditions(const FStoryBeat& StoryBeat) const;
    void ProcessStoryBeatTimers(float DeltaTime);
    void CheckForAutoUnlocks();
    void SaveProgressToFile();
    void SortActiveStorylinesByPriority();
    
    // Timers
    FTimerHandle AutoSaveTimer;
    FTimerHandle StoryProcessingTimer;
    
    bool bIsInitialized;
    float LastSaveTime;
};
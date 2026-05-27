#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeStoryManager.generated.h"

// Story progression tracking
UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player awakens alone, tutorial phase
    Survival,       // Basic survival needs: water, food, shelter
    Discovery,      // Finding signs of other survivors
    Territory,      // Claiming and defending territory
    Mastery,        // Advanced survival and leadership
    Endgame         // Final story resolution
};

// Story event types for narrative triggers
UENUM(BlueprintType)
enum class ENarr_StoryEventType : uint8
{
    FirstWater,     // Player finds first water source
    FirstHunt,      // Player kills first animal
    FirstShelter,   // Player builds first shelter
    FirstDanger,    // Player encounters major predator
    FirstTribe,     // Player meets other survivors
    TerritoryLost,  // Player's base is destroyed
    MajorVictory    // Player defeats alpha predator
};

// Narrative context data for dialogue selection
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_StoryPhase CurrentPhase = ENarr_StoryPhase::Awakening;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DaysAlive = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AnimalsHunted = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PredatorsDefeated = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasShelter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bMetOtherSurvivors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel = 0.5f; // 0.0 = fearless, 1.0 = terrified

    FNarr_NarrativeContext()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        DaysAlive = 0;
        AnimalsHunted = 0;
        PredatorsDefeated = 0;
        bHasWater = false;
        bHasShelter = false;
        bMetOtherSurvivors = false;
        FearLevel = 0.5f;
    }
};

// Story event data for tracking progression
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_StoryEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime EventTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EventLocation;

    FNarr_StoryEvent()
    {
        EventType = ENarr_StoryEventType::FirstWater;
        EventDescription = TEXT("");
        EventTime = FDateTime::Now();
        EventLocation = FVector::ZeroVector;
    }
};

/**
 * Manages the overarching narrative progression and story context
 * Tracks player achievements and triggers appropriate story moments
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarrativeStoryManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarrativeStoryManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return NarrativeContext.CurrentPhase; }

    // Event tracking
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RecordStoryEvent(ENarr_StoryEventType EventType, const FString& Description, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool HasEventOccurred(ENarr_StoryEventType EventType) const;

    // Context updates
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateSurvivalStats(bool bFoundWater, bool bBuiltShelter, int32 NewAnimalsHunted);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void IncrementDaysAlive();

    // Context access
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_NarrativeContext GetNarrativeContext() const { return NarrativeContext; }

    // Story triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetContextualNarration() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool ShouldTriggerStoryMoment() const;

protected:
    // Current narrative state
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    FNarr_NarrativeContext NarrativeContext;

    // Event history
    UPROPERTY(BlueprintReadOnly, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    // Story phase descriptions
    UPROPERTY(EditDefaultsOnly, Category = "Narrative")
    TMap<ENarr_StoryPhase, FString> PhaseDescriptions;

    // Initialize story data
    void InitializeStoryData();

    // Helper functions
    FString GetPhaseNarration(ENarr_StoryPhase Phase) const;
    bool IsStoryEventRecent(ENarr_StoryEventType EventType, float HoursAgo = 1.0f) const;
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
#include "Narr_StoryManager.generated.h"

// Story progression states for prehistoric survival narrative
UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,          // Player awakens in prehistoric world
    FirstSurvival,      // Learning basic survival mechanics
    TribalContact,      // First contact with other survivors
    TerritoryExploration, // Exploring dangerous territories
    DinosaurEncounters, // Major dinosaur confrontations
    TribalIntegration,  // Joining or forming tribal groups
    EnvironmentalCrisis, // Major environmental disasters
    TribalWarfare,      // Conflicts between groups
    Evolution,          // Technological/social advancement
    Legacy              // Establishing lasting civilization
};

// Story event types that can trigger narrative responses
UENUM(BlueprintType)
enum class ENarr_StoryEventType : uint8
{
    PlayerDeath,        // Player character dies
    DinosaurKill,       // Player kills major dinosaur
    TribalMeeting,      // First contact with NPC tribe
    ResourceDiscovery,  // Finding major resource cache
    EnvironmentalHazard, // Natural disaster occurs
    TerritoryConquest,  // Claiming new territory
    CraftingBreakthrough, // Major technological advance
    AllianceFormed,     // Political alliance created
    ConflictResolution, // Major conflict resolved
    SeasonalChange      // Environmental season shift
};

// Character archetype for narrative purposes
UENUM(BlueprintType)
enum class ENarr_CharacterArchetype : uint8
{
    Survivor,           // Basic survival-focused character
    Hunter,             // Predator/combat specialist
    Gatherer,           // Resource collection specialist
    Crafter,            // Tool/weapon creation expert
    Explorer,           // Territory expansion focused
    Leader,             // Tribal leadership role
    Healer,             // Medical/support specialist
    Scout,              // Reconnaissance specialist
    Warrior,            // Combat-focused tribal member
    Elder               // Wisdom/knowledge keeper
};

// Story progression data structure
USTRUCT(BlueprintType)
struct FNarr_StoryProgressionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    ENarr_StoryPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    TArray<ENarr_StoryEventType> CompletedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 DaysInWorld;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 DinosaurKillCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    int32 TribesEncountered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    float SurvivalRating;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Progression")
    ENarr_CharacterArchetype PlayerArchetype;

    FNarr_StoryProgressionData()
    {
        CurrentPhase = ENarr_StoryPhase::Awakening;
        CompletedEvents.Empty();
        DaysInWorld = 0;
        DinosaurKillCount = 0;
        TribesEncountered = 0;
        SurvivalRating = 0.0f;
        PlayerArchetype = ENarr_CharacterArchetype::Survivor;
    }
};

// Narrative event data for story triggers
USTRUCT(BlueprintType)
struct FNarr_NarrativeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    ENarr_StoryEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    ENarr_StoryPhase RequiredPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    float EventPriority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Event")
    bool bIsRepeatable;

    FNarr_NarrativeEvent()
    {
        EventTitle = TEXT("Unknown Event");
        EventDescription = TEXT("A mysterious event occurs in the prehistoric world.");
        EventType = ENarr_StoryEventType::SeasonalChange;
        RequiredPhase = ENarr_StoryPhase::Awakening;
        EventPriority = 1.0f;
        bIsRepeatable = false;
    }
};

/**
 * Story Manager for prehistoric survival narrative
 * Manages story progression, character development, and narrative events
 * Integrates with quest system and environmental storytelling
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_StoryManager : public AActor
{
    GENERATED_BODY()

public:
    ANarr_StoryManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core story progression system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    FNarr_StoryProgressionData StoryProgression;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TArray<FNarr_NarrativeEvent> ActiveEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story System")
    TArray<FNarr_NarrativeEvent> CompletedEvents;

    // Story progression methods
    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void TriggerStoryEvent(ENarr_StoryEventType EventType, const FString& EventContext);

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    bool IsStoryEventCompleted(ENarr_StoryEventType EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Story Progression")
    void UpdatePlayerArchetype(ENarr_CharacterArchetype NewArchetype);

    // Character development tracking
    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void RecordDinosaurKill(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void RecordTribalEncounter(const FString& TribeName);

    UFUNCTION(BlueprintCallable, Category = "Character Development")
    void UpdateSurvivalRating(float RatingChange);

    // Narrative event management
    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void RegisterNarrativeEvent(const FNarr_NarrativeEvent& NewEvent);

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    TArray<FNarr_NarrativeEvent> GetAvailableEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative Events")
    void CompleteNarrativeEvent(const FString& EventTitle);

    // Story query methods
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story Query")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return StoryProgression.CurrentPhase; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story Query")
    int32 GetDaysInWorld() const { return StoryProgression.DaysInWorld; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story Query")
    ENarr_CharacterArchetype GetPlayerArchetype() const { return StoryProgression.PlayerArchetype; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Story Query")
    float GetSurvivalRating() const { return StoryProgression.SurvivalRating; }

protected:
    // Internal story management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Story progression timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Timing")
    float DayProgressionTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Timing")
    float EventCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Timing")
    float LastEventCheckTime;

    // Story configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    float SecondsPerGameDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    bool bAutoProgressStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Config")
    float StoryProgressionThreshold;

private:
    void ProcessDayProgression(float DeltaTime);
    void CheckForStoryEvents();
    void EvaluateStoryPhaseProgression();
    FString GetStoryPhaseDescription(ENarr_StoryPhase Phase) const;
    FString GetEventTypeDescription(ENarr_StoryEventType EventType) const;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "../Core/SharedTypes.h"
#include "Narr_StoryManager.generated.h"

/**
 * NARRATIVE AGENT #15 - STORY MANAGER
 * 
 * Manages the overarching narrative flow of the prehistoric survival game.
 * Tracks player progress, triggers story beats, and manages narrative state.
 * Focuses on realistic survival themes - no spiritual/mystical content.
 */

UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Arrival = 0         UMETA(DisplayName = "Arrival"),
    FirstContact = 1    UMETA(DisplayName = "First Contact"),
    Exploration = 2     UMETA(DisplayName = "Exploration"),
    Survival = 3        UMETA(DisplayName = "Survival"),
    Adaptation = 4      UMETA(DisplayName = "Adaptation"),
    Mastery = 5         UMETA(DisplayName = "Mastery")
};

UENUM(BlueprintType)
enum class ENarr_ThreatLevel : uint8
{
    Safe = 0            UMETA(DisplayName = "Safe"),
    Cautious = 1        UMETA(DisplayName = "Cautious"),
    Dangerous = 2       UMETA(DisplayName = "Dangerous"),
    Deadly = 3          UMETA(DisplayName = "Deadly"),
    Extreme = 4         UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryPhase RequiredPhase = ENarr_StoryPhase::Arrival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float Timestamp = 0.0f;

    FNarr_StoryEvent()
    {
        EventID = TEXT("");
        EventName = TEXT("");
        Description = TEXT("");
        RequiredPhase = ENarr_StoryPhase::Arrival;
        bIsCompleted = false;
        Timestamp = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NarrativeContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_StoryPhase CurrentPhase = ENarr_StoryPhase::Arrival;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    ENarr_ThreatLevel ThreatLevel = ENarr_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 DaysElapsed = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 DinosaurEncounters = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    int32 SurvivalMilestones = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> CompletedEvents;

    FNarr_NarrativeContext()
    {
        CurrentPhase = ENarr_StoryPhase::Arrival;
        ThreatLevel = ENarr_ThreatLevel::Safe;
        CurrentBiome = TEXT("Unknown");
        DaysElapsed = 0;
        DinosaurEncounters = 0;
        SurvivalMilestones = 0;
        CompletedEvents.Empty();
    }
};

/**
 * World Subsystem that manages the narrative flow and story progression
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_StoryManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UNarr_StoryManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Story progression
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase(ENarr_StoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateThreatLevel(ENarr_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RecordDinosaurEncounter(const FString& DinosaurType);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return NarrativeContext.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_ThreatLevel GetCurrentThreatLevel() const { return NarrativeContext.ThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FNarr_NarrativeContext GetNarrativeContext() const { return NarrativeContext; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool IsEventCompleted(const FString& EventID) const;

    // Narrative triggers
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FString GetContextualNarration() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableDialogueOptions() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FNarr_NarrativeContext NarrativeContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<ENarr_StoryPhase, FString> PhaseDescriptions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TMap<ENarr_ThreatLevel, FString> ThreatDescriptions;

private:
    void InitializeStoryEvents();
    void InitializeDescriptions();
    void BroadcastStoryEvent(const FNarr_StoryEvent& Event);
    FString GenerateContextualNarration() const;
};
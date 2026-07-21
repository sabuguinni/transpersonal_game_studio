#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "NarrativeStoryManager.generated.h"

// Story progression states for the main narrative
UENUM(BlueprintType)
enum class ENarr_StoryPhase : uint8
{
    Awakening,      // Player awakens in prehistoric world
    FirstContact,   // Encounters first dinosaurs
    Survival,       // Learning basic survival skills
    Discovery,      // Finding tribal remnants
    Alliance,       // Building relationships with NPCs
    Conflict,       // Major dinosaur threats emerge
    Resolution      // Final story climax
};

// Critical story events that trigger narrative beats
UENUM(BlueprintType)
enum class ENarr_StoryEvent : uint8
{
    PlayerSpawned,
    FirstDinosaurSighted,
    FirstCraftingSuccess,
    FirstNPCMet,
    FirstCombatVictory,
    TribeDiscovered,
    AlphaRexEncounter,
    StoryComplete
};

// Story data structure for data table
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryBeat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    ENarr_StoryEvent TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString VoiceOverFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bBlocksGameplay;

    FNarr_StoryBeat()
    {
        TriggerEvent = ENarr_StoryEvent::PlayerSpawned;
        NarrativeText = TEXT("Default narrative text");
        VoiceOverFile = TEXT("");
        Duration = 3.0f;
        bBlocksGameplay = false;
    }
};

/**
 * Central narrative manager that tracks story progression and triggers narrative beats
 * Integrates with quest system and character development
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
    void TriggerStoryEvent(ENarr_StoryEvent Event);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AdvanceStoryPhase();

    UFUNCTION(BlueprintPure, Category = "Narrative")
    ENarr_StoryPhase GetCurrentStoryPhase() const { return CurrentStoryPhase; }

    UFUNCTION(BlueprintPure, Category = "Narrative")
    bool HasEventTriggered(ENarr_StoryEvent Event) const;

    // Narrative beats
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayNarrativeBeat(const FNarr_StoryBeat& StoryBeat);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetStoryDataTable(UDataTable* DataTable);

    // Story state queries
    UFUNCTION(BlueprintPure, Category = "Narrative")
    float GetStoryProgressPercent() const;

    UFUNCTION(BlueprintPure, Category = "Narrative")
    FString GetCurrentNarrativeContext() const;

    // Integration with other systems
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void RegisterStoryEventListener(UObject* Listener);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UnregisterStoryEventListener(UObject* Listener);

protected:
    // Current story state
    UPROPERTY(BlueprintReadOnly, Category = "Story State")
    ENarr_StoryPhase CurrentStoryPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Story State")
    TArray<ENarr_StoryEvent> TriggeredEvents;

    UPROPERTY(BlueprintReadOnly, Category = "Story State")
    float StoryStartTime;

    // Story configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    UDataTable* StoryBeatsDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoAdvancePhases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float NarrativeBeatDelay;

    // Event listeners
    UPROPERTY()
    TArray<TWeakObjectPtr<UObject>> EventListeners;

private:
    void InitializeStoryBeats();
    void BroadcastStoryEvent(ENarr_StoryEvent Event);
    FNarr_StoryBeat* FindStoryBeatForEvent(ENarr_StoryEvent Event);
    void UpdateStoryPhaseBasedOnEvents();
};
#include "NarrativeStoryManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNarrativeStoryManager::UNarrativeStoryManager()
{
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
    StoryStartTime = 0.0f;
    bAutoAdvancePhases = true;
    NarrativeBeatDelay = 1.0f;
    StoryBeatsDataTable = nullptr;
}

void UNarrativeStoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    StoryStartTime = FPlatformTime::Seconds();
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
    TriggeredEvents.Empty();
    
    InitializeStoryBeats();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager initialized - Story begins"));
}

void UNarrativeStoryManager::Deinitialize()
{
    EventListeners.Empty();
    Super::Deinitialize();
}

void UNarrativeStoryManager::TriggerStoryEvent(ENarr_StoryEvent Event)
{
    if (HasEventTriggered(Event))
    {
        return; // Event already triggered
    }
    
    TriggeredEvents.Add(Event);
    
    UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %d"), (int32)Event);
    
    // Find and play corresponding narrative beat
    FNarr_StoryBeat* StoryBeat = FindStoryBeatForEvent(Event);
    if (StoryBeat)
    {
        PlayNarrativeBeat(*StoryBeat);
    }
    
    // Update story phase if auto-advance is enabled
    if (bAutoAdvancePhases)
    {
        UpdateStoryPhaseBasedOnEvents();
    }
    
    // Broadcast to listeners
    BroadcastStoryEvent(Event);
}

void UNarrativeStoryManager::AdvanceStoryPhase()
{
    int32 CurrentPhaseInt = (int32)CurrentStoryPhase;
    int32 MaxPhaseInt = (int32)ENarr_StoryPhase::Resolution;
    
    if (CurrentPhaseInt < MaxPhaseInt)
    {
        CurrentStoryPhase = (ENarr_StoryPhase)(CurrentPhaseInt + 1);
        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced to: %d"), CurrentPhaseInt + 1);
    }
}

bool UNarrativeStoryManager::HasEventTriggered(ENarr_StoryEvent Event) const
{
    return TriggeredEvents.Contains(Event);
}

void UNarrativeStoryManager::PlayNarrativeBeat(const FNarr_StoryBeat& StoryBeat)
{
    UE_LOG(LogTemp, Warning, TEXT("Playing narrative beat: %s"), *StoryBeat.NarrativeText);
    
    // TODO: Integrate with UI system to display narrative text
    // TODO: Integrate with audio system to play voice-over
    
    if (StoryBeat.bBlocksGameplay)
    {
        // TODO: Pause game or show narrative overlay
    }
}

void UNarrativeStoryManager::SetStoryDataTable(UDataTable* DataTable)
{
    StoryBeatsDataTable = DataTable;
    InitializeStoryBeats();
}

float UNarrativeStoryManager::GetStoryProgressPercent() const
{
    int32 CurrentPhaseInt = (int32)CurrentStoryPhase;
    int32 MaxPhaseInt = (int32)ENarr_StoryPhase::Resolution;
    
    return (float)CurrentPhaseInt / (float)MaxPhaseInt * 100.0f;
}

FString UNarrativeStoryManager::GetCurrentNarrativeContext() const
{
    switch (CurrentStoryPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("You awaken in a strange, primitive world. The air is thick with danger.");
        case ENarr_StoryPhase::FirstContact:
            return TEXT("Massive creatures roam these lands. You must learn to survive among them.");
        case ENarr_StoryPhase::Survival:
            return TEXT("Each day is a struggle. Food, shelter, and safety are your priorities.");
        case ENarr_StoryPhase::Discovery:
            return TEXT("Ancient ruins hint at others who came before. You are not alone.");
        case ENarr_StoryPhase::Alliance:
            return TEXT("Trust is earned through action. Allies can mean the difference between life and death.");
        case ENarr_StoryPhase::Conflict:
            return TEXT("The apex predators have taken notice. The real test begins now.");
        case ENarr_StoryPhase::Resolution:
            return TEXT("You have proven yourself worthy of this harsh world. Your legend begins.");
        default:
            return TEXT("Unknown phase");
    }
}

void UNarrativeStoryManager::RegisterStoryEventListener(UObject* Listener)
{
    if (Listener)
    {
        EventListeners.AddUnique(TWeakObjectPtr<UObject>(Listener));
    }
}

void UNarrativeStoryManager::UnregisterStoryEventListener(UObject* Listener)
{
    if (Listener)
    {
        EventListeners.Remove(TWeakObjectPtr<UObject>(Listener));
    }
}

void UNarrativeStoryManager::InitializeStoryBeats()
{
    // Initialize default story beats if no data table is provided
    if (!StoryBeatsDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("No story beats data table provided - using defaults"));
        return;
    }
    
    // TODO: Load story beats from data table
    UE_LOG(LogTemp, Warning, TEXT("Story beats initialized from data table"));
}

void UNarrativeStoryManager::BroadcastStoryEvent(ENarr_StoryEvent Event)
{
    // Clean up invalid listeners
    EventListeners.RemoveAll([](const TWeakObjectPtr<UObject>& Listener)
    {
        return !Listener.IsValid();
    });
    
    // Broadcast to valid listeners
    for (const TWeakObjectPtr<UObject>& Listener : EventListeners)
    {
        if (Listener.IsValid())
        {
            // TODO: Call interface method on listener
            UE_LOG(LogTemp, Log, TEXT("Broadcasting story event to listener"));
        }
    }
}

FNarr_StoryBeat* UNarrativeStoryManager::FindStoryBeatForEvent(ENarr_StoryEvent Event)
{
    if (!StoryBeatsDataTable)
    {
        return nullptr;
    }
    
    // TODO: Search data table for matching story beat
    return nullptr;
}

void UNarrativeStoryManager::UpdateStoryPhaseBasedOnEvents()
{
    // Advance story phases based on triggered events
    switch (CurrentStoryPhase)
    {
        case ENarr_StoryPhase::Awakening:
            if (HasEventTriggered(ENarr_StoryEvent::FirstDinosaurSighted))
            {
                AdvanceStoryPhase();
            }
            break;
            
        case ENarr_StoryPhase::FirstContact:
            if (HasEventTriggered(ENarr_StoryEvent::FirstCraftingSuccess))
            {
                AdvanceStoryPhase();
            }
            break;
            
        case ENarr_StoryPhase::Survival:
            if (HasEventTriggered(ENarr_StoryEvent::FirstNPCMet))
            {
                AdvanceStoryPhase();
            }
            break;
            
        case ENarr_StoryPhase::Discovery:
            if (HasEventTriggered(ENarr_StoryEvent::TribeDiscovered))
            {
                AdvanceStoryPhase();
            }
            break;
            
        case ENarr_StoryPhase::Alliance:
            if (HasEventTriggered(ENarr_StoryEvent::FirstCombatVictory))
            {
                AdvanceStoryPhase();
            }
            break;
            
        case ENarr_StoryPhase::Conflict:
            if (HasEventTriggered(ENarr_StoryEvent::AlphaRexEncounter))
            {
                AdvanceStoryPhase();
            }
            break;
            
        default:
            break;
    }
}
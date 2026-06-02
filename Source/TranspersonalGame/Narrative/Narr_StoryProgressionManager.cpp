#include "Narr_StoryProgressionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UNarr_StoryProgressionManager::UNarr_StoryProgressionManager()
{
    CurrentProgress = FNarr_ChapterProgress();
}

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeChapterNarratives();
    InitializeDefaultEvents();
    UpdateAvailableEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Story Progression Manager initialized - Chapter: %d"), 
           static_cast<int32>(CurrentProgress.CurrentChapter));
}

void UNarr_StoryProgressionManager::TriggerStoryEvent(const FString& EventID)
{
    if (!CanTriggerEvent(EventID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot trigger story event: %s"), *EventID);
        return;
    }

    const FNarr_StoryEvent* Event = RegisteredEvents.Find(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Error, TEXT("Story event not found: %s"), *EventID);
        return;
    }

    // Mark event as completed
    MarkEventCompleted(EventID);
    
    // Update chapter progress
    CurrentProgress.ChapterProgress += 12.5f; // Each event advances chapter by 12.5%
    if (CurrentProgress.ChapterProgress >= 100.0f)
    {
        // Auto-advance to next chapter
        ENarr_StoryChapter NextChapter = static_cast<ENarr_StoryChapter>(
            static_cast<int32>(CurrentProgress.CurrentChapter) + 1);
        
        if (NextChapter <= ENarr_StoryChapter::Legacy)
        {
            AdvanceChapter(NextChapter);
        }
    }

    // Log narrative text
    UE_LOG(LogTemp, Log, TEXT("Story Event Triggered: %s - %s"), 
           *EventID, *Event->NarrativeText);

    // Update available events
    UpdateAvailableEvents();
}

void UNarr_StoryProgressionManager::AdvanceChapter(ENarr_StoryChapter NewChapter)
{
    if (NewChapter == CurrentProgress.CurrentChapter)
    {
        return;
    }

    ENarr_StoryChapter OldChapter = CurrentProgress.CurrentChapter;
    CurrentProgress.CurrentChapter = NewChapter;
    CurrentProgress.ChapterProgress = 0.0f;
    CurrentProgress.ChapterStartTime = FDateTime::Now();
    
    UE_LOG(LogTemp, Warning, TEXT("Chapter Advanced: %d -> %d"), 
           static_cast<int32>(OldChapter), static_cast<int32>(NewChapter));

    // Clear completed events for new chapter
    CurrentProgress.CompletedEvents.Empty();
    UpdateAvailableEvents();
}

bool UNarr_StoryProgressionManager::CanTriggerEvent(const FString& EventID) const
{
    const FNarr_StoryEvent* Event = RegisteredEvents.Find(EventID);
    if (!Event)
    {
        return false;
    }

    // Check if already completed and not repeatable
    if (!Event->bIsRepeatable && IsEventCompleted(EventID))
    {
        return false;
    }

    // Check chapter requirement
    if (Event->RequiredChapter != CurrentProgress.CurrentChapter)
    {
        return false;
    }

    // Check trigger condition
    return CheckTriggerCondition(*Event);
}

ENarr_StoryChapter UNarr_StoryProgressionManager::GetCurrentChapter() const
{
    return CurrentProgress.CurrentChapter;
}

float UNarr_StoryProgressionManager::GetChapterProgress() const
{
    return CurrentProgress.ChapterProgress;
}

TArray<FString> UNarr_StoryProgressionManager::GetAvailableEvents() const
{
    return CurrentProgress.AvailableEvents;
}

void UNarr_StoryProgressionManager::RegisterStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    RegisteredEvents.Add(NewEvent.EventID, NewEvent);
    UpdateAvailableEvents();
    
    UE_LOG(LogTemp, Log, TEXT("Story event registered: %s"), *NewEvent.EventID);
}

void UNarr_StoryProgressionManager::UnregisterStoryEvent(const FString& EventID)
{
    RegisteredEvents.Remove(EventID);
    CurrentProgress.AvailableEvents.Remove(EventID);
    CurrentProgress.CompletedEvents.Remove(EventID);
    
    UE_LOG(LogTemp, Log, TEXT("Story event unregistered: %s"), *EventID);
}

void UNarr_StoryProgressionManager::SetChapterProgress(float Progress)
{
    CurrentProgress.ChapterProgress = FMath::Clamp(Progress, 0.0f, 100.0f);
}

bool UNarr_StoryProgressionManager::IsEventCompleted(const FString& EventID) const
{
    return CurrentProgress.CompletedEvents.Contains(EventID);
}

void UNarr_StoryProgressionManager::MarkEventCompleted(const FString& EventID)
{
    if (!IsEventCompleted(EventID))
    {
        CurrentProgress.CompletedEvents.Add(EventID);
    }
}

FString UNarr_StoryProgressionManager::GetChapterNarrative(ENarr_StoryChapter Chapter) const
{
    const FString* Narrative = ChapterNarratives.Find(Chapter);
    return Narrative ? *Narrative : TEXT("Unknown chapter narrative");
}

FString UNarr_StoryProgressionManager::GetEventNarrative(const FString& EventID) const
{
    const FNarr_StoryEvent* Event = RegisteredEvents.Find(EventID);
    return Event ? Event->NarrativeText : TEXT("Unknown event narrative");
}

void UNarr_StoryProgressionManager::InitializeChapterNarratives()
{
    ChapterNarratives.Empty();
    
    ChapterNarratives.Add(ENarr_StoryChapter::Awakening, 
        TEXT("You awaken in a world ruled by giants. Every sound could mean death, every shadow could hide a predator. Learn quickly, or become prey."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::FirstHunt, 
        TEXT("The taste of victory is sweet, but the hunt has only begun. Larger beasts roam these lands, and they have noticed your presence."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::TribalContact, 
        TEXT("You are not alone. Others have survived the endless hunt, but trust is a luxury few can afford in this savage world."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::PackLeader, 
        TEXT("Leadership is earned through blood and cunning. Your pack looks to you for guidance, but every decision could be their last."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::TerritoryWars, 
        TEXT("The land grows scarce, and other tribes eye your territory with hungry eyes. War is coming, and only the strongest will claim these hunting grounds."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::Exodus, 
        TEXT("The old lands burn behind you. Ahead lies the unknown, but staying means certain death. Lead your people to safety, if such a place exists."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::NewLands, 
        TEXT("Fresh territories stretch before you, untamed and full of promise. But new lands bring new dangers, and the cycle begins anew."));
    
    ChapterNarratives.Add(ENarr_StoryChapter::Legacy, 
        TEXT("Your name will be remembered in the songs of future generations. The settlement you built stands as proof that humanity can endure, even in the age of giants."));
}

void UNarr_StoryProgressionManager::InitializeDefaultEvents()
{
    RegisteredEvents.Empty();

    // Awakening Chapter Events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("awakening_first_steps");
    FirstSteps.RequiredChapter = ENarr_StoryChapter::Awakening;
    FirstSteps.TriggerType = ENarr_StoryTrigger::PlayerAction;
    FirstSteps.TriggerCondition = TEXT("move_distance_100");
    FirstSteps.NarrativeText = TEXT("Your legs shake with each step. The ground beneath feels alien, dangerous. Every instinct screams to find shelter.");
    FirstSteps.AudioCueID = TEXT("narr_first_steps");
    FirstSteps.bIsRepeatable = false;
    RegisteredEvents.Add(FirstSteps.EventID, FirstSteps);

    FNarr_StoryEvent FirstShelter;
    FirstShelter.EventID = TEXT("awakening_first_shelter");
    FirstShelter.RequiredChapter = ENarr_StoryChapter::Awakening;
    FirstShelter.TriggerType = ENarr_StoryTrigger::PlayerAction;
    FirstShelter.TriggerCondition = TEXT("build_shelter");
    FirstShelter.NarrativeText = TEXT("The crude shelter offers little comfort, but it is yours. For the first time since awakening, you feel a moment of safety.");
    FirstShelter.AudioCueID = TEXT("narr_first_shelter");
    FirstShelter.bIsRepeatable = false;
    RegisteredEvents.Add(FirstShelter.EventID, FirstShelter);

    // FirstHunt Chapter Events
    FNarr_StoryEvent FirstKill;
    FirstKill.EventID = TEXT("firsthunt_first_kill");
    FirstKill.RequiredChapter = ENarr_StoryChapter::FirstHunt;
    FirstKill.TriggerType = ENarr_StoryTrigger::CombatVictory;
    FirstKill.TriggerCondition = TEXT("kill_dinosaur");
    FirstKill.NarrativeText = TEXT("The beast lies still, its blood warm on your hands. You are no longer just prey - you are a hunter.");
    FirstKill.AudioCueID = TEXT("narr_first_kill");
    FirstKill.bIsRepeatable = false;
    RegisteredEvents.Add(FirstKill.EventID, FirstKill);

    // TribalContact Chapter Events
    FNarr_StoryEvent FirstContact;
    FirstContact.EventID = TEXT("tribal_first_contact");
    FirstContact.RequiredChapter = ENarr_StoryChapter::TribalContact;
    FirstContact.TriggerType = ENarr_StoryTrigger::NPCInteraction;
    FirstContact.TriggerCondition = TEXT("meet_tribal_npc");
    FirstContact.NarrativeText = TEXT("The stranger's eyes hold the same wariness as your own. In this world of giants, finding another human is both blessing and curse.");
    FirstContact.AudioCueID = TEXT("narr_first_contact");
    FirstContact.bIsRepeatable = false;
    RegisteredEvents.Add(FirstContact.EventID, FirstContact);
}

void UNarr_StoryProgressionManager::UpdateAvailableEvents()
{
    CurrentProgress.AvailableEvents.Empty();
    
    for (const auto& EventPair : RegisteredEvents)
    {
        const FNarr_StoryEvent& Event = EventPair.Value;
        
        if (Event.RequiredChapter == CurrentProgress.CurrentChapter)
        {
            if (Event.bIsRepeatable || !IsEventCompleted(Event.EventID))
            {
                CurrentProgress.AvailableEvents.Add(Event.EventID);
            }
        }
    }
}

bool UNarr_StoryProgressionManager::CheckTriggerCondition(const FNarr_StoryEvent& Event) const
{
    // Basic condition checking - in a full implementation, this would check game state
    // For now, return true to allow manual triggering
    return true;
}
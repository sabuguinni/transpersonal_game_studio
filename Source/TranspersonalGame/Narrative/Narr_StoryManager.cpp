#include "Narr_StoryManager.h"
#include "Engine/Engine.h"

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentPhase = ENarr_StoryPhase::Awakening;
    CompletedEventsCount = 0;
    TotalEventsCount = 0;
    
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Story Manager initialized - Starting phase: Awakening"));
}

void UNarr_StoryManager::AdvanceStoryPhase()
{
    int32 CurrentPhaseInt = static_cast<int32>(CurrentPhase);
    int32 MaxPhaseInt = static_cast<int32>(ENarr_StoryPhase::Mastery);
    
    if (CurrentPhaseInt < MaxPhaseInt)
    {
        CurrentPhase = static_cast<ENarr_StoryPhase>(CurrentPhaseInt + 1);
        UE_LOG(LogTemp, Warning, TEXT("Advanced to story phase: %d"), CurrentPhaseInt + 1);
    }
}

ENarr_StoryPhase UNarr_StoryManager::GetCurrentStoryPhase() const
{
    return CurrentPhase;
}

void UNarr_StoryManager::CompleteStoryEvent(const FString& EventID)
{
    if (StoryEvents.Contains(EventID))
    {
        FNarr_StoryEvent& Event = StoryEvents[EventID];
        if (!Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            CompletedEventsCount++;
            
            UE_LOG(LogTemp, Warning, TEXT("Completed story event: %s"), *EventID);
            
            // Check if we should advance phase
            TArray<FNarr_StoryEvent> CurrentPhaseEvents = GetAvailableEvents();
            bool bAllCompleted = true;
            for (const FNarr_StoryEvent& PhaseEvent : CurrentPhaseEvents)
            {
                if (!PhaseEvent.bIsCompleted)
                {
                    bAllCompleted = false;
                    break;
                }
            }
            
            if (bAllCompleted && CurrentPhaseEvents.Num() > 0)
            {
                AdvanceStoryPhase();
            }
        }
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID) const
{
    if (StoryEvents.Contains(EventID))
    {
        return StoryEvents[EventID].bIsCompleted;
    }
    return false;
}

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const auto& EventPair : StoryEvents)
    {
        const FNarr_StoryEvent& Event = EventPair.Value;
        if (Event.RequiredPhase == CurrentPhase && ArePrerequisitesMet(Event))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

FString UNarr_StoryManager::GetCurrentPhaseDescription() const
{
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("You awaken in the prehistoric wilderness. Survival is your only goal.");
        case ENarr_StoryPhase::FirstHunt:
            return TEXT("Learn to hunt or become prey. The ancient beasts test your resolve.");
        case ENarr_StoryPhase::TribeContact:
            return TEXT("Other survivors emerge from the shadows. Trust is earned through action.");
        case ENarr_StoryPhase::TerritoryWars:
            return TEXT("Resources are scarce. Territories must be claimed and defended.");
        case ENarr_StoryPhase::AlphaChallenge:
            return TEXT("Face the apex predators. Only the strongest will lead.");
        case ENarr_StoryPhase::Mastery:
            return TEXT("You have mastered the ancient world. New challenges await.");
        default:
            return TEXT("Unknown phase");
    }
}

void UNarr_StoryManager::RegisterStoryEvent(const FNarr_StoryEvent& Event)
{
    StoryEvents.Add(Event.EventID, Event);
    TotalEventsCount++;
    UE_LOG(LogTemp, Warning, TEXT("Registered story event: %s"), *Event.EventID);
}

float UNarr_StoryManager::GetStoryProgress() const
{
    if (TotalEventsCount == 0)
    {
        return 0.0f;
    }
    
    return static_cast<float>(CompletedEventsCount) / static_cast<float>(TotalEventsCount);
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    // Awakening Phase Events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("FirstSteps");
    FirstSteps.EventDescription = TEXT("Take your first steps in the prehistoric world");
    FirstSteps.RequiredPhase = ENarr_StoryPhase::Awakening;
    RegisterStoryEvent(FirstSteps);
    
    FNarr_StoryEvent FindShelter;
    FindShelter.EventID = TEXT("FindShelter");
    FindShelter.EventDescription = TEXT("Locate safe shelter before nightfall");
    FindShelter.RequiredPhase = ENarr_StoryPhase::Awakening;
    RegisterStoryEvent(FindShelter);
    
    FNarr_StoryEvent GatherResources;
    GatherResources.EventID = TEXT("GatherResources");
    GatherResources.EventDescription = TEXT("Collect basic survival resources");
    GatherResources.RequiredPhase = ENarr_StoryPhase::Awakening;
    RegisterStoryEvent(GatherResources);
    
    // First Hunt Phase Events
    FNarr_StoryEvent CraftWeapon;
    CraftWeapon.EventID = TEXT("CraftWeapon");
    CraftWeapon.EventDescription = TEXT("Craft your first primitive weapon");
    CraftWeapon.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    CraftWeapon.Prerequisites.Add(TEXT("GatherResources"));
    RegisterStoryEvent(CraftWeapon);
    
    FNarr_StoryEvent FirstKill;
    FirstKill.EventID = TEXT("FirstKill");
    FirstKill.EventDescription = TEXT("Successfully hunt your first prey");
    FirstKill.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    FirstKill.Prerequisites.Add(TEXT("CraftWeapon"));
    RegisterStoryEvent(FirstKill);
    
    // Tribe Contact Phase Events
    FNarr_StoryEvent MeetSurvivor;
    MeetSurvivor.EventID = TEXT("MeetSurvivor");
    MeetSurvivor.EventDescription = TEXT("Encounter another survivor");
    MeetSurvivor.RequiredPhase = ENarr_StoryPhase::TribeContact;
    RegisterStoryEvent(MeetSurvivor);
    
    FNarr_StoryEvent EstablishTrust;
    EstablishTrust.EventID = TEXT("EstablishTrust");
    EstablishTrust.EventDescription = TEXT("Build trust with fellow survivors");
    EstablishTrust.RequiredPhase = ENarr_StoryPhase::TribeContact;
    EstablishTrust.Prerequisites.Add(TEXT("MeetSurvivor"));
    RegisterStoryEvent(EstablishTrust);
}

bool UNarr_StoryManager::ArePrerequisitesMet(const FNarr_StoryEvent& Event) const
{
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!IsStoryEventCompleted(Prerequisite))
        {
            return false;
        }
    }
    return true;
}
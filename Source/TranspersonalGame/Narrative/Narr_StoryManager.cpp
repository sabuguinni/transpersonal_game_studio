#include "Narr_StoryManager.h"
#include "Engine/Engine.h"

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentPhase = ENarr_StoryPhase::Awakening;
    CompletedEventsCount = 0;
    
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Narrative Story Manager initialized - Phase: Awakening"));
}

void UNarr_StoryManager::AdvanceStoryPhase()
{
    if (!CanAdvancePhase())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot advance story phase - prerequisites not met"));
        return;
    }
    
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            CurrentPhase = ENarr_StoryPhase::FirstHunt;
            break;
        case ENarr_StoryPhase::FirstHunt:
            CurrentPhase = ENarr_StoryPhase::TribeContact;
            break;
        case ENarr_StoryPhase::TribeContact:
            CurrentPhase = ENarr_StoryPhase::Survival;
            break;
        case ENarr_StoryPhase::Survival:
            CurrentPhase = ENarr_StoryPhase::Mastery;
            break;
        case ENarr_StoryPhase::Mastery:
            UE_LOG(LogTemp, Warning, TEXT("Story complete - player has mastered survival"));
            return;
    }
    
    FString PhaseDescription = GetCurrentPhaseDescription();
    UE_LOG(LogTemp, Warning, TEXT("Story phase advanced to: %s"), *PhaseDescription);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Story Phase: %s"), *PhaseDescription));
    }
}

void UNarr_StoryManager::CompleteStoryEvent(const FString& EventID)
{
    if (!StoryEvents.Contains(EventID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Story event not found: %s"), *EventID);
        return;
    }
    
    FNarr_StoryEvent& Event = StoryEvents[EventID];
    
    if (Event.bIsCompleted)
    {
        return;
    }
    
    // Check prerequisites
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!IsStoryEventCompleted(Prerequisite))
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot complete event %s - prerequisite %s not met"), 
                *EventID, *Prerequisite);
            return;
        }
    }
    
    Event.bIsCompleted = true;
    CompletedEventsCount++;
    
    UE_LOG(LogTemp, Warning, TEXT("Story event completed: %s - %s"), *EventID, *Event.EventDescription);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, 
            FString::Printf(TEXT("Achievement: %s"), *Event.EventDescription));
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID) const
{
    if (const FNarr_StoryEvent* Event = StoryEvents.Find(EventID))
    {
        return Event->bIsCompleted;
    }
    return false;
}

ENarr_StoryPhase UNarr_StoryManager::GetCurrentStoryPhase() const
{
    return CurrentPhase;
}

FString UNarr_StoryManager::GetCurrentPhaseDescription() const
{
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("Awakening - Learn to survive in the prehistoric world");
        case ENarr_StoryPhase::FirstHunt:
            return TEXT("First Hunt - Prove yourself as a hunter");
        case ENarr_StoryPhase::TribeContact:
            return TEXT("Tribe Contact - Find and join other survivors");
        case ENarr_StoryPhase::Survival:
            return TEXT("Survival - Master the art of staying alive");
        case ENarr_StoryPhase::Mastery:
            return TEXT("Mastery - Become the apex survivor");
        default:
            return TEXT("Unknown Phase");
    }
}

TArray<FString> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FString> AvailableEvents;
    
    for (const auto& EventPair : StoryEvents)
    {
        const FNarr_StoryEvent& Event = EventPair.Value;
        
        if (Event.bIsCompleted || Event.RequiredPhase != CurrentPhase)
        {
            continue;
        }
        
        // Check prerequisites
        bool bCanComplete = true;
        for (const FString& Prerequisite : Event.Prerequisites)
        {
            if (!IsStoryEventCompleted(Prerequisite))
            {
                bCanComplete = false;
                break;
            }
        }
        
        if (bCanComplete)
        {
            AvailableEvents.Add(Event.EventID);
        }
    }
    
    return AvailableEvents;
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    // Awakening Phase Events
    FNarr_StoryEvent WakeUp;
    WakeUp.EventID = TEXT("WakeUp");
    WakeUp.EventDescription = TEXT("Awaken in the prehistoric world");
    WakeUp.RequiredPhase = ENarr_StoryPhase::Awakening;
    StoryEvents.Add(WakeUp.EventID, WakeUp);
    
    FNarr_StoryEvent FirstTool;
    FirstTool.EventID = TEXT("FirstTool");
    FirstTool.EventDescription = TEXT("Craft your first stone tool");
    FirstTool.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstTool.Prerequisites.Add(TEXT("WakeUp"));
    StoryEvents.Add(FirstTool.EventID, FirstTool);
    
    FNarr_StoryEvent FirstShelter;
    FirstShelter.EventID = TEXT("FirstShelter");
    FirstShelter.EventDescription = TEXT("Build basic shelter");
    FirstShelter.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstShelter.Prerequisites.Add(TEXT("FirstTool"));
    StoryEvents.Add(FirstShelter.EventID, FirstShelter);
    
    // First Hunt Phase Events
    FNarr_StoryEvent FirstKill;
    FirstKill.EventID = TEXT("FirstKill");
    FirstKill.EventDescription = TEXT("Successfully hunt your first prey");
    FirstKill.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    FirstKill.Prerequisites.Add(TEXT("FirstShelter"));
    StoryEvents.Add(FirstKill.EventID, FirstKill);
    
    FNarr_StoryEvent DinosaurEncounter;
    DinosaurEncounter.EventID = TEXT("DinosaurEncounter");
    DinosaurEncounter.EventDescription = TEXT("Survive your first dinosaur encounter");
    DinosaurEncounter.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    DinosaurEncounter.Prerequisites.Add(TEXT("FirstKill"));
    StoryEvents.Add(DinosaurEncounter.EventID, DinosaurEncounter);
    
    // Tribe Contact Phase Events
    FNarr_StoryEvent MeetTribe;
    MeetTribe.EventID = TEXT("MeetTribe");
    MeetTribe.EventDescription = TEXT("Make contact with other survivors");
    MeetTribe.RequiredPhase = ENarr_StoryPhase::TribeContact;
    MeetTribe.Prerequisites.Add(TEXT("DinosaurEncounter"));
    StoryEvents.Add(MeetTribe.EventID, MeetTribe);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d story events"), StoryEvents.Num());
}

bool UNarr_StoryManager::CanAdvancePhase() const
{
    int32 RequiredEvents = 0;
    int32 CompletedRequiredEvents = 0;
    
    for (const auto& EventPair : StoryEvents)
    {
        const FNarr_StoryEvent& Event = EventPair.Value;
        
        if (Event.RequiredPhase == CurrentPhase)
        {
            RequiredEvents++;
            if (Event.bIsCompleted)
            {
                CompletedRequiredEvents++;
            }
        }
    }
    
    // Require at least 70% of phase events to be completed
    return RequiredEvents > 0 && (float)CompletedRequiredEvents / RequiredEvents >= 0.7f;
}
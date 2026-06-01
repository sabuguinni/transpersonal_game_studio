#include "Narr_StoryEventManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

void UNarr_StoryEventManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Story Event Manager initialized"));
    
    // Initialize global prerequisites
    GlobalPrerequisites.Empty();
    CompletedEventIDs.Empty();
    ActiveStoryEvents.Empty();
}

void UNarr_StoryEventManager::Tick(float DeltaTime)
{
    ProcessActiveEvents(DeltaTime);
    CheckEventTriggers();
}

void UNarr_StoryEventManager::LoadStoryEventData(UNarr_StoryEventData* EventData)
{
    if (!EventData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to load null story event data"));
        return;
    }

    LoadedStoryEvents = EventData->StoryEvents;
    UE_LOG(LogTemp, Log, TEXT("Loaded %d story events"), LoadedStoryEvents.Num());
}

void UNarr_StoryEventManager::TriggerStoryEvent(const FString& EventID)
{
    for (const FNarr_StoryEvent& Event : LoadedStoryEvents)
    {
        if (Event.EventID == EventID && CanTriggerEvent(EventID))
        {
            FNarr_ActiveStoryEvent NewActiveEvent;
            NewActiveEvent.StoryEvent = Event;
            NewActiveEvent.TimeRemaining = Event.TriggerDelay;
            NewActiveEvent.bIsActive = true;
            NewActiveEvent.LastTriggerTime = GetWorld()->GetTimeSeconds();
            
            ActiveStoryEvents.Add(NewActiveEvent);
            
            UE_LOG(LogTemp, Log, TEXT("Triggered story event: %s"), *Event.EventTitle.ToString());
            OnStoryEventTriggered.Broadcast(Event);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Could not trigger story event: %s"), *EventID);
}

void UNarr_StoryEventManager::CompleteStoryEvent(const FString& EventID)
{
    for (int32 i = ActiveStoryEvents.Num() - 1; i >= 0; i--)
    {
        if (ActiveStoryEvents[i].StoryEvent.EventID == EventID)
        {
            FNarr_StoryEvent CompletedEvent = ActiveStoryEvents[i].StoryEvent;
            ActiveStoryEvents[i].OccurrenceCount++;
            
            // Execute consequences
            ExecuteEventConsequences(CompletedEvent);
            
            // Add to completed events if not repeatable or max occurrences reached
            if (!CompletedEvent.bIsRepeatable || 
                ActiveStoryEvents[i].OccurrenceCount >= CompletedEvent.MaxOccurrences)
            {
                CompletedEventIDs.AddUnique(EventID);
                ActiveStoryEvents.RemoveAt(i);
            }
            else
            {
                // Reset for next occurrence
                ActiveStoryEvents[i].bIsActive = false;
                ActiveStoryEvents[i].TimeRemaining = CompletedEvent.CooldownTime;
            }
            
            UE_LOG(LogTemp, Log, TEXT("Completed story event: %s"), *CompletedEvent.EventTitle.ToString());
            OnStoryEventCompleted.Broadcast(CompletedEvent);
            return;
        }
    }
}

bool UNarr_StoryEventManager::CanTriggerEvent(const FString& EventID)
{
    // Check if event is already completed and not repeatable
    if (CompletedEventIDs.Contains(EventID))
    {
        for (const FNarr_StoryEvent& Event : LoadedStoryEvents)
        {
            if (Event.EventID == EventID && !Event.bIsRepeatable)
            {
                return false;
            }
        }
    }
    
    // Check if event is already active
    for (const FNarr_ActiveStoryEvent& ActiveEvent : ActiveStoryEvents)
    {
        if (ActiveEvent.StoryEvent.EventID == EventID && ActiveEvent.bIsActive)
        {
            return false;
        }
    }
    
    // Find the event and check prerequisites
    for (const FNarr_StoryEvent& Event : LoadedStoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return ArePrerequisitesMet(Event);
        }
    }
    
    return false;
}

TArray<FNarr_StoryEvent> UNarr_StoryEventManager::GetAvailableEvents()
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : LoadedStoryEvents)
    {
        if (CanTriggerEvent(Event.EventID))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

TArray<FNarr_ActiveStoryEvent> UNarr_StoryEventManager::GetActiveEvents()
{
    return ActiveStoryEvents;
}

void UNarr_StoryEventManager::SetEventPrerequisite(const FString& EventID, const FString& Prerequisite)
{
    GlobalPrerequisites.AddUnique(Prerequisite);
    UE_LOG(LogTemp, Log, TEXT("Set prerequisite '%s' for event '%s'"), *Prerequisite, *EventID);
}

void UNarr_StoryEventManager::ClearEventPrerequisite(const FString& EventID, const FString& Prerequisite)
{
    GlobalPrerequisites.Remove(Prerequisite);
    UE_LOG(LogTemp, Log, TEXT("Cleared prerequisite '%s' for event '%s'"), *Prerequisite, *EventID);
}

void UNarr_StoryEventManager::ProcessActiveEvents(float DeltaTime)
{
    for (FNarr_ActiveStoryEvent& ActiveEvent : ActiveStoryEvents)
    {
        if (ActiveEvent.bIsActive)
        {
            ActiveEvent.TimeRemaining -= DeltaTime;
            
            if (ActiveEvent.TimeRemaining <= 0.0f)
            {
                // Event timer expired - auto-complete or trigger next phase
                CompleteStoryEvent(ActiveEvent.StoryEvent.EventID);
            }
        }
        else if (ActiveEvent.TimeRemaining > 0.0f)
        {
            // Cooldown period
            ActiveEvent.TimeRemaining -= DeltaTime;
            if (ActiveEvent.TimeRemaining <= 0.0f)
            {
                ActiveEvent.bIsActive = true;
            }
        }
    }
}

void UNarr_StoryEventManager::CheckEventTriggers()
{
    // Auto-trigger high priority events when conditions are met
    for (const FNarr_StoryEvent& Event : LoadedStoryEvents)
    {
        if (Event.Priority == ENarr_StoryEventPriority::Critical && CanTriggerEvent(Event.EventID))
        {
            TriggerStoryEvent(Event.EventID);
        }
    }
}

bool UNarr_StoryEventManager::ArePrerequisitesMet(const FNarr_StoryEvent& Event)
{
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!GlobalPrerequisites.Contains(Prerequisite) && !CompletedEventIDs.Contains(Prerequisite))
        {
            return false;
        }
    }
    return true;
}

void UNarr_StoryEventManager::ExecuteEventConsequences(const FNarr_StoryEvent& Event)
{
    for (const FString& Consequence : Event.Consequences)
    {
        GlobalPrerequisites.AddUnique(Consequence);
        UE_LOG(LogTemp, Log, TEXT("Event consequence executed: %s"), *Consequence);
    }
}
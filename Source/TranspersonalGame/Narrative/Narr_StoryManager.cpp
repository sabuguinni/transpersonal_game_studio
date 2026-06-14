#include "Narr_StoryManager.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentPhase = ENarr_StoryPhase::Awakening;
    GameStartTime = 0.0f;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    GameStartTime = FPlatformTime::Seconds();
    InitializeStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Story Manager initialized - Beginning Awakening phase"));
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != CurrentPhase)
    {
        ENarr_StoryPhase OldPhase = CurrentPhase;
        CurrentPhase = NewPhase;
        OnPhaseChanged(OldPhase, NewPhase);
        
        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced from %d to %d"), (int32)OldPhase, (int32)NewPhase);
    }
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            Event.CompletionTime = FPlatformTime::Seconds() - GameStartTime;
            
            UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *EventID);
            break;
        }
    }
}

bool UNarr_StoryManager::IsEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bIsCompleted;
        }
    }
    return false;
}

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetActiveEvents() const
{
    TArray<FNarr_StoryEvent> ActiveEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted)
        {
            ActiveEvents.Add(Event);
        }
    }
    
    return ActiveEvents;
}

void UNarr_StoryManager::InitializeStoryEvents()
{
    StoryEvents.Empty();
    
    // Awakening phase events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("first_steps");
    FirstSteps.EventDescription = FText::FromString(TEXT("Take your first steps in the prehistoric world"));
    StoryEvents.Add(FirstSteps);
    
    FNarr_StoryEvent FirstDinosaur;
    FirstDinosaur.EventID = TEXT("first_dinosaur_encounter");
    FirstDinosaur.EventDescription = FText::FromString(TEXT("Encounter your first dinosaur"));
    StoryEvents.Add(FirstDinosaur);
    
    FNarr_StoryEvent FirstTool;
    FirstTool.EventID = TEXT("craft_first_tool");
    FirstTool.EventDescription = FText::FromString(TEXT("Craft your first stone tool"));
    StoryEvents.Add(FirstTool);
    
    // First Hunt phase events
    FNarr_StoryEvent FirstKill;
    FirstKill.EventID = TEXT("first_kill");
    FirstKill.EventDescription = FText::FromString(TEXT("Successfully hunt your first prey"));
    StoryEvents.Add(FirstKill);
    
    FNarr_StoryEvent PredatorEscape;
    PredatorEscape.EventID = TEXT("escape_predator");
    PredatorEscape.EventDescription = FText::FromString(TEXT("Survive an encounter with a large predator"));
    StoryEvents.Add(PredatorEscape);
}

void UNarr_StoryManager::OnPhaseChanged(ENarr_StoryPhase OldPhase, ENarr_StoryPhase NewPhase)
{
    // Handle phase transition logic
    switch (NewPhase)
    {
        case ENarr_StoryPhase::FirstHunt:
            // Unlock hunting-related events
            break;
        case ENarr_StoryPhase::TribalContact:
            // Introduce tribal NPCs
            break;
        case ENarr_StoryPhase::TerritoryWars:
            // Enable territorial conflicts
            break;
        case ENarr_StoryPhase::AlphaStatus:
            // Player becomes apex predator
            break;
    }
}
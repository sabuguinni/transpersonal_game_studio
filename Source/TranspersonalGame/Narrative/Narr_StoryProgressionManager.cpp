#include "Narr_StoryProgressionManager.h"
#include "Engine/Engine.h"

UNarr_StoryProgressionManager::UNarr_StoryProgressionManager()
{
    CurrentPhase = ENarr_StoryPhase::Awakening;
    CompletedEventsCount = 0;
}

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeStoryEvents();
    UE_LOG(LogTemp, Warning, TEXT("Narr_StoryProgressionManager initialized"));
}

void UNarr_StoryProgressionManager::AdvanceStoryPhase()
{
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            CurrentPhase = ENarr_StoryPhase::FirstHunt;
            break;
        case ENarr_StoryPhase::FirstHunt:
            CurrentPhase = ENarr_StoryPhase::TerritoryWars;
            break;
        case ENarr_StoryPhase::TerritoryWars:
            CurrentPhase = ENarr_StoryPhase::AlphaRising;
            break;
        case ENarr_StoryPhase::AlphaRising:
            CurrentPhase = ENarr_StoryPhase::Survival;
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Story phase advanced to: %d"), (int32)CurrentPhase);
}

ENarr_StoryPhase UNarr_StoryProgressionManager::GetCurrentStoryPhase() const
{
    return CurrentPhase;
}

void UNarr_StoryProgressionManager::CompleteStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bCompleted)
        {
            Event.bCompleted = true;
            CompletedEventsCount++;
            UE_LOG(LogTemp, Warning, TEXT("Story event completed: %s"), *EventID);
            break;
        }
    }
}

bool UNarr_StoryProgressionManager::IsStoryEventCompleted(const FString& EventID) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return Event.bCompleted;
        }
    }
    return false;
}

TArray<FNarr_StoryEvent> UNarr_StoryProgressionManager::GetAvailableEvents() const
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bCompleted && Event.RequiredPhase == CurrentPhase && ArePrerequisitesMet(Event))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

float UNarr_StoryProgressionManager::GetStoryProgress() const
{
    if (StoryEvents.Num() == 0)
    {
        return 0.0f;
    }
    
    return (float)CompletedEventsCount / (float)StoryEvents.Num();
}

FString UNarr_StoryProgressionManager::GetCurrentPhaseDescription() const
{
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("You awaken in a dangerous prehistoric world. Learn to survive.");
        case ENarr_StoryPhase::FirstHunt:
            return TEXT("The hunt begins. Prove yourself against the ancient predators.");
        case ENarr_StoryPhase::TerritoryWars:
            return TEXT("Territorial conflicts emerge. Establish your dominance.");
        case ENarr_StoryPhase::AlphaRising:
            return TEXT("Rise to become the apex predator of your domain.");
        case ENarr_StoryPhase::Survival:
            return TEXT("Ultimate survival. Master the prehistoric world.");
        default:
            return TEXT("Unknown phase");
    }
}

void UNarr_StoryProgressionManager::InitializeStoryEvents()
{
    StoryEvents.Empty();

    // Awakening Phase Events
    FNarr_StoryEvent Event1;
    Event1.EventID = TEXT("FirstSteps");
    Event1.EventDescription = TEXT("Take your first steps in the prehistoric world");
    Event1.RequiredPhase = ENarr_StoryPhase::Awakening;
    StoryEvents.Add(Event1);

    FNarr_StoryEvent Event2;
    Event2.EventID = TEXT("FindWater");
    Event2.EventDescription = TEXT("Locate a source of fresh water");
    Event2.RequiredPhase = ENarr_StoryPhase::Awakening;
    StoryEvents.Add(Event2);

    FNarr_StoryEvent Event3;
    Event3.EventID = TEXT("CraftTool");
    Event3.EventDescription = TEXT("Craft your first primitive tool");
    Event3.RequiredPhase = ENarr_StoryPhase::Awakening;
    Event3.PrerequisiteEvents.Add(TEXT("FirstSteps"));
    StoryEvents.Add(Event3);

    // First Hunt Phase Events
    FNarr_StoryEvent Event4;
    Event4.EventID = TEXT("FirstKill");
    Event4.EventDescription = TEXT("Successfully hunt your first prey");
    Event4.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    Event4.PrerequisiteEvents.Add(TEXT("CraftTool"));
    StoryEvents.Add(Event4);

    FNarr_StoryEvent Event5;
    Event5.EventID = TEXT("AvoidPredator");
    Event5.EventDescription = TEXT("Survive an encounter with a large predator");
    Event5.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    StoryEvents.Add(Event5);

    // Territory Wars Phase Events
    FNarr_StoryEvent Event6;
    Event6.EventID = TEXT("ClaimTerritory");
    Event6.EventDescription = TEXT("Establish your hunting territory");
    Event6.RequiredPhase = ENarr_StoryPhase::TerritoryWars;
    Event6.PrerequisiteEvents.Add(TEXT("FirstKill"));
    StoryEvents.Add(Event6);

    FNarr_StoryEvent Event7;
    Event7.EventID = TEXT("DefendTerritory");
    Event7.EventDescription = TEXT("Successfully defend your territory from intruders");
    Event7.RequiredPhase = ENarr_StoryPhase::TerritoryWars;
    Event7.PrerequisiteEvents.Add(TEXT("ClaimTerritory"));
    StoryEvents.Add(Event7);

    // Alpha Rising Phase Events
    FNarr_StoryEvent Event8;
    Event8.EventID = TEXT("DefeatAlpha");
    Event8.EventDescription = TEXT("Challenge and defeat an alpha predator");
    Event8.RequiredPhase = ENarr_StoryPhase::AlphaRising;
    Event8.PrerequisiteEvents.Add(TEXT("DefendTerritory"));
    StoryEvents.Add(Event8);

    // Survival Phase Events
    FNarr_StoryEvent Event9;
    Event9.EventID = TEXT("MasterSurvival");
    Event9.EventDescription = TEXT("Achieve mastery over the prehistoric world");
    Event9.RequiredPhase = ENarr_StoryPhase::Survival;
    Event9.PrerequisiteEvents.Add(TEXT("DefeatAlpha"));
    StoryEvents.Add(Event9);

    UE_LOG(LogTemp, Warning, TEXT("Initialized %d story events"), StoryEvents.Num());
}

bool UNarr_StoryProgressionManager::ArePrerequisitesMet(const FNarr_StoryEvent& Event) const
{
    for (const FString& PrereqID : Event.PrerequisiteEvents)
    {
        if (!IsStoryEventCompleted(PrereqID))
        {
            return false;
        }
    }
    return true;
}
#include "Narr_StoryManager.h"
#include "Engine/Engine.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    bStorySystemActive = true;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultStoryEvents();
    UE_LOG(LogTemp, Log, TEXT("Story Manager initialized with %d story events"), StoryEvents.Num());
}

bool UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    if (!bStorySystemActive)
    {
        return false;
    }
    
    FNarr_StoryEvent* Event = FindStoryEvent(EventID);
    if (!Event)
    {
        UE_LOG(LogTemp, Warning, TEXT("Story event not found: %s"), *EventID);
        return false;
    }
    
    if (Event->bIsCompleted)
    {
        UE_LOG(LogTemp, Log, TEXT("Story event already completed: %s"), *EventID);
        return false;
    }
    
    if (!CheckEventPrerequisites(*Event))
    {
        UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for story event: %s"), *EventID);
        return false;
    }
    
    if (!CurrentProgress.ActiveEvents.Contains(EventID))
    {
        CurrentProgress.ActiveEvents.Add(EventID);
        UE_LOG(LogTemp, Log, TEXT("Triggered story event: %s"), *EventID);
    }
    
    return true;
}

bool UNarr_StoryManager::CompleteStoryEvent(const FString& EventID)
{
    FNarr_StoryEvent* Event = FindStoryEvent(EventID);
    if (!Event)
    {
        return false;
    }
    
    if (Event->bIsCompleted)
    {
        return true;
    }
    
    Event->bIsCompleted = true;
    CurrentProgress.CompletedEvents.Add(EventID);
    CurrentProgress.ActiveEvents.Remove(EventID);
    
    UnlockFollowupEvents(*Event);
    
    UE_LOG(LogTemp, Log, TEXT("Completed story event: %s"), *EventID);
    return true;
}

bool UNarr_StoryManager::IsEventCompleted(const FString& EventID) const
{
    return CurrentProgress.CompletedEvents.Contains(EventID);
}

bool UNarr_StoryManager::IsEventActive(const FString& EventID) const
{
    return CurrentProgress.ActiveEvents.Contains(EventID);
}

TArray<FString> UNarr_StoryManager::GetActiveEvents() const
{
    return CurrentProgress.ActiveEvents;
}

TArray<FString> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FString> Available;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted && CheckEventPrerequisites(Event))
        {
            Available.Add(Event.EventID);
        }
    }
    
    return Available;
}

void UNarr_StoryManager::AdvanceChapter()
{
    CurrentProgress.CurrentChapter++;
    UE_LOG(LogTemp, Log, TEXT("Advanced to story chapter: %d"), CurrentProgress.CurrentChapter);
}

void UNarr_StoryManager::UpdateSurvivalStats(float DaysElapsed, int32 NewEncounters)
{
    CurrentProgress.SurvivalDays += DaysElapsed;
    CurrentProgress.DinosaurEncounters += NewEncounters;
    
    // Check for milestone triggers
    if (CurrentProgress.SurvivalDays >= 1.0f && !IsEventCompleted("FirstDayComplete"))
    {
        TriggerStoryEvent("FirstDayComplete");
    }
    
    if (CurrentProgress.DinosaurEncounters >= 5 && !IsEventCompleted("ExperiencedSurvivor"))
    {
        TriggerStoryEvent("ExperiencedSurvivor");
    }
}

void UNarr_StoryManager::SetMilestone(const FString& MilestoneName, bool bCompleted)
{
    if (MilestoneName == TEXT("FirstHunt"))
    {
        CurrentProgress.bFirstHuntCompleted = bCompleted;
        if (bCompleted)
        {
            TriggerStoryEvent("FirstHuntComplete");
        }
    }
    else if (MilestoneName == TEXT("SafeHaven"))
    {
        CurrentProgress.bSafeHavenFound = bCompleted;
        if (bCompleted)
        {
            TriggerStoryEvent("SafeHavenFound");
        }
    }
}

bool UNarr_StoryManager::GetMilestone(const FString& MilestoneName) const
{
    if (MilestoneName == TEXT("FirstHunt"))
    {
        return CurrentProgress.bFirstHuntCompleted;
    }
    else if (MilestoneName == TEXT("SafeHaven"))
    {
        return CurrentProgress.bSafeHavenFound;
    }
    
    return false;
}

void UNarr_StoryManager::AddStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    // Check if event already exists
    for (int32 i = 0; i < StoryEvents.Num(); i++)
    {
        if (StoryEvents[i].EventID == NewEvent.EventID)
        {
            StoryEvents[i] = NewEvent;
            return;
        }
    }
    
    StoryEvents.Add(NewEvent);
}

FText UNarr_StoryManager::GetChapterTitle() const
{
    switch (CurrentProgress.CurrentChapter)
    {
        case 1:
            return FText::FromString(TEXT("Chapter 1: First Steps"));
        case 2:
            return FText::FromString(TEXT("Chapter 2: The Hunt Begins"));
        case 3:
            return FText::FromString(TEXT("Chapter 3: Ancient Grounds"));
        default:
            return FText::FromString(TEXT("Chapter: Unknown"));
    }
}

FText UNarr_StoryManager::GetChapterDescription() const
{
    switch (CurrentProgress.CurrentChapter)
    {
        case 1:
            return FText::FromString(TEXT("Learn to survive in the prehistoric world. Find food, water, and shelter."));
        case 2:
            return FText::FromString(TEXT("Master the art of hunting. Face your first dinosaur encounters."));
        case 3:
            return FText::FromString(TEXT("Explore the ancient hunting grounds. Uncover the secrets of the past."));
        default:
            return FText::FromString(TEXT("Continue your journey of survival."));
    }
}

FNarr_StoryEvent* UNarr_StoryManager::FindStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            return &Event;
        }
    }
    return nullptr;
}

bool UNarr_StoryManager::CheckEventPrerequisites(const FNarr_StoryEvent& Event) const
{
    for (const FString& Prerequisite : Event.Prerequisites)
    {
        if (!IsEventCompleted(Prerequisite))
        {
            return false;
        }
    }
    return true;
}

void UNarr_StoryManager::UnlockFollowupEvents(const FNarr_StoryEvent& CompletedEvent)
{
    for (const FString& UnlockedEventID : CompletedEvent.UnlockedEvents)
    {
        TriggerStoryEvent(UnlockedEventID);
    }
}

void UNarr_StoryManager::InitializeDefaultStoryEvents()
{
    // Chapter 1 Events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("FirstSteps");
    FirstSteps.EventTitle = FText::FromString(TEXT("First Steps"));
    FirstSteps.EventDescription = FText::FromString(TEXT("Take your first steps into the prehistoric world."));
    FirstSteps.StoryChapter = 1;
    FirstSteps.UnlockedEvents.Add(TEXT("FindWater"));
    StoryEvents.Add(FirstSteps);
    
    FNarr_StoryEvent FindWater;
    FindWater.EventID = TEXT("FindWater");
    FindWater.EventTitle = FText::FromString(TEXT("Find Water"));
    FindWater.EventDescription = FText::FromString(TEXT("Locate a clean water source to survive."));
    FindWater.StoryChapter = 1;
    FindWater.Prerequisites.Add(TEXT("FirstSteps"));
    FindWater.UnlockedEvents.Add(TEXT("FirstDayComplete"));
    StoryEvents.Add(FindWater);
    
    FNarr_StoryEvent FirstDayComplete;
    FirstDayComplete.EventID = TEXT("FirstDayComplete");
    FirstDayComplete.EventTitle = FText::FromString(TEXT("First Day Survived"));
    FirstDayComplete.EventDescription = FText::FromString(TEXT("You have survived your first day in the prehistoric world."));
    FirstDayComplete.StoryChapter = 1;
    FirstDayComplete.Prerequisites.Add(TEXT("FindWater"));
    FirstDayComplete.UnlockedEvents.Add(TEXT("FirstHuntComplete"));
    StoryEvents.Add(FirstDayComplete);
    
    // Chapter 2 Events
    FNarr_StoryEvent FirstHuntComplete;
    FirstHuntComplete.EventID = TEXT("FirstHuntComplete");
    FirstHuntComplete.EventTitle = FText::FromString(TEXT("First Hunt"));
    FirstHuntComplete.EventDescription = FText::FromString(TEXT("Successfully complete your first hunt for food."));
    FirstHuntComplete.StoryChapter = 2;
    FirstHuntComplete.Prerequisites.Add(TEXT("FirstDayComplete"));
    FirstHuntComplete.UnlockedEvents.Add(TEXT("DinosaurEncounter"));
    StoryEvents.Add(FirstHuntComplete);
    
    FNarr_StoryEvent DinosaurEncounter;
    DinosaurEncounter.EventID = TEXT("DinosaurEncounter");
    DinosaurEncounter.EventTitle = FText::FromString(TEXT("Dinosaur Encounter"));
    DinosaurEncounter.EventDescription = FText::FromString(TEXT("Face your first dinosaur encounter and survive."));
    DinosaurEncounter.StoryChapter = 2;
    DinosaurEncounter.Prerequisites.Add(TEXT("FirstHuntComplete"));
    DinosaurEncounter.UnlockedEvents.Add(TEXT("SafeHavenFound"));
    StoryEvents.Add(DinosaurEncounter);
    
    // Chapter 3 Events
    FNarr_StoryEvent SafeHavenFound;
    SafeHavenFound.EventID = TEXT("SafeHavenFound");
    SafeHavenFound.EventTitle = FText::FromString(TEXT("Safe Haven"));
    SafeHavenFound.EventDescription = FText::FromString(TEXT("Discover a safe place to establish your base."));
    SafeHavenFound.StoryChapter = 3;
    SafeHavenFound.Prerequisites.Add(TEXT("DinosaurEncounter"));
    SafeHavenFound.UnlockedEvents.Add(TEXT("ExperiencedSurvivor"));
    StoryEvents.Add(SafeHavenFound);
    
    FNarr_StoryEvent ExperiencedSurvivor;
    ExperiencedSurvivor.EventID = TEXT("ExperiencedSurvivor");
    ExperiencedSurvivor.EventTitle = FText::FromString(TEXT("Experienced Survivor"));
    ExperiencedSurvivor.EventDescription = FText::FromString(TEXT("Become an experienced survivor of the prehistoric world."));
    ExperiencedSurvivor.StoryChapter = 3;
    ExperiencedSurvivor.Prerequisites.Add(TEXT("SafeHavenFound"));
    StoryEvents.Add(ExperiencedSurvivor);
    
    // Trigger initial event
    TriggerStoryEvent("FirstSteps");
}
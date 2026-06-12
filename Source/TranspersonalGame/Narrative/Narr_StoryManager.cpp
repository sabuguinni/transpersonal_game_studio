#include "Narr_StoryManager.h"
#include "Engine/Engine.h"

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    CurrentChapter = ENarr_StoryChapter::Awakening;
    
    InitializeDefaultStoryEvents();
    
    UE_LOG(LogTemp, Log, TEXT("Story Manager initialized - Starting Chapter: Awakening"));
}

void UNarr_StoryManager::AdvanceToChapter(ENarr_StoryChapter NewChapter)
{
    if (NewChapter != CurrentChapter)
    {
        ENarr_StoryChapter PreviousChapter = CurrentChapter;
        CurrentChapter = NewChapter;
        
        BroadcastChapterChange(NewChapter);
        
        UE_LOG(LogTemp, Log, TEXT("Story advanced from %s to %s"), 
               *GetChapterDescription(PreviousChapter), 
               *GetChapterDescription(NewChapter));
    }
}

ENarr_StoryChapter UNarr_StoryManager::GetCurrentChapter() const
{
    return CurrentChapter;
}

void UNarr_StoryManager::CompleteStoryEvent(const FString& EventID)
{
    // Find and mark event as completed
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID)
        {
            Event.bIsCompleted = true;
            CompletedEvents.AddUnique(EventID);
            
            UE_LOG(LogTemp, Log, TEXT("Story event completed: %s"), *EventID);
            
            // Check if this completion should trigger chapter advancement
            if (EventID == TEXT("FirstDinosaurEncounter") && CurrentChapter == ENarr_StoryChapter::Awakening)
            {
                AdvanceToChapter(ENarr_StoryChapter::Discovery);
            }
            else if (EventID == TEXT("CraftFirstTool") && CurrentChapter == ENarr_StoryChapter::Discovery)
            {
                AdvanceToChapter(ENarr_StoryChapter::Adaptation);
            }
            
            break;
        }
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID) const
{
    return CompletedEvents.Contains(EventID);
}

void UNarr_StoryManager::RegisterStoryEvent(const FNarr_StoryEvent& NewEvent)
{
    StoryEvents.Add(NewEvent);
    UE_LOG(LogTemp, Log, TEXT("Registered story event: %s"), *NewEvent.EventID);
}

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted && 
            Event.RequiredChapter <= CurrentChapter && 
            ArePrerequisitesMet(Event))
        {
            AvailableEvents.Add(Event);
        }
    }
    
    return AvailableEvents;
}

void UNarr_StoryManager::TriggerNarrativeEvent(const FString& EventName)
{
    UE_LOG(LogTemp, Log, TEXT("Narrative event triggered: %s"), *EventName);
    
    // Display narrative message
    if (GEngine)
    {
        FString NarrativeText = FString::Printf(TEXT("NARRATIVE: %s"), *EventName);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, NarrativeText);
    }
}

FString UNarr_StoryManager::GetChapterDescription(ENarr_StoryChapter Chapter) const
{
    switch (Chapter)
    {
        case ENarr_StoryChapter::Awakening:
            return TEXT("Awakening - First steps in a dangerous world");
        case ENarr_StoryChapter::Discovery:
            return TEXT("Discovery - Learning the rules of survival");
        case ENarr_StoryChapter::Adaptation:
            return TEXT("Adaptation - Mastering primitive tools");
        case ENarr_StoryChapter::Confrontation:
            return TEXT("Confrontation - Facing apex predators");
        case ENarr_StoryChapter::Mastery:
            return TEXT("Mastery - Becoming the ultimate survivor");
        case ENarr_StoryChapter::Legacy:
            return TEXT("Legacy - Shaping the future");
        default:
            return TEXT("Unknown Chapter");
    }
}

void UNarr_StoryManager::InitializeDefaultStoryEvents()
{
    // Awakening Chapter Events
    FNarr_StoryEvent FirstSteps;
    FirstSteps.EventID = TEXT("FirstSteps");
    FirstSteps.EventDescription = TEXT("Take your first steps in the prehistoric world");
    FirstSteps.RequiredChapter = ENarr_StoryChapter::Awakening;
    RegisterStoryEvent(FirstSteps);
    
    FNarr_StoryEvent FirstDinosaurEncounter;
    FirstDinosaurEncounter.EventID = TEXT("FirstDinosaurEncounter");
    FirstDinosaurEncounter.EventDescription = TEXT("Encounter your first dinosaur");
    FirstDinosaurEncounter.RequiredChapter = ENarr_StoryChapter::Awakening;
    FirstDinosaurEncounter.Prerequisites.Add(TEXT("FirstSteps"));
    RegisterStoryEvent(FirstDinosaurEncounter);
    
    // Discovery Chapter Events
    FNarr_StoryEvent ExploreTerritory;
    ExploreTerritory.EventID = TEXT("ExploreTerritory");
    ExploreTerritory.EventDescription = TEXT("Explore different biomes and territories");
    ExploreTerritory.RequiredChapter = ENarr_StoryChapter::Discovery;
    RegisterStoryEvent(ExploreTerritory);
    
    // Adaptation Chapter Events
    FNarr_StoryEvent CraftFirstTool;
    CraftFirstTool.EventID = TEXT("CraftFirstTool");
    CraftFirstTool.EventDescription = TEXT("Craft your first primitive tool");
    CraftFirstTool.RequiredChapter = ENarr_StoryChapter::Discovery;
    RegisterStoryEvent(CraftFirstTool);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d default story events"), StoryEvents.Num());
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

void UNarr_StoryManager::BroadcastChapterChange(ENarr_StoryChapter NewChapter)
{
    // Display chapter change message
    if (GEngine)
    {
        FString ChapterText = FString::Printf(TEXT("CHAPTER: %s"), *GetChapterDescription(NewChapter));
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Gold, ChapterText);
    }
    
    // Trigger narrative event for chapter change
    FString EventName = FString::Printf(TEXT("ChapterAdvanced_%s"), *GetChapterDescription(NewChapter));
    TriggerNarrativeEvent(EventName);
}
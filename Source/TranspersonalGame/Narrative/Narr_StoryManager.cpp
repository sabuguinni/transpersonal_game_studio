#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeChapterTitles();
    
    // Initialize default story state
    StoryState = FNarr_StoryState();
    
    UE_LOG(LogTemp, Log, TEXT("Story Manager initialized - Starting Chapter 1"));
}

void UNarr_StoryManager::InitializeChapterTitles()
{
    ChapterTitles.Empty();
    ChapterTitles.Add(1, TEXT("The Awakening"));
    ChapterTitles.Add(2, TEXT("First Hunt"));
    ChapterTitles.Add(3, TEXT("Valley of Shadows"));
    ChapterTitles.Add(4, TEXT("Ancient Secrets"));
    ChapterTitles.Add(5, TEXT("The Great Migration"));
    ChapterTitles.Add(6, TEXT("Predator's Domain"));
    ChapterTitles.Add(7, TEXT("Survival's Edge"));
}

void UNarr_StoryManager::AdvanceStory(const FString& EventName)
{
    if (EventName == TEXT("MeetElderThok"))
    {
        StoryState.bHasMetElderThok = true;
        StartQuest(TEXT("FirstHunt"));
        UE_LOG(LogTemp, Log, TEXT("Story Event: Met Elder Thok - First Hunt quest started"));
    }
    else if (EventName == TEXT("EnterValley"))
    {
        StoryState.bHasEnteredValley = true;
        if (StoryState.CurrentChapter < 3)
        {
            StoryState.CurrentChapter = 3;
            UE_LOG(LogTemp, Log, TEXT("Story Advanced: Entered Valley - Chapter 3"));
        }
    }
    else if (EventName == TEXT("FindAncientRuins"))
    {
        StoryState.bHasFoundAncientRuins = true;
        if (StoryState.CurrentChapter < 4)
        {
            StoryState.CurrentChapter = 4;
            StartQuest(TEXT("AncientSecrets"));
            UE_LOG(LogTemp, Log, TEXT("Story Advanced: Found Ancient Ruins - Chapter 4"));
        }
    }
    else if (EventName == TEXT("DinosaurKilled"))
    {
        StoryState.DinosaurKillCount++;
        if (StoryState.DinosaurKillCount >= 1 && IsQuestActive(TEXT("FirstHunt")))
        {
            CompleteQuest(TEXT("FirstHunt"));
            StoryState.CurrentChapter = 2;
            UE_LOG(LogTemp, Log, TEXT("Story Advanced: First Hunt completed - Chapter 2"));
        }
    }
}

bool UNarr_StoryManager::IsQuestActive(const FString& QuestName) const
{
    return StoryState.ActiveQuests.Contains(QuestName);
}

bool UNarr_StoryManager::IsQuestCompleted(const FString& QuestName) const
{
    return StoryState.CompletedQuests.Contains(QuestName);
}

void UNarr_StoryManager::CompleteQuest(const FString& QuestName)
{
    if (IsQuestActive(QuestName))
    {
        StoryState.ActiveQuests.Remove(QuestName);
        StoryState.CompletedQuests.AddUnique(QuestName);
        
        UE_LOG(LogTemp, Log, TEXT("Quest Completed: %s"), *QuestName);
        
        // Trigger completion events
        if (QuestName == TEXT("FirstHunt"))
        {
            StartQuest(TEXT("ExploreValley"));
        }
        else if (QuestName == TEXT("AncientSecrets"))
        {
            StartQuest(TEXT("SurvivalMastery"));
        }
    }
}

void UNarr_StoryManager::StartQuest(const FString& QuestName)
{
    if (!IsQuestActive(QuestName) && !IsQuestCompleted(QuestName))
    {
        StoryState.ActiveQuests.AddUnique(QuestName);
        UE_LOG(LogTemp, Log, TEXT("Quest Started: %s"), *QuestName);
    }
}

FString UNarr_StoryManager::GetCurrentChapterTitle() const
{
    if (const FString* Title = ChapterTitles.Find(StoryState.CurrentChapter))
    {
        return *Title;
    }
    return TEXT("Unknown Chapter");
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventName, const FVector& Location)
{
    HandleStoryTrigger(EventName, Location);
    AdvanceStory(EventName);
}

void UNarr_StoryManager::HandleStoryTrigger(const FString& TriggerName, const FVector& Location)
{
    if (TriggerName == TEXT("Story_Trigger_Valley_Entrance"))
    {
        AdvanceStory(TEXT("EnterValley"));
    }
    else if (TriggerName == TEXT("Story_Trigger_Ancient_Ruins"))
    {
        AdvanceStory(TEXT("FindAncientRuins"));
    }
    else if (TriggerName == TEXT("Story_Trigger_Predator_Territory"))
    {
        if (StoryState.CurrentChapter < 6)
        {
            StoryState.CurrentChapter = 6;
            StartQuest(TEXT("PredatorChallenge"));
            UE_LOG(LogTemp, Log, TEXT("Story Advanced: Entered Predator Territory - Chapter 6"));
        }
    }
}
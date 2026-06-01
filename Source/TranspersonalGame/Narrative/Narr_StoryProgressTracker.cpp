#include "Narr_StoryProgressTracker.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UNarr_StoryProgressTracker::UNarr_StoryProgressTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Tick every second
    
    CurrentChapter = TEXT("Chapter1_FirstSteps");
    CurrentDay = 1;
    DayStartTime = 0.0f;
    
    // Initialize default story events
    FNarr_StoryEvent firstKill;
    firstKill.EventID = TEXT("first_dinosaur_kill");
    firstKill.EventDescription = TEXT("Killed your first dinosaur");
    firstKill.UnlockedSequences.Add(TEXT("elder_congratulation"));
    StoryEvents.Add(firstKill);
    
    FNarr_StoryEvent firstNight;
    firstNight.EventID = TEXT("survive_first_night");
    firstNight.EventDescription = TEXT("Survived the first night");
    firstNight.UnlockedSequences.Add(TEXT("dawn_relief"));
    StoryEvents.Add(firstNight);
    
    FNarr_StoryEvent findCave;
    findCave.EventID = TEXT("discover_safe_cave");
    findCave.EventDescription = TEXT("Found a safe shelter");
    findCave.UnlockedSequences.Add(TEXT("cave_exploration"));
    StoryEvents.Add(findCave);
}

void UNarr_StoryProgressTracker::BeginPlay()
{
    Super::BeginPlay();
    
    DayStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Story Progress Tracker initialized - Day %d"), CurrentDay);
}

void UNarr_StoryProgressTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    PlayerProgress.TotalPlayTime += DeltaTime;
    UpdateDayCounter(DeltaTime);
    CheckStoryTriggers();
}

void UNarr_StoryProgressTracker::CompleteStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            Event.CompletionTime = PlayerProgress.TotalPlayTime;
            
            PlayerProgress.CompletedEvents.Add(EventID);
            
            // Unlock associated dialogues
            for (const FString& DialogueID : Event.UnlockedSequences)
            {
                UnlockDialogue(DialogueID);
            }
            
            UE_LOG(LogTemp, Log, TEXT("Story event completed: %s"), *EventID);
            break;
        }
    }
}

bool UNarr_StoryProgressTracker::IsEventCompleted(const FString& EventID) const
{
    return PlayerProgress.CompletedEvents.Contains(EventID);
}

void UNarr_StoryProgressTracker::UnlockDialogue(const FString& DialogueID)
{
    if (!PlayerProgress.UnlockedDialogues.Contains(DialogueID))
    {
        PlayerProgress.UnlockedDialogues.Add(DialogueID);
        UE_LOG(LogTemp, Log, TEXT("Dialogue unlocked: %s"), *DialogueID);
    }
}

bool UNarr_StoryProgressTracker::IsDialogueUnlocked(const FString& DialogueID) const
{
    return PlayerProgress.UnlockedDialogues.Contains(DialogueID);
}

void UNarr_StoryProgressTracker::DiscoverLocation(const FString& LocationName)
{
    if (!PlayerProgress.DiscoveredLocations.Contains(LocationName))
    {
        PlayerProgress.DiscoveredLocations.Add(LocationName);
        UE_LOG(LogTemp, Log, TEXT("Location discovered: %s"), *LocationName);
        
        // Check for location-based story events
        if (LocationName == TEXT("SafeCave"))
        {
            CompleteStoryEvent(TEXT("discover_safe_cave"));
        }
    }
}

void UNarr_StoryProgressTracker::RecordDinosaurKill(const FString& DinosaurType)
{
    PlayerProgress.DinosaurKills++;
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur killed: %s (Total: %d)"), *DinosaurType, PlayerProgress.DinosaurKills);
    
    // Check for first kill event
    if (PlayerProgress.DinosaurKills == 1)
    {
        CompleteStoryEvent(TEXT("first_dinosaur_kill"));
    }
    
    // Chapter progression based on kills
    if (PlayerProgress.DinosaurKills >= 5 && CurrentChapter == TEXT("Chapter1_FirstSteps"))
    {
        CurrentChapter = TEXT("Chapter2_Hunter");
        UE_LOG(LogTemp, Log, TEXT("Chapter advanced to: %s"), *CurrentChapter);
    }
}

void UNarr_StoryProgressTracker::AdvanceDay()
{
    CurrentDay++;
    PlayerProgress.DaysAlive = CurrentDay;
    DayStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Day advanced to: %d"), CurrentDay);
    
    // Check survival milestones
    if (CurrentDay == 2)
    {
        CompleteStoryEvent(TEXT("survive_first_night"));
    }
}

TArray<FString> UNarr_StoryProgressTracker::GetAvailableDialogues() const
{
    return PlayerProgress.UnlockedDialogues;
}

FString UNarr_StoryProgressTracker::GetProgressSummary() const
{
    FString Summary = FString::Printf(TEXT("Chapter: %s | Day: %d | Kills: %d | Locations: %d"), 
        *CurrentChapter, CurrentDay, PlayerProgress.DinosaurKills, PlayerProgress.DiscoveredLocations.Num());
    
    return Summary;
}

void UNarr_StoryProgressTracker::SaveProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Progress saved: %s"), *GetProgressSummary());
}

void UNarr_StoryProgressTracker::LoadProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Progress loaded"));
}

void UNarr_StoryProgressTracker::UpdateDayCounter(float DeltaTime)
{
    // Advance day every 10 minutes of real time (600 seconds)
    float DayLength = 600.0f;
    
    if (GetWorld()->GetTimeSeconds() - DayStartTime >= DayLength)
    {
        AdvanceDay();
    }
}

void UNarr_StoryProgressTracker::CheckStoryTriggers()
{
    // Auto-trigger story events based on conditions
    if (PlayerProgress.TotalPlayTime >= 300.0f && !IsEventCompleted(TEXT("survive_first_night")))
    {
        // Player has survived 5 minutes, count as first night
        CompleteStoryEvent(TEXT("survive_first_night"));
    }
}
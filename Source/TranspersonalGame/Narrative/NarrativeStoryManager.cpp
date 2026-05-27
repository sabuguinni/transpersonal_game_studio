#include "NarrativeStoryManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

UNarrativeStoryManager::UNarrativeStoryManager()
{
    // Initialize narrative context to starting state
    NarrativeContext = FNarr_NarrativeContext();
}

void UNarrativeStoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeStoryData();
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Initialized - Starting story phase: Awakening"));
}

void UNarrativeStoryManager::Deinitialize()
{
    StoryEvents.Empty();
    
    Super::Deinitialize();
}

void UNarrativeStoryManager::InitializeStoryData()
{
    // Initialize phase descriptions for contextual narration
    PhaseDescriptions.Empty();
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Awakening, 
        TEXT("You awaken in an unfamiliar world. The great cataclysm has separated you from your tribe."));
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Survival, 
        TEXT("Your immediate needs are clear: water, food, and shelter. The predators will not wait."));
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Discovery, 
        TEXT("Signs of other survivors emerge. You are not alone in this ancient land."));
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Territory, 
        TEXT("You have claimed your territory. Now you must defend it from those who would take it."));
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Mastery, 
        TEXT("You have become a master of survival. The land bends to your will."));
    
    PhaseDescriptions.Add(ENarr_StoryPhase::Endgame, 
        TEXT("The final challenge awaits. Your journey through this prehistoric world nears its conclusion."));

    // Record the awakening event
    RecordStoryEvent(ENarr_StoryEventType::FirstWater, 
        TEXT("Player awakens in prehistoric world"), 
        FVector::ZeroVector);
}

void UNarrativeStoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != NarrativeContext.CurrentPhase)
    {
        ENarr_StoryPhase OldPhase = NarrativeContext.CurrentPhase;
        NarrativeContext.CurrentPhase = NewPhase;
        
        UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Story phase advanced from %d to %d"), 
            (int32)OldPhase, (int32)NewPhase);
        
        // Trigger contextual narration for phase change
        FString PhaseNarration = GetPhaseNarration(NewPhase);
        UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Phase narration: %s"), *PhaseNarration);
    }
}

void UNarrativeStoryManager::RecordStoryEvent(ENarr_StoryEventType EventType, const FString& Description, const FVector& Location)
{
    FNarr_StoryEvent NewEvent;
    NewEvent.EventType = EventType;
    NewEvent.EventDescription = Description;
    NewEvent.EventTime = FDateTime::Now();
    NewEvent.EventLocation = Location;
    
    StoryEvents.Add(NewEvent);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Story event recorded - %s at %s"), 
        *Description, *Location.ToString());
    
    // Check for story phase advancement based on events
    switch (EventType)
    {
        case ENarr_StoryEventType::FirstWater:
            if (NarrativeContext.CurrentPhase == ENarr_StoryPhase::Awakening)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::Survival);
            }
            break;
            
        case ENarr_StoryEventType::FirstShelter:
            if (NarrativeContext.bHasWater && NarrativeContext.AnimalsHunted > 0)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::Discovery);
            }
            break;
            
        case ENarr_StoryEventType::FirstTribe:
            AdvanceStoryPhase(ENarr_StoryPhase::Territory);
            break;
            
        case ENarr_StoryEventType::MajorVictory:
            if (NarrativeContext.PredatorsDefeated >= 3)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::Mastery);
            }
            break;
    }
}

bool UNarrativeStoryManager::HasEventOccurred(ENarr_StoryEventType EventType) const
{
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventType == EventType)
        {
            return true;
        }
    }
    return false;
}

void UNarrativeStoryManager::UpdateSurvivalStats(bool bFoundWater, bool bBuiltShelter, int32 NewAnimalsHunted)
{
    bool bWaterChanged = (bFoundWater != NarrativeContext.bHasWater);
    bool bShelterChanged = (bBuiltShelter != NarrativeContext.bHasShelter);
    
    NarrativeContext.bHasWater = bFoundWater;
    NarrativeContext.bHasShelter = bBuiltShelter;
    NarrativeContext.AnimalsHunted = NewAnimalsHunted;
    
    // Record significant survival milestones
    if (bWaterChanged && bFoundWater && !HasEventOccurred(ENarr_StoryEventType::FirstWater))
    {
        RecordStoryEvent(ENarr_StoryEventType::FirstWater, 
            TEXT("Player found first water source"), 
            FVector::ZeroVector);
    }
    
    if (bShelterChanged && bBuiltShelter && !HasEventOccurred(ENarr_StoryEventType::FirstShelter))
    {
        RecordStoryEvent(ENarr_StoryEventType::FirstShelter, 
            TEXT("Player built first shelter"), 
            FVector::ZeroVector);
    }
    
    if (NewAnimalsHunted > 0 && !HasEventOccurred(ENarr_StoryEventType::FirstHunt))
    {
        RecordStoryEvent(ENarr_StoryEventType::FirstHunt, 
            TEXT("Player completed first successful hunt"), 
            FVector::ZeroVector);
    }
}

void UNarrativeStoryManager::UpdateFearLevel(float NewFearLevel)
{
    NarrativeContext.FearLevel = FMath::Clamp(NewFearLevel, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Fear level updated to %f"), 
        NarrativeContext.FearLevel);
}

void UNarrativeStoryManager::IncrementDaysAlive()
{
    NarrativeContext.DaysAlive++;
    
    UE_LOG(LogTemp, Warning, TEXT("NarrativeStoryManager: Player has survived %d days"), 
        NarrativeContext.DaysAlive);
    
    // Check for long-term survival milestones
    if (NarrativeContext.DaysAlive == 7)
    {
        RecordStoryEvent(ENarr_StoryEventType::MajorVictory, 
            TEXT("Player survived first week"), 
            FVector::ZeroVector);
    }
    else if (NarrativeContext.DaysAlive == 30)
    {
        RecordStoryEvent(ENarr_StoryEventType::MajorVictory, 
            TEXT("Player survived first month"), 
            FVector::ZeroVector);
    }
}

FString UNarrativeStoryManager::GetContextualNarration() const
{
    // Generate contextual narration based on current story state
    FString BaseNarration = GetPhaseNarration(NarrativeContext.CurrentPhase);
    
    // Add context based on survival status
    if (!NarrativeContext.bHasWater && NarrativeContext.DaysAlive > 0)
    {
        BaseNarration += TEXT(" Your thirst grows desperate.");
    }
    
    if (!NarrativeContext.bHasShelter && NarrativeContext.DaysAlive > 1)
    {
        BaseNarration += TEXT(" The elements threaten your survival.");
    }
    
    if (NarrativeContext.FearLevel > 0.8f)
    {
        BaseNarration += TEXT(" Terror grips your heart as predators circle.");
    }
    else if (NarrativeContext.FearLevel < 0.2f && NarrativeContext.PredatorsDefeated > 2)
    {
        BaseNarration += TEXT(" You have become the apex predator of these lands.");
    }
    
    return BaseNarration;
}

bool UNarrativeStoryManager::ShouldTriggerStoryMoment() const
{
    // Trigger story moments based on recent events and context
    
    // First day survival milestone
    if (NarrativeContext.DaysAlive == 1 && 
        NarrativeContext.bHasWater && 
        NarrativeContext.AnimalsHunted > 0)
    {
        return true;
    }
    
    // Major predator encounter
    if (NarrativeContext.FearLevel > 0.9f && 
        !IsStoryEventRecent(ENarr_StoryEventType::FirstDanger, 2.0f))
    {
        return true;
    }
    
    // Mastery achievement
    if (NarrativeContext.PredatorsDefeated >= 5 && 
        NarrativeContext.CurrentPhase != ENarr_StoryPhase::Mastery)
    {
        return true;
    }
    
    return false;
}

FString UNarrativeStoryManager::GetPhaseNarration(ENarr_StoryPhase Phase) const
{
    if (const FString* Description = PhaseDescriptions.Find(Phase))
    {
        return *Description;
    }
    
    return TEXT("Your journey continues through this ancient world.");
}

bool UNarrativeStoryManager::IsStoryEventRecent(ENarr_StoryEventType EventType, float HoursAgo) const
{
    FDateTime Threshold = FDateTime::Now() - FTimespan::FromHours(HoursAgo);
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventType == EventType && Event.EventTime > Threshold)
        {
            return true;
        }
    }
    
    return false;
}
#include "Narr_StoryManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentStoryPhase = ENarr_StoryPhase::Awakening;
    StoryEventDataTable = nullptr;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeDefaultEvents();
    LoadStoryEvents();
    
    UE_LOG(LogTemp, Warning, TEXT("Story Manager initialized with phase: %d"), (int32)CurrentStoryPhase);
}

void UNarr_StoryManager::Deinitialize()
{
    StoryEvents.Empty();
    StoryProgressMap.Empty();
    
    Super::Deinitialize();
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase != CurrentStoryPhase)
    {
        ENarr_StoryPhase PreviousPhase = CurrentStoryPhase;
        CurrentStoryPhase = NewPhase;
        
        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced from %d to %d"), (int32)PreviousPhase, (int32)NewPhase);
        
        // Trigger phase-specific events
        for (FNarr_StoryEvent& Event : StoryEvents)
        {
            if (Event.RequiredPhase == NewPhase && !Event.bIsCompleted)
            {
                if (CheckEventConditions(Event))
                {
                    TriggerStoryEvent(Event.EventID);
                }
            }
        }
    }
}

void UNarr_StoryManager::TriggerStoryEvent(const FString& EventID)
{
    for (FNarr_StoryEvent& Event : StoryEvents)
    {
        if (Event.EventID == EventID && !Event.bIsCompleted)
        {
            Event.bIsCompleted = true;
            
            UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *EventID);
            
            // Broadcast event completion (could add delegate here)
            break;
        }
    }
}

bool UNarr_StoryManager::IsStoryEventCompleted(const FString& EventID) const
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

TArray<FNarr_StoryEvent> UNarr_StoryManager::GetAvailableEvents() const
{
    TArray<FNarr_StoryEvent> AvailableEvents;
    
    for (const FNarr_StoryEvent& Event : StoryEvents)
    {
        if (!Event.bIsCompleted && Event.RequiredPhase == CurrentStoryPhase)
        {
            if (CheckEventConditions(Event))
            {
                AvailableEvents.Add(Event);
            }
        }
    }
    
    return AvailableEvents;
}

void UNarr_StoryManager::RegisterStoryProgress(const FString& ProgressKey, int32 Value)
{
    StoryProgressMap.Add(ProgressKey, Value);
    
    UE_LOG(LogTemp, Log, TEXT("Story progress registered: %s = %d"), *ProgressKey, Value);
}

int32 UNarr_StoryManager::GetStoryProgress(const FString& ProgressKey) const
{
    if (const int32* FoundValue = StoryProgressMap.Find(ProgressKey))
    {
        return *FoundValue;
    }
    return 0;
}

void UNarr_StoryManager::LoadStoryEvents()
{
    if (StoryEventDataTable)
    {
        TArray<FNarr_StoryEvent*> AllRows;
        StoryEventDataTable->GetAllRows<FNarr_StoryEvent>(TEXT("LoadStoryEvents"), AllRows);
        
        for (FNarr_StoryEvent* Row : AllRows)
        {
            if (Row)
            {
                StoryEvents.Add(*Row);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Loaded %d story events from data table"), StoryEvents.Num());
    }
}

void UNarr_StoryManager::InitializeDefaultEvents()
{
    StoryEvents.Empty();
    
    // Awakening Phase Events
    FNarr_StoryEvent FirstAwakening;
    FirstAwakening.EventID = TEXT("first_awakening");
    FirstAwakening.EventTitle = FText::FromString(TEXT("First Awakening"));
    FirstAwakening.EventDescription = FText::FromString(TEXT("You wake in an unknown land filled with ancient beasts."));
    FirstAwakening.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstAwakening.TriggerConditions.Add(TEXT("player_spawned"));
    StoryEvents.Add(FirstAwakening);
    
    // First Hunt Phase Events
    FNarr_StoryEvent FirstKill;
    FirstKill.EventID = TEXT("first_kill");
    FirstKill.EventTitle = FText::FromString(TEXT("First Blood"));
    FirstKill.EventDescription = FText::FromString(TEXT("Your first successful hunt marks the beginning of survival."));
    FirstKill.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    FirstKill.TriggerConditions.Add(TEXT("dinosaur_killed"));
    StoryEvents.Add(FirstKill);
    
    // Tribe Contact Phase Events
    FNarr_StoryEvent TribeEncounter;
    TribeEncounter.EventID = TEXT("tribe_encounter");
    TribeEncounter.EventTitle = FText::FromString(TEXT("Ancient Voices"));
    TribeEncounter.EventDescription = FText::FromString(TEXT("You encounter other survivors who speak of ancient hunting grounds."));
    TribeEncounter.RequiredPhase = ENarr_StoryPhase::TribeContact;
    TribeEncounter.TriggerConditions.Add(TEXT("npc_encountered"));
    StoryEvents.Add(TribeEncounter);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default story events"), StoryEvents.Num());
}

bool UNarr_StoryManager::CheckEventConditions(const FNarr_StoryEvent& Event) const
{
    // Check all trigger conditions
    for (const FString& Condition : Event.TriggerConditions)
    {
        if (Condition == TEXT("player_spawned"))
        {
            // Always true if player exists
            continue;
        }
        else if (Condition == TEXT("dinosaur_killed"))
        {
            // Check if player has killed any dinosaurs
            if (GetStoryProgress(TEXT("dinosaurs_killed")) <= 0)
            {
                return false;
            }
        }
        else if (Condition == TEXT("npc_encountered"))
        {
            // Check if player has encountered NPCs
            if (GetStoryProgress(TEXT("npcs_met")) <= 0)
            {
                return false;
            }
        }
    }
    
    return true;
}
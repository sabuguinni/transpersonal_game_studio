#include "Narr_StoryManager.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TranspersonalGame.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    StoryDataTable = nullptr;
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Narrative Story Manager initialized"));
    
    CurrentStoryState = FNarr_StoryState();
    StoryTriggers.Empty();
    
    InitializeDefaultObjectives();
}

void UNarr_StoryManager::TriggerStoryEvent(ENarr_StoryEvent Event, AActor* Instigator)
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Story event triggered: %d"), (int32)Event);

    // Update story state based on event
    switch (Event)
    {
        case ENarr_StoryEvent::FirstKill:
            CurrentStoryState.KillCount++;
            if (CurrentStoryState.CurrentPhase == ENarr_StoryPhase::Awakening)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::FirstHunt);
            }
            break;

        case ENarr_StoryEvent::FirstDeath:
            CurrentStoryState.DeathCount++;
            break;

        case ENarr_StoryEvent::TribalMeeting:
            CurrentStoryState.bTribalContactMade = true;
            if (CurrentStoryState.CurrentPhase == ENarr_StoryPhase::FirstHunt)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::TribalContact);
            }
            break;

        case ENarr_StoryEvent::TerritoryMarked:
            CurrentStoryState.bTerritoryEstablished = true;
            if (CurrentStoryState.CurrentPhase == ENarr_StoryPhase::TribalContact)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::TerritoryWar);
            }
            break;

        case ENarr_StoryEvent::AlphaEncounter:
            if (CurrentStoryState.CurrentPhase == ENarr_StoryPhase::TerritoryWar)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::AlphaChallenge);
            }
            break;

        case ENarr_StoryEvent::SurvivalMilestone:
            if (CurrentStoryState.SurvivalDays >= 30 && CurrentStoryState.bAlphaDefeated)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::Mastery);
            }
            break;
    }

    // Process story triggers
    if (FNarr_StoryTrigger* Trigger = StoryTriggers.Find(Event))
    {
        ProcessStoryTrigger(*Trigger, Instigator);
    }

    // Add event to completed events
    FString EventName = FString::Printf(TEXT("Event_%d"), (int32)Event);
    CurrentStoryState.CompletedEvents.AddUnique(EventName);
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (CurrentStoryState.CurrentPhase == NewPhase)
    {
        return;
    }

    if (!CheckPhaseRequirements(NewPhase))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot advance to phase %d: Requirements not met"), (int32)NewPhase);
        return;
    }

    ENarr_StoryPhase OldPhase = CurrentStoryState.CurrentPhase;
    CurrentStoryState.CurrentPhase = NewPhase;

    UE_LOG(LogTranspersonalGame, Log, TEXT("Story phase advanced from %d to %d"), (int32)OldPhase, (int32)NewPhase);
    UE_LOG(LogTranspersonalGame, Log, TEXT("New phase: %s"), *GetPhaseDescription(NewPhase));

    // Add phase-specific objectives
    switch (NewPhase)
    {
        case ENarr_StoryPhase::FirstHunt:
            AddObjective(TEXT("hunt_small_prey"), TEXT("Hunt small dinosaurs to survive"));
            AddObjective(TEXT("craft_tools"), TEXT("Craft basic stone tools"));
            break;

        case ENarr_StoryPhase::TribalContact:
            AddObjective(TEXT("find_survivors"), TEXT("Locate other human survivors"));
            AddObjective(TEXT("establish_communication"), TEXT("Learn to communicate with tribe"));
            break;

        case ENarr_StoryPhase::TerritoryWar:
            AddObjective(TEXT("mark_territory"), TEXT("Establish territorial boundaries"));
            AddObjective(TEXT("defend_resources"), TEXT("Protect valuable resources"));
            break;

        case ENarr_StoryPhase::AlphaChallenge:
            AddObjective(TEXT("face_apex_predator"), TEXT("Confront the apex predator"));
            AddObjective(TEXT("prove_dominance"), TEXT("Establish dominance in the ecosystem"));
            break;

        case ENarr_StoryPhase::Mastery:
            AddObjective(TEXT("master_environment"), TEXT("Achieve complete environmental mastery"));
            break;
    }
}

ENarr_StoryPhase UNarr_StoryManager::GetCurrentPhase() const
{
    return CurrentStoryState.CurrentPhase;
}

FNarr_StoryState UNarr_StoryManager::GetStoryState() const
{
    return CurrentStoryState;
}

void UNarr_StoryManager::UpdateSurvivalStats(int32 DaysAlive, int32 Kills, int32 Deaths)
{
    CurrentStoryState.SurvivalDays = DaysAlive;
    CurrentStoryState.KillCount = Kills;
    CurrentStoryState.DeathCount = Deaths;

    UE_LOG(LogTranspersonalGame, Log, TEXT("Survival stats updated: %d days, %d kills, %d deaths"), 
           DaysAlive, Kills, Deaths);

    // Check for survival milestones
    if (DaysAlive > 0 && DaysAlive % 7 == 0) // Weekly milestones
    {
        TriggerStoryEvent(ENarr_StoryEvent::SurvivalMilestone);
    }
}

void UNarr_StoryManager::CompleteObjective(const FString& ObjectiveID)
{
    if (CurrentStoryState.ActiveObjectives.Contains(ObjectiveID))
    {
        CurrentStoryState.ActiveObjectives.Remove(ObjectiveID);
        UE_LOG(LogTranspersonalGame, Log, TEXT("Objective completed: %s"), *ObjectiveID);
    }
}

void UNarr_StoryManager::AddObjective(const FString& ObjectiveID, const FString& Description)
{
    if (!CurrentStoryState.ActiveObjectives.Contains(ObjectiveID))
    {
        CurrentStoryState.ActiveObjectives.Add(ObjectiveID);
        UE_LOG(LogTranspersonalGame, Log, TEXT("Objective added: %s - %s"), *ObjectiveID, *Description);
    }
}

TArray<FString> UNarr_StoryManager::GetActiveObjectives() const
{
    return CurrentStoryState.ActiveObjectives;
}

void UNarr_StoryManager::LoadStoryData(UDataTable* StoryTable)
{
    if (!StoryTable)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Cannot load story data: Invalid data table"));
        return;
    }

    StoryDataTable = StoryTable;
    UE_LOG(LogTranspersonalGame, Log, TEXT("Loaded story data table"));
}

void UNarr_StoryManager::SaveStoryProgress()
{
    // TODO: Implement save system when available
    UE_LOG(LogTranspersonalGame, Log, TEXT("Story progress saved (placeholder)"));
}

void UNarr_StoryManager::LoadStoryProgress()
{
    // TODO: Implement load system when available
    UE_LOG(LogTranspersonalGame, Log, TEXT("Story progress loaded (placeholder)"));
}

void UNarr_StoryManager::ProcessStoryTrigger(const FNarr_StoryTrigger& Trigger, AActor* Instigator)
{
    UE_LOG(LogTranspersonalGame, Log, TEXT("Processing story trigger: %s"), *Trigger.TriggerDescription);

    // Add unlocked objectives
    for (const FString& ObjectiveID : Trigger.UnlockObjectives)
    {
        AddObjective(ObjectiveID, TEXT("Triggered objective"));
    }

    // Advance phase if required
    if (Trigger.bAdvancePhase)
    {
        // Logic for phase advancement is handled in TriggerStoryEvent
    }
}

bool UNarr_StoryManager::CheckPhaseRequirements(ENarr_StoryPhase Phase)
{
    switch (Phase)
    {
        case ENarr_StoryPhase::Awakening:
            return true;

        case ENarr_StoryPhase::FirstHunt:
            return CurrentStoryState.KillCount > 0;

        case ENarr_StoryPhase::TribalContact:
            return CurrentStoryState.bTribalContactMade;

        case ENarr_StoryPhase::TerritoryWar:
            return CurrentStoryState.bTerritoryEstablished;

        case ENarr_StoryPhase::AlphaChallenge:
            return CurrentStoryState.SurvivalDays >= 14;

        case ENarr_StoryPhase::Mastery:
            return CurrentStoryState.bAlphaDefeated && CurrentStoryState.SurvivalDays >= 30;
    }

    return false;
}

void UNarr_StoryManager::InitializeDefaultObjectives()
{
    AddObjective(TEXT("survive_first_day"), TEXT("Survive your first day in the prehistoric world"));
    AddObjective(TEXT("find_water"), TEXT("Locate a source of fresh water"));
    AddObjective(TEXT("find_shelter"), TEXT("Find or create shelter from the elements"));
}

FString UNarr_StoryManager::GetPhaseDescription(ENarr_StoryPhase Phase)
{
    switch (Phase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("The Awakening - A primitive human awakens in a dangerous prehistoric world");
        case ENarr_StoryPhase::FirstHunt:
            return TEXT("First Hunt - Learning to hunt and survive");
        case ENarr_StoryPhase::TribalContact:
            return TEXT("Tribal Contact - Meeting other survivors");
        case ENarr_StoryPhase::TerritoryWar:
            return TEXT("Territory War - Fighting for resources and territory");
        case ENarr_StoryPhase::AlphaChallenge:
            return TEXT("Alpha Challenge - Facing the apex predators");
        case ENarr_StoryPhase::Mastery:
            return TEXT("Mastery - Becoming the apex survivor");
        default:
            return TEXT("Unknown Phase");
    }
}
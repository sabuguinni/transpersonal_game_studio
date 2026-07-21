#include "Narr_StoryManager.h"
#include "Narr_DialogueManager.h"
#include "Engine/Engine.h"

UNarr_StoryManager::UNarr_StoryManager()
{
    CurrentProgress = FNarr_StoryProgress();
}

void UNarr_StoryManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializePhaseDescriptions();
    InitializeStoryTriggers();
    
    UE_LOG(LogTemp, Log, TEXT("Narrative Story Manager initialized"));
}

void UNarr_StoryManager::TriggerStoryEvent(ENarr_StoryEvent Event)
{
    if (!HasCompletedEvent(Event))
    {
        CurrentProgress.CompletedEvents.Add(Event);
        
        // Process any story triggers for this event
        for (FNarr_StoryTrigger& Trigger : StoryTriggers)
        {
            if (Trigger.TriggerEvent == Event && 
                !Trigger.bTriggered && 
                CurrentProgress.CurrentPhase >= Trigger.RequiredPhase)
            {
                ProcessStoryTrigger(Trigger);
                if (Trigger.bOnlyOnce)
                {
                    Trigger.bTriggered = true;
                }
            }
        }
        
        CheckPhaseAdvancement();
        
        UE_LOG(LogTemp, Log, TEXT("Story event triggered: %d"), (int32)Event);
    }
}

void UNarr_StoryManager::AdvanceStoryPhase(ENarr_StoryPhase NewPhase)
{
    if (NewPhase > CurrentProgress.CurrentPhase)
    {
        CurrentProgress.CurrentPhase = NewPhase;
        
        FString PhaseDesc = GetCurrentPhaseDescription();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, 
                FString::Printf(TEXT("Story Phase: %s"), *PhaseDesc));
        }
        
        UE_LOG(LogTemp, Log, TEXT("Advanced to story phase: %s"), *PhaseDesc);
    }
}

ENarr_StoryPhase UNarr_StoryManager::GetCurrentStoryPhase() const
{
    return CurrentProgress.CurrentPhase;
}

bool UNarr_StoryManager::HasCompletedEvent(ENarr_StoryEvent Event) const
{
    return CurrentProgress.CompletedEvents.Contains(Event);
}

void UNarr_StoryManager::IncrementDinosaurKills()
{
    CurrentProgress.DinosaurKills++;
    
    if (CurrentProgress.DinosaurKills == 1)
    {
        TriggerStoryEvent(ENarr_StoryEvent::FirstKill);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur kills: %d"), CurrentProgress.DinosaurKills);
}

void UNarr_StoryManager::IncrementTerritoriesDiscovered()
{
    CurrentProgress.TerritoriesDiscovered++;
    
    if (CurrentProgress.TerritoriesDiscovered == 1)
    {
        TriggerStoryEvent(ENarr_StoryEvent::TerritoryDiscovered);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Territories discovered: %d"), CurrentProgress.TerritoriesDiscovered);
}

void UNarr_StoryManager::UpdateSurvivalTime(float DeltaTime)
{
    CurrentProgress.SurvivalTime += DeltaTime;
}

FNarr_StoryProgress UNarr_StoryManager::GetStoryProgress() const
{
    return CurrentProgress;
}

void UNarr_StoryManager::SetTribalContact(bool bContactMade)
{
    if (bContactMade && !CurrentProgress.bTribalContactMade)
    {
        CurrentProgress.bTribalContactMade = true;
        TriggerStoryEvent(ENarr_StoryEvent::TribalMeeting);
    }
}

void UNarr_StoryManager::SetAlphaDefeated(bool bDefeated)
{
    if (bDefeated && !CurrentProgress.bAlphaDefeated)
    {
        CurrentProgress.bAlphaDefeated = true;
        TriggerStoryEvent(ENarr_StoryEvent::AlphaDefeated);
    }
}

FString UNarr_StoryManager::GetCurrentPhaseDescription() const
{
    if (PhaseDescriptions.Contains(CurrentProgress.CurrentPhase))
    {
        return PhaseDescriptions[CurrentProgress.CurrentPhase];
    }
    return TEXT("Unknown Phase");
}

void UNarr_StoryManager::RegisterStoryTrigger(const FNarr_StoryTrigger& Trigger)
{
    StoryTriggers.Add(Trigger);
}

void UNarr_StoryManager::InitializeStoryTriggers()
{
    // Player Spawned trigger
    FNarr_StoryTrigger SpawnTrigger;
    SpawnTrigger.TriggerEvent = ENarr_StoryEvent::PlayerSpawned;
    SpawnTrigger.DialogueSequenceName = TEXT("TribalWelcome");
    SpawnTrigger.NarrativeText = TEXT("You awaken in the ancient valley, surrounded by the sounds of a prehistoric world.");
    SpawnTrigger.RequiredPhase = ENarr_StoryPhase::Awakening;
    SpawnTrigger.bOnlyOnce = true;
    RegisterStoryTrigger(SpawnTrigger);
    
    // First Dinosaur Sighted trigger
    FNarr_StoryTrigger SightTrigger;
    SightTrigger.TriggerEvent = ENarr_StoryEvent::FirstDinosaurSighted;
    SightTrigger.DialogueSequenceName = TEXT("HuntWarning");
    SightTrigger.NarrativeText = TEXT("A massive creature moves through the undergrowth. Your survival instincts awaken.");
    SightTrigger.RequiredPhase = ENarr_StoryPhase::Awakening;
    SightTrigger.bOnlyOnce = true;
    RegisterStoryTrigger(SightTrigger);
    
    // First Kill trigger
    FNarr_StoryTrigger KillTrigger;
    KillTrigger.TriggerEvent = ENarr_StoryEvent::FirstKill;
    KillTrigger.DialogueSequenceName = TEXT("AncientWisdom");
    KillTrigger.NarrativeText = TEXT("Your first victory against the ancient beasts. You are becoming a true hunter.");
    KillTrigger.RequiredPhase = ENarr_StoryPhase::Awakening;
    KillTrigger.bOnlyOnce = true;
    RegisterStoryTrigger(KillTrigger);
    
    // Tribal Meeting trigger
    FNarr_StoryTrigger TribalTrigger;
    TribalTrigger.TriggerEvent = ENarr_StoryEvent::TribalMeeting;
    TribalTrigger.DialogueSequenceName = TEXT("TribalWelcome");
    TribalTrigger.NarrativeText = TEXT("You encounter other survivors. Perhaps you are not alone in this world.");
    TribalTrigger.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    TribalTrigger.bOnlyOnce = true;
    RegisterStoryTrigger(TribalTrigger);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d story triggers"), StoryTriggers.Num());
}

void UNarr_StoryManager::InitializePhaseDescriptions()
{
    PhaseDescriptions.Add(ENarr_StoryPhase::Awakening, TEXT("The Awakening - Learning to survive"));
    PhaseDescriptions.Add(ENarr_StoryPhase::FirstHunt, TEXT("First Hunt - Proving your skills"));
    PhaseDescriptions.Add(ENarr_StoryPhase::TribalContact, TEXT("Tribal Contact - Meeting other survivors"));
    PhaseDescriptions.Add(ENarr_StoryPhase::TerritoryExploration, TEXT("Territory Exploration - Mapping the valley"));
    PhaseDescriptions.Add(ENarr_StoryPhase::AlphaEncounter, TEXT("Alpha Encounter - Facing the apex predators"));
    PhaseDescriptions.Add(ENarr_StoryPhase::PackLeadership, TEXT("Pack Leadership - Leading the tribe"));
    PhaseDescriptions.Add(ENarr_StoryPhase::ValleyMastery, TEXT("Valley Mastery - Mastering the prehistoric world"));
}

void UNarr_StoryManager::ProcessStoryTrigger(const FNarr_StoryTrigger& Trigger)
{
    // Display narrative text
    if (!Trigger.NarrativeText.IsEmpty() && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Cyan, Trigger.NarrativeText);
    }
    
    // Trigger dialogue sequence if specified
    if (!Trigger.DialogueSequenceName.IsEmpty())
    {
        if (UNarr_DialogueManager* DialogueManager = GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
        {
            DialogueManager->PlayDialogueSequence(Trigger.DialogueSequenceName);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Processed story trigger: %s"), *Trigger.NarrativeText);
}

void UNarr_StoryManager::CheckPhaseAdvancement()
{
    switch (CurrentProgress.CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            if (HasCompletedEvent(ENarr_StoryEvent::FirstKill))
            {
                AdvanceStoryPhase(ENarr_StoryPhase::FirstHunt);
            }
            break;
            
        case ENarr_StoryPhase::FirstHunt:
            if (CurrentProgress.DinosaurKills >= 3)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::TribalContact);
            }
            break;
            
        case ENarr_StoryPhase::TribalContact:
            if (CurrentProgress.bTribalContactMade && CurrentProgress.TerritoriesDiscovered >= 2)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::TerritoryExploration);
            }
            break;
            
        case ENarr_StoryPhase::TerritoryExploration:
            if (CurrentProgress.DinosaurKills >= 10 && CurrentProgress.TerritoriesDiscovered >= 5)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::AlphaEncounter);
            }
            break;
            
        case ENarr_StoryPhase::AlphaEncounter:
            if (CurrentProgress.bAlphaDefeated)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::PackLeadership);
            }
            break;
            
        case ENarr_StoryPhase::PackLeadership:
            if (CurrentProgress.DinosaurKills >= 25 && CurrentProgress.TerritoriesDiscovered >= 10)
            {
                AdvanceStoryPhase(ENarr_StoryPhase::ValleyMastery);
            }
            break;
            
        default:
            break;
    }
}
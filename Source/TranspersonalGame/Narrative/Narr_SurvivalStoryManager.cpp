#include "Narr_SurvivalStoryManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ANarr_SurvivalStoryManager::ANarr_SurvivalStoryManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    StoryUpdateInterval = 30.0f; // Update every 30 seconds
    bAutoProgressStory = true;
    LastUpdateTime = 0.0f;
    
    // Initialize story data
    StoryData.CurrentPhase = ENarr_SurvivalStoryPhase::Awakening;
    StoryData.SurvivalDays = 0;
    StoryData.CharacterGrowthScore = 0.0f;
}

void ANarr_SurvivalStoryManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStoryNarratives();
    
    if (bAutoProgressStory)
    {
        GetWorld()->GetTimerManager().SetTimer(
            StoryUpdateTimer,
            this,
            &ANarr_SurvivalStoryManager::CheckForPhaseTransitions,
            StoryUpdateInterval,
            true
        );
    }
}

void ANarr_SurvivalStoryManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Update survival days based on game time
    if (LastUpdateTime >= 86400.0f) // 24 hours in seconds (simplified)
    {
        StoryData.SurvivalDays++;
        LastUpdateTime = 0.0f;
        BroadcastStoryUpdate();
    }
}

void ANarr_SurvivalStoryManager::InitializeStoryNarratives()
{
    // Phase narratives
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::Awakening, 
        TEXT("You wake in a world that time forgot. Every breath is a victory against extinction."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::FirstHunt, 
        TEXT("The hunt teaches you that in this world, you are both predator and prey."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::FirstCraft, 
        TEXT("Your hands shape stone and bone into tools. Civilization begins with a single strike."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::FirstShelter, 
        TEXT("Four walls against the endless night. You have claimed your first territory."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::DinosaurThreat, 
        TEXT("The earth shakes with ancient footsteps. The true masters of this world have noticed you."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::TribeContact, 
        TEXT("Other eyes watch from the shadows. You are not alone in this struggle."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::Leadership, 
        TEXT("They look to you for guidance. The weight of survival rests on your shoulders."));
    
    PhaseNarratives.Add(ENarr_SurvivalStoryPhase::Mastery, 
        TEXT("You have become what this world demands: apex survivor, tribal leader, master of the prehistoric realm."));

    // Milestone narratives
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstFire, 
        TEXT("Fire dances in your hands. You have stolen light from the gods."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstKill, 
        TEXT("Life ends by your hand. The circle of survival claims its first victory."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstTool, 
        TEXT("Stone becomes weapon. Your mind shapes the world to your will."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstShelter, 
        TEXT("Walls rise from nothing. You have carved safety from chaos."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstTaming, 
        TEXT("Beast bows to your will. The impossible becomes your ally."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstTrade, 
        TEXT("Value exchanges hands. Commerce blooms in the wasteland."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstBattle, 
        TEXT("Steel meets flesh. War comes to paradise."));
    
    MilestoneNarratives.Add(ENarr_SurvivalMilestone::FirstVictory, 
        TEXT("Victory tastes of blood and determination. You have proven your worth."));
}

void ANarr_SurvivalStoryManager::AdvanceStoryPhase(ENarr_SurvivalStoryPhase NewPhase)
{
    if (NewPhase != StoryData.CurrentPhase)
    {
        StoryData.CurrentPhase = NewPhase;
        BroadcastStoryUpdate();
        
        UE_LOG(LogTemp, Warning, TEXT("Story phase advanced to: %d"), (int32)NewPhase);
    }
}

void ANarr_SurvivalStoryManager::CompleteMilestone(ENarr_SurvivalMilestone Milestone)
{
    if (!HasCompletedMilestone(Milestone))
    {
        StoryData.CompletedMilestones.Add(Milestone);
        StoryData.CharacterGrowthScore += 10.0f;
        BroadcastStoryUpdate();
        
        UE_LOG(LogTemp, Warning, TEXT("Milestone completed: %d"), (int32)Milestone);
    }
}

void ANarr_SurvivalStoryManager::UpdateSurvivalProgress(int32 Days, float GrowthScore)
{
    StoryData.SurvivalDays = Days;
    StoryData.CharacterGrowthScore = GrowthScore;
    BroadcastStoryUpdate();
}

FString ANarr_SurvivalStoryManager::GetCurrentPhaseNarrative() const
{
    const FString* Narrative = PhaseNarratives.Find(StoryData.CurrentPhase);
    return Narrative ? *Narrative : TEXT("The story unfolds...");
}

FString ANarr_SurvivalStoryManager::GetMilestoneNarrative(ENarr_SurvivalMilestone Milestone) const
{
    const FString* Narrative = MilestoneNarratives.Find(Milestone);
    return Narrative ? *Narrative : TEXT("A moment of achievement...");
}

bool ANarr_SurvivalStoryManager::HasCompletedMilestone(ENarr_SurvivalMilestone Milestone) const
{
    return StoryData.CompletedMilestones.Contains(Milestone);
}

float ANarr_SurvivalStoryManager::GetStoryCompletionPercentage() const
{
    int32 TotalPhases = (int32)ENarr_SurvivalStoryPhase::Mastery + 1;
    int32 CurrentPhaseIndex = (int32)StoryData.CurrentPhase;
    
    float PhaseProgress = (float)CurrentPhaseIndex / (float)TotalPhases;
    float MilestoneProgress = (float)StoryData.CompletedMilestones.Num() / 8.0f; // 8 total milestones
    
    return (PhaseProgress + MilestoneProgress) * 0.5f; // Average of both
}

TArray<FString> ANarr_SurvivalStoryManager::GetAvailableStoryBeats() const
{
    return StoryData.UnlockedStoryBeats;
}

void ANarr_SurvivalStoryManager::TriggerStoryEvent(const FString& EventName)
{
    if (!StoryData.UnlockedStoryBeats.Contains(EventName))
    {
        StoryData.UnlockedStoryBeats.Add(EventName);
        BroadcastStoryUpdate();
        
        UE_LOG(LogTemp, Warning, TEXT("Story event triggered: %s"), *EventName);
    }
}

void ANarr_SurvivalStoryManager::CheckForPhaseTransitions()
{
    // Auto-advance story phases based on milestones and survival days
    switch (StoryData.CurrentPhase)
    {
        case ENarr_SurvivalStoryPhase::Awakening:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstFire) || StoryData.SurvivalDays >= 1)
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::FirstHunt);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::FirstHunt:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstKill))
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::FirstCraft);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::FirstCraft:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstTool))
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::FirstShelter);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::FirstShelter:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstShelter))
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::DinosaurThreat);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::DinosaurThreat:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstBattle) || StoryData.SurvivalDays >= 7)
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::TribeContact);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::TribeContact:
            if (HasCompletedMilestone(ENarr_SurvivalMilestone::FirstTrade) || StoryData.SurvivalDays >= 14)
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::Leadership);
            }
            break;
            
        case ENarr_SurvivalStoryPhase::Leadership:
            if (StoryData.CompletedMilestones.Num() >= 6 || StoryData.SurvivalDays >= 30)
            {
                AdvanceStoryPhase(ENarr_SurvivalStoryPhase::Mastery);
            }
            break;
            
        default:
            break;
    }
}

void ANarr_SurvivalStoryManager::BroadcastStoryUpdate()
{
    // Broadcast story update to other systems
    UE_LOG(LogTemp, Log, TEXT("Story Update: Phase %d, Days %d, Growth %.1f"), 
           (int32)StoryData.CurrentPhase, StoryData.SurvivalDays, StoryData.CharacterGrowthScore);
}
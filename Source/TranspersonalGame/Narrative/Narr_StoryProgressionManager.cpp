#include "Narr_StoryProgressionManager.h"
#include "Engine/Engine.h"
#include "Narr_DialogueManager.h"

UNarr_StoryProgressionManager::UNarr_StoryProgressionManager()
{
    CurrentPhase = ENarr_StoryPhase::Awakening;
    DinosaurKillCount = 0;
    DaysStoryMilestones.Empty();
    StoryFlags.Empty();
}

void UNarr_StoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeStoryMilestones();
    UE_LOG(LogTemp, Log, TEXT("Story Progression Manager initialized"));
}

void UNarr_StoryProgressionManager::AdvanceStoryPhase()
{
    ENarr_StoryPhase NewPhase = CurrentPhase;
    
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            if (DinosaurKillCount >= 1)
            {
                NewPhase = ENarr_StoryPhase::FirstHunt;
            }
            break;
        case ENarr_StoryPhase::FirstHunt:
            if (DinosaurKillCount >= 5)
            {
                NewPhase = ENarr_StoryPhase::TerritoryWars;
            }
            break;
        case ENarr_StoryPhase::TerritoryWars:
            if (DinosaurKillCount >= 15)
            {
                NewPhase = ENarr_StoryPhase::AlphaChallenge;
            }
            break;
        case ENarr_StoryPhase::AlphaChallenge:
            if (StoryFlags.Contains(TEXT("AlphaDefeated")) && StoryFlags[TEXT("AlphaDefeated")])
            {
                NewPhase = ENarr_StoryPhase::Mastery;
            }
            break;
        case ENarr_StoryPhase::Mastery:
            // Final phase - no advancement
            break;
    }

    if (NewPhase != CurrentPhase)
    {
        CurrentPhase = NewPhase;
        UnlockPhaseContent(NewPhase);
        UE_LOG(LogTemp, Log, TEXT("Story phase advanced to: %d"), (int32)NewPhase);
    }
}

ENarr_StoryPhase UNarr_StoryProgressionManager::GetCurrentStoryPhase() const
{
    return CurrentPhase;
}

void UNarr_StoryProgressionManager::CompleteMilestone(const FString& MilestoneID)
{
    for (FNarr_StoryMilestone& Milestone : StoryMilestones)
    {
        if (Milestone.MilestoneID == MilestoneID && !Milestone.bIsCompleted)
        {
            Milestone.bIsCompleted = true;
            
            // Unlock associated dialogues
            if (UNarr_DialogueManager* DialogueManager = GetGameInstance()->GetSubsystem<UNarr_DialogueManager>())
            {
                for (const FString& DialogueID : Milestone.UnlockDialogues)
                {
                    // Trigger unlocked dialogue
                    UE_LOG(LogTemp, Log, TEXT("Unlocked dialogue: %s"), *DialogueID);
                }
            }
            
            UE_LOG(LogTemp, Log, TEXT("Completed milestone: %s"), *MilestoneID);
            CheckPhaseProgression();
            break;
        }
    }
}

bool UNarr_StoryProgressionManager::IsMilestoneCompleted(const FString& MilestoneID) const
{
    for (const FNarr_StoryMilestone& Milestone : StoryMilestones)
    {
        if (Milestone.MilestoneID == MilestoneID)
        {
            return Milestone.bIsCompleted;
        }
    }
    return false;
}

void UNarr_StoryProgressionManager::TriggerStoryEvent(const FString& EventName)
{
    if (EventName == TEXT("DinosaurKilled"))
    {
        DinosaurKillCount++;
        UE_LOG(LogTemp, Log, TEXT("Dinosaur kill count: %d"), DinosaurKillCount);
        AdvanceStoryPhase();
    }
    else if (EventName == TEXT("DayPassed"))
    {
        DaysStoryMilestones++;
        UE_LOG(LogTemp, Log, TEXT("Days survived: %d"), DaysStoryMilestones);
    }
    else if (EventName == TEXT("AlphaDefeated"))
    {
        StoryFlags.Add(TEXT("AlphaDefeated"), true);
        AdvanceStoryPhase();
    }
}

FString UNarr_StoryProgressionManager::GetCurrentPhaseNarration() const
{
    switch (CurrentPhase)
    {
        case ENarr_StoryPhase::Awakening:
            return TEXT("You awaken in a world where giants roam. Every shadow could hide death, every sound could be your last warning.");
        case ENarr_StoryPhase::FirstHunt:
            return TEXT("Blood on your hands marks the beginning. You are no longer prey - you are learning to hunt.");
        case ENarr_StoryPhase::TerritoryWars:
            return TEXT("The land knows your scent now. Predators circle, testing your resolve. Territory must be claimed with fang and claw.");
        case ENarr_StoryPhase::AlphaChallenge:
            return TEXT("The apex predator has taken notice. Only one can rule this domain. The final hunt begins.");
        case ENarr_StoryPhase::Mastery:
            return TEXT("You stand atop the food chain. The prehistoric world bows to your dominance. You have become the apex predator.");
        default:
            return TEXT("The story unfolds...");
    }
}

void UNarr_StoryProgressionManager::InitializeStoryMilestones()
{
    // First Kill Milestone
    FNarr_StoryMilestone FirstKill;
    FirstKill.MilestoneID = TEXT("FirstKill");
    FirstKill.Description = TEXT("Kill your first dinosaur");
    FirstKill.RequiredPhase = ENarr_StoryPhase::Awakening;
    FirstKill.UnlockDialogues.Add(TEXT("FirstKillCelebration"));
    StoryMilestones.Add(FirstKill);

    // Pack Hunter Milestone
    FNarr_StoryMilestone PackHunter;
    PackHunter.MilestoneID = TEXT("PackHunter");
    PackHunter.Description = TEXT("Defeat a pack of raptors");
    PackHunter.RequiredPhase = ENarr_StoryPhase::FirstHunt;
    PackHunter.UnlockDialogues.Add(TEXT("PackVictory"));
    StoryMilestones.Add(PackHunter);

    // Territory Claimed Milestone
    FNarr_StoryMilestone TerritoryClaimed;
    TerritoryClaimed.MilestoneID = TEXT("TerritoryClaimed");
    TerritoryClaimed.Description = TEXT("Establish dominance over a territory");
    TerritoryClaimed.RequiredPhase = ENarr_StoryPhase::TerritoryWars;
    TerritoryClaimed.UnlockDialogues.Add(TEXT("TerritoryVictory"));
    StoryMilestones.Add(TerritoryClaimed);

    // Alpha Defeated Milestone
    FNarr_StoryMilestone AlphaDefeated;
    AlphaDefeated.MilestoneID = TEXT("AlphaDefeated");
    AlphaDefeated.Description = TEXT("Defeat the apex predator");
    AlphaDefeated.RequiredPhase = ENarr_StoryPhase::AlphaChallenge;
    AlphaDefeated.UnlockDialogues.Add(TEXT("AlphaVictory"));
    StoryMilestones.Add(AlphaDefeated);
}

void UNarr_StoryProgressionManager::CheckPhaseProgression()
{
    AdvanceStoryPhase();
}

void UNarr_StoryProgressionManager::UnlockPhaseContent(ENarr_StoryPhase Phase)
{
    // Unlock phase-specific content like new areas, creatures, or abilities
    switch (Phase)
    {
        case ENarr_StoryPhase::FirstHunt:
            StoryFlags.Add(TEXT("CanCraftAdvancedWeapons"), true);
            break;
        case ENarr_StoryPhase::TerritoryWars:
            StoryFlags.Add(TEXT("CanBuildShelters"), true);
            break;
        case ENarr_StoryPhase::AlphaChallenge:
            StoryFlags.Add(TEXT("CanAccessAlphaTerritory"), true);
            break;
        case ENarr_StoryPhase::Mastery:
            StoryFlags.Add(TEXT("HasMasteredSurvival"), true);
            break;
    }
}
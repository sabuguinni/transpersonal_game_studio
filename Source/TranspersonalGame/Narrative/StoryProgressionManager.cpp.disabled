#include "StoryProgressionManager.h"
#include "Engine/World.h"

UStoryProgressionManager::UStoryProgressionManager()
{
}

void UStoryProgressionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize player progress
    PlayerProgress.CurrentAct = ENarr_StoryAct::Survival;
    PlayerProgress.SurvivalDays = 0;
    PlayerProgress.DinosaurEncounters = 0;
    PlayerProgress.MaxThreatSurvived = 0.0f;
    
    InitializeStoryMilestones();
    
    UE_LOG(LogTemp, Warning, TEXT("StoryProgressionManager initialized"));
}

void UStoryProgressionManager::Deinitialize()
{
    StoryMilestones.Empty();
    Super::Deinitialize();
}

void UStoryProgressionManager::InitializeStoryMilestones()
{
    StoryMilestones.Empty();

    // Act 1: Survival milestones
    FNarr_StoryMilestone FirstNight;
    FirstNight.MilestoneID = TEXT("first_night");
    FirstNight.MilestoneName = FText::FromString(TEXT("First Night"));
    FirstNight.Description = FText::FromString(TEXT("Survive your first night in the prehistoric world"));
    FirstNight.RequiredAct = ENarr_StoryAct::Survival;
    FirstNight.RequiredSurvivalDays = 1;
    FirstNight.UnlockedDialogues.Add(TEXT("morning_survival"));
    StoryMilestones.Add(FirstNight);

    FNarr_StoryMilestone FirstWeek;
    FirstWeek.MilestoneID = TEXT("first_week");
    FirstWeek.MilestoneName = FText::FromString(TEXT("Week Survivor"));
    FirstWeek.Description = FText::FromString(TEXT("Survive for seven days"));
    FirstWeek.RequiredAct = ENarr_StoryAct::Survival;
    FirstWeek.RequiredSurvivalDays = 7;
    FirstWeek.UnlockedDialogues.Add(TEXT("experienced_survivor"));
    StoryMilestones.Add(FirstWeek);

    // Act 2: Exploration milestones
    FNarr_StoryMilestone BiomeExplorer;
    BiomeExplorer.MilestoneID = TEXT("biome_explorer");
    BiomeExplorer.MilestoneName = FText::FromString(TEXT("Biome Explorer"));
    BiomeExplorer.Description = FText::FromString(TEXT("Discover three different biomes"));
    BiomeExplorer.RequiredAct = ENarr_StoryAct::Exploration;
    BiomeExplorer.RequiredSurvivalDays = 3;
    BiomeExplorer.UnlockedDialogues.Add(TEXT("biome_knowledge"));
    StoryMilestones.Add(BiomeExplorer);

    // Act 3: Mastery milestones
    FNarr_StoryMilestone ApexSurvivor;
    ApexSurvivor.MilestoneID = TEXT("apex_survivor");
    ApexSurvivor.MilestoneName = FText::FromString(TEXT("Apex Survivor"));
    ApexSurvivor.Description = FText::FromString(TEXT("Survive encounters with apex predators"));
    ApexSurvivor.RequiredAct = ENarr_StoryAct::Mastery;
    ApexSurvivor.RequiredSurvivalDays = 14;
    ApexSurvivor.UnlockedDialogues.Add(TEXT("apex_mastery"));
    StoryMilestones.Add(ApexSurvivor);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d story milestones"), StoryMilestones.Num());
}

void UStoryProgressionManager::UpdatePlayerProgress(const FNarr_PlayerProgress& NewProgress)
{
    PlayerProgress = NewProgress;
    CheckMilestoneCompletion();
    UpdateStoryAct();
}

void UStoryProgressionManager::CompleteQuest(const FString& QuestID)
{
    if (!PlayerProgress.CompletedQuests.Contains(QuestID))
    {
        PlayerProgress.CompletedQuests.Add(QuestID);
        CheckMilestoneCompletion();
        
        UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *QuestID);
    }
}

void UStoryProgressionManager::DiscoverBiome(ENarr_BiomeType BiomeType)
{
    if (!PlayerProgress.DiscoveredBiomes.Contains(BiomeType))
    {
        PlayerProgress.DiscoveredBiomes.Add(BiomeType);
        CheckMilestoneCompletion();
        
        UE_LOG(LogTemp, Log, TEXT("Biome discovered: %d"), (int32)BiomeType);
    }
}

void UStoryProgressionManager::RecordDinosaurEncounter(const FString& DinosaurType, float ThreatLevel)
{
    PlayerProgress.DinosaurEncounters++;
    
    if (ThreatLevel > PlayerProgress.MaxThreatSurvived)
    {
        PlayerProgress.MaxThreatSurvived = ThreatLevel;
    }
    
    CheckMilestoneCompletion();
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur encounter: %s (Threat: %.2f)"), *DinosaurType, ThreatLevel);
}

void UStoryProgressionManager::AdvanceSurvivalDay()
{
    PlayerProgress.SurvivalDays++;
    CheckMilestoneCompletion();
    UpdateStoryAct();
    
    UE_LOG(LogTemp, Log, TEXT("Survival day advanced: %d"), PlayerProgress.SurvivalDays);
}

void UStoryProgressionManager::CheckMilestoneCompletion()
{
    for (FNarr_StoryMilestone& Milestone : StoryMilestones)
    {
        if (!Milestone.bIsCompleted && AreMilestoneRequirementsMet(Milestone))
        {
            Milestone.bIsCompleted = true;
            PlayerProgress.CompletedMilestones.AddUnique(Milestone.MilestoneID);
            
            OnMilestoneReached.Broadcast(Milestone);
            
            UE_LOG(LogTemp, Warning, TEXT("Milestone reached: %s"), *Milestone.MilestoneName.ToString());
        }
    }
}

void UStoryProgressionManager::UpdateStoryAct()
{
    ENarr_StoryAct NewAct = PlayerProgress.CurrentAct;

    // Progress to Exploration act
    if (PlayerProgress.CurrentAct == ENarr_StoryAct::Survival && PlayerProgress.SurvivalDays >= 3)
    {
        NewAct = ENarr_StoryAct::Exploration;
    }
    // Progress to Mastery act
    else if (PlayerProgress.CurrentAct == ENarr_StoryAct::Exploration && 
             PlayerProgress.SurvivalDays >= 10 && 
             PlayerProgress.DiscoveredBiomes.Num() >= 2)
    {
        NewAct = ENarr_StoryAct::Mastery;
    }

    if (NewAct != PlayerProgress.CurrentAct)
    {
        PlayerProgress.CurrentAct = NewAct;
        OnActChanged.Broadcast(NewAct);
        
        UE_LOG(LogTemp, Warning, TEXT("Story act changed to: %d"), (int32)NewAct);
    }
}

bool UStoryProgressionManager::AreMilestoneRequirementsMet(const FNarr_StoryMilestone& Milestone) const
{
    // Check survival days requirement
    if (PlayerProgress.SurvivalDays < Milestone.RequiredSurvivalDays)
    {
        return false;
    }

    // Check act requirement
    if (PlayerProgress.CurrentAct < Milestone.RequiredAct)
    {
        return false;
    }

    // Check required quests
    for (const FString& RequiredQuest : Milestone.RequiredQuests)
    {
        if (!PlayerProgress.CompletedQuests.Contains(RequiredQuest))
        {
            return false;
        }
    }

    return true;
}

TArray<FNarr_StoryMilestone> UStoryProgressionManager::GetAvailableMilestones() const
{
    TArray<FNarr_StoryMilestone> Available;
    
    for (const FNarr_StoryMilestone& Milestone : StoryMilestones)
    {
        if (!Milestone.bIsCompleted)
        {
            Available.Add(Milestone);
        }
    }
    
    return Available;
}

bool UStoryProgressionManager::IsMilestoneCompleted(const FString& MilestoneID) const
{
    return PlayerProgress.CompletedMilestones.Contains(MilestoneID);
}
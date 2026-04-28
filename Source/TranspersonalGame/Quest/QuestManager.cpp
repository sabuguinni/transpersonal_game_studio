#include "QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
    // Initialize quest storage
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    FailedQuestIDs.Empty();
    RegisteredQuestGivers.Empty();
    QuestLocationMarkers.Empty();
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initializing Quest System"));
    
    // Create default survival quests
    InitializeDefaultQuests();
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest System Initialized with %d quests"), AllQuests.Num());
}

void UQuestManager::Deinitialize()
{
    // Clean up quest data
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    FailedQuestIDs.Empty();
    RegisteredQuestGivers.Empty();
    QuestLocationMarkers.Empty();
    
    Super::Deinitialize();
}

void UQuestManager::InitializeDefaultQuests()
{
    // Quest 1: Hunt the Raptors
    FQuest_QuestData HuntQuest;
    HuntQuest.QuestID = TEXT("HUNT_RAPTORS_01");
    HuntQuest.Title = FText::FromString(TEXT("Raptor Threat"));
    HuntQuest.Description = FText::FromString(TEXT("The raptors have been hunting too close to our camp. Eliminate 3 raptors to ensure our safety."));
    HuntQuest.QuestType = EQuest_QuestType::Survival;
    HuntQuest.Status = EQuest_QuestStatus::Available;
    HuntQuest.ExperienceReward = 150;
    HuntQuest.ItemRewards.Add(TEXT("Stone_Spear"));
    HuntQuest.bIsRepeatable = false;
    
    // Hunt objective
    FQuest_ObjectiveData HuntObjective;
    HuntObjective.ObjectiveID = TEXT("KILL_RAPTORS");
    HuntObjective.Description = FText::FromString(TEXT("Kill 3 Raptors"));
    HuntObjective.Type = EQuest_ObjectiveType::KillTarget;
    HuntObjective.TargetActorID = TEXT("Raptor");
    HuntObjective.RequiredQuantity = 3;
    HuntObjective.CurrentProgress = 0;
    HuntObjective.bIsCompleted = false;
    HuntObjective.bIsOptional = false;
    
    HuntQuest.Objectives.Add(HuntObjective);
    AllQuests.Add(HuntQuest.QuestID, HuntQuest);
    
    // Quest 2: Gather Crystal Shards
    FQuest_QuestData GatherQuest;
    GatherQuest.QuestID = TEXT("GATHER_CRYSTALS_01");
    GatherQuest.Title = FText::FromString(TEXT("Crystal Harvest"));
    GatherQuest.Description = FText::FromString(TEXT("Collect crystal shards from the northern caves to craft better tools."));
    GatherQuest.QuestType = EQuest_QuestType::Exploration;
    GatherQuest.Status = EQuest_QuestStatus::Available;
    GatherQuest.ExperienceReward = 100;
    GatherQuest.ItemRewards.Add(TEXT("Crystal_Knife"));
    GatherQuest.bIsRepeatable = true;
    
    // Gather objective
    FQuest_ObjectiveData GatherObjective;
    GatherObjective.ObjectiveID = TEXT("COLLECT_CRYSTALS");
    GatherObjective.Description = FText::FromString(TEXT("Collect 5 Crystal Shards"));
    GatherObjective.Type = EQuest_ObjectiveType::CollectItem;
    GatherObjective.TargetActorID = TEXT("Crystal_Shard");
    GatherObjective.RequiredQuantity = 5;
    GatherObjective.CurrentProgress = 0;
    GatherObjective.bIsCompleted = false;
    GatherObjective.bIsOptional = false;
    
    GatherQuest.Objectives.Add(GatherObjective);
    AllQuests.Add(GatherQuest.QuestID, GatherQuest);
    
    // Quest 3: Explore the Ancient Cave
    FQuest_QuestData ExploreQuest;
    ExploreQuest.QuestID = TEXT("EXPLORE_CAVE_01");
    ExploreQuest.Title = FText::FromString(TEXT("Cave Exploration"));
    ExploreQuest.Description = FText::FromString(TEXT("Investigate the mysterious cave system to the north. What secrets does it hold?"));
    ExploreQuest.QuestType = EQuest_QuestType::Discovery;
    ExploreQuest.Status = EQuest_QuestStatus::Available;
    ExploreQuest.ExperienceReward = 200;
    ExploreQuest.ItemRewards.Add(TEXT("Ancient_Map"));
    ExploreQuest.bIsRepeatable = false;
    
    // Exploration objective
    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveID = TEXT("REACH_CAVE_DEPTHS");
    ExploreObjective.Description = FText::FromString(TEXT("Reach the deepest chamber of the cave"));
    ExploreObjective.Type = EQuest_ObjectiveType::GoToLocation;
    ExploreObjective.TargetLocation = FVector(200, 800, 300);
    ExploreObjective.RequiredQuantity = 1;
    ExploreObjective.CurrentProgress = 0;
    ExploreObjective.bIsCompleted = false;
    ExploreObjective.bIsOptional = false;
    
    ExploreQuest.Objectives.Add(ExploreObjective);
    AllQuests.Add(ExploreQuest.QuestID, ExploreQuest);
    
    // Quest 4: Survival Challenge
    FQuest_QuestData SurvivalQuest;
    SurvivalQuest.QuestID = TEXT("SURVIVE_NIGHT_01");
    SurvivalQuest.Title = FText::FromString(TEXT("Night Survival"));
    SurvivalQuest.Description = FText::FromString(TEXT("Survive through the night without returning to the main camp. Build shelter and maintain your health."));
    SurvivalQuest.QuestType = EQuest_QuestType::Survival;
    SurvivalQuest.Status = EQuest_QuestStatus::Available;
    SurvivalQuest.ExperienceReward = 300;
    SurvivalQuest.ItemRewards.Add(TEXT("Survival_Kit"));
    SurvivalQuest.bIsRepeatable = true;
    SurvivalQuest.TimeLimit = 600.0f; // 10 minutes
    
    // Survival objective
    FQuest_ObjectiveData SurvivalObjective;
    SurvivalObjective.ObjectiveID = TEXT("SURVIVE_DURATION");
    SurvivalObjective.Description = FText::FromString(TEXT("Survive for 10 minutes away from camp"));
    SurvivalObjective.Type = EQuest_ObjectiveType::Survive;
    SurvivalObjective.RequiredQuantity = 1;
    SurvivalObjective.CurrentProgress = 0;
    SurvivalObjective.bIsCompleted = false;
    SurvivalObjective.bIsOptional = false;
    
    SurvivalQuest.Objectives.Add(SurvivalObjective);
    AllQuests.Add(SurvivalQuest.QuestID, SurvivalQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Created %d default survival quests"), AllQuests.Num());
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Error, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }
    
    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is not available (Status: %d)"), *QuestID, (int32)Quest.Status);
        return false;
    }
    
    if (!CheckQuestPrerequisites(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s prerequisites not met"), *QuestID);
        return false;
    }
    
    // Start the quest
    Quest.Status = EQuest_QuestStatus::Active;
    ActiveQuestIDs.AddUnique(QuestID);
    
    // Reset all objectives
    for (FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        Objective.CurrentProgress = 0;
        Objective.bIsCompleted = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Started quest %s"), *QuestID);
    
    // Broadcast quest started
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Active);
    
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }
    
    // Check if all required objectives are completed
    if (!AreAllObjectivesCompleted(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot complete quest %s - objectives not finished"), *QuestID);
        return false;
    }
    
    // Complete the quest
    Quest.Status = EQuest_QuestStatus::Completed;
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);
    
    // Give rewards
    GiveQuestRewards(Quest);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed quest %s"), *QuestID);
    
    // Broadcast quest completed
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Completed);
    
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }
    
    // Fail the quest
    Quest.Status = EQuest_QuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);
    FailedQuestIDs.AddUnique(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Failed quest %s"), *QuestID);
    
    // Broadcast quest failed
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Failed);
    
    return true;
}

bool UQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }
    
    // Find and update the objective
    for (FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            if (Objective.bIsCompleted)
            {
                return false; // Already completed
            }
            
            Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + ProgressAmount, Objective.RequiredQuantity);
            
            // Check if objective is now completed
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed objective %s for quest %s"), *ObjectiveID, *QuestID);
            }
            
            // Broadcast objective updated
            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, Objective.CurrentProgress);
            
            // Check if quest can be completed
            if (AreAllObjectivesCompleted(QuestID))
            {
                CompleteQuest(QuestID);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    return AllQuests[QuestID].Status == EQuest_QuestStatus::Active;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    return AllQuests[QuestID].Status == EQuest_QuestStatus::Completed;
}

TArray<FQuest_QuestData> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            ActiveQuests.Add(AllQuests[QuestID]);
        }
    }
    
    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuestManager::GetCompletedQuests() const
{
    TArray<FQuest_QuestData> CompletedQuests;
    
    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            CompletedQuests.Add(AllQuests[QuestID]);
        }
    }
    
    return CompletedQuests;
}

FQuest_QuestData UQuestManager::GetQuestData(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID];
    }
    
    return FQuest_QuestData(); // Return empty quest data
}

void UQuestManager::OnPlayerKilledActor(AActor* KilledActor)
{
    if (!KilledActor)
    {
        return;
    }
    
    FString ActorName = KilledActor->GetName();
    
    // Check all active quests for kill objectives
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& Quest = AllQuests[QuestID];
            
            for (FQuest_ObjectiveData& Objective : Quest.Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::KillTarget && 
                    !Objective.bIsCompleted &&
                    ActorName.Contains(Objective.TargetActorID))
                {
                    UpdateObjectiveProgress(QuestID, Objective.ObjectiveID, 1);
                    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Kill objective progress - %s (%d/%d)"), 
                           *Objective.ObjectiveID, Objective.CurrentProgress, Objective.RequiredQuantity);
                }
            }
        }
    }
}

void UQuestManager::OnPlayerCollectedItem(const FString& ItemID, int32 Quantity)
{
    // Check all active quests for collection objectives
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& Quest = AllQuests[QuestID];
            
            for (FQuest_ObjectiveData& Objective : Quest.Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::CollectItem && 
                    !Objective.bIsCompleted &&
                    ItemID.Contains(Objective.TargetActorID))
                {
                    UpdateObjectiveProgress(QuestID, Objective.ObjectiveID, Quantity);
                    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Collection objective progress - %s (%d/%d)"), 
                           *Objective.ObjectiveID, Objective.CurrentProgress, Objective.RequiredQuantity);
                }
            }
        }
    }
}

void UQuestManager::OnPlayerReachedLocation(const FVector& Location, float Radius)
{
    // Check all active quests for location objectives
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& Quest = AllQuests[QuestID];
            
            for (FQuest_ObjectiveData& Objective : Quest.Objectives)
            {
                if (Objective.Type == EQuest_ObjectiveType::GoToLocation && 
                    !Objective.bIsCompleted)
                {
                    float Distance = FVector::Dist(Location, Objective.TargetLocation);
                    if (Distance <= Radius)
                    {
                        UpdateObjectiveProgress(QuestID, Objective.ObjectiveID, 1);
                        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Location objective reached - %s"), *Objective.ObjectiveID);
                    }
                }
            }
        }
    }
}

bool UQuestManager::CheckQuestPrerequisites(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    const FQuest_QuestData& Quest = AllQuests[QuestID];
    
    // Check if all prerequisite quests are completed
    for (const FString& PrereqID : Quest.Prerequisites)
    {
        if (!IsQuestCompleted(PrereqID))
        {
            return false;
        }
    }
    
    return true;
}

bool UQuestManager::AreAllObjectivesCompleted(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    const FQuest_QuestData& Quest = AllQuests[QuestID];
    
    for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }
    
    return true;
}

void UQuestManager::GiveQuestRewards(const FQuest_QuestData& QuestData)
{
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Giving rewards for quest %s"), *QuestData.QuestID);
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Experience reward: %d"), QuestData.ExperienceReward);
    
    for (const FString& ItemReward : QuestData.ItemRewards)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Item reward: %s"), *ItemReward);
    }
    
    // TODO: Integrate with player progression and inventory systems
}

void UQuestManager::CreateDefaultSurvivalQuests()
{
    InitializeDefaultQuests();
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Recreated default survival quests"));
}

void UQuestManager::DebugPrintActiveQuests()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: === ACTIVE QUESTS ==="));
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            const FQuest_QuestData& Quest = AllQuests[QuestID];
            UE_LOG(LogTemp, Warning, TEXT("Quest: %s - %s"), *QuestID, *Quest.Title.ToString());
            
            for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
            {
                UE_LOG(LogTemp, Warning, TEXT("  Objective: %s (%d/%d) %s"), 
                       *Objective.ObjectiveID, 
                       Objective.CurrentProgress, 
                       Objective.RequiredQuantity,
                       Objective.bIsCompleted ? TEXT("[COMPLETED]") : TEXT("[ACTIVE]"));
            }
        }
    }
}

void UQuestManager::ResetAllQuests()
{
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    FailedQuestIDs.Empty();
    
    for (auto& QuestPair : AllQuests)
    {
        FQuest_QuestData& Quest = QuestPair.Value;
        Quest.Status = EQuest_QuestStatus::Available;
        
        for (FQuest_ObjectiveData& Objective : Quest.Objectives)
        {
            Objective.CurrentProgress = 0;
            Objective.bIsCompleted = false;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Reset all quests"));
}
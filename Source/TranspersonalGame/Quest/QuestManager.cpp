#include "QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UQuestManager::UQuestManager()
{
    // Constructor
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager initialized"));
    
    // Initialize default survival quests
    InitializeDefaultQuests();
}

void UQuestManager::Deinitialize()
{
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest not found: %s"), *QuestID);
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Available && QuestData.Status != EQuest_QuestStatus::NotStarted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest cannot be started: %s (Status: %d)"), *QuestID, (int32)QuestData.Status);
        return false;
    }

    if (!ArePrerequisitesMet(QuestData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for quest: %s"), *QuestID);
        return false;
    }

    QuestData.Status = EQuest_QuestStatus::Active;
    ActiveQuestIDs.AddUnique(QuestID);
    
    // Reset all objectives
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        Objective.CurrentProgress = 0;
        Objective.bIsCompleted = false;
    }

    OnQuestStarted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest started: %s"), *QuestID);
    
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Check if all non-optional objectives are completed
    bool bAllObjectivesCompleted = true;
    for (const FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            bAllObjectivesCompleted = false;
            break;
        }
    }

    if (!bAllObjectivesCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete quest - objectives not finished: %s"), *QuestID);
        return false;
    }

    QuestData.Status = EQuest_QuestStatus::Completed;
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);

    ProcessQuestCompletion(QuestID);
    OnQuestCompleted.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest completed: %s"), *QuestID);
    
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    QuestData.Status = EQuest_QuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);

    OnQuestFailed.Broadcast(QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest failed: %s"), *QuestID);
    
    return true;
}

bool UQuestManager::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Find the objective
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Clamp(Progress, 0, Objective.RequiredQuantity);
            
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("Objective completed: %s in quest %s"), *ObjectiveID, *QuestID);
            }

            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID);
            
            // Check if quest can be auto-completed
            bool bAllObjectivesCompleted = true;
            for (const FQuest_ObjectiveData& CheckObjective : QuestData.Objectives)
            {
                if (!CheckObjective.bIsOptional && !CheckObjective.bIsCompleted)
                {
                    bAllObjectivesCompleted = false;
                    break;
                }
            }
            
            if (bAllObjectivesCompleted)
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
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID].Status == EQuest_QuestStatus::Active;
    }
    return false;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID].Status == EQuest_QuestStatus::Completed;
    }
    return false;
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
    
    return FQuest_QuestData();
}

void UQuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    AllQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("Quest registered: %s"), *QuestData.QuestID);
}

void UQuestManager::UnregisterQuest(const FString& QuestID)
{
    AllQuests.Remove(QuestID);
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.Remove(QuestID);
    UE_LOG(LogTemp, Log, TEXT("Quest unregistered: %s"), *QuestID);
}

void UQuestManager::CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description, float TimeLimit)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = Title;
    NewQuest.Description = Description;
    NewQuest.QuestType = EQuest_QuestType::Survival;
    NewQuest.Status = EQuest_QuestStatus::Available;
    NewQuest.TimeLimit = TimeLimit;
    NewQuest.bIsRepeatable = false;
    
    RegisterQuest(NewQuest);
}

void UQuestManager::CreateHuntingQuest(const FString& QuestID, const FString& TargetDinosaurType, int32 RequiredKills)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = FText::FromString(FString::Printf(TEXT("Hunt %s"), *TargetDinosaurType));
    NewQuest.Description = FText::FromString(FString::Printf(TEXT("Hunt and kill %d %s for survival resources"), RequiredKills, *TargetDinosaurType));
    NewQuest.QuestType = EQuest_QuestType::Survival;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    // Add kill objective
    FQuest_ObjectiveData KillObjective;
    KillObjective.ObjectiveID = GenerateObjectiveID();
    KillObjective.Description = FText::FromString(FString::Printf(TEXT("Kill %d %s"), RequiredKills, *TargetDinosaurType));
    KillObjective.Type = EQuest_ObjectiveType::KillTarget;
    KillObjective.TargetActorID = TargetDinosaurType;
    KillObjective.RequiredQuantity = RequiredKills;
    KillObjective.CurrentProgress = 0;
    KillObjective.bIsCompleted = false;
    KillObjective.bIsOptional = false;
    
    NewQuest.Objectives.Add(KillObjective);
    
    RegisterQuest(NewQuest);
}

void UQuestManager::CreateExplorationQuest(const FString& QuestID, const FVector& TargetLocation, float AcceptanceRadius)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = FText::FromString(TEXT("Explore New Territory"));
    NewQuest.Description = FText::FromString(TEXT("Venture into unexplored territory to find new resources and shelter locations"));
    NewQuest.QuestType = EQuest_QuestType::Exploration;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    // Add exploration objective
    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveID = GenerateObjectiveID();
    ExploreObjective.Description = FText::FromString(TEXT("Reach the target location"));
    ExploreObjective.Type = EQuest_ObjectiveType::GoToLocation;
    ExploreObjective.TargetLocation = TargetLocation;
    ExploreObjective.RequiredQuantity = 1;
    ExploreObjective.CurrentProgress = 0;
    ExploreObjective.bIsCompleted = false;
    ExploreObjective.bIsOptional = false;
    
    NewQuest.Objectives.Add(ExploreObjective);
    
    RegisterQuest(NewQuest);
}

void UQuestManager::CreateGatheringQuest(const FString& QuestID, const FString& ItemType, int32 RequiredQuantity)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = FText::FromString(FString::Printf(TEXT("Gather %s"), *ItemType));
    NewQuest.Description = FText::FromString(FString::Printf(TEXT("Collect %d %s for crafting and survival"), RequiredQuantity, *ItemType));
    NewQuest.QuestType = EQuest_QuestType::Survival;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    // Add gathering objective
    FQuest_ObjectiveData GatherObjective;
    GatherObjective.ObjectiveID = GenerateObjectiveID();
    GatherObjective.Description = FText::FromString(FString::Printf(TEXT("Collect %d %s"), RequiredQuantity, *ItemType));
    GatherObjective.Type = EQuest_ObjectiveType::CollectItem;
    GatherObjective.TargetActorID = ItemType;
    GatherObjective.RequiredQuantity = RequiredQuantity;
    GatherObjective.CurrentProgress = 0;
    GatherObjective.bIsCompleted = false;
    GatherObjective.bIsOptional = false;
    
    NewQuest.Objectives.Add(GatherObjective);
    
    RegisterQuest(NewQuest);
}

void UQuestManager::InitializeDefaultQuests()
{
    // Create basic survival tutorial quests
    CreateGatheringQuest("GATHER_STICKS", "Sticks", 5);
    CreateGatheringQuest("GATHER_STONES", "Stones", 3);
    CreateHuntingQuest("HUNT_SMALL_PREY", "Compsognathus", 2);
    CreateExplorationQuest("EXPLORE_RIVER", FVector(2000, 1000, 100), 300.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Default survival quests initialized"));
}

bool UQuestManager::ArePrerequisitesMet(const FQuest_QuestData& QuestData) const
{
    for (const FString& PrerequisiteID : QuestData.Prerequisites)
    {
        if (!IsQuestCompleted(PrerequisiteID))
        {
            return false;
        }
    }
    return true;
}

void UQuestManager::ProcessQuestCompletion(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    const FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    // Award experience and rewards
    UE_LOG(LogTemp, Log, TEXT("Quest rewards processed for: %s (XP: %d)"), *QuestID, QuestData.ExperienceReward);
    
    // TODO: Implement actual reward system integration
}

FString UQuestManager::GenerateObjectiveID() const
{
    return FString::Printf(TEXT("OBJ_%d"), FMath::RandRange(1000, 9999));
}
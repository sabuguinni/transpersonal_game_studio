#include "Quest_QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UQuest_QuestManager::UQuest_QuestManager()
{
}

void UQuest_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized"));
    InitializeDefaultQuests();
}

void UQuest_QuestManager::Deinitialize()
{
    RegisteredQuests.Empty();
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    
    Super::Deinitialize();
}

void UQuest_QuestManager::InitializeDefaultQuests()
{
    // Survival Tutorial Quest
    FQuest_QuestData TutorialQuest;
    TutorialQuest.QuestID = TEXT("tutorial_survival");
    TutorialQuest.Title = TEXT("First Steps");
    TutorialQuest.Description = TEXT("Learn the basics of survival in the prehistoric world");
    TutorialQuest.Status = EQuestStatus::NotStarted;
    TutorialQuest.ExperienceReward = 100;
    
    FQuest_QuestObjective CollectSticks;
    CollectSticks.ObjectiveID = TEXT("collect_sticks");
    CollectSticks.Description = TEXT("Collect 5 sticks for crafting");
    CollectSticks.Type = EQuestObjectiveType::Collect;
    CollectSticks.TargetCount = 5;
    CollectSticks.CurrentCount = 0;
    CollectSticks.bCompleted = false;
    
    FQuest_QuestObjective CollectRocks;
    CollectRocks.ObjectiveID = TEXT("collect_rocks");
    CollectRocks.Description = TEXT("Collect 3 rocks for tools");
    CollectRocks.Type = EQuestObjectiveType::Collect;
    CollectRocks.TargetCount = 3;
    CollectRocks.CurrentCount = 0;
    CollectRocks.bCompleted = false;
    
    TutorialQuest.Objectives.Add(CollectSticks);
    TutorialQuest.Objectives.Add(CollectRocks);
    RegisterQuest(TutorialQuest);
    
    // Hunting Quest
    FQuest_QuestData HuntingQuest;
    HuntingQuest.QuestID = TEXT("first_hunt");
    HuntingQuest.Title = TEXT("The Hunt Begins");
    HuntingQuest.Description = TEXT("Prove your hunting skills by taking down a small dinosaur");
    HuntingQuest.Status = EQuestStatus::NotStarted;
    HuntingQuest.ExperienceReward = 250;
    
    FQuest_QuestObjective KillCompy;
    KillCompy.ObjectiveID = TEXT("kill_compy");
    KillCompy.Description = TEXT("Hunt down a Compsognathus");
    KillCompy.Type = EQuestObjectiveType::Kill;
    KillCompy.TargetCount = 1;
    KillCompy.CurrentCount = 0;
    KillCompy.bCompleted = false;
    
    HuntingQuest.Objectives.Add(KillCompy);
    RegisterQuest(HuntingQuest);
    
    // Exploration Quest
    FQuest_QuestData ExplorationQuest;
    ExplorationQuest.QuestID = TEXT("explore_valley");
    ExplorationQuest.Title = TEXT("Valley Explorer");
    ExplorationQuest.Description = TEXT("Discover the secrets of the prehistoric valley");
    ExplorationQuest.Status = EQuestStatus::NotStarted;
    ExplorationQuest.ExperienceReward = 150;
    
    FQuest_QuestObjective ReachWaterfall;
    ReachWaterfall.ObjectiveID = TEXT("reach_waterfall");
    ReachWaterfall.Description = TEXT("Find the hidden waterfall");
    ReachWaterfall.Type = EQuestObjectiveType::Reach;
    ReachWaterfall.TargetCount = 1;
    ReachWaterfall.CurrentCount = 0;
    ReachWaterfall.bCompleted = false;
    
    ExplorationQuest.Objectives.Add(ReachWaterfall);
    RegisterQuest(ExplorationQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Default quests initialized: %d quests registered"), RegisteredQuests.Num());
}

bool UQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (RegisteredQuests.Contains(QuestID))
    {
        FQuest_QuestData QuestData = RegisteredQuests[QuestID];
        QuestData.Status = EQuestStatus::Active;
        ActiveQuests.Add(QuestID, QuestData);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest started: %s"), *QuestData.Title);
        BroadcastQuestUpdate(QuestID);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Failed to start quest: %s not found"), *QuestID);
    return false;
}

bool UQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    if (ActiveQuests.Contains(QuestID))
    {
        FQuest_QuestData QuestData = ActiveQuests[QuestID];
        QuestData.Status = EQuestStatus::Completed;
        
        CompletedQuests.Add(QuestID, QuestData);
        ActiveQuests.Remove(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest completed: %s (Reward: %d XP)"), *QuestData.Title, QuestData.ExperienceReward);
        BroadcastQuestUpdate(QuestID);
        return true;
    }
    
    return false;
}

bool UQuest_QuestManager::UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (ActiveQuests.Contains(QuestID))
    {
        FQuest_QuestData& QuestData = ActiveQuests[QuestID];
        
        for (FQuest_QuestObjective& Objective : QuestData.Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID && !Objective.bCompleted)
            {
                Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);
                
                if (Objective.CurrentCount >= Objective.TargetCount)
                {
                    Objective.bCompleted = true;
                    UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), *Objective.Description);
                }
                
                CheckQuestCompletion(QuestID);
                BroadcastQuestUpdate(QuestID);
                return true;
            }
        }
    }
    
    return false;
}

FQuest_QuestData UQuest_QuestManager::GetQuestData(const FString& QuestID)
{
    if (ActiveQuests.Contains(QuestID))
    {
        return ActiveQuests[QuestID];
    }
    else if (CompletedQuests.Contains(QuestID))
    {
        return CompletedQuests[QuestID];
    }
    else if (RegisteredQuests.Contains(QuestID))
    {
        return RegisteredQuests[QuestID];
    }
    
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetActiveQuests()
{
    TArray<FQuest_QuestData> Result;
    for (const auto& QuestPair : ActiveQuests)
    {
        Result.Add(QuestPair.Value);
    }
    return Result;
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetCompletedQuests()
{
    TArray<FQuest_QuestData> Result;
    for (const auto& QuestPair : CompletedQuests)
    {
        Result.Add(QuestPair.Value);
    }
    return Result;
}

void UQuest_QuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    RegisteredQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("Quest registered: %s"), *QuestData.Title);
}

void UQuest_QuestManager::OnDinosaurKilled(const FString& DinosaurType)
{
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_QuestData& QuestData = QuestPair.Value;
        
        for (FQuest_QuestObjective& Objective : QuestData.Objectives)
        {
            if (Objective.Type == EQuestObjectiveType::Kill && 
                Objective.ObjectiveID.Contains(DinosaurType.ToLower()) &&
                !Objective.bCompleted)
            {
                UpdateObjective(QuestData.QuestID, Objective.ObjectiveID, 1);
                break;
            }
        }
    }
}

void UQuest_QuestManager::OnItemCollected(const FString& ItemType)
{
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_QuestData& QuestData = QuestPair.Value;
        
        for (FQuest_QuestObjective& Objective : QuestData.Objectives)
        {
            if (Objective.Type == EQuestObjectiveType::Collect && 
                Objective.ObjectiveID.Contains(ItemType.ToLower()) &&
                !Objective.bCompleted)
            {
                UpdateObjective(QuestData.QuestID, Objective.ObjectiveID, 1);
                break;
            }
        }
    }
}

void UQuest_QuestManager::OnLocationReached(const FString& LocationName)
{
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_QuestData& QuestData = QuestPair.Value;
        
        for (FQuest_QuestObjective& Objective : QuestData.Objectives)
        {
            if (Objective.Type == EQuestObjectiveType::Reach && 
                Objective.ObjectiveID.Contains(LocationName.ToLower()) &&
                !Objective.bCompleted)
            {
                UpdateObjective(QuestData.QuestID, Objective.ObjectiveID, 1);
                break;
            }
        }
    }
}

void UQuest_QuestManager::CheckQuestCompletion(const FString& QuestID)
{
    if (ActiveQuests.Contains(QuestID))
    {
        FQuest_QuestData& QuestData = ActiveQuests[QuestID];
        bool bAllObjectivesComplete = true;
        
        for (const FQuest_QuestObjective& Objective : QuestData.Objectives)
        {
            if (!Objective.bCompleted)
            {
                bAllObjectivesComplete = false;
                break;
            }
        }
        
        if (bAllObjectivesComplete)
        {
            CompleteQuest(QuestID);
        }
    }
}

void UQuest_QuestManager::BroadcastQuestUpdate(const FString& QuestID)
{
    // TODO: Implement quest update broadcasting to UI
    UE_LOG(LogTemp, Log, TEXT("Quest update broadcast: %s"), *QuestID);
}
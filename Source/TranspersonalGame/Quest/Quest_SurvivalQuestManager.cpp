#include "Quest_SurvivalQuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UQuest_SurvivalQuestManager::UQuest_SurvivalQuestManager()
{
    NextQuestID = 1;
}

void UQuest_SurvivalQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalQuestManager initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
    
    // Start quest timer for time-based updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            QuestTimerHandle,
            this,
            &UQuest_SurvivalQuestManager::UpdateQuestTimers,
            1.0f,
            true
        );
    }
}

void UQuest_SurvivalQuestManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestTimerHandle);
    }
    
    Super::Deinitialize();
}

void UQuest_SurvivalQuestManager::StartQuest(const FString& QuestID)
{
    for (FQuest_SurvivalQuest& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Status == EQuest_QuestStatus::NotStarted)
        {
            Quest.Status = EQuest_QuestStatus::Active;
            ActiveQuests.Add(Quest);
            
            UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *Quest.QuestName);
            BroadcastQuestUpdate(Quest);
            break;
        }
    }
}

void UQuest_SurvivalQuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].Status = EQuest_QuestStatus::Completed;
            
            UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *ActiveQuests[i].QuestName);
            UE_LOG(LogTemp, Warning, TEXT("Experience gained: %d"), ActiveQuests[i].ExperienceReward);
            
            BroadcastQuestUpdate(ActiveQuests[i]);
            
            // Update in AllQuests array
            for (FQuest_SurvivalQuest& Quest : AllQuests)
            {
                if (Quest.QuestID == QuestID)
                {
                    Quest.Status = EQuest_QuestStatus::Completed;
                    break;
                }
            }
            
            ActiveQuests.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalQuestManager::FailQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].Status = EQuest_QuestStatus::Failed;
            
            UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s"), *ActiveQuests[i].QuestName);
            BroadcastQuestUpdate(ActiveQuests[i]);
            
            // Update in AllQuests array
            for (FQuest_SurvivalQuest& Quest : AllQuests)
            {
                if (Quest.QuestID == QuestID)
                {
                    Quest.Status = EQuest_QuestStatus::Failed;
                    break;
                }
            }
            
            ActiveQuests.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalQuestManager::UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID && Quest.Objectives.IsValidIndex(ObjectiveIndex))
        {
            Quest.Objectives[ObjectiveIndex].CurrentCount = FMath::Min(
                Quest.Objectives[ObjectiveIndex].CurrentCount + Progress,
                Quest.Objectives[ObjectiveIndex].TargetCount
            );
            
            if (Quest.Objectives[ObjectiveIndex].CurrentCount >= Quest.Objectives[ObjectiveIndex].TargetCount)
            {
                Quest.Objectives[ObjectiveIndex].bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), *Quest.Objectives[ObjectiveIndex].Description);
            }
            
            CheckQuestCompletion(Quest);
            BroadcastQuestUpdate(Quest);
            break;
        }
    }
}

bool UQuest_SurvivalQuestManager::IsQuestActive(const FString& QuestID) const
{
    for (const FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

TArray<FQuest_SurvivalQuest> UQuest_SurvivalQuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

FQuest_SurvivalQuest UQuest_SurvivalQuestManager::GetQuest(const FString& QuestID) const
{
    for (const FQuest_SurvivalQuest& Quest : AllQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    return FQuest_SurvivalQuest();
}

void UQuest_SurvivalQuestManager::CreateHuntQuest(const FString& TargetDinosaur, int32 Count)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = GenerateQuestID();
    NewQuest.QuestName = FString::Printf(TEXT("Hunt %s"), *TargetDinosaur);
    NewQuest.QuestDescription = FString::Printf(TEXT("Hunt and kill %d %s for survival"), Count, *TargetDinosaur);
    NewQuest.QuestType = EQuest_QuestType::Hunt;
    NewQuest.Status = EQuest_QuestStatus::NotStarted;
    NewQuest.ExperienceReward = Count * 50;
    
    FQuest_QuestObjective Objective;
    Objective.Description = FString::Printf(TEXT("Kill %d %s"), Count, *TargetDinosaur);
    Objective.TargetCount = Count;
    Objective.CurrentCount = 0;
    Objective.bIsCompleted = false;
    
    NewQuest.Objectives.Add(Objective);
    AllQuests.Add(NewQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Created hunt quest: %s"), *NewQuest.QuestName);
}

void UQuest_SurvivalQuestManager::CreateGatherQuest(const FString& ResourceType, int32 Count)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = GenerateQuestID();
    NewQuest.QuestName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    NewQuest.QuestDescription = FString::Printf(TEXT("Collect %d %s for crafting and survival"), Count, *ResourceType);
    NewQuest.QuestType = EQuest_QuestType::Gather;
    NewQuest.Status = EQuest_QuestStatus::NotStarted;
    NewQuest.ExperienceReward = Count * 10;
    
    FQuest_QuestObjective Objective;
    Objective.Description = FString::Printf(TEXT("Gather %d %s"), Count, *ResourceType);
    Objective.TargetCount = Count;
    Objective.CurrentCount = 0;
    Objective.bIsCompleted = false;
    
    NewQuest.Objectives.Add(Objective);
    AllQuests.Add(NewQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Created gather quest: %s"), *NewQuest.QuestName);
}

void UQuest_SurvivalQuestManager::CreateSurvivalQuest(float Duration)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = GenerateQuestID();
    NewQuest.QuestName = TEXT("Survival Challenge");
    NewQuest.QuestDescription = FString::Printf(TEXT("Survive for %.0f seconds in the wilderness"), Duration);
    NewQuest.QuestType = EQuest_QuestType::Survive;
    NewQuest.Status = EQuest_QuestStatus::NotStarted;
    NewQuest.ExperienceReward = FMath::RoundToInt(Duration / 10.0f);
    NewQuest.TimeLimit = Duration;
    NewQuest.bHasTimeLimit = true;
    
    FQuest_QuestObjective Objective;
    Objective.Description = FString::Printf(TEXT("Survive for %.0f seconds"), Duration);
    Objective.TargetCount = 1;
    Objective.CurrentCount = 0;
    Objective.bIsCompleted = false;
    
    NewQuest.Objectives.Add(Objective);
    AllQuests.Add(NewQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Created survival quest: %s"), *NewQuest.QuestName);
}

void UQuest_SurvivalQuestManager::CreateExploreQuest(const FString& LocationName)
{
    FQuest_SurvivalQuest NewQuest;
    NewQuest.QuestID = GenerateQuestID();
    NewQuest.QuestName = FString::Printf(TEXT("Explore %s"), *LocationName);
    NewQuest.QuestDescription = FString::Printf(TEXT("Discover and explore the %s area"), *LocationName);
    NewQuest.QuestType = EQuest_QuestType::Explore;
    NewQuest.Status = EQuest_QuestStatus::NotStarted;
    NewQuest.ExperienceReward = 75;
    
    FQuest_QuestObjective Objective;
    Objective.Description = FString::Printf(TEXT("Reach %s"), *LocationName);
    Objective.TargetCount = 1;
    Objective.CurrentCount = 0;
    Objective.bIsCompleted = false;
    
    NewQuest.Objectives.Add(Objective);
    AllQuests.Add(NewQuest);
    
    UE_LOG(LogTemp, Warning, TEXT("Created explore quest: %s"), *NewQuest.QuestName);
}

void UQuest_SurvivalQuestManager::OnDinosaurKilled(const FString& DinosaurType)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Hunt)
        {
            for (int32 i = 0; i < Quest.Objectives.Num(); i++)
            {
                if (Quest.Objectives[i].Description.Contains(DinosaurType))
                {
                    UpdateObjective(Quest.QuestID, i, 1);
                    break;
                }
            }
        }
    }
}

void UQuest_SurvivalQuestManager::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Gather)
        {
            for (int32 i = 0; i < Quest.Objectives.Num(); i++)
            {
                if (Quest.Objectives[i].Description.Contains(ResourceType))
                {
                    UpdateObjective(Quest.QuestID, i, Amount);
                    break;
                }
            }
        }
    }
}

void UQuest_SurvivalQuestManager::OnLocationDiscovered(const FString& LocationName)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Explore)
        {
            for (int32 i = 0; i < Quest.Objectives.Num(); i++)
            {
                if (Quest.Objectives[i].Description.Contains(LocationName))
                {
                    UpdateObjective(Quest.QuestID, i, 1);
                    break;
                }
            }
        }
    }
}

void UQuest_SurvivalQuestManager::OnPlayerSurvived(float Duration)
{
    for (FQuest_SurvivalQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Survive)
        {
            for (int32 i = 0; i < Quest.Objectives.Num(); i++)
            {
                if (!Quest.Objectives[i].bIsCompleted)
                {
                    UpdateObjective(Quest.QuestID, i, 1);
                    break;
                }
            }
        }
    }
}

void UQuest_SurvivalQuestManager::InitializeDefaultQuests()
{
    // Create default starter quests
    CreateGatherQuest(TEXT("Stone"), 5);
    CreateGatherQuest(TEXT("Wood"), 3);
    CreateHuntQuest(TEXT("Raptor"), 1);
    CreateExploreQuest(TEXT("Northern Territory"));
    CreateSurvivalQuest(300.0f); // 5 minutes
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default quests"), AllQuests.Num());
}

FString UQuest_SurvivalQuestManager::GenerateQuestID()
{
    return FString::Printf(TEXT("QUEST_%d"), NextQuestID++);
}

void UQuest_SurvivalQuestManager::CheckQuestCompletion(FQuest_SurvivalQuest& Quest)
{
    bool bAllObjectivesComplete = true;
    for (const FQuest_QuestObjective& Objective : Quest.Objectives)
    {
        if (!Objective.bIsCompleted)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }
    
    if (bAllObjectivesComplete)
    {
        CompleteQuest(Quest.QuestID);
    }
}

void UQuest_SurvivalQuestManager::BroadcastQuestUpdate(const FQuest_SurvivalQuest& Quest)
{
    // Log quest update for debugging
    UE_LOG(LogTemp, Warning, TEXT("Quest Update - %s: %s"), 
           *Quest.QuestName, 
           *UEnum::GetValueAsString(Quest.Status));
    
    // In a full implementation, this would broadcast to UI and other systems
}

void UQuest_SurvivalQuestManager::UpdateQuestTimers()
{
    for (int32 i = ActiveQuests.Num() - 1; i >= 0; i--)
    {
        FQuest_SurvivalQuest& Quest = ActiveQuests[i];
        
        if (Quest.bHasTimeLimit)
        {
            Quest.TimeLimit -= 1.0f;
            
            if (Quest.TimeLimit <= 0.0f)
            {
                FailQuest(Quest.QuestID);
            }
        }
    }
}
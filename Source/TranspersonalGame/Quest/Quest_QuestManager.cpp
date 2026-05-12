#include "Quest_QuestManager.h"
#include "../TranspersonalCharacter.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"

UQuest_QuestManager::UQuest_QuestManager()
{
    MaxActiveQuests = 10;
    bAutoCompleteObjectives = true;
    QuestDataTable = nullptr;
}

void UQuest_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Quest Manager initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
    
    // Load quest database if available
    LoadQuestDatabase();
}

void UQuest_QuestManager::Deinitialize()
{
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    
    Super::Deinitialize();
}

bool UQuest_QuestManager::StartQuest(const FString& QuestID, ATranspersonalCharacter* Player)
{
    if (!Player || QuestID.IsEmpty())
    {
        return false;
    }

    // Check if quest is already active
    if (IsQuestActive(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s is already active"), *QuestID);
        return false;
    }

    // Check if quest was already completed and is not repeatable
    if (IsQuestCompleted(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s already completed"), *QuestID);
        return false;
    }

    // Check active quest limit
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active quests reached"));
        return false;
    }

    // Find quest data (for now, create a default survival quest)
    FQuest_QuestData QuestData;
    QuestData.QuestID = QuestID;
    
    if (QuestID == TEXT("SURVIVAL_GATHER_WOOD"))
    {
        QuestData.QuestName = TEXT("Gather Wood");
        QuestData.QuestDescription = TEXT("Collect wood to build basic tools and shelter");
        QuestData.RequiredLevel = 1;
        QuestData.ExperienceReward = 50.0f;
        QuestData.bIsMainQuest = false;
        QuestData.bIsRepeatable = true;

        FQuest_ObjectiveData Objective;
        Objective.ObjectiveID = TEXT("COLLECT_WOOD_10");
        Objective.Description = TEXT("Collect 10 pieces of wood");
        Objective.RequiredResourceType = ESurvivalResourceType::Wood;
        Objective.RequiredAmount = 10;
        Objective.RewardExperience = 25.0f;
        Objective.bIsCompleted = false;

        QuestData.Objectives.Add(Objective);
    }
    else if (QuestID == TEXT("SURVIVAL_HUNT_RAPTOR"))
    {
        QuestData.QuestName = TEXT("Hunt Raptor");
        QuestData.QuestDescription = TEXT("Hunt a raptor for meat and materials");
        QuestData.RequiredLevel = 3;
        QuestData.ExperienceReward = 150.0f;
        QuestData.bIsMainQuest = false;
        QuestData.bIsRepeatable = true;

        FQuest_ObjectiveData Objective;
        Objective.ObjectiveID = TEXT("KILL_RAPTOR_1");
        Objective.Description = TEXT("Kill 1 raptor");
        Objective.RequiredResourceType = ESurvivalResourceType::Meat;
        Objective.RequiredAmount = 1;
        Objective.RewardExperience = 75.0f;
        Objective.bIsCompleted = false;

        QuestData.Objectives.Add(Objective);
    }

    // Create active quest
    FQuest_ActiveQuest NewActiveQuest;
    NewActiveQuest.QuestID = QuestID;
    NewActiveQuest.QuestData = QuestData;
    NewActiveQuest.Status = EQuest_QuestStatus::Active;
    NewActiveQuest.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Initialize objective completion status
    for (int32 i = 0; i < QuestData.Objectives.Num(); i++)
    {
        NewActiveQuest.ObjectiveCompletionStatus.Add(false);
    }

    ActiveQuests.Add(NewActiveQuest);

    UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestData.QuestName);
    
    return true;
}

bool UQuest_QuestManager::CompleteQuest(const FString& QuestID, ATranspersonalCharacter* Player)
{
    if (!Player || QuestID.IsEmpty())
    {
        return false;
    }

    int32 QuestIndex = FindActiveQuestIndex(QuestID);
    if (QuestIndex == INDEX_NONE)
    {
        return false;
    }

    FQuest_ActiveQuest& Quest = ActiveQuests[QuestIndex];
    Quest.Status = EQuest_QuestStatus::Completed;

    // Give rewards
    GiveQuestRewards(Quest.QuestData, Player);

    // Add to completed quests
    CompletedQuests.AddUnique(QuestID);

    // Remove from active quests
    ActiveQuests.RemoveAt(QuestIndex);

    UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *Quest.QuestData.QuestName);

    return true;
}

bool UQuest_QuestManager::AbandonQuest(const FString& QuestID, ATranspersonalCharacter* Player)
{
    if (QuestID.IsEmpty())
    {
        return false;
    }

    int32 QuestIndex = FindActiveQuestIndex(QuestID);
    if (QuestIndex == INDEX_NONE)
    {
        return false;
    }

    ActiveQuests.RemoveAt(QuestIndex);
    UE_LOG(LogTemp, Log, TEXT("Abandoned quest: %s"), *QuestID);

    return true;
}

void UQuest_QuestManager::UpdateQuestProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    int32 QuestIndex = FindActiveQuestIndex(QuestID);
    if (QuestIndex == INDEX_NONE)
    {
        return;
    }

    FQuest_ActiveQuest& Quest = ActiveQuests[QuestIndex];
    
    // Find objective
    for (int32 i = 0; i < Quest.QuestData.Objectives.Num(); i++)
    {
        if (Quest.QuestData.Objectives[i].ObjectiveID == ObjectiveID)
        {
            if (Progress >= Quest.QuestData.Objectives[i].RequiredAmount)
            {
                Quest.ObjectiveCompletionStatus[i] = true;
                UE_LOG(LogTemp, Log, TEXT("Objective completed: %s"), *ObjectiveID);
            }
            break;
        }
    }

    // Check if quest is complete
    if (bAutoCompleteObjectives && CheckQuestCompletion(Quest))
    {
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)))
        {
            CompleteQuest(QuestID, Player);
        }
    }
}

TArray<FQuest_ActiveQuest> UQuest_QuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

FQuest_ActiveQuest UQuest_QuestManager::GetQuestByID(const FString& QuestID) const
{
    for (const FQuest_ActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    return FQuest_ActiveQuest();
}

bool UQuest_QuestManager::IsQuestActive(const FString& QuestID) const
{
    return FindActiveQuestIndex(QuestID) != INDEX_NONE;
}

bool UQuest_QuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuests.Contains(QuestID);
}

void UQuest_QuestManager::OnResourceCollected(ESurvivalResourceType ResourceType, int32 Amount)
{
    for (FQuest_ActiveQuest& Quest : ActiveQuests)
    {
        for (int32 i = 0; i < Quest.QuestData.Objectives.Num(); i++)
        {
            const FQuest_ObjectiveData& Objective = Quest.QuestData.Objectives[i];
            if (Objective.RequiredResourceType == ResourceType && !Quest.ObjectiveCompletionStatus[i])
            {
                UpdateQuestProgress(Quest.QuestID, Objective.ObjectiveID, Amount);
            }
        }
    }
}

void UQuest_QuestManager::OnDinosaurKilled(EDinosaurSpecies Species)
{
    // Update hunting quests
    for (FQuest_ActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID.Contains(TEXT("HUNT")) || Quest.QuestID.Contains(TEXT("KILL")))
        {
            for (int32 i = 0; i < Quest.QuestData.Objectives.Num(); i++)
            {
                if (!Quest.ObjectiveCompletionStatus[i])
                {
                    UpdateQuestProgress(Quest.QuestID, Quest.QuestData.Objectives[i].ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_QuestManager::OnAreaExplored(const FString& AreaName)
{
    // Update exploration quests
    for (FQuest_ActiveQuest& Quest : ActiveQuests)
    {
        if (Quest.QuestID.Contains(TEXT("EXPLORE")) || Quest.QuestID.Contains(TEXT("DISCOVER")))
        {
            for (int32 i = 0; i < Quest.QuestData.Objectives.Num(); i++)
            {
                if (!Quest.ObjectiveCompletionStatus[i])
                {
                    UpdateQuestProgress(Quest.QuestID, Quest.QuestData.Objectives[i].ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_QuestManager::LoadQuestDatabase()
{
    // Load quest data table if available
    if (QuestDataTable)
    {
        TArray<FQuest_QuestData*> AllQuests;
        QuestDataTable->GetAllRows<FQuest_QuestData>(TEXT("LoadQuestDatabase"), AllQuests);
        
        UE_LOG(LogTemp, Log, TEXT("Loaded %d quests from data table"), AllQuests.Num());
    }
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetAvailableQuests(int32 PlayerLevel) const
{
    TArray<FQuest_QuestData> AvailableQuests;
    
    // Add default survival quests
    FQuest_QuestData GatherWoodQuest;
    GatherWoodQuest.QuestID = TEXT("SURVIVAL_GATHER_WOOD");
    GatherWoodQuest.QuestName = TEXT("Gather Wood");
    GatherWoodQuest.RequiredLevel = 1;
    if (PlayerLevel >= GatherWoodQuest.RequiredLevel && !IsQuestCompleted(GatherWoodQuest.QuestID))
    {
        AvailableQuests.Add(GatherWoodQuest);
    }

    FQuest_QuestData HuntRaptorQuest;
    HuntRaptorQuest.QuestID = TEXT("SURVIVAL_HUNT_RAPTOR");
    HuntRaptorQuest.QuestName = TEXT("Hunt Raptor");
    HuntRaptorQuest.RequiredLevel = 3;
    if (PlayerLevel >= HuntRaptorQuest.RequiredLevel && !IsQuestCompleted(HuntRaptorQuest.QuestID))
    {
        AvailableQuests.Add(HuntRaptorQuest);
    }

    return AvailableQuests;
}

void UQuest_QuestManager::InitializeDefaultQuests()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing default quest system"));
}

bool UQuest_QuestManager::CheckQuestCompletion(FQuest_ActiveQuest& Quest)
{
    for (bool bCompleted : Quest.ObjectiveCompletionStatus)
    {
        if (!bCompleted)
        {
            return false;
        }
    }
    return true;
}

void UQuest_QuestManager::GiveQuestRewards(const FQuest_QuestData& QuestData, ATranspersonalCharacter* Player)
{
    if (!Player)
    {
        return;
    }

    // Give experience reward
    // Player->AddExperience(QuestData.ExperienceReward);

    // Give item rewards
    for (ESurvivalResourceType ResourceType : QuestData.ItemRewards)
    {
        // Player->AddResource(ResourceType, 1);
    }

    UE_LOG(LogTemp, Log, TEXT("Gave quest rewards for: %s"), *QuestData.QuestName);
}

int32 UQuest_QuestManager::FindActiveQuestIndex(const FString& QuestID) const
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            return i;
        }
    }
    return INDEX_NONE;
}
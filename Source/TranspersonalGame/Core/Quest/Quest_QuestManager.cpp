#include "Quest_QuestManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UQuest_QuestManager::UQuest_QuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxActiveQuests = 5;
}

void UQuest_QuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultQuests();
}

void UQuest_QuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update quest progress for active quests
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.bIsActive && !Quest.bIsCompleted)
        {
            UpdateQuestProgress(Quest.QuestID);
        }
    }
}

bool UQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest %s: Maximum active quests reached"), *QuestID);
        return false;
    }

    // Check if quest is already active
    for (const FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest %s is already active"), *QuestID);
            return false;
        }
    }

    // Create quest data based on QuestID
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.bIsActive = true;
    NewQuest.bIsCompleted = false;
    NewQuest.QuestProgress = 0.0f;

    // Set quest details based on ID
    if (QuestID == TEXT("HUNT_RAPTOR"))
    {
        NewQuest.QuestTitle = TEXT("Hunt the Pack");
        NewQuest.QuestDescription = TEXT("Hunt 3 Velociraptors to protect the settlement");
        
        // Add hunt objective
        FQuest_ObjectiveData HuntObjective;
        HuntObjective.ObjectiveID = TEXT("HUNT_RAPTOR_OBJ");
        HuntObjective.ObjectiveText = TEXT("Kill 3 Velociraptors");
        HuntObjective.ObjectiveType = EQuestObjectiveType::Kill;
        HuntObjective.TargetActorClass = TEXT("Velociraptor");
        HuntObjective.RequiredCount = 3;
        HuntObjective.CurrentCount = 0;
        
        QuestObjectives.Add(HuntObjective.ObjectiveID, HuntObjective);
        NewQuest.ObjectiveIDs.Add(HuntObjective.ObjectiveID);
    }
    else if (QuestID == TEXT("GATHER_CROWD"))
    {
        NewQuest.QuestTitle = TEXT("Rally the Tribe");
        NewQuest.QuestDescription = TEXT("Gather tribe members at the central waypoint");
        
        // Add crowd gathering objective
        FQuest_ObjectiveData GatherObjective;
        GatherObjective.ObjectiveID = TEXT("GATHER_CROWD_OBJ");
        GatherObjective.ObjectiveText = TEXT("Gather 10 tribe members");
        GatherObjective.ObjectiveType = EQuestObjectiveType::Interact;
        GatherObjective.TargetActorClass = TEXT("CrowdEntity");
        GatherObjective.RequiredCount = 10;
        GatherObjective.CurrentCount = 0;
        
        QuestObjectives.Add(GatherObjective.ObjectiveID, GatherObjective);
        NewQuest.ObjectiveIDs.Add(GatherObjective.ObjectiveID);
        
        // Register crowd event trigger
        RegisterCrowdQuestTrigger(QuestID, TEXT("GATHERING"));
    }
    else if (QuestID == TEXT("EXPLORE_TERRITORY"))
    {
        NewQuest.QuestTitle = TEXT("Scout New Lands");
        NewQuest.QuestDescription = TEXT("Explore 5 different areas to map the territory");
        
        // Add exploration objective
        FQuest_ObjectiveData ExploreObjective;
        ExploreObjective.ObjectiveID = TEXT("EXPLORE_TERRITORY_OBJ");
        ExploreObjective.ObjectiveText = TEXT("Visit 5 exploration points");
        ExploreObjective.ObjectiveType = EQuestObjectiveType::Reach;
        ExploreObjective.TargetActorClass = TEXT("ExplorationPoint");
        ExploreObjective.RequiredCount = 5;
        ExploreObjective.CurrentCount = 0;
        
        QuestObjectives.Add(ExploreObjective.ObjectiveID, ExploreObjective);
        NewQuest.ObjectiveIDs.Add(ExploreObjective.ObjectiveID);
    }

    ActiveQuests.Add(NewQuest);
    UE_LOG(LogTemp, Log, TEXT("Started quest: %s - %s"), *NewQuest.QuestID, *NewQuest.QuestTitle);
    return true;
}

bool UQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests[i].bIsCompleted = true;
            ActiveQuests[i].bIsActive = false;
            ActiveQuests[i].QuestProgress = 1.0f;
            
            // Move to completed quests
            CompletedQuests.Add(ActiveQuests[i]);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestID);
            return true;
        }
    }
    return false;
}

bool UQuest_QuestManager::UpdateObjective(const FString& ObjectiveID, int32 ProgressAmount)
{
    if (FQuest_ObjectiveData* Objective = QuestObjectives.Find(ObjectiveID))
    {
        Objective->CurrentCount = FMath::Min(Objective->CurrentCount + ProgressAmount, Objective->RequiredCount);
        
        if (Objective->CurrentCount >= Objective->RequiredCount)
        {
            Objective->bIsCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("Objective completed: %s"), *ObjectiveID);
        }
        
        // Find and update quest progress
        for (FQuest_QuestData& Quest : ActiveQuests)
        {
            if (Quest.ObjectiveIDs.Contains(ObjectiveID))
            {
                CheckQuestCompletion(Quest.QuestID);
                break;
            }
        }
        
        return true;
    }
    return false;
}

FQuest_QuestData UQuest_QuestManager::GetQuestData(const FString& QuestID)
{
    for (const FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    for (const FQuest_QuestData& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return Quest;
        }
    }
    
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetActiveQuests()
{
    return ActiveQuests;
}

bool UQuest_QuestManager::IsQuestActive(const FString& QuestID)
{
    for (const FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID && Quest.bIsActive)
        {
            return true;
        }
    }
    return false;
}

bool UQuest_QuestManager::IsQuestCompleted(const FString& QuestID)
{
    for (const FQuest_QuestData& Quest : CompletedQuests)
    {
        if (Quest.QuestID == QuestID && Quest.bIsCompleted)
        {
            return true;
        }
    }
    return false;
}

bool UQuest_QuestManager::AddObjectiveToQuest(const FString& QuestID, const FQuest_ObjectiveData& ObjectiveData)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            Quest.ObjectiveIDs.Add(ObjectiveData.ObjectiveID);
            QuestObjectives.Add(ObjectiveData.ObjectiveID, ObjectiveData);
            return true;
        }
    }
    return false;
}

FQuest_ObjectiveData UQuest_QuestManager::GetObjectiveData(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = QuestObjectives.Find(ObjectiveID))
    {
        return *Objective;
    }
    return FQuest_ObjectiveData();
}

bool UQuest_QuestManager::IsObjectiveCompleted(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = QuestObjectives.Find(ObjectiveID))
    {
        return Objective->bIsCompleted;
    }
    return false;
}

void UQuest_QuestManager::OnCrowdEventTriggered(const FString& EventType, const FVector& Location)
{
    if (FString* QuestID = CrowdEventToQuestMap.Find(EventType))
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd event %s triggered for quest %s at location %s"), 
               *EventType, **QuestID, *Location.ToString());
        
        // Update relevant quest objectives
        if (*QuestID == TEXT("GATHER_CROWD") && EventType == TEXT("GATHERING"))
        {
            UpdateObjective(TEXT("GATHER_CROWD_OBJ"), 1);
        }
    }
}

void UQuest_QuestManager::RegisterCrowdQuestTrigger(const FString& QuestID, const FString& EventType)
{
    CrowdEventToQuestMap.Add(EventType, QuestID);
    UE_LOG(LogTemp, Log, TEXT("Registered crowd trigger: %s -> %s"), *EventType, *QuestID);
}

void UQuest_QuestManager::InitializeDefaultQuests()
{
    UE_LOG(LogTemp, Log, TEXT("Quest Manager initialized with default quest templates"));
}

void UQuest_QuestManager::CheckQuestCompletion(const FString& QuestID)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            bool bAllObjectivesCompleted = true;
            
            for (const FString& ObjectiveID : Quest.ObjectiveIDs)
            {
                if (!IsObjectiveCompleted(ObjectiveID))
                {
                    bAllObjectivesCompleted = false;
                    break;
                }
            }
            
            if (bAllObjectivesCompleted)
            {
                CompleteQuest(QuestID);
            }
            break;
        }
    }
}

void UQuest_QuestManager::UpdateQuestProgress(const FString& QuestID)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            if (Quest.ObjectiveIDs.Num() > 0)
            {
                int32 CompletedObjectives = 0;
                for (const FString& ObjectiveID : Quest.ObjectiveIDs)
                {
                    if (IsObjectiveCompleted(ObjectiveID))
                    {
                        CompletedObjectives++;
                    }
                }
                Quest.QuestProgress = static_cast<float>(CompletedObjectives) / Quest.ObjectiveIDs.Num();
            }
            break;
        }
    }
}
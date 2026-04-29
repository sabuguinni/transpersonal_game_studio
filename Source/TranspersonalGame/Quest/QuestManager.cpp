#include "QuestManager.h"
#include "QuestGiver.h"
#include "QuestNPC.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UQuest_QuestManager::UQuest_QuestManager()
{
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    QuestGivers.Empty();
}

void UQuest_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized"));
    InitializeDefaultQuests();
}

void UQuest_QuestManager::Deinitialize()
{
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    QuestGivers.Empty();
    
    Super::Deinitialize();
}

bool UQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest not found: %s"), *QuestID);
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest not available: %s"), *QuestID);
        return false;
    }

    QuestData.Status = EQuest_QuestStatus::Active;
    ActiveQuestIDs.AddUnique(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *QuestID);
    return true;
}

bool UQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (!ValidateQuestCompletion(QuestID))
    {
        return false;
    }

    QuestData.Status = EQuest_QuestStatus::Completed;
    ActiveQuestIDs.Remove(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *QuestID);
    return true;
}

bool UQuest_QuestManager::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    QuestData.Status = EQuest_QuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);
    
    UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s"), *QuestID);
    return true;
}

void UQuest_QuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Min(Progress, Objective.RequiredQuantity);
            
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s in quest %s"), *ObjectiveID, *QuestID);
            }
            break;
        }
    }
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetActiveQuests() const
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

FQuest_QuestData UQuest_QuestManager::GetQuestData(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID];
    }
    
    return FQuest_QuestData();
}

void UQuest_QuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    AllQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Warning, TEXT("Registered quest: %s"), *QuestData.QuestID);
}

void UQuest_QuestManager::RegisterQuestGiver(UQuest_QuestGiver* QuestGiver)
{
    if (QuestGiver)
    {
        QuestGivers.AddUnique(QuestGiver);
        UE_LOG(LogTemp, Warning, TEXT("Registered quest giver"));
    }
}

void UQuest_QuestManager::CreateSurvivalQuest(const FString& QuestID, const FText& Title, const FText& Description)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = Title;
    NewQuest.Description = Description;
    NewQuest.QuestType = EQuest_QuestType::Survival;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    RegisterQuest(NewQuest);
}

void UQuest_QuestManager::CreateHuntQuest(const FString& QuestID, const FString& TargetDinosaur, int32 RequiredKills)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = FText::FromString(FString::Printf(TEXT("Hunt %s"), *TargetDinosaur));
    NewQuest.Description = FText::FromString(FString::Printf(TEXT("Hunt and kill %d %s for survival"), RequiredKills, *TargetDinosaur));
    NewQuest.QuestType = EQuest_QuestType::Survival;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    // Add hunt objective
    FQuest_ObjectiveData HuntObjective;
    HuntObjective.ObjectiveID = QuestID + "_hunt";
    HuntObjective.Description = FText::FromString(FString::Printf(TEXT("Kill %d %s"), RequiredKills, *TargetDinosaur));
    HuntObjective.Type = EQuest_ObjectiveType::KillTarget;
    HuntObjective.TargetActorID = TargetDinosaur;
    HuntObjective.RequiredQuantity = RequiredKills;
    HuntObjective.CurrentProgress = 0;
    
    NewQuest.Objectives.Add(HuntObjective);
    RegisterQuest(NewQuest);
}

void UQuest_QuestManager::CreateExplorationQuest(const FString& QuestID, const FVector& TargetLocation, float Radius)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.Title = FText::FromString(TEXT("Explore Unknown Territory"));
    NewQuest.Description = FText::FromString(TEXT("Venture into unexplored areas to find resources and map the territory"));
    NewQuest.QuestType = EQuest_QuestType::Exploration;
    NewQuest.Status = EQuest_QuestStatus::Available;
    
    // Add exploration objective
    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveID = QuestID + "_explore";
    ExploreObjective.Description = FText::FromString(TEXT("Reach the target location"));
    ExploreObjective.Type = EQuest_ObjectiveType::GoToLocation;
    ExploreObjective.TargetLocation = TargetLocation;
    ExploreObjective.RequiredQuantity = 1;
    ExploreObjective.CurrentProgress = 0;
    
    NewQuest.Objectives.Add(ExploreObjective);
    RegisterQuest(NewQuest);
}

void UQuest_QuestManager::InitializeDefaultQuests()
{
    CreateDefaultSurvivalQuests();
}

void UQuest_QuestManager::CreateDefaultSurvivalQuests()
{
    // Basic survival quest
    CreateSurvivalQuest(
        TEXT("survive_first_day"),
        FText::FromString(TEXT("Survive the First Day")),
        FText::FromString(TEXT("Find shelter and water to survive your first day in this dangerous prehistoric world"))
    );
    
    // Hunt quest for small prey
    CreateHuntQuest(TEXT("hunt_compies"), TEXT("Compsognathus"), 3);
    
    // Exploration quest
    CreateExplorationQuest(TEXT("explore_river"), FVector(2000, 0, 100), 500.0f);
    
    // Resource gathering quest
    FQuest_QuestData GatherQuest;
    GatherQuest.QuestID = TEXT("gather_resources");
    GatherQuest.Title = FText::FromString(TEXT("Gather Basic Resources"));
    GatherQuest.Description = FText::FromString(TEXT("Collect stones and wood to craft basic tools for survival"));
    GatherQuest.QuestType = EQuest_QuestType::Survival;
    GatherQuest.Status = EQuest_QuestStatus::Available;
    
    // Add gathering objectives
    FQuest_ObjectiveData StoneObjective;
    StoneObjective.ObjectiveID = TEXT("gather_stones");
    StoneObjective.Description = FText::FromString(TEXT("Collect 5 stones"));
    StoneObjective.Type = EQuest_ObjectiveType::CollectItem;
    StoneObjective.TargetActorID = TEXT("Stone");
    StoneObjective.RequiredQuantity = 5;
    
    FQuest_ObjectiveData WoodObjective;
    WoodObjective.ObjectiveID = TEXT("gather_wood");
    WoodObjective.Description = FText::FromString(TEXT("Collect 3 pieces of wood"));
    WoodObjective.Type = EQuest_ObjectiveType::CollectItem;
    WoodObjective.TargetActorID = TEXT("Wood");
    WoodObjective.RequiredQuantity = 3;
    
    GatherQuest.Objectives.Add(StoneObjective);
    GatherQuest.Objectives.Add(WoodObjective);
    
    RegisterQuest(GatherQuest);
}

bool UQuest_QuestManager::ValidateQuestCompletion(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    // Check if all required objectives are completed
    for (const FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }
    
    return true;
}
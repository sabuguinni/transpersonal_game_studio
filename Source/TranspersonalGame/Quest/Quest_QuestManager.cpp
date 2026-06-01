#include "Quest_QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_QuestManager::UQuest_QuestManager()
{
    MaxActiveQuests = 5;
    bQuestSystemEnabled = true;
}

void UQuest_QuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
}

void UQuest_QuestManager::Deinitialize()
{
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    
    Super::Deinitialize();
}

void UQuest_QuestManager::StartQuest(const FString& QuestID)
{
    if (!bQuestSystemEnabled)
    {
        return;
    }

    if (!CanStartQuest(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest: %s"), *QuestID);
        return;
    }

    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        QuestData->QuestStatus = EQuestStatus::Active;
        ActiveQuestIDs.AddUnique(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("Started quest: %s - %s"), *QuestID, *QuestData->QuestTitle);
        NotifyQuestUpdate(QuestID);
    }
}

void UQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        QuestData->QuestStatus = EQuestStatus::Completed;
        ActiveQuestIDs.Remove(QuestID);
        CompletedQuestIDs.AddUnique(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s - %s (XP: %.0f)"), 
               *QuestID, *QuestData->QuestTitle, QuestData->RewardExperience);
        NotifyQuestUpdate(QuestID);
    }
}

void UQuest_QuestManager::FailQuest(const FString& QuestID)
{
    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        QuestData->QuestStatus = EQuestStatus::Failed;
        ActiveQuestIDs.Remove(QuestID);
        
        UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s - %s"), *QuestID, *QuestData->QuestTitle);
        NotifyQuestUpdate(QuestID);
    }
}

bool UQuest_QuestManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool UQuest_QuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

FQuest_QuestData UQuest_QuestManager::GetQuestData(const FString& QuestID) const
{
    if (const FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        return *QuestData;
    }
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (const FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            ActiveQuests.Add(*QuestData);
        }
    }
    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuest_QuestManager::GetCompletedQuests() const
{
    TArray<FQuest_QuestData> CompletedQuests;
    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (const FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            CompletedQuests.Add(*QuestData);
        }
    }
    return CompletedQuests;
}

void UQuest_QuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        if (QuestData->QuestStatus == EQuestStatus::Active)
        {
            QuestData->CurrentCount = FMath::Min(QuestData->CurrentCount + Progress, QuestData->TargetCount);
            CheckQuestCompletion(QuestID);
        }
    }
}

void UQuest_QuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        // Mark objective as completed in the arrays
        for (int32 i = 0; i < QuestData->ObjectiveTexts.Num(); i++)
        {
            if (QuestData->ObjectiveTexts[i].Contains(ObjectiveID))
            {
                if (i < QuestData->ObjectiveCompleted.Num())
                {
                    QuestData->ObjectiveCompleted[i] = true;
                }
                break;
            }
        }
        CheckQuestCompletion(QuestID);
    }
}

bool UQuest_QuestManager::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const
{
    if (const FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        for (int32 i = 0; i < QuestData->ObjectiveTexts.Num(); i++)
        {
            if (QuestData->ObjectiveTexts[i].Contains(ObjectiveID))
            {
                if (i < QuestData->ObjectiveCompleted.Num())
                {
                    return QuestData->ObjectiveCompleted[i];
                }
                break;
            }
        }
    }
    return false;
}

void UQuest_QuestManager::CreateHuntQuest(const FString& QuestID, const FString& Title, const FString& TargetSpecies, int32 Count)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestTitle = Title;
    NewQuest.QuestDescription = FString::Printf(TEXT("Hunt %d %s in the wild"), Count, *TargetSpecies);
    NewQuest.QuestType = EQuestType::Hunt;
    NewQuest.QuestStatus = EQuestStatus::NotStarted;
    NewQuest.TargetCount = Count;
    NewQuest.CurrentCount = 0;
    NewQuest.RewardExperience = Count * 50.0f;
    
    NewQuest.ObjectiveTexts.Add(FString::Printf(TEXT("Kill %d %s"), Count, *TargetSpecies));
    NewQuest.ObjectiveCompleted.Add(false);
    
    AllQuests.Add(QuestID, NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created hunt quest: %s"), *Title);
}

void UQuest_QuestManager::CreateExploreQuest(const FString& QuestID, const FString& Title, const FVector& Location, float Radius)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestTitle = Title;
    NewQuest.QuestDescription = FString::Printf(TEXT("Explore the area around coordinates (%.0f, %.0f)"), Location.X, Location.Y);
    NewQuest.QuestType = EQuestType::Explore;
    NewQuest.QuestStatus = EQuestStatus::NotStarted;
    NewQuest.TargetLocation = Location;
    NewQuest.TargetCount = 1;
    NewQuest.CurrentCount = 0;
    NewQuest.RewardExperience = 100.0f;
    
    NewQuest.ObjectiveTexts.Add(FString::Printf(TEXT("Reach location (%.0f, %.0f)"), Location.X, Location.Y));
    NewQuest.ObjectiveCompleted.Add(false);
    
    AllQuests.Add(QuestID, NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created explore quest: %s"), *Title);
}

void UQuest_QuestManager::CreateGatherQuest(const FString& QuestID, const FString& Title, const FString& ResourceType, int32 Count)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestTitle = Title;
    NewQuest.QuestDescription = FString::Printf(TEXT("Gather %d %s from the environment"), Count, *ResourceType);
    NewQuest.QuestType = EQuestType::Gather;
    NewQuest.QuestStatus = EQuestStatus::NotStarted;
    NewQuest.TargetCount = Count;
    NewQuest.CurrentCount = 0;
    NewQuest.RewardExperience = Count * 25.0f;
    
    NewQuest.ObjectiveTexts.Add(FString::Printf(TEXT("Collect %d %s"), Count, *ResourceType));
    NewQuest.ObjectiveCompleted.Add(false);
    
    AllQuests.Add(QuestID, NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created gather quest: %s"), *Title);
}

void UQuest_QuestManager::CreateSurvivalQuest(const FString& QuestID, const FString& Title, float Duration)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestTitle = Title;
    NewQuest.QuestDescription = FString::Printf(TEXT("Survive for %.0f minutes in the wilderness"), Duration / 60.0f);
    NewQuest.QuestType = EQuestType::Survival;
    NewQuest.QuestStatus = EQuestStatus::NotStarted;
    NewQuest.TargetCount = FMath::RoundToInt(Duration);
    NewQuest.CurrentCount = 0;
    NewQuest.RewardExperience = Duration * 2.0f;
    
    NewQuest.ObjectiveTexts.Add(FString::Printf(TEXT("Survive for %.1f minutes"), Duration / 60.0f));
    NewQuest.ObjectiveCompleted.Add(false);
    
    AllQuests.Add(QuestID, NewQuest);
    UE_LOG(LogTemp, Warning, TEXT("Created survival quest: %s"), *Title);
}

void UQuest_QuestManager::OnDinosaurKilled(const FString& Species)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            if (QuestData->QuestType == EQuestType::Hunt && 
                QuestData->QuestDescription.Contains(Species))
            {
                UpdateObjectiveProgress(QuestID, Species, 1);
                UE_LOG(LogTemp, Warning, TEXT("Hunt progress: %s killed (%d/%d)"), 
                       *Species, QuestData->CurrentCount, QuestData->TargetCount);
            }
        }
    }
}

void UQuest_QuestManager::OnLocationReached(const FVector& Location)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            if (QuestData->QuestType == EQuestType::Explore)
            {
                float Distance = FVector::Dist(Location, QuestData->TargetLocation);
                if (Distance <= 1000.0f) // Within 10 meters
                {
                    CompleteObjective(QuestID, "Explore");
                    UE_LOG(LogTemp, Warning, TEXT("Exploration objective completed at distance: %.2f"), Distance);
                }
            }
        }
    }
}

void UQuest_QuestManager::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            if (QuestData->QuestType == EQuestType::Gather && 
                QuestData->QuestDescription.Contains(ResourceType))
            {
                UpdateObjectiveProgress(QuestID, ResourceType, Amount);
                UE_LOG(LogTemp, Warning, TEXT("Gather progress: %s collected (%d/%d)"), 
                       *ResourceType, QuestData->CurrentCount, QuestData->TargetCount);
            }
        }
    }
}

void UQuest_QuestManager::OnPlayerSurvived(float TimeAlive)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
        {
            if (QuestData->QuestType == EQuestType::Survival)
            {
                QuestData->CurrentCount = FMath::RoundToInt(TimeAlive);
                CheckQuestCompletion(QuestID);
            }
        }
    }
}

void UQuest_QuestManager::InitializeDefaultQuests()
{
    // Create default starter quests
    CreateHuntQuest("HUNT_RAPTOR_01", "First Hunt", "Velociraptor", 1);
    CreateExploreQuest("EXPLORE_SAVANA_01", "Explore the Savana", FVector(5000, 5000, 100), 1000.0f);
    CreateGatherQuest("GATHER_WOOD_01", "Collect Wood", "Wood", 5);
    CreateSurvivalQuest("SURVIVE_NIGHT_01", "Survive the Night", 300.0f); // 5 minutes
    
    CreateHuntQuest("HUNT_TREX_01", "Apex Predator", "T-Rex", 1);
    CreateExploreQuest("EXPLORE_FOREST_01", "Forest Discovery", FVector(-45000, 40000, 100), 1000.0f);
    CreateGatherQuest("GATHER_STONE_01", "Stone Collection", "Stone", 10);
    CreateSurvivalQuest("SURVIVE_DAY_01", "Day Survivor", 600.0f); // 10 minutes
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default quests"), AllQuests.Num());
}

void UQuest_QuestManager::CheckQuestCompletion(const FString& QuestID)
{
    if (FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        bool bShouldComplete = false;
        
        switch (QuestData->QuestType)
        {
            case EQuestType::Hunt:
            case EQuestType::Gather:
                bShouldComplete = (QuestData->CurrentCount >= QuestData->TargetCount);
                break;
                
            case EQuestType::Explore:
                bShouldComplete = QuestData->ObjectiveCompleted.Num() > 0 && QuestData->ObjectiveCompleted[0];
                break;
                
            case EQuestType::Survival:
                bShouldComplete = (QuestData->CurrentCount >= QuestData->TargetCount);
                break;
        }
        
        if (bShouldComplete && QuestData->QuestStatus == EQuestStatus::Active)
        {
            CompleteQuest(QuestID);
        }
    }
}

bool UQuest_QuestManager::CanStartQuest(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }
    
    if (ActiveQuestIDs.Num() >= MaxActiveQuests)
    {
        return false;
    }
    
    if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID))
    {
        return false;
    }
    
    return true;
}

void UQuest_QuestManager::NotifyQuestUpdate(const FString& QuestID)
{
    // This could trigger UI updates, sound effects, etc.
    if (const FQuest_QuestData* QuestData = AllQuests.Find(QuestID))
    {
        UE_LOG(LogTemp, Log, TEXT("Quest Update: %s - Status: %d"), 
               *QuestData->QuestTitle, (int32)QuestData->QuestStatus);
    }
}
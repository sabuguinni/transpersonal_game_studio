#include "Quest_QuestSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TranspersonalGame/TranspersonalGame.h"

UQuest_QuestSystem::UQuest_QuestSystem()
{
    // Initialize quest system
}

void UQuest_QuestSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest System initialized"));
    
    // Initialize default quests
    InitializeDefaultQuests();
}

void UQuest_QuestSystem::Deinitialize()
{
    // Clean up quest system
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest System deinitialized"));
    
    Super::Deinitialize();
}

bool UQuest_QuestSystem::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Quest not found: %s"), *QuestID);
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuestStatus::NotStarted)
    {
        UE_LOG(LogTranspersonalGame, Warning, TEXT("Quest already started or completed: %s"), *QuestID);
        return false;
    }

    QuestData.Status = EQuestStatus::InProgress;
    ActiveQuestIDs.AddUnique(QuestID);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest started: %s - %s"), *QuestID, *QuestData.QuestTitle);
    
    NotifyQuestUpdate(QuestID);
    return true;
}

bool UQuest_QuestSystem::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuestStatus::InProgress)
    {
        return false;
    }

    QuestData.Status = EQuestStatus::Completed;
    QuestData.CurrentProgress = QuestData.RequiredAmount;
    
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);
    
    ProcessQuestCompletion(QuestID);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest completed: %s"), *QuestData.QuestTitle);
    return true;
}

bool UQuest_QuestSystem::UpdateQuestProgress(const FString& QuestID, int32 ProgressAmount)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    if (QuestData.Status != EQuestStatus::InProgress)
    {
        return false;
    }

    QuestData.CurrentProgress = FMath::Min(QuestData.CurrentProgress + ProgressAmount, QuestData.RequiredAmount);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest progress updated: %s (%d/%d)"), 
           *QuestData.QuestTitle, QuestData.CurrentProgress, QuestData.RequiredAmount);
    
    // Check if quest is now complete
    if (IsQuestComplete(QuestData))
    {
        CompleteQuest(QuestID);
    }
    else
    {
        NotifyQuestUpdate(QuestID);
    }
    
    return true;
}

FQuest_QuestData UQuest_QuestSystem::GetQuestData(const FString& QuestID)
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests[QuestID];
    }
    
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuest_QuestSystem::GetActiveQuests()
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

TArray<FQuest_QuestData> UQuest_QuestSystem::GetCompletedQuests()
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

void UQuest_QuestSystem::CreateHuntQuest(const FString& QuestID, const FString& TargetDinosaur, const FVector& Location)
{
    FString Title = FString::Printf(TEXT("Hunt the %s"), *TargetDinosaur);
    FString Description = FString::Printf(TEXT("Track and eliminate the dangerous %s terrorizing the hunting grounds."), *TargetDinosaur);
    
    CreateQuestTemplate(QuestID, Title, Description, EQuestType::Hunt, 1, Location, 500.0f);
}

void UQuest_QuestSystem::CreateGatherQuest(const FString& QuestID, const FString& ResourceType, int32 Amount)
{
    FString Title = FString::Printf(TEXT("Gather %s"), *ResourceType);
    FString Description = FString::Printf(TEXT("Collect %d %s for the tribe's survival."), Amount, *ResourceType);
    
    CreateQuestTemplate(QuestID, Title, Description, EQuestType::Gather, Amount, FVector::ZeroVector, 200.0f);
}

void UQuest_QuestSystem::CreateExploreQuest(const FString& QuestID, const FVector& TargetLocation, float Radius)
{
    FString Title = TEXT("Explore Unknown Territory");
    FString Description = TEXT("Venture into uncharted lands and discover new areas for the tribe.");
    
    CreateQuestTemplate(QuestID, Title, Description, EQuestType::Explore, 1, TargetLocation, 300.0f);
}

void UQuest_QuestSystem::CreateCraftQuest(const FString& QuestID, const FString& ItemToCraft, int32 Amount)
{
    FString Title = FString::Printf(TEXT("Craft %s"), *ItemToCraft);
    FString Description = FString::Printf(TEXT("Create %d %s using available resources."), Amount, *ItemToCraft);
    
    CreateQuestTemplate(QuestID, Title, Description, EQuestType::Craft, Amount, FVector::ZeroVector, 250.0f);
}

void UQuest_QuestSystem::CreateSurviveQuest(const FString& QuestID, float SurvivalTime)
{
    FString Title = TEXT("Survive the Night");
    FString Description = FString::Printf(TEXT("Survive for %.0f seconds against the dangers of the prehistoric world."), SurvivalTime);
    
    CreateQuestTemplate(QuestID, Title, Description, EQuestType::Survive, 1, FVector::ZeroVector, 400.0f);
}

void UQuest_QuestSystem::OnDinosaurKilled(const FString& DinosaurType, const FVector& Location)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& QuestData = AllQuests[QuestID];
            
            if (QuestData.QuestType == EQuestType::Hunt)
            {
                // Check if this dinosaur type matches the quest target
                if (QuestData.QuestDescription.Contains(DinosaurType))
                {
                    UpdateQuestProgress(QuestID, 1);
                }
            }
        }
    }
}

void UQuest_QuestSystem::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& QuestData = AllQuests[QuestID];
            
            if (QuestData.QuestType == EQuestType::Gather)
            {
                // Check if this resource type matches the quest target
                if (QuestData.QuestDescription.Contains(ResourceType))
                {
                    UpdateQuestProgress(QuestID, Amount);
                }
            }
        }
    }
}

void UQuest_QuestSystem::OnLocationReached(const FVector& Location)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& QuestData = AllQuests[QuestID];
            
            if (QuestData.QuestType == EQuestType::Explore)
            {
                float Distance = FVector::Dist(Location, QuestData.TargetLocation);
                if (Distance <= 500.0f) // Within 5 meter radius
                {
                    UpdateQuestProgress(QuestID, 1);
                }
            }
        }
    }
}

void UQuest_QuestSystem::OnItemCrafted(const FString& ItemType, int32 Amount)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& QuestData = AllQuests[QuestID];
            
            if (QuestData.QuestType == EQuestType::Craft)
            {
                // Check if this item type matches the quest target
                if (QuestData.QuestDescription.Contains(ItemType))
                {
                    UpdateQuestProgress(QuestID, Amount);
                }
            }
        }
    }
}

void UQuest_QuestSystem::OnSurvivalTimeReached(float TimeInSeconds)
{
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            FQuest_QuestData& QuestData = AllQuests[QuestID];
            
            if (QuestData.QuestType == EQuestType::Survive)
            {
                UpdateQuestProgress(QuestID, 1);
            }
        }
    }
}

void UQuest_QuestSystem::InitializeDefaultQuests()
{
    // Create default hunt quests
    CreateHuntQuest(TEXT("HUNT_TREX_001"), TEXT("T-Rex"), FVector(2000, 1000, 200));
    CreateHuntQuest(TEXT("HUNT_RAPTOR_001"), TEXT("Velociraptor"), FVector(-1000, 1500, 150));
    
    // Create default gather quests
    CreateGatherQuest(TEXT("GATHER_BERRIES_001"), TEXT("Crimson Berries"), 20);
    CreateGatherQuest(TEXT("GATHER_WOOD_001"), TEXT("Hardwood"), 15);
    
    // Create default explore quests
    CreateExploreQuest(TEXT("EXPLORE_CAVE_001"), FVector(1000, -1500, 180), 1000.0f);
    CreateExploreQuest(TEXT("EXPLORE_VALLEY_001"), FVector(-2000, -1000, 220), 1500.0f);
    
    // Create default craft quests
    CreateCraftQuest(TEXT("CRAFT_SPEAR_001"), TEXT("Stone Spear"), 1);
    CreateCraftQuest(TEXT("CRAFT_SHELTER_001"), TEXT("Basic Shelter"), 1);
    
    // Create default survival quests
    CreateSurviveQuest(TEXT("SURVIVE_NIGHT_001"), 300.0f); // 5 minutes
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Default quests initialized: %d total quests"), AllQuests.Num());
}

void UQuest_QuestSystem::CreateQuestTemplate(const FString& QuestID, const FString& Title, const FString& Description, 
                                           EQuestType Type, int32 RequiredAmount, const FVector& Location, float Reward)
{
    FQuest_QuestData NewQuest;
    NewQuest.QuestID = QuestID;
    NewQuest.QuestTitle = Title;
    NewQuest.QuestDescription = Description;
    NewQuest.QuestType = Type;
    NewQuest.Status = EQuestStatus::NotStarted;
    NewQuest.RequiredAmount = RequiredAmount;
    NewQuest.CurrentProgress = 0;
    NewQuest.TargetLocation = Location;
    NewQuest.RewardExperience = Reward;
    
    AllQuests.Add(QuestID, NewQuest);
}

void UQuest_QuestSystem::ProcessQuestCompletion(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    const FQuest_QuestData& QuestData = AllQuests[QuestID];
    
    // Award experience points
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest completed! Awarded %.0f experience points"), QuestData.RewardExperience);
    
    // TODO: Integrate with player progression system when available
    // PlayerProgressionSystem->AddExperience(QuestData.RewardExperience);
    
    NotifyQuestUpdate(QuestID);
}

bool UQuest_QuestSystem::IsQuestComplete(const FQuest_QuestData& QuestData)
{
    return QuestData.CurrentProgress >= QuestData.RequiredAmount;
}

void UQuest_QuestSystem::NotifyQuestUpdate(const FString& QuestID)
{
    // TODO: Implement UI notification system when available
    // QuestUIManager->UpdateQuestDisplay(QuestID);
    
    UE_LOG(LogTranspersonalGame, Log, TEXT("Quest update notification: %s"), *QuestID);
}
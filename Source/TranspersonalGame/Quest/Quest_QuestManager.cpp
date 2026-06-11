#include "Quest_QuestManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AQuest_QuestManager::AQuest_QuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    QuestCheckInterval = 1.0f;
    TimeSinceLastCheck = 0.0f;
}

void AQuest_QuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeQuestTemplates();
    UE_LOG(LogTemp, Warning, TEXT("Quest Manager initialized with %d available quests"), AvailableQuests.Num());
}

void AQuest_QuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= QuestCheckInterval)
    {
        CheckQuestCompletion();
        ProcessQuestEvents();
        TimeSinceLastCheck = 0.0f;
    }
}

void AQuest_QuestManager::StartQuest(const FString& QuestID)
{
    for (int32 i = 0; i < AvailableQuests.Num(); i++)
    {
        if (AvailableQuests[i].QuestID == QuestID)
        {
            FQuest_QuestData NewQuest = AvailableQuests[i];
            NewQuest.QuestStatus = EQuest_QuestStatus::Active;
            NewQuest.CurrentProgress = 0;
            ActiveQuests.Add(NewQuest);
            AvailableQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *NewQuest.QuestName);
            break;
        }
    }
}

void AQuest_QuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            FQuest_QuestData CompletedQuest = ActiveQuests[i];
            CompletedQuest.QuestStatus = EQuest_QuestStatus::Completed;
            CompletedQuests.Add(CompletedQuest);
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *CompletedQuest.QuestName);
            break;
        }
    }
}

void AQuest_QuestManager::FailQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); i++)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            FQuest_QuestData FailedQuest = ActiveQuests[i];
            FailedQuest.QuestStatus = EQuest_QuestStatus::Failed;
            ActiveQuests.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s"), *FailedQuest.QuestName);
            break;
        }
    }
}

void AQuest_QuestManager::UpdateQuestProgress(const FString& QuestID, int32 ProgressAmount)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (Quest)
    {
        Quest->CurrentProgress += ProgressAmount;
        Quest->CurrentProgress = FMath::Clamp(Quest->CurrentProgress, 0, Quest->RequiredAmount);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest %s progress: %d/%d"), 
               *Quest->QuestName, Quest->CurrentProgress, Quest->RequiredAmount);
        
        if (IsQuestCompleted(*Quest))
        {
            CompleteQuest(QuestID);
        }
    }
}

FQuest_QuestData AQuest_QuestManager::GetQuestData(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (Quest)
    {
        return *Quest;
    }
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> AQuest_QuestManager::GetActiveQuests()
{
    return ActiveQuests;
}

TArray<FQuest_QuestData> AQuest_QuestManager::GetCompletedQuests()
{
    return CompletedQuests;
}

void AQuest_QuestManager::OnDinosaurKilled(const FString& DinosaurType)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Hunt && Quest.TargetActorTag == DinosaurType)
        {
            UpdateQuestProgress(Quest.QuestID, 1);
        }
    }
}

void AQuest_QuestManager::OnResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Gather && Quest.TargetActorTag == ResourceType)
        {
            UpdateQuestProgress(Quest.QuestID, Amount);
        }
    }
}

void AQuest_QuestManager::OnLocationReached(const FVector& Location)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Exploration)
        {
            float Distance = FVector::Dist(Location, Quest.ObjectiveLocation);
            if (Distance <= 200.0f) // 2 meter radius
            {
                UpdateQuestProgress(Quest.QuestID, 1);
            }
        }
    }
}

void AQuest_QuestManager::OnNPCInteraction(const FString& NPCName)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestType == EQuest_QuestType::Interaction && Quest.TargetActorTag == NPCName)
        {
            UpdateQuestProgress(Quest.QuestID, 1);
        }
    }
}

void AQuest_QuestManager::InitializeQuestTemplates()
{
    CreateHuntQuests();
    CreateGatherQuests();
    CreateExplorationQuests();
    CreateSurvivalQuests();
}

void AQuest_QuestManager::CreateHuntQuests()
{
    // Hunt Raptor Quest
    FQuest_QuestData HuntRaptor;
    HuntRaptor.QuestID = TEXT("HUNT_RAPTOR_001");
    HuntRaptor.QuestName = TEXT("Pack Hunter");
    HuntRaptor.QuestDescription = TEXT("The raptors threaten our territory. Hunt down 3 raptors to secure the area.");
    HuntRaptor.QuestType = EQuest_QuestType::Hunt;
    HuntRaptor.QuestStatus = EQuest_QuestStatus::NotStarted;
    HuntRaptor.RequiredAmount = 3;
    HuntRaptor.TargetActorTag = TEXT("Raptor");
    AvailableQuests.Add(HuntRaptor);

    // Hunt T-Rex Quest
    FQuest_QuestData HuntTRex;
    HuntTRex.QuestID = TEXT("HUNT_TREX_001");
    HuntTRex.QuestName = TEXT("Apex Predator");
    HuntTRex.QuestDescription = TEXT("The great T-Rex rules these lands. Prove your worth by taking down this apex predator.");
    HuntTRex.QuestType = EQuest_QuestType::Hunt;
    HuntTRex.QuestStatus = EQuest_QuestStatus::NotStarted;
    HuntTRex.RequiredAmount = 1;
    HuntTRex.TargetActorTag = TEXT("TRex");
    AvailableQuests.Add(HuntTRex);
}

void AQuest_QuestManager::CreateGatherQuests()
{
    // Gather Stone Quest
    FQuest_QuestData GatherStone;
    GatherStone.QuestID = TEXT("GATHER_STONE_001");
    GatherStone.QuestName = TEXT("Stone Collector");
    GatherStone.QuestDescription = TEXT("Collect 10 stones to craft essential tools for survival.");
    GatherStone.QuestType = EQuest_QuestType::Gather;
    GatherStone.QuestStatus = EQuest_QuestStatus::NotStarted;
    GatherStone.RequiredAmount = 10;
    GatherStone.TargetActorTag = TEXT("Stone");
    AvailableQuests.Add(GatherStone);

    // Gather Wood Quest
    FQuest_QuestData GatherWood;
    GatherWood.QuestID = TEXT("GATHER_WOOD_001");
    GatherWood.QuestName = TEXT("Wood Gatherer");
    GatherWood.QuestDescription = TEXT("Collect 15 pieces of wood to build shelter and craft weapons.");
    GatherWood.QuestType = EQuest_QuestType::Gather;
    GatherWood.QuestStatus = EQuest_QuestStatus::NotStarted;
    GatherWood.RequiredAmount = 15;
    GatherWood.TargetActorTag = TEXT("Wood");
    AvailableQuests.Add(GatherWood);
}

void AQuest_QuestManager::CreateExplorationQuests()
{
    // Explore Cave Quest
    FQuest_QuestData ExploreCave;
    ExploreCave.QuestID = TEXT("EXPLORE_CAVE_001");
    ExploreCave.QuestName = TEXT("Cave Explorer");
    ExploreCave.QuestDescription = TEXT("Explore the mysterious cave to the north and discover its secrets.");
    ExploreCave.QuestType = EQuest_QuestType::Exploration;
    ExploreCave.QuestStatus = EQuest_QuestStatus::NotStarted;
    ExploreCave.RequiredAmount = 1;
    ExploreCave.ObjectiveLocation = FVector(0, 1000, 100);
    AvailableQuests.Add(ExploreCave);

    // Explore River Quest
    FQuest_QuestData ExploreRiver;
    ExploreRiver.QuestID = TEXT("EXPLORE_RIVER_001");
    ExploreRiver.QuestName = TEXT("Water Source");
    ExploreRiver.QuestDescription = TEXT("Find the river to the east to secure a reliable water source.");
    ExploreRiver.QuestType = EQuest_QuestType::Exploration;
    ExploreRiver.QuestStatus = EQuest_QuestStatus::NotStarted;
    ExploreRiver.RequiredAmount = 1;
    ExploreRiver.ObjectiveLocation = FVector(1500, 0, 50);
    AvailableQuests.Add(ExploreRiver);
}

void AQuest_QuestManager::CreateSurvivalQuests()
{
    // Survival Challenge Quest
    FQuest_QuestData SurvivalChallenge;
    SurvivalChallenge.QuestID = TEXT("SURVIVAL_001");
    SurvivalChallenge.QuestName = TEXT("Endurance Test");
    SurvivalChallenge.QuestDescription = TEXT("Survive for 5 minutes without your health dropping below 25%.");
    SurvivalChallenge.QuestType = EQuest_QuestType::Survival;
    SurvivalChallenge.QuestStatus = EQuest_QuestStatus::NotStarted;
    SurvivalChallenge.RequiredAmount = 300; // 5 minutes in seconds
    AvailableQuests.Add(SurvivalChallenge);
}

void AQuest_QuestManager::CheckQuestCompletion()
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (IsQuestCompleted(Quest))
        {
            CompleteQuest(Quest.QuestID);
            break; // Break to avoid modifying array during iteration
        }
    }
}

void AQuest_QuestManager::ProcessQuestEvents()
{
    // Process any ongoing quest events or conditions
    // This could include time-based quests, proximity checks, etc.
}

FQuest_QuestData* AQuest_QuestManager::FindQuestByID(const FString& QuestID)
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        if (Quest.QuestID == QuestID)
        {
            return &Quest;
        }
    }
    return nullptr;
}

bool AQuest_QuestManager::IsQuestCompleted(const FQuest_QuestData& Quest)
{
    return Quest.CurrentProgress >= Quest.RequiredAmount;
}
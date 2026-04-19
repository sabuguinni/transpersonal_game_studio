#include "QuestManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UQuestManager::UQuestManager()
{
    // Initialize arrays
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    RegisteredQuests.Empty();
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initializing quest system"));
    
    // Initialize default quests for survival gameplay
    InitializeDefaultQuests();
}

void UQuestManager::Deinitialize()
{
    // Clear all quest data
    RegisteredQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }

    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    // Check prerequisites
    if (!ValidateQuestPrerequisites(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Prerequisites not met for quest %s"), *QuestID);
        return false;
    }

    // Check if already active or completed
    if (QuestData.Status == EQuest_QuestStatus::Active || 
        QuestData.Status == EQuest_QuestStatus::Completed)
    {
        return false;
    }

    // Start the quest
    QuestData.Status = EQuest_QuestStatus::Active;
    ActiveQuestIDs.AddUnique(QuestID);
    
    // Reset all objectives
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        Objective.CurrentProgress = 0;
        Objective.bIsCompleted = false;
    }

    NotifyQuestStatusChange(QuestID, EQuest_QuestStatus::Active);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Started quest %s"), *QuestID);
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Mark as completed
    QuestData.Status = EQuest_QuestStatus::Completed;
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);

    NotifyQuestStatusChange(QuestID, EQuest_QuestStatus::Completed);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed quest %s"), *QuestID);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    if (QuestData.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Mark as failed
    QuestData.Status = EQuest_QuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);

    NotifyQuestStatusChange(QuestID, EQuest_QuestStatus::Failed);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Failed quest %s"), *QuestID);
    return true;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    return RegisteredQuests[QuestID].Status == EQuest_QuestStatus::Active;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    return RegisteredQuests[QuestID].Status == EQuest_QuestStatus::Completed;
}

EQuest_QuestStatus UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return EQuest_QuestStatus::NotStarted;
    }

    return RegisteredQuests[QuestID].Status;
}

bool UQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
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
            
            // Check if objective is completed
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                CompleteObjective(QuestID, ObjectiveID);
            }
            
            return true;
        }
    }

    return false;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    // Find and complete the objective
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && !Objective.bIsCompleted)
        {
            Objective.bIsCompleted = true;
            Objective.CurrentProgress = Objective.RequiredQuantity;
            
            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
            
            // Check if all objectives are completed
            CheckQuestCompletion(QuestID);
            
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed objective %s for quest %s"), *ObjectiveID, *QuestID);
            return true;
        }
    }

    return false;
}

int32 UQuestManager::GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return 0;
    }

    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    for (const FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.CurrentProgress;
        }
    }

    return 0;
}

FQuest_QuestData UQuestManager::GetQuestData(const FString& QuestID) const
{
    if (RegisteredQuests.Contains(QuestID))
    {
        return RegisteredQuests[QuestID];
    }

    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (RegisteredQuests.Contains(QuestID))
        {
            ActiveQuests.Add(RegisteredQuests[QuestID]);
        }
    }

    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_QuestData> AvailableQuests;
    
    for (const auto& QuestPair : RegisteredQuests)
    {
        const FQuest_QuestData& QuestData = QuestPair.Value;
        if (QuestData.Status == EQuest_QuestStatus::Available)
        {
            AvailableQuests.Add(QuestData);
        }
    }

    return AvailableQuests;
}

TArray<FQuest_QuestData> UQuestManager::GetCompletedQuests() const
{
    TArray<FQuest_QuestData> CompletedQuests;
    
    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (RegisteredQuests.Contains(QuestID))
        {
            CompletedQuests.Add(RegisteredQuests[QuestID]);
        }
    }

    return CompletedQuests;
}

void UQuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot register quest with empty ID"));
        return;
    }

    RegisteredQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Registered quest %s"), *QuestData.QuestID);
}

void UQuestManager::UnregisterQuest(const FString& QuestID)
{
    RegisteredQuests.Remove(QuestID);
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.Remove(QuestID);
}

void UQuestManager::CreateTestQuests()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Creating test quests"));
    
    // Create and register test quests
    RegisterQuest(CreateHuntingQuest());
    RegisterQuest(CreateGatheringQuest());
    RegisterQuest(CreateExplorationQuest());
    RegisterQuest(CreateSurvivalQuest());
}

void UQuestManager::LogAllQuests()
{
    UE_LOG(LogTemp, Warning, TEXT("=== QUEST MANAGER STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Quests: %d"), RegisteredQuests.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active Quests: %d"), ActiveQuestIDs.Num());
    UE_LOG(LogTemp, Warning, TEXT("Completed Quests: %d"), CompletedQuestIDs.Num());
    
    for (const auto& QuestPair : RegisteredQuests)
    {
        const FQuest_QuestData& Quest = QuestPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Quest: %s - Status: %d - Objectives: %d"), 
               *Quest.QuestID, (int32)Quest.Status, Quest.Objectives.Num());
    }
}

void UQuestManager::InitializeDefaultQuests()
{
    // Create default survival quests
    RegisterQuest(CreateHuntingQuest());
    RegisterQuest(CreateGatheringQuest());
    RegisterQuest(CreateExplorationQuest());
    RegisterQuest(CreateSurvivalQuest());
    
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initialized %d default quests"), RegisteredQuests.Num());
}

bool UQuestManager::ValidateQuestPrerequisites(const FString& QuestID) const
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    // Check if all prerequisite quests are completed
    for (const FString& PrereqID : QuestData.Prerequisites)
    {
        if (!IsQuestCompleted(PrereqID))
        {
            return false;
        }
    }

    return true;
}

void UQuestManager::NotifyQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus)
{
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}

void UQuestManager::CheckQuestCompletion(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        return;
    }

    const FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    // Check if all required objectives are completed
    bool bAllObjectivesCompleted = true;
    for (const FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            bAllObjectivesCompleted = false;
            break;
        }
    }

    if (bAllObjectivesCompleted)
    {
        CompleteQuest(QuestID);
    }
}

FQuest_QuestData UQuestManager::CreateHuntingQuest() const
{
    FQuest_QuestData HuntQuest;
    HuntQuest.QuestID = "HUNT_RAPTORS_001";
    HuntQuest.Title = FText::FromString("The Raptor Hunt");
    HuntQuest.Description = FText::FromString("Kael the Hunter needs your help tracking down dangerous raptors that threaten the tribe. Hunt them down and bring back their claws as proof.");
    HuntQuest.QuestType = EQuest_QuestType::SideQuest;
    HuntQuest.Status = EQuest_QuestStatus::Available;
    HuntQuest.ExperienceReward = 100;
    HuntQuest.bIsRepeatable = true;

    // Create hunting objectives
    FQuest_ObjectiveData HuntObjective;
    HuntObjective.ObjectiveID = "KILL_RAPTORS";
    HuntObjective.Description = FText::FromString("Hunt down 3 raptors in the eastern hunting grounds");
    HuntObjective.Type = EQuest_ObjectiveType::KillTarget;
    HuntObjective.TargetLocation = FVector(1000, 1000, 100);
    HuntObjective.RequiredQuantity = 3;
    HuntObjective.CurrentProgress = 0;
    HuntObjective.bIsCompleted = false;
    HuntObjective.bIsOptional = false;

    FQuest_ObjectiveData CollectClawsObjective;
    CollectClawsObjective.ObjectiveID = "COLLECT_CLAWS";
    CollectClawsObjective.Description = FText::FromString("Collect 3 raptor claws");
    CollectClawsObjective.Type = EQuest_ObjectiveType::CollectItem;
    CollectClawsObjective.RequiredQuantity = 3;
    CollectClawsObjective.CurrentProgress = 0;
    CollectClawsObjective.bIsCompleted = false;
    CollectClawsObjective.bIsOptional = false;

    HuntQuest.Objectives.Add(HuntObjective);
    HuntQuest.Objectives.Add(CollectClawsObjective);

    return HuntQuest;
}

FQuest_QuestData UQuestManager::CreateGatheringQuest() const
{
    FQuest_QuestData GatherQuest;
    GatherQuest.QuestID = "GATHER_CRYSTALS_001";
    GatherQuest.Title = FText::FromString("Crystal Harvest");
    GatherQuest.Description = FText::FromString("Nara the Gatherer seeks glowing crystals from the deep caves. These crystals have healing properties essential for tribal medicine.");
    GatherQuest.QuestType = EQuest_QuestType::SideQuest;
    GatherQuest.Status = EQuest_QuestStatus::Available;
    GatherQuest.ExperienceReward = 75;
    GatherQuest.bIsRepeatable = true;

    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveID = "EXPLORE_CAVE";
    ExploreObjective.Description = FText::FromString("Explore the crystal caves");
    ExploreObjective.Type = EQuest_ObjectiveType::GoToLocation;
    ExploreObjective.TargetLocation = FVector(-1000, 1000, 100);
    ExploreObjective.RequiredQuantity = 1;
    ExploreObjective.CurrentProgress = 0;
    ExploreObjective.bIsCompleted = false;
    ExploreObjective.bIsOptional = false;

    FQuest_ObjectiveData CollectCrystalsObjective;
    CollectCrystalsObjective.ObjectiveID = "COLLECT_CRYSTALS";
    CollectCrystalsObjective.Description = FText::FromString("Collect 5 glowing crystal shards");
    CollectCrystalsObjective.Type = EQuest_ObjectiveType::CollectItem;
    CollectCrystalsObjective.RequiredQuantity = 5;
    CollectCrystalsObjective.CurrentProgress = 0;
    CollectCrystalsObjective.bIsCompleted = false;
    CollectCrystalsObjective.bIsOptional = false;

    GatherQuest.Objectives.Add(ExploreObjective);
    GatherQuest.Objectives.Add(CollectCrystalsObjective);

    return GatherQuest;
}

FQuest_QuestData UQuestManager::CreateExplorationQuest() const
{
    FQuest_QuestData ExploreQuest;
    ExploreQuest.QuestID = "SCOUT_TERRITORY_001";
    ExploreQuest.Title = FText::FromString("Scouting Mission");
    ExploreQuest.Description = FText::FromString("The tribe needs to know what lies beyond our borders. Scout the distant lookout point and report back on any threats or resources.");
    ExploreQuest.QuestType = EQuest_QuestType::Exploration;
    ExploreQuest.Status = EQuest_QuestStatus::Available;
    ExploreQuest.ExperienceReward = 50;
    ExploreQuest.bIsRepeatable = false;

    FQuest_ObjectiveData ScoutObjective;
    ScoutObjective.ObjectiveID = "REACH_LOOKOUT";
    ScoutObjective.Description = FText::FromString("Reach the high lookout point to the south");
    ScoutObjective.Type = EQuest_ObjectiveType::GoToLocation;
    ScoutObjective.TargetLocation = FVector(0, -1000, 200);
    ScoutObjective.RequiredQuantity = 1;
    ScoutObjective.CurrentProgress = 0;
    ScoutObjective.bIsCompleted = false;
    ScoutObjective.bIsOptional = false;

    FQuest_ObjectiveData ObserveObjective;
    ObserveObjective.ObjectiveID = "OBSERVE_AREA";
    ObserveObjective.Description = FText::FromString("Observe the surrounding area for threats and resources");
    ObserveObjective.Type = EQuest_ObjectiveType::Observe;
    ObserveObjective.RequiredQuantity = 1;
    ObserveObjective.CurrentProgress = 0;
    ObserveObjective.bIsCompleted = false;
    ObserveObjective.bIsOptional = false;

    ExploreQuest.Objectives.Add(ScoutObjective);
    ExploreQuest.Objectives.Add(ObserveObjective);

    return ExploreQuest;
}

FQuest_QuestData UQuestManager::CreateSurvivalQuest() const
{
    FQuest_QuestData SurvivalQuest;
    SurvivalQuest.QuestID = "SURVIVE_NIGHT_001";
    SurvivalQuest.Title = FText::FromString("First Night");
    SurvivalQuest.Description = FText::FromString("The sun is setting and the predators emerge at night. Find shelter, build a fire, and survive until dawn.");
    SurvivalQuest.QuestType = EQuest_QuestType::Survival;
    SurvivalQuest.Status = EQuest_QuestStatus::Available;
    SurvivalQuest.ExperienceReward = 125;
    SurvivalQuest.bIsRepeatable = false;

    FQuest_ObjectiveData ShelterObjective;
    ShelterObjective.ObjectiveID = "BUILD_SHELTER";
    ShelterObjective.Description = FText::FromString("Build a basic shelter for protection");
    ShelterObjective.Type = EQuest_ObjectiveType::Build;
    ShelterObjective.RequiredQuantity = 1;
    ShelterObjective.CurrentProgress = 0;
    ShelterObjective.bIsCompleted = false;
    ShelterObjective.bIsOptional = false;

    FQuest_ObjectiveData FireObjective;
    FireObjective.ObjectiveID = "MAKE_FIRE";
    FireObjective.Description = FText::FromString("Create a campfire to keep predators away");
    FireObjective.Type = EQuest_ObjectiveType::Build;
    FireObjective.RequiredQuantity = 1;
    FireObjective.CurrentProgress = 0;
    FireObjective.bIsCompleted = false;
    FireObjective.bIsOptional = false;

    FQuest_ObjectiveData SurviveObjective;
    SurviveObjective.ObjectiveID = "SURVIVE_DAWN";
    SurviveObjective.Description = FText::FromString("Survive until dawn breaks");
    SurviveObjective.Type = EQuest_ObjectiveType::Survive;
    SurviveObjective.RequiredQuantity = 1;
    SurviveObjective.CurrentProgress = 0;
    SurviveObjective.bIsCompleted = false;
    SurviveObjective.bIsOptional = false;

    SurvivalQuest.Objectives.Add(ShelterObjective);
    SurvivalQuest.Objectives.Add(FireObjective);
    SurvivalQuest.Objectives.Add(SurviveObjective);

    return SurvivalQuest;
}
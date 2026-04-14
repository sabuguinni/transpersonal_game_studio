#include "QuestManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
    CurrentGlobalEmotionalState = EQuest_EmotionalState::Neutral;
    TotalSpiritualGrowth = 0.0f;
    bAutoUpdateAvailability = true;
    EmotionalStateUpdateThreshold = 0.25f;
    MaxActiveQuests = 5;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initializing Quest System"));
    
    // Initialize quest storage
    AllQuests.Empty();
    ActiveQuestIDs.Empty();
    CompletedQuestIDs.Empty();
    AvailableQuestIDs.Empty();
    
    // Create initial test quests
    CreateTestQuests();
    
    // Load saved quest progress
    LoadQuestProgress();
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest System Initialized"));
}

void UQuestManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Shutting down Quest System"));
    
    // Save quest progress before shutdown
    SaveQuestProgress();
    
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    // Check prerequisites
    if (!CheckQuestPrerequisites(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s prerequisites not met"), *QuestID);
        return false;
    }

    // Check if already active or completed
    if (Quest.Status == EQuest_QuestStatus::Active || Quest.Status == EQuest_QuestStatus::Completed)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already active or completed"), *QuestID);
        return false;
    }

    // Check active quest limit
    if (ActiveQuestIDs.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Maximum active quests reached (%d)"), MaxActiveQuests);
        return false;
    }

    // Start the quest
    Quest.Status = EQuest_QuestStatus::Active;
    Quest.EmotionalArc.CurrentEmotion = Quest.EmotionalArc.StartingEmotion;
    
    ActiveQuestIDs.AddUnique(QuestID);
    AvailableQuestIDs.Remove(QuestID);

    // Initialize objectives
    for (FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        Objective.CurrentProgress = 0;
        Objective.bIsCompleted = false;
    }

    BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Active);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest %s"), *QuestID);
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Mark quest as completed
    Quest.Status = EQuest_QuestStatus::Completed;
    Quest.EmotionalArc.CurrentEmotion = Quest.EmotionalArc.TargetEmotion;
    Quest.EmotionalArc.EmotionalProgress = 1.0f;

    // Update quest lists
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.AddUnique(QuestID);

    // Apply rewards
    ApplyQuestRewards(QuestID);

    // Update quest availability for dependent quests
    if (bAutoUpdateAvailability)
    {
        UpdateQuestAvailability();
    }

    BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Completed);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed quest %s"), *QuestID);
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    Quest.Status = EQuest_QuestStatus::Failed;
    ActiveQuestIDs.Remove(QuestID);

    BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Failed);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Failed quest %s"), *QuestID);
    return true;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->Status == EQuest_QuestStatus::Active;
    }
    return false;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->Status == EQuest_QuestStatus::Completed;
    }
    return false;
}

EQuest_QuestStatus UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->Status;
    }
    return EQuest_QuestStatus::NotStarted;
}

TArray<FQuest_QuestData> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            ActiveQuests.Add(*Quest);
        }
    }
    
    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_QuestData> AvailableQuests;
    
    for (const FString& QuestID : AvailableQuestIDs)
    {
        if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
        {
            AvailableQuests.Add(*Quest);
        }
    }
    
    return AvailableQuests;
}

FQuest_QuestData UQuestManager::GetQuestData(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return *Quest;
    }
    return FQuest_QuestData();
}

bool UQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (Quest.Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Find and update the objective
    for (FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Clamp(Progress, 0, Objective.RequiredQuantity);
            
            // Check if objective is completed
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %s completed for quest %s"), *ObjectiveID, *QuestID);
            }

            BroadcastObjectiveUpdate(QuestID, ObjectiveID, Objective.CurrentProgress);
            
            // Update emotional progress
            UpdateQuestEmotionalProgress(QuestID);
            
            // Check if quest is completed
            CheckQuestCompletion(QuestID);
            
            return true;
        }
    }

    return false;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    for (FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return UpdateObjectiveProgress(QuestID, ObjectiveID, Objective.RequiredQuantity);
        }
    }

    return false;
}

bool UQuestManager::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        for (const FQuest_ObjectiveData& Objective : Quest->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                return Objective.bIsCompleted;
            }
        }
    }
    return false;
}

int32 UQuestManager::GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        for (const FQuest_ObjectiveData& Objective : Quest->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                return Objective.CurrentProgress;
            }
        }
    }
    return 0;
}

void UQuestManager::UpdateEmotionalState(const FString& QuestID, EQuest_EmotionalState NewState)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    EQuest_EmotionalState OldState = Quest.EmotionalArc.CurrentEmotion;
    
    Quest.EmotionalArc.CurrentEmotion = NewState;
    
    // Update global emotional state
    CurrentGlobalEmotionalState = NewState;
    
    OnEmotionalStateChanged.Broadcast(OldState, NewState);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Emotional state changed from %d to %d for quest %s"), 
           (int32)OldState, (int32)NewState, *QuestID);
}

EQuest_EmotionalState UQuestManager::GetCurrentEmotionalState(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->EmotionalArc.CurrentEmotion;
    }
    return EQuest_EmotionalState::Neutral;
}

float UQuestManager::GetEmotionalProgress(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        return Quest->EmotionalArc.EmotionalProgress;
    }
    return 0.0f;
}

void UQuestManager::AddEmotionalMilestone(const FString& QuestID, const FText& Milestone)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    Quest.EmotionalArc.EmotionalMilestones.Add(Milestone);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Added emotional milestone for quest %s"), *QuestID);
}

void UQuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    AllQuests.Add(QuestData.QuestID, QuestData);
    
    // Check if quest should be available
    if (CheckQuestPrerequisites(QuestData.QuestID))
    {
        AvailableQuestIDs.AddUnique(QuestData.QuestID);
    }
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered quest %s"), *QuestData.QuestID);
}

bool UQuestManager::CheckQuestPrerequisites(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        for (const FString& PrereqID : Quest->Prerequisites)
        {
            if (!IsQuestCompleted(PrereqID))
            {
                return false;
            }
        }
        return true;
    }
    return false;
}

void UQuestManager::UpdateQuestAvailability()
{
    for (const auto& QuestPair : AllQuests)
    {
        const FString& QuestID = QuestPair.Key;
        const FQuest_QuestData& Quest = QuestPair.Value;
        
        if (Quest.Status == EQuest_QuestStatus::NotStarted || Quest.Status == EQuest_QuestStatus::Locked)
        {
            if (CheckQuestPrerequisites(QuestID))
            {
                AvailableQuestIDs.AddUnique(QuestID);
                UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest %s is now available"), *QuestID);
            }
        }
    }
}

float UQuestManager::CalculateSpiritualGrowth(const FString& QuestID) const
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        float BaseReward = Quest->SpiritualGrowthReward;
        float EmotionalMultiplier = Quest->EmotionalArc.EmotionalProgress;
        
        return BaseReward * (1.0f + EmotionalMultiplier);
    }
    return 0.0f;
}

void UQuestManager::ApplyQuestRewards(const FString& QuestID)
{
    if (const FQuest_QuestData* Quest = AllQuests.Find(QuestID))
    {
        // Apply spiritual growth
        float SpiritualReward = CalculateSpiritualGrowth(QuestID);
        TotalSpiritualGrowth += SpiritualReward;
        
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Applied rewards for quest %s - Spiritual Growth: %f"), 
               *QuestID, SpiritualReward);
    }
}

void UQuestManager::CreateTestQuests()
{
    // Create "Path of Inner Wisdom" quest
    FQuest_QuestData InnerWisdomQuest;
    InnerWisdomQuest.QuestID = "QUEST_INNER_WISDOM";
    InnerWisdomQuest.Title = FText::FromString("The Path of Inner Wisdom");
    InnerWisdomQuest.Description = FText::FromString("Embark on a spiritual journey to discover the ancient wisdom hidden within yourself and the world around you.");
    InnerWisdomQuest.QuestType = EQuest_QuestType::SpiritualJourney;
    InnerWisdomQuest.Status = EQuest_QuestStatus::Available;
    InnerWisdomQuest.ExperienceReward = 500;
    InnerWisdomQuest.SpiritualGrowthReward = 25.0f;
    
    // Set up emotional arc
    InnerWisdomQuest.EmotionalArc.StartingEmotion = EQuest_EmotionalState::Curious;
    InnerWisdomQuest.EmotionalArc.TargetEmotion = EQuest_EmotionalState::Enlightened;
    InnerWisdomQuest.EmotionalArc.CurrentEmotion = EQuest_EmotionalState::Curious;
    
    // Create objectives
    FQuest_ObjectiveData Obj1;
    Obj1.ObjectiveID = "OBJ_VISIT_GROVE";
    Obj1.Description = FText::FromString("Visit the Sacred Grove");
    Obj1.Type = EQuest_ObjectiveType::GoToLocation;
    Obj1.TargetLocation = FVector(1000, 500, 100);
    Obj1.RequiredQuantity = 1;
    
    FQuest_ObjectiveData Obj2;
    Obj2.ObjectiveID = "OBJ_MEDITATE_STONES";
    Obj2.Description = FText::FromString("Meditate at the Ancient Stones");
    Obj2.Type = EQuest_ObjectiveType::Meditate;
    Obj2.TargetLocation = FVector(-500, 1000, 150);
    Obj2.RequiredQuantity = 1;
    
    FQuest_ObjectiveData Obj3;
    Obj3.ObjectiveID = "OBJ_RITUAL_CIRCLE";
    Obj3.Description = FText::FromString("Perform ritual at the Meditation Circle");
    Obj3.Type = EQuest_ObjectiveType::Ritual;
    Obj3.TargetLocation = FVector(0, -800, 80);
    Obj3.RequiredQuantity = 1;
    
    InnerWisdomQuest.Objectives.Add(Obj1);
    InnerWisdomQuest.Objectives.Add(Obj2);
    InnerWisdomQuest.Objectives.Add(Obj3);
    
    RegisterQuest(InnerWisdomQuest);
    
    // Create "Nature's Call" exploration quest
    FQuest_QuestData NatureQuest;
    NatureQuest.QuestID = "QUEST_NATURE_CALL";
    NatureQuest.Title = FText::FromString("Nature's Call");
    NatureQuest.Description = FText::FromString("Connect with the natural world by observing and understanding the creatures that inhabit this ancient land.");
    NatureQuest.QuestType = EQuest_QuestType::Exploration;
    NatureQuest.Status = EQuest_QuestStatus::Available;
    NatureQuest.ExperienceReward = 300;
    NatureQuest.SpiritualGrowthReward = 15.0f;
    
    NatureQuest.EmotionalArc.StartingEmotion = EQuest_EmotionalState::Neutral;
    NatureQuest.EmotionalArc.TargetEmotion = EQuest_EmotionalState::Connected;
    
    FQuest_ObjectiveData NatureObj1;
    NatureObj1.ObjectiveID = "OBJ_OBSERVE_DINOSAURS";
    NatureObj1.Description = FText::FromString("Observe 3 different dinosaur species");
    NatureObj1.Type = EQuest_ObjectiveType::Observe;
    NatureObj1.RequiredQuantity = 3;
    
    NatureQuest.Objectives.Add(NatureObj1);
    RegisterQuest(NatureQuest);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Created test quests"));
}

void UQuestManager::LogQuestSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== QUEST SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Total Quests: %d"), AllQuests.Num());
    UE_LOG(LogTemp, Log, TEXT("Active Quests: %d"), ActiveQuestIDs.Num());
    UE_LOG(LogTemp, Log, TEXT("Completed Quests: %d"), CompletedQuestIDs.Num());
    UE_LOG(LogTemp, Log, TEXT("Available Quests: %d"), AvailableQuestIDs.Num());
    UE_LOG(LogTemp, Log, TEXT("Current Global Emotional State: %d"), (int32)CurrentGlobalEmotionalState);
    UE_LOG(LogTemp, Log, TEXT("Total Spiritual Growth: %f"), TotalSpiritualGrowth);
    UE_LOG(LogTemp, Log, TEXT("==========================="));
}

// Private Helper Functions
void UQuestManager::CheckQuestCompletion(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    const FQuest_QuestData& Quest = AllQuests[QuestID];
    
    if (AreAllObjectivesCompleted(Quest))
    {
        CompleteQuest(QuestID);
    }
}

void UQuestManager::UpdateQuestEmotionalProgress(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuest_QuestData& Quest = AllQuests[QuestID];
    
    // Calculate progress based on completed objectives
    int32 CompletedObjectives = 0;
    int32 TotalObjectives = 0;
    
    for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (!Objective.bIsOptional)
        {
            TotalObjectives++;
            if (Objective.bIsCompleted)
            {
                CompletedObjectives++;
            }
        }
    }
    
    if (TotalObjectives > 0)
    {
        Quest.EmotionalArc.EmotionalProgress = static_cast<float>(CompletedObjectives) / TotalObjectives;
    }
}

bool UQuestManager::AreAllObjectivesCompleted(const FQuest_QuestData& Quest) const
{
    for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }
    return true;
}

void UQuestManager::BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus)
{
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}

void UQuestManager::BroadcastObjectiveUpdate(const FString& QuestID, const FString& ObjectiveID, int32 NewProgress)
{
    OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, NewProgress);
}

void UQuestManager::SaveQuestProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Saving quest progress"));
}

void UQuestManager::LoadQuestProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Loading quest progress"));
}
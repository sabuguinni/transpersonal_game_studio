#include "QuestManager.h"
#include "QuestNPC.h"
#include "QuestObjective.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AQuestManager::AQuestManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    // Initialize quest system settings
    QuestUpdateInterval = 1.0f;
    MaxActiveQuests = 10;
    bEnableEmotionalJourneys = true;
    bEnableQuestChaining = true;

    // Initialize emotional state
    CurrentEmotionalState = EQuest_EmotionalState::Neutral;
    EmotionalGrowthPoints = 0.0f;

    // Initialize emotional mastery levels
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Neutral, 1.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Curious, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Anxious, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Peaceful, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Excited, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Contemplative, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Enlightened, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Connected, 0.0f);
    EmotionalMasteryLevels.Add(EQuest_EmotionalState::Transcendent, 0.0f);

    // Initialize timers
    QuestUpdateTimer = 0.0f;
    EmotionalJourneyTimer = 0.0f;
}

void AQuestManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Initializing quest system..."));

    // Initialize available quests with sample spiritual quests
    FQuest_QuestData SampleQuest;
    SampleQuest.QuestID = "spiritual_awakening_001";
    SampleQuest.Title = FText::FromString("The Path of Inner Discovery");
    SampleQuest.Description = FText::FromString("Begin your spiritual journey by connecting with the ancient wisdom of this land.");
    SampleQuest.QuestType = EQuest_QuestType::SpiritualJourney;
    SampleQuest.Status = EQuest_QuestStatus::Available;

    // Add sample objectives
    FQuest_ObjectiveData Objective1;
    Objective1.ObjectiveID = "meet_spiritual_guide";
    Objective1.Description = FText::FromString("Meet with the Spiritual Guide Aria");
    Objective1.Type = EQuest_ObjectiveType::TalkToNPC;
    Objective1.TargetActorID = "SpiritualGuide_Aria";
    SampleQuest.Objectives.Add(Objective1);

    FQuest_ObjectiveData Objective2;
    Objective2.ObjectiveID = "meditate_sacred_circle";
    Objective2.Description = FText::FromString("Meditate at the Sacred Circle");
    Objective2.Type = EQuest_ObjectiveType::Meditate;
    Objective2.TargetLocation = FVector(1000, 0, 0);
    SampleQuest.Objectives.Add(Objective2);

    // Set emotional journey
    SampleQuest.EmotionalArc.StartingEmotion = EQuest_EmotionalState::Neutral;
    SampleQuest.EmotionalArc.TargetEmotion = EQuest_EmotionalState::Peaceful;
    SampleQuest.EmotionalArc.CurrentEmotion = EQuest_EmotionalState::Neutral;

    // Set rewards
    SampleQuest.ExperienceReward = 150;
    SampleQuest.SpiritualGrowthReward = 2.5f;
    SampleQuest.ItemRewards.Add("Ancient Wisdom Scroll");

    AvailableQuests.Add(SampleQuest);

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest system initialized with %d available quests"), AvailableQuests.Num());
}

void AQuestManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    QuestUpdateTimer += DeltaTime;
    EmotionalJourneyTimer += DeltaTime;

    // Update quest system at regular intervals
    if (QuestUpdateTimer >= QuestUpdateInterval)
    {
        ProcessActiveQuests();
        CheckQuestPrerequisites();
        CheckQuestTimeouts();
        QuestUpdateTimer = 0.0f;
    }

    // Process emotional journeys
    if (bEnableEmotionalJourneys && EmotionalJourneyTimer >= 0.5f)
    {
        ProcessEmotionalJourneys();
        EmotionalJourneyTimer = 0.0f;
    }
}

bool AQuestManager::StartQuest(const FString& QuestID)
{
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest %s - maximum active quests reached"), *QuestID);
        return false;
    }

    // Find quest in available quests
    for (int32 i = 0; i < AvailableQuests.Num(); i++)
    {
        if (AvailableQuests[i].QuestID == QuestID)
        {
            if (!ArePrerequisitesMet(QuestID))
            {
                UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest %s - prerequisites not met"), *QuestID);
                return false;
            }

            // Move quest to active quests
            FQuest_QuestData QuestToStart = AvailableQuests[i];
            QuestToStart.Status = EQuest_QuestStatus::Active;
            ActiveQuests.Add(QuestToStart);
            AvailableQuests.RemoveAt(i);

            // Trigger emotional transition if applicable
            if (bEnableEmotionalJourneys)
            {
                TriggerEmotionalTransition(QuestToStart.EmotionalArc.StartingEmotion);
            }

            BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Active);
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Started quest %s"), *QuestID);
            return true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found in available quests"), *QuestID);
    return false;
}

bool AQuestManager::CompleteQuest(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (!Quest || Quest->Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    // Check if all non-optional objectives are completed
    bool AllObjectivesCompleted = true;
    for (const FQuest_ObjectiveData& Objective : Quest->Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            AllObjectivesCompleted = false;
            break;
        }
    }

    if (!AllObjectivesCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot complete quest %s - objectives not finished"), *QuestID);
        return false;
    }

    // Complete the quest
    Quest->Status = EQuest_QuestStatus::Completed;

    // Apply rewards
    AddEmotionalGrowth(Quest->SpiritualGrowthReward);
    
    // Trigger final emotional state
    if (bEnableEmotionalJourneys)
    {
        TriggerEmotionalTransition(Quest->EmotionalArc.TargetEmotion);
    }

    // Move to completed quests
    CompletedQuests.Add(*Quest);
    ActiveQuests.RemoveAll([QuestID](const FQuest_QuestData& Q) { return Q.QuestID == QuestID; });

    BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Completed);
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed quest %s"), *QuestID);

    // Check for quest chains
    if (bEnableQuestChaining)
    {
        UpdateQuestChains();
    }

    return true;
}

bool AQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressIncrement)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (!Quest || Quest->Status != EQuest_QuestStatus::Active)
    {
        return false;
    }

    for (FQuest_ObjectiveData& Objective : Quest->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + ProgressIncrement, Objective.RequiredQuantity);
            
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                Objective.bIsCompleted = true;
                OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
                UE_LOG(LogTemp, Warning, TEXT("QuestManager: Completed objective %s for quest %s"), *ObjectiveID, *QuestID);
            }

            return true;
        }
    }

    return false;
}

void AQuestManager::TriggerEmotionalTransition(EQuest_EmotionalState NewState, float IntensityMultiplier)
{
    if (CurrentEmotionalState == NewState)
    {
        return;
    }

    EQuest_EmotionalState OldState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;

    // Increase mastery level for this emotional state
    if (EmotionalMasteryLevels.Contains(NewState))
    {
        EmotionalMasteryLevels[NewState] += 0.1f * IntensityMultiplier;
    }

    BroadcastEmotionalStateChange(OldState, NewState);
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Emotional transition from %d to %d"), (int32)OldState, (int32)NewState);
}

void AQuestManager::ProcessActiveQuests()
{
    for (FQuest_QuestData& Quest : ActiveQuests)
    {
        // Update emotional journey progress
        if (bEnableEmotionalJourneys)
        {
            float CompletedObjectives = 0.0f;
            float TotalObjectives = Quest.Objectives.Num();

            for (const FQuest_ObjectiveData& Objective : Quest.Objectives)
            {
                if (Objective.bIsCompleted)
                {
                    CompletedObjectives += 1.0f;
                }
                else
                {
                    CompletedObjectives += (float)Objective.CurrentProgress / (float)Objective.RequiredQuantity;
                }
            }

            Quest.EmotionalArc.EmotionalProgress = TotalObjectives > 0 ? CompletedObjectives / TotalObjectives : 0.0f;
        }
    }
}

void AQuestManager::ProcessEmotionalJourneys()
{
    // Gradually decay emotional intensity back to neutral if no active stimuli
    // This creates a natural emotional rhythm
    // Implementation would depend on game design requirements
}

void AQuestManager::CheckQuestPrerequisites()
{
    // Check if any locked quests should become available
    for (FQuest_QuestData& Quest : AvailableQuests)
    {
        if (Quest.Status == EQuest_QuestStatus::Locked && ArePrerequisitesMet(Quest.QuestID))
        {
            Quest.Status = EQuest_QuestStatus::Available;
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s is now available"), *Quest.QuestID);
        }
    }
}

bool AQuestManager::ArePrerequisitesMet(const FString& QuestID)
{
    FQuest_QuestData* Quest = nullptr;
    
    // Find quest in available or active quests
    for (FQuest_QuestData& Q : AvailableQuests)
    {
        if (Q.QuestID == QuestID)
        {
            Quest = &Q;
            break;
        }
    }

    if (!Quest)
    {
        return false;
    }

    // Check all prerequisites
    for (const FString& PrerequisiteID : Quest->Prerequisites)
    {
        bool PrerequisiteCompleted = false;
        for (const FQuest_QuestData& CompletedQuest : CompletedQuests)
        {
            if (CompletedQuest.QuestID == PrerequisiteID)
            {
                PrerequisiteCompleted = true;
                break;
            }
        }

        if (!PrerequisiteCompleted)
        {
            return false;
        }
    }

    return true;
}

FQuest_QuestData* AQuestManager::FindQuestByID(const FString& QuestID)
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

void AQuestManager::BroadcastQuestStatusChange(const FString& QuestID, EQuest_QuestStatus NewStatus)
{
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}

void AQuestManager::BroadcastEmotionalStateChange(EQuest_EmotionalState OldState, EQuest_EmotionalState NewState)
{
    OnEmotionalStateChanged.Broadcast(OldState, NewState);
}

void AQuestManager::CheckQuestTimeouts()
{
    // Implementation for time-limited quests
}

void AQuestManager::UpdateQuestChains()
{
    // Implementation for quest chain unlocking
}

FQuest_QuestData AQuestManager::GetQuestData(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    return Quest ? *Quest : FQuest_QuestData();
}

TArray<FQuest_QuestData> AQuestManager::GetActiveQuests() const
{
    return ActiveQuests;
}

TArray<FQuest_QuestData> AQuestManager::GetAvailableQuests() const
{
    return AvailableQuests;
}

bool AQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    return UpdateObjectiveProgress(QuestID, ObjectiveID, 999); // Complete immediately
}

float AQuestManager::GetQuestProgress(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (!Quest)
    {
        return 0.0f;
    }

    float CompletedObjectives = 0.0f;
    for (const FQuest_ObjectiveData& Objective : Quest->Objectives)
    {
        if (Objective.bIsCompleted)
        {
            CompletedObjectives += 1.0f;
        }
    }

    return Quest->Objectives.Num() > 0 ? CompletedObjectives / Quest->Objectives.Num() : 0.0f;
}

TArray<FQuest_ObjectiveData> AQuestManager::GetQuestObjectives(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    return Quest ? Quest->Objectives : TArray<FQuest_ObjectiveData>();
}

EQuest_EmotionalState AQuestManager::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

float AQuestManager::GetEmotionalMastery(EQuest_EmotionalState EmotionalState) const
{
    return EmotionalMasteryLevels.Contains(EmotionalState) ? EmotionalMasteryLevels[EmotionalState] : 0.0f;
}

void AQuestManager::AddEmotionalGrowth(float GrowthPoints)
{
    EmotionalGrowthPoints += GrowthPoints;
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Added %f emotional growth points. Total: %f"), GrowthPoints, EmotionalGrowthPoints);
}

void AQuestManager::RegisterQuestNPC(AQuestNPC* QuestNPC)
{
    if (QuestNPC && !RegisteredQuestNPCs.Contains(QuestNPC))
    {
        RegisteredQuestNPCs.Add(QuestNPC);
    }
}

void AQuestManager::RegisterQuestObjective(AQuestObjective* QuestObjective)
{
    if (QuestObjective && !RegisteredQuestObjectives.Contains(QuestObjective))
    {
        RegisteredQuestObjectives.Add(QuestObjective);
    }
}

bool AQuestManager::FailQuest(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (!Quest)
    {
        return false;
    }

    Quest->Status = EQuest_QuestStatus::Failed;
    BroadcastQuestStatusChange(QuestID, EQuest_QuestStatus::Failed);
    return true;
}

bool AQuestManager::AbandonQuest(const FString& QuestID)
{
    FQuest_QuestData* Quest = FindQuestByID(QuestID);
    if (!Quest)
    {
        return false;
    }

    ActiveQuests.RemoveAll([QuestID](const FQuest_QuestData& Q) { return Q.QuestID == QuestID; });
    UE_LOG(LogTemp, Warning, TEXT("QuestManager: Abandoned quest %s"), *QuestID);
    return true;
}

bool AQuestManager::IsQuestActive(const FString& QuestID)
{
    return FindQuestByID(QuestID) != nullptr;
}
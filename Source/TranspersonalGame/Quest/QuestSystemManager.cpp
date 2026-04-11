#include "QuestSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxActiveQuests = 10;
    bAutoCheckPrerequisites = true;
    PrerequisiteCheckInterval = 5.0f;
    LastPrerequisiteCheck = 0.0f;
}

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultQuests();
    
    if (bAutoCheckPrerequisites)
    {
        CheckQuestPrerequisites();
    }
}

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoCheckPrerequisites)
    {
        LastPrerequisiteCheck += DeltaTime;
        if (LastPrerequisiteCheck >= PrerequisiteCheckInterval)
        {
            CheckQuestPrerequisites();
            LastPrerequisiteCheck = 0.0f;
        }
    }
}

bool AQuestSystemManager::StartQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s not found"), *QuestID);
        return false;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData)
    {
        return false;
    }

    if (QuestData->QuestState != EQuestState::Available)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s is not available for starting"), *QuestID);
        return false;
    }

    if (ActiveQuestIDs.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active quests reached"));
        return false;
    }

    if (!ArePrerequisitesMet(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Prerequisites not met for quest %s"), *QuestID);
        return false;
    }

    UpdateQuestState(QuestID, EQuestState::Active);
    ActiveQuestIDs.Add(QuestID);

    // Initialize emotional journey
    QuestData->EmotionalArc.CurrentStage = EEmotionalStage::Curiosity;
    QuestData->EmotionalArc.EmotionalIntensity = 0.3f;
    QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("The journey begins with wonder and curiosity"));

    OnQuestStateChanged.Broadcast(QuestID, EQuestState::Active);
    OnEmotionalStageChanged.Broadcast(QuestID, EEmotionalStage::Curiosity);

    UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestData->QuestTitle.ToString());
    return true;
}

bool AQuestSystemManager::CompleteQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData || QuestData->QuestState != EQuestState::Active)
    {
        return false;
    }

    // Check if all required objectives are completed
    if (!CheckAllObjectivesCompleted(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete quest %s - objectives not finished"), *QuestID);
        return false;
    }

    UpdateQuestState(QuestID, EQuestState::Completed);
    ActiveQuestIDs.Remove(QuestID);
    CompletedQuestIDs.Add(QuestID);

    // Complete emotional journey
    AdvanceEmotionalStage(QuestID, EEmotionalStage::Transformation, 1.0f);
    AddInsight(QuestID, TEXT("Quest completed with deep understanding"));

    OnQuestStateChanged.Broadcast(QuestID, EQuestState::Completed);

    UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestData->QuestTitle.ToString());
    return true;
}

bool AQuestSystemManager::FailQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData || QuestData->QuestState != EQuestState::Active)
    {
        return false;
    }

    UpdateQuestState(QuestID, EQuestState::Failed);
    ActiveQuestIDs.Remove(QuestID);

    OnQuestStateChanged.Broadcast(QuestID, EQuestState::Failed);

    UE_LOG(LogTemp, Log, TEXT("Failed quest: %s"), *QuestData->QuestTitle.ToString());
    return true;
}

bool AQuestSystemManager::AbandonQuest(const FString& QuestID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData || QuestData->QuestState != EQuestState::Active)
    {
        return false;
    }

    UpdateQuestState(QuestID, EQuestState::Abandoned);
    ActiveQuestIDs.Remove(QuestID);

    OnQuestStateChanged.Broadcast(QuestID, EQuestState::Abandoned);

    UE_LOG(LogTemp, Log, TEXT("Abandoned quest: %s"), *QuestData->QuestTitle.ToString());
    return true;
}

void AQuestSystemManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData || QuestData->QuestState != EQuestState::Active)
    {
        return;
    }

    for (FQuestObjective& Objective : QuestData->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Clamp(Progress, 0, Objective.RequiredProgress);
            
            if (Objective.CurrentProgress >= Objective.RequiredProgress && !Objective.bIsCompleted)
            {
                CompleteObjective(QuestID, ObjectiveID);
            }
            break;
        }
    }
}

bool AQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData || QuestData->QuestState != EQuestState::Active)
    {
        return false;
    }

    for (FQuestObjective& Objective : QuestData->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.bIsCompleted = true;
            Objective.CurrentProgress = Objective.RequiredProgress;

            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);

            // Advance emotional journey based on objective completion
            if (QuestData->EmotionalArc.CurrentStage == EEmotionalStage::Curiosity)
            {
                AdvanceEmotionalStage(QuestID, EEmotionalStage::Challenge, 0.5f);
            }
            else if (QuestData->EmotionalArc.CurrentStage == EEmotionalStage::Challenge)
            {
                AdvanceEmotionalStage(QuestID, EEmotionalStage::Growth, 0.7f);
            }

            UE_LOG(LogTemp, Log, TEXT("Completed objective %s for quest %s"), *ObjectiveID, *QuestID);
            return true;
        }
    }

    return false;
}

TArray<FQuestData> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuestData> ActiveQuests;
    
    for (const FString& QuestID : ActiveQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            ActiveQuests.Add(*AllQuests.Find(QuestID));
        }
    }
    
    return ActiveQuests;
}

TArray<FQuestData> AQuestSystemManager::GetCompletedQuests() const
{
    TArray<FQuestData> CompletedQuests;
    
    for (const FString& QuestID : CompletedQuestIDs)
    {
        if (AllQuests.Contains(QuestID))
        {
            CompletedQuests.Add(*AllQuests.Find(QuestID));
        }
    }
    
    return CompletedQuests;
}

FQuestData AQuestSystemManager::GetQuestData(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return *AllQuests.Find(QuestID);
    }
    
    return FQuestData();
}

bool AQuestSystemManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuestIDs.Contains(QuestID);
}

bool AQuestSystemManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

void AQuestSystemManager::AdvanceEmotionalStage(const FString& QuestID, EEmotionalStage NewStage, float Intensity)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData)
    {
        return;
    }

    QuestData->EmotionalArc.CurrentStage = NewStage;
    QuestData->EmotionalArc.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Set stage-specific descriptions
    switch (NewStage)
    {
        case EEmotionalStage::Curiosity:
            QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("Wonder and curiosity drive the journey forward"));
            break;
        case EEmotionalStage::Challenge:
            QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("Obstacles test resolve and determination"));
            break;
        case EEmotionalStage::Growth:
            QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("Understanding deepens through experience"));
            break;
        case EEmotionalStage::Resolution:
            QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("Clarity emerges from the journey"));
            break;
        case EEmotionalStage::Transformation:
            QuestData->EmotionalArc.StageDescription = FText::FromString(TEXT("Profound change has taken root"));
            break;
    }

    OnEmotionalStageChanged.Broadcast(QuestID, NewStage);
}

void AQuestSystemManager::AddInsight(const FString& QuestID, const FString& Insight)
{
    if (!AllQuests.Contains(QuestID))
    {
        return;
    }

    FQuestData* QuestData = AllQuests.Find(QuestID);
    if (QuestData)
    {
        QuestData->EmotionalArc.UnlockedInsights.AddUnique(Insight);
    }
}

FEmotionalJourney AQuestSystemManager::GetEmotionalJourney(const FString& QuestID) const
{
    if (AllQuests.Contains(QuestID))
    {
        return AllQuests.Find(QuestID)->EmotionalArc;
    }
    
    return FEmotionalJourney();
}

void AQuestSystemManager::RegisterQuest(const FQuestData& QuestData)
{
    AllQuests.Add(QuestData.QuestID, QuestData);
    UE_LOG(LogTemp, Log, TEXT("Registered quest: %s"), *QuestData.QuestTitle.ToString());
}

void AQuestSystemManager::CheckQuestPrerequisites()
{
    for (auto& QuestPair : AllQuests)
    {
        FQuestData& QuestData = QuestPair.Value;
        
        if (QuestData.QuestState == EQuestState::Inactive && ArePrerequisitesMet(QuestData.QuestID))
        {
            UpdateQuestState(QuestData.QuestID, EQuestState::Available);
            UE_LOG(LogTemp, Log, TEXT("Quest %s is now available"), *QuestData.QuestTitle.ToString());
        }
    }
}

bool AQuestSystemManager::ArePrerequisitesMet(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData)
    {
        return false;
    }

    for (const FString& PrerequisiteID : QuestData->Prerequisites)
    {
        if (!IsQuestCompleted(PrerequisiteID))
        {
            return false;
        }
    }

    return true;
}

void AQuestSystemManager::InitializeDefaultQuests()
{
    // Spiritual Journey: Sacred Grove Meditation
    FQuestData SacredGroveQuest;
    SacredGroveQuest.QuestID = TEXT("sacred_grove_meditation");
    SacredGroveQuest.QuestTitle = FText::FromString(TEXT("The Sacred Grove"));
    SacredGroveQuest.QuestDescription = FText::FromString(TEXT("Elder Kaya has asked you to visit the sacred grove and meditate among the ancient stones. This is the first step in understanding the spiritual connection between your people and the land."));
    SacredGroveQuest.QuestType = EQuestType::SpiritualJourney;
    SacredGroveQuest.QuestState = EQuestState::Available;
    SacredGroveQuest.ExperienceReward = 100;
    SacredGroveQuest.EstimatedDuration = 300.0f; // 5 minutes

    FQuestObjective GroveObjective;
    GroveObjective.ObjectiveID = TEXT("reach_sacred_grove");
    GroveObjective.Description = FText::FromString(TEXT("Travel to the Sacred Grove"));
    GroveObjective.RequiredProgress = 1;
    SacredGroveQuest.Objectives.Add(GroveObjective);

    FQuestObjective MeditationObjective;
    MeditationObjective.ObjectiveID = TEXT("meditate_at_stones");
    MeditationObjective.Description = FText::FromString(TEXT("Meditate at the ancient stones for 60 seconds"));
    MeditationObjective.RequiredProgress = 60;
    SacredGroveQuest.Objectives.Add(MeditationObjective);

    RegisterQuest(SacredGroveQuest);

    // Discovery Quest: Ancient Cave
    FQuestData CaveQuest;
    CaveQuest.QuestID = TEXT("ancient_cave_discovery");
    CaveQuest.QuestTitle = FText::FromString(TEXT("Echoes of the Past"));
    CaveQuest.QuestDescription = FText::FromString(TEXT("Strange markings have been discovered in a cave near the river. Investigate these ancient symbols and uncover their meaning."));
    CaveQuest.QuestType = EQuestType::Discovery;
    CaveQuest.QuestState = EQuestState::Available;
    CaveQuest.ExperienceReward = 150;
    CaveQuest.EstimatedDuration = 600.0f; // 10 minutes

    FQuestObjective CaveObjective;
    CaveObjective.ObjectiveID = TEXT("explore_cave");
    CaveObjective.Description = FText::FromString(TEXT("Explore the ancient cave"));
    CaveObjective.RequiredProgress = 1;
    CaveQuest.Objectives.Add(CaveObjective);

    FQuestObjective SymbolObjective;
    SymbolObjective.ObjectiveID = TEXT("examine_symbols");
    SymbolObjective.Description = FText::FromString(TEXT("Examine the ancient symbols (0/5)"));
    SymbolObjective.RequiredProgress = 5;
    CaveQuest.Objectives.Add(SymbolObjective);

    RegisterQuest(CaveQuest);

    // Survival Quest: River Crossing
    FQuestData RiverQuest;
    RiverQuest.QuestID = TEXT("river_crossing_survival");
    RiverQuest.QuestTitle = FText::FromString(TEXT("The Rushing Waters"));
    RiverQuest.QuestDescription = FText::FromString(TEXT("The seasonal floods have made the river crossing dangerous. Find a safe way across while avoiding the territorial river beasts."));
    RiverQuest.QuestType = EQuestType::Survival;
    RiverQuest.QuestState = EQuestState::Inactive;
    RiverQuest.ExperienceReward = 200;
    RiverQuest.EstimatedDuration = 900.0f; // 15 minutes
    RiverQuest.Prerequisites.Add(TEXT("sacred_grove_meditation"));

    FQuestObjective CrossingObjective;
    CrossingObjective.ObjectiveID = TEXT("find_safe_crossing");
    CrossingObjective.Description = FText::FromString(TEXT("Find a safe way to cross the river"));
    CrossingObjective.RequiredProgress = 1;
    RiverQuest.Objectives.Add(CrossingObjective);

    FQuestObjective AvoidBeastsObjective;
    AvoidBeastsObjective.ObjectiveID = TEXT("avoid_river_beasts");
    AvoidBeastsObjective.Description = FText::FromString(TEXT("Cross without alerting the river beasts"));
    AvoidBeastsObjective.RequiredProgress = 1;
    RiverQuest.Objectives.Add(AvoidBeastsObjective);

    RegisterQuest(RiverQuest);

    UE_LOG(LogTemp, Log, TEXT("Initialized %d default quests"), AllQuests.Num());
}

void AQuestSystemManager::UpdateQuestState(const FString& QuestID, EQuestState NewState)
{
    if (AllQuests.Contains(QuestID))
    {
        AllQuests.Find(QuestID)->QuestState = NewState;
    }
}

bool AQuestSystemManager::CheckAllObjectivesCompleted(const FString& QuestID) const
{
    if (!AllQuests.Contains(QuestID))
    {
        return false;
    }

    const FQuestData* QuestData = AllQuests.Find(QuestID);
    if (!QuestData)
    {
        return false;
    }

    for (const FQuestObjective& Objective : QuestData->Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            return false;
        }
    }

    return true;
}
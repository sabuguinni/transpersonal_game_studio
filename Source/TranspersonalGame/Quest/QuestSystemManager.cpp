#include "QuestSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AQuestSystemManager::AQuestSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize spiritual growth tracking
    TotalSpiritualGrowth = 0.0f;
    
    // Initialize emotional journey tracking
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Wonder, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Fear, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Hope, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Determination, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Enlightenment, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Connection, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Growth, 0.0f);
    EmotionalJourneyProgress.Add(EQuest_EmotionalJourney::Transcendence, 0.0f);
}

void AQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest System Manager initialized"));
    
    // Load quest data if available
    if (QuestDataTable)
    {
        TArray<FQuest_QuestData*> AllQuests;
        QuestDataTable->GetAllRows<FQuest_QuestData>(TEXT("Quest System"), AllQuests);
        
        UE_LOG(LogTemp, Warning, TEXT("Loaded %d quests from data table"), AllQuests.Num());
    }
}

void AQuestSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update active quest timers and check for time-based objectives
    for (auto& QuestPair : ActiveQuests)
    {
        FQuest_ActiveQuest& ActiveQuest = QuestPair.Value;
        
        // Check for survival time objectives
        for (auto& Objective : ActiveQuest.QuestData.Objectives)
        {
            if (Objective.Type == EQuest_ObjectiveType::SurviveTime && !Objective.bIsCompleted)
            {
                float ElapsedTime = GetWorld()->GetTimeSeconds() - ActiveQuest.StartTime;
                int32 ElapsedMinutes = FMath::FloorToInt(ElapsedTime / 60.0f);
                
                if (ElapsedMinutes >= Objective.RequiredAmount)
                {
                    UpdateObjectiveProgress(QuestPair.Key, Objective.ObjectiveID, Objective.RequiredAmount);
                }
            }
        }
    }
}

bool AQuestSystemManager::StartQuest(const FString& QuestID)
{
    if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s is already active or completed"), *QuestID);
        return false;
    }
    
    if (!CanStartQuest(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest %s - prerequisites not met"), *QuestID);
        return false;
    }
    
    // Get quest data from table
    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest data table is null"));
        return false;
    }
    
    FQuest_QuestData* QuestData = QuestDataTable->FindRow<FQuest_QuestData>(FName(*QuestID), TEXT("StartQuest"));
    if (!QuestData)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest data not found for ID: %s"), *QuestID);
        return false;
    }
    
    // Create active quest
    FQuest_ActiveQuest NewActiveQuest;
    NewActiveQuest.QuestData = *QuestData;
    NewActiveQuest.Status = EQuest_QuestStatus::Active;
    NewActiveQuest.StartTime = GetWorld()->GetTimeSeconds();
    
    ActiveQuests.Add(QuestID, NewActiveQuest);
    
    // Broadcast quest started
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Active);
    
    UE_LOG(LogTemp, Warning, TEXT("Started quest: %s"), *QuestData->Title.ToString());
    
    return true;
}

bool AQuestSystemManager::CompleteQuest(const FString& QuestID)
{
    FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest %s is not active"), *QuestID);
        return false;
    }
    
    // Check if all required objectives are completed
    for (const auto& Objective : ActiveQuest->QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest %s cannot be completed - objective %s not finished"), *QuestID, *Objective.ObjectiveID);
            return false;
        }
    }
    
    // Complete the quest
    ActiveQuest->Status = EQuest_QuestStatus::Completed;
    ActiveQuest->CompletionTime = GetWorld()->GetTimeSeconds();
    ActiveQuest->CompletionCount++;
    
    // Give rewards
    GiveQuestRewards(ActiveQuest->QuestData);
    
    // Add to completed quests
    CompletedQuestIDs.AddUnique(QuestID);
    
    // Remove from active quests if not repeatable
    if (!ActiveQuest->QuestData.bIsRepeatable)
    {
        ActiveQuests.Remove(QuestID);
    }
    else
    {
        // Reset for repeat
        ActiveQuest->Status = EQuest_QuestStatus::Available;
        for (auto& Objective : ActiveQuest->QuestData.Objectives)
        {
            Objective.CurrentAmount = 0;
            Objective.bIsCompleted = false;
        }
    }
    
    // Broadcast completion
    OnQuestCompleted.Broadcast(QuestID);
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Completed);
    
    UE_LOG(LogTemp, Warning, TEXT("Completed quest: %s"), *ActiveQuest->QuestData.Title.ToString());
    
    return true;
}

bool AQuestSystemManager::FailQuest(const FString& QuestID)
{
    FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        return false;
    }
    
    ActiveQuest->Status = EQuest_QuestStatus::Failed;
    
    // Broadcast failure
    OnQuestStatusChanged.Broadcast(QuestID, EQuest_QuestStatus::Failed);
    
    UE_LOG(LogTemp, Warning, TEXT("Failed quest: %s"), *ActiveQuest->QuestData.Title.ToString());
    
    return true;
}

bool AQuestSystemManager::IsQuestActive(const FString& QuestID) const
{
    const FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    return ActiveQuest && ActiveQuest->Status == EQuest_QuestStatus::Active;
}

bool AQuestSystemManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

EQuest_QuestStatus AQuestSystemManager::GetQuestStatus(const FString& QuestID) const
{
    const FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (ActiveQuest)
    {
        return ActiveQuest->Status;
    }
    
    if (IsQuestCompleted(QuestID))
    {
        return EQuest_QuestStatus::Completed;
    }
    
    return EQuest_QuestStatus::NotStarted;
}

bool AQuestSystemManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Amount)
{
    FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        return false;
    }
    
    // Find the objective
    for (auto& Objective : ActiveQuest->QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            if (Objective.bIsCompleted)
            {
                return false; // Already completed
            }
            
            Objective.CurrentAmount = FMath::Min(Objective.CurrentAmount + Amount, Objective.RequiredAmount);
            
            // Check if objective is now completed
            if (Objective.CurrentAmount >= Objective.RequiredAmount)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), *Objective.Description.ToString());
                
                // Track emotional moment for objective completion
                TrackEmotionalMoment(QuestID, ActiveQuest->QuestData.EmotionalTheme, 1.0f);
            }
            
            // Broadcast objective update
            OnObjectiveUpdated.Broadcast(QuestID, ObjectiveID, Objective.CurrentAmount);
            
            // Check if quest is now complete
            CheckQuestCompletion(QuestID);
            
            return true;
        }
    }
    
    return false;
}

bool AQuestSystemManager::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const
{
    const FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        return false;
    }
    
    for (const auto& Objective : ActiveQuest->QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.bIsCompleted;
        }
    }
    
    return false;
}

int32 AQuestSystemManager::GetObjectiveProgress(const FString& QuestID, const FString& ObjectiveID) const
{
    const FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        return 0;
    }
    
    for (const auto& Objective : ActiveQuest->QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            return Objective.CurrentAmount;
        }
    }
    
    return 0;
}

TArray<FString> AQuestSystemManager::GetAvailableQuests() const
{
    TArray<FString> AvailableQuests;
    
    if (!QuestDataTable)
    {
        return AvailableQuests;
    }
    
    TArray<FQuest_QuestData*> AllQuests;
    QuestDataTable->GetAllRows<FQuest_QuestData>(TEXT("GetAvailableQuests"), AllQuests);
    
    for (const FQuest_QuestData* QuestData : AllQuests)
    {
        if (QuestData && !IsQuestActive(QuestData->QuestID) && !IsQuestCompleted(QuestData->QuestID))
        {
            if (CanStartQuest(QuestData->QuestID))
            {
                AvailableQuests.Add(QuestData->QuestID);
            }
        }
    }
    
    return AvailableQuests;
}

TArray<FString> AQuestSystemManager::GetActiveQuests() const
{
    TArray<FString> ActiveQuestList;
    
    for (const auto& QuestPair : ActiveQuests)
    {
        if (QuestPair.Value.Status == EQuest_QuestStatus::Active)
        {
            ActiveQuestList.Add(QuestPair.Key);
        }
    }
    
    return ActiveQuestList;
}

TArray<FString> AQuestSystemManager::GetCompletedQuests() const
{
    return CompletedQuestIDs;
}

FQuest_QuestData AQuestSystemManager::GetQuestData(const FString& QuestID) const
{
    if (QuestDataTable)
    {
        FQuest_QuestData* QuestData = QuestDataTable->FindRow<FQuest_QuestData>(FName(*QuestID), TEXT("GetQuestData"));
        if (QuestData)
        {
            return *QuestData;
        }
    }
    
    return FQuest_QuestData();
}

void AQuestSystemManager::TrackEmotionalMoment(const FString& QuestID, EQuest_EmotionalJourney Emotion, float Intensity)
{
    UpdateEmotionalJourney(Emotion, Intensity);
    
    UE_LOG(LogTemp, Log, TEXT("Emotional moment tracked - Quest: %s, Emotion: %d, Intensity: %f"), 
           *QuestID, (int32)Emotion, Intensity);
}

float AQuestSystemManager::GetSpiritualGrowthLevel() const
{
    return TotalSpiritualGrowth;
}

bool AQuestSystemManager::CanStartQuest(const FString& QuestID) const
{
    if (!QuestDataTable)
    {
        return false;
    }
    
    FQuest_QuestData* QuestData = QuestDataTable->FindRow<FQuest_QuestData>(FName(*QuestID), TEXT("CanStartQuest"));
    if (!QuestData)
    {
        return false;
    }
    
    return ArePrerequisitesMet(QuestData->Prerequisites);
}

bool AQuestSystemManager::ArePrerequisitesMet(const TArray<FString>& Prerequisites) const
{
    for (const FString& PrereqID : Prerequisites)
    {
        if (!IsQuestCompleted(PrereqID))
        {
            return false;
        }
    }
    
    return true;
}

void AQuestSystemManager::CheckQuestCompletion(const FString& QuestID)
{
    FQuest_ActiveQuest* ActiveQuest = ActiveQuests.Find(QuestID);
    if (!ActiveQuest)
    {
        return;
    }
    
    // Check if all required objectives are completed
    bool bAllRequiredCompleted = true;
    for (const auto& Objective : ActiveQuest->QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            bAllRequiredCompleted = false;
            break;
        }
    }
    
    if (bAllRequiredCompleted)
    {
        CompleteQuest(QuestID);
    }
}

void AQuestSystemManager::GiveQuestRewards(const FQuest_QuestData& QuestData)
{
    // Add spiritual growth
    TotalSpiritualGrowth += QuestData.SpiritualGrowthReward;
    
    // Track emotional journey completion
    UpdateEmotionalJourney(QuestData.EmotionalTheme, 2.0f); // Bonus for completion
    
    UE_LOG(LogTemp, Warning, TEXT("Quest rewards given - XP: %d, Spiritual Growth: %f"), 
           QuestData.ExperienceReward, QuestData.SpiritualGrowthReward);
}

void AQuestSystemManager::UpdateEmotionalJourney(EQuest_EmotionalJourney Emotion, float Intensity)
{
    if (EmotionalJourneyProgress.Contains(Emotion))
    {
        EmotionalJourneyProgress[Emotion] += Intensity;
        
        // Cap at reasonable maximum
        EmotionalJourneyProgress[Emotion] = FMath::Min(EmotionalJourneyProgress[Emotion], 100.0f);
    }
}
#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UQuestManager::UQuestManager()
{
    bAutoSaveProgress = true;
    AutoSaveInterval = 30.0f; // Save every 30 seconds
    MaxActiveQuests = 10;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized"));
    
    // Load quest progress from save file
    LoadQuestProgress();
    
    // Setup auto-save timer
    if (bAutoSaveProgress && AutoSaveInterval > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                AutoSaveTimerHandle,
                this,
                &UQuestManager::AutoSaveQuestProgress,
                AutoSaveInterval,
                true
            );
        }
    }
}

void UQuestManager::Deinitialize()
{
    // Save progress before shutdown
    SaveQuestProgress();
    
    // Clear auto-save timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AutoSaveTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Deinitialized"));
    
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!RegisteredQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s not found"), *QuestID);
        return false;
    }
    
    // Check prerequisites
    if (!CheckQuestPrerequisites(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Prerequisites not met for quest %s"), *QuestID);
        return false;
    }
    
    // Check if we have room for more active quests
    TArray<FQuest_QuestData> ActiveQuests = GetActiveQuests();
    if (ActiveQuests.Num() >= MaxActiveQuests)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Maximum active quests reached (%d)"), MaxActiveQuests);
        return false;
    }
    
    // Start the quest
    UpdateQuestStatus(QuestID, EQuest_QuestStatus::Active);
    
    // Initialize emotional journey
    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    EmotionalJourneys.Add(QuestID, QuestData.EmotionalArc);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Started quest %s"), *QuestID);
    
    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    if (!IsQuestActive(QuestID))
    {
        return false;
    }
    
    UpdateQuestStatus(QuestID, EQuest_QuestStatus::Completed);
    ProcessQuestCompletion(QuestID);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed quest %s"), *QuestID);
    
    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    if (!IsQuestActive(QuestID))
    {
        return false;
    }
    
    UpdateQuestStatus(QuestID, EQuest_QuestStatus::Failed);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Failed quest %s"), *QuestID);
    
    return true;
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    return GetQuestStatus(QuestID) == EQuest_QuestStatus::Active;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return GetQuestStatus(QuestID) == EQuest_QuestStatus::Completed;
}

EQuest_QuestStatus UQuestManager::GetQuestStatus(const FString& QuestID) const
{
    if (const EQuest_QuestStatus* Status = QuestStatuses.Find(QuestID))
    {
        return *Status;
    }
    return EQuest_QuestStatus::NotStarted;
}

FQuest_QuestData UQuestManager::GetQuestData(const FString& QuestID) const
{
    if (const FQuest_QuestData* Data = RegisteredQuests.Find(QuestID))
    {
        return *Data;
    }
    return FQuest_QuestData();
}

TArray<FQuest_QuestData> UQuestManager::GetActiveQuests() const
{
    TArray<FQuest_QuestData> ActiveQuests;
    
    for (const auto& QuestPair : RegisteredQuests)
    {
        if (IsQuestActive(QuestPair.Key))
        {
            ActiveQuests.Add(QuestPair.Value);
        }
    }
    
    return ActiveQuests;
}

TArray<FQuest_QuestData> UQuestManager::GetAvailableQuests() const
{
    TArray<FQuest_QuestData> AvailableQuests;
    
    for (const auto& QuestPair : RegisteredQuests)
    {
        EQuest_QuestStatus Status = GetQuestStatus(QuestPair.Key);
        if (Status == EQuest_QuestStatus::Available || Status == EQuest_QuestStatus::NotStarted)
        {
            if (CheckQuestPrerequisites(QuestPair.Key))
            {
                AvailableQuests.Add(QuestPair.Value);
            }
        }
    }
    
    return AvailableQuests;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!RegisteredQuests.Contains(QuestID) || !IsQuestActive(QuestID))
    {
        return false;
    }
    
    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.bIsCompleted = true;
            Objective.CurrentProgress = Objective.RequiredQuantity;
            
            // Check if all objectives are completed
            bool bAllCompleted = true;
            for (const FQuest_ObjectiveData& Obj : QuestData.Objectives)
            {
                if (!Obj.bIsCompleted && !Obj.bIsOptional)
                {
                    bAllCompleted = false;
                    break;
                }
            }
            
            // Broadcast objective completion
            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID, bAllCompleted);
            
            // Complete quest if all objectives done
            if (bAllCompleted)
            {
                CompleteQuest(QuestID);
            }
            
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Completed objective %s in quest %s"), *ObjectiveID, *QuestID);
            
            return true;
        }
    }
    
    return false;
}

bool UQuestManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!RegisteredQuests.Contains(QuestID) || !IsQuestActive(QuestID))
    {
        return false;
    }
    
    FQuest_QuestData& QuestData = RegisteredQuests[QuestID];
    
    for (FQuest_ObjectiveData& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Clamp(Progress, 0, Objective.RequiredQuantity);
            
            // Auto-complete if progress reaches requirement
            if (Objective.CurrentProgress >= Objective.RequiredQuantity)
            {
                return CompleteObjective(QuestID, ObjectiveID);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UQuestManager::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const
{
    if (const FQuest_QuestData* QuestData = RegisteredQuests.Find(QuestID))
    {
        for (const FQuest_ObjectiveData& Objective : QuestData->Objectives)
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
    if (const FQuest_QuestData* QuestData = RegisteredQuests.Find(QuestID))
    {
        for (const FQuest_ObjectiveData& Objective : QuestData->Objectives)
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
    if (FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
    {
        EQuest_EmotionalState OldState = Journey->CurrentEmotion;
        Journey->CurrentEmotion = NewState;
        
        // Calculate progress towards target emotion
        CalculateEmotionalProgress(QuestID);
        
        // Broadcast emotional state change
        OnEmotionalStateChanged.Broadcast(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Emotional state changed from %d to %d for quest %s"), 
               (int32)OldState, (int32)NewState, *QuestID);
    }
}

EQuest_EmotionalState UQuestManager::GetCurrentEmotionalState(const FString& QuestID) const
{
    if (const FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
    {
        return Journey->CurrentEmotion;
    }
    return EQuest_EmotionalState::Neutral;
}

float UQuestManager::GetEmotionalProgress(const FString& QuestID) const
{
    if (const FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
    {
        return Journey->EmotionalProgress;
    }
    return 0.0f;
}

void UQuestManager::AddEmotionalMilestone(const FString& QuestID, const FText& Milestone)
{
    if (FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
    {
        Journey->EmotionalMilestones.Add(Milestone);
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Added emotional milestone for quest %s"), *QuestID);
    }
}

bool UQuestManager::RegisterQuest(const FQuest_QuestData& QuestData)
{
    if (QuestData.QuestID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot register quest with empty ID"));
        return false;
    }
    
    RegisteredQuests.Add(QuestData.QuestID, QuestData);
    QuestStatuses.Add(QuestData.QuestID, QuestData.Status);
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Registered quest %s"), *QuestData.QuestID);
    
    return true;
}

bool UQuestManager::UnregisterQuest(const FString& QuestID)
{
    if (RegisteredQuests.Contains(QuestID))
    {
        RegisteredQuests.Remove(QuestID);
        QuestStatuses.Remove(QuestID);
        EmotionalJourneys.Remove(QuestID);
        
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Unregistered quest %s"), *QuestID);
        return true;
    }
    
    return false;
}

void UQuestManager::SaveQuestProgress()
{
    // TODO: Implement save/load system with UE5 save game system
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Saving quest progress (stub implementation)"));
}

void UQuestManager::LoadQuestProgress()
{
    // TODO: Implement save/load system with UE5 save game system
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Loading quest progress (stub implementation)"));
}

void UQuestManager::ResetAllQuests()
{
    QuestStatuses.Empty();
    EmotionalJourneys.Empty();
    
    // Reset all registered quests to not started
    for (auto& QuestPair : RegisteredQuests)
    {
        QuestStatuses.Add(QuestPair.Key, EQuest_QuestStatus::NotStarted);
    }
    
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Reset all quests"));
}

bool UQuestManager::CheckQuestPrerequisites(const FString& QuestID) const
{
    if (const FQuest_QuestData* QuestData = RegisteredQuests.Find(QuestID))
    {
        for (const FString& PrereqID : QuestData->Prerequisites)
        {
            if (!IsQuestCompleted(PrereqID))
            {
                return false;
            }
        }
    }
    return true;
}

void UQuestManager::UpdateQuestStatus(const FString& QuestID, EQuest_QuestStatus NewStatus)
{
    EQuest_QuestStatus OldStatus = GetQuestStatus(QuestID);
    QuestStatuses.Add(QuestID, NewStatus);
    
    // Broadcast status change
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}

void UQuestManager::ProcessQuestCompletion(const FString& QuestID)
{
    if (const FQuest_QuestData* QuestData = RegisteredQuests.Find(QuestID))
    {
        // TODO: Award experience, items, spiritual growth
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Processing completion rewards for quest %s"), *QuestID);
        
        // Update emotional journey to target state
        if (FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
        {
            Journey->CurrentEmotion = Journey->TargetEmotion;
            Journey->EmotionalProgress = 1.0f;
        }
    }
}

void UQuestManager::CalculateEmotionalProgress(const FString& QuestID)
{
    if (FQuest_EmotionalJourney* Journey = EmotionalJourneys.Find(QuestID))
    {
        // Simple progress calculation based on emotional state progression
        // TODO: Implement more sophisticated emotional progression system
        int32 StartValue = (int32)Journey->StartingEmotion;
        int32 CurrentValue = (int32)Journey->CurrentEmotion;
        int32 TargetValue = (int32)Journey->TargetEmotion;
        
        if (TargetValue != StartValue)
        {
            Journey->EmotionalProgress = FMath::Clamp(
                (float)(CurrentValue - StartValue) / (float)(TargetValue - StartValue),
                0.0f, 1.0f
            );
        }
        else
        {
            Journey->EmotionalProgress = 1.0f;
        }
    }
}

void UQuestManager::AutoSaveQuestProgress()
{
    SaveQuestProgress();
}
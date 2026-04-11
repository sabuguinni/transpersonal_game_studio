#include "QuestSystemManager.h"
#include "QuestData.h"
#include "QuestInstance.h"
#include "QuestObjective.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

UQuestSystemManager::UQuestSystemManager()
{
    bIsInitialized = false;
}

void UQuestSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Initializing Quest System"));
    
    InitializeEmotionalJourneys();
    LoadQuestProgress();
    
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Quest System initialized successfully"));
}

void UQuestSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Shutting down Quest System"));
    
    SaveQuestProgress();
    
    // Clear all quest data
    ActiveQuests.Empty();
    CompletedQuests.Empty();
    FailedQuests.Empty();
    QuestDatabase.Empty();
    EmotionalJourneyProgress.Empty();
    
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UQuestSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UQuestSystemManager::InitializeEmotionalJourneys()
{
    // Initialize core emotional journey types
    EmotionalJourneyProgress.Add(TEXT("SpiritualAwakening"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("TribalBonding"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("NatureConnection"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("InnerHealing"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("WisdomGathering"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("CourageBuilding"), 0.0f);
    EmotionalJourneyProgress.Add(TEXT("CompassionDevelopment"), 0.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Initialized %d emotional journey types"), EmotionalJourneyProgress.Num());
}

UQuestInstance* UQuestSystemManager::StartQuest(const FString& QuestID, AActor* QuestGiver)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestSystemManager: Cannot start quest - system not initialized"));
        return nullptr;
    }

    // Check if quest is already active
    if (ActiveQuests.Contains(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Quest %s is already active"), *QuestID);
        return ActiveQuests[QuestID];
    }

    // Check if quest exists in database
    UQuestData* QuestData = GetQuestData(QuestID);
    if (!QuestData)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestSystemManager: Quest %s not found in database"), *QuestID);
        return nullptr;
    }

    // Check prerequisites
    if (!CheckQuestPrerequisites(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Prerequisites not met for quest %s"), *QuestID);
        return nullptr;
    }

    // Create quest instance
    UQuestInstance* NewQuest = NewObject<UQuestInstance>(this);
    NewQuest->Initialize(QuestData, QuestGiver);
    
    // Add to active quests
    ActiveQuests.Add(QuestID, NewQuest);
    
    // Broadcast quest started
    OnQuestStateChanged.Broadcast(NewQuest, EQuestState::Active);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Started quest %s"), *QuestID);
    
    return NewQuest;
}

bool UQuestSystemManager::CompleteQuest(const FString& QuestID)
{
    UQuestInstance* Quest = GetActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Cannot complete quest %s - not active"), *QuestID);
        return false;
    }

    // Mark quest as completed
    Quest->SetQuestState(EQuestState::Completed);
    
    // Move from active to completed
    ActiveQuests.Remove(QuestID);
    CompletedQuests.Add(Quest);
    
    // Process completion effects
    ProcessQuestCompletion(Quest);
    
    // Update quest chains
    UpdateQuestChains(QuestID);
    
    // Broadcast completion
    OnQuestStateChanged.Broadcast(Quest, EQuestState::Completed);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Completed quest %s"), *QuestID);
    
    return true;
}

bool UQuestSystemManager::FailQuest(const FString& QuestID, const FString& Reason)
{
    UQuestInstance* Quest = GetActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Cannot fail quest %s - not active"), *QuestID);
        return false;
    }

    // Mark quest as failed
    Quest->SetQuestState(EQuestState::Failed);
    Quest->SetFailureReason(Reason);
    
    // Move from active to failed
    ActiveQuests.Remove(QuestID);
    FailedQuests.Add(Quest);
    
    // Broadcast failure
    OnQuestStateChanged.Broadcast(Quest, EQuestState::Failed);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Failed quest %s - Reason: %s"), *QuestID, *Reason);
    
    return true;
}

bool UQuestSystemManager::AbandonQuest(const FString& QuestID)
{
    UQuestInstance* Quest = GetActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Cannot abandon quest %s - not active"), *QuestID);
        return false;
    }

    // Only allow abandoning if quest allows it
    if (!Quest->GetQuestData()->bCanBeAbandoned)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Quest %s cannot be abandoned"), *QuestID);
        return false;
    }

    // Mark quest as abandoned
    Quest->SetQuestState(EQuestState::Abandoned);
    
    // Remove from active quests
    ActiveQuests.Remove(QuestID);
    
    // Broadcast abandonment
    OnQuestStateChanged.Broadcast(Quest, EQuestState::Abandoned);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Abandoned quest %s"), *QuestID);
    
    return true;
}

UQuestInstance* UQuestSystemManager::GetActiveQuest(const FString& QuestID) const
{
    if (ActiveQuests.Contains(QuestID))
    {
        return ActiveQuests[QuestID];
    }
    return nullptr;
}

TArray<UQuestInstance*> UQuestSystemManager::GetActiveQuests() const
{
    TArray<UQuestInstance*> Result;
    ActiveQuests.GenerateValueArray(Result);
    return Result;
}

TArray<UQuestInstance*> UQuestSystemManager::GetCompletedQuests() const
{
    return CompletedQuests;
}

bool UQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    UQuestInstance* Quest = GetActiveQuest(QuestID);
    if (!Quest)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Cannot complete objective - quest %s not active"), *QuestID);
        return false;
    }

    UQuestObjective* Objective = Quest->GetObjective(ObjectiveID);
    if (!Objective)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Objective %s not found in quest %s"), *ObjectiveID, *QuestID);
        return false;
    }

    // Complete the objective
    Objective->Complete();
    
    // Process objective completion
    ProcessObjectiveCompletion(Quest, Objective);
    
    // Check if quest is complete
    if (Quest->AreAllObjectivesComplete())
    {
        CompleteQuest(QuestID);
    }
    
    // Broadcast objective completion
    OnObjectiveCompleted.Broadcast(Quest, Objective);
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Completed objective %s in quest %s"), *ObjectiveID, *QuestID);
    
    return true;
}

bool UQuestSystemManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, float Progress)
{
    UQuestInstance* Quest = GetActiveQuest(QuestID);
    if (!Quest)
    {
        return false;
    }

    UQuestObjective* Objective = Quest->GetObjective(ObjectiveID);
    if (!Objective)
    {
        return false;
    }

    Objective->UpdateProgress(Progress);
    
    // Check if objective is now complete
    if (Objective->IsComplete())
    {
        ProcessObjectiveCompletion(Quest, Objective);
        OnObjectiveCompleted.Broadcast(Quest, Objective);
        
        // Check if quest is complete
        if (Quest->AreAllObjectivesComplete())
        {
            CompleteQuest(QuestID);
        }
    }
    
    return true;
}

void UQuestSystemManager::UpdateEmotionalJourney(const FString& JourneyType, float ProgressDelta)
{
    if (!EmotionalJourneyProgress.Contains(JourneyType))
    {
        EmotionalJourneyProgress.Add(JourneyType, 0.0f);
    }
    
    float CurrentProgress = EmotionalJourneyProgress[JourneyType];
    float NewProgress = FMath::Clamp(CurrentProgress + ProgressDelta, 0.0f, 1.0f);
    
    EmotionalJourneyProgress[JourneyType] = NewProgress;
    
    // Broadcast progress update
    OnEmotionalJourneyProgressed.Broadcast(JourneyType, NewProgress);
    
    UE_LOG(LogTemp, Log, TEXT("QuestSystemManager: Updated emotional journey %s: %.2f -> %.2f"), 
           *JourneyType, CurrentProgress, NewProgress);
}

float UQuestSystemManager::GetEmotionalJourneyProgress(const FString& JourneyType) const
{
    if (EmotionalJourneyProgress.Contains(JourneyType))
    {
        return EmotionalJourneyProgress[JourneyType];
    }
    return 0.0f;
}

TArray<FString> UQuestSystemManager::GetActiveEmotionalJourneys() const
{
    TArray<FString> Result;
    for (const auto& Journey : EmotionalJourneyProgress)
    {
        if (Journey.Value > 0.0f)
        {
            Result.Add(Journey.Key);
        }
    }
    return Result;
}

UQuestData* UQuestSystemManager::GetQuestData(const FString& QuestID) const
{
    if (QuestDatabase.Contains(QuestID))
    {
        return QuestDatabase[QuestID];
    }
    return nullptr;
}

bool UQuestSystemManager::CheckQuestPrerequisites(const FString& QuestID, AActor* Player) const
{
    UQuestData* QuestData = GetQuestData(QuestID);
    if (!QuestData)
    {
        return false;
    }

    // Check completed quest prerequisites
    for (const FString& PrereqQuest : QuestData->PrerequisiteQuests)
    {
        if (!HasCompletedQuest(PrereqQuest))
        {
            return false;
        }
    }

    // Check emotional journey prerequisites
    for (const auto& EmotionalPrereq : QuestData->EmotionalPrerequisites)
    {
        float CurrentProgress = GetEmotionalJourneyProgress(EmotionalPrereq.Key);
        if (CurrentProgress < EmotionalPrereq.Value)
        {
            return false;
        }
    }

    return true;
}

bool UQuestSystemManager::HasCompletedQuest(const FString& QuestID) const
{
    for (const UQuestInstance* Quest : CompletedQuests)
    {
        if (Quest && Quest->GetQuestData()->QuestID == QuestID)
        {
            return true;
        }
    }
    return false;
}

void UQuestSystemManager::ProcessQuestCompletion(UQuestInstance* Quest)
{
    if (!Quest || !Quest->GetQuestData())
    {
        return;
    }

    UQuestData* QuestData = Quest->GetQuestData();
    
    // Apply emotional journey rewards
    for (const auto& EmotionalReward : QuestData->EmotionalRewards)
    {
        UpdateEmotionalJourney(EmotionalReward.Key, EmotionalReward.Value);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Processed completion for quest %s"), *QuestData->QuestID);
}

void UQuestSystemManager::ProcessObjectiveCompletion(UQuestInstance* Quest, UQuestObjective* Objective)
{
    if (!Quest || !Objective)
    {
        return;
    }

    // Apply objective-specific emotional rewards
    for (const auto& EmotionalReward : Objective->GetEmotionalRewards())
    {
        UpdateEmotionalJourney(EmotionalReward.Key, EmotionalReward.Value);
    }
}

void UQuestSystemManager::UpdateQuestChains(const FString& CompletedQuestID)
{
    // Check if completing this quest unlocks any new quests
    for (const auto& QuestEntry : QuestDatabase)
    {
        UQuestData* QuestData = QuestEntry.Value;
        if (QuestData && QuestData->PrerequisiteQuests.Contains(CompletedQuestID))
        {
            if (IsQuestAvailable(QuestData->QuestID))
            {
                UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Quest %s is now available after completing %s"), 
                       *QuestData->QuestID, *CompletedQuestID);
            }
        }
    }
}

void UQuestSystemManager::SaveQuestProgress()
{
    // Implementation for saving quest progress to persistent storage
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Saving quest progress..."));
}

void UQuestSystemManager::LoadQuestProgress()
{
    // Implementation for loading quest progress from persistent storage
    UE_LOG(LogTemp, Warning, TEXT("QuestSystemManager: Loading quest progress..."));
}

void UQuestSystemManager::DebugPrintActiveQuests() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ACTIVE QUESTS ==="));
    for (const auto& QuestEntry : ActiveQuests)
    {
        UQuestInstance* Quest = QuestEntry.Value;
        if (Quest && Quest->GetQuestData())
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest: %s - %s"), 
                   *Quest->GetQuestData()->QuestID, 
                   *Quest->GetQuestData()->Title);
        }
    }
}

void UQuestSystemManager::DebugPrintEmotionalJourneys() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== EMOTIONAL JOURNEYS ==="));
    for (const auto& Journey : EmotionalJourneyProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s: %.2f"), *Journey.Key, Journey.Value);
    }
}
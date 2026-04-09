#include "QuestManager.h"
#include "QuestInstance.h"
#include "QuestObjective.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuestManager::UQuestManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check quest states every second
}

void UQuestManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeQuestSystem();
}

void UQuestManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update active quest instances
    for (auto& QuestPair : ActiveQuestInstances)
    {
        if (QuestPair.Value)
        {
            QuestPair.Value->UpdateQuest(DeltaTime);
        }
    }
    
    // Check for newly available quests
    CheckQuestAvailability();
}

bool UQuestManager::StartQuest(const FString& QuestID)
{
    if (!CanStartQuest(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest: %s"), *QuestID);
        return false;
    }

    // Get quest data
    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("QuestDataTable is null"));
        return false;
    }

    FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), TEXT("StartQuest"));
    if (!QuestData)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest data not found for ID: %s"), *QuestID);
        return false;
    }

    // Create quest instance
    UQuestInstance* QuestInstance = NewObject<UQuestInstance>(this);
    QuestInstance->Initialize(*QuestData);
    ActiveQuestInstances.Add(QuestID, QuestInstance);

    // Update quest state
    QuestStates.Add(QuestID, EQuestState::Active);
    NotifyQuestStateChange(QuestID, EQuestState::Active);

    UE_LOG(LogTemp, Log, TEXT("Started quest: %s - %s"), *QuestID, *QuestData->QuestTitle.ToString());
    OnQuestStarted.Broadcast(QuestID);

    return true;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    EQuestState* CurrentState = QuestStates.Find(QuestID);
    if (!CurrentState || *CurrentState != EQuestState::Active)
    {
        return false;
    }

    // Check if all objectives are completed
    UQuestInstance** QuestInstancePtr = ActiveQuestInstances.Find(QuestID);
    if (!QuestInstancePtr || !*QuestInstancePtr)
    {
        return false;
    }

    UQuestInstance* QuestInstance = *QuestInstancePtr;
    if (!QuestInstance->AreAllObjectivesCompleted())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete quest %s - not all objectives completed"), *QuestID);
        return false;
    }

    // Complete the quest
    QuestStates[QuestID] = EQuestState::Completed;
    NotifyQuestStateChange(QuestID, EQuestState::Completed);

    UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestID);
    OnQuestCompleted.Broadcast(QuestID);

    return true;
}

bool UQuestManager::FailQuest(const FString& QuestID)
{
    EQuestState* CurrentState = QuestStates.Find(QuestID);
    if (!CurrentState || *CurrentState != EQuestState::Active)
    {
        return false;
    }

    QuestStates[QuestID] = EQuestState::Failed;
    NotifyQuestStateChange(QuestID, EQuestState::Failed);

    // Remove from active instances
    ActiveQuestInstances.Remove(QuestID);

    UE_LOG(LogTemp, Log, TEXT("Failed quest: %s"), *QuestID);
    return true;
}

bool UQuestManager::TurnInQuest(const FString& QuestID)
{
    EQuestState* CurrentState = QuestStates.Find(QuestID);
    if (!CurrentState || *CurrentState != EQuestState::Completed)
    {
        return false;
    }

    QuestStates[QuestID] = EQuestState::Turned_In;
    NotifyQuestStateChange(QuestID, EQuestState::Turned_In);

    // Remove from active instances and give rewards
    UQuestInstance** QuestInstancePtr = ActiveQuestInstances.Find(QuestID);
    if (QuestInstancePtr && *QuestInstancePtr)
    {
        (*QuestInstancePtr)->GiveRewards();
        ActiveQuestInstances.Remove(QuestID);
    }

    UE_LOG(LogTemp, Log, TEXT("Turned in quest: %s"), *QuestID);
    return true;
}

EQuestState UQuestManager::GetQuestState(const FString& QuestID) const
{
    const EQuestState* State = QuestStates.Find(QuestID);
    return State ? *State : EQuestState::Inactive;
}

TArray<FString> UQuestManager::GetActiveQuests() const
{
    TArray<FString> ActiveQuests;
    for (const auto& QuestPair : QuestStates)
    {
        if (QuestPair.Value == EQuestState::Active)
        {
            ActiveQuests.Add(QuestPair.Key);
        }
    }
    return ActiveQuests;
}

TArray<FString> UQuestManager::GetAvailableQuests() const
{
    TArray<FString> AvailableQuests;
    for (const auto& QuestPair : QuestStates)
    {
        if (QuestPair.Value == EQuestState::Available)
        {
            AvailableQuests.Add(QuestPair.Key);
        }
    }
    return AvailableQuests;
}

TArray<FString> UQuestManager::GetCompletedQuests() const
{
    TArray<FString> CompletedQuests;
    for (const auto& QuestPair : QuestStates)
    {
        if (QuestPair.Value == EQuestState::Completed || QuestPair.Value == EQuestState::Turned_In)
        {
            CompletedQuests.Add(QuestPair.Key);
        }
    }
    return CompletedQuests;
}

bool UQuestManager::UpdateObjectiveProgress(const FString& ObjectiveID, float Progress)
{
    float& CurrentProgress = ObjectiveProgress.FindOrAdd(ObjectiveID);
    CurrentProgress = FMath::Clamp(Progress, 0.0f, 1.0f);

    // Check if objective is completed
    if (CurrentProgress >= 1.0f)
    {
        CompleteObjective(ObjectiveID);
    }

    UE_LOG(LogTemp, VeryVerbose, TEXT("Updated objective %s progress to %f"), *ObjectiveID, CurrentProgress);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& ObjectiveID)
{
    ObjectiveProgress.FindOrAdd(ObjectiveID) = 1.0f;

    // Find which quest this objective belongs to and check if quest can be completed
    for (auto& QuestPair : ActiveQuestInstances)
    {
        if (QuestPair.Value && QuestPair.Value->HasObjective(ObjectiveID))
        {
            QuestPair.Value->CompleteObjective(ObjectiveID);
            
            // Check if all objectives are completed
            if (QuestPair.Value->AreAllObjectivesCompleted())
            {
                CompleteQuest(QuestPair.Key);
            }
            break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Completed objective: %s"), *ObjectiveID);
    return true;
}

float UQuestManager::GetObjectiveProgress(const FString& ObjectiveID) const
{
    const float* Progress = ObjectiveProgress.Find(ObjectiveID);
    return Progress ? *Progress : 0.0f;
}

void UQuestManager::CheckQuestAvailability()
{
    if (!QuestDataTable)
    {
        return;
    }

    TArray<FName> RowNames = QuestDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FString QuestID = RowName.ToString();
        EQuestState CurrentState = GetQuestState(QuestID);
        
        if (CurrentState == EQuestState::Inactive)
        {
            FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(RowName, TEXT("CheckAvailability"));
            if (QuestData && ArePrerequisitesMet(*QuestData))
            {
                QuestStates.Add(QuestID, EQuestState::Available);
                NotifyQuestStateChange(QuestID, EQuestState::Available);
                UE_LOG(LogTemp, Log, TEXT("Quest %s is now available"), *QuestID);
            }
        }
    }
}

bool UQuestManager::CanStartQuest(const FString& QuestID) const
{
    EQuestState CurrentState = GetQuestState(QuestID);
    return CurrentState == EQuestState::Available;
}

void UQuestManager::InitializeQuestSystem()
{
    LoadQuestData();
    CheckQuestAvailability();
    
    UE_LOG(LogTemp, Log, TEXT("Quest system initialized"));
}

void UQuestManager::LoadQuestData()
{
    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestDataTable not set in QuestManager"));
        return;
    }

    // Initialize all quests as inactive
    TArray<FName> RowNames = QuestDataTable->GetRowNames();
    for (const FName& RowName : RowNames)
    {
        FString QuestID = RowName.ToString();
        QuestStates.Add(QuestID, EQuestState::Inactive);
    }

    UE_LOG(LogTemp, Log, TEXT("Loaded %d quests from data table"), RowNames.Num());
}

bool UQuestManager::ArePrerequisitesMet(const FQuestData& QuestData) const
{
    for (const FString& PrerequisiteID : QuestData.PrerequisiteQuests)
    {
        EQuestState PrereqState = GetQuestState(PrerequisiteID);
        if (PrereqState != EQuestState::Completed && PrereqState != EQuestState::Turned_In)
        {
            return false;
        }
    }
    return true;
}

void UQuestManager::NotifyQuestStateChange(const FString& QuestID, EQuestState NewState)
{
    OnQuestStateChanged.Broadcast(QuestID, NewState);
}
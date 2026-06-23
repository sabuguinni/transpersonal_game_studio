// QuestManager.cpp — Transpersonal Game Studio
// Agent #15 — Narrative & Dialogue Agent
// Implements quest tracking, objective management, and dialogue trigger integration.

#include "QuestManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UQuestManager::UQuestManager()
{
    ActiveQuestCount = 0;
    CompletedQuestCount = 0;
    bQuestSystemInitialized = false;
}

void UQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bQuestSystemInitialized = true;
    ActiveQuestCount = 0;
    CompletedQuestCount = 0;
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Initialized — quest system ready."));
}

void UQuestManager::Deinitialize()
{
    ActiveQuests.Empty();
    CompletedQuestIDs.Empty();
    Super::Deinitialize();
}

bool UQuestManager::StartQuest(const FNarr_QuestData& QuestData)
{
    if (!bQuestSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestManager: Cannot start quest — system not initialized."));
        return false;
    }

    // Check if already active
    for (const FNarr_ActiveQuest& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestData.QuestID)
        {
            UE_LOG(LogTemp, Warning, TEXT("QuestManager: Quest %s already active."), *QuestData.QuestID);
            return false;
        }
    }

    // Check if already completed
    if (CompletedQuestIDs.Contains(QuestData.QuestID))
    {
        UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest %s already completed."), *QuestData.QuestID);
        return false;
    }

    FNarr_ActiveQuest NewActiveQuest;
    NewActiveQuest.QuestID = QuestData.QuestID;
    NewActiveQuest.QuestTitle = QuestData.Title;
    NewActiveQuest.CurrentObjectiveIndex = 0;
    NewActiveQuest.bIsTracked = true;
    NewActiveQuest.Objectives = QuestData.Objectives;

    ActiveQuests.Add(NewActiveQuest);
    ActiveQuestCount = ActiveQuests.Num();

    OnQuestStarted.Broadcast(QuestData.QuestID, QuestData.Title);
    UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest STARTED — %s (%s)"), *QuestData.Title, *QuestData.QuestID);
    return true;
}

bool UQuestManager::CompleteObjective(const FString& QuestID, int32 ObjectiveIndex)
{
    for (FNarr_ActiveQuest& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestID)
        {
            if (!AQ.Objectives.IsValidIndex(ObjectiveIndex))
            {
                UE_LOG(LogTemp, Warning, TEXT("QuestManager: Invalid objective index %d for quest %s"), ObjectiveIndex, *QuestID);
                return false;
            }

            AQ.Objectives[ObjectiveIndex].bCompleted = true;
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Objective %d completed for quest %s"), ObjectiveIndex, *QuestID);

            // Check if all objectives done
            bool bAllDone = true;
            for (const FNarr_QuestObjective& Obj : AQ.Objectives)
            {
                if (!Obj.bCompleted)
                {
                    bAllDone = false;
                    break;
                }
            }

            if (bAllDone)
            {
                CompleteQuest(QuestID);
            }
            return true;
        }
    }
    return false;
}

bool UQuestManager::CompleteQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); ++i)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            FString Title = ActiveQuests[i].QuestTitle;
            ActiveQuests.RemoveAt(i);
            CompletedQuestIDs.Add(QuestID);
            ActiveQuestCount = ActiveQuests.Num();
            CompletedQuestCount = CompletedQuestIDs.Num();

            OnQuestCompleted.Broadcast(QuestID, Title);
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest COMPLETED — %s (%s)"), *Title, *QuestID);
            return true;
        }
    }
    return false;
}

bool UQuestManager::AbandonQuest(const FString& QuestID)
{
    for (int32 i = 0; i < ActiveQuests.Num(); ++i)
    {
        if (ActiveQuests[i].QuestID == QuestID)
        {
            ActiveQuests.RemoveAt(i);
            ActiveQuestCount = ActiveQuests.Num();
            UE_LOG(LogTemp, Log, TEXT("QuestManager: Quest ABANDONED — %s"), *QuestID);
            return true;
        }
    }
    return false;
}

FNarr_ActiveQuest UQuestManager::GetActiveQuest(const FString& QuestID) const
{
    for (const FNarr_ActiveQuest& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestID)
        {
            return AQ;
        }
    }
    return FNarr_ActiveQuest();
}

bool UQuestManager::IsQuestActive(const FString& QuestID) const
{
    for (const FNarr_ActiveQuest& AQ : ActiveQuests)
    {
        if (AQ.QuestID == QuestID) return true;
    }
    return false;
}

bool UQuestManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuestIDs.Contains(QuestID);
}

TArray<FNarr_ActiveQuest> UQuestManager::GetAllActiveQuests() const
{
    return ActiveQuests;
}

int32 UQuestManager::GetActiveQuestCount() const
{
    return ActiveQuestCount;
}

int32 UQuestManager::GetCompletedQuestCount() const
{
    return CompletedQuestCount;
}

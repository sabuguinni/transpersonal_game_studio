#include "QuestSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

UQuestSystemManager::UQuestSystemManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    TotalSpiritualEnergy = 0.0f;
}

void UQuestSystemManager::BeginPlay()
{
    Super::BeginPlay();
    LoadQuestData();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest System Manager initialized"));
}

void UQuestSystemManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check location-based quest triggers
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (ACharacter* PlayerCharacter = Cast<ACharacter>(PC->GetPawn()))
        {
            CheckLocationTriggers(PlayerCharacter->GetActorLocation());
        }
    }
}

bool UQuestSystemManager::StartQuest(const FString& QuestID)
{
    if (!CanStartQuest(QuestID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start quest: %s"), *QuestID);
        return false;
    }

    // Load quest from data table
    if (QuestDataTable)
    {
        FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), TEXT(""));
        if (QuestData)
        {
            FQuestData NewQuest = *QuestData;
            NewQuest.QuestStatus = EQuestStatus::InProgress;
            ActiveQuests.Add(QuestID, NewQuest);
            
            UpdateQuestStatus(QuestID, EQuestStatus::InProgress);
            
            UE_LOG(LogTemp, Log, TEXT("Started quest: %s"), *QuestData->QuestName.ToString());
            return true;
        }
    }

    UE_LOG(LogTemp, Error, TEXT("Quest data not found for ID: %s"), *QuestID);
    return false;
}

bool UQuestSystemManager::CompleteQuest(const FString& QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& QuestData = ActiveQuests[QuestID];
    
    // Check if all required objectives are completed
    bool bAllObjectivesComplete = true;
    for (const FQuestObjective& Objective : QuestData.Objectives)
    {
        if (!Objective.bIsOptional && !Objective.bIsCompleted)
        {
            bAllObjectivesComplete = false;
            break;
        }
    }

    if (!bAllObjectivesComplete)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot complete quest %s - not all objectives finished"), *QuestID);
        return false;
    }

    // Complete the quest
    QuestData.QuestStatus = EQuestStatus::Completed;
    CompletedQuests.Add(QuestID, QuestData);
    ActiveQuests.Remove(QuestID);

    // Give rewards
    GiveQuestRewards(QuestData);

    // Broadcast completion
    OnQuestCompleted.Broadcast(QuestID);
    UpdateQuestStatus(QuestID, EQuestStatus::Completed);

    UE_LOG(LogTemp, Log, TEXT("Completed quest: %s"), *QuestData.QuestName.ToString());
    return true;
}

bool UQuestSystemManager::AbandonQuest(const FString& QuestID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& QuestData = ActiveQuests[QuestID];
    QuestData.QuestStatus = EQuestStatus::Abandoned;
    ActiveQuests.Remove(QuestID);

    UpdateQuestStatus(QuestID, EQuestStatus::Abandoned);

    UE_LOG(LogTemp, Log, TEXT("Abandoned quest: %s"), *QuestData.QuestName.ToString());
    return true;
}

bool UQuestSystemManager::IsQuestActive(const FString& QuestID) const
{
    return ActiveQuests.Contains(QuestID);
}

bool UQuestSystemManager::IsQuestCompleted(const FString& QuestID) const
{
    return CompletedQuests.Contains(QuestID);
}

TArray<FQuestData> UQuestSystemManager::GetActiveQuests() const
{
    TArray<FQuestData> Result;
    for (const auto& QuestPair : ActiveQuests)
    {
        Result.Add(QuestPair.Value);
    }
    return Result;
}

TArray<FQuestData> UQuestSystemManager::GetCompletedQuests() const
{
    TArray<FQuestData> Result;
    for (const auto& QuestPair : CompletedQuests)
    {
        Result.Add(QuestPair.Value);
    }
    return Result;
}

FQuestData UQuestSystemManager::GetQuestData(const FString& QuestID) const
{
    if (ActiveQuests.Contains(QuestID))
    {
        return ActiveQuests[QuestID];
    }
    if (CompletedQuests.Contains(QuestID))
    {
        return CompletedQuests[QuestID];
    }
    
    // Return empty quest data if not found
    return FQuestData();
}

bool UQuestSystemManager::UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& QuestData = ActiveQuests[QuestID];
    
    for (FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Min(Progress, Objective.RequiredQuantity);
            
            if (Objective.CurrentProgress >= Objective.RequiredQuantity && !Objective.bIsCompleted)
            {
                CompleteObjective(QuestID, ObjectiveID);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UQuestSystemManager::CompleteObjective(const FString& QuestID, const FString& ObjectiveID)
{
    if (!ActiveQuests.Contains(QuestID))
    {
        return false;
    }

    FQuestData& QuestData = ActiveQuests[QuestID];
    
    for (FQuestObjective& Objective : QuestData.Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID && !Objective.bIsCompleted)
        {
            Objective.bIsCompleted = true;
            Objective.CurrentProgress = Objective.RequiredQuantity;
            
            OnObjectiveCompleted.Broadcast(QuestID, ObjectiveID);
            
            UE_LOG(LogTemp, Log, TEXT("Completed objective %s in quest %s"), *ObjectiveID, *QuestID);
            
            // Check if all objectives are complete
            bool bAllComplete = true;
            for (const FQuestObjective& CheckObjective : QuestData.Objectives)
            {
                if (!CheckObjective.bIsOptional && !CheckObjective.bIsCompleted)
                {
                    bAllComplete = false;
                    break;
                }
            }
            
            if (bAllComplete)
            {
                CompleteQuest(QuestID);
            }
            
            return true;
        }
    }
    
    return false;
}

bool UQuestSystemManager::IsObjectiveCompleted(const FString& QuestID, const FString& ObjectiveID) const
{
    if (const FQuestData* QuestData = ActiveQuests.Find(QuestID))
    {
        for (const FQuestObjective& Objective : QuestData->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                return Objective.bIsCompleted;
            }
        }
    }
    
    return false;
}

TArray<FQuestData> UQuestSystemManager::GetAvailableQuests() const
{
    TArray<FQuestData> AvailableQuests;
    
    if (QuestDataTable)
    {
        TArray<FQuestData*> AllQuests;
        QuestDataTable->GetAllRows<FQuestData>(TEXT(""), AllQuests);
        
        for (FQuestData* Quest : AllQuests)
        {
            if (Quest && CanStartQuest(Quest->QuestID))
            {
                AvailableQuests.Add(*Quest);
            }
        }
    }
    
    return AvailableQuests;
}

bool UQuestSystemManager::CanStartQuest(const FString& QuestID) const
{
    // Check if already active or completed
    if (IsQuestActive(QuestID) || IsQuestCompleted(QuestID))
    {
        return false;
    }
    
    // Check prerequisites
    return CheckPrerequisites(QuestID);
}

void UQuestSystemManager::CheckLocationTriggers(const FVector& PlayerLocation)
{
    for (const auto& TriggerPair : QuestLocationTriggers)
    {
        const FString& QuestID = TriggerPair.Key;
        const FVector& TriggerLocation = TriggerPair.Value;
        
        if (QuestTriggerRadii.Contains(QuestID))
        {
            float TriggerRadius = QuestTriggerRadii[QuestID];
            float Distance = FVector::Dist(PlayerLocation, TriggerLocation);
            
            if (Distance <= TriggerRadius && CanStartQuest(QuestID))
            {
                StartQuest(QuestID);
                UE_LOG(LogTemp, Log, TEXT("Location triggered quest: %s"), *QuestID);
            }
        }
    }
}

void UQuestSystemManager::RegisterQuestLocation(const FString& QuestID, const FVector& Location, float TriggerRadius)
{
    QuestLocationTriggers.Add(QuestID, Location);
    QuestTriggerRadii.Add(QuestID, TriggerRadius);
    
    UE_LOG(LogTemp, Log, TEXT("Registered location trigger for quest %s at %s"), *QuestID, *Location.ToString());
}

void UQuestSystemManager::UpdateSpiritualProgress(const FString& QuestID, float SpiritualGain)
{
    TotalSpiritualEnergy += SpiritualGain;
    
    UE_LOG(LogTemp, Log, TEXT("Gained %.2f spiritual energy from quest %s. Total: %.2f"), 
           SpiritualGain, *QuestID, TotalSpiritualEnergy);
}

float UQuestSystemManager::GetTotalSpiritualEnergy() const
{
    return TotalSpiritualEnergy;
}

void UQuestSystemManager::LoadQuestData()
{
    // Quest data table will be set in Blueprint or loaded from asset
    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest Data Table not set - quests will need to be loaded manually"));
    }
}

bool UQuestSystemManager::CheckPrerequisites(const FString& QuestID) const
{
    if (QuestDataTable)
    {
        FQuestData* QuestData = QuestDataTable->FindRow<FQuestData>(FName(*QuestID), TEXT(""));
        if (QuestData)
        {
            for (const FString& PrereqID : QuestData->Prerequisites)
            {
                if (!IsQuestCompleted(PrereqID))
                {
                    return false;
                }
            }
        }
    }
    
    return true;
}

void UQuestSystemManager::GiveQuestRewards(const FQuestData& QuestData)
{
    // Give experience points
    if (QuestData.Rewards.ExperiencePoints > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Gained %d experience points"), QuestData.Rewards.ExperiencePoints);
    }
    
    // Give spiritual energy
    if (QuestData.Rewards.SpiritualEnergy > 0)
    {
        UpdateSpiritualProgress(QuestData.QuestID, QuestData.Rewards.SpiritualEnergy);
    }
    
    // Give item rewards
    for (const FString& ItemID : QuestData.Rewards.ItemRewards)
    {
        UE_LOG(LogTemp, Log, TEXT("Received item: %s"), *ItemID);
        // TODO: Add to player inventory
    }
    
    // Special rewards
    if (!QuestData.Rewards.SpecialRewardDescription.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("Special reward: %s"), *QuestData.Rewards.SpecialRewardDescription.ToString());
    }
}

void UQuestSystemManager::UpdateQuestStatus(const FString& QuestID, EQuestStatus NewStatus)
{
    OnQuestStatusChanged.Broadcast(QuestID, NewStatus);
}
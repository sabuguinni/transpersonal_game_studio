#include "Quest_QuestObjective.h"
#include "Engine/Engine.h"

UQuest_QuestObjective::UQuest_QuestObjective()
{
    ObjectiveData = FQuest_ObjectiveData();
}

void UQuest_QuestObjective::UpdateProgress(int32 Amount)
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active)
    {
        return;
    }

    ObjectiveData.CurrentAmount = FMath::Clamp(
        ObjectiveData.CurrentAmount + Amount,
        0,
        ObjectiveData.RequiredAmount
    );

    CheckCompletion();

    // Log progress update
    UE_LOG(LogTemp, Log, TEXT("Quest Objective %s: Progress %d/%d"), 
        *ObjectiveData.ObjectiveID, 
        ObjectiveData.CurrentAmount, 
        ObjectiveData.RequiredAmount);
}

void UQuest_QuestObjective::SetStatus(EQuest_ObjectiveStatus NewStatus)
{
    ObjectiveData.Status = NewStatus;
    
    if (NewStatus == EQuest_ObjectiveStatus::Active)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest Objective %s activated: %s"), 
            *ObjectiveData.ObjectiveID, *ObjectiveData.Description);
    }
    else if (NewStatus == EQuest_ObjectiveStatus::Completed)
    {
        UE_LOG(LogTemp, Log, TEXT("Quest Objective %s completed!"), 
            *ObjectiveData.ObjectiveID);
    }
}

bool UQuest_QuestObjective::IsCompleted() const
{
    return ObjectiveData.Status == EQuest_ObjectiveStatus::Completed;
}

bool UQuest_QuestObjective::IsFailed() const
{
    return ObjectiveData.Status == EQuest_ObjectiveStatus::Failed;
}

float UQuest_QuestObjective::GetCompletionPercentage() const
{
    if (ObjectiveData.RequiredAmount <= 0)
    {
        return 0.0f;
    }

    return (float)ObjectiveData.CurrentAmount / (float)ObjectiveData.RequiredAmount;
}

bool UQuest_QuestObjective::CheckLocationObjective(const FVector& PlayerLocation)
{
    if (ObjectiveData.ObjectiveType != EQuest_ObjectiveType::ReachLocation ||
        ObjectiveData.Status != EQuest_ObjectiveStatus::Active)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerLocation, ObjectiveData.TargetLocation);
    
    if (Distance <= ObjectiveData.TargetRadius)
    {
        UpdateProgress(1);
        return true;
    }

    return false;
}

void UQuest_QuestObjective::ResetObjective()
{
    ObjectiveData.CurrentAmount = 0;
    ObjectiveData.Status = EQuest_ObjectiveStatus::Inactive;
    
    UE_LOG(LogTemp, Log, TEXT("Quest Objective %s reset"), *ObjectiveData.ObjectiveID);
}

void UQuest_QuestObjective::CheckCompletion()
{
    if (ObjectiveData.Status != EQuest_ObjectiveStatus::Active)
    {
        return;
    }

    if (ObjectiveData.CurrentAmount >= ObjectiveData.RequiredAmount)
    {
        SetStatus(EQuest_ObjectiveStatus::Completed);
    }
}
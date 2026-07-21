#include "Quest_QuestObjective.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UQuest_QuestObjective::UQuest_QuestObjective()
{
    ObjectiveData = FQuest_ObjectiveData();
}

bool UQuest_QuestObjective::UpdateProgress(int32 Amount)
{
    if (ObjectiveData.bCompleted)
    {
        return false;
    }

    ObjectiveData.CurrentCount = FMath::Clamp(ObjectiveData.CurrentCount + Amount, 0, ObjectiveData.RequiredCount);
    
    if (ObjectiveData.CurrentCount >= ObjectiveData.RequiredCount)
    {
        ObjectiveData.bCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Completed: %s"), *ObjectiveData.Description);
        return true;
    }

    UE_LOG(LogTemp, Log, TEXT("Quest Objective Progress: %s (%d/%d)"), 
           *ObjectiveData.Description, ObjectiveData.CurrentCount, ObjectiveData.RequiredCount);
    return false;
}

bool UQuest_QuestObjective::CheckLocationProgress(const FVector& PlayerLocation)
{
    if (ObjectiveData.Type != EQuest_ObjectiveType::ReachLocation || ObjectiveData.bCompleted)
    {
        return false;
    }

    float Distance = FVector::Dist(PlayerLocation, ObjectiveData.TargetLocation);
    if (Distance <= ObjectiveData.TargetRadius)
    {
        ObjectiveData.bCompleted = true;
        ObjectiveData.CurrentCount = ObjectiveData.RequiredCount;
        UE_LOG(LogTemp, Log, TEXT("Location Objective Completed: %s"), *ObjectiveData.Description);
        return true;
    }

    return false;
}

bool UQuest_QuestObjective::IsCompleted() const
{
    return ObjectiveData.bCompleted;
}

float UQuest_QuestObjective::GetCompletionPercentage() const
{
    if (ObjectiveData.RequiredCount <= 0)
    {
        return 0.0f;
    }

    return FMath::Clamp(static_cast<float>(ObjectiveData.CurrentCount) / static_cast<float>(ObjectiveData.RequiredCount), 0.0f, 1.0f);
}

void UQuest_QuestObjective::ResetObjective()
{
    ObjectiveData.CurrentCount = 0;
    ObjectiveData.bCompleted = false;
    UE_LOG(LogTemp, Log, TEXT("Quest Objective Reset: %s"), *ObjectiveData.Description);
}
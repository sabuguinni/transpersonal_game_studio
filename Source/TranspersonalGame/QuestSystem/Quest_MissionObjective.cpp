#include "Quest_MissionObjective.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

UQuest_MissionObjective::UQuest_MissionObjective()
{
    // Initialize default objective data
    ObjectiveData = FQuest_ObjectiveData();
}

void UQuest_MissionObjective::ActivateObjective()
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Inactive)
    {
        ObjectiveData.Status = EQuest_ObjectiveStatus::Active;
        ObjectiveData.ElapsedTime = 0.0f;
        ObjectiveData.CurrentCount = 0;
        
        OnObjectiveStatusChanged();
        
        UE_LOG(LogTemp, Log, TEXT("Mission Objective Activated: %s"), *ObjectiveData.Title);
    }
}

void UQuest_MissionObjective::CompleteObjective()
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
    {
        ObjectiveData.Status = EQuest_ObjectiveStatus::Completed;
        ObjectiveData.CurrentCount = ObjectiveData.TargetCount;
        
        OnObjectiveStatusChanged();
        
        UE_LOG(LogTemp, Log, TEXT("Mission Objective Completed: %s"), *ObjectiveData.Title);
    }
}

void UQuest_MissionObjective::FailObjective()
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
    {
        ObjectiveData.Status = EQuest_ObjectiveStatus::Failed;
        
        OnObjectiveStatusChanged();
        
        UE_LOG(LogTemp, Warning, TEXT("Mission Objective Failed: %s"), *ObjectiveData.Title);
    }
}

void UQuest_MissionObjective::UpdateProgress(int32 IncrementAmount)
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Active)
    {
        ObjectiveData.CurrentCount = FMath::Clamp(
            ObjectiveData.CurrentCount + IncrementAmount,
            0,
            ObjectiveData.TargetCount
        );
        
        // Auto-complete if target reached
        if (ObjectiveData.CurrentCount >= ObjectiveData.TargetCount)
        {
            CompleteObjective();
        }
        
        UE_LOG(LogTemp, Log, TEXT("Objective Progress: %s (%d/%d)"), 
            *ObjectiveData.Title, ObjectiveData.CurrentCount, ObjectiveData.TargetCount);
    }
}

bool UQuest_MissionObjective::IsCompleted() const
{
    return ObjectiveData.Status == EQuest_ObjectiveStatus::Completed;
}

bool UQuest_MissionObjective::IsActive() const
{
    return ObjectiveData.Status == EQuest_ObjectiveStatus::Active;
}

float UQuest_MissionObjective::GetProgressPercentage() const
{
    if (ObjectiveData.TargetCount <= 0)
    {
        return 0.0f;
    }
    
    return (float)ObjectiveData.CurrentCount / (float)ObjectiveData.TargetCount * 100.0f;
}

void UQuest_MissionObjective::UpdateTimer(float DeltaTime)
{
    if (ObjectiveData.Status == EQuest_ObjectiveStatus::Active && ObjectiveData.TimeLimit > 0.0f)
    {
        ObjectiveData.ElapsedTime += DeltaTime;
        
        // Check if time limit exceeded
        if (IsTimeLimitExceeded())
        {
            FailObjective();
        }
    }
}

bool UQuest_MissionObjective::IsTimeLimitExceeded() const
{
    return ObjectiveData.TimeLimit > 0.0f && ObjectiveData.ElapsedTime >= ObjectiveData.TimeLimit;
}

void UQuest_MissionObjective::SetTargetLocation(const FVector& NewLocation, float Radius)
{
    ObjectiveData.TargetLocation = NewLocation;
    ObjectiveData.TargetRadius = Radius;
    
    UE_LOG(LogTemp, Log, TEXT("Objective target location set: %s"), *NewLocation.ToString());
}

bool UQuest_MissionObjective::IsPlayerInTargetArea(const FVector& PlayerLocation) const
{
    if (ObjectiveData.TargetLocation.IsZero())
    {
        return false;
    }
    
    float Distance = FVector::Dist(PlayerLocation, ObjectiveData.TargetLocation);
    return Distance <= ObjectiveData.TargetRadius;
}

void UQuest_MissionObjective::ResetObjective()
{
    ObjectiveData.Status = EQuest_ObjectiveStatus::Inactive;
    ObjectiveData.CurrentCount = 0;
    ObjectiveData.ElapsedTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Mission Objective Reset: %s"), *ObjectiveData.Title);
}

void UQuest_MissionObjective::OnObjectiveStatusChanged()
{
    // Broadcast status change to quest system
    // This can be expanded with delegate broadcasts for UI updates
    
    switch (ObjectiveData.Status)
    {
        case EQuest_ObjectiveStatus::Active:
            UE_LOG(LogTemp, Log, TEXT("Objective Status: ACTIVE - %s"), *ObjectiveData.Title);
            break;
        case EQuest_ObjectiveStatus::Completed:
            UE_LOG(LogTemp, Log, TEXT("Objective Status: COMPLETED - %s"), *ObjectiveData.Title);
            break;
        case EQuest_ObjectiveStatus::Failed:
            UE_LOG(LogTemp, Warning, TEXT("Objective Status: FAILED - %s"), *ObjectiveData.Title);
            break;
        default:
            break;
    }
}
#include "Quest_MissionObjective.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuest_MissionObjective::UQuest_MissionObjective()
{
    CurrentStatus = EQuest_ObjectiveStatus::Inactive;
    CurrentProgress = 0;
    StartTime = 0.0f;
    bIsOptional = false;
}

void UQuest_MissionObjective::ActivateObjective()
{
    if (CurrentStatus == EQuest_ObjectiveStatus::Inactive)
    {
        CurrentStatus = EQuest_ObjectiveStatus::Active;
        CurrentProgress = 0;
        
        if (UWorld* World = GetWorld())
        {
            StartTime = World->GetTimeSeconds();
        }
        
        OnObjectiveStatusChanged.Broadcast(this);
        
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Activated: %s"), *ObjectiveData.ObjectiveTitle.ToString());
    }
}

void UQuest_MissionObjective::UpdateProgress(int32 ProgressAmount)
{
    if (CurrentStatus != EQuest_ObjectiveStatus::Active)
    {
        return;
    }
    
    CurrentProgress = FMath::Clamp(CurrentProgress + ProgressAmount, 0, ObjectiveData.RequiredCount);
    
    float ProgressPercentage = GetProgressPercentage();
    OnObjectiveProgressUpdated.Broadcast(this, ProgressPercentage);
    
    // Auto-complete if progress reaches requirement
    if (CurrentProgress >= ObjectiveData.RequiredCount)
    {
        CompleteObjective();
    }
    
    // Check for timeout
    if (ObjectiveData.TimeLimit > 0.0f && IsTimedOut())
    {
        FailObjective();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest Objective Progress: %s - %d/%d (%.1f%%)"), 
           *ObjectiveData.ObjectiveTitle.ToString(), 
           CurrentProgress, 
           ObjectiveData.RequiredCount, 
           ProgressPercentage);
}

void UQuest_MissionObjective::CompleteObjective()
{
    if (CurrentStatus == EQuest_ObjectiveStatus::Active)
    {
        CurrentStatus = EQuest_ObjectiveStatus::Completed;
        CurrentProgress = ObjectiveData.RequiredCount;
        
        OnObjectiveStatusChanged.Broadcast(this);
        
        UE_LOG(LogTemp, Log, TEXT("Quest Objective Completed: %s"), *ObjectiveData.ObjectiveTitle.ToString());
    }
}

void UQuest_MissionObjective::FailObjective()
{
    if (CurrentStatus == EQuest_ObjectiveStatus::Active)
    {
        CurrentStatus = EQuest_ObjectiveStatus::Failed;
        
        OnObjectiveStatusChanged.Broadcast(this);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest Objective Failed: %s"), *ObjectiveData.ObjectiveTitle.ToString());
    }
}

bool UQuest_MissionObjective::IsCompleted() const
{
    return CurrentStatus == EQuest_ObjectiveStatus::Completed;
}

bool UQuest_MissionObjective::IsFailed() const
{
    return CurrentStatus == EQuest_ObjectiveStatus::Failed;
}

bool UQuest_MissionObjective::IsActive() const
{
    return CurrentStatus == EQuest_ObjectiveStatus::Active;
}

float UQuest_MissionObjective::GetProgressPercentage() const
{
    if (ObjectiveData.RequiredCount <= 0)
    {
        return 0.0f;
    }
    
    return (float)CurrentProgress / (float)ObjectiveData.RequiredCount * 100.0f;
}

float UQuest_MissionObjective::GetRemainingTime() const
{
    if (ObjectiveData.TimeLimit <= 0.0f || CurrentStatus != EQuest_ObjectiveStatus::Active)
    {
        return 0.0f;
    }
    
    if (UWorld* World = GetWorld())
    {
        float ElapsedTime = World->GetTimeSeconds() - StartTime;
        return FMath::Max(0.0f, ObjectiveData.TimeLimit - ElapsedTime);
    }
    
    return ObjectiveData.TimeLimit;
}

bool UQuest_MissionObjective::IsTimedOut() const
{
    if (ObjectiveData.TimeLimit <= 0.0f || CurrentStatus != EQuest_ObjectiveStatus::Active)
    {
        return false;
    }
    
    return GetRemainingTime() <= 0.0f;
}
#include "Quest_MissionProgressTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UQuest_MissionProgressTracker::UQuest_MissionProgressTracker()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f;
    
    ProgressUpdateInterval = 1.0f;
    bAutoCompleteOnTargetReached = true;
    bLogProgressUpdates = true;
    LastUpdateTime = 0.0f;
}

void UQuest_MissionProgressTracker::BeginPlay()
{
    Super::BeginPlay();
    
    LogMissionEvent(TEXT("Mission Progress Tracker initialized"));
    
    // Initialize any default missions here if needed
    if (ActiveMissions.Num() == 0)
    {
        LogMissionEvent(TEXT("No active missions found - ready for new mission assignments"));
    }
}

void UQuest_MissionProgressTracker::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= ProgressUpdateInterval)
    {
        // Check for mission completion conditions
        for (auto& Mission : ActiveMissions)
        {
            if (!Mission.bIsCompleted && !Mission.bIsFailed)
            {
                UpdateMissionFromObjectives(Mission.MissionID);
            }
        }
        
        LastUpdateTime = 0.0f;
    }
}

void UQuest_MissionProgressTracker::StartMission(const FString& MissionID, const FString& MissionName, float TargetProgress)
{
    // Check if mission already exists
    if (FindMissionProgress(MissionID) != nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s already exists"), *MissionID));
        return;
    }
    
    FQuest_MissionProgress NewMission;
    NewMission.MissionID = MissionID;
    NewMission.MissionName = MissionName;
    NewMission.TargetProgress = TargetProgress;
    NewMission.StartTime = FDateTime::Now();
    
    ActiveMissions.Add(NewMission);
    
    LogMissionEvent(FString::Printf(TEXT("Started mission: %s (%s)"), *MissionName, *MissionID));
}

void UQuest_MissionProgressTracker::UpdateMissionProgress(const FString& MissionID, float ProgressDelta)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s not found for progress update"), *MissionID));
        return;
    }
    
    if (Mission->bIsCompleted || Mission->bIsFailed)
    {
        return;
    }
    
    float OldProgress = Mission->CurrentProgress;
    Mission->CurrentProgress = FMath::Clamp(Mission->CurrentProgress + ProgressDelta, 0.0f, Mission->TargetProgress);
    
    if (bLogProgressUpdates)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s progress: %.1f -> %.1f (%.1f%%)"), 
            *MissionID, OldProgress, Mission->CurrentProgress, 
            (Mission->CurrentProgress / Mission->TargetProgress) * 100.0f));
    }
    
    OnMissionProgressUpdated.Broadcast(MissionID, Mission->CurrentProgress);
    
    if (bAutoCompleteOnTargetReached && Mission->CurrentProgress >= Mission->TargetProgress)
    {
        CompleteMission(MissionID);
    }
}

void UQuest_MissionProgressTracker::CompleteMission(const FString& MissionID)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s not found for completion"), *MissionID));
        return;
    }
    
    if (Mission->bIsCompleted)
    {
        return;
    }
    
    Mission->bIsCompleted = true;
    Mission->CompletionTime = FDateTime::Now();
    Mission->CurrentProgress = Mission->TargetProgress;
    
    // Move to completed missions
    CompletedMissions.Add(*Mission);
    ActiveMissions.RemoveAll([MissionID](const FQuest_MissionProgress& M) { return M.MissionID == MissionID; });
    
    LogMissionEvent(FString::Printf(TEXT("Mission completed: %s (%s)"), *Mission->MissionName, *MissionID));
    OnMissionCompleted.Broadcast(MissionID);
}

void UQuest_MissionProgressTracker::FailMission(const FString& MissionID)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s not found for failure"), *MissionID));
        return;
    }
    
    if (Mission->bIsFailed || Mission->bIsCompleted)
    {
        return;
    }
    
    Mission->bIsFailed = true;
    Mission->CompletionTime = FDateTime::Now();
    
    LogMissionEvent(FString::Printf(TEXT("Mission failed: %s (%s)"), *Mission->MissionName, *MissionID));
    OnMissionFailed.Broadcast(MissionID);
}

bool UQuest_MissionProgressTracker::IsMissionActive(const FString& MissionID) const
{
    const FQuest_MissionProgress* Mission = ActiveMissions.FindByPredicate([MissionID](const FQuest_MissionProgress& M) {
        return M.MissionID == MissionID;
    });
    
    return Mission != nullptr && !Mission->bIsCompleted && !Mission->bIsFailed;
}

float UQuest_MissionProgressTracker::GetMissionProgress(const FString& MissionID) const
{
    const FQuest_MissionProgress* Mission = ActiveMissions.FindByPredicate([MissionID](const FQuest_MissionProgress& M) {
        return M.MissionID == MissionID;
    });
    
    if (Mission != nullptr)
    {
        return Mission->TargetProgress > 0.0f ? (Mission->CurrentProgress / Mission->TargetProgress) * 100.0f : 0.0f;
    }
    
    return 0.0f;
}

void UQuest_MissionProgressTracker::AddObjective(const FString& MissionID, const FString& ObjectiveID, const FString& Description, float TargetValue, bool bIsOptional)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Mission %s not found for objective addition"), *MissionID));
        return;
    }
    
    // Check if objective already exists
    if (FindObjectiveProgress(ObjectiveID) != nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Objective %s already exists"), *ObjectiveID));
        return;
    }
    
    FQuest_ObjectiveProgress NewObjective;
    NewObjective.ObjectiveID = ObjectiveID;
    NewObjective.Description = Description;
    NewObjective.TargetValue = TargetValue;
    NewObjective.bIsOptional = bIsOptional;
    
    AllObjectives.Add(NewObjective);
    Mission->ObjectiveIDs.Add(ObjectiveID);
    
    LogMissionEvent(FString::Printf(TEXT("Added objective to mission %s: %s"), *MissionID, *Description));
}

void UQuest_MissionProgressTracker::UpdateObjectiveProgress(const FString& ObjectiveID, float ProgressDelta)
{
    FQuest_ObjectiveProgress* Objective = FindObjectiveProgress(ObjectiveID);
    if (Objective == nullptr)
    {
        LogMissionEvent(FString::Printf(TEXT("Objective %s not found for progress update"), *ObjectiveID));
        return;
    }
    
    if (Objective->bIsCompleted)
    {
        return;
    }
    
    float OldValue = Objective->CurrentValue;
    Objective->CurrentValue = FMath::Clamp(Objective->CurrentValue + ProgressDelta, 0.0f, Objective->TargetValue);
    
    if (bLogProgressUpdates)
    {
        LogMissionEvent(FString::Printf(TEXT("Objective %s progress: %.1f -> %.1f"), 
            *ObjectiveID, OldValue, Objective->CurrentValue));
    }
    
    if (Objective->CurrentValue >= Objective->TargetValue)
    {
        CompleteObjective(ObjectiveID);
    }
}

void UQuest_MissionProgressTracker::CompleteObjective(const FString& ObjectiveID)
{
    FQuest_ObjectiveProgress* Objective = FindObjectiveProgress(ObjectiveID);
    if (Objective == nullptr || Objective->bIsCompleted)
    {
        return;
    }
    
    Objective->bIsCompleted = true;
    Objective->CurrentValue = Objective->TargetValue;
    
    LogMissionEvent(FString::Printf(TEXT("Objective completed: %s"), *Objective->Description));
}

bool UQuest_MissionProgressTracker::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    const FQuest_ObjectiveProgress* Objective = AllObjectives.FindByPredicate([ObjectiveID](const FQuest_ObjectiveProgress& O) {
        return O.ObjectiveID == ObjectiveID;
    });
    
    return Objective != nullptr && Objective->bIsCompleted;
}

TArray<FQuest_MissionProgress> UQuest_MissionProgressTracker::GetActiveMissions() const
{
    return ActiveMissions;
}

TArray<FQuest_ObjectiveProgress> UQuest_MissionProgressTracker::GetMissionObjectives(const FString& MissionID) const
{
    TArray<FQuest_ObjectiveProgress> MissionObjectives;
    
    const FQuest_MissionProgress* Mission = ActiveMissions.FindByPredicate([MissionID](const FQuest_MissionProgress& M) {
        return M.MissionID == MissionID;
    });
    
    if (Mission != nullptr)
    {
        for (const FString& ObjectiveID : Mission->ObjectiveIDs)
        {
            const FQuest_ObjectiveProgress* Objective = AllObjectives.FindByPredicate([ObjectiveID](const FQuest_ObjectiveProgress& O) {
                return O.ObjectiveID == ObjectiveID;
            });
            
            if (Objective != nullptr)
            {
                MissionObjectives.Add(*Objective);
            }
        }
    }
    
    return MissionObjectives;
}

int32 UQuest_MissionProgressTracker::GetCompletedMissionCount() const
{
    return CompletedMissions.Num();
}

float UQuest_MissionProgressTracker::GetOverallProgressPercentage() const
{
    if (ActiveMissions.Num() == 0 && CompletedMissions.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalProgress = 0.0f;
    int32 TotalMissions = ActiveMissions.Num() + CompletedMissions.Num();
    
    for (const auto& Mission : ActiveMissions)
    {
        TotalProgress += Mission.TargetProgress > 0.0f ? (Mission.CurrentProgress / Mission.TargetProgress) : 0.0f;
    }
    
    for (const auto& Mission : CompletedMissions)
    {
        TotalProgress += 1.0f; // Completed missions count as 100%
    }
    
    return TotalMissions > 0 ? (TotalProgress / TotalMissions) * 100.0f : 0.0f;
}

void UQuest_MissionProgressTracker::CheckMissionCompletion(const FString& MissionID)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr || Mission->bIsCompleted || Mission->bIsFailed)
    {
        return;
    }
    
    // Check if all required objectives are completed
    bool bAllRequiredObjectivesCompleted = true;
    for (const FString& ObjectiveID : Mission->ObjectiveIDs)
    {
        const FQuest_ObjectiveProgress* Objective = FindObjectiveProgress(ObjectiveID);
        if (Objective != nullptr && !Objective->bIsOptional && !Objective->bIsCompleted)
        {
            bAllRequiredObjectivesCompleted = false;
            break;
        }
    }
    
    if (bAllRequiredObjectivesCompleted)
    {
        CompleteMission(MissionID);
    }
}

void UQuest_MissionProgressTracker::UpdateMissionFromObjectives(const FString& MissionID)
{
    FQuest_MissionProgress* Mission = FindMissionProgress(MissionID);
    if (Mission == nullptr)
    {
        return;
    }
    
    // Calculate progress based on objectives
    if (Mission->ObjectiveIDs.Num() > 0)
    {
        float ObjectiveProgress = 0.0f;
        int32 ValidObjectives = 0;
        
        for (const FString& ObjectiveID : Mission->ObjectiveIDs)
        {
            const FQuest_ObjectiveProgress* Objective = FindObjectiveProgress(ObjectiveID);
            if (Objective != nullptr)
            {
                float ObjectiveCompletion = Objective->TargetValue > 0.0f ? 
                    (Objective->CurrentValue / Objective->TargetValue) : 0.0f;
                ObjectiveProgress += ObjectiveCompletion;
                ValidObjectives++;
            }
        }
        
        if (ValidObjectives > 0)
        {
            float NewProgress = (ObjectiveProgress / ValidObjectives) * Mission->TargetProgress;
            if (FMath::Abs(NewProgress - Mission->CurrentProgress) > 0.1f)
            {
                Mission->CurrentProgress = NewProgress;
                OnMissionProgressUpdated.Broadcast(MissionID, Mission->CurrentProgress);
            }
        }
    }
    
    CheckMissionCompletion(MissionID);
}

FQuest_MissionProgress* UQuest_MissionProgressTracker::FindMissionProgress(const FString& MissionID)
{
    return ActiveMissions.FindByPredicate([MissionID](const FQuest_MissionProgress& M) {
        return M.MissionID == MissionID;
    });
}

FQuest_ObjectiveProgress* UQuest_MissionProgressTracker::FindObjectiveProgress(const FString& ObjectiveID)
{
    return AllObjectives.FindByPredicate([ObjectiveID](const FQuest_ObjectiveProgress& O) {
        return O.ObjectiveID == ObjectiveID;
    });
}

void UQuest_MissionProgressTracker::LogMissionEvent(const FString& Message)
{
    if (bLogProgressUpdates && GEngine)
    {
        UE_LOG(LogTemp, Log, TEXT("[MissionProgressTracker] %s"), *Message);
        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
            FString::Printf(TEXT("[Quest] %s"), *Message));
    }
}
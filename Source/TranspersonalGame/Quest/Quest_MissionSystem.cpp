#include "Quest_MissionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Quest_ResourceGatheringManager.h"
#include "Crowd/Crowd_MassSimulationManager.h"

UQuest_MissionSystem::UQuest_MissionSystem()
{
    MissionCounter = 0;
    ResourceManager = nullptr;
    CrowdManager = nullptr;
}

void UQuest_MissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Initializing mission system"));
    
    // Get references to other subsystems
    ResourceManager = GetGameInstance()->GetSubsystem<UQuest_ResourceGatheringManager>();
    CrowdManager = GetGameInstance()->GetSubsystem<UCrowd_MassSimulationManager>();
    
    // Clear mission data
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    MissionTimers.Empty();
    MissionCounter = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Mission system initialized successfully"));
}

void UQuest_MissionSystem::Deinitialize()
{
    // Clear all timers
    if (UWorld* World = GetWorld())
    {
        for (auto& TimerPair : MissionTimers)
        {
            World->GetTimerManager().ClearTimer(TimerPair.Value);
        }
    }
    
    MissionTimers.Empty();
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Mission system deinitialized"));
    
    Super::Deinitialize();
}

FString UQuest_MissionSystem::CreateMission(const FQuest_MissionData& MissionData)
{
    if (!ValidateMissionObjectives(MissionData))
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionSystem: Invalid mission objectives"));
        return TEXT("");
    }
    
    FQuest_MissionData NewMission = MissionData;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.MissionStatus = EQuest_MissionStatus::Inactive;
    NewMission.TimeRemaining = NewMission.TimeLimit;
    
    // Initialize objective progress
    for (FQuest_MissionObjective& Objective : NewMission.Objectives)
    {
        Objective.CurrentProgress = 0;
        Objective.bIsCompleted = false;
    }
    
    ActiveMissions.Add(NewMission.MissionID, NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Created mission %s: %s"), 
           *NewMission.MissionID, *NewMission.MissionName);
    
    return NewMission.MissionID;
}

bool UQuest_MissionSystem::StartMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = ActiveMissions.Find(MissionID);
    if (!Mission)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionSystem: Mission %s not found"), *MissionID);
        return false;
    }
    
    if (Mission->MissionStatus != EQuest_MissionStatus::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Mission %s already started"), *MissionID);
        return false;
    }
    
    Mission->MissionStatus = EQuest_MissionStatus::Active;
    Mission->TimeRemaining = Mission->TimeLimit;
    
    // Start mission timer if time limit is set
    if (Mission->TimeLimit > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, 
                FTimerDelegate::CreateUObject(this, &UQuest_MissionSystem::OnMissionTimeExpired, MissionID),
                Mission->TimeLimit, false);
            MissionTimers.Add(MissionID, TimerHandle);
        }
    }
    
    // Notify crowd system if mission requires crowd interaction
    if (Mission->bRequiresCrowdInteraction && CrowdManager)
    {
        NotifyCrowdOfMissionProgress(MissionID, EQuest_MissionStatus::Active);
    }
    
    BroadcastMissionStatusChange(MissionID, EQuest_MissionStatus::Active);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Started mission %s"), *MissionID);
    return true;
}

bool UQuest_MissionSystem::CompleteMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = ActiveMissions.Find(MissionID);
    if (!Mission)
    {
        return false;
    }
    
    Mission->MissionStatus = EQuest_MissionStatus::Completed;
    
    // Clear mission timer
    if (FTimerHandle* TimerHandle = MissionTimers.Find(MissionID))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(*TimerHandle);
        }
        MissionTimers.Remove(MissionID);
    }
    
    // Move to completed missions
    CompletedMissions.Add(MissionID, *Mission);
    
    // Notify crowd system
    if (Mission->bRequiresCrowdInteraction && CrowdManager)
    {
        NotifyCrowdOfMissionProgress(MissionID, EQuest_MissionStatus::Completed);
    }
    
    BroadcastMissionStatusChange(MissionID, EQuest_MissionStatus::Completed);
    OnMissionCompleted.Broadcast(MissionID);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Completed mission %s"), *MissionID);
    return true;
}

bool UQuest_MissionSystem::FailMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = ActiveMissions.Find(MissionID);
    if (!Mission)
    {
        return false;
    }
    
    Mission->MissionStatus = EQuest_MissionStatus::Failed;
    
    // Clear mission timer
    if (FTimerHandle* TimerHandle = MissionTimers.Find(MissionID))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(*TimerHandle);
        }
        MissionTimers.Remove(MissionID);
    }
    
    // Trigger emergency response if needed
    if (Mission->bTriggersEmergencyResponse && CrowdManager)
    {
        TriggerCrowdEmergencyResponse(Mission->MissionLocation, Mission->MissionRadius);
    }
    
    BroadcastMissionStatusChange(MissionID, EQuest_MissionStatus::Failed);
    OnMissionFailed.Broadcast(MissionID);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Failed mission %s"), *MissionID);
    return true;
}

bool UQuest_MissionSystem::AbandonMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = ActiveMissions.Find(MissionID);
    if (!Mission)
    {
        return false;
    }
    
    Mission->MissionStatus = EQuest_MissionStatus::Abandoned;
    
    // Clear mission timer
    if (FTimerHandle* TimerHandle = MissionTimers.Find(MissionID))
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(*TimerHandle);
        }
        MissionTimers.Remove(MissionID);
    }
    
    BroadcastMissionStatusChange(MissionID, EQuest_MissionStatus::Abandoned);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Abandoned mission %s"), *MissionID);
    return true;
}

bool UQuest_MissionSystem::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressDelta)
{
    FQuest_MissionData* Mission = ActiveMissions.Find(MissionID);
    if (!Mission || Mission->MissionStatus != EQuest_MissionStatus::Active)
    {
        return false;
    }
    
    for (FQuest_MissionObjective& Objective : Mission->Objectives)
    {
        if (Objective.ObjectiveID == ObjectiveID)
        {
            Objective.CurrentProgress = FMath::Clamp(Objective.CurrentProgress + ProgressDelta, 0, Objective.TargetQuantity);
            
            if (Objective.CurrentProgress >= Objective.TargetQuantity)
            {
                Objective.bIsCompleted = true;
            }
            
            OnObjectiveUpdated.Broadcast(MissionID, ObjectiveID, Objective.CurrentProgress);
            
            // Check if all objectives are completed
            bool bAllObjectivesCompleted = true;
            for (const FQuest_MissionObjective& CheckObjective : Mission->Objectives)
            {
                if (!CheckObjective.bIsCompleted)
                {
                    bAllObjectivesCompleted = false;
                    break;
                }
            }
            
            if (bAllObjectivesCompleted)
            {
                CompleteMission(MissionID);
            }
            
            return true;
        }
    }
    
    return false;
}

TArray<FQuest_MissionData> UQuest_MissionSystem::GetActiveMissions() const
{
    TArray<FQuest_MissionData> Result;
    for (const auto& MissionPair : ActiveMissions)
    {
        if (MissionPair.Value.MissionStatus == EQuest_MissionStatus::Active)
        {
            Result.Add(MissionPair.Value);
        }
    }
    return Result;
}

TArray<FQuest_MissionData> UQuest_MissionSystem::GetMissionsByType(EQuest_MissionType MissionType) const
{
    TArray<FQuest_MissionData> Result;
    for (const auto& MissionPair : ActiveMissions)
    {
        if (MissionPair.Value.MissionType == MissionType)
        {
            Result.Add(MissionPair.Value);
        }
    }
    return Result;
}

FQuest_MissionData UQuest_MissionSystem::GetMissionData(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        return *Mission;
    }
    if (const FQuest_MissionData* Mission = CompletedMissions.Find(MissionID))
    {
        return *Mission;
    }
    return FQuest_MissionData();
}

bool UQuest_MissionSystem::IsMissionActive(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        return Mission->MissionStatus == EQuest_MissionStatus::Active;
    }
    return false;
}

int32 UQuest_MissionSystem::GetActiveMissionCount() const
{
    int32 Count = 0;
    for (const auto& MissionPair : ActiveMissions)
    {
        if (MissionPair.Value.MissionStatus == EQuest_MissionStatus::Active)
        {
            Count++;
        }
    }
    return Count;
}

void UQuest_MissionSystem::TriggerCrowdEmergencyResponse(const FVector& Location, float Radius)
{
    if (CrowdManager)
    {
        // Trigger panic response in crowd simulation
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Triggering crowd emergency response at %s"), 
               *Location.ToString());
        // CrowdManager->TriggerPanicResponse(Location, Radius); // This would be called if the method exists
    }
}

void UQuest_MissionSystem::NotifyCrowdOfMissionProgress(const FString& MissionID, EQuest_MissionStatus Status)
{
    if (CrowdManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Notifying crowd of mission %s status change"), *MissionID);
        // Additional crowd notification logic would go here
    }
}

FString UQuest_MissionSystem::CreateGatheringMission(const FVector& Location, const FString& ResourceType, int32 Quantity)
{
    FQuest_MissionData MissionData;
    MissionData.MissionName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    MissionData.MissionDescription = FString::Printf(TEXT("Collect %d units of %s from the designated area"), Quantity, *ResourceType);
    MissionData.MissionType = EQuest_MissionType::Gather;
    MissionData.MissionPriority = EQuest_MissionPriority::Medium;
    MissionData.MissionLocation = Location;
    MissionData.MissionRadius = 1500.0f;
    MissionData.TimeLimit = 1800.0f; // 30 minutes
    MissionData.ExperienceReward = Quantity * 10;
    MissionData.bRequiresCrowdInteraction = true;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveID = TEXT("Gather_Primary");
    Objective.Description = FString::Printf(TEXT("Collect %d %s"), Quantity, *ResourceType);
    Objective.ObjectiveType = EQuest_MissionType::Gather;
    Objective.TargetQuantity = Quantity;
    Objective.TargetLocation = Location;
    Objective.CompletionRadius = 1500.0f;
    
    MissionData.Objectives.Add(Objective);
    
    return CreateMission(MissionData);
}

FString UQuest_MissionSystem::CreateHuntingMission(const FVector& Location, const FString& TargetSpecies, int32 TargetCount)
{
    FQuest_MissionData MissionData;
    MissionData.MissionName = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    MissionData.MissionDescription = FString::Printf(TEXT("Hunt %d %s in the designated hunting grounds"), TargetCount, *TargetSpecies);
    MissionData.MissionType = EQuest_MissionType::Hunt;
    MissionData.MissionPriority = EQuest_MissionPriority::High;
    MissionData.MissionLocation = Location;
    MissionData.MissionRadius = 2000.0f;
    MissionData.TimeLimit = 3600.0f; // 60 minutes
    MissionData.ExperienceReward = TargetCount * 50;
    MissionData.bTriggersEmergencyResponse = true;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveID = TEXT("Hunt_Primary");
    Objective.Description = FString::Printf(TEXT("Hunt %d %s"), TargetCount, *TargetSpecies);
    Objective.ObjectiveType = EQuest_MissionType::Hunt;
    Objective.TargetQuantity = TargetCount;
    Objective.TargetLocation = Location;
    Objective.CompletionRadius = 2000.0f;
    
    MissionData.Objectives.Add(Objective);
    
    return CreateMission(MissionData);
}

FString UQuest_MissionSystem::CreateExplorationMission(const FVector& TargetLocation, float ExplorationRadius)
{
    FQuest_MissionData MissionData;
    MissionData.MissionName = TEXT("Explore Unknown Territory");
    MissionData.MissionDescription = TEXT("Venture into unexplored lands and map the area");
    MissionData.MissionType = EQuest_MissionType::Explore;
    MissionData.MissionPriority = EQuest_MissionPriority::Medium;
    MissionData.MissionLocation = TargetLocation;
    MissionData.MissionRadius = ExplorationRadius;
    MissionData.TimeLimit = 2400.0f; // 40 minutes
    MissionData.ExperienceReward = 200;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveID = TEXT("Explore_Primary");
    Objective.Description = TEXT("Reach the exploration target");
    Objective.ObjectiveType = EQuest_MissionType::Explore;
    Objective.TargetQuantity = 1;
    Objective.TargetLocation = TargetLocation;
    Objective.CompletionRadius = ExplorationRadius;
    
    MissionData.Objectives.Add(Objective);
    
    return CreateMission(MissionData);
}

FString UQuest_MissionSystem::CreateEscortMission(const FVector& StartLocation, const FVector& EndLocation, const FString& NPCName)
{
    FQuest_MissionData MissionData;
    MissionData.MissionName = FString::Printf(TEXT("Escort %s"), *NPCName);
    MissionData.MissionDescription = FString::Printf(TEXT("Safely escort %s to their destination"), *NPCName);
    MissionData.MissionType = EQuest_MissionType::Escort;
    MissionData.MissionPriority = EQuest_MissionPriority::High;
    MissionData.MissionLocation = StartLocation;
    MissionData.MissionRadius = FVector::Dist(StartLocation, EndLocation);
    MissionData.TimeLimit = 1800.0f; // 30 minutes
    MissionData.ExperienceReward = 300;
    MissionData.bRequiresCrowdInteraction = true;
    MissionData.bTriggersEmergencyResponse = true;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveID = TEXT("Escort_Primary");
    Objective.Description = FString::Printf(TEXT("Escort %s safely"), *NPCName);
    Objective.ObjectiveType = EQuest_MissionType::Escort;
    Objective.TargetQuantity = 1;
    Objective.TargetLocation = EndLocation;
    Objective.CompletionRadius = 500.0f;
    
    MissionData.Objectives.Add(Objective);
    
    return CreateMission(MissionData);
}

FString UQuest_MissionSystem::CreateDefenseMission(const FVector& DefenseLocation, float DefenseRadius, float Duration)
{
    FQuest_MissionData MissionData;
    MissionData.MissionName = TEXT("Defend Settlement");
    MissionData.MissionDescription = TEXT("Protect the settlement from incoming threats");
    MissionData.MissionType = EQuest_MissionType::Defend;
    MissionData.MissionPriority = EQuest_MissionPriority::Critical;
    MissionData.MissionLocation = DefenseLocation;
    MissionData.MissionRadius = DefenseRadius;
    MissionData.TimeLimit = Duration;
    MissionData.ExperienceReward = 500;
    MissionData.bRequiresCrowdInteraction = true;
    MissionData.bTriggersEmergencyResponse = true;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveID = TEXT("Defend_Primary");
    Objective.Description = TEXT("Survive the defense period");
    Objective.ObjectiveType = EQuest_MissionType::Defend;
    Objective.TargetQuantity = 1;
    Objective.TargetLocation = DefenseLocation;
    Objective.CompletionRadius = DefenseRadius;
    
    MissionData.Objectives.Add(Objective);
    
    return CreateMission(MissionData);
}

void UQuest_MissionSystem::UpdateMissionTimers()
{
    // This would be called by a timer to update mission time remaining
    for (auto& MissionPair : ActiveMissions)
    {
        if (MissionPair.Value.MissionStatus == EQuest_MissionStatus::Active && MissionPair.Value.TimeLimit > 0.0f)
        {
            MissionPair.Value.TimeRemaining -= 1.0f; // Decrease by 1 second
            if (MissionPair.Value.TimeRemaining <= 0.0f)
            {
                OnMissionTimeExpired(MissionPair.Key);
            }
        }
    }
}

void UQuest_MissionSystem::OnMissionTimeExpired(const FString& MissionID)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionSystem: Mission %s time expired"), *MissionID);
    FailMission(MissionID);
}

bool UQuest_MissionSystem::ValidateMissionObjectives(const FQuest_MissionData& MissionData) const
{
    if (MissionData.Objectives.Num() == 0)
    {
        return false;
    }
    
    for (const FQuest_MissionObjective& Objective : MissionData.Objectives)
    {
        if (Objective.ObjectiveID.IsEmpty() || Objective.TargetQuantity <= 0)
        {
            return false;
        }
    }
    
    return true;
}

void UQuest_MissionSystem::BroadcastMissionStatusChange(const FString& MissionID, EQuest_MissionStatus NewStatus)
{
    OnMissionStatusChanged.Broadcast(MissionID, NewStatus);
}

FString UQuest_MissionSystem::GenerateUniqueMissionID()
{
    MissionCounter++;
    return FString::Printf(TEXT("MISSION_%04d"), MissionCounter);
}
#include "Quest_MissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_MissionManager::UQuest_MissionManager()
{
    MaxActiveMissions = 5;
    bIsInitialized = false;
}

void UQuest_MissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Initializing quest system"));
    
    InitializeDefaultMissions();
    SetupCrowdIntegration();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Quest system initialized with %d available missions"), AvailableMissions.Num());
}

void UQuest_MissionManager::Deinitialize()
{
    ActiveMissions.Empty();
    AvailableMissions.Empty();
    CrowdMissionRegistry.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UQuest_MissionManager::CreateMission(const FString& MissionID, const FString& Name, const FString& Description, EQuest_MissionType Type)
{
    if (MissionID.IsEmpty() || Name.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Cannot create mission with empty ID or name"));
        return false;
    }
    
    if (AvailableMissions.Contains(MissionID) || ActiveMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Mission %s already exists"), *MissionID);
        return false;
    }
    
    FQuest_MissionData NewMission;
    NewMission.MissionID = MissionID;
    NewMission.MissionName = Name;
    NewMission.MissionDescription = Description;
    NewMission.MissionType = Type;
    NewMission.MissionStatus = EQuest_MissionStatus::Available;
    
    AvailableMissions.Add(MissionID, NewMission);
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Created mission %s - %s"), *MissionID, *Name);
    
    return true;
}

bool UQuest_MissionManager::StartMission(const FString& MissionID)
{
    if (!AvailableMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Mission %s not found in available missions"), *MissionID);
        return false;
    }
    
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Cannot start mission %s - maximum active missions reached"), *MissionID);
        return false;
    }
    
    FQuest_MissionData MissionData = AvailableMissions[MissionID];
    MissionData.MissionStatus = EQuest_MissionStatus::Active;
    MissionData.ElapsedTime = 0.0f;
    
    ActiveMissions.Add(MissionID, MissionData);
    AvailableMissions.Remove(MissionID);
    
    BroadcastMissionEvent(MissionID, TEXT("MissionStarted"));
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Started mission %s"), *MissionID);
    
    return true;
}

bool UQuest_MissionManager::CompleteMission(const FString& MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Cannot complete mission %s - not active"), *MissionID);
        return false;
    }
    
    FQuest_MissionData& MissionData = ActiveMissions[MissionID];
    MissionData.MissionStatus = EQuest_MissionStatus::Completed;
    
    // Award experience
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Mission %s completed! Awarded %d experience"), *MissionID, MissionData.ExperienceReward);
    
    BroadcastMissionEvent(MissionID, TEXT("MissionCompleted"));
    ActiveMissions.Remove(MissionID);
    
    return true;
}

bool UQuest_MissionManager::FailMission(const FString& MissionID)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Cannot fail mission %s - not active"), *MissionID);
        return false;
    }
    
    FQuest_MissionData& MissionData = ActiveMissions[MissionID];
    MissionData.MissionStatus = EQuest_MissionStatus::Failed;
    
    BroadcastMissionEvent(MissionID, TEXT("MissionFailed"));
    ActiveMissions.Remove(MissionID);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Mission %s failed"), *MissionID);
    return true;
}

FQuest_MissionData UQuest_MissionManager::GetMissionData(const FString& MissionID)
{
    if (ActiveMissions.Contains(MissionID))
    {
        return ActiveMissions[MissionID];
    }
    
    if (AvailableMissions.Contains(MissionID))
    {
        return AvailableMissions[MissionID];
    }
    
    return FQuest_MissionData();
}

TArray<FQuest_MissionData> UQuest_MissionManager::GetActiveMissions()
{
    TArray<FQuest_MissionData> Missions;
    for (const auto& Mission : ActiveMissions)
    {
        Missions.Add(Mission.Value);
    }
    return Missions;
}

TArray<FQuest_MissionData> UQuest_MissionManager::GetAvailableMissions()
{
    TArray<FQuest_MissionData> Missions;
    for (const auto& Mission : AvailableMissions)
    {
        Missions.Add(Mission.Value);
    }
    return Missions;
}

bool UQuest_MissionManager::AddObjective(const FString& MissionID, const FString& ObjectiveText, int32 TargetCount)
{
    FQuest_MissionData* MissionData = nullptr;
    
    if (ActiveMissions.Contains(MissionID))
    {
        MissionData = &ActiveMissions[MissionID];
    }
    else if (AvailableMissions.Contains(MissionID))
    {
        MissionData = &AvailableMissions[MissionID];
    }
    
    if (!MissionData)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Cannot add objective to mission %s - mission not found"), *MissionID);
        return false;
    }
    
    FQuest_MissionObjective NewObjective;
    NewObjective.ObjectiveText = ObjectiveText;
    NewObjective.TargetCount = TargetCount;
    NewObjective.CurrentCount = 0;
    NewObjective.bIsCompleted = false;
    
    MissionData->Objectives.Add(NewObjective);
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Added objective to mission %s: %s"), *MissionID, *ObjectiveText);
    
    return true;
}

bool UQuest_MissionManager::UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 Progress)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return false;
    }
    
    FQuest_MissionData& MissionData = ActiveMissions[MissionID];
    
    if (!MissionData.Objectives.IsValidIndex(ObjectiveIndex))
    {
        return false;
    }
    
    FQuest_MissionObjective& Objective = MissionData.Objectives[ObjectiveIndex];
    Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);
    
    if (Objective.CurrentCount >= Objective.TargetCount)
    {
        Objective.bIsCompleted = true;
        UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Objective completed: %s"), *Objective.ObjectiveText);
    }
    
    CheckMissionCompletion();
    return true;
}

bool UQuest_MissionManager::CompleteObjective(const FString& MissionID, int32 ObjectiveIndex)
{
    if (!ActiveMissions.Contains(MissionID))
    {
        return false;
    }
    
    FQuest_MissionData& MissionData = ActiveMissions[MissionID];
    
    if (!MissionData.Objectives.IsValidIndex(ObjectiveIndex))
    {
        return false;
    }
    
    FQuest_MissionObjective& Objective = MissionData.Objectives[ObjectiveIndex];
    Objective.bIsCompleted = true;
    Objective.CurrentCount = Objective.TargetCount;
    
    CheckMissionCompletion();
    return true;
}

void UQuest_MissionManager::OnCrowdEventTriggered(const FString& EventType, const FVector& Location, int32 CrowdSize)
{
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Crowd event triggered - %s at location (%f, %f, %f) with %d entities"), 
           *EventType, Location.X, Location.Y, Location.Z, CrowdSize);
    
    // Check if any active missions are waiting for this crowd event
    for (auto& Mission : ActiveMissions)
    {
        if (CrowdMissionRegistry.Contains(Mission.Key))
        {
            FString ExpectedEventType = CrowdMissionRegistry[Mission.Key];
            if (ExpectedEventType == EventType)
            {
                // Update mission progress based on crowd event
                UpdateObjectiveProgress(Mission.Key, 0, 1);
            }
        }
    }
}

void UQuest_MissionManager::RegisterCrowdMission(const FString& MissionID, const FString& CrowdEventType)
{
    CrowdMissionRegistry.Add(MissionID, CrowdEventType);
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Registered crowd mission %s for event %s"), *MissionID, *CrowdEventType);
}

void UQuest_MissionManager::UpdateMissionTimers(float DeltaTime)
{
    for (auto& Mission : ActiveMissions)
    {
        FQuest_MissionData& MissionData = Mission.Value;
        
        if (MissionData.TimeLimit > 0.0f)
        {
            MissionData.ElapsedTime += DeltaTime;
            
            if (MissionData.ElapsedTime >= MissionData.TimeLimit)
            {
                FailMission(Mission.Key);
                break; // Exit loop since we modified the container
            }
        }
    }
}

void UQuest_MissionManager::CheckMissionCompletion()
{
    TArray<FString> CompletedMissions;
    
    for (const auto& Mission : ActiveMissions)
    {
        const FQuest_MissionData& MissionData = Mission.Value;
        
        bool bAllObjectivesComplete = true;
        for (const FQuest_MissionObjective& Objective : MissionData.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesComplete = false;
                break;
            }
        }
        
        if (bAllObjectivesComplete && MissionData.Objectives.Num() > 0)
        {
            CompletedMissions.Add(Mission.Key);
        }
    }
    
    for (const FString& MissionID : CompletedMissions)
    {
        CompleteMission(MissionID);
    }
}

void UQuest_MissionManager::InitializeDefaultMissions()
{
    // Gathering mission
    CreateMission(TEXT("GATHER_BERRIES"), TEXT("Sacred Grove Harvest"), 
                 TEXT("Collect crimson berries from the dangerous territories to feed the hungry tribe."), 
                 EQuest_MissionType::Gathering);
    AddObjective(TEXT("GATHER_BERRIES"), TEXT("Collect 10 crimson berries"), 10);
    AddObjective(TEXT("GATHER_BERRIES"), TEXT("Return to Elder Gatherer"), 1);
    
    // Escort mission
    CreateMission(TEXT("ESCORT_HUNTER"), TEXT("Lost in the Hills"), 
                 TEXT("Guide the lost hunter safely through the treacherous hills to the river crossing."), 
                 EQuest_MissionType::Escort);
    AddObjective(TEXT("ESCORT_HUNTER"), TEXT("Find the lost hunter"), 1);
    AddObjective(TEXT("ESCORT_HUNTER"), TEXT("Escort to river crossing"), 1);
    AddObjective(TEXT("ESCORT_HUNTER"), TEXT("Avoid predator encounters"), 1);
    
    // Evacuation mission
    CreateMission(TEXT("VILLAGE_EVACUATION"), TEXT("Emergency Evacuation"), 
                 TEXT("Help evacuate the village before the approaching predator pack arrives."), 
                 EQuest_MissionType::Evacuation);
    AddObjective(TEXT("VILLAGE_EVACUATION"), TEXT("Warn 5 villagers"), 5);
    AddObjective(TEXT("VILLAGE_EVACUATION"), TEXT("Guide crowd to safety"), 1);
    
    // Register crowd integration
    RegisterCrowdMission(TEXT("VILLAGE_EVACUATION"), TEXT("evacuation"));
    RegisterCrowdMission(TEXT("GATHER_BERRIES"), TEXT("gathering"));
    
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Initialized %d default missions"), AvailableMissions.Num());
}

void UQuest_MissionManager::SetupCrowdIntegration()
{
    // This would typically connect to the crowd simulation system
    // For now, we just log that the integration is set up
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Crowd integration system ready"));
}

bool UQuest_MissionManager::ValidateMissionData(const FQuest_MissionData& MissionData)
{
    if (MissionData.MissionID.IsEmpty() || MissionData.MissionName.IsEmpty())
    {
        return false;
    }
    
    return true;
}

void UQuest_MissionManager::BroadcastMissionEvent(const FString& MissionID, const FString& EventType)
{
    // This would typically broadcast to other systems (UI, audio, etc.)
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Broadcasting event %s for mission %s"), *EventType, *MissionID);
}
#include "Quest_SurvivalMissionSystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UQuest_SurvivalMissionSystem::UQuest_SurvivalMissionSystem()
{
    MaxActiveMissions = 5;
    MissionCheckInterval = 1.0f;
    LastMissionCheck = 0.0f;
}

void UQuest_SurvivalMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initializing quest system"));
    
    InitializeDefaultMissions();
    
    // Start the first basic mission automatically
    StartMission(TEXT("MISSION_GATHER_WOOD"));
}

void UQuest_SurvivalMissionSystem::Tick(float DeltaTime)
{
    LastMissionCheck += DeltaTime;
    
    if (LastMissionCheck >= MissionCheckInterval)
    {
        CheckMissionTimeouts(DeltaTime);
        CheckObjectiveCompletion();
        LastMissionCheck = 0.0f;
    }
}

void UQuest_SurvivalMissionSystem::StartMission(const FString& MissionID)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Cannot start mission %s - max active missions reached"), *MissionID);
        return;
    }

    // Check if mission already active
    if (IsMissionActive(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Mission %s already active"), *MissionID);
        return;
    }

    // Find mission template and activate it
    FQuest_SurvivalMission* Mission = FindMissionByID(MissionID);
    if (Mission)
    {
        Mission->bIsActive = true;
        Mission->ElapsedTime = 0.0f;
        ActiveMissions.Add(*Mission);
        
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Started mission: %s"), *Mission->MissionTitle);
    }
}

void UQuest_SurvivalMissionSystem::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].bIsCompleted = true;
            ActiveMissions[i].bIsActive = false;
            
            CompletedMissions.Add(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Completed mission: %s"), *MissionID);
            break;
        }
    }
}

void UQuest_SurvivalMissionSystem::UpdateObjectiveProgress(const FString& MissionID, int32 ObjectiveIndex, int32 ProgressAmount)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Objectives.IsValidIndex(ObjectiveIndex))
        {
            FQuest_MissionObjective& Objective = Mission.Objectives[ObjectiveIndex];
            Objective.CurrentProgress = FMath::Min(Objective.CurrentProgress + ProgressAmount, Objective.RequiredProgress);
            
            if (Objective.CurrentProgress >= Objective.RequiredProgress)
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Objective completed: %s"), *Objective.ObjectiveText);
            }
            
            break;
        }
    }
}

bool UQuest_SurvivalMissionSystem::IsMissionActive(const FString& MissionID) const
{
    for (const FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission.bIsActive;
        }
    }
    return false;
}

TArray<FQuest_SurvivalMission> UQuest_SurvivalMissionSystem::GetActiveMissions() const
{
    return ActiveMissions;
}

void UQuest_SurvivalMissionSystem::CreateHuntingMission(const FString& DinosaurType, int32 RequiredKills)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.MissionTitle = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    NewMission.MissionDescription = FString::Printf(TEXT("Hunt and kill %d %s for meat and materials"), RequiredKills, *DinosaurType);
    NewMission.TimeLimit = 1200.0f; // 20 minutes
    NewMission.ExperienceReward = RequiredKills * 50;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Kill %d %s"), RequiredKills, *DinosaurType);
    Objective.RequiredProgress = RequiredKills;
    Objective.TargetActorTag = DinosaurType;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Created hunting mission for %s"), *DinosaurType);
}

void UQuest_SurvivalMissionSystem::CreateGatheringMission(const FString& ResourceType, int32 RequiredAmount)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.MissionTitle = FString::Printf(TEXT("Gather %s"), *ResourceType);
    NewMission.MissionDescription = FString::Printf(TEXT("Collect %d units of %s for crafting and survival"), RequiredAmount, *ResourceType);
    NewMission.TimeLimit = 900.0f; // 15 minutes
    NewMission.ExperienceReward = RequiredAmount * 10;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Gather %d %s"), RequiredAmount, *ResourceType);
    Objective.RequiredProgress = RequiredAmount;
    Objective.TargetActorTag = ResourceType;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Created gathering mission for %s"), *ResourceType);
}

void UQuest_SurvivalMissionSystem::CreateExplorationMission(const FString& LocationName, float DiscoveryRadius)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.MissionTitle = FString::Printf(TEXT("Explore %s"), *LocationName);
    NewMission.MissionDescription = FString::Printf(TEXT("Discover and explore the %s region"), *LocationName);
    NewMission.TimeLimit = 1800.0f; // 30 minutes
    NewMission.ExperienceReward = 200;
    
    FQuest_MissionObjective Objective;
    Objective.ObjectiveText = FString::Printf(TEXT("Explore %s"), *LocationName);
    Objective.RequiredProgress = 1;
    Objective.TargetActorTag = LocationName;
    
    NewMission.Objectives.Add(Objective);
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Created exploration mission for %s"), *LocationName);
}

void UQuest_SurvivalMissionSystem::RegisterKill(const FString& DinosaurType)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        for (int32 i = 0; i < Mission.Objectives.Num(); i++)
        {
            FQuest_MissionObjective& Objective = Mission.Objectives[i];
            if (Objective.TargetActorTag == DinosaurType && !Objective.bIsCompleted)
            {
                UpdateObjectiveProgress(Mission.MissionID, i, 1);
            }
        }
    }
}

void UQuest_SurvivalMissionSystem::RegisterResourceGathered(const FString& ResourceType, int32 Amount)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        for (int32 i = 0; i < Mission.Objectives.Num(); i++)
        {
            FQuest_MissionObjective& Objective = Mission.Objectives[i];
            if (Objective.TargetActorTag == ResourceType && !Objective.bIsCompleted)
            {
                UpdateObjectiveProgress(Mission.MissionID, i, Amount);
            }
        }
    }
}

void UQuest_SurvivalMissionSystem::RegisterLocationDiscovered(const FString& LocationName)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        for (int32 i = 0; i < Mission.Objectives.Num(); i++)
        {
            FQuest_MissionObjective& Objective = Mission.Objectives[i];
            if (Objective.TargetActorTag == LocationName && !Objective.bIsCompleted)
            {
                UpdateObjectiveProgress(Mission.MissionID, i, 1);
            }
        }
    }
}

void UQuest_SurvivalMissionSystem::CheckMissionTimeouts(float DeltaTime)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        Mission.ElapsedTime += DeltaTime;
        
        if (Mission.ElapsedTime >= Mission.TimeLimit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Mission %s timed out"), *Mission.MissionTitle);
            ActiveMissions.RemoveAt(i);
        }
    }
}

void UQuest_SurvivalMissionSystem::CheckObjectiveCompletion()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        FQuest_SurvivalMission& Mission = ActiveMissions[i];
        bool bAllObjectivesComplete = true;
        
        for (const FQuest_MissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesComplete = false;
                break;
            }
        }
        
        if (bAllObjectivesComplete)
        {
            CompleteMission(Mission.MissionID);
        }
    }
}

FQuest_SurvivalMission* UQuest_SurvivalMissionSystem::FindMissionByID(const FString& MissionID)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return &Mission;
        }
    }
    return nullptr;
}

void UQuest_SurvivalMissionSystem::InitializeDefaultMissions()
{
    // Basic gathering mission
    FQuest_SurvivalMission GatherWoodMission;
    GatherWoodMission.MissionID = TEXT("MISSION_GATHER_WOOD");
    GatherWoodMission.MissionTitle = TEXT("Gather Wood");
    GatherWoodMission.MissionDescription = TEXT("Collect wood to build your first shelter and tools");
    GatherWoodMission.TimeLimit = 600.0f;
    GatherWoodMission.ExperienceReward = 50;
    
    FQuest_MissionObjective WoodObjective;
    WoodObjective.ObjectiveText = TEXT("Gather 10 pieces of wood");
    WoodObjective.RequiredProgress = 10;
    WoodObjective.TargetActorTag = TEXT("Wood");
    GatherWoodMission.Objectives.Add(WoodObjective);
    
    // Basic hunting mission
    FQuest_SurvivalMission HuntRaptorMission;
    HuntRaptorMission.MissionID = TEXT("MISSION_HUNT_RAPTOR");
    HuntRaptorMission.MissionTitle = TEXT("Hunt Raptors");
    HuntRaptorMission.MissionDescription = TEXT("Hunt 2 raptors for meat and hide");
    HuntRaptorMission.TimeLimit = 1200.0f;
    HuntRaptorMission.ExperienceReward = 150;
    
    FQuest_MissionObjective RaptorObjective;
    RaptorObjective.ObjectiveText = TEXT("Kill 2 Raptors");
    RaptorObjective.RequiredProgress = 2;
    RaptorObjective.TargetActorTag = TEXT("Raptor");
    HuntRaptorMission.Objectives.Add(RaptorObjective);
    
    // Exploration mission
    FQuest_SurvivalMission ExploreCaveMission;
    ExploreCaveMission.MissionID = TEXT("MISSION_EXPLORE_CAVE");
    ExploreCaveMission.MissionTitle = TEXT("Explore the Ancient Cave");
    ExploreCaveMission.MissionDescription = TEXT("Discover the mysterious cave system");
    ExploreCaveMission.TimeLimit = 1800.0f;
    ExploreCaveMission.ExperienceReward = 200;
    
    FQuest_MissionObjective CaveObjective;
    CaveObjective.ObjectiveText = TEXT("Explore the Ancient Cave");
    CaveObjective.RequiredProgress = 1;
    CaveObjective.TargetActorTag = TEXT("AncientCave");
    ExploreCaveMission.Objectives.Add(CaveObjective);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initialized 3 default missions"));
}

FString UQuest_SurvivalMissionSystem::GenerateUniqueMissionID()
{
    static int32 MissionCounter = 1000;
    return FString::Printf(TEXT("MISSION_%d"), ++MissionCounter);
}
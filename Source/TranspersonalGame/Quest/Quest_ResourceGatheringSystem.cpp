#include "Quest_ResourceGatheringSystem.h"
#include "Engine/Engine.h"

UQuest_ResourceGatheringSystem::UQuest_ResourceGatheringSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
}

void UQuest_ResourceGatheringSystem::BeginPlay()
{
    Super::BeginPlay();
    CreateBasicSurvivalMissions();
}

void UQuest_ResourceGatheringSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateMissionTimers(DeltaTime);
    CheckAllMissionsCompletion();
}

void UQuest_ResourceGatheringSystem::StartGatheringMission(const FString& MissionName, const TArray<FQuest_ResourceRequirement>& Requirements, float TimeLimit)
{
    FQuest_GatheringMission NewMission;
    NewMission.MissionName = MissionName;
    NewMission.Description = FString::Printf(TEXT("Gather resources: %s"), *MissionName);
    NewMission.Requirements = Requirements;
    NewMission.TimeLimit = TimeLimit;
    NewMission.bIsActive = true;
    NewMission.bIsCompleted = false;

    ActiveMissions.Add(NewMission);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Quest Started: %s"), *MissionName));
    }
}

void UQuest_ResourceGatheringSystem::AddResource(EQuest_ResourceType ResourceType, int32 Amount)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (!Mission.bIsCompleted)
        {
            for (FQuest_ResourceRequirement& Requirement : Mission.Requirements)
            {
                if (Requirement.ResourceType == ResourceType)
                {
                    Requirement.CurrentAmount = FMath::Min(Requirement.CurrentAmount + Amount, Requirement.RequiredAmount);
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow,
                            FString::Printf(TEXT("Resource Added: %d/%d"), Requirement.CurrentAmount, Requirement.RequiredAmount));
                    }
                }
            }
        }
    }
}

bool UQuest_ResourceGatheringSystem::CheckMissionCompletion(const FString& MissionName)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName && Mission.bIsActive && !Mission.bIsCompleted)
        {
            bool bAllRequirementsMet = true;
            for (const FQuest_ResourceRequirement& Requirement : Mission.Requirements)
            {
                if (Requirement.CurrentAmount < Requirement.RequiredAmount)
                {
                    bAllRequirementsMet = false;
                    break;
                }
            }
            return bAllRequirementsMet;
        }
    }
    return false;
}

void UQuest_ResourceGatheringSystem::CompleteMission(const FString& MissionName)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionName == MissionName)
        {
            ActiveMissions[i].bIsCompleted = true;
            ActiveMissions[i].bIsActive = false;
            CompletedMissions.Add(ActiveMissions[i]);
            ActiveMissions.RemoveAt(i);

            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
                    FString::Printf(TEXT("Quest Completed: %s"), *MissionName));
            }
            break;
        }
    }
}

FQuest_GatheringMission UQuest_ResourceGatheringSystem::GetActiveMission(const FString& MissionName)
{
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.MissionName == MissionName)
        {
            return Mission;
        }
    }
    return FQuest_GatheringMission();
}

TArray<FQuest_GatheringMission> UQuest_ResourceGatheringSystem::GetAllActiveMissions()
{
    return ActiveMissions;
}

void UQuest_ResourceGatheringSystem::CreateBasicSurvivalMissions()
{
    // Mission 1: First Tools
    TArray<FQuest_ResourceRequirement> ToolRequirements;
    FQuest_ResourceRequirement StoneReq;
    StoneReq.ResourceType = EQuest_ResourceType::Stone;
    StoneReq.RequiredAmount = 3;
    ToolRequirements.Add(StoneReq);

    FQuest_ResourceRequirement WoodReq;
    WoodReq.ResourceType = EQuest_ResourceType::Wood;
    WoodReq.RequiredAmount = 2;
    ToolRequirements.Add(WoodReq);

    StartGatheringMission(TEXT("First Tools"), ToolRequirements, 600.0f);

    // Mission 2: Shelter Materials
    TArray<FQuest_ResourceRequirement> ShelterRequirements;
    FQuest_ResourceRequirement WoodReq2;
    WoodReq2.ResourceType = EQuest_ResourceType::Wood;
    WoodReq2.RequiredAmount = 10;
    ShelterRequirements.Add(WoodReq2);

    FQuest_ResourceRequirement PlantReq;
    PlantReq.ResourceType = EQuest_ResourceType::Plant;
    PlantReq.RequiredAmount = 5;
    ShelterRequirements.Add(PlantReq);

    StartGatheringMission(TEXT("Shelter Materials"), ShelterRequirements, 900.0f);

    // Mission 3: Water Container
    TArray<FQuest_ResourceRequirement> WaterRequirements;
    FQuest_ResourceRequirement StoneReq2;
    StoneReq2.ResourceType = EQuest_ResourceType::Stone;
    StoneReq2.RequiredAmount = 1;
    WaterRequirements.Add(StoneReq2);

    FQuest_ResourceRequirement PlantReq2;
    PlantReq2.ResourceType = EQuest_ResourceType::Plant;
    PlantReq2.RequiredAmount = 3;
    WaterRequirements.Add(PlantReq2);

    StartGatheringMission(TEXT("Water Container"), WaterRequirements, 450.0f);
}

void UQuest_ResourceGatheringSystem::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted)
        {
            Mission.TimeLimit -= DeltaTime;
            if (Mission.TimeLimit <= 0.0f)
            {
                Mission.bIsActive = false;
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
                        FString::Printf(TEXT("Quest Failed: %s (Time Limit Exceeded)"), *Mission.MissionName));
                }
            }
        }
    }
}

void UQuest_ResourceGatheringSystem::CheckAllMissionsCompletion()
{
    TArray<FString> MissionsToComplete;
    
    for (const FQuest_GatheringMission& Mission : ActiveMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted)
        {
            if (CheckMissionCompletion(Mission.MissionName))
            {
                MissionsToComplete.Add(Mission.MissionName);
            }
        }
    }

    for (const FString& MissionName : MissionsToComplete)
    {
        CompleteMission(MissionName);
    }
}
#include "Quest_SurvivalMissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UQuest_SurvivalMissionManager::UQuest_SurvivalMissionManager()
{
    NextMissionID = 1;
}

void UQuest_SurvivalMissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionManager initialized"));
    
    InitializeDefaultMissions();
}

void UQuest_SurvivalMissionManager::Deinitialize()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    Super::Deinitialize();
}

void UQuest_SurvivalMissionManager::StartMission(const FString& MissionID)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Inactive)
        {
            Mission.Status = EQuest_MissionStatus::Active;
            Mission.TimeRemaining = Mission.TimeLimit;
            
            UE_LOG(LogTemp, Warning, TEXT("Mission started: %s"), *Mission.MissionName);
            BroadcastMissionUpdate(Mission);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EQuest_MissionStatus::Completed;
            CompletedMissions.Add(ActiveMissions[i]);
            
            UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s"), *ActiveMissions[i].MissionName);
            BroadcastMissionUpdate(ActiveMissions[i]);
            
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::FailMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].Status = EQuest_MissionStatus::Failed;
            
            UE_LOG(LogTemp, Warning, TEXT("Mission failed: %s"), *ActiveMissions[i].MissionName);
            BroadcastMissionUpdate(ActiveMissions[i]);
            
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalMissionManager::UpdateMissionProgress(const FString& MissionID, int32 ProgressIncrement)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::Active)
        {
            Mission.CurrentProgress += ProgressIncrement;
            
            if (Mission.CurrentProgress >= Mission.TargetQuantity)
            {
                CompleteMission(MissionID);
            }
            else
            {
                BroadcastMissionUpdate(Mission);
            }
            break;
        }
    }
}

FQuest_SurvivalMission UQuest_SurvivalMissionManager::GetMission(const FString& MissionID)
{
    for (const FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    return FQuest_SurvivalMission();
}

TArray<FQuest_SurvivalMission> UQuest_SurvivalMissionManager::GetActiveMissions()
{
    return ActiveMissions;
}

void UQuest_SurvivalMissionManager::CreateHuntMission(const FString& DinosaurType, const FVector& HuntLocation)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    NewMission.Description = FString::Printf(TEXT("Track and hunt a %s in the designated area. Use primitive weapons and stealth tactics."), *DinosaurType);
    NewMission.MissionType = EQuest_MissionType::Hunt_Dinosaur;
    NewMission.Status = EQuest_MissionStatus::Inactive;
    NewMission.TargetQuantity = 1;
    NewMission.TargetLocation = HuntLocation;
    NewMission.TimeLimit = 600.0f;
    NewMission.TimeRemaining = 600.0f;
    NewMission.RequiredItems.Add(TEXT("Stone Spear"));
    NewMission.RewardItems.Add(TEXT("Dinosaur Meat"));
    NewMission.RewardItems.Add(TEXT("Dinosaur Hide"));
    
    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Hunt mission created: %s"), *NewMission.MissionName);
}

void UQuest_SurvivalMissionManager::CreateGatherMission(const FString& ResourceType, int32 Quantity)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    NewMission.Description = FString::Printf(TEXT("Collect %d %s from the environment. Search near water sources and rocky areas."), Quantity, *ResourceType);
    NewMission.MissionType = EQuest_MissionType::Gather_Resources;
    NewMission.Status = EQuest_MissionStatus::Inactive;
    NewMission.TargetQuantity = Quantity;
    NewMission.TimeLimit = 300.0f;
    NewMission.TimeRemaining = 300.0f;
    NewMission.RewardItems.Add(TEXT("Crafting Experience"));
    
    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Gather mission created: %s"), *NewMission.MissionName);
}

void UQuest_SurvivalMissionManager::CreateExploreMission(const FVector& ExploreLocation, float ExploreRadius)
{
    FQuest_SurvivalMission NewMission;
    NewMission.MissionID = GenerateMissionID();
    NewMission.MissionName = TEXT("Explore Unknown Territory");
    NewMission.Description = TEXT("Venture into unexplored territory and map the area. Watch for predators and mark resource locations.");
    NewMission.MissionType = EQuest_MissionType::Explore_Territory;
    NewMission.Status = EQuest_MissionStatus::Inactive;
    NewMission.TargetQuantity = 1;
    NewMission.TargetLocation = ExploreLocation;
    NewMission.TimeLimit = 900.0f;
    NewMission.TimeRemaining = 900.0f;
    NewMission.RewardItems.Add(TEXT("Territory Knowledge"));
    NewMission.RewardItems.Add(TEXT("Survival Experience"));
    
    ActiveMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Explore mission created: %s"), *NewMission.MissionName);
}

void UQuest_SurvivalMissionManager::ProcessMissionTimers(float DeltaTime)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveMissions[i].Status == EQuest_MissionStatus::Active)
        {
            ActiveMissions[i].TimeRemaining -= DeltaTime;
            
            if (ActiveMissions[i].TimeRemaining <= 0.0f)
            {
                FailMission(ActiveMissions[i].MissionID);
            }
        }
    }
}

void UQuest_SurvivalMissionManager::InitializeDefaultMissions()
{
    // Create initial survival missions
    CreateHuntMission(TEXT("Velociraptor"), FVector(5000.0f, 5000.0f, 100.0f));
    CreateGatherMission(TEXT("Stone"), 5);
    CreateGatherMission(TEXT("Wood"), 3);
    CreateExploreMission(FVector(10000.0f, 10000.0f, 100.0f), 2000.0f);
    
    // Create shelter building mission
    FQuest_SurvivalMission ShelterMission;
    ShelterMission.MissionID = GenerateMissionID();
    ShelterMission.MissionName = TEXT("Build Basic Shelter");
    ShelterMission.Description = TEXT("Construct a basic shelter to protect against predators and weather. Gather materials and find a suitable location.");
    ShelterMission.MissionType = EQuest_MissionType::Build_Shelter;
    ShelterMission.Status = EQuest_MissionStatus::Inactive;
    ShelterMission.TargetQuantity = 1;
    ShelterMission.TimeLimit = 1200.0f;
    ShelterMission.TimeRemaining = 1200.0f;
    ShelterMission.RequiredItems.Add(TEXT("Wood"));
    ShelterMission.RequiredItems.Add(TEXT("Stone"));
    ShelterMission.RequiredItems.Add(TEXT("Leaves"));
    ShelterMission.RewardItems.Add(TEXT("Shelter"));
    ShelterMission.RewardItems.Add(TEXT("Safety Bonus"));
    ActiveMissions.Add(ShelterMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Default survival missions initialized: %d missions"), ActiveMissions.Num());
}

FString UQuest_SurvivalMissionManager::GenerateMissionID()
{
    FString MissionID = FString::Printf(TEXT("MISSION_%04d"), NextMissionID);
    NextMissionID++;
    return MissionID;
}

void UQuest_SurvivalMissionManager::BroadcastMissionUpdate(const FQuest_SurvivalMission& Mission)
{
    // Log mission update for debugging
    FString StatusString;
    switch (Mission.Status)
    {
        case EQuest_MissionStatus::Inactive: StatusString = TEXT("Inactive"); break;
        case EQuest_MissionStatus::Active: StatusString = TEXT("Active"); break;
        case EQuest_MissionStatus::Completed: StatusString = TEXT("Completed"); break;
        case EQuest_MissionStatus::Failed: StatusString = TEXT("Failed"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mission Update: %s - Status: %s - Progress: %d/%d"), 
           *Mission.MissionName, *StatusString, Mission.CurrentProgress, Mission.TargetQuantity);
}
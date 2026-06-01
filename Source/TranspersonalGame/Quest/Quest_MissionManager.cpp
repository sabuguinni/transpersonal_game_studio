#include "Quest_MissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"

UQuest_MissionManager::UQuest_MissionManager()
{
    MissionDataTable = nullptr;
}

void UQuest_MissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Initializing mission system"));
    
    LoadMissionDatabase();
    InitializeSurvivalMissions();
    RegisterMissionCallbacks();
}

void UQuest_MissionManager::Deinitialize()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    MissionDatabase.Empty();
    
    Super::Deinitialize();
}

bool UQuest_MissionManager::StartMission(const FString& MissionID)
{
    if (MissionID.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Cannot start mission with empty ID"));
        return false;
    }

    if (IsMissionActive(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Mission %s is already active"), *MissionID);
        return false;
    }

    if (IsMissionCompleted(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Mission %s is already completed"), *MissionID);
        return false;
    }

    if (!ValidateMissionPrerequisites(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Mission %s prerequisites not met"), *MissionID);
        return false;
    }

    FQuest_MissionData* MissionData = MissionDatabase.Find(MissionID);
    if (!MissionData)
    {
        UE_LOG(LogTemp, Error, TEXT("Quest_MissionManager: Mission data not found for %s"), *MissionID);
        return false;
    }

    FQuest_ActiveMission NewActiveMission;
    NewActiveMission.MissionID = MissionID;
    NewActiveMission.StartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewActiveMission.ObjectiveStates.SetNum(MissionData->Objectives.Num());
    for (int32 i = 0; i < NewActiveMission.ObjectiveStates.Num(); i++)
    {
        NewActiveMission.ObjectiveStates[i] = false;
    }

    ActiveMissions.Add(NewActiveMission);
    
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Started mission %s with %d objectives"), 
           *MissionID, MissionData->Objectives.Num());
    
    BroadcastMissionUpdate(MissionID, TEXT("Started"));
    return true;
}

bool UQuest_MissionManager::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].bIsCompleted = true;
            CompletedMissions.AddUnique(MissionID);
            
            FQuest_MissionData* MissionData = MissionDatabase.Find(MissionID);
            if (MissionData)
            {
                UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Completed mission %s - Reward: %d XP"), 
                       *MissionID, MissionData->ExperienceReward);
            }
            
            ActiveMissions.RemoveAt(i);
            BroadcastMissionUpdate(MissionID, TEXT("Completed"));
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Cannot complete mission %s - not active"), *MissionID);
    return false;
}

bool UQuest_MissionManager::FailMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].bIsFailed = true;
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Failed mission %s"), *MissionID);
            BroadcastMissionUpdate(MissionID, TEXT("Failed"));
            return true;
        }
    }
    
    return false;
}

bool UQuest_MissionManager::UpdateObjective(const FString& MissionID, int32 ObjectiveIndex, bool bCompleted)
{
    for (FQuest_ActiveMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Mission.ObjectiveStates.Num())
            {
                Mission.ObjectiveStates[ObjectiveIndex] = bCompleted;
                
                UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Updated objective %d for mission %s: %s"), 
                       ObjectiveIndex, *MissionID, bCompleted ? TEXT("Completed") : TEXT("Reset"));
                
                // Check if all objectives are completed
                bool bAllCompleted = true;
                for (bool ObjectiveState : Mission.ObjectiveStates)
                {
                    if (!ObjectiveState)
                    {
                        bAllCompleted = false;
                        break;
                    }
                }
                
                if (bAllCompleted)
                {
                    CompleteMission(MissionID);
                }
                
                BroadcastMissionUpdate(MissionID, TEXT("ObjectiveUpdated"));
                return true;
            }
        }
    }
    
    return false;
}

TArray<FQuest_ActiveMission> UQuest_MissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

TArray<FString> UQuest_MissionManager::GetCompletedMissions() const
{
    return CompletedMissions;
}

FQuest_MissionData UQuest_MissionManager::GetMissionData(const FString& MissionID) const
{
    if (const FQuest_MissionData* FoundData = MissionDatabase.Find(MissionID))
    {
        return *FoundData;
    }
    
    return FQuest_MissionData();
}

bool UQuest_MissionManager::IsMissionActive(const FString& MissionID) const
{
    for (const FQuest_ActiveMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return true;
        }
    }
    return false;
}

bool UQuest_MissionManager::IsMissionCompleted(const FString& MissionID) const
{
    return CompletedMissions.Contains(MissionID);
}

void UQuest_MissionManager::LoadMissionDatabase()
{
    // In a real implementation, this would load from a DataTable asset
    // For now, we'll create missions programmatically
    CreateSurvivalMissions();
}

void UQuest_MissionManager::CreateSurvivalMissions()
{
    // Survival Mission 1: First Hunt
    FQuest_MissionData FirstHunt;
    FirstHunt.MissionID = TEXT("HUNT_001");
    FirstHunt.MissionTitle = TEXT("First Hunt");
    FirstHunt.MissionDescription = TEXT("Hunt your first dinosaur to survive in this prehistoric world.");
    FirstHunt.Objectives.Add(TEXT("Find a small herbivore dinosaur"));
    FirstHunt.Objectives.Add(TEXT("Craft a stone spear"));
    FirstHunt.Objectives.Add(TEXT("Successfully hunt the dinosaur"));
    FirstHunt.Rewards.Add(TEXT("Raw Meat x5"));
    FirstHunt.Rewards.Add(TEXT("Dinosaur Hide x2"));
    FirstHunt.ExperienceReward = 100;
    FirstHunt.bIsMainQuest = true;
    FirstHunt.MissionLocation = FVector(0, 0, 100);
    FirstHunt.MissionRadius = 5000.0f;
    MissionDatabase.Add(FirstHunt.MissionID, FirstHunt);

    // Survival Mission 2: Shelter Building
    FQuest_MissionData ShelterBuild;
    ShelterBuild.MissionID = TEXT("BUILD_001");
    ShelterBuild.MissionTitle = TEXT("Build Your First Shelter");
    ShelterBuild.MissionDescription = TEXT("Create a basic shelter to protect yourself from predators and weather.");
    ShelterBuild.Objectives.Add(TEXT("Gather 20 wood pieces"));
    ShelterBuild.Objectives.Add(TEXT("Collect 15 stones"));
    ShelterBuild.Objectives.Add(TEXT("Find a suitable location"));
    ShelterBuild.Objectives.Add(TEXT("Build a basic hut"));
    ShelterBuild.Rewards.Add(TEXT("Shelter Blueprint"));
    ShelterBuild.Rewards.Add(TEXT("Crafting Table"));
    ShelterBuild.ExperienceReward = 150;
    ShelterBuild.bIsMainQuest = true;
    ShelterBuild.MissionLocation = FVector(2000, 2000, 100);
    ShelterBuild.MissionRadius = 3000.0f;
    MissionDatabase.Add(ShelterBuild.MissionID, ShelterBuild);

    // Survival Mission 3: Water Source
    FQuest_MissionData WaterSource;
    WaterSource.MissionID = TEXT("WATER_001");
    WaterSource.MissionTitle = TEXT("Find Fresh Water");
    WaterSource.MissionDescription = TEXT("Locate a reliable source of fresh water for long-term survival.");
    WaterSource.Objectives.Add(TEXT("Explore the nearby river"));
    WaterSource.Objectives.Add(TEXT("Craft a water container"));
    WaterSource.Objectives.Add(TEXT("Collect 10 units of fresh water"));
    WaterSource.Rewards.Add(TEXT("Water Container"));
    WaterSource.Rewards.Add(TEXT("Fresh Water x10"));
    WaterSource.ExperienceReward = 75;
    WaterSource.bIsMainQuest = false;
    WaterSource.MissionLocation = FVector(-3000, 1000, 50);
    WaterSource.MissionRadius = 4000.0f;
    MissionDatabase.Add(WaterSource.MissionID, WaterSource);

    // Survival Mission 4: Predator Encounter
    FQuest_MissionData PredatorEncounter;
    PredatorEncounter.MissionID = TEXT("COMBAT_001");
    PredatorEncounter.MissionTitle = TEXT("Survive the Predator");
    PredatorEncounter.MissionDescription = TEXT("A dangerous predator has been spotted near your area. Prepare for combat or find a way to avoid it.");
    PredatorEncounter.Objectives.Add(TEXT("Craft advanced weapons"));
    PredatorEncounter.Objectives.Add(TEXT("Set up defensive traps"));
    PredatorEncounter.Objectives.Add(TEXT("Survive the predator encounter"));
    PredatorEncounter.Prerequisites.Add(TEXT("HUNT_001"));
    PredatorEncounter.Prerequisites.Add(TEXT("BUILD_001"));
    PredatorEncounter.Rewards.Add(TEXT("Predator Trophy"));
    PredatorEncounter.Rewards.Add(TEXT("Advanced Crafting Materials"));
    PredatorEncounter.ExperienceReward = 250;
    PredatorEncounter.bIsMainQuest = true;
    PredatorEncounter.MissionLocation = FVector(5000, -2000, 200);
    PredatorEncounter.MissionRadius = 6000.0f;
    MissionDatabase.Add(PredatorEncounter.MissionID, PredatorEncounter);

    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Created %d survival missions"), MissionDatabase.Num());
}

void UQuest_MissionManager::InitializeSurvivalMissions()
{
    // Auto-start the first tutorial mission
    if (MissionDatabase.Contains(TEXT("HUNT_001")))
    {
        StartMission(TEXT("HUNT_001"));
        UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Auto-started tutorial mission HUNT_001"));
    }
}

void UQuest_MissionManager::RegisterMissionCallbacks()
{
    // This would register delegates for mission events in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Mission callbacks registered"));
}

bool UQuest_MissionManager::ValidateMissionPrerequisites(const FString& MissionID) const
{
    const FQuest_MissionData* MissionData = MissionDatabase.Find(MissionID);
    if (!MissionData)
    {
        return false;
    }

    for (const FString& Prerequisite : MissionData->Prerequisites)
    {
        if (!IsMissionCompleted(Prerequisite))
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_MissionManager: Prerequisite %s not completed for mission %s"), 
                   *Prerequisite, *MissionID);
            return false;
        }
    }

    return true;
}

void UQuest_MissionManager::BroadcastMissionUpdate(const FString& MissionID, const FString& UpdateType)
{
    // In a full implementation, this would broadcast to UI and other systems
    UE_LOG(LogTemp, Log, TEXT("Quest_MissionManager: Mission %s - %s"), *MissionID, *UpdateType);
}
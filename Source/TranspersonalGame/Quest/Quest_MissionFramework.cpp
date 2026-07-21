#include "Quest_MissionFramework.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UQuest_MissionFramework::UQuest_MissionFramework()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxActiveMissions = 5;
    bAutoGenerateMissions = true;
    MissionGenerationInterval = 300.0f; // 5 minutes
    LastMissionGenerationTime = 0.0f;
}

void UQuest_MissionFramework::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with some basic missions
    if (bAutoGenerateMissions)
    {
        GenerateRandomMissions();
    }
}

void UQuest_MissionFramework::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateMissionTimers(DeltaTime);
    CheckMissionCompletion();
    
    // Auto-generate missions periodically
    if (bAutoGenerateMissions)
    {
        LastMissionGenerationTime += DeltaTime;
        if (LastMissionGenerationTime >= MissionGenerationInterval)
        {
            GenerateRandomMissions();
            LastMissionGenerationTime = 0.0f;
        }
    }
}

bool UQuest_MissionFramework::CreateMission(const FQuest_MissionData& MissionData)
{
    if (!ValidateMissionData(MissionData))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid mission data for mission: %s"), *MissionData.MissionTitle);
        return false;
    }
    
    // Check if mission already exists
    for (const FQuest_MissionData& ExistingMission : AvailableMissions)
    {
        if (ExistingMission.MissionID == MissionData.MissionID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission with ID %s already exists"), *MissionData.MissionID);
            return false;
        }
    }
    
    AvailableMissions.Add(MissionData);
    UE_LOG(LogTemp, Log, TEXT("Created mission: %s"), *MissionData.MissionTitle);
    return true;
}

bool UQuest_MissionFramework::StartMission(const FString& MissionID)
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start mission - maximum active missions reached"));
        return false;
    }
    
    // Find mission in available missions
    for (int32 i = 0; i < AvailableMissions.Num(); i++)
    {
        if (AvailableMissions[i].MissionID == MissionID)
        {
            FQuest_MissionData MissionToStart = AvailableMissions[i];
            MissionToStart.MissionStatus = EQuest_MissionStatus::Active;
            MissionToStart.ElapsedTime = 0.0f;
            
            ActiveMissions.Add(MissionToStart);
            AvailableMissions.RemoveAt(i);
            
            OnMissionStarted(MissionToStart);
            UE_LOG(LogTemp, Log, TEXT("Started mission: %s"), *MissionToStart.MissionTitle);
            return true;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mission with ID %s not found in available missions"), *MissionID);
    return false;
}

bool UQuest_MissionFramework::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_MissionData CompletedMission = ActiveMissions[i];
            CompletedMission.MissionStatus = EQuest_MissionStatus::Completed;
            
            CompletedMissions.Add(CompletedMission);
            ActiveMissions.RemoveAt(i);
            
            OnMissionCompleted(CompletedMission);
            UE_LOG(LogTemp, Log, TEXT("Completed mission: %s"), *CompletedMission.MissionTitle);
            return true;
        }
    }
    
    return false;
}

bool UQuest_MissionFramework::FailMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_MissionData FailedMission = ActiveMissions[i];
            FailedMission.MissionStatus = EQuest_MissionStatus::Failed;
            
            CompletedMissions.Add(FailedMission);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Failed mission: %s"), *FailedMission.MissionTitle);
            return true;
        }
    }
    
    return false;
}

bool UQuest_MissionFramework::AbandonMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_MissionData AbandonedMission = ActiveMissions[i];
            AbandonedMission.MissionStatus = EQuest_MissionStatus::Abandoned;
            
            // Return to available missions if it can be retried
            AvailableMissions.Add(AbandonedMission);
            ActiveMissions.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Abandoned mission: %s"), *AbandonedMission.MissionTitle);
            return true;
        }
    }
    
    return false;
}

bool UQuest_MissionFramework::UpdateObjective(const FString& MissionID, int32 ObjectiveIndex, int32 Progress)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Mission.Objectives.Num())
            {
                FQuest_MissionObjective& Objective = Mission.Objectives[ObjectiveIndex];
                Objective.CurrentCount = FMath::Min(Progress, Objective.RequiredCount);
                
                if (Objective.CurrentCount >= Objective.RequiredCount)
                {
                    Objective.bIsCompleted = true;
                }
                
                OnObjectiveUpdated(MissionID, Objective);
                UE_LOG(LogTemp, Log, TEXT("Updated objective %d for mission %s: %d/%d"), 
                       ObjectiveIndex, *MissionID, Objective.CurrentCount, Objective.RequiredCount);
                return true;
            }
        }
    }
    
    return false;
}

bool UQuest_MissionFramework::CompleteObjective(const FString& MissionID, int32 ObjectiveIndex)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            if (ObjectiveIndex >= 0 && ObjectiveIndex < Mission.Objectives.Num())
            {
                FQuest_MissionObjective& Objective = Mission.Objectives[ObjectiveIndex];
                Objective.bIsCompleted = true;
                Objective.CurrentCount = Objective.RequiredCount;
                
                OnObjectiveUpdated(MissionID, Objective);
                UE_LOG(LogTemp, Log, TEXT("Completed objective %d for mission %s"), ObjectiveIndex, *MissionID);
                return true;
            }
        }
    }
    
    return false;
}

TArray<FQuest_MissionObjective> UQuest_MissionFramework::GetMissionObjectives(const FString& MissionID)
{
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission.Objectives;
        }
    }
    
    return TArray<FQuest_MissionObjective>();
}

TArray<FQuest_MissionData> UQuest_MissionFramework::GetAvailableMissions()
{
    return AvailableMissions;
}

TArray<FQuest_MissionData> UQuest_MissionFramework::GetActiveMissions()
{
    return ActiveMissions;
}

FQuest_MissionData UQuest_MissionFramework::GetMissionData(const FString& MissionID)
{
    // Check active missions first
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    // Check available missions
    for (const FQuest_MissionData& Mission : AvailableMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    // Check completed missions
    for (const FQuest_MissionData& Mission : CompletedMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    return FQuest_MissionData();
}

bool UQuest_MissionFramework::IsMissionCompleted(const FString& MissionID)
{
    for (const FQuest_MissionData& Mission : CompletedMissions)
    {
        if (Mission.MissionID == MissionID && Mission.MissionStatus == EQuest_MissionStatus::Completed)
        {
            return true;
        }
    }
    
    return false;
}

FQuest_MissionData UQuest_MissionFramework::GenerateHuntMission(const FVector& Location, const FString& TargetSpecies)
{
    FQuest_MissionData HuntMission;
    HuntMission.MissionID = GenerateUniqueMissionID();
    HuntMission.MissionTitle = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    HuntMission.MissionDescription = FString::Printf(TEXT("Track and hunt a %s in the designated area"), *TargetSpecies);
    HuntMission.MissionType = EQuest_MissionType::Hunt;
    HuntMission.MissionStatus = EQuest_MissionStatus::Available;
    HuntMission.Difficulty = EQuest_Difficulty::Experienced;
    HuntMission.MissionLocation = Location;
    HuntMission.MissionRadius = 2000.0f;
    HuntMission.TimeLimit = 1800.0f; // 30 minutes
    
    // Create hunt objective
    FQuest_MissionObjective HuntObjective;
    HuntObjective.ObjectiveDescription = FString::Printf(TEXT("Hunt 1 %s"), *TargetSpecies);
    HuntObjective.RequiredCount = 1;
    HuntObjective.TargetActorTag = TargetSpecies;
    HuntMission.Objectives.Add(HuntObjective);
    
    // Set rewards
    HuntMission.Reward.ExperiencePoints = 500;
    HuntMission.Reward.ResourcePoints = 200;
    HuntMission.Reward.ItemRewards.Add(TEXT("DinosaurMeat"));
    HuntMission.Reward.ItemRewards.Add(TEXT("DinosaurHide"));
    
    return HuntMission;
}

FQuest_MissionData UQuest_MissionFramework::GenerateGatherMission(const FVector& Location, const FString& ResourceType, int32 Quantity)
{
    FQuest_MissionData GatherMission;
    GatherMission.MissionID = GenerateUniqueMissionID();
    GatherMission.MissionTitle = FString::Printf(TEXT("Gather %s"), *ResourceType);
    GatherMission.MissionDescription = FString::Printf(TEXT("Collect %d %s from the surrounding area"), Quantity, *ResourceType);
    GatherMission.MissionType = EQuest_MissionType::Gather;
    GatherMission.MissionStatus = EQuest_MissionStatus::Available;
    GatherMission.Difficulty = EQuest_Difficulty::Novice;
    GatherMission.MissionLocation = Location;
    GatherMission.MissionRadius = 1500.0f;
    GatherMission.TimeLimit = 900.0f; // 15 minutes
    
    // Create gather objective
    FQuest_MissionObjective GatherObjective;
    GatherObjective.ObjectiveDescription = FString::Printf(TEXT("Collect %d %s"), Quantity, *ResourceType);
    GatherObjective.RequiredCount = Quantity;
    GatherObjective.TargetActorTag = ResourceType;
    GatherMission.Objectives.Add(GatherObjective);
    
    // Set rewards
    GatherMission.Reward.ExperiencePoints = 100 * Quantity;
    GatherMission.Reward.ResourcePoints = 50 * Quantity;
    GatherMission.Reward.ItemRewards.Add(TEXT("CraftingMaterials"));
    
    return GatherMission;
}

FQuest_MissionData UQuest_MissionFramework::GenerateExplorationMission(const FVector& Location, float ExplorationRadius)
{
    FQuest_MissionData ExplorationMission;
    ExplorationMission.MissionID = GenerateUniqueMissionID();
    ExplorationMission.MissionTitle = TEXT("Explore Unknown Territory");
    ExplorationMission.MissionDescription = TEXT("Venture into unexplored areas and discover new locations");
    ExplorationMission.MissionType = EQuest_MissionType::Explore;
    ExplorationMission.MissionStatus = EQuest_MissionStatus::Available;
    ExplorationMission.Difficulty = EQuest_Difficulty::Experienced;
    ExplorationMission.MissionLocation = Location;
    ExplorationMission.MissionRadius = ExplorationRadius;
    ExplorationMission.TimeLimit = 2400.0f; // 40 minutes
    
    // Create exploration objectives
    FQuest_MissionObjective ExploreObjective;
    ExploreObjective.ObjectiveDescription = TEXT("Discover 3 new locations");
    ExploreObjective.RequiredCount = 3;
    ExploreObjective.TargetActorTag = TEXT("ExplorationPoint");
    ExplorationMission.Objectives.Add(ExploreObjective);
    
    FQuest_MissionObjective SurviveObjective;
    SurviveObjective.ObjectiveDescription = TEXT("Survive the exploration");
    SurviveObjective.RequiredCount = 1;
    SurviveObjective.TargetActorTag = TEXT("PlayerSurvival");
    ExplorationMission.Objectives.Add(SurviveObjective);
    
    // Set rewards
    ExplorationMission.Reward.ExperiencePoints = 750;
    ExplorationMission.Reward.ResourcePoints = 300;
    ExplorationMission.Reward.ItemRewards.Add(TEXT("MapFragment"));
    ExplorationMission.Reward.UnlockableContent = TEXT("NewArea");
    
    return ExplorationMission;
}

void UQuest_MissionFramework::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.TimeLimit > 0.0f)
        {
            Mission.ElapsedTime += DeltaTime;
            
            // Check for mission timeout
            if (Mission.ElapsedTime >= Mission.TimeLimit)
            {
                FailMission(Mission.MissionID);
                break; // Array was modified, break to avoid issues
            }
        }
    }
}

void UQuest_MissionFramework::CheckMissionCompletion()
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        bool bAllObjectivesCompleted = true;
        
        for (const FQuest_MissionObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesCompleted = false;
                break;
            }
        }
        
        if (bAllObjectivesCompleted)
        {
            CompleteMission(Mission.MissionID);
            break; // Array was modified, break to avoid issues
        }
    }
}

void UQuest_MissionFramework::GenerateRandomMissions()
{
    if (AvailableMissions.Num() >= 10) // Don't generate too many missions
    {
        return;
    }
    
    // Generate random missions based on different types
    TArray<FVector> RandomLocations = {
        FVector(1000, 500, 100),
        FVector(-800, 1200, 150),
        FVector(1500, -600, 200),
        FVector(-500, -1000, 50)
    };
    
    TArray<FString> DinosaurSpecies = {
        TEXT("Raptor"),
        TEXT("Triceratops"),
        TEXT("Parasaurolophus"),
        TEXT("Compsognathus")
    };
    
    TArray<FString> ResourceTypes = {
        TEXT("Stone"),
        TEXT("Wood"),
        TEXT("Berries"),
        TEXT("Herbs")
    };
    
    // Generate a hunt mission
    FVector HuntLocation = RandomLocations[FMath::RandRange(0, RandomLocations.Num() - 1)];
    FString TargetSpecies = DinosaurSpecies[FMath::RandRange(0, DinosaurSpecies.Num() - 1)];
    FQuest_MissionData HuntMission = GenerateHuntMission(HuntLocation, TargetSpecies);
    CreateMission(HuntMission);
    
    // Generate a gather mission
    FVector GatherLocation = RandomLocations[FMath::RandRange(0, RandomLocations.Num() - 1)];
    FString ResourceType = ResourceTypes[FMath::RandRange(0, ResourceTypes.Num() - 1)];
    int32 Quantity = FMath::RandRange(3, 8);
    FQuest_MissionData GatherMission = GenerateGatherMission(GatherLocation, ResourceType, Quantity);
    CreateMission(GatherMission);
    
    // Generate an exploration mission
    FVector ExploreLocation = RandomLocations[FMath::RandRange(0, RandomLocations.Num() - 1)];
    float ExploreRadius = FMath::RandRange(1000.0f, 3000.0f);
    FQuest_MissionData ExplorationMission = GenerateExplorationMission(ExploreLocation, ExploreRadius);
    CreateMission(ExplorationMission);
    
    UE_LOG(LogTemp, Log, TEXT("Generated 3 random missions"));
}

FString UQuest_MissionFramework::GenerateUniqueMissionID()
{
    static int32 MissionCounter = 0;
    MissionCounter++;
    return FString::Printf(TEXT("MISSION_%04d"), MissionCounter);
}

bool UQuest_MissionFramework::ValidateMissionData(const FQuest_MissionData& MissionData)
{
    if (MissionData.MissionID.IsEmpty() || MissionData.MissionTitle.IsEmpty())
    {
        return false;
    }
    
    if (MissionData.Objectives.Num() == 0)
    {
        return false;
    }
    
    return true;
}
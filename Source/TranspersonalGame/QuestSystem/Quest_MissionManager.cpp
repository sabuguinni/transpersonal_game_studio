#include "Quest_MissionManager.h"
#include "Engine/Engine.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

UQuest_MissionManager::UQuest_MissionManager()
{
    // Initialize arrays
    AllMissions.Empty();
    ActiveMissions.Empty();
    CompletedMissions.Empty();
}

void UQuest_MissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Quest Mission Manager Initialized"));
    
    // Create initial test missions for development
    CreateTestMissions();
}

void UQuest_MissionManager::Deinitialize()
{
    ClearAllMissions();
    Super::Deinitialize();
}

void UQuest_MissionManager::StartMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission && !Mission->bIsActive)
    {
        Mission->bIsActive = true;
        ActiveMissions.Add(*Mission);
        
        // Activate all objectives
        for (UQuest_MissionObjective* Objective : Mission->Objectives)
        {
            if (Objective)
            {
                Objective->ActivateObjective();
            }
        }
        
        OnMissionStarted.Broadcast(MissionID);
        UE_LOG(LogTemp, Log, TEXT("Mission Started: %s - %s"), *MissionID, *Mission->Title);
    }
}

void UQuest_MissionManager::CompleteMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission && Mission->bIsActive)
    {
        Mission->bIsActive = false;
        Mission->bIsCompleted = true;
        
        // Remove from active missions
        ActiveMissions.RemoveAll([&MissionID](const FQuest_MissionData& M) {
            return M.MissionID == MissionID;
        });
        
        // Add to completed missions
        CompletedMissions.Add(*Mission);
        
        OnMissionCompleted.Broadcast(MissionID);
        UE_LOG(LogTemp, Log, TEXT("Mission Completed: %s - %s"), *MissionID, *Mission->Title);
    }
}

void UQuest_MissionManager::AbandonMission(const FString& MissionID)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission && Mission->bIsActive)
    {
        Mission->bIsActive = false;
        
        // Reset all objectives
        for (UQuest_MissionObjective* Objective : Mission->Objectives)
        {
            if (Objective)
            {
                Objective->ResetObjective();
            }
        }
        
        // Remove from active missions
        ActiveMissions.RemoveAll([&MissionID](const FQuest_MissionData& M) {
            return M.MissionID == MissionID;
        });
        
        UE_LOG(LogTemp, Log, TEXT("Mission Abandoned: %s"), *MissionID);
    }
}

bool UQuest_MissionManager::IsMissionActive(const FString& MissionID) const
{
    return ActiveMissions.ContainsByPredicate([&MissionID](const FQuest_MissionData& M) {
        return M.MissionID == MissionID;
    });
}

bool UQuest_MissionManager::IsMissionCompleted(const FString& MissionID) const
{
    return CompletedMissions.ContainsByPredicate([&MissionID](const FQuest_MissionData& M) {
        return M.MissionID == MissionID;
    });
}

TArray<FQuest_MissionData> UQuest_MissionManager::GetActiveMissions() const
{
    return ActiveMissions;
}

TArray<FQuest_MissionData> UQuest_MissionManager::GetCompletedMissions() const
{
    return CompletedMissions;
}

void UQuest_MissionManager::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 IncrementAmount)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission)
    {
        UQuest_MissionObjective* Objective = FindObjectiveByID(*Mission, ObjectiveID);
        if (Objective)
        {
            Objective->UpdateProgress(IncrementAmount);
            CheckMissionCompletion(*Mission);
        }
    }
}

void UQuest_MissionManager::CompleteObjective(const FString& MissionID, const FString& ObjectiveID)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission)
    {
        UQuest_MissionObjective* Objective = FindObjectiveByID(*Mission, ObjectiveID);
        if (Objective)
        {
            Objective->CompleteObjective();
            OnObjectiveCompleted.Broadcast(ObjectiveID);
            CheckMissionCompletion(*Mission);
        }
    }
}

void UQuest_MissionManager::SetObjectiveTargetLocation(const FString& MissionID, const FString& ObjectiveID, const FVector& Location, float Radius)
{
    FQuest_MissionData* Mission = FindMissionByID(MissionID);
    if (Mission)
    {
        UQuest_MissionObjective* Objective = FindObjectiveByID(*Mission, ObjectiveID);
        if (Objective)
        {
            Objective->SetTargetLocation(Location, Radius);
        }
    }
}

void UQuest_MissionManager::CreateHuntMission(const FString& DinosaurType, int32 TargetCount, const FVector& HuntArea)
{
    FQuest_MissionData NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.Title = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    NewMission.Description = FString::Printf(TEXT("Hunt and eliminate %d %s in the designated area. Use stealth and strategy to survive."), TargetCount, *DinosaurType);
    NewMission.bIsMainMission = false;
    NewMission.Priority = 2;
    NewMission.RewardDescription = TEXT("Meat, Hide, Bones");
    
    // Create hunt objective
    UQuest_MissionObjective* HuntObjective = NewObject<UQuest_MissionObjective>();
    HuntObjective->ObjectiveData.ObjectiveID = TEXT("hunt_target");
    HuntObjective->ObjectiveData.Title = FString::Printf(TEXT("Eliminate %s"), *DinosaurType);
    HuntObjective->ObjectiveData.Description = FString::Printf(TEXT("Hunt %d %s"), TargetCount, *DinosaurType);
    HuntObjective->ObjectiveData.ObjectiveType = EQuest_ObjectiveType::Hunt_Dinosaur;
    HuntObjective->ObjectiveData.TargetCount = TargetCount;
    HuntObjective->ObjectiveData.TargetTag = DinosaurType;
    HuntObjective->ObjectiveData.TargetLocation = HuntArea;
    HuntObjective->ObjectiveData.TargetRadius = 2000.0f;
    
    NewMission.Objectives.Add(HuntObjective);
    AllMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created Hunt Mission: %s"), *NewMission.Title);
}

void UQuest_MissionManager::CreateGatherMission(const FString& ResourceType, int32 TargetCount, float TimeLimit)
{
    FQuest_MissionData NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.Title = FString::Printf(TEXT("Gather %s"), *ResourceType);
    NewMission.Description = FString::Printf(TEXT("Collect %d %s for the tribe's survival needs."), TargetCount, *ResourceType);
    NewMission.bIsMainMission = false;
    NewMission.Priority = 1;
    NewMission.RewardDescription = TEXT("Survival Points");
    
    // Create gather objective
    UQuest_MissionObjective* GatherObjective = NewObject<UQuest_MissionObjective>();
    GatherObjective->ObjectiveData.ObjectiveID = TEXT("gather_resource");
    GatherObjective->ObjectiveData.Title = FString::Printf(TEXT("Collect %s"), *ResourceType);
    GatherObjective->ObjectiveData.Description = FString::Printf(TEXT("Find and gather %d %s"), TargetCount, *ResourceType);
    GatherObjective->ObjectiveData.ObjectiveType = EQuest_ObjectiveType::Gather_Resources;
    GatherObjective->ObjectiveData.TargetCount = TargetCount;
    GatherObjective->ObjectiveData.TargetTag = ResourceType;
    GatherObjective->ObjectiveData.TimeLimit = TimeLimit;
    
    NewMission.Objectives.Add(GatherObjective);
    AllMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created Gather Mission: %s"), *NewMission.Title);
}

void UQuest_MissionManager::CreateExplorationMission(const FVector& TargetLocation, float ExploreRadius)
{
    FQuest_MissionData NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.Title = TEXT("Explore Unknown Territory");
    NewMission.Description = TEXT("Venture into uncharted lands to discover new resources and map the area for the tribe.");
    NewMission.bIsMainMission = false;
    NewMission.Priority = 2;
    NewMission.RewardDescription = TEXT("Map Knowledge, Discovery Points");
    
    // Create exploration objective
    UQuest_MissionObjective* ExploreObjective = NewObject<UQuest_MissionObjective>();
    ExploreObjective->ObjectiveData.ObjectiveID = TEXT("explore_area");
    ExploreObjective->ObjectiveData.Title = TEXT("Reach Exploration Point");
    ExploreObjective->ObjectiveData.Description = TEXT("Travel to the marked location and explore the area");
    ExploreObjective->ObjectiveData.ObjectiveType = EQuest_ObjectiveType::Explore_Area;
    ExploreObjective->ObjectiveData.TargetCount = 1;
    ExploreObjective->ObjectiveData.TargetLocation = TargetLocation;
    ExploreObjective->ObjectiveData.TargetRadius = ExploreRadius;
    
    NewMission.Objectives.Add(ExploreObjective);
    AllMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created Exploration Mission at: %s"), *TargetLocation.ToString());
}

void UQuest_MissionManager::CreateSurvivalMission(float SurvivalTime, const FVector& SafeZone)
{
    FQuest_MissionData NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.Title = TEXT("Survival Challenge");
    NewMission.Description = FString::Printf(TEXT("Survive for %.0f minutes in the wilderness while maintaining health and avoiding predators."), SurvivalTime / 60.0f);
    NewMission.bIsMainMission = true;
    NewMission.Priority = 3;
    NewMission.RewardDescription = TEXT("Survival Mastery, Tribe Respect");
    
    // Create survival objective
    UQuest_MissionObjective* SurviveObjective = NewObject<UQuest_MissionObjective>();
    SurviveObjective->ObjectiveData.ObjectiveID = TEXT("survive_time");
    SurviveObjective->ObjectiveData.Title = TEXT("Survive the Challenge");
    SurviveObjective->ObjectiveData.Description = FString::Printf(TEXT("Stay alive for %.0f minutes"), SurvivalTime / 60.0f);
    SurviveObjective->ObjectiveData.ObjectiveType = EQuest_ObjectiveType::Survive_Time;
    SurviveObjective->ObjectiveData.TargetCount = 1;
    SurviveObjective->ObjectiveData.TimeLimit = SurvivalTime;
    SurviveObjective->ObjectiveData.TargetLocation = SafeZone;
    SurviveObjective->ObjectiveData.TargetRadius = 1000.0f;
    
    NewMission.Objectives.Add(SurviveObjective);
    AllMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created Survival Mission: %.0f minutes"), SurvivalTime / 60.0f);
}

void UQuest_MissionManager::CreateDefenseMission(const FVector& DefensePoint, float DefenseRadius, float Duration)
{
    FQuest_MissionData NewMission;
    NewMission.MissionID = GenerateUniqueMissionID();
    NewMission.Title = TEXT("Defend the Settlement");
    NewMission.Description = FString::Printf(TEXT("Protect the tribe's settlement from predator attacks for %.0f minutes."), Duration / 60.0f);
    NewMission.bIsMainMission = true;
    NewMission.Priority = 4;
    NewMission.RewardDescription = TEXT("Tribal Leadership, Defense Mastery");
    
    // Create defense objective
    UQuest_MissionObjective* DefendObjective = NewObject<UQuest_MissionObjective>();
    DefendObjective->ObjectiveData.ObjectiveID = TEXT("defend_area");
    DefendObjective->ObjectiveData.Title = TEXT("Hold the Defense Line");
    DefendObjective->ObjectiveData.Description = TEXT("Prevent predators from reaching the settlement");
    DefendObjective->ObjectiveData.ObjectiveType = EQuest_ObjectiveType::Defend_Area;
    DefendObjective->ObjectiveData.TargetCount = 1;
    DefendObjective->ObjectiveData.TimeLimit = Duration;
    DefendObjective->ObjectiveData.TargetLocation = DefensePoint;
    DefendObjective->ObjectiveData.TargetRadius = DefenseRadius;
    
    NewMission.Objectives.Add(DefendObjective);
    AllMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created Defense Mission at: %s"), *DefensePoint.ToString());
}

void UQuest_MissionManager::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (UQuest_MissionObjective* Objective : Mission.Objectives)
        {
            if (Objective && Objective->IsActive())
            {
                Objective->UpdateTimer(DeltaTime);
            }
        }
        CheckMissionCompletion(Mission);
    }
}

void UQuest_MissionManager::CheckPlayerLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (UQuest_MissionObjective* Objective : Mission.Objectives)
        {
            if (Objective && Objective->IsActive())
            {
                if (Objective->ObjectiveData.ObjectiveType == EQuest_ObjectiveType::Explore_Area ||
                    Objective->ObjectiveData.ObjectiveType == EQuest_ObjectiveType::Reach_Location)
                {
                    if (Objective->IsPlayerInTargetArea(PlayerLocation))
                    {
                        Objective->UpdateProgress(1);
                    }
                }
            }
        }
    }
}

void UQuest_MissionManager::CreateTestMissions()
{
    // Create sample survival missions for testing
    CreateHuntMission(TEXT("Velociraptor"), 3, FVector(5000, 0, 0));
    CreateGatherMission(TEXT("Berries"), 10, 300.0f);
    CreateExplorationMission(FVector(8000, 8000, 500), 1000.0f);
    CreateSurvivalMission(600.0f, FVector(0, 0, 0));
    CreateDefenseMission(FVector(2000, 2000, 0), 1500.0f, 480.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Created %d test missions"), AllMissions.Num());
}

void UQuest_MissionManager::ClearAllMissions()
{
    AllMissions.Empty();
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("All missions cleared"));
}

void UQuest_MissionManager::CheckMissionCompletion(FQuest_MissionData& Mission)
{
    if (!Mission.bIsActive) return;
    
    bool bAllObjectivesCompleted = true;
    for (UQuest_MissionObjective* Objective : Mission.Objectives)
    {
        if (Objective && !Objective->ObjectiveData.bIsOptional && !Objective->IsCompleted())
        {
            bAllObjectivesCompleted = false;
            break;
        }
    }
    
    if (bAllObjectivesCompleted)
    {
        CompleteMission(Mission.MissionID);
    }
}

FQuest_MissionData* UQuest_MissionManager::FindMissionByID(const FString& MissionID)
{
    return AllMissions.FindByPredicate([&MissionID](const FQuest_MissionData& M) {
        return M.MissionID == MissionID;
    });
}

UQuest_MissionObjective* UQuest_MissionManager::FindObjectiveByID(FQuest_MissionData& Mission, const FString& ObjectiveID)
{
    for (UQuest_MissionObjective* Objective : Mission.Objectives)
    {
        if (Objective && Objective->ObjectiveData.ObjectiveID == ObjectiveID)
        {
            return Objective;
        }
    }
    return nullptr;
}

FString UQuest_MissionManager::GenerateUniqueMissionID()
{
    return FString::Printf(TEXT("MISSION_%d_%d"), 
        FMath::RandRange(1000, 9999), 
        FDateTime::Now().ToUnixTimestamp());
}
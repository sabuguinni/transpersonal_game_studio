#include "Quest_MissionManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

void UQuest_MissionManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Mission Manager Initialized"));
    
    // Initialize default missions
    InitializeDefaultMissions();
}

void UQuest_MissionManager::Deinitialize()
{
    Missions.Empty();
    Objectives.Empty();
    ActiveMissionIDs.Empty();
    
    Super::Deinitialize();
}

void UQuest_MissionManager::StartMission(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        Mission->bIsActive = true;
        Mission->bIsCompleted = false;
        
        if (!ActiveMissionIDs.Contains(MissionID))
        {
            ActiveMissionIDs.Add(MissionID);
        }
        
        // Activate all objectives for this mission
        for (const FString& ObjectiveID : Mission->ObjectiveIDs)
        {
            if (FQuest_ObjectiveData* Objective = Objectives.Find(ObjectiveID))
            {
                Objective->CurrentCount = 0;
                Objective->bIsCompleted = false;
            }
        }
        
        BroadcastMissionUpdate(MissionID);
        UE_LOG(LogTemp, Warning, TEXT("Mission Started: %s"), *Mission->MissionName);
    }
}

void UQuest_MissionManager::CompleteMission(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        Mission->bIsActive = false;
        Mission->bIsCompleted = true;
        
        ActiveMissionIDs.Remove(MissionID);
        
        BroadcastMissionUpdate(MissionID);
        UE_LOG(LogTemp, Warning, TEXT("Mission Completed: %s - Reward: %.1f XP"), 
               *Mission->MissionName, Mission->RewardExperience);
    }
}

void UQuest_MissionManager::FailMission(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        Mission->bIsActive = false;
        Mission->bIsCompleted = false;
        
        ActiveMissionIDs.Remove(MissionID);
        
        BroadcastMissionUpdate(MissionID);
        UE_LOG(LogTemp, Warning, TEXT("Mission Failed: %s"), *Mission->MissionName);
    }
}

bool UQuest_MissionManager::IsMissionActive(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        return Mission->bIsActive;
    }
    return false;
}

bool UQuest_MissionManager::IsMissionCompleted(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        return Mission->bIsCompleted;
    }
    return false;
}

void UQuest_MissionManager::UpdateObjectiveProgress(const FString& ObjectiveID, int32 Progress)
{
    if (FQuest_ObjectiveData* Objective = Objectives.Find(ObjectiveID))
    {
        Objective->CurrentCount = FMath::Min(Objective->CurrentCount + Progress, Objective->RequiredCount);
        
        if (Objective->CurrentCount >= Objective->RequiredCount && !Objective->bIsCompleted)
        {
            CompleteObjective(ObjectiveID);
        }
        
        BroadcastObjectiveUpdate(ObjectiveID);
    }
}

void UQuest_MissionManager::CompleteObjective(const FString& ObjectiveID)
{
    if (FQuest_ObjectiveData* Objective = Objectives.Find(ObjectiveID))
    {
        Objective->bIsCompleted = true;
        Objective->CurrentCount = Objective->RequiredCount;
        
        BroadcastObjectiveUpdate(ObjectiveID);
        UE_LOG(LogTemp, Warning, TEXT("Objective Completed: %s"), *Objective->ObjectiveName);
        
        // Check if all objectives in parent mission are completed
        for (const auto& MissionPair : Missions)
        {
            const FQuest_MissionData& Mission = MissionPair.Value;
            if (Mission.ObjectiveIDs.Contains(ObjectiveID) && Mission.bIsActive)
            {
                bool bAllObjectivesCompleted = true;
                for (const FString& ObjID : Mission.ObjectiveIDs)
                {
                    if (const FQuest_ObjectiveData* Obj = Objectives.Find(ObjID))
                    {
                        if (!Obj->bIsCompleted)
                        {
                            bAllObjectivesCompleted = false;
                            break;
                        }
                    }
                }
                
                if (bAllObjectivesCompleted)
                {
                    CompleteMission(MissionPair.Key);
                }
                break;
            }
        }
    }
}

bool UQuest_MissionManager::IsObjectiveCompleted(const FString& ObjectiveID) const
{
    if (const FQuest_ObjectiveData* Objective = Objectives.Find(ObjectiveID))
    {
        return Objective->bIsCompleted;
    }
    return false;
}

void UQuest_MissionManager::CreateHuntMission(const FString& DinosaurSpecies, int32 RequiredKills)
{
    FString MissionID = GenerateUniqueID(TEXT("HUNT"));
    FString ObjectiveID = GenerateUniqueID(TEXT("OBJ_HUNT"));
    
    // Create objective
    FQuest_ObjectiveData HuntObjective;
    HuntObjective.ObjectiveID = ObjectiveID;
    HuntObjective.ObjectiveName = FString::Printf(TEXT("Hunt %s"), *DinosaurSpecies);
    HuntObjective.Description = FString::Printf(TEXT("Track and kill %d %s"), RequiredKills, *DinosaurSpecies);
    HuntObjective.QuestType = EQuestType::Hunt;
    HuntObjective.TargetID = DinosaurSpecies;
    HuntObjective.RequiredCount = RequiredKills;
    HuntObjective.CurrentCount = 0;
    HuntObjective.bIsCompleted = false;
    
    Objectives.Add(ObjectiveID, HuntObjective);
    
    // Create mission
    FQuest_MissionData HuntMission;
    HuntMission.MissionID = MissionID;
    HuntMission.MissionName = FString::Printf(TEXT("The %s Hunt"), *DinosaurSpecies);
    HuntMission.Description = FString::Printf(TEXT("A dangerous %s has been spotted in the area. Hunt it down to protect the tribe."), *DinosaurSpecies);
    HuntMission.Difficulty = RequiredKills > 3 ? ESurvivalDifficulty::Hard : ESurvivalDifficulty::Medium;
    HuntMission.ObjectiveIDs.Add(ObjectiveID);
    HuntMission.RewardExperience = RequiredKills * 50.0f;
    HuntMission.RewardItems.Add(TEXT("Meat"));
    HuntMission.RewardItems.Add(TEXT("Hide"));
    
    Missions.Add(MissionID, HuntMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Hunt Mission: %s"), *HuntMission.MissionName);
}

void UQuest_MissionManager::CreateGatherMission(const FString& ResourceType, int32 RequiredAmount)
{
    FString MissionID = GenerateUniqueID(TEXT("GATHER"));
    FString ObjectiveID = GenerateUniqueID(TEXT("OBJ_GATHER"));
    
    // Create objective
    FQuest_ObjectiveData GatherObjective;
    GatherObjective.ObjectiveID = ObjectiveID;
    GatherObjective.ObjectiveName = FString::Printf(TEXT("Gather %s"), *ResourceType);
    GatherObjective.Description = FString::Printf(TEXT("Collect %d %s from the environment"), RequiredAmount, *ResourceType);
    GatherObjective.QuestType = EQuestType::Gather;
    GatherObjective.TargetID = ResourceType;
    GatherObjective.RequiredCount = RequiredAmount;
    GatherObjective.CurrentCount = 0;
    GatherObjective.bIsCompleted = false;
    
    Objectives.Add(ObjectiveID, GatherObjective);
    
    // Create mission
    FQuest_MissionData GatherMission;
    GatherMission.MissionID = MissionID;
    GatherMission.MissionName = FString::Printf(TEXT("Gathering %s"), *ResourceType);
    GatherMission.Description = FString::Printf(TEXT("The tribe needs %s. Search the area and gather what we need."), *ResourceType);
    GatherMission.Difficulty = RequiredAmount > 10 ? ESurvivalDifficulty::Medium : ESurvivalDifficulty::Easy;
    GatherMission.ObjectiveIDs.Add(ObjectiveID);
    GatherMission.RewardExperience = RequiredAmount * 10.0f;
    GatherMission.RewardItems.Add(ResourceType);
    
    Missions.Add(MissionID, GatherMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Gather Mission: %s"), *GatherMission.MissionName);
}

void UQuest_MissionManager::CreateExploreMission(const FVector& TargetLocation, float ExploreRadius)
{
    FString MissionID = GenerateUniqueID(TEXT("EXPLORE"));
    FString ObjectiveID = GenerateUniqueID(TEXT("OBJ_EXPLORE"));
    
    // Create objective
    FQuest_ObjectiveData ExploreObjective;
    ExploreObjective.ObjectiveID = ObjectiveID;
    ExploreObjective.ObjectiveName = TEXT("Explore Unknown Territory");
    ExploreObjective.Description = TEXT("Venture into unexplored lands and discover what lies beyond");
    ExploreObjective.QuestType = EQuestType::Explore;
    ExploreObjective.TargetID = TEXT("ExploreArea");
    ExploreObjective.RequiredCount = 1;
    ExploreObjective.CurrentCount = 0;
    ExploreObjective.bIsCompleted = false;
    ExploreObjective.TargetLocation = TargetLocation;
    
    Objectives.Add(ObjectiveID, ExploreObjective);
    
    // Create mission
    FQuest_MissionData ExploreMission;
    ExploreMission.MissionID = MissionID;
    ExploreMission.MissionName = TEXT("Into the Unknown");
    ExploreMission.Description = TEXT("Strange sounds have been heard from the distant lands. Explore the area and report back what you find.");
    ExploreMission.Difficulty = ESurvivalDifficulty::Medium;
    ExploreMission.ObjectiveIDs.Add(ObjectiveID);
    ExploreMission.RewardExperience = 100.0f;
    ExploreMission.RewardItems.Add(TEXT("Map Fragment"));
    
    Missions.Add(MissionID, ExploreMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Explore Mission: %s"), *ExploreMission.MissionName);
}

void UQuest_MissionManager::CreateSurvivalMission(float Duration, ESurvivalDifficulty Difficulty)
{
    FString MissionID = GenerateUniqueID(TEXT("SURVIVE"));
    FString ObjectiveID = GenerateUniqueID(TEXT("OBJ_SURVIVE"));
    
    // Create objective
    FQuest_ObjectiveData SurviveObjective;
    SurviveObjective.ObjectiveID = ObjectiveID;
    SurviveObjective.ObjectiveName = TEXT("Survive the Trial");
    SurviveObjective.Description = FString::Printf(TEXT("Survive for %.0f minutes in harsh conditions"), Duration);
    SurviveObjective.QuestType = EQuestType::Survive;
    SurviveObjective.TargetID = TEXT("SurvivalTimer");
    SurviveObjective.RequiredCount = FMath::RoundToInt(Duration);
    SurviveObjective.CurrentCount = 0;
    SurviveObjective.bIsCompleted = false;
    
    Objectives.Add(ObjectiveID, SurviveObjective);
    
    // Create mission
    FQuest_MissionData SurviveMission;
    SurviveMission.MissionID = MissionID;
    SurviveMission.MissionName = TEXT("Trial of Endurance");
    SurviveMission.Description = TEXT("Prove your worth by surviving in the wilderness. Only the strong will endure.");
    SurviveMission.Difficulty = Difficulty;
    SurviveMission.ObjectiveIDs.Add(ObjectiveID);
    SurviveMission.RewardExperience = Duration * 5.0f;
    SurviveMission.RewardItems.Add(TEXT("Survival Badge"));
    
    Missions.Add(MissionID, SurviveMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Created Survival Mission: %s"), *SurviveMission.MissionName);
}

TArray<FQuest_MissionData> UQuest_MissionManager::GetActiveMissions() const
{
    TArray<FQuest_MissionData> ActiveMissions;
    
    for (const FString& MissionID : ActiveMissionIDs)
    {
        if (const FQuest_MissionData* Mission = Missions.Find(MissionID))
        {
            ActiveMissions.Add(*Mission);
        }
    }
    
    return ActiveMissions;
}

FQuest_MissionData UQuest_MissionManager::GetMissionData(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        return *Mission;
    }
    return FQuest_MissionData();
}

TArray<FQuest_ObjectiveData> UQuest_MissionManager::GetMissionObjectives(const FString& MissionID) const
{
    TArray<FQuest_ObjectiveData> MissionObjectives;
    
    if (const FQuest_MissionData* Mission = Missions.Find(MissionID))
    {
        for (const FString& ObjectiveID : Mission->ObjectiveIDs)
        {
            if (const FQuest_ObjectiveData* Objective = Objectives.Find(ObjectiveID))
            {
                MissionObjectives.Add(*Objective);
            }
        }
    }
    
    return MissionObjectives;
}

void UQuest_MissionManager::InitializeDefaultMissions()
{
    CreateDefaultHuntMissions();
    CreateDefaultGatherMissions();
    CreateDefaultExploreMissions();
}

void UQuest_MissionManager::CreateDefaultHuntMissions()
{
    CreateHuntMission(TEXT("Raptor"), 3);
    CreateHuntMission(TEXT("Triceratops"), 1);
    CreateHuntMission(TEXT("Parasaurolophus"), 2);
}

void UQuest_MissionManager::CreateDefaultGatherMissions()
{
    CreateGatherMission(TEXT("Stone"), 15);
    CreateGatherMission(TEXT("Wood"), 10);
    CreateGatherMission(TEXT("Berries"), 20);
    CreateGatherMission(TEXT("Fiber"), 25);
}

void UQuest_MissionManager::CreateDefaultExploreMissions()
{
    CreateExploreMission(FVector(5000, 0, 200), 1000.0f);
    CreateExploreMission(FVector(-3000, 4000, 300), 1500.0f);
    CreateExploreMission(FVector(0, -5000, 100), 800.0f);
}

FString UQuest_MissionManager::GenerateUniqueID(const FString& Prefix) const
{
    return FString::Printf(TEXT("%s_%d"), *Prefix, FMath::RandRange(1000, 9999));
}

void UQuest_MissionManager::BroadcastMissionUpdate(const FString& MissionID)
{
    // TODO: Implement mission update broadcasting to UI/HUD
    UE_LOG(LogTemp, Log, TEXT("Mission Update Broadcast: %s"), *MissionID);
}

void UQuest_MissionManager::BroadcastObjectiveUpdate(const FString& ObjectiveID)
{
    // TODO: Implement objective update broadcasting to UI/HUD
    UE_LOG(LogTemp, Log, TEXT("Objective Update Broadcast: %s"), *ObjectiveID);
}
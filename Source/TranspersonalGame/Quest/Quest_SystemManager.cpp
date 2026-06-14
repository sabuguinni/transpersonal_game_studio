#include "Quest_SystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UQuest_SystemManager::UQuest_SystemManager()
{
    MaxActiveMissions = 10;
    QuestUpdateInterval = 1.0f;
    NextMissionIDCounter = 1000;
}

void UQuest_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest System Manager Initialized"));
    
    // Initialize default mission templates
    AvailableMissionTemplates.Add(TEXT("HuntMammoth"));
    AvailableMissionTemplates.Add(TEXT("GatherBerries"));
    AvailableMissionTemplates.Add(TEXT("ExploreCave"));
    AvailableMissionTemplates.Add(TEXT("DefendCamp"));
    AvailableMissionTemplates.Add(TEXT("RescueTribalMember"));
    
    // Start quest update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(QuestUpdateTimer, [this]()
        {
            CleanupExpiredMissions();
        }, QuestUpdateInterval, true);
    }
}

void UQuest_SystemManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(QuestUpdateTimer);
    }
    
    Super::Deinitialize();
}

void UQuest_SystemManager::CreateMission(const FString& MissionID, const FString& Title, const FString& Description, EQuestType Type, EQuestDifficulty Difficulty)
{
    if (ActiveMissions.Contains(MissionID))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mission %s already exists"), *MissionID);
        return;
    }
    
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum active missions reached (%d)"), MaxActiveMissions);
        return;
    }
    
    FQuest_MissionData NewMission;
    NewMission.MissionID = MissionID;
    NewMission.Title = Title;
    NewMission.Description = Description;
    NewMission.MissionType = Type;
    NewMission.Difficulty = Difficulty;
    NewMission.bIsActive = false;
    NewMission.bIsCompleted = false;
    
    // Set default time limits based on difficulty
    switch (Difficulty)
    {
        case EQuestDifficulty::Easy:
            NewMission.TimeLimit = 300.0f; // 5 minutes
            break;
        case EQuestDifficulty::Medium:
            NewMission.TimeLimit = 600.0f; // 10 minutes
            break;
        case EQuestDifficulty::Hard:
            NewMission.TimeLimit = 900.0f; // 15 minutes
            break;
        case EQuestDifficulty::Extreme:
            NewMission.TimeLimit = 1200.0f; // 20 minutes
            break;
    }
    
    ActiveMissions.Add(MissionID, NewMission);
    
    UE_LOG(LogTemp, Log, TEXT("Created mission: %s - %s"), *MissionID, *Title);
}

void UQuest_SystemManager::ActivateMission(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->bIsActive = true;
        UE_LOG(LogTemp, Log, TEXT("Activated mission: %s"), *MissionID);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot activate mission %s - not found"), *MissionID);
    }
}

void UQuest_SystemManager::CompleteMission(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->bIsCompleted = true;
        Mission->bIsActive = false;
        
        // Move to completed missions
        CompletedMissions.Add(MissionID, *Mission);
        ActiveMissions.Remove(MissionID);
        
        UE_LOG(LogTemp, Log, TEXT("Completed mission: %s"), *MissionID);
    }
}

bool UQuest_SystemManager::IsMissionActive(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        return Mission->bIsActive;
    }
    return false;
}

bool UQuest_SystemManager::IsMissionCompleted(const FString& MissionID) const
{
    return CompletedMissions.Contains(MissionID);
}

void UQuest_SystemManager::AddObjectiveToMission(const FString& MissionID, const FString& ObjectiveID, const FString& Description, EQuestType Type, int32 TargetCount)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        FQuest_ObjectiveData NewObjective;
        NewObjective.ObjectiveID = ObjectiveID;
        NewObjective.Description = Description;
        NewObjective.QuestType = Type;
        NewObjective.TargetCount = TargetCount;
        NewObjective.CurrentCount = 0;
        NewObjective.bIsCompleted = false;
        
        Mission->Objectives.Add(NewObjective);
        
        UE_LOG(LogTemp, Log, TEXT("Added objective %s to mission %s"), *ObjectiveID, *MissionID);
    }
}

void UQuest_SystemManager::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 Progress)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        for (FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.CurrentCount = FMath::Min(Objective.CurrentCount + Progress, Objective.TargetCount);
                
                if (Objective.CurrentCount >= Objective.TargetCount)
                {
                    Objective.bIsCompleted = true;
                    UE_LOG(LogTemp, Log, TEXT("Objective %s completed"), *ObjectiveID);
                }
                
                CheckMissionCompletion(MissionID);
                break;
            }
        }
    }
}

void UQuest_SystemManager::CompleteObjective(const FString& MissionID, const FString& ObjectiveID)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        for (FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.bIsCompleted = true;
                Objective.CurrentCount = Objective.TargetCount;
                UE_LOG(LogTemp, Log, TEXT("Manually completed objective %s"), *ObjectiveID);
                CheckMissionCompletion(MissionID);
                break;
            }
        }
    }
}

bool UQuest_SystemManager::IsObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        for (const FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                return Objective.bIsCompleted;
            }
        }
    }
    return false;
}

void UQuest_SystemManager::SetObjectiveLocation(const FString& MissionID, const FString& ObjectiveID, const FVector& Location, float Radius)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        for (FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                Objective.TargetLocation = Location;
                Objective.CompletionRadius = Radius;
                break;
            }
        }
    }
}

bool UQuest_SystemManager::CheckLocationObjective(const FString& MissionID, const FString& ObjectiveID, const FVector& PlayerLocation) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        for (const FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (Objective.ObjectiveID == ObjectiveID)
            {
                float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                return Distance <= Objective.CompletionRadius;
            }
        }
    }
    return false;
}

void UQuest_SystemManager::CreateCrowdBasedMission(const FString& MissionID, const FString& Title, const FVector& CrowdArea, float Radius)
{
    CreateMission(MissionID, Title, TEXT("Mission based on crowd simulation behavior"), EQuestType::Exploration, EQuestDifficulty::Medium);
    
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->MissionArea = CrowdArea;
        Mission->MissionRadius = Radius;
        
        // Add crowd-specific objectives
        AddObjectiveToMission(MissionID, TEXT("ObserveCrowd"), TEXT("Observe crowd behavior in the area"), EQuestType::Exploration, 1);
        SetObjectiveLocation(MissionID, TEXT("ObserveCrowd"), CrowdArea, Radius);
    }
}

void UQuest_SystemManager::UpdateCrowdMissionProgress(const FString& MissionID, int32 CrowdDensity)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        // Update progress based on crowd density
        if (CrowdDensity > 50)
        {
            UpdateObjectiveProgress(MissionID, TEXT("ObserveCrowd"), 1);
        }
    }
}

void UQuest_SystemManager::GenerateHuntMission(const FString& TargetSpecies, const FVector& HuntingGrounds, int32 TargetCount)
{
    FString MissionID = FString::Printf(TEXT("Hunt_%s_%d"), *TargetSpecies, NextMissionIDCounter++);
    FString Title = FString::Printf(TEXT("Hunt %s"), *TargetSpecies);
    FString Description = FString::Printf(TEXT("Track and hunt %d %s in the designated hunting grounds"), TargetCount, *TargetSpecies);
    
    CreateMission(MissionID, Title, Description, EQuestType::Hunt, EQuestDifficulty::Medium);
    
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->MissionArea = HuntingGrounds;
        Mission->MissionRadius = 1500.0f;
        
        AddObjectiveToMission(MissionID, TEXT("TrackPrey"), FString::Printf(TEXT("Track %s"), *TargetSpecies), EQuestType::Hunt, 1);
        AddObjectiveToMission(MissionID, TEXT("HuntTarget"), FString::Printf(TEXT("Hunt %d %s"), TargetCount, *TargetSpecies), EQuestType::Hunt, TargetCount);
        
        SetObjectiveLocation(MissionID, TEXT("TrackPrey"), HuntingGrounds, 1500.0f);
        SetObjectiveLocation(MissionID, TEXT("HuntTarget"), HuntingGrounds, 1500.0f);
    }
}

void UQuest_SystemManager::GenerateGatherMission(const FString& ResourceType, const FVector& GatheringArea, int32 ResourceCount)
{
    FString MissionID = FString::Printf(TEXT("Gather_%s_%d"), *ResourceType, NextMissionIDCounter++);
    FString Title = FString::Printf(TEXT("Gather %s"), *ResourceType);
    FString Description = FString::Printf(TEXT("Collect %d %s from the gathering area"), ResourceCount, *ResourceType);
    
    CreateMission(MissionID, Title, Description, EQuestType::Gather, EQuestDifficulty::Easy);
    
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->MissionArea = GatheringArea;
        Mission->MissionRadius = 1000.0f;
        
        AddObjectiveToMission(MissionID, TEXT("FindResources"), FString::Printf(TEXT("Locate %s"), *ResourceType), EQuestType::Gather, 1);
        AddObjectiveToMission(MissionID, TEXT("CollectResources"), FString::Printf(TEXT("Collect %d %s"), ResourceCount, *ResourceType), EQuestType::Gather, ResourceCount);
        
        SetObjectiveLocation(MissionID, TEXT("FindResources"), GatheringArea, 1000.0f);
        SetObjectiveLocation(MissionID, TEXT("CollectResources"), GatheringArea, 1000.0f);
    }
}

void UQuest_SystemManager::GenerateExplorationMission(const FVector& ExploreLocation, float ExploreRadius)
{
    FString MissionID = FString::Printf(TEXT("Explore_%d"), NextMissionIDCounter++);
    FString Title = TEXT("Explore Unknown Territory");
    FString Description = TEXT("Venture into unexplored lands and discover what lies beyond");
    
    CreateMission(MissionID, Title, Description, EQuestType::Exploration, EQuestDifficulty::Hard);
    
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        Mission->MissionArea = ExploreLocation;
        Mission->MissionRadius = ExploreRadius;
        
        AddObjectiveToMission(MissionID, TEXT("ReachLocation"), TEXT("Reach the exploration target"), EQuestType::Exploration, 1);
        AddObjectiveToMission(MissionID, TEXT("SurveyArea"), TEXT("Survey the area for points of interest"), EQuestType::Exploration, 3);
        AddObjectiveToMission(MissionID, TEXT("ReturnSafely"), TEXT("Return to base camp safely"), EQuestType::Exploration, 1);
        
        SetObjectiveLocation(MissionID, TEXT("ReachLocation"), ExploreLocation, ExploreRadius);
    }
}

TArray<FQuest_MissionData> UQuest_SystemManager::GetActiveMissions() const
{
    TArray<FQuest_MissionData> Missions;
    for (const auto& MissionPair : ActiveMissions)
    {
        if (MissionPair.Value.bIsActive)
        {
            Missions.Add(MissionPair.Value);
        }
    }
    return Missions;
}

FQuest_MissionData UQuest_SystemManager::GetMissionData(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        return *Mission;
    }
    return FQuest_MissionData();
}

TArray<FQuest_ObjectiveData> UQuest_SystemManager::GetMissionObjectives(const FString& MissionID) const
{
    if (const FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        return Mission->Objectives;
    }
    return TArray<FQuest_ObjectiveData>();
}

void UQuest_SystemManager::SaveQuestProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Quest progress saved"));
}

void UQuest_SystemManager::LoadQuestProgress()
{
    // TODO: Implement save system integration
    UE_LOG(LogTemp, Log, TEXT("Quest progress loaded"));
}

void UQuest_SystemManager::ResetAllQuests()
{
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    NextMissionIDCounter = 1000;
    
    UE_LOG(LogTemp, Log, TEXT("All quests reset"));
}

void UQuest_SystemManager::ValidateMissionObjectives(FQuest_MissionData& Mission)
{
    for (FQuest_ObjectiveData& Objective : Mission.Objectives)
    {
        if (Objective.CurrentCount >= Objective.TargetCount)
        {
            Objective.bIsCompleted = true;
        }
    }
}

void UQuest_SystemManager::CheckMissionCompletion(const FString& MissionID)
{
    if (FQuest_MissionData* Mission = ActiveMissions.Find(MissionID))
    {
        bool bAllObjectivesCompleted = true;
        
        for (const FQuest_ObjectiveData& Objective : Mission->Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesCompleted = false;
                break;
            }
        }
        
        if (bAllObjectivesCompleted && !Mission->bIsCompleted)
        {
            CompleteMission(MissionID);
        }
    }
}

void UQuest_SystemManager::GenerateRandomMissionID()
{
    NextMissionIDCounter++;
}

void UQuest_SystemManager::CleanupExpiredMissions()
{
    TArray<FString> ExpiredMissions;
    
    for (const auto& MissionPair : ActiveMissions)
    {
        const FQuest_MissionData& Mission = MissionPair.Value;
        if (Mission.TimeLimit > 0.0f && Mission.bIsActive)
        {
            // Check if mission has expired (simplified check)
            // In a real implementation, you'd track mission start time
            // ExpiredMissions.Add(MissionPair.Key);
        }
    }
    
    for (const FString& MissionID : ExpiredMissions)
    {
        ActiveMissions.Remove(MissionID);
        UE_LOG(LogTemp, Log, TEXT("Removed expired mission: %s"), *MissionID);
    }
}
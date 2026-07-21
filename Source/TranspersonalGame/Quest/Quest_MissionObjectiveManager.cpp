#include "Quest_MissionObjectiveManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UQuest_MissionObjectiveManager::UQuest_MissionObjectiveManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MissionDataTable = nullptr;
}

void UQuest_MissionObjectiveManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMissions();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest Mission Objective Manager initialized with %d default missions"), ActiveMissions.Num());
}

void UQuest_MissionObjectiveManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Check location-based objectives
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                CheckLocationObjectives(PlayerPawn->GetActorLocation());
            }
        }
    }
}

bool UQuest_MissionObjectiveManager::StartMission(const FString& MissionID)
{
    // Check if mission is already active
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission %s is already active"), *MissionID);
            return false;
        }
    }
    
    // Find mission in default missions or data table
    FQuest_MissionData MissionToStart;
    bool bFoundMission = false;
    
    // Create default missions if not found
    if (MissionID == "HUNT_FIRST_RAPTOR")
    {
        MissionToStart.MissionID = "HUNT_FIRST_RAPTOR";
        MissionToStart.MissionName = "First Hunt";
        MissionToStart.MissionDescription = "Hunt your first Velociraptor to prove your survival skills";
        MissionToStart.bIsMainQuest = true;
        MissionToStart.ExperienceReward = 200;
        
        FQuest_ObjectiveData HuntObjective;
        HuntObjective.ObjectiveID = "KILL_RAPTOR_1";
        HuntObjective.Title = "Hunt Velociraptor";
        HuntObjective.Description = "Find and kill 1 Velociraptor";
        HuntObjective.Type = EQuest_ObjectiveType::Hunt_Target;
        HuntObjective.Status = EQuest_ObjectiveStatus::Active;
        HuntObjective.TargetCount = 1;
        HuntObjective.CurrentCount = 0;
        
        MissionToStart.Objectives.Add(HuntObjective);
        bFoundMission = true;
    }
    else if (MissionID == "GATHER_RESOURCES")
    {
        MissionToStart.MissionID = "GATHER_RESOURCES";
        MissionToStart.MissionName = "Resource Gathering";
        MissionToStart.MissionDescription = "Collect basic resources for survival";
        MissionToStart.bIsMainQuest = false;
        MissionToStart.ExperienceReward = 100;
        
        FQuest_ObjectiveData GatherObjective;
        GatherObjective.ObjectiveID = "COLLECT_STONES_5";
        GatherObjective.Title = "Collect Stones";
        GatherObjective.Description = "Gather 5 stones for crafting";
        GatherObjective.Type = EQuest_ObjectiveType::Collect_Resources;
        GatherObjective.Status = EQuest_ObjectiveStatus::Active;
        GatherObjective.TargetCount = 5;
        GatherObjective.CurrentCount = 0;
        GatherObjective.RequiredItems.Add("Stone");
        
        MissionToStart.Objectives.Add(GatherObjective);
        bFoundMission = true;
    }
    
    if (bFoundMission)
    {
        ActiveMissions.Add(MissionToStart);
        
        if (MissionToStart.bIsMainQuest)
        {
            CurrentMainMission = MissionToStart;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Started mission: %s"), *MissionToStart.MissionName);
        return true;
    }
    
    UE_LOG(LogTemp, Error, TEXT("Mission %s not found"), *MissionID);
    return false;
}

bool UQuest_MissionObjectiveManager::CompleteMission(const FString& MissionID)
{
    for (int32 i = 0; i < ActiveMissions.Num(); i++)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            FQuest_MissionData CompletedMission = ActiveMissions[i];
            CompletedMissions.Add(CompletedMission);
            ActiveMissions.RemoveAt(i);
            
            OnMissionCompleted(MissionID);
            return true;
        }
    }
    return false;
}

void UQuest_MissionObjectiveManager::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, int32 ProgressAmount)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (FQuest_ObjectiveData& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID && Objective.Status == EQuest_ObjectiveStatus::Active)
                {
                    Objective.CurrentCount = FMath::Min(Objective.CurrentCount + ProgressAmount, Objective.TargetCount);
                    
                    if (CheckObjectiveCompletion(Objective))
                    {
                        Objective.Status = EQuest_ObjectiveStatus::Completed;
                        OnObjectiveCompleted(MissionID, ObjectiveID);
                        
                        // Check if all objectives are complete
                        bool bAllComplete = true;
                        for (const FQuest_ObjectiveData& CheckObj : Mission.Objectives)
                        {
                            if (CheckObj.Status != EQuest_ObjectiveStatus::Completed)
                            {
                                bAllComplete = false;
                                break;
                            }
                        }
                        
                        if (bAllComplete)
                        {
                            CompleteMission(MissionID);
                        }
                    }
                    return;
                }
            }
        }
    }
}

bool UQuest_MissionObjectiveManager::IsObjectiveComplete(const FString& MissionID, const FString& ObjectiveID)
{
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (const FQuest_ObjectiveData& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID)
                {
                    return Objective.Status == EQuest_ObjectiveStatus::Completed;
                }
            }
        }
    }
    return false;
}

TArray<FQuest_ObjectiveData> UQuest_MissionObjectiveManager::GetActiveObjectives()
{
    TArray<FQuest_ObjectiveData> AllActiveObjectives;
    
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        for (const FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.Status == EQuest_ObjectiveStatus::Active)
            {
                AllActiveObjectives.Add(Objective);
            }
        }
    }
    
    return AllActiveObjectives;
}

FQuest_MissionData UQuest_MissionObjectiveManager::GetMissionByID(const FString& MissionID)
{
    for (const FQuest_MissionData& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    for (const FQuest_MissionData& Mission : CompletedMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return Mission;
        }
    }
    
    return FQuest_MissionData();
}

void UQuest_MissionObjectiveManager::CheckLocationObjectives(const FVector& PlayerLocation)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.Type == EQuest_ObjectiveType::Reach_Location && 
                Objective.Status == EQuest_ObjectiveStatus::Active)
            {
                float Distance = FVector::Dist(PlayerLocation, Objective.TargetLocation);
                if (Distance <= Objective.TargetRadius)
                {
                    UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_MissionObjectiveManager::RegisterItemCollection(const FString& ItemName)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.Type == EQuest_ObjectiveType::Collect_Resources && 
                Objective.Status == EQuest_ObjectiveStatus::Active)
            {
                if (Objective.RequiredItems.Contains(ItemName))
                {
                    UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
                }
            }
        }
    }
}

void UQuest_MissionObjectiveManager::RegisterEnemyKill(const FString& EnemyType)
{
    for (FQuest_MissionData& Mission : ActiveMissions)
    {
        for (FQuest_ObjectiveData& Objective : Mission.Objectives)
        {
            if (Objective.Type == EQuest_ObjectiveType::Hunt_Target && 
                Objective.Status == EQuest_ObjectiveStatus::Active)
            {
                // For now, accept any enemy kill for hunt objectives
                UpdateObjectiveProgress(Mission.MissionID, Objective.ObjectiveID, 1);
            }
        }
    }
}

void UQuest_MissionObjectiveManager::InitializeDefaultMissions()
{
    // Start with the first hunt mission automatically
    StartMission("HUNT_FIRST_RAPTOR");
    StartMission("GATHER_RESOURCES");
}

bool UQuest_MissionObjectiveManager::CheckObjectiveCompletion(FQuest_ObjectiveData& Objective)
{
    switch (Objective.Type)
    {
        case EQuest_ObjectiveType::Hunt_Target:
        case EQuest_ObjectiveType::Collect_Resources:
        case EQuest_ObjectiveType::Craft_Item:
            return Objective.CurrentCount >= Objective.TargetCount;
            
        case EQuest_ObjectiveType::Reach_Location:
        case EQuest_ObjectiveType::Explore_Area:
            return Objective.CurrentCount > 0;
            
        case EQuest_ObjectiveType::Survive_Duration:
            return Objective.CurrentCount >= Objective.TargetCount;
            
        default:
            return false;
    }
}

void UQuest_MissionObjectiveManager::OnObjectiveCompleted(const FString& MissionID, const FString& ObjectiveID)
{
    UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s in mission %s"), *ObjectiveID, *MissionID);
    
    // Here you could trigger UI updates, sound effects, etc.
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Objective Complete: %s"), *ObjectiveID);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, Message);
    }
}

void UQuest_MissionObjectiveManager::OnMissionCompleted(const FString& MissionID)
{
    UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s"), *MissionID);
    
    // Award experience and items
    FQuest_MissionData CompletedMission = GetMissionByID(MissionID);
    
    if (GEngine)
    {
        FString Message = FString::Printf(TEXT("Mission Complete: %s (+%d XP)"), 
            *CompletedMission.MissionName, CompletedMission.ExperienceReward);
        GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Yellow, Message);
    }
}
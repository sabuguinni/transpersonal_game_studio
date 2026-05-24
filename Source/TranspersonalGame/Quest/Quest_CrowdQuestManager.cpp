#include "Quest_CrowdQuestManager.h"
#include "../Crowd/Crowd_MassSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UQuest_CrowdQuestManager::UQuest_CrowdQuestManager()
{
    CrowdManagerRef = nullptr;
    LastUpdateTime = 0.0f;
    ObjectiveCheckInterval = 2.0f;
}

void UQuest_CrowdQuestManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Initializing crowd quest system"));
    
    InitializeDefaultCrowdMissions();
    
    // Start objective checking timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            ObjectiveCheckTimer,
            this,
            &UQuest_CrowdQuestManager::CheckCrowdObjectiveProgress,
            ObjectiveCheckInterval,
            true
        );
    }
}

void UQuest_CrowdQuestManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ObjectiveCheckTimer);
    }
    
    Super::Deinitialize();
}

void UQuest_CrowdQuestManager::StartCrowdMission(const FString& MissionID)
{
    for (FQuest_CrowdMission& Mission : ActiveCrowdMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::NotStarted)
        {
            Mission.Status = EQuest_MissionStatus::InProgress;
            Mission.MissionStartTime = GetWorld()->GetTimeSeconds();
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Started crowd mission: %s"), *MissionID);
            
            // Reset all objectives
            for (FQuest_CrowdObjective& Objective : Mission.Objectives)
            {
                Objective.bIsCompleted = false;
                Objective.CompletionProgress = 0.0f;
            }
            
            break;
        }
    }
}

void UQuest_CrowdQuestManager::CompleteCrowdMission(const FString& MissionID)
{
    for (int32 i = ActiveCrowdMissions.Num() - 1; i >= 0; i--)
    {
        if (ActiveCrowdMissions[i].MissionID == MissionID)
        {
            FQuest_CrowdMission CompletedMission = ActiveCrowdMissions[i];
            CompletedMission.Status = EQuest_MissionStatus::Completed;
            
            CompletedCrowdMissions.Add(CompletedMission);
            ActiveCrowdMissions.RemoveAt(i);
            
            OnMissionCompleted(CompletedMission);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Completed crowd mission: %s"), *MissionID);
            break;
        }
    }
}

void UQuest_CrowdQuestManager::UpdateCrowdObjective(const FString& MissionID, const FString& ObjectiveID, float Progress)
{
    for (FQuest_CrowdMission& Mission : ActiveCrowdMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::InProgress)
        {
            for (FQuest_CrowdObjective& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID)
                {
                    Objective.CompletionProgress = FMath::Clamp(Progress, 0.0f, 1.0f);
                    
                    if (Objective.CompletionProgress >= 1.0f)
                    {
                        Objective.bIsCompleted = true;
                        UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Objective completed: %s"), *ObjectiveID);
                    }
                    
                    // Check if all objectives are completed
                    bool bAllCompleted = true;
                    for (const FQuest_CrowdObjective& CheckObjective : Mission.Objectives)
                    {
                        if (!CheckObjective.bIsCompleted)
                        {
                            bAllCompleted = false;
                            break;
                        }
                    }
                    
                    if (bAllCompleted)
                    {
                        CompleteCrowdMission(MissionID);
                    }
                    
                    return;
                }
            }
        }
    }
}

bool UQuest_CrowdQuestManager::IsCrowdMissionActive(const FString& MissionID) const
{
    for (const FQuest_CrowdMission& Mission : ActiveCrowdMissions)
    {
        if (Mission.MissionID == MissionID && Mission.Status == EQuest_MissionStatus::InProgress)
        {
            return true;
        }
    }
    return false;
}

TArray<FQuest_CrowdMission> UQuest_CrowdQuestManager::GetActiveCrowdMissions() const
{
    return ActiveCrowdMissions;
}

void UQuest_CrowdQuestManager::CreateHerdingMission(const FVector& StartLocation, const FVector& TargetLocation, int32 RequiredHerdSize)
{
    FQuest_CrowdMission HerdingMission;
    HerdingMission.MissionID = FString::Printf(TEXT("HERDING_%d"), FMath::RandRange(1000, 9999));
    HerdingMission.MissionName = TEXT("Herd Management");
    HerdingMission.MissionDescription = TEXT("Guide the herbivore herd to the new grazing area safely.");
    HerdingMission.Status = EQuest_MissionStatus::NotStarted;
    HerdingMission.MissionDuration = 600.0f;
    HerdingMission.RewardExperience = 200;
    
    // Create herding objective
    FQuest_CrowdObjective HerdObjective;
    HerdObjective.ObjectiveID = TEXT("HERD_GUIDE");
    HerdObjective.Description = FString::Printf(TEXT("Guide %d animals to the target location"), RequiredHerdSize);
    HerdObjective.ObjectiveType = EQuest_ObjectiveType::Escort;
    HerdObjective.TargetLocation = TargetLocation;
    HerdObjective.RequiredRadius = 500.0f;
    HerdObjective.RequiredCrowdCount = RequiredHerdSize;
    HerdObjective.TimeLimit = 600.0f;
    
    HerdingMission.Objectives.Add(HerdObjective);
    ActiveCrowdMissions.Add(HerdingMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Created herding mission: %s"), *HerdingMission.MissionID);
}

void UQuest_CrowdQuestManager::CreateCrowdEvacuationMission(const FVector& DangerZone, const FVector& SafeZone, float EvacuationRadius)
{
    FQuest_CrowdMission EvacuationMission;
    EvacuationMission.MissionID = FString::Printf(TEXT("EVACUATION_%d"), FMath::RandRange(1000, 9999));
    EvacuationMission.MissionName = TEXT("Emergency Evacuation");
    EvacuationMission.MissionDescription = TEXT("Lead the tribe away from danger to safety.");
    EvacuationMission.Status = EQuest_MissionStatus::NotStarted;
    EvacuationMission.MissionDuration = 300.0f;
    EvacuationMission.RewardExperience = 300;
    
    // Create evacuation objective
    FQuest_CrowdObjective EvacObjective;
    EvacObjective.ObjectiveID = TEXT("EVACUATE_CROWD");
    EvacObjective.Description = TEXT("Lead all tribe members to the safe zone");
    EvacObjective.ObjectiveType = EQuest_ObjectiveType::Escort;
    EvacObjective.TargetLocation = SafeZone;
    EvacObjective.RequiredRadius = EvacuationRadius;
    EvacObjective.RequiredCrowdCount = 10;
    EvacObjective.TimeLimit = 300.0f;
    
    EvacuationMission.Objectives.Add(EvacObjective);
    ActiveCrowdMissions.Add(EvacuationMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Created evacuation mission: %s"), *EvacuationMission.MissionID);
}

void UQuest_CrowdQuestManager::CreateTribalGatheringMission(const FVector& GatheringLocation, int32 RequiredAttendees)
{
    FQuest_CrowdMission GatheringMission;
    GatheringMission.MissionID = FString::Printf(TEXT("GATHERING_%d"), FMath::RandRange(1000, 9999));
    GatheringMission.MissionName = TEXT("Tribal Gathering");
    GatheringMission.MissionDescription = TEXT("Organize a tribal meeting at the designated location.");
    GatheringMission.Status = EQuest_MissionStatus::NotStarted;
    GatheringMission.MissionDuration = 400.0f;
    GatheringMission.RewardExperience = 150;
    
    // Create gathering objective
    FQuest_CrowdObjective GatherObjective;
    GatherObjective.ObjectiveID = TEXT("ORGANIZE_GATHERING");
    GatherObjective.Description = FString::Printf(TEXT("Gather %d tribe members at the meeting point"), RequiredAttendees);
    GatherObjective.ObjectiveType = EQuest_ObjectiveType::Collect;
    GatherObjective.TargetLocation = GatheringLocation;
    GatherObjective.RequiredRadius = 300.0f;
    GatherObjective.RequiredCrowdCount = RequiredAttendees;
    GatherObjective.TimeLimit = 400.0f;
    
    GatheringMission.Objectives.Add(GatherObjective);
    ActiveCrowdMissions.Add(GatheringMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Created gathering mission: %s"), *GatheringMission.MissionID);
}

void UQuest_CrowdQuestManager::CheckCrowdObjectiveProgress()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FQuest_CrowdMission& Mission : ActiveCrowdMissions)
    {
        if (Mission.Status != EQuest_MissionStatus::InProgress)
            continue;
            
        // Check mission timeout
        if (CurrentTime - Mission.MissionStartTime > Mission.MissionDuration)
        {
            Mission.Status = EQuest_MissionStatus::Failed;
            UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Mission timed out: %s"), *Mission.MissionID);
            continue;
        }
        
        // Check each objective
        for (FQuest_CrowdObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bool bCompleted = CheckObjectiveCompletion(Objective);
                if (bCompleted)
                {
                    Objective.bIsCompleted = true;
                    Objective.CompletionProgress = 1.0f;
                    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Auto-completed objective: %s"), *Objective.ObjectiveID);
                }
            }
        }
    }
    
    LastUpdateTime = CurrentTime;
}

void UQuest_CrowdQuestManager::RegisterCrowdManager(ACrowd_MassSimulationManager* CrowdManager)
{
    CrowdManagerRef = CrowdManager;
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Registered crowd manager"));
}

void UQuest_CrowdQuestManager::InitializeDefaultCrowdMissions()
{
    // Create default herding mission
    CreateHerdingMission(
        FVector(2000.0f, 2000.0f, 100.0f),
        FVector(5000.0f, 5000.0f, 100.0f),
        8
    );
    
    // Create default evacuation mission
    CreateCrowdEvacuationMission(
        FVector(0.0f, 0.0f, 100.0f),
        FVector(3000.0f, 3000.0f, 100.0f),
        800.0f
    );
    
    // Create default gathering mission
    CreateTribalGatheringMission(
        FVector(1500.0f, 1500.0f, 100.0f),
        12
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Initialized %d default crowd missions"), ActiveCrowdMissions.Num());
}

void UQuest_CrowdQuestManager::UpdateMissionTimers()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FQuest_CrowdMission& Mission : ActiveCrowdMissions)
    {
        if (Mission.Status == EQuest_MissionStatus::InProgress)
        {
            float ElapsedTime = CurrentTime - Mission.MissionStartTime;
            if (ElapsedTime > Mission.MissionDuration)
            {
                Mission.Status = EQuest_MissionStatus::Failed;
            }
        }
    }
}

bool UQuest_CrowdQuestManager::CheckObjectiveCompletion(const FQuest_CrowdObjective& Objective)
{
    int32 CrowdCount = GetCrowdCountInRadius(Objective.TargetLocation, Objective.RequiredRadius);
    return CrowdCount >= Objective.RequiredCrowdCount;
}

int32 UQuest_CrowdQuestManager::GetCrowdCountInRadius(const FVector& Location, float Radius)
{
    if (!CrowdManagerRef)
        return 0;
        
    // This would interface with the crowd manager to get actual crowd count
    // For now, return a simulated value
    return FMath::RandRange(0, 15);
}

void UQuest_CrowdQuestManager::OnMissionCompleted(const FQuest_CrowdMission& Mission)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_CrowdQuestManager: Mission completed with %d XP reward: %s"), 
           Mission.RewardExperience, *Mission.MissionName);
    
    // Here we would award experience, trigger events, etc.
}
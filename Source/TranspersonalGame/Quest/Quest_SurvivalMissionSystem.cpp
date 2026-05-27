#include "Quest_SurvivalMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UQuest_SurvivalMissionSystem::UQuest_SurvivalMissionSystem()
{
    LastStatCheckTime = 0.0f;
}

void UQuest_SurvivalMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initializing survival mission system"));
    
    InitializeMissionDatabase();
    CreateBasicSurvivalMissions();
    
    // Set up timer for stat monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UQuest_SurvivalMissionSystem::MonitorSurvivalStats,
            1.0f, // Check every second
            true  // Loop
        );
    }
}

void UQuest_SurvivalMissionSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    ActiveMissions.Empty();
    CompletedMissions.Empty();
    MissionDatabase.Empty();
    
    Super::Deinitialize();
}

void UQuest_SurvivalMissionSystem::InitializeMissionDatabase()
{
    MissionDatabase.Empty();
    
    // Water Survival Mission
    FQuest_SurvivalMission WaterMission;
    WaterMission.MissionID = "WATER_SURVIVAL_001";
    WaterMission.Title = "Find Clean Water";
    WaterMission.Description = "Your thirst is becoming critical. Find a clean water source and drink to restore your hydration levels.";
    WaterMission.RequiredBiome = EBiomeType::Savana;
    
    FQuest_SurvivalObjective WaterObjective;
    WaterObjective.ObjectiveID = "DRINK_WATER";
    WaterObjective.Description = "Restore thirst to 80% or higher";
    WaterObjective.RequiredStat = ESurvivalStat::Thirst;
    WaterObjective.TargetValue = 80.0f;
    WaterObjective.TimeLimit = 600.0f; // 10 minutes
    WaterMission.Objectives.Add(WaterObjective);
    WaterMission.RewardItems.Add("Water Container");
    
    MissionDatabase.Add(WaterMission.MissionID, WaterMission);
    
    // Food Survival Mission
    FQuest_SurvivalMission FoodMission;
    FoodMission.MissionID = "FOOD_SURVIVAL_001";
    FoodMission.Title = "Hunt for Food";
    FoodMission.Description = "Your hunger is weakening you. Hunt small prey or gather edible plants to restore your energy.";
    FoodMission.RequiredBiome = EBiomeType::Savana;
    
    FQuest_SurvivalObjective FoodObjective;
    FoodObjective.ObjectiveID = "CONSUME_FOOD";
    FoodObjective.Description = "Restore hunger to 70% or higher";
    FoodObjective.RequiredStat = ESurvivalStat::Hunger;
    FoodObjective.TargetValue = 70.0f;
    FoodObjective.TimeLimit = 900.0f; // 15 minutes
    FoodMission.Objectives.Add(FoodObjective);
    FoodMission.RewardItems.Add("Hunting Spear");
    
    MissionDatabase.Add(FoodMission.MissionID, FoodMission);
    
    // Shelter Survival Mission
    FQuest_SurvivalMission ShelterMission;
    ShelterMission.MissionID = "SHELTER_SURVIVAL_001";
    ShelterMission.Title = "Build Emergency Shelter";
    ShelterMission.Description = "Weather conditions are deteriorating. Build a basic shelter to protect yourself from the elements.";
    ShelterMission.RequiredBiome = EBiomeType::Savana;
    
    FQuest_SurvivalObjective ShelterObjective;
    ShelterObjective.ObjectiveID = "BUILD_SHELTER";
    ShelterObjective.Description = "Construct a basic shelter structure";
    ShelterObjective.RequiredStat = ESurvivalStat::Health;
    ShelterObjective.TargetValue = 1.0f; // Binary completion
    ShelterObjective.TimeLimit = 1200.0f; // 20 minutes
    ShelterMission.Objectives.Add(ShelterObjective);
    ShelterMission.RewardItems.Add("Crafting Tools");
    
    MissionDatabase.Add(ShelterMission.MissionID, ShelterMission);
}

void UQuest_SurvivalMissionSystem::StartMission(const FString& MissionID)
{
    if (FQuest_SurvivalMission* Mission = MissionDatabase.Find(MissionID))
    {
        if (!IsMissionActive(MissionID))
        {
            Mission->bIsActive = true;
            Mission->bIsCompleted = false;
            Mission->MissionStartTime = GetWorld()->GetTimeSeconds();
            
            // Reset all objectives
            for (FQuest_SurvivalObjective& Objective : Mission->Objectives)
            {
                Objective.bIsCompleted = false;
                Objective.CurrentValue = 0.0f;
                Objective.ElapsedTime = 0.0f;
            }
            
            ActiveMissions.Add(*Mission);
            OnMissionStarted(*Mission);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Started mission %s"), *MissionID);
        }
    }
}

void UQuest_SurvivalMissionSystem::CompleteMission(const FString& MissionID)
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; --i)
    {
        if (ActiveMissions[i].MissionID == MissionID)
        {
            ActiveMissions[i].bIsCompleted = true;
            ActiveMissions[i].bIsActive = false;
            
            CompletedMissions.Add(ActiveMissions[i]);
            OnMissionCompleted(ActiveMissions[i]);
            
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Completed mission %s"), *MissionID);
            
            ActiveMissions.RemoveAt(i);
            break;
        }
    }
}

void UQuest_SurvivalMissionSystem::UpdateObjectiveProgress(const FString& MissionID, const FString& ObjectiveID, float Progress)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            for (FQuest_SurvivalObjective& Objective : Mission.Objectives)
            {
                if (Objective.ObjectiveID == ObjectiveID)
                {
                    Objective.CurrentValue = Progress;
                    
                    if (Progress >= Objective.TargetValue && !Objective.bIsCompleted)
                    {
                        Objective.bIsCompleted = true;
                        OnObjectiveCompleted(Objective);
                        
                        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Completed objective %s"), *ObjectiveID);
                        
                        // Check if all objectives are completed
                        CheckObjectiveCompletion();
                    }
                    break;
                }
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

void UQuest_SurvivalMissionSystem::CreateBasicSurvivalMissions()
{
    // Auto-start water mission if thirst is low
    // This would typically be called by the game state or character system
}

void UQuest_SurvivalMissionSystem::CreateWaterSurvivalMission()
{
    StartMission("WATER_SURVIVAL_001");
}

void UQuest_SurvivalMissionSystem::CreateFoodSurvivalMission()
{
    StartMission("FOOD_SURVIVAL_001");
}

void UQuest_SurvivalMissionSystem::CreateShelterSurvivalMission()
{
    StartMission("SHELTER_SURVIVAL_001");
}

void UQuest_SurvivalMissionSystem::MonitorSurvivalStats()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastStatCheckTime;
    LastStatCheckTime = CurrentTime;
    
    UpdateMissionTimers(DeltaTime);
    ProcessMissionTimeouts();
    
    // This would integrate with the character's survival stats
    // For now, we simulate stat monitoring
    CheckObjectiveCompletion();
}

void UQuest_SurvivalMissionSystem::CheckObjectiveCompletion()
{
    TArray<FString> CompletedMissionIDs;
    
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        bool bAllObjectivesCompleted = true;
        
        for (const FQuest_SurvivalObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                bAllObjectivesCompleted = false;
                break;
            }
        }
        
        if (bAllObjectivesCompleted)
        {
            CompletedMissionIDs.Add(Mission.MissionID);
        }
    }
    
    for (const FString& MissionID : CompletedMissionIDs)
    {
        CompleteMission(MissionID);
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

void UQuest_SurvivalMissionSystem::ProcessMissionTimeouts()
{
    TArray<FString> TimedOutMissions;
    
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        for (FQuest_SurvivalObjective& Objective : Mission.Objectives)
        {
            if (Objective.ElapsedTime >= Objective.TimeLimit && !Objective.bIsCompleted)
            {
                UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Objective %s timed out"), *Objective.ObjectiveID);
                TimedOutMissions.AddUnique(Mission.MissionID);
            }
        }
    }
    
    // Handle timed out missions (could fail them or extend time)
    for (const FString& MissionID : TimedOutMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Mission %s timed out"), *MissionID);
    }
}

void UQuest_SurvivalMissionSystem::UpdateMissionTimers(float DeltaTime)
{
    for (FQuest_SurvivalMission& Mission : ActiveMissions)
    {
        for (FQuest_SurvivalObjective& Objective : Mission.Objectives)
        {
            if (!Objective.bIsCompleted)
            {
                Objective.ElapsedTime += DeltaTime;
            }
        }
    }
}
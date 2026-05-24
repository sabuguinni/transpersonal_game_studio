#include "Quest_SurvivalMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UQuest_SurvivalMissionSystem::UQuest_SurvivalMissionSystem()
{
    MaxActiveMissions = 5;
    MissionCheckInterval = 2.0f;
}

void UQuest_SurvivalMissionSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initializing survival mission system"));
    
    // Create default missions after a short delay
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MissionUpdateTimer, this, &UQuest_SurvivalMissionSystem::CreateDefaultMissions, 1.0f, false);
    }
}

void UQuest_SurvivalMissionSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MissionUpdateTimer);
    }
    
    SurvivalMissions.Empty();
    HuntingMissions.Empty();
    
    Super::Deinitialize();
}

void UQuest_SurvivalMissionSystem::StartSurvivalMission(const FString& MissionID, ESurvivalStat StatType, float TargetValue, float RewardAmount)
{
    if (SurvivalMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Cannot start mission %s - max active missions reached"), *MissionID);
        return;
    }

    FQuest_SurvivalMissionData NewMission;
    NewMission.MissionID = MissionID;
    NewMission.RequiredStat = StatType;
    NewMission.TargetValue = TargetValue;
    NewMission.RewardAmount = RewardAmount;
    NewMission.bIsActive = true;
    NewMission.bIsCompleted = false;

    switch (StatType)
    {
        case ESurvivalStat::Health:
            NewMission.MissionName = TEXT("Maintain Health");
            NewMission.Description = FString::Printf(TEXT("Keep your health above %d for survival"), FMath::RoundToInt(TargetValue));
            break;
        case ESurvivalStat::Hunger:
            NewMission.MissionName = TEXT("Find Food");
            NewMission.Description = FString::Printf(TEXT("Reduce hunger below %d by finding food"), FMath::RoundToInt(TargetValue));
            break;
        case ESurvivalStat::Thirst:
            NewMission.MissionName = TEXT("Find Water");
            NewMission.Description = FString::Printf(TEXT("Reduce thirst below %d by finding water"), FMath::RoundToInt(TargetValue));
            break;
        case ESurvivalStat::Stamina:
            NewMission.MissionName = TEXT("Rest and Recover");
            NewMission.Description = FString::Printf(TEXT("Restore stamina above %d by resting"), FMath::RoundToInt(TargetValue));
            break;
        case ESurvivalStat::Fear:
            NewMission.MissionName = TEXT("Overcome Fear");
            NewMission.Description = FString::Printf(TEXT("Reduce fear below %d by surviving encounters"), FMath::RoundToInt(TargetValue));
            break;
    }

    SurvivalMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Started survival mission %s"), *MissionID);
}

void UQuest_SurvivalMissionSystem::StartHuntingMission(const FString& MissionID, const FString& TargetSpecies, int32 RequiredKills, FVector ZoneCenter, float ZoneRadius)
{
    if (HuntingMissions.Num() >= MaxActiveMissions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Cannot start hunting mission %s - max active missions reached"), *MissionID);
        return;
    }

    FQuest_HuntingMissionData NewMission;
    NewMission.MissionID = MissionID;
    NewMission.TargetSpecies = TargetSpecies;
    NewMission.RequiredKills = RequiredKills;
    NewMission.CurrentKills = 0;
    NewMission.HuntingZoneCenter = ZoneCenter;
    NewMission.HuntingZoneRadius = ZoneRadius;
    NewMission.bIsCompleted = false;

    HuntingMissions.Add(NewMission);
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Started hunting mission %s - Hunt %d %s"), *MissionID, RequiredKills, *TargetSpecies);
}

void UQuest_SurvivalMissionSystem::UpdateSurvivalProgress(ESurvivalStat StatType, float CurrentValue)
{
    for (FQuest_SurvivalMissionData& Mission : SurvivalMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted && Mission.RequiredStat == StatType)
        {
            bool bMissionComplete = false;
            
            // Check completion based on stat type and target
            switch (StatType)
            {
                case ESurvivalStat::Health:
                case ESurvivalStat::Stamina:
                    bMissionComplete = CurrentValue >= Mission.TargetValue;
                    break;
                case ESurvivalStat::Hunger:
                case ESurvivalStat::Thirst:
                case ESurvivalStat::Fear:
                    bMissionComplete = CurrentValue <= Mission.TargetValue;
                    break;
            }

            if (bMissionComplete)
            {
                Mission.bIsCompleted = true;
                NotifyMissionComplete(Mission.MissionID);
            }
        }
    }
}

void UQuest_SurvivalMissionSystem::UpdateHuntingProgress(const FString& Species, FVector KillLocation)
{
    for (FQuest_HuntingMissionData& Mission : HuntingMissions)
    {
        if (!Mission.bIsCompleted && Mission.TargetSpecies == Species)
        {
            // Check if kill is within hunting zone
            float DistanceToZone = FVector::Dist(KillLocation, Mission.HuntingZoneCenter);
            if (DistanceToZone <= Mission.HuntingZoneRadius)
            {
                Mission.CurrentKills++;
                UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Hunting progress %s - %d/%d kills"), *Mission.MissionID, Mission.CurrentKills, Mission.RequiredKills);

                if (Mission.CurrentKills >= Mission.RequiredKills)
                {
                    Mission.bIsCompleted = true;
                    NotifyMissionComplete(Mission.MissionID);
                }
            }
        }
    }
}

bool UQuest_SurvivalMissionSystem::CompleteMission(const FString& MissionID)
{
    // Check survival missions
    for (int32 i = SurvivalMissions.Num() - 1; i >= 0; i--)
    {
        if (SurvivalMissions[i].MissionID == MissionID && SurvivalMissions[i].bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Completed survival mission %s - Reward: %.1f"), *MissionID, SurvivalMissions[i].RewardAmount);
            SurvivalMissions.RemoveAt(i);
            return true;
        }
    }

    // Check hunting missions
    for (int32 i = HuntingMissions.Num() - 1; i >= 0; i--)
    {
        if (HuntingMissions[i].MissionID == MissionID && HuntingMissions[i].bIsCompleted)
        {
            UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Completed hunting mission %s"), *MissionID);
            HuntingMissions.RemoveAt(i);
            return true;
        }
    }

    return false;
}

TArray<FQuest_SurvivalMissionData> UQuest_SurvivalMissionSystem::GetActiveSurvivalMissions() const
{
    TArray<FQuest_SurvivalMissionData> ActiveMissions;
    for (const FQuest_SurvivalMissionData& Mission : SurvivalMissions)
    {
        if (Mission.bIsActive && !Mission.bIsCompleted)
        {
            ActiveMissions.Add(Mission);
        }
    }
    return ActiveMissions;
}

TArray<FQuest_HuntingMissionData> UQuest_SurvivalMissionSystem::GetActiveHuntingMissions() const
{
    TArray<FQuest_HuntingMissionData> ActiveMissions;
    for (const FQuest_HuntingMissionData& Mission : HuntingMissions)
    {
        if (!Mission.bIsCompleted)
        {
            ActiveMissions.Add(Mission);
        }
    }
    return ActiveMissions;
}

void UQuest_SurvivalMissionSystem::CreateDefaultMissions()
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Creating default survival missions"));

    // Create basic survival missions
    StartSurvivalMission(TEXT("SURV_001"), ESurvivalStat::Health, 80.0f, 10.0f);
    StartSurvivalMission(TEXT("SURV_002"), ESurvivalStat::Hunger, 30.0f, 15.0f);
    StartSurvivalMission(TEXT("SURV_003"), ESurvivalStat::Thirst, 25.0f, 12.0f);

    // Create hunting missions
    StartHuntingMission(TEXT("HUNT_001"), TEXT("Raptor"), 2, FVector(0, 0, 0), 5000.0f);
    StartHuntingMission(TEXT("HUNT_002"), TEXT("Compsognathus"), 5, FVector(2000, 2000, 0), 3000.0f);

    // Start periodic mission checking
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MissionUpdateTimer, this, &UQuest_SurvivalMissionSystem::CheckMissionProgress, MissionCheckInterval, true);
    }
}

void UQuest_SurvivalMissionSystem::CheckMissionProgress()
{
    // This method is called periodically to check mission status
    // In a real implementation, this would interface with the player's survival stats
    // For now, it just maintains the timer system
}

void UQuest_SurvivalMissionSystem::NotifyMissionComplete(const FString& MissionID)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: MISSION COMPLETE - %s"), *MissionID);
    
    // In a full implementation, this would trigger UI notifications, sound effects, etc.
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, FString::Printf(TEXT("Mission Complete: %s"), *MissionID));
    }
}

FQuest_SurvivalMissionData* UQuest_SurvivalMissionSystem::FindSurvivalMission(const FString& MissionID)
{
    for (FQuest_SurvivalMissionData& Mission : SurvivalMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return &Mission;
        }
    }
    return nullptr;
}

FQuest_HuntingMissionData* UQuest_SurvivalMissionSystem::FindHuntingMission(const FString& MissionID)
{
    for (FQuest_HuntingMissionData& Mission : HuntingMissions)
    {
        if (Mission.MissionID == MissionID)
        {
            return &Mission;
        }
    }
    return nullptr;
}
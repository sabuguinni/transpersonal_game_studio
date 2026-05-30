#include "Quest_SurvivalMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "../TranspersonalCharacter.h"

UQuest_SurvivalMissionSystem::UQuest_SurvivalMissionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    bHasActiveMission = false;
    MissionCheckInterval = 2.0f;
    CompletedMissionsCount = 0;
}

void UQuest_SurvivalMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultMissions();
    
    GetWorld()->GetTimerManager().SetTimer(
        MissionUpdateTimer,
        this,
        &UQuest_SurvivalMissionSystem::UpdateMissionProgress,
        MissionCheckInterval,
        true
    );
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initialized with %d available missions"), AvailableMissions.Num());
}

void UQuest_SurvivalMissionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bHasActiveMission)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        float ElapsedTime = CurrentTime - CurrentMission.MissionStartTime;
        
        // Check for mission timeout
        for (const FQuest_SurvivalObjective& Objective : CurrentMission.Objectives)
        {
            if (Objective.TimeLimit > 0.0f && ElapsedTime > Objective.TimeLimit)
            {
                UE_LOG(LogTemp, Warning, TEXT("Mission '%s' timed out"), *CurrentMission.MissionName);
                CompleteMission();
                break;
            }
        }
    }
}

void UQuest_SurvivalMissionSystem::StartMission(int32 MissionIndex)
{
    if (MissionIndex >= 0 && MissionIndex < AvailableMissions.Num())
    {
        if (bHasActiveMission)
        {
            UE_LOG(LogTemp, Warning, TEXT("Cannot start mission - another mission is already active"));
            return;
        }
        
        CurrentMission = AvailableMissions[MissionIndex];
        CurrentMission.bIsActive = true;
        CurrentMission.MissionStartTime = GetWorld()->GetTimeSeconds();
        bHasActiveMission = true;
        
        // Reset objective completion status
        for (FQuest_SurvivalObjective& Objective : CurrentMission.Objectives)
        {
            Objective.bIsCompleted = false;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Started mission: %s"), *CurrentMission.MissionName);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
                FString::Printf(TEXT("Mission Started: %s"), *CurrentMission.MissionName));
        }
    }
}

void UQuest_SurvivalMissionSystem::CompleteMission()
{
    if (!bHasActiveMission)
    {
        return;
    }
    
    CurrentMission.bIsCompleted = true;
    CurrentMission.bIsActive = false;
    bHasActiveMission = false;
    CompletedMissionsCount++;
    
    UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s (Reward: %d XP)"), 
        *CurrentMission.MissionName, CurrentMission.RewardExperience);
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Mission Complete! +%d XP"), CurrentMission.RewardExperience));
    }
    
    // Generate new random mission for next biome
    EBiomeType NextBiome = static_cast<EBiomeType>((static_cast<int32>(CurrentMission.TargetBiome) + 1) % 5);
    GenerateRandomMission(NextBiome);
}

void UQuest_SurvivalMissionSystem::UpdateMissionProgress()
{
    if (!bHasActiveMission)
    {
        return;
    }
    
    bool bAllObjectivesComplete = true;
    
    for (FQuest_SurvivalObjective& Objective : CurrentMission.Objectives)
    {
        if (!Objective.bIsCompleted)
        {
            if (CheckObjectiveCompletion(Objective))
            {
                Objective.bIsCompleted = true;
                UE_LOG(LogTemp, Warning, TEXT("Objective completed: %s"), *Objective.ObjectiveName);
                
                if (GEngine)
                {
                    GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                        FString::Printf(TEXT("Objective Complete: %s"), *Objective.ObjectiveName));
                }
            }
            else
            {
                bAllObjectivesComplete = false;
            }
        }
    }
    
    if (bAllObjectivesComplete)
    {
        CompleteMission();
    }
}

bool UQuest_SurvivalMissionSystem::CheckObjectiveCompletion(const FQuest_SurvivalObjective& Objective)
{
    // Check location requirement
    if (Objective.RequiredLocation != FVector::ZeroVector)
    {
        if (!IsPlayerInLocation(Objective.RequiredLocation, Objective.LocationRadius))
        {
            return false;
        }
    }
    
    // Check survival stat requirement
    float CurrentStatValue = GetPlayerSurvivalStat(Objective.RequiredStat);
    
    switch (Objective.RequiredStat)
    {
        case ESurvivalStat::Health:
        case ESurvivalStat::Hunger:
        case ESurvivalStat::Thirst:
        case ESurvivalStat::Stamina:
            return CurrentStatValue >= Objective.TargetValue;
        case ESurvivalStat::Fear:
            return CurrentStatValue <= Objective.TargetValue; // Lower fear is better
        default:
            return false;
    }
}

void UQuest_SurvivalMissionSystem::GenerateRandomMission(EBiomeType BiomeType)
{
    FQuest_SurvivalMission NewMission;
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            NewMission.MissionName = TEXT("Savanna Endurance Test");
            NewMission.MissionDescription = TEXT("Survive in the open savanna while maintaining high health");
            break;
        case EBiomeType::Forest:
            NewMission.MissionName = TEXT("Forest Navigation Challenge");
            NewMission.MissionDescription = TEXT("Navigate through the dense forest without getting lost");
            break;
        case EBiomeType::Desert:
            NewMission.MissionName = TEXT("Desert Survival Trial");
            NewMission.MissionDescription = TEXT("Survive the harsh desert conditions");
            break;
        case EBiomeType::Swamp:
            NewMission.MissionName = TEXT("Swamp Courage Test");
            NewMission.MissionDescription = TEXT("Overcome fear in the dangerous swamplands");
            break;
        case EBiomeType::Mountain:
            NewMission.MissionName = TEXT("Mountain Stamina Challenge");
            NewMission.MissionDescription = TEXT("Maintain stamina at high altitude");
            break;
    }
    
    NewMission.TargetBiome = BiomeType;
    NewMission.RewardExperience = 150 + (CompletedMissionsCount * 25);
    
    // Create biome-specific objective
    FQuest_SurvivalObjective Objective;
    Objective.RequiredLocation = BiomeCenter;
    Objective.LocationRadius = 5000.0f;
    Objective.TimeLimit = 180.0f; // 3 minutes
    
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            Objective.ObjectiveName = TEXT("Maintain Health in Savanna");
            Objective.RequiredStat = ESurvivalStat::Health;
            Objective.TargetValue = 80.0f;
            break;
        case EBiomeType::Forest:
            Objective.ObjectiveName = TEXT("Stay Hydrated in Forest");
            Objective.RequiredStat = ESurvivalStat::Thirst;
            Objective.TargetValue = 70.0f;
            break;
        case EBiomeType::Desert:
            Objective.ObjectiveName = TEXT("Survive Desert Heat");
            Objective.RequiredStat = ESurvivalStat::Thirst;
            Objective.TargetValue = 60.0f;
            break;
        case EBiomeType::Swamp:
            Objective.ObjectiveName = TEXT("Control Fear in Swamp");
            Objective.RequiredStat = ESurvivalStat::Fear;
            Objective.TargetValue = 30.0f; // Lower is better
            break;
        case EBiomeType::Mountain:
            Objective.ObjectiveName = TEXT("Maintain Stamina at Altitude");
            Objective.RequiredStat = ESurvivalStat::Stamina;
            Objective.TargetValue = 75.0f;
            break;
    }
    
    NewMission.Objectives.Add(Objective);
    AvailableMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated new mission: %s for biome %d"), 
        *NewMission.MissionName, static_cast<int32>(BiomeType));
}

void UQuest_SurvivalMissionSystem::InitializeDefaultMissions()
{
    AvailableMissions.Empty();
    
    CreateSavannaSurvivalMission();
    CreateForestSurvivalMission();
    CreateDesertSurvivalMission();
    CreateSwampSurvivalMission();
    CreateMountainSurvivalMission();
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d default survival missions"), AvailableMissions.Num());
}

FVector UQuest_SurvivalMissionSystem::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 500.0f);
        default:
            return FVector::ZeroVector;
    }
}

void UQuest_SurvivalMissionSystem::CreateSavannaSurvivalMission()
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Savanna Survival Challenge");
    Mission.MissionDescription = TEXT("Prove your survival skills in the open savanna grasslands");
    Mission.TargetBiome = EBiomeType::Savanna;
    Mission.RewardExperience = 100;
    
    FQuest_SurvivalObjective Objective;
    Objective.ObjectiveName = TEXT("Reach Savanna Center");
    Objective.RequiredLocation = GetBiomeCenter(EBiomeType::Savanna);
    Objective.LocationRadius = 3000.0f;
    Objective.RequiredStat = ESurvivalStat::Health;
    Objective.TargetValue = 75.0f;
    Objective.TimeLimit = 300.0f;
    
    Mission.Objectives.Add(Objective);
    AvailableMissions.Add(Mission);
}

void UQuest_SurvivalMissionSystem::CreateForestSurvivalMission()
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Forest Exploration Mission");
    Mission.MissionDescription = TEXT("Navigate through the dense forest while staying hydrated");
    Mission.TargetBiome = EBiomeType::Forest;
    Mission.RewardExperience = 120;
    
    FQuest_SurvivalObjective Objective;
    Objective.ObjectiveName = TEXT("Explore Forest Depths");
    Objective.RequiredLocation = GetBiomeCenter(EBiomeType::Forest);
    Objective.LocationRadius = 4000.0f;
    Objective.RequiredStat = ESurvivalStat::Thirst;
    Objective.TargetValue = 70.0f;
    Objective.TimeLimit = 240.0f;
    
    Mission.Objectives.Add(Objective);
    AvailableMissions.Add(Mission);
}

void UQuest_SurvivalMissionSystem::CreateDesertSurvivalMission()
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Desert Endurance Trial");
    Mission.MissionDescription = TEXT("Survive the harsh desert conditions and extreme heat");
    Mission.TargetBiome = EBiomeType::Desert;
    Mission.RewardExperience = 150;
    
    FQuest_SurvivalObjective Objective;
    Objective.ObjectiveName = TEXT("Cross Desert Safely");
    Objective.RequiredLocation = GetBiomeCenter(EBiomeType::Desert);
    Objective.LocationRadius = 2500.0f;
    Objective.RequiredStat = ESurvivalStat::Thirst;
    Objective.TargetValue = 50.0f;
    Objective.TimeLimit = 180.0f;
    
    Mission.Objectives.Add(Objective);
    AvailableMissions.Add(Mission);
}

void UQuest_SurvivalMissionSystem::CreateSwampSurvivalMission()
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Swamp Courage Test");
    Mission.MissionDescription = TEXT("Overcome your fears in the dangerous swamplands");
    Mission.TargetBiome = EBiomeType::Swamp;
    Mission.RewardExperience = 140;
    
    FQuest_SurvivalObjective Objective;
    Objective.ObjectiveName = TEXT("Brave the Swamp");
    Objective.RequiredLocation = GetBiomeCenter(EBiomeType::Swamp);
    Objective.LocationRadius = 3500.0f;
    Objective.RequiredStat = ESurvivalStat::Fear;
    Objective.TargetValue = 25.0f; // Lower fear is better
    Objective.TimeLimit = 200.0f;
    
    Mission.Objectives.Add(Objective);
    AvailableMissions.Add(Mission);
}

void UQuest_SurvivalMissionSystem::CreateMountainSurvivalMission()
{
    FQuest_SurvivalMission Mission;
    Mission.MissionName = TEXT("Mountain Ascent Challenge");
    Mission.MissionDescription = TEXT("Climb to high altitude while maintaining stamina");
    Mission.TargetBiome = EBiomeType::Mountain;
    Mission.RewardExperience = 160;
    
    FQuest_SurvivalObjective Objective;
    Objective.ObjectiveName = TEXT("Reach Mountain Peak");
    Objective.RequiredLocation = GetBiomeCenter(EBiomeType::Mountain);
    Objective.LocationRadius = 2000.0f;
    Objective.RequiredStat = ESurvivalStat::Stamina;
    Objective.TargetValue = 80.0f;
    Objective.TimeLimit = 360.0f;
    
    Mission.Objectives.Add(Objective);
    AvailableMissions.Add(Mission);
}

bool UQuest_SurvivalMissionSystem::IsPlayerInLocation(const FVector& Location, float Radius)
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Location);
                return Distance <= Radius;
            }
        }
    }
    return false;
}

float UQuest_SurvivalMissionSystem::GetPlayerSurvivalStat(ESurvivalStat StatType)
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (ATranspersonalCharacter* PlayerChar = Cast<ATranspersonalCharacter>(PC->GetPawn()))
            {
                switch (StatType)
                {
                    case ESurvivalStat::Health:
                        return PlayerChar->GetHealth();
                    case ESurvivalStat::Hunger:
                        return PlayerChar->GetHunger();
                    case ESurvivalStat::Thirst:
                        return PlayerChar->GetThirst();
                    case ESurvivalStat::Stamina:
                        return PlayerChar->GetStamina();
                    case ESurvivalStat::Fear:
                        return PlayerChar->GetFear();
                    default:
                        return 0.0f;
                }
            }
        }
    }
    return 0.0f;
}
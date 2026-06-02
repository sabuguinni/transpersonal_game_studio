#include "Quest_SurvivalMissionController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"

AQuest_SurvivalMissionController::AQuest_SurvivalMissionController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    CurrentMissionType = EQuest_SurvivalMissionType::BasicSurvival;
    MissionDifficulty = EQuest_SurvivalDifficulty::Beginner;
    MissionTimeRemaining = 300.0f;
    bIsMissionActive = false;
    bIsMissionCompleted = false;
    
    // Player survival thresholds
    PlayerHealthThreshold = 30.0f;
    PlayerHungerThreshold = 25.0f;
    PlayerThirstThreshold = 20.0f;
    PlayerStaminaThreshold = 15.0f;
    
    // Internal state
    LastPlayerHealthCheck = 0.0f;
    LastObjectiveCheck = 0.0f;
    CompletedObjectivesCount = 0;
    bPlayerInEmergencyState = false;
    
    InitializeMissionDefaults();
}

void AQuest_SurvivalMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: BeginPlay - System initialized"));
    
    // Start with a basic survival mission for new players
    GenerateSurvivalMission(EQuest_SurvivalMissionType::BasicSurvival, EQuest_SurvivalDifficulty::Beginner);
}

void AQuest_SurvivalMissionController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMissionActive && !bIsMissionCompleted)
    {
        // Update mission timer
        MissionTimeRemaining -= DeltaTime;
        
        // Check for mission failure due to timeout
        if (MissionTimeRemaining <= 0.0f)
        {
            FailSurvivalMission();
            return;
        }
        
        // Monitor player survival state every 2 seconds
        LastPlayerHealthCheck += DeltaTime;
        if (LastPlayerHealthCheck >= 2.0f)
        {
            MonitorPlayerSurvivalState();
            LastPlayerHealthCheck = 0.0f;
        }
        
        // Check objective completion every 1 second
        LastObjectiveCheck += DeltaTime;
        if (LastObjectiveCheck >= 1.0f)
        {
            CheckAllObjectivesCompletion();
            LastObjectiveCheck = 0.0f;
        }
    }
}

void AQuest_SurvivalMissionController::GenerateSurvivalMission(EQuest_SurvivalMissionType MissionType, EQuest_SurvivalDifficulty Difficulty)
{
    CurrentMissionType = MissionType;
    MissionDifficulty = Difficulty;
    
    // Clear previous objectives
    SurvivalObjectives.Empty();
    
    // Generate mission based on type
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::ResourceGathering:
            GenerateResourceGatheringMission();
            break;
        case EQuest_SurvivalMissionType::ShelterBuilding:
            GenerateShelterBuildingMission();
            break;
        case EQuest_SurvivalMissionType::FoodHunting:
            GenerateFoodHuntingMission();
            break;
        case EQuest_SurvivalMissionType::WaterCollection:
            GenerateWaterCollectionMission();
            break;
        case EQuest_SurvivalMissionType::ToolCrafting:
            GenerateToolCraftingMission();
            break;
        case EQuest_SurvivalMissionType::TerritoryDefense:
            GenerateTerritoryDefenseMission();
            break;
        case EQuest_SurvivalMissionType::WeatherSurvival:
            GenerateWeatherSurvivalMission();
            break;
        default:
            // Basic survival mission
            FQuest_SurvivalObjective BasicObjective;
            BasicObjective.ObjectiveDescription = TEXT("Survive for 5 minutes in the wilderness");
            BasicObjective.MissionType = EQuest_SurvivalMissionType::BasicSurvival;
            BasicObjective.RequiredAmount = 1;
            BasicObjective.TimeLimit = 300.0f;
            SurvivalObjectives.Add(BasicObjective);
            break;
    }
    
    // Adjust for difficulty
    AdjustRewardsForDifficulty();
    SetupObjectiveLocations();
    CalculateTimeRequirements();
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Generated %s mission with difficulty %d"), 
           *UEnum::GetValueAsString(MissionType), (int32)Difficulty);
}

void AQuest_SurvivalMissionController::StartSurvivalMission()
{
    if (SurvivalObjectives.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("SurvivalMissionController: Cannot start mission - no objectives defined"));
        return;
    }
    
    bIsMissionActive = true;
    bIsMissionCompleted = false;
    CompletedObjectivesCount = 0;
    
    // Reset all objectives
    for (FQuest_SurvivalObjective& Objective : SurvivalObjectives)
    {
        Objective.bIsCompleted = false;
    }
    
    NotifyPlayerOfMissionUpdate();
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Mission started - %s"), *GetMissionDescription());
}

void AQuest_SurvivalMissionController::CompleteSurvivalMission()
{
    if (!bIsMissionActive)
    {
        return;
    }
    
    bIsMissionActive = false;
    bIsMissionCompleted = true;
    
    // Award rewards to player
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Mission completed! Rewards: %d XP, %d Survival Skill"), 
           MissionReward.ExperiencePoints, MissionReward.SurvivalSkillBonus);
    
    NotifyPlayerOfMissionUpdate();
    
    // Generate next mission after a delay
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this]()
    {
        // Generate a slightly harder mission
        EQuest_SurvivalDifficulty NextDifficulty = MissionDifficulty;
        if (MissionDifficulty != EQuest_SurvivalDifficulty::Master)
        {
            NextDifficulty = static_cast<EQuest_SurvivalDifficulty>(static_cast<int32>(MissionDifficulty) + 1);
        }
        
        // Randomly select next mission type
        int32 RandomType = FMath::RandRange(0, 7);
        EQuest_SurvivalMissionType NextType = static_cast<EQuest_SurvivalMissionType>(RandomType);
        
        GenerateSurvivalMission(NextType, NextDifficulty);
    }, 10.0f, false);
}

void AQuest_SurvivalMissionController::FailSurvivalMission()
{
    if (!bIsMissionActive)
    {
        return;
    }
    
    bIsMissionActive = false;
    bIsMissionCompleted = false;
    
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Mission failed - %s"), *GetMissionDescription());
    NotifyPlayerOfMissionUpdate();
    
    // Generate easier mission after failure
    EQuest_SurvivalDifficulty EasierDifficulty = MissionDifficulty;
    if (MissionDifficulty != EQuest_SurvivalDifficulty::Beginner)
    {
        EasierDifficulty = static_cast<EQuest_SurvivalDifficulty>(static_cast<int32>(MissionDifficulty) - 1);
    }
    
    FTimerHandle TimerHandle;
    GetWorldTimerManager().SetTimer(TimerHandle, [this, EasierDifficulty]()
    {
        GenerateSurvivalMission(EQuest_SurvivalMissionType::BasicSurvival, EasierDifficulty);
    }, 5.0f, false);
}

void AQuest_SurvivalMissionController::UpdateObjectiveProgress(int32 ObjectiveIndex, int32 Progress)
{
    if (!SurvivalObjectives.IsValidIndex(ObjectiveIndex))
    {
        return;
    }
    
    FQuest_SurvivalObjective& Objective = SurvivalObjectives[ObjectiveIndex];
    if (!Objective.bIsCompleted && Progress >= Objective.RequiredAmount)
    {
        Objective.bIsCompleted = true;
        CompletedObjectivesCount++;
        
        UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Objective completed - %s"), 
               *Objective.ObjectiveDescription);
        
        NotifyPlayerOfMissionUpdate();
    }
}

bool AQuest_SurvivalMissionController::CheckObjectiveCompletion(int32 ObjectiveIndex)
{
    if (!SurvivalObjectives.IsValidIndex(ObjectiveIndex))
    {
        return false;
    }
    
    return SurvivalObjectives[ObjectiveIndex].bIsCompleted;
}

void AQuest_SurvivalMissionController::CheckAllObjectivesCompletion()
{
    if (!bIsMissionActive || bIsMissionCompleted)
    {
        return;
    }
    
    bool bAllCompleted = true;
    for (const FQuest_SurvivalObjective& Objective : SurvivalObjectives)
    {
        if (!Objective.bIsCompleted)
        {
            bAllCompleted = false;
            break;
        }
    }
    
    if (bAllCompleted)
    {
        CompleteSurvivalMission();
    }
}

void AQuest_SurvivalMissionController::MonitorPlayerSurvivalState()
{
    // This would normally check the player's actual survival stats
    // For now, simulate with random values to demonstrate the system
    
    float SimulatedHealth = FMath::RandRange(0.0f, 100.0f);
    float SimulatedHunger = FMath::RandRange(0.0f, 100.0f);
    float SimulatedThirst = FMath::RandRange(0.0f, 100.0f);
    
    bool bWasInEmergency = bPlayerInEmergencyState;
    bPlayerInEmergencyState = (SimulatedHealth < PlayerHealthThreshold) ||
                              (SimulatedHunger < PlayerHungerThreshold) ||
                              (SimulatedThirst < PlayerThirstThreshold);
    
    if (bPlayerInEmergencyState && !bWasInEmergency)
    {
        TriggerEmergencySurvivalMission();
    }
}

bool AQuest_SurvivalMissionController::IsPlayerInDanger()
{
    return bPlayerInEmergencyState;
}

void AQuest_SurvivalMissionController::TriggerEmergencySurvivalMission()
{
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Player in danger! Triggering emergency survival mission"));
    
    // Override current mission with emergency survival
    GenerateSurvivalMission(EQuest_SurvivalMissionType::BasicSurvival, EQuest_SurvivalDifficulty::Beginner);
    StartSurvivalMission();
}

void AQuest_SurvivalMissionController::GenerateResourceGatheringMission()
{
    FQuest_SurvivalObjective GatherStones;
    GatherStones.ObjectiveDescription = TEXT("Gather 5 stones for tool crafting");
    GatherStones.MissionType = EQuest_SurvivalMissionType::ResourceGathering;
    GatherStones.RequiredAmount = 5;
    GatherStones.TimeLimit = 180.0f;
    SurvivalObjectives.Add(GatherStones);
    
    FQuest_SurvivalObjective GatherSticks;
    GatherSticks.ObjectiveDescription = TEXT("Collect 3 wooden sticks");
    GatherSticks.MissionType = EQuest_SurvivalMissionType::ResourceGathering;
    GatherSticks.RequiredAmount = 3;
    GatherSticks.TimeLimit = 120.0f;
    SurvivalObjectives.Add(GatherSticks);
}

void AQuest_SurvivalMissionController::GenerateShelterBuildingMission()
{
    FQuest_SurvivalObjective BuildShelter;
    BuildShelter.ObjectiveDescription = TEXT("Build a basic shelter to protect from weather");
    BuildShelter.MissionType = EQuest_SurvivalMissionType::ShelterBuilding;
    BuildShelter.RequiredAmount = 1;
    BuildShelter.TimeLimit = 600.0f;
    SurvivalObjectives.Add(BuildShelter);
}

void AQuest_SurvivalMissionController::GenerateFoodHuntingMission()
{
    FQuest_SurvivalObjective HuntSmallGame;
    HuntSmallGame.ObjectiveDescription = TEXT("Hunt 2 small creatures for food");
    HuntSmallGame.MissionType = EQuest_SurvivalMissionType::FoodHunting;
    HuntSmallGame.RequiredAmount = 2;
    HuntSmallGame.TimeLimit = 300.0f;
    SurvivalObjectives.Add(HuntSmallGame);
}

void AQuest_SurvivalMissionController::GenerateWaterCollectionMission()
{
    FQuest_SurvivalObjective FindWater;
    FindWater.ObjectiveDescription = TEXT("Find and collect clean water");
    FindWater.MissionType = EQuest_SurvivalMissionType::WaterCollection;
    FindWater.RequiredAmount = 3;
    FindWater.TimeLimit = 240.0f;
    SurvivalObjectives.Add(FindWater);
}

void AQuest_SurvivalMissionController::GenerateToolCraftingMission()
{
    FQuest_SurvivalObjective CraftAxe;
    CraftAxe.ObjectiveDescription = TEXT("Craft a stone axe for better resource gathering");
    CraftAxe.MissionType = EQuest_SurvivalMissionType::ToolCrafting;
    CraftAxe.RequiredAmount = 1;
    CraftAxe.TimeLimit = 300.0f;
    SurvivalObjectives.Add(CraftAxe);
}

void AQuest_SurvivalMissionController::GenerateTerritoryDefenseMission()
{
    FQuest_SurvivalObjective DefendArea;
    DefendArea.ObjectiveDescription = TEXT("Defend your territory from predator attacks");
    DefendArea.MissionType = EQuest_SurvivalMissionType::TerritoryDefense;
    DefendArea.RequiredAmount = 3;
    DefendArea.TimeLimit = 480.0f;
    SurvivalObjectives.Add(DefendArea);
}

void AQuest_SurvivalMissionController::GenerateWeatherSurvivalMission()
{
    FQuest_SurvivalObjective SurviveStorm;
    SurviveStorm.ObjectiveDescription = TEXT("Survive the incoming storm by finding shelter");
    SurviveStorm.MissionType = EQuest_SurvivalMissionType::WeatherSurvival;
    SurviveStorm.RequiredAmount = 1;
    SurviveStorm.TimeLimit = 420.0f;
    SurvivalObjectives.Add(SurviveStorm);
}

FVector AQuest_SurvivalMissionController::GetNearbyResourceLocation(const FString& ResourceType)
{
    // Return a random location within 1000 units of the player
    FVector PlayerLocation = GetActorLocation();
    float RandomX = FMath::RandRange(-1000.0f, 1000.0f);
    float RandomY = FMath::RandRange(-1000.0f, 1000.0f);
    
    return PlayerLocation + FVector(RandomX, RandomY, 0.0f);
}

float AQuest_SurvivalMissionController::CalculateMissionDifficultyMultiplier()
{
    switch (MissionDifficulty)
    {
        case EQuest_SurvivalDifficulty::Beginner: return 1.0f;
        case EQuest_SurvivalDifficulty::Novice: return 1.25f;
        case EQuest_SurvivalDifficulty::Experienced: return 1.5f;
        case EQuest_SurvivalDifficulty::Expert: return 1.75f;
        case EQuest_SurvivalDifficulty::Master: return 2.0f;
        default: return 1.0f;
    }
}

void AQuest_SurvivalMissionController::AdjustRewardsForDifficulty()
{
    float Multiplier = CalculateMissionDifficultyMultiplier();
    
    MissionReward.ExperiencePoints = FMath::RoundToInt(MissionReward.ExperiencePoints * Multiplier);
    MissionReward.SurvivalSkillBonus = FMath::RoundToInt(MissionReward.SurvivalSkillBonus * Multiplier);
    MissionReward.HealthRestore *= Multiplier;
    MissionReward.StaminaBonus *= Multiplier;
}

FString AQuest_SurvivalMissionController::GetMissionDescription()
{
    FString Description = FString::Printf(TEXT("%s Mission (%s difficulty)"), 
                                        *UEnum::GetValueAsString(CurrentMissionType),
                                        *UEnum::GetValueAsString(MissionDifficulty));
    
    if (SurvivalObjectives.Num() > 0)
    {
        Description += TEXT("\nObjectives:\n");
        for (int32 i = 0; i < SurvivalObjectives.Num(); i++)
        {
            const FQuest_SurvivalObjective& Obj = SurvivalObjectives[i];
            FString Status = Obj.bIsCompleted ? TEXT("[COMPLETE]") : TEXT("[PENDING]");
            Description += FString::Printf(TEXT("%d. %s %s\n"), i + 1, *Obj.ObjectiveDescription, *Status);
        }
    }
    
    return Description;
}

void AQuest_SurvivalMissionController::DebugPrintMissionStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SURVIVAL MISSION STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Mission Active: %s"), bIsMissionActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Mission Completed: %s"), bIsMissionCompleted ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Time Remaining: %.1f seconds"), MissionTimeRemaining);
    UE_LOG(LogTemp, Warning, TEXT("Objectives Completed: %d/%d"), CompletedObjectivesCount, SurvivalObjectives.Num());
    UE_LOG(LogTemp, Warning, TEXT("Player In Emergency: %s"), bPlayerInEmergencyState ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Mission Description: %s"), *GetMissionDescription());
}

void AQuest_SurvivalMissionController::InitializeMissionDefaults()
{
    // Set default reward values
    MissionReward.ExperiencePoints = 100;
    MissionReward.SurvivalSkillBonus = 10;
    MissionReward.HealthRestore = 25.0f;
    MissionReward.StaminaBonus = 15.0f;
    MissionReward.ItemRewards.Add(TEXT("Basic Survival Kit"));
}

void AQuest_SurvivalMissionController::SetupObjectiveLocations()
{
    // Assign random locations to objectives that need them
    for (FQuest_SurvivalObjective& Objective : SurvivalObjectives)
    {
        if (Objective.TargetLocation.IsZero())
        {
            Objective.TargetLocation = GetNearbyResourceLocation(TEXT("Generic"));
        }
    }
}

void AQuest_SurvivalMissionController::CalculateTimeRequirements()
{
    float TotalTime = 0.0f;
    for (const FQuest_SurvivalObjective& Objective : SurvivalObjectives)
    {
        TotalTime += Objective.TimeLimit;
    }
    
    // Add 20% buffer time
    MissionTimeRemaining = TotalTime * 1.2f;
}

bool AQuest_SurvivalMissionController::ValidatePlayerState()
{
    // This would check if the player character exists and is valid
    return true;
}

void AQuest_SurvivalMissionController::NotifyPlayerOfMissionUpdate()
{
    // This would send notifications to the player UI
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController: Mission update notification sent"));
}
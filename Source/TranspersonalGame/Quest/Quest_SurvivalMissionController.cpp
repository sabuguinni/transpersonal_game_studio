#include "Quest_SurvivalMissionController.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AQuest_SurvivalMissionController::AQuest_SurvivalMissionController()
{
    PrimaryActorTick.bCanEverTick = false;
    MaxActiveMissions = 5;
    MissionGenerationInterval = 300.0f; // 5 minutes
}

void AQuest_SurvivalMissionController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionTemplates();
    
    // Start mission generation timer
    GetWorldTimerManager().SetTimer(
        MissionGenerationTimer,
        this,
        &AQuest_SurvivalMissionController::GenerateRandomMission,
        MissionGenerationInterval,
        true
    );
    
    // Generate initial missions
    for (int32 i = 0; i < 3; ++i)
    {
        GenerateRandomMission();
    }
}

void AQuest_SurvivalMissionController::GenerateRandomMission()
{
    if (ActiveMissions.Num() >= MaxActiveMissions)
    {
        CleanupExpiredMissions();
        return;
    }

    // Random mission type selection
    int32 RandomType = FMath::RandRange(0, 7);
    EQuest_SurvivalMissionType MissionType = static_cast<EQuest_SurvivalMissionType>(RandomType);
    
    FQuest_SurvivalMissionData NewMission;
    
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::HuntDinosaur:
        {
            TArray<FString> DinosaurTypes = {TEXT("Velociraptor"), TEXT("Triceratops"), TEXT("Parasaurolophus")};
            FString TargetDinosaur = DinosaurTypes[FMath::RandRange(0, DinosaurTypes.Num() - 1)];
            FVector HuntLocation = FVector(
                FMath::RandRange(-2000.0f, 2000.0f),
                FMath::RandRange(-2000.0f, 2000.0f),
                100.0f
            );
            NewMission = CreateHuntingMission(TargetDinosaur, HuntLocation);
            break;
        }
        case EQuest_SurvivalMissionType::GatherResources:
        {
            TArray<FString> Resources = {TEXT("Stone"), TEXT("Wood"), TEXT("Berries")};
            int32 Quantity = FMath::RandRange(5, 15);
            NewMission = CreateGatheringMission(Resources, Quantity);
            break;
        }
        case EQuest_SurvivalMissionType::CraftTools:
        {
            TArray<FString> Tools = {TEXT("Stone Axe"), TEXT("Spear"), TEXT("Fire Starter")};
            FString ToolToCraft = Tools[FMath::RandRange(0, Tools.Num() - 1)];
            NewMission = CreateCraftingMission(ToolToCraft);
            break;
        }
        case EQuest_SurvivalMissionType::ExploreTerritory:
        {
            FVector ExploreLocation = FVector(
                FMath::RandRange(-3000.0f, 3000.0f),
                FMath::RandRange(-3000.0f, 3000.0f),
                100.0f
            );
            NewMission = CreateExplorationMission(ExploreLocation, 500.0f);
            break;
        }
        default:
        {
            // Default gathering mission
            TArray<FString> DefaultResources = {TEXT("Stone")};
            NewMission = CreateGatheringMission(DefaultResources, 5);
            break;
        }
    }
    
    NewMission.Difficulty = CalculateMissionDifficulty(MissionType);
    NewMission.ExperienceReward = CalculateExperienceReward(NewMission.Difficulty);
    
    ActiveMissions.Add(NewMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Generated new mission: %s"), *NewMission.MissionName);
}

bool AQuest_SurvivalMissionController::AssignMissionToPlayer(const FQuest_SurvivalMissionData& Mission)
{
    // Find mission in active list
    for (int32 i = 0; i < ActiveMissions.Num(); ++i)
    {
        if (ActiveMissions[i].MissionName == Mission.MissionName)
        {
            UE_LOG(LogTemp, Warning, TEXT("Mission assigned to player: %s"), *Mission.MissionName);
            return true;
        }
    }
    
    return false;
}

void AQuest_SurvivalMissionController::CompleteMission(int32 MissionIndex)
{
    if (ActiveMissions.IsValidIndex(MissionIndex))
    {
        FQuest_SurvivalMissionData CompletedMission = ActiveMissions[MissionIndex];
        CompletedMission.bIsCompleted = true;
        
        CompletedMissions.Add(CompletedMission);
        ActiveMissions.RemoveAt(MissionIndex);
        
        UE_LOG(LogTemp, Warning, TEXT("Mission completed: %s (Reward: %d XP)"), 
               *CompletedMission.MissionName, CompletedMission.ExperienceReward);
    }
}

TArray<FQuest_SurvivalMissionData> AQuest_SurvivalMissionController::GetAvailableMissions() const
{
    return ActiveMissions;
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionController::CreateHuntingMission(const FString& DinosaurType, const FVector& HuntLocation)
{
    FQuest_SurvivalMissionData Mission;
    Mission.MissionName = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    Mission.Description = FString::Printf(TEXT("Track and hunt a %s in the designated area. Use stealth and strategy."), *DinosaurType);
    Mission.MissionType = EQuest_SurvivalMissionType::HuntDinosaur;
    Mission.TargetLocation = HuntLocation;
    Mission.TimeLimit = 900.0f; // 15 minutes
    Mission.RequiredItems.Add(TEXT("Spear"));
    
    return Mission;
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionController::CreateGatheringMission(const TArray<FString>& ResourceTypes, int32 Quantity)
{
    FQuest_SurvivalMissionData Mission;
    Mission.MissionName = TEXT("Gather Resources");
    Mission.Description = FString::Printf(TEXT("Collect %d units of various resources for the tribe."), Quantity);
    Mission.MissionType = EQuest_SurvivalMissionType::GatherResources;
    Mission.TimeLimit = 600.0f; // 10 minutes
    Mission.RequiredItems = ResourceTypes;
    
    return Mission;
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionController::CreateCraftingMission(const FString& ItemToCraft)
{
    FQuest_SurvivalMissionData Mission;
    Mission.MissionName = FString::Printf(TEXT("Craft %s"), *ItemToCraft);
    Mission.Description = FString::Printf(TEXT("Create a %s using available materials and tools."), *ItemToCraft);
    Mission.MissionType = EQuest_SurvivalMissionType::CraftTools;
    Mission.TimeLimit = 480.0f; // 8 minutes
    
    return Mission;
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionController::CreateExplorationMission(const FVector& ExploreLocation, float ExploreRadius)
{
    FQuest_SurvivalMissionData Mission;
    Mission.MissionName = TEXT("Explore New Territory");
    Mission.Description = TEXT("Scout unknown territory and report back with findings. Watch for dangers.");
    Mission.MissionType = EQuest_SurvivalMissionType::ExploreTerritory;
    Mission.TargetLocation = ExploreLocation;
    Mission.TimeLimit = 720.0f; // 12 minutes
    
    return Mission;
}

void AQuest_SurvivalMissionController::UpdateMissionProgress(int32 MissionIndex, float ProgressPercent)
{
    if (ActiveMissions.IsValidIndex(MissionIndex))
    {
        UE_LOG(LogTemp, Log, TEXT("Mission %s progress: %.1f%%"), 
               *ActiveMissions[MissionIndex].MissionName, ProgressPercent);
    }
}

bool AQuest_SurvivalMissionController::IsMissionExpired(const FQuest_SurvivalMissionData& Mission) const
{
    // Simple time-based expiration check
    // In a real implementation, you'd track mission start times
    return false;
}

void AQuest_SurvivalMissionController::InitializeMissionTemplates()
{
    UE_LOG(LogTemp, Warning, TEXT("SurvivalMissionController initialized with %d max active missions"), MaxActiveMissions);
}

void AQuest_SurvivalMissionController::CleanupExpiredMissions()
{
    for (int32 i = ActiveMissions.Num() - 1; i >= 0; --i)
    {
        if (IsMissionExpired(ActiveMissions[i]))
        {
            UE_LOG(LogTemp, Warning, TEXT("Removing expired mission: %s"), *ActiveMissions[i].MissionName);
            ActiveMissions.RemoveAt(i);
        }
    }
}

EQuest_MissionDifficulty AQuest_SurvivalMissionController::CalculateMissionDifficulty(EQuest_SurvivalMissionType MissionType) const
{
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::HuntDinosaur:
            return EQuest_MissionDifficulty::Veteran;
        case EQuest_SurvivalMissionType::DefendCamp:
            return EQuest_MissionDifficulty::Hunter;
        case EQuest_SurvivalMissionType::ExploreTerritory:
            return EQuest_MissionDifficulty::Hunter;
        case EQuest_SurvivalMissionType::RescueTribalMember:
            return EQuest_MissionDifficulty::Elder;
        default:
            return EQuest_MissionDifficulty::Novice;
    }
}

int32 AQuest_SurvivalMissionController::CalculateExperienceReward(EQuest_MissionDifficulty Difficulty) const
{
    switch (Difficulty)
    {
        case EQuest_MissionDifficulty::Novice:
            return FMath::RandRange(50, 100);
        case EQuest_MissionDifficulty::Hunter:
            return FMath::RandRange(100, 200);
        case EQuest_MissionDifficulty::Veteran:
            return FMath::RandRange(200, 350);
        case EQuest_MissionDifficulty::Elder:
            return FMath::RandRange(350, 500);
        case EQuest_MissionDifficulty::Legendary:
            return FMath::RandRange(500, 750);
        default:
            return 100;
    }
}
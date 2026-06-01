#include "Quest_SurvivalMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Character.h"

AQuest_SurvivalMissionSystem::AQuest_SurvivalMissionSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    MissionCheckInterval = 5.0f;
    MaxSimultaneousMissions = 3;
    CurrentProgress = 0;
    MissionTimer = 0.0f;
    
    // Set default mission data
    CurrentMission = FQuest_SurvivalMissionData();
}

void AQuest_SurvivalMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMissionTemplates();
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: System initialized with %d mission templates"), AvailableMissions.Num());
}

void AQuest_SurvivalMissionSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CurrentMission.bIsActive)
    {
        MissionTimer += DeltaTime;
        CheckMissionTimeout();
    }
}

void AQuest_SurvivalMissionSystem::StartMission(EQuest_SurvivalMissionType MissionType, const FVector& Location)
{
    if (CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Cannot start new mission - mission already active"));
        return;
    }
    
    CurrentMission = CreateMissionFromType(MissionType, Location);
    CurrentMission.bIsActive = true;
    CurrentMission.bIsCompleted = false;
    CurrentProgress = 0;
    MissionTimer = 0.0f;
    
    LogMissionEvent(TEXT("Mission Started"), CurrentMission.MissionName);
    OnMissionStarted(CurrentMission);
}

void AQuest_SurvivalMissionSystem::CompleteMission()
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    CurrentMission.bIsCompleted = true;
    
    LogMissionEvent(TEXT("Mission Completed"), CurrentMission.MissionName);
    OnMissionCompleted(CurrentMission);
    
    // Reset mission data
    CurrentMission = FQuest_SurvivalMissionData();
    CurrentProgress = 0;
    MissionTimer = 0.0f;
}

void AQuest_SurvivalMissionSystem::FailMission()
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    CurrentMission.bIsCompleted = false;
    
    LogMissionEvent(TEXT("Mission Failed"), CurrentMission.MissionName);
    OnMissionFailed(CurrentMission);
    
    // Reset mission data
    CurrentMission = FQuest_SurvivalMissionData();
    CurrentProgress = 0;
    MissionTimer = 0.0f;
}

bool AQuest_SurvivalMissionSystem::IsMissionActive() const
{
    return CurrentMission.bIsActive;
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionSystem::GetCurrentMission() const
{
    return CurrentMission;
}

void AQuest_SurvivalMissionSystem::GenerateRandomMission()
{
    if (CurrentMission.bIsActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Cannot generate random mission - mission already active"));
        return;
    }
    
    // Select random mission type
    TArray<EQuest_SurvivalMissionType> MissionTypes = {
        EQuest_SurvivalMissionType::GatherFood,
        EQuest_SurvivalMissionType::FindWater,
        EQuest_SurvivalMissionType::BuildShelter,
        EQuest_SurvivalMissionType::CraftTool,
        EQuest_SurvivalMissionType::HuntPrey,
        EQuest_SurvivalMissionType::ExploreArea
    };
    
    int32 RandomIndex = FMath::RandRange(0, MissionTypes.Num() - 1);
    EQuest_SurvivalMissionType SelectedType = MissionTypes[RandomIndex];
    
    // Generate random location within 1000 units of origin
    FVector RandomLocation = FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );
    
    StartMission(SelectedType, RandomLocation);
}

void AQuest_SurvivalMissionSystem::GenerateMissionBasedOnPlayerState()
{
    // Get player character
    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PlayerController)
    {
        GenerateRandomMission();
        return;
    }
    
    ACharacter* PlayerCharacter = PlayerController->GetCharacter();
    if (!PlayerCharacter)
    {
        GenerateRandomMission();
        return;
    }
    
    // For now, generate based on simple logic
    // In a full implementation, this would check player's survival stats
    EQuest_SurvivalMissionType SelectedType = EQuest_SurvivalMissionType::GatherFood;
    
    // Simple state-based mission selection
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    if (RandomValue < 0.3f)
    {
        SelectedType = EQuest_SurvivalMissionType::GatherFood;
    }
    else if (RandomValue < 0.5f)
    {
        SelectedType = EQuest_SurvivalMissionType::FindWater;
    }
    else if (RandomValue < 0.7f)
    {
        SelectedType = EQuest_SurvivalMissionType::CraftTool;
    }
    else
    {
        SelectedType = EQuest_SurvivalMissionType::ExploreArea;
    }
    
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    FVector MissionLocation = PlayerLocation + FVector(
        FMath::RandRange(-500.0f, 500.0f),
        FMath::RandRange(-500.0f, 500.0f),
        0.0f
    );
    
    StartMission(SelectedType, MissionLocation);
}

void AQuest_SurvivalMissionSystem::UpdateMissionProgress(int32 Progress)
{
    if (!CurrentMission.bIsActive)
    {
        return;
    }
    
    CurrentProgress = FMath::Clamp(Progress, 0, CurrentMission.TargetQuantity);
    
    float ProgressPercentage = (float)CurrentProgress / (float)CurrentMission.TargetQuantity;
    OnMissionProgressUpdated(ProgressPercentage);
    
    if (CurrentProgress >= CurrentMission.TargetQuantity)
    {
        CompleteMission();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Quest_SurvivalMissionSystem: Mission progress updated - %d/%d"), CurrentProgress, CurrentMission.TargetQuantity);
}

float AQuest_SurvivalMissionSystem::GetMissionProgress() const
{
    if (CurrentMission.TargetQuantity <= 0)
    {
        return 0.0f;
    }
    
    return (float)CurrentProgress / (float)CurrentMission.TargetQuantity;
}

void AQuest_SurvivalMissionSystem::InitializeMissionTemplates()
{
    AvailableMissions.Empty();
    
    // Gather Food Mission
    FQuest_SurvivalMissionData GatherFoodMission;
    GatherFoodMission.MissionName = TEXT("Gather Food");
    GatherFoodMission.Description = TEXT("Collect berries and edible plants to sustain yourself");
    GatherFoodMission.MissionType = EQuest_SurvivalMissionType::GatherFood;
    GatherFoodMission.TargetQuantity = 5;
    GatherFoodMission.TimeLimit = 300.0f;
    GatherFoodMission.DifficultyLevel = 1.0f;
    AvailableMissions.Add(GatherFoodMission);
    
    // Find Water Mission
    FQuest_SurvivalMissionData FindWaterMission;
    FindWaterMission.MissionName = TEXT("Find Water Source");
    FindWaterMission.Description = TEXT("Locate a clean water source to quench your thirst");
    FindWaterMission.MissionType = EQuest_SurvivalMissionType::FindWater;
    FindWaterMission.TargetQuantity = 1;
    FindWaterMission.TimeLimit = 600.0f;
    FindWaterMission.DifficultyLevel = 1.5f;
    AvailableMissions.Add(FindWaterMission);
    
    // Build Shelter Mission
    FQuest_SurvivalMissionData BuildShelterMission;
    BuildShelterMission.MissionName = TEXT("Build Shelter");
    BuildShelterMission.Description = TEXT("Construct a basic shelter to protect yourself from the elements");
    BuildShelterMission.MissionType = EQuest_SurvivalMissionType::BuildShelter;
    BuildShelterMission.TargetQuantity = 1;
    BuildShelterMission.TimeLimit = 900.0f;
    BuildShelterMission.DifficultyLevel = 2.0f;
    AvailableMissions.Add(BuildShelterMission);
    
    // Craft Tool Mission
    FQuest_SurvivalMissionData CraftToolMission;
    CraftToolMission.MissionName = TEXT("Craft Stone Tool");
    CraftToolMission.Description = TEXT("Create a basic stone tool for hunting and gathering");
    CraftToolMission.MissionType = EQuest_SurvivalMissionType::CraftTool;
    CraftToolMission.TargetQuantity = 1;
    CraftToolMission.TimeLimit = 240.0f;
    CraftToolMission.DifficultyLevel = 1.2f;
    AvailableMissions.Add(CraftToolMission);
    
    // Hunt Prey Mission
    FQuest_SurvivalMissionData HuntPreyMission;
    HuntPreyMission.MissionName = TEXT("Hunt Small Prey");
    HuntPreyMission.Description = TEXT("Track and hunt small animals for meat and materials");
    HuntPreyMission.MissionType = EQuest_SurvivalMissionType::HuntPrey;
    HuntPreyMission.TargetQuantity = 2;
    HuntPreyMission.TimeLimit = 720.0f;
    HuntPreyMission.DifficultyLevel = 2.5f;
    AvailableMissions.Add(HuntPreyMission);
    
    // Explore Area Mission
    FQuest_SurvivalMissionData ExploreAreaMission;
    ExploreAreaMission.MissionName = TEXT("Explore Territory");
    ExploreAreaMission.Description = TEXT("Scout the surrounding area and discover new locations");
    ExploreAreaMission.MissionType = EQuest_SurvivalMissionType::ExploreArea;
    ExploreAreaMission.TargetQuantity = 3;
    ExploreAreaMission.TimeLimit = 480.0f;
    ExploreAreaMission.DifficultyLevel = 1.8f;
    AvailableMissions.Add(ExploreAreaMission);
    
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Initialized %d mission templates"), AvailableMissions.Num());
}

void AQuest_SurvivalMissionSystem::CheckMissionTimeout()
{
    if (CurrentMission.bIsActive && MissionTimer >= CurrentMission.TimeLimit)
    {
        UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: Mission timed out - %s"), *CurrentMission.MissionName);
        FailMission();
    }
}

FQuest_SurvivalMissionData AQuest_SurvivalMissionSystem::CreateMissionFromType(EQuest_SurvivalMissionType Type, const FVector& Location)
{
    // Find template for this mission type
    for (const FQuest_SurvivalMissionData& Template : AvailableMissions)
    {
        if (Template.MissionType == Type)
        {
            FQuest_SurvivalMissionData NewMission = Template;
            NewMission.TargetLocation = Location;
            return NewMission;
        }
    }
    
    // Fallback - create basic mission
    FQuest_SurvivalMissionData FallbackMission;
    FallbackMission.MissionName = TEXT("Basic Survival Task");
    FallbackMission.Description = TEXT("Complete a basic survival objective");
    FallbackMission.MissionType = Type;
    FallbackMission.TargetQuantity = 1;
    FallbackMission.TimeLimit = 300.0f;
    FallbackMission.DifficultyLevel = 1.0f;
    FallbackMission.TargetLocation = Location;
    
    return FallbackMission;
}

void AQuest_SurvivalMissionSystem::LogMissionEvent(const FString& Event, const FString& Details)
{
    UE_LOG(LogTemp, Warning, TEXT("Quest_SurvivalMissionSystem: %s - %s"), *Event, *Details);
    
    if (GEngine)
    {
        FString LogMessage = FString::Printf(TEXT("Mission System: %s - %s"), *Event, *Details);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, LogMessage);
    }
}
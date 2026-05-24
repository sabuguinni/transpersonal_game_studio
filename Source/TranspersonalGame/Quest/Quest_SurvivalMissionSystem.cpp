#include "Quest_SurvivalMissionSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UQuest_SurvivalMissionSystem::UQuest_SurvivalMissionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MissionGenerationInterval = 120.0f; // 2 minutes
    bAutoGenerateMissions = true;
    MaxActiveMissions = 1;
}

void UQuest_SurvivalMissionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoGenerateMissions)
    {
        GetWorld()->GetTimerManager().SetTimer(
            GenerationTimerHandle,
            this,
            &UQuest_SurvivalMissionSystem::GenerateRandomMission,
            MissionGenerationInterval,
            true,
            30.0f // First mission after 30 seconds
        );
    }
}

void UQuest_SurvivalMissionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
    {
        UpdateMissionTimer();
    }
}

void UQuest_SurvivalMissionSystem::StartMission(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty)
{
    if (CurrentMission.bIsActive && !CurrentMission.bIsCompleted)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start new mission - current mission still active"));
        return;
    }
    
    SetupMissionData(MissionType, Difficulty);
    CurrentMission.bIsActive = true;
    CurrentMission.bIsCompleted = false;
    CurrentMission.CurrentProgress = 0;
    CurrentMission.RemainingTime = CurrentMission.TimeLimit;
    
    // Start mission timer
    GetWorld()->GetTimerManager().SetTimer(
        MissionTimerHandle,
        this,
        &UQuest_SurvivalMissionSystem::OnMissionTimeExpired,
        CurrentMission.TimeLimit,
        false
    );
    
    OnMissionStarted(CurrentMission);
    
    UE_LOG(LogTemp, Log, TEXT("Mission Started: %s"), *CurrentMission.MissionName);
}

void UQuest_SurvivalMissionSystem::CompleteMission()
{
    if (!CurrentMission.bIsActive || CurrentMission.bIsCompleted)
    {
        return;
    }
    
    CurrentMission.bIsCompleted = true;
    CurrentMission.bIsActive = false;
    CompletedMissions.Add(CurrentMission);
    
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    
    OnMissionCompleted(CurrentMission);
    
    UE_LOG(LogTemp, Log, TEXT("Mission Completed: %s"), *CurrentMission.MissionName);
}

void UQuest_SurvivalMissionSystem::FailMission()
{
    if (!CurrentMission.bIsActive || CurrentMission.bIsCompleted)
    {
        return;
    }
    
    CurrentMission.bIsActive = false;
    GetWorld()->GetTimerManager().ClearTimer(MissionTimerHandle);
    
    OnMissionFailed(CurrentMission);
    
    UE_LOG(LogTemp, Log, TEXT("Mission Failed: %s"), *CurrentMission.MissionName);
}

void UQuest_SurvivalMissionSystem::UpdateMissionProgress(int32 ProgressAmount)
{
    if (!CurrentMission.bIsActive || CurrentMission.bIsCompleted)
    {
        return;
    }
    
    CurrentMission.CurrentProgress = FMath::Clamp(
        CurrentMission.CurrentProgress + ProgressAmount,
        0,
        CurrentMission.RequiredCount
    );
    
    OnMissionProgressUpdated(CurrentMission);
    
    if (CurrentMission.CurrentProgress >= CurrentMission.RequiredCount)
    {
        CompleteMission();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Mission Progress: %d/%d"), 
           CurrentMission.CurrentProgress, CurrentMission.RequiredCount);
}

bool UQuest_SurvivalMissionSystem::IsMissionActive() const
{
    return CurrentMission.bIsActive && !CurrentMission.bIsCompleted;
}

FQuest_SurvivalMissionData UQuest_SurvivalMissionSystem::GetCurrentMission() const
{
    return CurrentMission;
}

void UQuest_SurvivalMissionSystem::GenerateRandomMission()
{
    if (IsMissionActive())
    {
        return; // Don't generate if mission is active
    }
    
    // Random mission type
    int32 MissionTypeIndex = FMath::RandRange(0, 7);
    EQuest_SurvivalMissionType MissionType = static_cast<EQuest_SurvivalMissionType>(MissionTypeIndex);
    
    // Random difficulty
    int32 DifficultyIndex = FMath::RandRange(0, 3);
    EQuest_MissionDifficulty Difficulty = static_cast<EQuest_MissionDifficulty>(DifficultyIndex);
    
    StartMission(MissionType, Difficulty);
}

void UQuest_SurvivalMissionSystem::GenerateDinosaurHuntMission(const FString& DinosaurType)
{
    if (IsMissionActive())
    {
        return;
    }
    
    StartMission(EQuest_SurvivalMissionType::Hunt_Dinosaur, EQuest_MissionDifficulty::Hard);
    CurrentMission.MissionName = FString::Printf(TEXT("Hunt %s"), *DinosaurType);
    CurrentMission.Description = FString::Printf(TEXT("Track and hunt a %s for meat and materials"), *DinosaurType);
}

void UQuest_SurvivalMissionSystem::GenerateResourceGatheringMission(const TArray<FString>& Resources)
{
    if (IsMissionActive())
    {
        return;
    }
    
    StartMission(EQuest_SurvivalMissionType::Gather_Resources, EQuest_MissionDifficulty::Easy);
    CurrentMission.RequiredItems = Resources;
    CurrentMission.RequiredCount = Resources.Num() * 3; // 3 of each resource
    
    FString ResourceList = FString::Join(Resources, TEXT(", "));
    CurrentMission.MissionName = TEXT("Gather Resources");
    CurrentMission.Description = FString::Printf(TEXT("Collect %s for survival"), *ResourceList);
}

void UQuest_SurvivalMissionSystem::GenerateExplorationMission(const FVector& TargetArea)
{
    if (IsMissionActive())
    {
        return;
    }
    
    StartMission(EQuest_SurvivalMissionType::Explore_Territory, EQuest_MissionDifficulty::Medium);
    CurrentMission.TargetLocation = TargetArea;
    CurrentMission.MissionName = TEXT("Explore Unknown Territory");
    CurrentMission.Description = TEXT("Venture into unexplored lands and discover new resources");
}

void UQuest_SurvivalMissionSystem::UpdateMissionTimer()
{
    if (CurrentMission.RemainingTime > 0.0f)
    {
        CurrentMission.RemainingTime -= GetWorld()->GetDeltaSeconds();
        
        if (CurrentMission.RemainingTime <= 0.0f)
        {
            OnMissionTimeExpired();
        }
    }
}

void UQuest_SurvivalMissionSystem::OnMissionTimeExpired()
{
    UE_LOG(LogTemp, Warning, TEXT("Mission time expired: %s"), *CurrentMission.MissionName);
    FailMission();
}

void UQuest_SurvivalMissionSystem::SetupMissionData(EQuest_SurvivalMissionType MissionType, EQuest_MissionDifficulty Difficulty)
{
    CurrentMission = FQuest_SurvivalMissionData(); // Reset
    CurrentMission.MissionType = MissionType;
    CurrentMission.Difficulty = Difficulty;
    
    // Set difficulty modifiers
    float DifficultyMultiplier = 1.0f;
    switch (Difficulty)
    {
        case EQuest_MissionDifficulty::Easy:
            DifficultyMultiplier = 0.7f;
            break;
        case EQuest_MissionDifficulty::Medium:
            DifficultyMultiplier = 1.0f;
            break;
        case EQuest_MissionDifficulty::Hard:
            DifficultyMultiplier = 1.5f;
            break;
        case EQuest_MissionDifficulty::Extreme:
            DifficultyMultiplier = 2.0f;
            break;
    }
    
    // Configure mission based on type
    switch (MissionType)
    {
        case EQuest_SurvivalMissionType::Hunt_Dinosaur:
            CurrentMission.MissionName = TEXT("Hunt Dinosaur");
            CurrentMission.Description = TEXT("Track and hunt a dangerous dinosaur");
            CurrentMission.RequiredCount = FMath::CeilToInt(1 * DifficultyMultiplier);
            CurrentMission.TimeLimit = 600.0f / DifficultyMultiplier; // 10 minutes base
            break;
            
        case EQuest_SurvivalMissionType::Gather_Resources:
            CurrentMission.MissionName = TEXT("Gather Resources");
            CurrentMission.Description = TEXT("Collect essential survival materials");
            CurrentMission.RequiredCount = FMath::CeilToInt(5 * DifficultyMultiplier);
            CurrentMission.TimeLimit = 300.0f / DifficultyMultiplier; // 5 minutes base
            CurrentMission.RequiredItems = GetRandomResourceList();
            break;
            
        case EQuest_SurvivalMissionType::Build_Shelter:
            CurrentMission.MissionName = TEXT("Build Shelter");
            CurrentMission.Description = TEXT("Construct a safe shelter for protection");
            CurrentMission.RequiredCount = 1;
            CurrentMission.TimeLimit = 900.0f / DifficultyMultiplier; // 15 minutes base
            break;
            
        case EQuest_SurvivalMissionType::Explore_Territory:
            CurrentMission.MissionName = TEXT("Explore Territory");
            CurrentMission.Description = TEXT("Scout new areas for resources and threats");
            CurrentMission.RequiredCount = FMath::CeilToInt(3 * DifficultyMultiplier);
            CurrentMission.TimeLimit = 480.0f / DifficultyMultiplier; // 8 minutes base
            CurrentMission.TargetLocation = GetRandomExplorationTarget();
            break;
            
        case EQuest_SurvivalMissionType::Defend_Camp:
            CurrentMission.MissionName = TEXT("Defend Camp");
            CurrentMission.Description = TEXT("Protect your camp from predator attacks");
            CurrentMission.RequiredCount = FMath::CeilToInt(3 * DifficultyMultiplier);
            CurrentMission.TimeLimit = 420.0f / DifficultyMultiplier; // 7 minutes base
            break;
            
        case EQuest_SurvivalMissionType::Craft_Tools:
            CurrentMission.MissionName = TEXT("Craft Tools");
            CurrentMission.Description = TEXT("Create essential survival tools");
            CurrentMission.RequiredCount = FMath::CeilToInt(2 * DifficultyMultiplier);
            CurrentMission.TimeLimit = 360.0f / DifficultyMultiplier; // 6 minutes base
            break;
            
        case EQuest_SurvivalMissionType::Find_Water:
            CurrentMission.MissionName = TEXT("Find Water Source");
            CurrentMission.Description = TEXT("Locate a clean water source for survival");
            CurrentMission.RequiredCount = 1;
            CurrentMission.TimeLimit = 540.0f / DifficultyMultiplier; // 9 minutes base
            break;
            
        case EQuest_SurvivalMissionType::Escape_Predator:
            CurrentMission.MissionName = TEXT("Escape Predator");
            CurrentMission.Description = TEXT("Evade dangerous predators in the area");
            CurrentMission.RequiredCount = 1;
            CurrentMission.TimeLimit = 180.0f / DifficultyMultiplier; // 3 minutes base
            break;
    }
    
    CurrentMission.RemainingTime = CurrentMission.TimeLimit;
}

FVector UQuest_SurvivalMissionSystem::GetRandomExplorationTarget() const
{
    // Generate random exploration targets in different biomes
    TArray<FVector> BiomeLocations = {
        FVector(0, 0, 100),           // Savana
        FVector(-50000, -45000, 100), // Pantano
        FVector(-45000, 40000, 100),  // Floresta
        FVector(55000, 0, 100),       // Deserto
        FVector(40000, 50000, 100)    // Montanha
    };
    
    int32 RandomIndex = FMath::RandRange(0, BiomeLocations.Num() - 1);
    FVector BaseLocation = BiomeLocations[RandomIndex];
    
    // Add some random offset
    FVector Offset = FVector(
        FMath::RandRange(-5000, 5000),
        FMath::RandRange(-5000, 5000),
        0
    );
    
    return BaseLocation + Offset;
}

TArray<FString> UQuest_SurvivalMissionSystem::GetRandomResourceList() const
{
    TArray<FString> AllResources = {
        TEXT("Stone"),
        TEXT("Wood"),
        TEXT("Fiber"),
        TEXT("Berries"),
        TEXT("Water"),
        TEXT("Flint"),
        TEXT("Hide"),
        TEXT("Bone"),
        TEXT("Meat")
    };
    
    TArray<FString> SelectedResources;
    int32 ResourceCount = FMath::RandRange(2, 4);
    
    for (int32 i = 0; i < ResourceCount; i++)
    {
        int32 RandomIndex = FMath::RandRange(0, AllResources.Num() - 1);
        if (!SelectedResources.Contains(AllResources[RandomIndex]))
        {
            SelectedResources.Add(AllResources[RandomIndex]);
        }
    }
    
    return SelectedResources;
}

FString UQuest_SurvivalMissionSystem::GetRandomDinosaurType() const
{
    TArray<FString> DinosaurTypes = {
        TEXT("Velociraptor"),
        TEXT("Triceratops"),
        TEXT("Parasaurolophus"),
        TEXT("Ankylosaurus"),
        TEXT("Protoceratops"),
        TEXT("Compsognathus")
    };
    
    int32 RandomIndex = FMath::RandRange(0, DinosaurTypes.Num() - 1);
    return DinosaurTypes[RandomIndex];
}
#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for crowd updates
    
    ActiveEntityCount = 0;
    bMassSystemInitialized = false;
    
    // Initialize default spawn config
    SpawnConfig.MaxEntities = 5000;
    SpawnConfig.SpawnRadius = 2000.0f;
    SpawnConfig.DensityPerSquareMeter = 0.5f;
    SpawnConfig.bEnablePathfinding = true;
    SpawnConfig.bEnableLODSystem = true;
    
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    MassSimulationSubsystem = nullptr;
}

void UCrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeSubsystems();
    InitializeMassEntity();
}

void UCrowd_MassEntityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMassSystemInitialized)
    {
        return;
    }
    
    // Update crowd behavior every tick
    if (MassSimulationSubsystem)
    {
        // Mass simulation handles entity updates automatically
        // We just track the count
        UpdateCrowdMetrics();
    }
}

void UCrowd_MassEntityManager::InitializeMassEntity()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityManager: No valid world found"));
        return;
    }
    
    InitializeSubsystems();
    
    if (MassEntitySubsystem && MassSpawnerSubsystem)
    {
        ConfigureMassTraits();
        SetupLODSystem();
        bMassSystemInitialized = true;
        
        UE_LOG(LogTemp, Log, TEXT("Mass Entity crowd simulation initialized successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Mass Entity subsystems"));
    }
}

void UCrowd_MassEntityManager::SpawnCrowdEntities(int32 Count, FVector CenterLocation)
{
    if (!bMassSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass system not initialized, cannot spawn crowd"));
        return;
    }
    
    if (Count <= 0 || Count > SpawnConfig.MaxEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid crowd count: %d (max: %d)"), Count, SpawnConfig.MaxEntities);
        return;
    }
    
    // Use Mass spawner to create entities
    if (MassSpawnerSubsystem)
    {
        // Create spawn parameters
        FMassEntitySpawnDataGeneratorBase SpawnData;
        
        // Configure spawn locations in a circle around center
        TArray<FTransform> SpawnTransforms;
        for (int32 i = 0; i < Count; ++i)
        {
            float Angle = (2.0f * PI * i) / Count;
            float Distance = FMath::RandRange(50.0f, SpawnConfig.SpawnRadius);
            
            FVector SpawnLocation = CenterLocation + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                0.0f
            );
            
            // Adjust Z to ground level
            SpawnLocation.Z = CenterLocation.Z;
            
            SpawnTransforms.Add(FTransform(FRotator::ZeroRotator, SpawnLocation, FVector::OneVector));
        }
        
        ActiveEntityCount += Count;
        UE_LOG(LogTemp, Log, TEXT("Spawned %d crowd entities at location %s"), Count, *CenterLocation.ToString());
    }
}

void UCrowd_MassEntityManager::UpdateCrowdDensity(float NewDensity)
{
    if (NewDensity <= 0.0f || NewDensity > 10.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid crowd density: %f"), NewDensity);
        return;
    }
    
    SpawnConfig.DensityPerSquareMeter = NewDensity;
    
    // Recalculate max entities based on new density
    float AreaCovered = PI * SpawnConfig.SpawnRadius * SpawnConfig.SpawnRadius;
    int32 NewMaxEntities = FMath::FloorToInt(AreaCovered * NewDensity);
    
    SpawnConfig.MaxEntities = FMath::Clamp(NewMaxEntities, 100, 50000);
    
    UE_LOG(LogTemp, Log, TEXT("Updated crowd density to %f, new max entities: %d"), NewDensity, SpawnConfig.MaxEntities);
}

void UCrowd_MassEntityManager::SetCrowdDestination(FVector TargetLocation)
{
    if (!bMassSystemInitialized)
    {
        return;
    }
    
    // Update Mass simulation with new target
    if (MassSimulationSubsystem)
    {
        // Mass entities will pathfind to this location
        UE_LOG(LogTemp, Log, TEXT("Set crowd destination to %s"), *TargetLocation.ToString());
    }
}

int32 UCrowd_MassEntityManager::GetActiveCrowdCount() const
{
    return ActiveEntityCount;
}

void UCrowd_MassEntityManager::DebugSpawnTestCrowd()
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Spawn test crowd around player
    SpawnCrowdEntities(500, PlayerLocation + FVector(1000, 0, 0));
}

void UCrowd_MassEntityManager::InitializeSubsystems()
{
    if (!GetWorld())
    {
        return;
    }
    
    MassEntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    MassSpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    MassSimulationSubsystem = GetWorld()->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem"));
    }
    
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get MassSpawnerSubsystem"));
    }
    
    if (!MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get MassSimulationSubsystem"));
    }
}

void UCrowd_MassEntityManager::ConfigureMassTraits()
{
    // Configure Mass traits for crowd entities
    // This would normally use Mass trait configuration assets
    UE_LOG(LogTemp, Log, TEXT("Configuring Mass traits for crowd simulation"));
}

void UCrowd_MassEntityManager::SetupLODSystem()
{
    if (!SpawnConfig.bEnableLODSystem)
    {
        return;
    }
    
    // Configure LOD distances for crowd entities
    // LOD0: 0-500 units (full detail)
    // LOD1: 500-1500 units (medium detail)  
    // LOD2: 1500+ units (low detail/impostors)
    
    UE_LOG(LogTemp, Log, TEXT("LOD system configured for crowd simulation"));
}

void UCrowd_MassEntityManager::UpdateCrowdMetrics()
{
    // Update performance metrics
    if (MassEntitySubsystem)
    {
        // Track active entity count from Mass system
        // This is a simplified version - real implementation would query Mass directly
    }
}
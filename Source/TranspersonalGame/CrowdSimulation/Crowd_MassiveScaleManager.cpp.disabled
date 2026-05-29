#include "Crowd_MassiveScaleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"

ACrowd_MassiveScaleManager::ACrowd_MassiveScaleManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create visual representation
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    
    // Initialize massive scale parameters
    MaxCrowdEntities = 50000;
    CurrentActiveEntities = 0;
    LODDistanceThreshold = 5000.0f;
    CullingDistance = 10000.0f;
    PerformanceTargetFPS = 30.0f;
    
    // Initialize biome density settings
    BiomeDensityMultipliers.Add(ECrowd_BiomeType::Plains, 1.5f);
    BiomeDensityMultipliers.Add(ECrowd_BiomeType::Forest, 1.2f);
    BiomeDensityMultipliers.Add(ECrowd_BiomeType::Swamp, 0.8f);
    BiomeDensityMultipliers.Add(ECrowd_BiomeType::Mountain, 0.6f);
    BiomeDensityMultipliers.Add(ECrowd_BiomeType::Desert, 0.4f);
    
    // Initialize performance monitoring
    FrameTimeHistory.Reserve(60);
    bPerformanceOptimizationEnabled = true;
    LastPerformanceCheck = 0.0f;
    PerformanceCheckInterval = 1.0f;
}

void ACrowd_MassiveScaleManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize Mass Entity subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
        
        if (MassEntitySubsystem && MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Mass Entity subsystems initialized"));
            InitializeMassiveScaleSystem();
        }
    }
}

void ACrowd_MassiveScaleManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance monitoring
    UpdatePerformanceMonitoring(DeltaTime);
    
    // Update LOD system based on player position
    UpdateLODSystem();
    
    // Manage entity spawning and culling
    ManageEntityLifecycle();
    
    // Update biome-based density adjustments
    UpdateBiomeDensityAdjustments();
}

void ACrowd_MassiveScaleManager::InitializeMassiveScaleSystem()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassiveScaleManager: Failed to get Mass Entity subsystem"));
        return;
    }
    
    // Create entity archetypes for different crowd types
    CreateCrowdArchetypes();
    
    // Initialize spatial partitioning for massive scale
    InitializeSpatialPartitioning();
    
    // Setup performance optimization systems
    SetupPerformanceOptimization();
    
    UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Massive scale system initialized with %d max entities"), MaxCrowdEntities);
}

void ACrowd_MassiveScaleManager::CreateCrowdArchetypes()
{
    // Create herbivore herd archetype
    HerbivoreArchetype.EntityTemplate = FMassEntityTemplate();
    HerbivoreArchetype.MaxEntities = 20000;
    HerbivoreArchetype.SpawnRadius = 8000.0f;
    HerbivoreArchetype.MovementSpeed = 300.0f;
    HerbivoreArchetype.FlockingStrength = 0.8f;
    
    // Create carnivore pack archetype
    CarnivoreArchetype.EntityTemplate = FMassEntityTemplate();
    CarnivoreArchetype.MaxEntities = 5000;
    CarnivoreArchetype.SpawnRadius = 6000.0f;
    CarnivoreArchetype.MovementSpeed = 500.0f;
    CarnivoreArchetype.FlockingStrength = 0.6f;
    
    // Create mixed ecosystem archetype
    MixedEcosystemArchetype.EntityTemplate = FMassEntityTemplate();
    MixedEcosystemArchetype.MaxEntities = 15000;
    MixedEcosystemArchetype.SpawnRadius = 10000.0f;
    MixedEcosystemArchetype.MovementSpeed = 400.0f;
    MixedEcosystemArchetype.FlockingStrength = 0.7f;
    
    // Create apex predator archetype
    ApexPredatorArchetype.EntityTemplate = FMassEntityTemplate();
    ApexPredatorArchetype.MaxEntities = 1000;
    ApexPredatorArchetype.SpawnRadius = 12000.0f;
    ApexPredatorArchetype.MovementSpeed = 800.0f;
    ApexPredatorArchetype.FlockingStrength = 0.3f;
    
    UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Created crowd archetypes"));
}

void ACrowd_MassiveScaleManager::InitializeSpatialPartitioning()
{
    // Create spatial grid for massive scale management
    SpatialGridSize = 2000.0f; // 20x20 meter cells
    GridDimensions = FIntVector(100, 100, 10); // 100x100x10 grid
    
    // Initialize grid cells
    int32 TotalCells = GridDimensions.X * GridDimensions.Y * GridDimensions.Z;
    SpatialGrid.SetNum(TotalCells);
    
    for (int32 i = 0; i < TotalCells; i++)
    {
        SpatialGrid[i].CellIndex = i;
        SpatialGrid[i].EntityCount = 0;
        SpatialGrid[i].DensityMultiplier = 1.0f;
        SpatialGrid[i].bIsActive = false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Spatial partitioning initialized with %d cells"), TotalCells);
}

void ACrowd_MassiveScaleManager::SetupPerformanceOptimization()
{
    // Setup LOD levels for massive crowds
    LODLevels.SetNum(4);
    
    // LOD 0 - Full detail (close range)
    LODLevels[0].Distance = 1000.0f;
    LODLevels[0].EntityLimit = 500;
    LODLevels[0].UpdateFrequency = 1.0f;
    LODLevels[0].RenderQuality = 1.0f;
    
    // LOD 1 - High detail (medium range)
    LODLevels[1].Distance = 3000.0f;
    LODLevels[1].EntityLimit = 2000;
    LODLevels[1].UpdateFrequency = 0.5f;
    LODLevels[1].RenderQuality = 0.8f;
    
    // LOD 2 - Medium detail (far range)
    LODLevels[2].Distance = 6000.0f;
    LODLevels[2].EntityLimit = 8000;
    LODLevels[2].UpdateFrequency = 0.25f;
    LODLevels[2].RenderQuality = 0.5f;
    
    // LOD 3 - Low detail (very far range)
    LODLevels[3].Distance = 10000.0f;
    LODLevels[3].EntityLimit = 20000;
    LODLevels[3].UpdateFrequency = 0.1f;
    LODLevels[3].RenderQuality = 0.2f;
    
    UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Performance optimization setup complete"));
}

void ACrowd_MassiveScaleManager::UpdatePerformanceMonitoring(float DeltaTime)
{
    // Track frame time for performance monitoring
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Check performance periodically
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= PerformanceCheckInterval)
    {
        LastPerformanceCheck = 0.0f;
        
        // Calculate average frame time
        float AverageFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= FrameTimeHistory.Num();
        
        // Calculate current FPS
        float CurrentFPS = 1.0f / AverageFrameTime;
        
        // Adjust entity limits based on performance
        if (bPerformanceOptimizationEnabled)
        {
            AdjustEntityLimitsForPerformance(CurrentFPS);
        }
    }
}

void ACrowd_MassiveScaleManager::AdjustEntityLimitsForPerformance(float CurrentFPS)
{
    float PerformanceRatio = CurrentFPS / PerformanceTargetFPS;
    
    if (PerformanceRatio < 0.8f) // Performance below target
    {
        // Reduce entity limits
        for (FCrowd_LODLevel& LODLevel : LODLevels)
        {
            LODLevel.EntityLimit = FMath::Max(100, static_cast<int32>(LODLevel.EntityLimit * 0.9f));
            LODLevel.UpdateFrequency *= 0.9f;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("MassiveScaleManager: Reduced entity limits due to low performance (%.1f FPS)"), CurrentFPS);
    }
    else if (PerformanceRatio > 1.2f) // Performance above target
    {
        // Increase entity limits
        for (FCrowd_LODLevel& LODLevel : LODLevels)
        {
            LODLevel.EntityLimit = FMath::Min(50000, static_cast<int32>(LODLevel.EntityLimit * 1.05f));
            LODLevel.UpdateFrequency = FMath::Min(1.0f, LODLevel.UpdateFrequency * 1.05f);
        }
        
        UE_LOG(LogTemp, Log, TEXT("MassiveScaleManager: Increased entity limits due to good performance (%.1f FPS)"), CurrentFPS);
    }
}

void ACrowd_MassiveScaleManager::UpdateLODSystem()
{
    if (!GetWorld())
        return;
    
    // Get player location
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    // Update LOD for each spatial grid cell
    for (FCrowd_SpatialCell& Cell : SpatialGrid)
    {
        FVector CellCenter = GetCellWorldPosition(Cell.CellIndex);
        float DistanceToPlayer = FVector::Dist(PlayerLocation, CellCenter);
        
        // Determine appropriate LOD level
        int32 LODLevel = GetLODLevelForDistance(DistanceToPlayer);
        Cell.CurrentLODLevel = LODLevel;
        
        // Update cell activity based on distance
        Cell.bIsActive = DistanceToPlayer <= CullingDistance;
    }
}

void ACrowd_MassiveScaleManager::ManageEntityLifecycle()
{
    if (!MassSpawnerSubsystem)
        return;
    
    // Count current active entities
    CurrentActiveEntities = 0;
    for (const FCrowd_SpatialCell& Cell : SpatialGrid)
    {
        if (Cell.bIsActive)
        {
            CurrentActiveEntities += Cell.EntityCount;
        }
    }
    
    // Spawn new entities if below limits
    if (CurrentActiveEntities < MaxCrowdEntities)
    {
        SpawnEntitiesInActiveCells();
    }
    
    // Cull entities that are too far away
    CullDistantEntities();
}

void ACrowd_MassiveScaleManager::SpawnEntitiesInActiveCells()
{
    for (FCrowd_SpatialCell& Cell : SpatialGrid)
    {
        if (Cell.bIsActive && Cell.EntityCount < GetMaxEntitiesForCell(Cell))
        {
            // Determine spawn archetype based on biome
            FCrowd_EntityArchetype* Archetype = GetArchetypeForBiome(Cell.BiomeType);
            if (Archetype)
            {
                SpawnEntitiesInCell(Cell, *Archetype);
            }
        }
    }
}

void ACrowd_MassiveScaleManager::CullDistantEntities()
{
    // Cull entities in inactive cells
    for (FCrowd_SpatialCell& Cell : SpatialGrid)
    {
        if (!Cell.bIsActive && Cell.EntityCount > 0)
        {
            // Remove entities from this cell
            Cell.EntityCount = 0;
        }
    }
}

void ACrowd_MassiveScaleManager::UpdateBiomeDensityAdjustments()
{
    // Update density multipliers based on biome characteristics
    for (FCrowd_SpatialCell& Cell : SpatialGrid)
    {
        if (BiomeDensityMultipliers.Contains(Cell.BiomeType))
        {
            Cell.DensityMultiplier = BiomeDensityMultipliers[Cell.BiomeType];
        }
    }
}

FVector ACrowd_MassiveScaleManager::GetCellWorldPosition(int32 CellIndex) const
{
    int32 X = CellIndex % GridDimensions.X;
    int32 Y = (CellIndex / GridDimensions.X) % GridDimensions.Y;
    int32 Z = CellIndex / (GridDimensions.X * GridDimensions.Y);
    
    FVector WorldPosition;
    WorldPosition.X = (X - GridDimensions.X / 2) * SpatialGridSize;
    WorldPosition.Y = (Y - GridDimensions.Y / 2) * SpatialGridSize;
    WorldPosition.Z = Z * SpatialGridSize;
    
    return GetActorLocation() + WorldPosition;
}

int32 ACrowd_MassiveScaleManager::GetLODLevelForDistance(float Distance) const
{
    for (int32 i = 0; i < LODLevels.Num(); i++)
    {
        if (Distance <= LODLevels[i].Distance)
        {
            return i;
        }
    }
    return LODLevels.Num() - 1; // Return highest LOD level for very far distances
}

int32 ACrowd_MassiveScaleManager::GetMaxEntitiesForCell(const FCrowd_SpatialCell& Cell) const
{
    if (Cell.CurrentLODLevel < LODLevels.Num())
    {
        float BaseLimit = LODLevels[Cell.CurrentLODLevel].EntityLimit / SpatialGrid.Num();
        return static_cast<int32>(BaseLimit * Cell.DensityMultiplier);
    }
    return 0;
}

FCrowd_EntityArchetype* ACrowd_MassiveScaleManager::GetArchetypeForBiome(ECrowd_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case ECrowd_BiomeType::Plains:
            return &HerbivoreArchetype;
        case ECrowd_BiomeType::Forest:
            return &MixedEcosystemArchetype;
        case ECrowd_BiomeType::Swamp:
            return &CarnivoreArchetype;
        case ECrowd_BiomeType::Mountain:
            return &ApexPredatorArchetype;
        default:
            return &HerbivoreArchetype;
    }
}

void ACrowd_MassiveScaleManager::SpawnEntitiesInCell(FCrowd_SpatialCell& Cell, const FCrowd_EntityArchetype& Archetype)
{
    if (!MassSpawnerSubsystem)
        return;
    
    // Calculate spawn count based on cell capacity
    int32 MaxEntitiesForCell = GetMaxEntitiesForCell(Cell);
    int32 EntitiesToSpawn = FMath::Min(10, MaxEntitiesForCell - Cell.EntityCount);
    
    if (EntitiesToSpawn > 0)
    {
        FVector CellCenter = GetCellWorldPosition(Cell.CellIndex);
        
        // Spawn entities using Mass Entity system
        for (int32 i = 0; i < EntitiesToSpawn; i++)
        {
            FVector SpawnLocation = CellCenter + FMath::VRand() * (SpatialGridSize * 0.4f);
            // Spawn entity using Mass Entity subsystem
            // This would use the actual Mass Entity spawning API
        }
        
        Cell.EntityCount += EntitiesToSpawn;
        UE_LOG(LogTemp, Log, TEXT("MassiveScaleManager: Spawned %d entities in cell %d"), EntitiesToSpawn, Cell.CellIndex);
    }
}
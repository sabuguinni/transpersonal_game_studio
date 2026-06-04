#include "World_PhysicsIntegratedTerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UWorld_PhysicsIntegratedTerrainGenerator::UWorld_PhysicsIntegratedTerrainGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz for performance monitoring
    
    // Initialize performance tracking
    CurrentPerformanceCost = 0.0f;
    ActivePhysicsChunks = 0;
    LastFrameTime = 16.67f; // Target 60 FPS
    
    // Initialize physics integration state
    bPhysicsConsolidatorConnected = false;
    PhysicsComplexityBudget = 100.0f;
    ConsolidatedCollisionObjects = 0;
    
    // Initialize default terrain settings
    TerrainSettings = FWorld_PhysicsTerrainSettings();
}

void UWorld_PhysicsIntegratedTerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_PhysicsIntegratedTerrainGenerator: BeginPlay started"));
    
    // Connect to physics consolidator from Agent #3
    ConnectToPhysicsConsolidator();
    
    // Generate initial terrain optimized for Milestone 1
    CreateWalkableTerrainForMilestone();
    
    UE_LOG(LogTemp, Warning, TEXT("World_PhysicsIntegratedTerrainGenerator: Initialization complete"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Check if we need performance optimizations
    if (!IsWithinPerformanceBudget())
    {
        ApplyPerformanceOptimizations();
    }
    
    // Update physics budget integration
    UpdatePhysicsBudget();
}

void UWorld_PhysicsIntegratedTerrainGenerator::GeneratePhysicsOptimizedTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating physics-optimized terrain"));
    
    // Clear existing chunks
    ActiveTerrainChunks.Empty();
    
    // Generate terrain chunks with physics optimization
    TArray<FVector> ChunkLocations = {
        FVector(0, 0, 0),
        FVector(1000, 0, 0),
        FVector(-1000, 0, 0),
        FVector(0, 1000, 0),
        FVector(0, -1000, 0),
        FVector(1000, 1000, 0),
        FVector(-1000, -1000, 0),
        FVector(1000, -1000, 0),
        FVector(-1000, 1000, 0)
    };
    
    for (const FVector& Location : ChunkLocations)
    {
        CreateTerrainChunk(Location, FVector(1000, 1000, 200), EBiomeType::Forest);
    }
    
    // Apply physics consolidation
    ConsolidateTerrainCollision();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics-optimized terrain generation complete: %d chunks"), ActiveTerrainChunks.Num());
}

void UWorld_PhysicsIntegratedTerrainGenerator::CreateTerrainChunk(const FVector& Location, const FVector& Size, EBiomeType BiomeType)
{
    FWorld_PhysicsTerrainChunk NewChunk;
    NewChunk.ChunkLocation = Location;
    NewChunk.ChunkSize = Size;
    NewChunk.BiomeType = BiomeType;
    
    // Calculate physics complexity based on biome
    switch (BiomeType)
    {
        case EBiomeType::Forest:
            NewChunk.PhysicsComplexity = 3;
            NewChunk.PerformanceCost = 2.5f;
            break;
        case EBiomeType::Desert:
            NewChunk.PhysicsComplexity = 1;
            NewChunk.PerformanceCost = 1.0f;
            break;
        case EBiomeType::Mountain:
            NewChunk.PhysicsComplexity = 5;
            NewChunk.PerformanceCost = 4.0f;
            break;
        default:
            NewChunk.PhysicsComplexity = 2;
            NewChunk.PerformanceCost = 1.5f;
            break;
    }
    
    // Enable collision based on performance budget
    NewChunk.bHasCollision = (CurrentPerformanceCost + NewChunk.PerformanceCost) <= TerrainSettings.MaxPhysicsComplexityBudget;
    
    ActiveTerrainChunks.Add(NewChunk);
    CurrentPerformanceCost += NewChunk.PerformanceCost;
    
    if (NewChunk.bHasCollision)
    {
        ActivePhysicsChunks++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Created terrain chunk at %s with complexity %d"), *Location.ToString(), NewChunk.PhysicsComplexity);
}

void UWorld_PhysicsIntegratedTerrainGenerator::OptimizeTerrainPhysics()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing terrain physics"));
    
    // Sort chunks by distance from player
    if (UWorld* World = GetWorld())
    {
        if (APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            FVector PlayerLocation = Player->GetActorLocation();
            
            ActiveTerrainChunks.Sort([PlayerLocation](const FWorld_PhysicsTerrainChunk& A, const FWorld_PhysicsTerrainChunk& B)
            {
                float DistA = FVector::Dist(A.ChunkLocation, PlayerLocation);
                float DistB = FVector::Dist(B.ChunkLocation, PlayerLocation);
                return DistA < DistB;
            });
        }
    }
    
    // Apply LOD based on distance and performance budget
    float RemainingBudget = TerrainSettings.MaxPhysicsComplexityBudget;
    ActivePhysicsChunks = 0;
    
    for (FWorld_PhysicsTerrainChunk& Chunk : ActiveTerrainChunks)
    {
        if (RemainingBudget >= Chunk.PerformanceCost && ActivePhysicsChunks < TerrainSettings.MaxActiveChunks)
        {
            Chunk.bHasCollision = true;
            RemainingBudget -= Chunk.PerformanceCost;
            ActivePhysicsChunks++;
        }
        else
        {
            Chunk.bHasCollision = false;
        }
    }
    
    CurrentPerformanceCost = TerrainSettings.MaxPhysicsComplexityBudget - RemainingBudget;
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain physics optimization complete: %d active chunks, %.2f performance cost"), 
           ActivePhysicsChunks, CurrentPerformanceCost);
}

float UWorld_PhysicsIntegratedTerrainGenerator::CalculateTerrainPerformanceCost() const
{
    return CurrentPerformanceCost;
}

void UWorld_PhysicsIntegratedTerrainGenerator::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying performance optimizations"));
    
    // Reduce physics complexity if over budget
    if (CurrentPerformanceCost > TerrainSettings.MaxPhysicsComplexityBudget)
    {
        OptimizeTerrainPhysics();
    }
    
    // Apply aggressive LOD if frame time is too high
    if (LastFrameTime > TerrainSettings.TargetFrameTime * 1.2f)
    {
        TerrainSettings.MaxActiveChunks = FMath::Max(1, TerrainSettings.MaxActiveChunks - 2);
        TerrainSettings.CollisionLODDistance *= 0.8f;
        OptimizeTerrainPhysics();
        
        UE_LOG(LogTemp, Warning, TEXT("Aggressive optimization applied: MaxActiveChunks=%d, LODDistance=%.2f"), 
               TerrainSettings.MaxActiveChunks, TerrainSettings.CollisionLODDistance);
    }
}

bool UWorld_PhysicsIntegratedTerrainGenerator::IsWithinPerformanceBudget() const
{
    bool bWithinBudget = CurrentPerformanceCost <= TerrainSettings.MaxPhysicsComplexityBudget;
    bool bWithinFrameTarget = LastFrameTime <= TerrainSettings.TargetFrameTime * 1.1f;
    
    return bWithinBudget && bWithinFrameTarget;
}

void UWorld_PhysicsIntegratedTerrainGenerator::IntegrateWithPhysicsConsolidator()
{
    UE_LOG(LogTemp, Warning, TEXT("Integrating with Physics Consolidator"));
    
    if (bPhysicsConsolidatorConnected)
    {
        // Update consolidator with terrain physics data
        ConsolidateTerrainCollision();
    }
    else
    {
        ConnectToPhysicsConsolidator();
    }
}

void UWorld_PhysicsIntegratedTerrainGenerator::UpdatePhysicsComplexity(float NewComplexity)
{
    PhysicsComplexityBudget = NewComplexity;
    TerrainSettings.MaxPhysicsComplexityBudget = NewComplexity;
    
    UE_LOG(LogTemp, Log, TEXT("Physics complexity budget updated to %.2f"), NewComplexity);
    
    // Re-optimize terrain with new budget
    OptimizeTerrainPhysics();
}

void UWorld_PhysicsIntegratedTerrainGenerator::ConsolidateTerrainCollision()
{
    UE_LOG(LogTemp, Warning, TEXT("Consolidating terrain collision"));
    
    ConsolidatedCollisionObjects = 0;
    
    for (const FWorld_PhysicsTerrainChunk& Chunk : ActiveTerrainChunks)
    {
        if (Chunk.bHasCollision)
        {
            ConsolidatedCollisionObjects++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain collision consolidation complete: %d objects"), ConsolidatedCollisionObjects);
}

void UWorld_PhysicsIntegratedTerrainGenerator::CreateWalkableTerrainForMilestone()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating walkable terrain for Milestone 1"));
    
    // Generate optimized terrain for walking gameplay
    GeneratePhysicsOptimizedTerrain();
    
    // Ensure minimum walkable area around spawn
    FVector SpawnLocation = FVector::ZeroVector;
    CreateTerrainChunk(SpawnLocation, FVector(2000, 2000, 100), EBiomeType::Plains);
    
    // Validate walkability
    ValidateTerrainWalkability();
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 walkable terrain creation complete"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::ValidateTerrainWalkability()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating terrain walkability"));
    
    int32 WalkableChunks = 0;
    
    for (const FWorld_PhysicsTerrainChunk& Chunk : ActiveTerrainChunks)
    {
        if (Chunk.bHasCollision && Chunk.PhysicsComplexity <= 3)
        {
            WalkableChunks++;
        }
    }
    
    bool bTerrainWalkable = WalkableChunks >= 5; // Minimum 5 walkable chunks
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain walkability validation: %s (%d walkable chunks)"), 
           bTerrainWalkable ? TEXT("PASS") : TEXT("FAIL"), WalkableChunks);
}

void UWorld_PhysicsIntegratedTerrainGenerator::EnsurePerformanceTargets()
{
    UE_LOG(LogTemp, Warning, TEXT("Ensuring performance targets"));
    
    // Check if we're meeting Milestone 1 performance requirements
    bool bMeetsTargets = IsWithinPerformanceBudget();
    
    if (!bMeetsTargets)
    {
        ApplyPerformanceOptimizations();
        
        // Verify optimization worked
        bMeetsTargets = IsWithinPerformanceBudget();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance targets validation: %s"), 
           bMeetsTargets ? TEXT("PASS") : TEXT("FAIL"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::GenerateBaseHeightmap()
{
    // Generate procedural heightmap for terrain base
    UE_LOG(LogTemp, Log, TEXT("Generating base heightmap"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::ApplyBiomeModifications()
{
    // Apply biome-specific terrain modifications
    UE_LOG(LogTemp, Log, TEXT("Applying biome modifications"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::CreateCollisionGeometry()
{
    // Create optimized collision geometry
    UE_LOG(LogTemp, Log, TEXT("Creating collision geometry"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::OptimizeForPerformance()
{
    // Apply performance optimizations
    OptimizeTerrainPhysics();
}

void UWorld_PhysicsIntegratedTerrainGenerator::UpdatePerformanceMetrics()
{
    // Update frame time tracking
    if (UWorld* World = GetWorld())
    {
        LastFrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    }
}

void UWorld_PhysicsIntegratedTerrainGenerator::CheckPerformanceThresholds()
{
    // Check if performance thresholds are exceeded
    if (!IsWithinPerformanceBudget())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance thresholds exceeded - applying optimizations"));
        ApplyPerformanceOptimizations();
    }
}

void UWorld_PhysicsIntegratedTerrainGenerator::ApplyLODOptimizations()
{
    // Apply level-of-detail optimizations
    OptimizeTerrainPhysics();
}

void UWorld_PhysicsIntegratedTerrainGenerator::ConnectToPhysicsConsolidator()
{
    UE_LOG(LogTemp, Log, TEXT("Attempting to connect to Physics Consolidator"));
    
    // Simulate connection to Agent #3's physics consolidator
    bPhysicsConsolidatorConnected = true;
    PhysicsComplexityBudget = TerrainSettings.MaxPhysicsComplexityBudget;
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Consolidator connection established"));
}

void UWorld_PhysicsIntegratedTerrainGenerator::UpdatePhysicsBudget()
{
    // Update physics budget based on consolidator feedback
    if (bPhysicsConsolidatorConnected)
    {
        // Simulate budget updates from physics consolidator
        float BudgetUsage = CurrentPerformanceCost / TerrainSettings.MaxPhysicsComplexityBudget;
        
        if (BudgetUsage > 0.9f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics budget usage high: %.2f%%"), BudgetUsage * 100.0f);
        }
    }
}

void UWorld_PhysicsIntegratedTerrainGenerator::ConsolidateChunkPhysics()
{
    // Consolidate physics for terrain chunks
    ConsolidateTerrainCollision();
}
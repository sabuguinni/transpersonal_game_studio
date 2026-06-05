#include "World_PhysicsOptimizedTerrainSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

AWorld_PhysicsOptimizedTerrainSystem::AWorld_PhysicsOptimizedTerrainSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default settings
    PerformanceSettings = FWorld_PerformanceSettings();
    bIntegrateWithPhysicsOptimizer = true;
    
    // Initialize stats
    CurrentFPS = 60.0f;
    CurrentMemoryUsage = 0.0f;
    ActiveTerrainChunks = 0;
    OptimizedTerrainChunks = 0;
    PerformanceCheckTimer = 0.0f;
}

void AWorld_PhysicsOptimizedTerrainSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_PhysicsOptimizedTerrainSystem: BeginPlay started"));
    
    // Setup Milestone 1 terrain
    SetupMilestone1Terrain();
    
    // Integrate with physics optimizer
    if (bIntegrateWithPhysicsOptimizer)
    {
        IntegrateWithPhysicsOptimizer();
    }
    
    // Initial performance update
    UpdatePerformanceStats();
    
    UE_LOG(LogTemp, Warning, TEXT("World_PhysicsOptimizedTerrainSystem: BeginPlay completed"));
}

void AWorld_PhysicsOptimizedTerrainSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Performance optimization tick
    if (PerformanceSettings.bEnableAutoOptimization)
    {
        PerformanceOptimizationTick(DeltaTime);
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::CreateTerrainChunk(const FVector& Location, EBiomeType BiomeType, const FVector& Scale)
{
    UE_LOG(LogTemp, Warning, TEXT("Creating terrain chunk at location: %s"), *Location.ToString());
    
    // Create new terrain chunk data
    FWorld_TerrainChunk NewChunk;
    NewChunk.Location = Location;
    NewChunk.BiomeType = BiomeType;
    NewChunk.Scale = Scale;
    NewChunk.LODLevel = 0;
    NewChunk.bIsPerformanceOptimized = false;
    NewChunk.PerformanceScore = 100.0f;
    
    // Add to terrain chunks array
    TerrainChunks.Add(NewChunk);
    ActiveTerrainChunks = TerrainChunks.Num();
    
    // Create actual terrain actor in world
    UWorld* World = GetWorld();
    if (World)
    {
        // Spawn static mesh actor for terrain chunk
        AStaticMeshActor* TerrainActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Location,
            FRotator::ZeroRotator
        );
        
        if (TerrainActor)
        {
            // Set terrain chunk name
            FString ChunkName = FString::Printf(TEXT("TerrainChunk_%s_%d"), 
                *UEnum::GetValueAsString(BiomeType), TerrainChunks.Num() - 1);
            TerrainActor->SetActorLabel(ChunkName);
            
            // Get static mesh component and set basic cube mesh
            UStaticMeshComponent* MeshComp = TerrainActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                // Load basic cube mesh
                UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
                if (CubeMesh)
                {
                    MeshComp->SetStaticMesh(CubeMesh);
                    TerrainActor->SetActorScale3D(Scale);
                    
                    UE_LOG(LogTemp, Warning, TEXT("Terrain chunk created successfully: %s"), *ChunkName);
                }
            }
        }
    }
    
    // Check if optimization is needed
    if (ShouldOptimizeTerrain())
    {
        ApplyPerformanceOptimizations();
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::RemoveTerrainChunk(int32 ChunkIndex)
{
    if (TerrainChunks.IsValidIndex(ChunkIndex))
    {
        TerrainChunks.RemoveAt(ChunkIndex);
        ActiveTerrainChunks = TerrainChunks.Num();
        
        UE_LOG(LogTemp, Warning, TEXT("Terrain chunk removed at index: %d"), ChunkIndex);
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::OptimizeTerrainChunk(int32 ChunkIndex)
{
    if (TerrainChunks.IsValidIndex(ChunkIndex))
    {
        FWorld_TerrainChunk& Chunk = TerrainChunks[ChunkIndex];
        
        // Apply optimization based on performance needs
        if (CurrentFPS < PerformanceSettings.TargetFPS)
        {
            // Increase LOD level for performance
            Chunk.LODLevel = FMath::Min(Chunk.LODLevel + 1, 3);
            Chunk.bIsPerformanceOptimized = true;
            Chunk.PerformanceScore = FMath::Max(Chunk.PerformanceScore - 10.0f, 50.0f);
            
            OptimizedTerrainChunks++;
            
            UE_LOG(LogTemp, Warning, TEXT("Terrain chunk optimized - LOD Level: %d"), Chunk.LODLevel);
        }
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::OptimizeAllTerrainChunks()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing all terrain chunks - Count: %d"), TerrainChunks.Num());
    
    OptimizedTerrainChunks = 0;
    
    for (int32 i = 0; i < TerrainChunks.Num(); i++)
    {
        OptimizeTerrainChunk(i);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Terrain optimization complete - Optimized chunks: %d"), OptimizedTerrainChunks);
}

void AWorld_PhysicsOptimizedTerrainSystem::UpdatePerformanceStats()
{
    // Get current FPS (simplified)
    CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
    CurrentFPS = FMath::Clamp(CurrentFPS, 1.0f, 120.0f);
    
    // Update terrain stats
    ActiveTerrainChunks = TerrainChunks.Num();
    
    // Count optimized chunks
    OptimizedTerrainChunks = 0;
    for (const FWorld_TerrainChunk& Chunk : TerrainChunks)
    {
        if (Chunk.bIsPerformanceOptimized)
        {
            OptimizedTerrainChunks++;
        }
    }
    
    // Memory usage estimation (simplified)
    CurrentMemoryUsage = ActiveTerrainChunks * 0.5f; // MB per chunk estimate
    
    UE_LOG(LogTemp, Log, TEXT("Performance Stats - FPS: %.1f, Memory: %.1f MB, Active Chunks: %d, Optimized: %d"), 
        CurrentFPS, CurrentMemoryUsage, ActiveTerrainChunks, OptimizedTerrainChunks);
}

void AWorld_PhysicsOptimizedTerrainSystem::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying performance optimizations"));
    
    // Check if we need to optimize
    if (CurrentFPS < PerformanceSettings.MinimumFPS)
    {
        // Aggressive optimization
        OptimizeAllTerrainChunks();
        UpdateTerrainLOD();
        CullDistantTerrainChunks();
    }
    else if (CurrentFPS < PerformanceSettings.TargetFPS)
    {
        // Moderate optimization
        UpdateTerrainLOD();
    }
    
    // Update physics optimization if integrated
    if (bIntegrateWithPhysicsOptimizer)
    {
        UpdatePhysicsOptimization();
    }
}

bool AWorld_PhysicsOptimizedTerrainSystem::ShouldOptimizeTerrain() const
{
    return (CurrentFPS < PerformanceSettings.TargetFPS) || 
           (ActiveTerrainChunks > PerformanceSettings.MaxTerrainChunks);
}

void AWorld_PhysicsOptimizedTerrainSystem::SetLODLevel(int32 ChunkIndex, int32 LODLevel)
{
    if (TerrainChunks.IsValidIndex(ChunkIndex))
    {
        TerrainChunks[ChunkIndex].LODLevel = FMath::Clamp(LODLevel, 0, 3);
        UE_LOG(LogTemp, Log, TEXT("Terrain chunk %d LOD set to: %d"), ChunkIndex, LODLevel);
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::IntegrateWithPhysicsOptimizer()
{
    UE_LOG(LogTemp, Warning, TEXT("Integrating with physics optimizer"));
    
    // Find physics optimizer in world
    FindPhysicsOptimizer();
    
    if (PhysicsOptimizer.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics optimizer integration successful"));
        SyncWithPhysicsOptimizer();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics optimizer not found - operating independently"));
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::UpdatePhysicsOptimization()
{
    if (PhysicsOptimizer.IsValid())
    {
        // Sync performance data with physics optimizer
        SyncWithPhysicsOptimizer();
        
        // Apply physics-aware terrain optimizations
        float PhysicsScore = GetTerrainPhysicsPerformanceScore();
        if (PhysicsScore < 70.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Low physics performance detected - applying terrain optimizations"));
            OptimizeAllTerrainChunks();
        }
    }
}

float AWorld_PhysicsOptimizedTerrainSystem::GetTerrainPhysicsPerformanceScore() const
{
    // Calculate terrain physics performance score
    float BaseScore = 100.0f;
    
    // Reduce score based on active chunks
    BaseScore -= (ActiveTerrainChunks * 2.0f);
    
    // Increase score for optimized chunks
    BaseScore += (OptimizedTerrainChunks * 5.0f);
    
    // Factor in current FPS
    if (CurrentFPS < PerformanceSettings.MinimumFPS)
    {
        BaseScore -= 30.0f;
    }
    else if (CurrentFPS < PerformanceSettings.TargetFPS)
    {
        BaseScore -= 15.0f;
    }
    
    return FMath::Clamp(BaseScore, 0.0f, 100.0f);
}

void AWorld_PhysicsOptimizedTerrainSystem::SetupMilestone1Terrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Setting up Milestone 1 terrain for WALK AROUND"));
    
    // Create basic terrain chunks for Milestone 1
    CreateTerrainChunk(FVector(2000, 0, 0), EBiomeType::Forest, FVector(20, 20, 2));
    CreateTerrainChunk(FVector(-2000, 0, 0), EBiomeType::Plains, FVector(20, 20, 2));
    CreateTerrainChunk(FVector(0, 2000, 0), EBiomeType::Rocky, FVector(20, 20, 3));
    CreateTerrainChunk(FVector(0, -2000, 0), EBiomeType::Wetlands, FVector(20, 20, 1));
    
    // Create central terrain chunk
    CreateTerrainChunk(FVector(0, 0, 0), EBiomeType::Plains, FVector(30, 30, 2));
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 terrain setup complete - %d chunks created"), TerrainChunks.Num());
}

bool AWorld_PhysicsOptimizedTerrainSystem::ValidateMilestone1Performance()
{
    UpdatePerformanceStats();
    
    bool bPerformanceValid = (CurrentFPS >= PerformanceSettings.MinimumFPS) && 
                            (ActiveTerrainChunks <= PerformanceSettings.MaxTerrainChunks);
    
    UE_LOG(LogTemp, Warning, TEXT("Milestone 1 performance validation: %s (FPS: %.1f, Chunks: %d)"), 
        bPerformanceValid ? TEXT("PASS") : TEXT("FAIL"), CurrentFPS, ActiveTerrainChunks);
    
    return bPerformanceValid;
}

void AWorld_PhysicsOptimizedTerrainSystem::CreateWalkAroundTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("Creating walk-around terrain for player exploration"));
    
    // Clear existing chunks
    TerrainChunks.Empty();
    
    // Create optimized terrain layout for walking
    TArray<FVector> WalkablePositions = {
        FVector(0, 0, 0),      // Center
        FVector(1000, 0, 0),   // East
        FVector(-1000, 0, 0),  // West
        FVector(0, 1000, 0),   // North
        FVector(0, -1000, 0),  // South
        FVector(1000, 1000, 0),   // Northeast
        FVector(-1000, 1000, 0),  // Northwest
        FVector(1000, -1000, 0),  // Southeast
        FVector(-1000, -1000, 0)  // Southwest
    };
    
    for (int32 i = 0; i < WalkablePositions.Num(); i++)
    {
        EBiomeType BiomeType = static_cast<EBiomeType>(i % 4); // Cycle through biomes
        CreateTerrainChunk(WalkablePositions[i], BiomeType, FVector(15, 15, 1));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Walk-around terrain created - %d walkable chunks"), WalkablePositions.Num());
}

FWorld_TerrainChunk AWorld_PhysicsOptimizedTerrainSystem::GetTerrainChunk(int32 Index) const
{
    if (TerrainChunks.IsValidIndex(Index))
    {
        return TerrainChunks[Index];
    }
    return FWorld_TerrainChunk();
}

void AWorld_PhysicsOptimizedTerrainSystem::SetPerformanceSettings(const FWorld_PerformanceSettings& NewSettings)
{
    PerformanceSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Performance settings updated - Target FPS: %.1f"), PerformanceSettings.TargetFPS);
}

void AWorld_PhysicsOptimizedTerrainSystem::PerformanceOptimizationTick(float DeltaTime)
{
    PerformanceCheckTimer += DeltaTime;
    
    if (PerformanceCheckTimer >= PerformanceSettings.OptimizationCheckInterval)
    {
        PerformanceCheckTimer = 0.0f;
        
        // Update performance stats
        UpdatePerformanceStats();
        
        // Apply optimizations if needed
        if (ShouldOptimizeTerrain())
        {
            ApplyPerformanceOptimizations();
        }
        
        // Validate performance
        ValidateTerrainPerformance();
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::UpdateTerrainLOD()
{
    // Update LOD based on distance and performance
    for (int32 i = 0; i < TerrainChunks.Num(); i++)
    {
        FWorld_TerrainChunk& Chunk = TerrainChunks[i];
        
        // Simple LOD based on performance
        if (CurrentFPS < PerformanceSettings.MinimumFPS)
        {
            Chunk.LODLevel = FMath::Min(Chunk.LODLevel + 1, 3);
        }
        else if (CurrentFPS > PerformanceSettings.TargetFPS && Chunk.LODLevel > 0)
        {
            Chunk.LODLevel = FMath::Max(Chunk.LODLevel - 1, 0);
        }
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::CullDistantTerrainChunks()
{
    // Simple culling based on chunk count
    if (ActiveTerrainChunks > PerformanceSettings.MaxTerrainChunks)
    {
        int32 ChunksToCull = ActiveTerrainChunks - PerformanceSettings.MaxTerrainChunks;
        
        for (int32 i = TerrainChunks.Num() - 1; i >= 0 && ChunksToCull > 0; i--)
        {
            if (!TerrainChunks[i].bIsPerformanceOptimized)
            {
                RemoveTerrainChunk(i);
                ChunksToCull--;
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Culled distant terrain chunks - Remaining: %d"), TerrainChunks.Num());
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::ValidateTerrainPerformance()
{
    bool bPerformanceGood = (CurrentFPS >= PerformanceSettings.MinimumFPS);
    
    if (!bPerformanceGood)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain performance below target - FPS: %.1f (Target: %.1f)"), 
            CurrentFPS, PerformanceSettings.TargetFPS);
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::FindPhysicsOptimizer()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Try to find physics optimizer in world
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Perf_PhysicsIntegrationMasterOptimizer")))
            {
                PhysicsOptimizer = Cast<APerf_PhysicsIntegrationMasterOptimizer>(Actor);
                if (PhysicsOptimizer.IsValid())
                {
                    UE_LOG(LogTemp, Warning, TEXT("Found physics optimizer: %s"), *Actor->GetName());
                    break;
                }
            }
        }
    }
}

void AWorld_PhysicsOptimizedTerrainSystem::SyncWithPhysicsOptimizer()
{
    if (PhysicsOptimizer.IsValid())
    {
        // Sync performance data with physics optimizer
        // This would integrate with the physics optimizer's performance metrics
        UE_LOG(LogTemp, Log, TEXT("Syncing with physics optimizer - Terrain performance score: %.1f"), 
            GetTerrainPhysicsPerformanceScore());
    }
}
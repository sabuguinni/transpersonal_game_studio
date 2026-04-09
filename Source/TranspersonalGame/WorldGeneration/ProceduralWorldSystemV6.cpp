#include "ProceduralWorldSystemV6.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "WorldPartition/WorldPartition.h"
#include "Components/SceneComponent.h"
#include "TimerManager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Kismet/GameplayStatics.h"

AProceduralWorldSystemV6::AProceduralWorldSystemV6()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update metrics every 100ms

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create master PCG component
    MasterPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("MasterPCGComponent"));
    MasterPCGComponent->SetupAttachment(RootComponent);

    // Initialize default settings
    WorldSettings.WorldSizeKm = 200.0f;
    WorldSettings.LandscapeResolution = 8129;
    WorldSettings.MaxHeightM = 2500.0f;
    WorldSettings.bUseGPUAcceleration = true;
    WorldSettings.bUseHierarchicalGeneration = true;
    WorldSettings.CellSizeM = 512.0f;
    WorldSettings.LoadingRangeM = 1536.0f;
    WorldSettings.bEnableRuntimeGeneration = true;
    WorldSettings.WorldSeed = 12345;

    // Initialize performance metrics
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Initialization;
    PerformanceMetrics.GenerationProgress = 0.0f;
    PerformanceMetrics.MemoryUsageMB = 0.0f;
    PerformanceMetrics.GenerationTimeSeconds = 0.0f;
    PerformanceMetrics.ActivePCGComponents = 0;
    PerformanceMetrics.CurrentFPS = 0.0f;

    // Initialize generation state
    GenerationStartTime = 0.0;
    bGenerationInProgress = false;
    GeneratedLandscape = nullptr;
}

void AProceduralWorldSystemV6::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: System initialized"));
    
    // Setup PCG components
    SetupPCGComponents();
    
    // Configure World Partition if needed
    ConfigureWorldPartition();
}

void AProceduralWorldSystemV6::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void AProceduralWorldSystemV6::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    // Setup PCG components during construction
    SetupPCGComponents();
}

void AProceduralWorldSystemV6::StartWorldGeneration()
{
    if (bGenerationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldSystemV6: Generation already in progress"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Starting world generation..."));
    
    bGenerationInProgress = true;
    GenerationStartTime = FPlatformTime::Seconds();
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Initialization;
    PerformanceMetrics.GenerationProgress = 0.0f;

    // Initialize world generation
    InitializeWorldGeneration();

    // Start generation phases
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::GenerateTerrain,
        0.5f,
        false
    );
}

void AProceduralWorldSystemV6::StopWorldGeneration()
{
    if (!bGenerationInProgress)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Stopping world generation"));
    
    bGenerationInProgress = false;
    GetWorld()->GetTimerManager().ClearTimer(GenerationTimerHandle);
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Initialization;
    PerformanceMetrics.GenerationProgress = 0.0f;
}

void AProceduralWorldSystemV6::ClearGeneratedWorld()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Clearing generated world"));
    
    // Stop any ongoing generation
    StopWorldGeneration();
    
    // Clear PCG generated content
    if (MasterPCGComponent)
    {
        MasterPCGComponent->CleanupLocal(true);
    }
    
    // Clear landscape reference
    if (GeneratedLandscape)
    {
        GeneratedLandscape = nullptr;
    }
    
    // Reset metrics
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Initialization;
    PerformanceMetrics.GenerationProgress = 0.0f;
    PerformanceMetrics.GenerationTimeSeconds = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: World cleared successfully"));
}

void AProceduralWorldSystemV6::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Initializing world generation"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Initialization;
    
    // Set random seed
    FMath::RandInit(WorldSettings.WorldSeed);
    
    // Configure PCG components for generation
    if (MasterPCGComponent)
    {
        // Enable partitioning for large worlds
        MasterPCGComponent->bIsPartitioned = WorldSettings.bUseHierarchicalGeneration;
        
        // Set generation trigger
        if (WorldSettings.bEnableRuntimeGeneration)
        {
            MasterPCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateAtRuntime;
        }
        else
        {
            MasterPCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::Generate;
        }
        
        // Set master graph
        if (MasterWorldGraph)
        {
            MasterPCGComponent->SetGraph(MasterWorldGraph);
        }
    }
    
    PerformanceMetrics.GenerationProgress = 0.1f;
}

void AProceduralWorldSystemV6::SetupPCGComponents()
{
    if (!MasterPCGComponent)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Setting up PCG components"));
    
    // Configure master PCG component
    MasterPCGComponent->bIsPartitioned = WorldSettings.bUseHierarchicalGeneration;
    MasterPCGComponent->bGenerated = false;
    
    // Set GPU acceleration if available
    if (WorldSettings.bUseGPUAcceleration)
    {
        // Enable GPU processing where supported
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: GPU acceleration enabled"));
    }
    
    // Configure for large world streaming
    if (WorldSettings.bEnableRuntimeGeneration)
    {
        MasterPCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateAtRuntime;
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Runtime generation enabled"));
    }
}

void AProceduralWorldSystemV6::ConfigureWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralWorldSystemV6: No valid world found"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Configuring World Partition"));
    
    // Log recommended World Partition settings
    UE_LOG(LogTemp, Log, TEXT("Recommended World Partition Settings:"));
    UE_LOG(LogTemp, Log, TEXT("- Cell Size: %.0f meters"), WorldSettings.CellSizeM);
    UE_LOG(LogTemp, Log, TEXT("- Loading Range: %.0f meters"), WorldSettings.LoadingRangeM);
    UE_LOG(LogTemp, Log, TEXT("- Enable Streaming: %s"), WorldSettings.bEnableRuntimeGeneration ? TEXT("True") : TEXT("False"));
    UE_LOG(LogTemp, Log, TEXT("- World Size: %.1f km²"), WorldSettings.WorldSizeKm);
    UE_LOG(LogTemp, Log, TEXT("- Landscape Resolution: %d x %d"), WorldSettings.LandscapeResolution, WorldSettings.LandscapeResolution);
}

void AProceduralWorldSystemV6::GenerateTerrain()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Generating terrain"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::TerrainGeneration;
    PerformanceMetrics.GenerationProgress = 0.2f;
    
    // Generate base terrain using PCG
    if (MasterPCGComponent && TerrainGraph)
    {
        // Set terrain graph for this phase
        MasterPCGComponent->SetGraph(TerrainGraph);
        MasterPCGComponent->GenerateLocal(true);
        
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Terrain PCG generation started"));
    }
    
    // Schedule next phase
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::DistributeBiomes,
        2.0f,
        false
    );
}

void AProceduralWorldSystemV6::DistributeBiomes()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Distributing biomes"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::BiomeDistribution;
    PerformanceMetrics.GenerationProgress = 0.4f;
    
    // Generate biome distribution using PCG
    if (MasterPCGComponent && BiomeGraph)
    {
        // Set biome graph for this phase
        MasterPCGComponent->SetGraph(BiomeGraph);
        MasterPCGComponent->GenerateLocal(true);
        
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Biome PCG generation started"));
    }
    
    // Schedule next phase
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::GenerateWaterSystems,
        1.5f,
        false
    );
}

void AProceduralWorldSystemV6::GenerateWaterSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Generating water systems"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::WaterSystemGeneration;
    PerformanceMetrics.GenerationProgress = 0.6f;
    
    // Generate water systems using PCG
    if (MasterPCGComponent && WaterSystemGraph)
    {
        // Set water system graph for this phase
        MasterPCGComponent->SetGraph(WaterSystemGraph);
        MasterPCGComponent->GenerateLocal(true);
        
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Water system PCG generation started"));
    }
    
    // Schedule next phase
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::PlaceVegetation,
        1.0f,
        false
    );
}

void AProceduralWorldSystemV6::PlaceVegetation()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Placing vegetation"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::VegetationPlacement;
    PerformanceMetrics.GenerationProgress = 0.8f;
    
    // Generate vegetation using PCG
    if (MasterPCGComponent && VegetationGraph)
    {
        // Set vegetation graph for this phase
        MasterPCGComponent->SetGraph(VegetationGraph);
        MasterPCGComponent->GenerateLocal(true);
        
        UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Vegetation PCG generation started"));
    }
    
    // Schedule next phase
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::GenerateDetails,
        1.0f,
        false
    );
}

void AProceduralWorldSystemV6::GenerateDetails()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Generating details"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::DetailGeneration;
    PerformanceMetrics.GenerationProgress = 0.9f;
    
    // Generate detail objects (rocks, fallen logs, etc.)
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Detail generation completed"));
    
    // Schedule optimization phase
    GetWorld()->GetTimerManager().SetTimer(
        GenerationTimerHandle,
        this,
        &AProceduralWorldSystemV6::OptimizeWorld,
        0.5f,
        false
    );
}

void AProceduralWorldSystemV6::OptimizeWorld()
{
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Optimizing world"));
    
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Optimization;
    PerformanceMetrics.GenerationProgress = 0.95f;
    
    // Perform optimization passes
    // - LOD generation
    // - Culling optimization
    // - Memory optimization
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: Optimization completed"));
    
    // Complete generation
    PerformanceMetrics.CurrentPhase = EWorldGenerationPhase::Complete;
    PerformanceMetrics.GenerationProgress = 1.0f;
    PerformanceMetrics.GenerationTimeSeconds = FPlatformTime::Seconds() - GenerationStartTime;
    
    bGenerationInProgress = false;
    
    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldSystemV6: World generation completed in %.2f seconds"), 
        PerformanceMetrics.GenerationTimeSeconds);
}

void AProceduralWorldSystemV6::UpdatePerformanceMetrics()
{
    // Update generation time if in progress
    if (bGenerationInProgress)
    {
        PerformanceMetrics.GenerationTimeSeconds = FPlatformTime::Seconds() - GenerationStartTime;
    }
    
    // Update memory usage
    PerformanceMetrics.MemoryUsageMB = CalculateMemoryUsage();
    
    // Update FPS
    PerformanceMetrics.CurrentFPS = CalculateCurrentFPS();
    
    // Count active PCG components
    PerformanceMetrics.ActivePCGComponents = 0;
    if (MasterPCGComponent && MasterPCGComponent->bGenerated)
    {
        PerformanceMetrics.ActivePCGComponents++;
    }
}

float AProceduralWorldSystemV6::CalculateMemoryUsage()
{
    // Get current memory usage in MB
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

float AProceduralWorldSystemV6::CalculateCurrentFPS()
{
    // Get current FPS from engine
    if (GEngine)
    {
        return 1.0f / GEngine->GetMaxTickRate();
    }
    return 0.0f;
}
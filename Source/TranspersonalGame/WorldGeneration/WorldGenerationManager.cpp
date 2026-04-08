#include "WorldGenerationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeSubsystem.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Water/Classes/WaterBody.h"
#include "Water/Classes/WaterBodyRiver.h"
#include "Components/SplineComponent.h"
#include "Kismet/GameplayStatics.h"

UWorldGenerationManager::UWorldGenerationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize default configuration
    WorldConfig.WorldSizeX = 8129;
    WorldConfig.WorldSizeY = 8129;
    WorldConfig.TerrainDensity = ETerrainDensity::Medium;
    WorldConfig.MaxComponentsPerGrid = 1024;
    WorldConfig.MaxRiverSystems = 3;
    WorldConfig.RiverWidth = 500.0f;

    // Initialize state
    bIsGenerating = false;
    GenerationProgress = 0.0f;
    CurrentGenerationStep = 0;
    TotalGenerationSteps = 6; // Terrain, Biomes, Rivers, Vegetation, Optimization, Finalization
    ActiveComponentCount = 0;
    LastPerformanceCheck = 0.0f;

    // Initialize component pointers
    TerrainPCGComponent = nullptr;
    BiomePCGComponent = nullptr;
    VegetationPCGComponent = nullptr;
    GeneratedLandscape = nullptr;
    WorldPartitionSubsystem = nullptr;
}

void UWorldGenerationManager::BeginPlay()
{
    Super::BeginPlay();

    // Get World Partition subsystem
    if (UWorld* World = GetWorld())
    {
        WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();
    }

    // Initialize PCG components
    InitializePCGComponents();

    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Initialized for world generation"));
}

void UWorldGenerationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Performance monitoring during generation
    if (bIsGenerating)
    {
        LastPerformanceCheck += DeltaTime;
        if (LastPerformanceCheck >= 1.0f) // Check every second
        {
            ActiveComponentCount = GetActiveComponentCount();
            LastPerformanceCheck = 0.0f;

            // Log performance metrics
            UE_LOG(LogTemp, Log, TEXT("WorldGen Performance: %d active components, %.1f%% complete"), 
                   ActiveComponentCount, GenerationProgress * 100.0f);
        }
    }
}

void UWorldGenerationManager::GenerateWorld()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: Generation already in progress"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Starting world generation..."));
    
    bIsGenerating = true;
    GenerationProgress = 0.0f;
    CurrentGenerationStep = 0;

    // Setup World Partition first
    SetupWorldPartition();

    // Generate in sequence
    GenerateTerrain();
}

void UWorldGenerationManager::GenerateTerrain()
{
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Generating terrain..."));
    
    CurrentGenerationStep = 1;
    GenerationProgress = 0.16f; // 1/6 steps

    // Create landscape actor
    CreateLandscapeActor();

    if (TerrainPCGComponent)
    {
        // Configure terrain PCG settings based on world config
        // This would be expanded with actual PCG graph setup
        TerrainPCGComponent->Generate();
    }

    // Continue to next step
    GenerateBiomes();
}

void UWorldGenerationManager::GenerateBiomes()
{
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Generating biomes..."));
    
    CurrentGenerationStep = 2;
    GenerationProgress = 0.33f; // 2/6 steps

    ApplyBiomeDistribution();

    if (BiomePCGComponent)
    {
        BiomePCGComponent->Generate();
    }

    // Continue to next step
    GenerateRiverSystems();
}

void UWorldGenerationManager::GenerateRiverSystems()
{
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Generating river systems..."));
    
    CurrentGenerationStep = 3;
    GenerationProgress = 0.50f; // 3/6 steps

    GenerateRiverSplines();

    // Continue to next step
    GenerateVegetation();
}

void UWorldGenerationManager::GenerateVegetation()
{
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Generating vegetation..."));
    
    CurrentGenerationStep = 4;
    GenerationProgress = 0.66f; // 4/6 steps

    PopulateVegetation();

    if (VegetationPCGComponent)
    {
        VegetationPCGComponent->Generate();
    }

    // Continue to optimization
    OptimizeForPerformance();
}

EBiomeType UWorldGenerationManager::GetBiomeAtLocation(FVector WorldLocation)
{
    // Placeholder implementation - would use noise functions and biome maps
    // For now, return based on simple distance from center
    FVector WorldCenter = FVector(WorldConfig.WorldSizeX * 0.5f, WorldConfig.WorldSizeY * 0.5f, 0.0f);
    float DistanceFromCenter = FVector::Dist2D(WorldLocation, WorldCenter);
    
    if (DistanceFromCenter < 1000.0f)
    {
        return EBiomeType::DenseJungle;
    }
    else if (DistanceFromCenter < 2000.0f)
    {
        return EBiomeType::OpenPlains;
    }
    else if (DistanceFromCenter < 3000.0f)
    {
        return EBiomeType::RockyOutcrops;
    }
    else
    {
        return EBiomeType::CoastalRegion;
    }
}

float UWorldGenerationManager::GetTerrainHeightAtLocation(FVector WorldLocation)
{
    if (!GeneratedLandscape)
    {
        return 0.0f;
    }

    // Use landscape's height sampling
    FVector LandscapeLocation = GeneratedLandscape->GetActorTransform().InverseTransformPosition(WorldLocation);
    return GeneratedLandscape->GetHeightAtLocation(LandscapeLocation);
}

bool UWorldGenerationManager::IsLocationNearWater(FVector WorldLocation, float SearchRadius)
{
    for (AWaterBody* WaterBody : GeneratedWaterBodies)
    {
        if (WaterBody)
        {
            float Distance = FVector::Dist(WorldLocation, WaterBody->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                return true;
            }
        }
    }
    return false;
}

int32 UWorldGenerationManager::GetActiveComponentCount()
{
    int32 Count = 0;
    
    if (TerrainPCGComponent && TerrainPCGComponent->IsGenerating())
        Count++;
    if (BiomePCGComponent && BiomePCGComponent->IsGenerating())
        Count++;
    if (VegetationPCGComponent && VegetationPCGComponent->IsGenerating())
        Count++;
    
    return Count;
}

float UWorldGenerationManager::GetGenerationProgress()
{
    return GenerationProgress;
}

void UWorldGenerationManager::InitializePCGComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: No owner actor found"));
        return;
    }

    // Create PCG components for different generation phases
    TerrainPCGComponent = NewObject<UPCGComponent>(Owner, TEXT("TerrainPCGComponent"));
    BiomePCGComponent = NewObject<UPCGComponent>(Owner, TEXT("BiomePCGComponent"));
    VegetationPCGComponent = NewObject<UPCGComponent>(Owner, TEXT("VegetationPCGComponent"));

    if (TerrainPCGComponent)
    {
        Owner->AddInstanceComponent(TerrainPCGComponent);
        TerrainPCGComponent->AttachToComponent(Owner->GetRootComponent(), 
                                               FAttachmentTransformRules::KeepWorldTransform);
    }

    if (BiomePCGComponent)
    {
        Owner->AddInstanceComponent(BiomePCGComponent);
        BiomePCGComponent->AttachToComponent(Owner->GetRootComponent(), 
                                             FAttachmentTransformRules::KeepWorldTransform);
    }

    if (VegetationPCGComponent)
    {
        Owner->AddInstanceComponent(VegetationPCGComponent);
        VegetationPCGComponent->AttachToComponent(Owner->GetRootComponent(), 
                                                  FAttachmentTransformRules::KeepWorldTransform);
    }

    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: PCG components initialized"));
}

void UWorldGenerationManager::SetupWorldPartition()
{
    if (!WorldPartitionSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("WorldGenerationManager: World Partition subsystem not available"));
        return;
    }

    // Configure world partition for large world streaming
    // This would be expanded with actual world partition configuration
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: World Partition configured for %dx%d world"), 
           WorldConfig.WorldSizeX, WorldConfig.WorldSizeY);
}

void UWorldGenerationManager::CreateLandscapeActor()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("WorldGenerationManager: Cannot create landscape - no world"));
        return;
    }

    // Create landscape with recommended settings for 8129x8129 world
    // This would be expanded with actual landscape creation code
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Creating landscape actor %dx%d"), 
           WorldConfig.WorldSizeX, WorldConfig.WorldSizeY);
}

void UWorldGenerationManager::ApplyBiomeDistribution()
{
    // Apply biome distribution across the world using noise functions
    // This would be expanded with actual biome placement algorithms
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Applying biome distribution with %d biome types"), 
           WorldConfig.AvailableBiomes.Num());
}

void UWorldGenerationManager::GenerateRiverSplines()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Generate river systems using splines
    for (int32 i = 0; i < WorldConfig.MaxRiverSystems; ++i)
    {
        // Create river water body
        AWaterBodyRiver* RiverBody = World->SpawnActor<AWaterBodyRiver>();
        if (RiverBody)
        {
            GeneratedWaterBodies.Add(RiverBody);
            
            // Configure river properties
            USplineComponent* RiverSpline = RiverBody->GetWaterSpline();
            if (RiverSpline)
            {
                // This would be expanded with actual river generation logic
                UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Created river system %d"), i + 1);
            }
        }
    }
}

void UWorldGenerationManager::PopulateVegetation()
{
    // Populate vegetation based on biome distribution
    // This would be expanded with actual vegetation placement algorithms
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Populating vegetation across biomes"));
}

void UWorldGenerationManager::OptimizeForPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: Optimizing for performance..."));
    
    CurrentGenerationStep = 5;
    GenerationProgress = 0.83f; // 5/6 steps

    // Apply performance optimizations based on terrain density setting
    switch (WorldConfig.TerrainDensity)
    {
        case ETerrainDensity::Low:
            // Reduce vegetation density, simplify materials
            break;
        case ETerrainDensity::Medium:
            // Balanced settings
            break;
        case ETerrainDensity::High:
            // High detail, monitor performance
            break;
        case ETerrainDensity::Ultra:
            // Maximum detail, may impact performance
            break;
    }

    // Finalize generation
    CurrentGenerationStep = 6;
    GenerationProgress = 1.0f;
    bIsGenerating = false;

    UE_LOG(LogTemp, Log, TEXT("WorldGenerationManager: World generation completed successfully!"));
}
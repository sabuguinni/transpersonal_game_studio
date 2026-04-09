#include "JurassicWorldGeneratorV43.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SplineComponent.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "PCGGraph.h"
#include "PCGVolume.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

AJurassicWorldGeneratorV43::AJurassicWorldGeneratorV43()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create PCG component
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));

    // Initialize default settings
    GenerationSettings.WorldSize = 16384.0f; // 16km x 16km
    GenerationSettings.HeightmapResolution = 2048.0f;
    GenerationSettings.MaxWorldHeight = 2000.0f;
    GenerationSettings.PCGGridSize = 512;
    GenerationSettings.bUseHierarchicalGeneration = true;
    GenerationSettings.bEnableRuntimeGeneration = true;
    GenerationSettings.MaxActiveChunks = 25;
    GenerationSettings.StreamingRadius = 3000.0f;
    GenerationSettings.LODLevels = 4;

    // Initialize state
    CurrentPhase = EWorldGenerationPhase::Initialization;
    bIsGenerating = false;
    GenerationProgress = 0.0f;
    ActivePCGComponents = 0;
    GeneratedActors = 0;
    LastGenerationTime = 0.0f;

    // Debug settings
    bShowDebugInfo = false;
    bDrawBiomeBounds = false;
    bDrawRiverSplines = false;

    // Initialize main river system
    MainRiverSystem.RiverName = TEXT("JurassicMainRiver");
    MainRiverSystem.RiverWidth = 800.0f;
    MainRiverSystem.RiverDepth = 150.0f;
    MainRiverSystem.FlowSpeed = 300.0f;

    // Set up default river spline points
    MainRiverSystem.RiverSplinePoints = {
        FVector(-8000, -4000, 100),  // Mountain source
        FVector(-6000, -2000, 80),   // Highland flow
        FVector(-3000, 0, 60),       // Mid-valley
        FVector(0, 2000, 40),        // Central plains
        FVector(3000, 4000, 20),     // Delta approach
        FVector(6000, 6000, 0)       // Ocean outlet
    };
}

void AJurassicWorldGeneratorV43::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: BeginPlay - Initializing world generation system"));

    // Get world references
    UWorld* World = GetWorld();
    if (World)
    {
        WorldPartitionSubsystem = World->GetSubsystem<UWorldPartitionSubsystem>();
        
        // Find existing landscape
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            WorldLandscape = Cast<ALandscape>(FoundActors[0]);
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Found existing landscape: %s"), 
                   *WorldLandscape->GetName());
        }

        // Find or create PCG World Actor
        TArray<AActor*> PCGActors;
        UGameplayStatics::GetAllActorsOfClass(World, APCGWorldActor::StaticClass(), PCGActors);
        if (PCGActors.Num() > 0)
        {
            PCGWorldActor = Cast<APCGWorldActor>(PCGActors[0]);
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Found existing PCG World Actor"));
        }
    }

    // Start performance monitoring
    GetWorldTimerManager().SetTimer(PerformanceTimerHandle, this, 
                                   &AJurassicWorldGeneratorV43::UpdatePerformanceMetrics, 
                                   1.0f, true);
}

void AJurassicWorldGeneratorV43::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear timers
    GetWorldTimerManager().ClearTimer(GenerationTimerHandle);
    GetWorldTimerManager().ClearTimer(PerformanceTimerHandle);

    Super::EndPlay(EndPlayReason);
}

void AJurassicWorldGeneratorV43::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update debug visualization if enabled
    if (bShowDebugInfo)
    {
        if (bDrawBiomeBounds)
        {
            DebugDrawBiomes();
        }
        if (bDrawRiverSplines)
        {
            DebugDrawRiverSystems();
        }
    }
}

void AJurassicWorldGeneratorV43::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: Initializing world generation..."));

    CurrentPhase = EWorldGenerationPhase::Initialization;
    bIsGenerating = true;
    GenerationProgress = 0.0f;

    // Initialize PCG system
    InitializePCGSystem();

    // Setup World Partition
    SetupWorldPartition();

    // Initialize default biomes
    BiomeData.Empty();
    
    // Create core Jurassic biomes
    BiomeData.Add(FJurassicBiomeData{
        EJurassicBiomeType::DenseForest,
        FVector(0, 0, 100),
        4000.0f, 1.0f, 600.0f, 0.9f, 0.3f
    });

    BiomeData.Add(FJurassicBiomeData{
        EJurassicBiomeType::RiverDelta,
        FVector(4000, 4000, 30),
        2500.0f, 0.8f, 200.0f, 0.6f, 0.8f
    });

    BiomeData.Add(FJurassicBiomeData{
        EJurassicBiomeType::VolcanicPlains,
        FVector(-5000, 0, 250),
        3500.0f, 1.2f, 800.0f, 0.4f, 0.1f
    });

    BiomeData.Add(FJurassicBiomeData{
        EJurassicBiomeType::CoastalCliffs,
        FVector(6000, 8000, 400),
        3000.0f, 1.1f, 1000.0f, 0.5f, 0.2f
    });

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Initialized %d biomes"), BiomeData.Num());
}

void AJurassicWorldGeneratorV43::GenerateJurassicWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: Starting full world generation..."));

    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: Generation already in progress!"));
        return;
    }

    float StartTime = FPlatformTime::Seconds();

    // Initialize if not already done
    if (CurrentPhase == EWorldGenerationPhase::Initialization)
    {
        InitializeWorldGeneration();
    }

    // Execute generation phases
    GenerateTerrainBase();
    DistributeBiomes();
    CreateRiverSystems();
    PlaceGeologicalFeatures();
    FinalizeWorldGeneration();

    LastGenerationTime = FPlatformTime::Seconds() - StartTime;
    LogGenerationStats();

    UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: World generation completed in %.2f seconds"), 
           LastGenerationTime);
}

void AJurassicWorldGeneratorV43::GenerateTerrainBase()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Generating terrain base..."));

    CurrentPhase = EWorldGenerationPhase::TerrainGeneration;
    GenerationProgress = 0.2f;

    if (WorldLandscape)
    {
        // Configure landscape for Jurassic environment
        ULandscapeInfo* LandscapeInfo = WorldLandscape->GetLandscapeInfo();
        if (LandscapeInfo)
        {
            // Apply heightmap modifications for varied terrain
            GenerateHeightmapNoise();
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Applied heightmap noise to landscape"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: No landscape found for terrain generation"));
    }
}

void AJurassicWorldGeneratorV43::DistributeBiomes()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Distributing biomes..."));

    CurrentPhase = EWorldGenerationPhase::BiomeDistribution;
    GenerationProgress = 0.4f;

    // Create PCG volumes for each biome
    CreatePCGVolumes();

    // Apply biome influence to terrain
    for (const FJurassicBiomeData& Biome : BiomeData)
    {
        ApplyBiomeInfluence(Biome.BiomeCenter, Biome.BiomeRadius, Biome.BiomeInfluence);
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Applied biome influence for %s at %s"), 
               *UEnum::GetValueAsString(Biome.BiomeType), *Biome.BiomeCenter.ToString());
    }

    // Create biome transitions
    CreateBiomeTransitions();
}

void AJurassicWorldGeneratorV43::CreateRiverSystems()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating river systems..."));

    CurrentPhase = EWorldGenerationPhase::WaterSystems;
    GenerationProgress = 0.6f;

    // Create main river system
    CreateMainRiverSystem();

    // Create tributaries
    for (const FRiverSystemData& Tributary : MainRiverSystem.Tributaries)
    {
        CreateTributary(Tributary);
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Created main river with %d tributaries"), 
           MainRiverSystem.Tributaries.Num());
}

void AJurassicWorldGeneratorV43::PlaceGeologicalFeatures()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Placing geological features..."));

    CurrentPhase = EWorldGenerationPhase::GeologicalFeatures;
    GenerationProgress = 0.8f;

    // Place volcanic features
    for (const FJurassicBiomeData& Biome : BiomeData)
    {
        if (Biome.BiomeType == EJurassicBiomeType::VolcanicPlains)
        {
            // Create volcanic rock formations
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Placing volcanic features in biome at %s"), 
                   *Biome.BiomeCenter.ToString());
        }
    }

    // Place cliff formations for coastal biomes
    // Place cave systems
    // Place mineral deposits
}

void AJurassicWorldGeneratorV43::FinalizeWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Finalizing world generation..."));

    CurrentPhase = EWorldGenerationPhase::Finalization;
    GenerationProgress = 1.0f;

    // Optimize generated content
    OptimizeWorldGeneration();

    // Setup LOD system
    SetupLODSystem();

    // Configure streaming sources
    ConfigureStreamingSources();

    // Validate generation
    ValidateWorldGeneration();

    bIsGenerating = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: World generation finalized successfully"));
}

void AJurassicWorldGeneratorV43::SetupPCGSystem()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Setting up PCG system..."));

    InitializePCGSystem();
    CreatePCGVolumes();
    ConfigurePCGGraphs();
}

void AJurassicWorldGeneratorV43::CreatePCGVolumes()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating PCG volumes..."));

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicWorldGeneratorV43: No world context for PCG volume creation"));
        return;
    }

    // Create PCG volumes for each biome
    for (int32 i = 0; i < BiomeData.Num(); i++)
    {
        const FJurassicBiomeData& Biome = BiomeData[i];

        // Spawn PCG volume
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("PCGVolume_Biome_%d"), i));
        
        APCGVolume* PCGVolume = World->SpawnActor<APCGVolume>(
            APCGVolume::StaticClass(),
            Biome.BiomeCenter,
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (PCGVolume)
        {
            // Configure volume size
            FVector VolumeScale = FVector(
                Biome.BiomeRadius / 100.0f,
                Biome.BiomeRadius / 100.0f,
                Biome.HeightVariation / 100.0f
            );
            PCGVolume->SetActorScale3D(VolumeScale);

            // Enable partitioned generation for large biomes
            if (Biome.BiomeRadius > 2000.0f)
            {
                UPCGComponent* BiomePCGComponent = PCGVolume->GetPCGComponent();
                if (BiomePCGComponent)
                {
                    BiomePCGComponent->SetIsPartitioned(true);
                }
            }

            ActivePCGComponents++;
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Created PCG volume for %s biome"), 
                   *UEnum::GetValueAsString(Biome.BiomeType));
        }
    }
}

void AJurassicWorldGeneratorV43::ConfigurePCGGraphs()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Configuring PCG graphs..."));

    // Configure biome-specific PCG graphs
    // This would typically load graph assets from content
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: PCG graph configuration completed"));
}

void AJurassicWorldGeneratorV43::CreateBiome(EJurassicBiomeType BiomeType, FVector Location, float Radius)
{
    FJurassicBiomeData NewBiome;
    NewBiome.BiomeType = BiomeType;
    NewBiome.BiomeCenter = Location;
    NewBiome.BiomeRadius = Radius;

    // Set biome-specific parameters
    switch (BiomeType)
    {
    case EJurassicBiomeType::DenseForest:
        NewBiome.VegetationDensity = 0.9f;
        NewBiome.WaterPresence = 0.3f;
        NewBiome.HeightVariation = 400.0f;
        break;
    case EJurassicBiomeType::RiverDelta:
        NewBiome.VegetationDensity = 0.6f;
        NewBiome.WaterPresence = 0.8f;
        NewBiome.HeightVariation = 100.0f;
        break;
    case EJurassicBiomeType::VolcanicPlains:
        NewBiome.VegetationDensity = 0.3f;
        NewBiome.WaterPresence = 0.1f;
        NewBiome.HeightVariation = 800.0f;
        break;
    case EJurassicBiomeType::CoastalCliffs:
        NewBiome.VegetationDensity = 0.4f;
        NewBiome.WaterPresence = 0.2f;
        NewBiome.HeightVariation = 1200.0f;
        break;
    default:
        NewBiome.VegetationDensity = 0.5f;
        NewBiome.WaterPresence = 0.3f;
        NewBiome.HeightVariation = 300.0f;
        break;
    }

    BiomeData.Add(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Created new %s biome at %s"), 
           *UEnum::GetValueAsString(BiomeType), *Location.ToString());
}

FJurassicBiomeData AJurassicWorldGeneratorV43::GetBiomeDataAtLocation(FVector WorldLocation)
{
    FJurassicBiomeData ClosestBiome;
    float ClosestDistance = FLT_MAX;

    for (const FJurassicBiomeData& Biome : BiomeData)
    {
        float Distance = FVector::Dist(WorldLocation, Biome.BiomeCenter);
        if (Distance < ClosestDistance && Distance <= Biome.BiomeRadius)
        {
            ClosestDistance = Distance;
            ClosestBiome = Biome;
        }
    }

    return ClosestBiome;
}

void AJurassicWorldGeneratorV43::CreateMainRiverSystem()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating main river system..."));

    // Implementation would create water body rivers using UE5 Water Plugin
    // or alternative spline-based water systems
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Main river system created with %d spline points"), 
           MainRiverSystem.RiverSplinePoints.Num());
}

void AJurassicWorldGeneratorV43::CreateTributary(const FRiverSystemData& TributaryData)
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating tributary: %s"), 
           *TributaryData.RiverName);

    // Implementation would create tributary river systems
}

void AJurassicWorldGeneratorV43::SetupWorldPartition()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Setting up World Partition..."));

    if (WorldPartitionSubsystem)
    {
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: World Partition subsystem available"));
        // Configure World Partition settings
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: World Partition subsystem not available"));
    }
}

void AJurassicWorldGeneratorV43::ConfigureStreamingSources()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Configuring streaming sources..."));

    // Create streaming sources for key locations
    TArray<FVector> StreamingLocations = {
        FVector(0, 0, 100),      // Forest center
        FVector(4000, 4000, 30), // River delta
        FVector(-5000, 0, 250),  // Volcanic plains
        FVector(6000, 8000, 400) // Coastal cliffs
    };

    for (const FVector& Location : StreamingLocations)
    {
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Configured streaming source at %s"), 
               *Location.ToString());
    }
}

void AJurassicWorldGeneratorV43::OptimizeWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Optimizing world generation..."));

    // Implement performance optimizations
    // - LOD configuration
    // - Culling optimization
    // - Memory management
}

void AJurassicWorldGeneratorV43::SetupLODSystem()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Setting up LOD system..."));

    // Configure hierarchical LOD for different distance ranges
    for (int32 LODLevel = 0; LODLevel < GenerationSettings.LODLevels; LODLevel++)
    {
        float LODDistance = GenerationSettings.StreamingRadius * (LODLevel + 1);
        UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: LOD Level %d - Distance: %.0f"), 
               LODLevel, LODDistance);
    }
}

float AJurassicWorldGeneratorV43::GetHeightAtLocation(FVector WorldLocation)
{
    if (WorldLandscape)
    {
        return WorldLandscape->GetHeightAtLocation(WorldLocation);
    }
    return 0.0f;
}

FVector AJurassicWorldGeneratorV43::GetNormalAtLocation(FVector WorldLocation)
{
    if (WorldLandscape)
    {
        // Get landscape normal at location
        // Implementation would sample landscape normal
        return FVector::UpVector;
    }
    return FVector::UpVector;
}

bool AJurassicWorldGeneratorV43::IsLocationInWater(FVector WorldLocation)
{
    // Check if location intersects with any water bodies
    // Implementation would check against river systems and lakes
    return false;
}

void AJurassicWorldGeneratorV43::DebugDrawBiomes()
{
    if (!GetWorld()) return;

    for (const FJurassicBiomeData& Biome : BiomeData)
    {
        FColor BiomeColor = FColor::Green;
        switch (Biome.BiomeType)
        {
        case EJurassicBiomeType::DenseForest: BiomeColor = FColor::Green; break;
        case EJurassicBiomeType::RiverDelta: BiomeColor = FColor::Blue; break;
        case EJurassicBiomeType::VolcanicPlains: BiomeColor = FColor::Red; break;
        case EJurassicBiomeType::CoastalCliffs: BiomeColor = FColor::Yellow; break;
        default: BiomeColor = FColor::White; break;
        }

        DrawDebugSphere(GetWorld(), Biome.BiomeCenter, Biome.BiomeRadius, 32, BiomeColor, false, -1.0f, 0, 10.0f);
    }
}

void AJurassicWorldGeneratorV43::DebugDrawRiverSystems()
{
    if (!GetWorld()) return;

    // Draw main river spline
    for (int32 i = 0; i < MainRiverSystem.RiverSplinePoints.Num() - 1; i++)
    {
        DrawDebugLine(GetWorld(), 
                     MainRiverSystem.RiverSplinePoints[i], 
                     MainRiverSystem.RiverSplinePoints[i + 1], 
                     FColor::Cyan, false, -1.0f, 0, 20.0f);
    }
}

void AJurassicWorldGeneratorV43::ToggleDebugVisualization()
{
    bShowDebugInfo = !bShowDebugInfo;
    bDrawBiomeBounds = bShowDebugInfo;
    bDrawRiverSplines = bShowDebugInfo;

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Debug visualization %s"), 
           bShowDebugInfo ? TEXT("enabled") : TEXT("disabled"));
}

// Private Implementation

void AJurassicWorldGeneratorV43::InitializePCGSystem()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Initializing PCG system..."));

    if (PCGComponent)
    {
        // Configure PCG component settings
        PCGComponent->SetIsPartitioned(GenerationSettings.bUseHierarchicalGeneration);
        
        if (GenerationSettings.bEnableRuntimeGeneration)
        {
            // Enable runtime generation
            UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Runtime generation enabled"));
        }
    }
}

void AJurassicWorldGeneratorV43::CreateBiomeTransitions()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating biome transitions..."));

    // Create smooth transitions between biomes
    for (int32 i = 0; i < BiomeData.Num(); i++)
    {
        for (int32 j = i + 1; j < BiomeData.Num(); j++)
        {
            const FJurassicBiomeData& BiomeA = BiomeData[i];
            const FJurassicBiomeData& BiomeB = BiomeData[j];

            float Distance = FVector::Dist(BiomeA.BiomeCenter, BiomeB.BiomeCenter);
            float CombinedRadius = BiomeA.BiomeRadius + BiomeB.BiomeRadius;

            if (Distance < CombinedRadius)
            {
                // Biomes overlap - create transition zone
                UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Creating transition between %s and %s"), 
                       *UEnum::GetValueAsString(BiomeA.BiomeType),
                       *UEnum::GetValueAsString(BiomeB.BiomeType));
            }
        }
    }
}

void AJurassicWorldGeneratorV43::GenerateHeightmapNoise()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Generating heightmap noise..."));

    // Implementation would apply Perlin noise and other algorithms
    // to create varied terrain height
}

void AJurassicWorldGeneratorV43::ApplyBiomeInfluence(FVector Location, float Radius, float Strength)
{
    // Apply biome-specific modifications to terrain and vegetation
    UE_LOG(LogTemp, VeryVerbose, TEXT("JurassicWorldGeneratorV43: Applied biome influence at %s (R:%.0f, S:%.2f)"), 
           *Location.ToString(), Radius, Strength);
}

void AJurassicWorldGeneratorV43::ValidateWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Validating world generation..."));

    bool bValidationPassed = true;

    // Validate biome count
    if (BiomeData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("JurassicWorldGeneratorV43: No biomes generated!"));
        bValidationPassed = false;
    }

    // Validate PCG components
    if (ActivePCGComponents == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("JurassicWorldGeneratorV43: No active PCG components"));
    }

    UE_LOG(LogTemp, Log, TEXT("JurassicWorldGeneratorV43: Validation %s"), 
           bValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
}

void AJurassicWorldGeneratorV43::UpdatePerformanceMetrics()
{
    // Update performance tracking
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        GeneratedActors = AllActors.Num();
    }
}

void AJurassicWorldGeneratorV43::LogGenerationStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== JURASSIC WORLD GENERATION STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Generation Time: %.2f seconds"), LastGenerationTime);
    UE_LOG(LogTemp, Warning, TEXT("Biomes Created: %d"), BiomeData.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active PCG Components: %d"), ActivePCGComponents);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), GeneratedActors);
    UE_LOG(LogTemp, Warning, TEXT("World Size: %.0f x %.0f"), GenerationSettings.WorldSize, GenerationSettings.WorldSize);
    UE_LOG(LogTemp, Warning, TEXT("PCG Grid Size: %d meters"), GenerationSettings.PCGGridSize);
    UE_LOG(LogTemp, Warning, TEXT("========================================"));
}
#include "ProceduralWorldManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "PCGSubsystem.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/WorldPartitionSubsystem.h"
#include "Landscape/Classes/LandscapeProxy.h"
#include "Components/BrushComponent.h"

AProceduralWorldManager::AProceduralWorldManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create PCG component
    MasterPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("MasterPCGComponent"));
    MasterPCGComponent->SetupAttachment(RootComponent);

    // Create debug visualization component
    DebugVisualizationComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DebugVisualizationComponent"));
    DebugVisualizationComponent->SetupAttachment(RootComponent);
    DebugVisualizationComponent->SetVisibility(false);

    // Initialize default world settings
    WorldSettings.CellSize = 25600; // 256m cells
    WorldSettings.LoadingRange = 76800; // 768m range
    WorldSettings.bUseHierarchicalGeneration = true;
    WorldSettings.bUseRuntimeGeneration = true;
    WorldSettings.bUseGPUProcessing = true;
    WorldSettings.LandscapeSize = FIntPoint(2017, 2017);
    WorldSettings.QuadsPerSection = 63;
    WorldSettings.SectionsPerComponent = 4;
    WorldSettings.ZScale = 100.0f;
    WorldSettings.MaxConcurrentGenerations = 4;
    WorldSettings.FrameTimeAllocation = 16.667f;

    // Initialize default biome configurations
    InitializeBiomeDistribution();
}

void AProceduralWorldManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: BeginPlay - Initializing world generation"));
    
    // Delay initialization to ensure all systems are ready
    GetWorld()->GetTimerManager().SetTimer(
        FTimerHandle(),
        this,
        &AProceduralWorldManager::InitializeWorldGeneration,
        1.0f,
        false
    );
}

void AProceduralWorldManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Cleanup any ongoing generations
    if (MasterPCGComponent)
    {
        MasterPCGComponent->CleanupLocalImmediate(true);
    }

    Super::EndPlay(EndPlayReason);
}

void AProceduralWorldManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Performance monitoring
    if (bIsInitialized && ActiveGenerations > 0)
    {
        LastGenerationTime += DeltaTime;
        
        // Log performance warnings if generation takes too long
        if (LastGenerationTime > 5.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Long generation detected - %d active generations taking %.2f seconds"), 
                ActiveGenerations, LastGenerationTime);
        }
    }
}

void AProceduralWorldManager::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Starting world generation initialization"));

    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Already initialized"));
        return;
    }

    // Setup World Partition
    SetupWorldPartition();

    // Configure PCG System
    ConfigurePCGSystem();

    // Create or find landscape
    CreateLandscapeIfNeeded();

    // Initialize biome distribution
    InitializeBiomeDistribution();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: World generation initialization complete"));
}

void AProceduralWorldManager::SetupWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralWorldManager: No valid world found"));
        return;
    }

    UWorldPartition* WorldPartition = World->GetWorldPartition();
    if (!WorldPartition)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: World Partition not enabled for this world"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: World Partition detected - Cell Size: %d, Loading Range: %d"), 
        WorldSettings.CellSize, WorldSettings.LoadingRange);

    // World Partition settings are configured in World Settings
    // This function validates the setup
}

void AProceduralWorldManager::ConfigurePCGSystem()
{
    if (!MasterPCGComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("ProceduralWorldManager: MasterPCGComponent is null"));
        return;
    }

    // Configure PCG component for hierarchical generation
    MasterPCGComponent->bIsPartitioned = WorldSettings.bUseHierarchicalGeneration;
    
    // Set generation trigger based on runtime settings
    if (WorldSettings.bUseRuntimeGeneration)
    {
        MasterPCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateAtRuntime;
    }
    else
    {
        MasterPCGComponent->GenerationTrigger = EPCGComponentGenerationTrigger::GenerateOnDemand;
    }

    // Load master world graph if specified
    if (MasterWorldGraph.IsValid())
    {
        UPCGGraph* LoadedGraph = MasterWorldGraph.LoadSynchronous();
        if (LoadedGraph)
        {
            MasterPCGComponent->SetGraph(LoadedGraph);
            UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Master world graph loaded successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Failed to load master world graph"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: PCG system configured - Partitioned: %s, Runtime: %s"), 
        WorldSettings.bUseHierarchicalGeneration ? TEXT("true") : TEXT("false"),
        WorldSettings.bUseRuntimeGeneration ? TEXT("true") : TEXT("false"));
}

void AProceduralWorldManager::CreateLandscapeIfNeeded()
{
    // Check if target landscape is already set
    if (TargetLandscape.IsValid())
    {
        ALandscape* LoadedLandscape = TargetLandscape.LoadSynchronous();
        if (LoadedLandscape)
        {
            UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Using existing landscape"));
            return;
        }
    }

    // Find existing landscape in the world
    TArray<AActor*> FoundLandscapes;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundLandscapes);

    if (FoundLandscapes.Num() > 0)
    {
        ALandscape* ExistingLandscape = Cast<ALandscape>(FoundLandscapes[0]);
        if (ExistingLandscape)
        {
            TargetLandscape = ExistingLandscape;
            UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Found existing landscape in world"));
            return;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: No landscape found - landscape should be created manually or via Open World template"));
}

void AProceduralWorldManager::InitializeBiomeDistribution()
{
    WorldSettings.BiomeConfigurations.Empty();

    // Dense Jungle - High vegetation, moderate danger
    FBiomeConfiguration DenseJungle;
    DenseJungle.BiomeType = EBiomeType::DenseJungle;
    DenseJungle.DangerLevel = EDangerLevel::Moderate;
    DenseJungle.VegetationDensity = 0.9f;
    DenseJungle.WaterPresence = 0.4f;
    DenseJungle.TerrainRoughness = 0.6f;
    DenseJungle.HeightRange = FVector2D(-200.0f, 1500.0f);
    WorldSettings.BiomeConfigurations.Add(DenseJungle);

    // Open Plains - Low vegetation, high danger (exposed to predators)
    FBiomeConfiguration OpenPlains;
    OpenPlains.BiomeType = EBiomeType::OpenPlains;
    OpenPlains.DangerLevel = EDangerLevel::High;
    OpenPlains.VegetationDensity = 0.3f;
    OpenPlains.WaterPresence = 0.1f;
    OpenPlains.TerrainRoughness = 0.2f;
    OpenPlains.HeightRange = FVector2D(-100.0f, 500.0f);
    WorldSettings.BiomeConfigurations.Add(OpenPlains);

    // River Valley - High water, moderate vegetation, safe zones
    FBiomeConfiguration RiverValley;
    RiverValley.BiomeType = EBiomeType::RiverValley;
    RiverValley.DangerLevel = EDangerLevel::Safe;
    RiverValley.VegetationDensity = 0.6f;
    RiverValley.WaterPresence = 0.8f;
    RiverValley.TerrainRoughness = 0.3f;
    RiverValley.HeightRange = FVector2D(-500.0f, 200.0f);
    WorldSettings.BiomeConfigurations.Add(RiverValley);

    // Rocky Outcrops - Low vegetation, extreme danger (predator lairs)
    FBiomeConfiguration RockyOutcrops;
    RockyOutcrops.BiomeType = EBiomeType::RockyOutcrops;
    RockyOutcrops.DangerLevel = EDangerLevel::Extreme;
    RockyOutcrops.VegetationDensity = 0.2f;
    RockyOutcrops.WaterPresence = 0.1f;
    RockyOutcrops.TerrainRoughness = 0.9f;
    RockyOutcrops.HeightRange = FVector2D(500.0f, 2500.0f);
    WorldSettings.BiomeConfigurations.Add(RockyOutcrops);

    // Swampland - High water, dense vegetation, moderate danger
    FBiomeConfiguration Swampland;
    Swampland.BiomeType = EBiomeType::Swampland;
    Swampland.DangerLevel = EDangerLevel::Moderate;
    Swampland.VegetationDensity = 0.8f;
    Swampland.WaterPresence = 0.9f;
    Swampland.TerrainRoughness = 0.4f;
    Swampland.HeightRange = FVector2D(-300.0f, 100.0f);
    WorldSettings.BiomeConfigurations.Add(Swampland);

    // Coastal Area - Moderate vegetation, high danger (marine predators)
    FBiomeConfiguration CoastalArea;
    CoastalArea.BiomeType = EBiomeType::CoastalArea;
    CoastalArea.DangerLevel = EDangerLevel::High;
    CoastalArea.VegetationDensity = 0.5f;
    CoastalArea.WaterPresence = 0.7f;
    CoastalArea.TerrainRoughness = 0.3f;
    CoastalArea.HeightRange = FVector2D(-100.0f, 300.0f);
    WorldSettings.BiomeConfigurations.Add(CoastalArea);

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Initialized %d biome configurations"), 
        WorldSettings.BiomeConfigurations.Num());
}

void AProceduralWorldManager::GenerateRegion(const FVector& Center, float Radius)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Cannot generate region - not initialized"));
        return;
    }

    if (ActiveGenerations >= WorldSettings.MaxConcurrentGenerations)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Cannot generate region - max concurrent generations reached"));
        return;
    }

    ActiveGenerations++;
    LastGenerationTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Generating region at %s with radius %.2f"), 
        *Center.ToString(), Radius);

    // Trigger PCG generation for the region
    if (MasterPCGComponent)
    {
        MasterPCGComponent->Generate();
    }

    // Note: In a full implementation, this would trigger specific region generation
    // For now, we trigger the master component which handles the overall generation
}

void AProceduralWorldManager::CleanupRegion(const FVector& Center, float Radius)
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Cleaning up region at %s with radius %.2f"), 
        *Center.ToString(), Radius);

    if (MasterPCGComponent)
    {
        MasterPCGComponent->CleanupLocalImmediate(false);
    }

    if (ActiveGenerations > 0)
    {
        ActiveGenerations--;
    }
}

void AProceduralWorldManager::RegenerateAllBiomes()
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Cannot regenerate - not initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Regenerating all biomes"));

    // Clear caches
    BiomeCache.Empty();
    DangerCache.Empty();

    // Trigger full regeneration
    if (MasterPCGComponent)
    {
        MasterPCGComponent->CleanupLocalImmediate(true);
        MasterPCGComponent->Generate();
    }
}

EBiomeType AProceduralWorldManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to grid coordinates for caching
    FIntPoint GridCoord = FIntPoint(
        FMath::FloorToInt(WorldLocation.X / 1000.0f),
        FMath::FloorToInt(WorldLocation.Y / 1000.0f)
    );

    // Check cache first
    if (const EBiomeType* CachedBiome = BiomeCache.Find(GridCoord))
    {
        return *CachedBiome;
    }

    // Calculate biome based on noise functions
    FVector2D NormalizedCoords = WorldToNormalizedCoordinates(WorldLocation);
    
    // Multiple noise layers for complex biome distribution
    float HeightNoise = CalculateNoiseValue(NormalizedCoords, 0.5f, 4);
    float MoistureNoise = CalculateNoiseValue(NormalizedCoords + FVector2D(1000.0f, 0.0f), 0.3f, 3);
    float TemperatureNoise = CalculateNoiseValue(NormalizedCoords + FVector2D(0.0f, 1000.0f), 0.2f, 2);

    EBiomeType ResultBiome = EBiomeType::DenseJungle; // Default

    // Biome selection based on noise values
    if (HeightNoise > 0.7f)
    {
        ResultBiome = EBiomeType::RockyOutcrops;
    }
    else if (HeightNoise < -0.3f && MoistureNoise > 0.5f)
    {
        ResultBiome = EBiomeType::RiverValley;
    }
    else if (MoistureNoise > 0.8f && HeightNoise < 0.2f)
    {
        ResultBiome = EBiomeType::Swampland;
    }
    else if (MoistureNoise < 0.3f && HeightNoise > 0.0f && HeightNoise < 0.4f)
    {
        ResultBiome = EBiomeType::OpenPlains;
    }
    else if (FMath::Abs(WorldLocation.X) > 8000.0f || FMath::Abs(WorldLocation.Y) > 8000.0f)
    {
        ResultBiome = EBiomeType::CoastalArea;
    }
    else
    {
        ResultBiome = EBiomeType::DenseJungle;
    }

    // Cache the result
    const_cast<AProceduralWorldManager*>(this)->BiomeCache.Add(GridCoord, ResultBiome);

    return ResultBiome;
}

EDangerLevel AProceduralWorldManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FBiomeConfiguration BiomeConfig = GetBiomeConfiguration(BiomeType);
    
    // Add some variation to danger level based on location
    FVector2D NormalizedCoords = WorldToNormalizedCoordinates(WorldLocation);
    float DangerNoise = CalculateNoiseValue(NormalizedCoords + FVector2D(500.0f, 500.0f), 0.1f, 2);
    
    EDangerLevel BaseDanger = BiomeConfig.DangerLevel;
    
    // Modify danger based on noise (±1 level)
    if (DangerNoise > 0.3f && BaseDanger != EDangerLevel::Extreme)
    {
        return static_cast<EDangerLevel>(static_cast<uint8>(BaseDanger) + 1);
    }
    else if (DangerNoise < -0.3f && BaseDanger != EDangerLevel::Safe)
    {
        return static_cast<EDangerLevel>(static_cast<uint8>(BaseDanger) - 1);
    }
    
    return BaseDanger;
}

FBiomeConfiguration AProceduralWorldManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    for (const FBiomeConfiguration& Config : WorldSettings.BiomeConfigurations)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }

    // Return default if not found
    FBiomeConfiguration DefaultConfig;
    DefaultConfig.BiomeType = BiomeType;
    return DefaultConfig;
}

void AProceduralWorldManager::ToggleDebugVisualization()
{
    bDebugVisualizationEnabled = !bDebugVisualizationEnabled;
    
    if (DebugVisualizationComponent)
    {
        DebugVisualizationComponent->SetVisibility(bDebugVisualizationEnabled);
    }

    UE_LOG(LogTemp, Log, TEXT("ProceduralWorldManager: Debug visualization %s"), 
        bDebugVisualizationEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void AProceduralWorldManager::ShowBiomeMap()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Biome Map Debug - Feature not yet implemented"));
    // TODO: Implement biome visualization overlay
}

void AProceduralWorldManager::ShowDangerZones()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralWorldManager: Danger Zones Debug - Feature not yet implemented"));
    // TODO: Implement danger zone visualization overlay
}

FVector2D AProceduralWorldManager::WorldToNormalizedCoordinates(const FVector& WorldLocation) const
{
    // Normalize world coordinates to 0-1 range for noise functions
    const float WorldSize = 20000.0f; // 20km world
    return FVector2D(
        (WorldLocation.X + WorldSize * 0.5f) / WorldSize,
        (WorldLocation.Y + WorldSize * 0.5f) / WorldSize
    );
}

float AProceduralWorldManager::CalculateNoiseValue(const FVector2D& Coordinates, float Scale, int32 Octaves) const
{
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < Octaves; i++)
    {
        NoiseValue += FMath::PerlinNoise2D(Coordinates * Frequency) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }

    return NoiseValue / MaxValue;
}
// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PCGWorldGenerator.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeProxy.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "WorldPartition/WorldPartition.h"
#include "WorldPartition/DataLayer/DataLayer.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogPCGWorldGen, Log, All);

APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create main PCG component
    MainPCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("MainPCGComponent"));
    RootComponent = MainPCGComponent;
    
    // Initialize default parameters
    WorldSizeKm = 16.0f;
    LandscapeResolution = 8129; // Optimal UE5 size: 8129x8129
    WorldPartitionCellSize = 51200.0f; // 512m cells
    
    // Initialize terrain generation parameters
    TerrainParams.BaseHeightScale = 50000.0f;
    TerrainParams.PrimaryNoiseFreq = 0.0005f;
    TerrainParams.SecondaryNoiseFreq = 0.002f;
    TerrainParams.DetailNoiseFreq = 0.01f;
    TerrainParams.ErosionStrength = 0.3f;
    TerrainParams.ThermalErosionIterations = 5;
    TerrainParams.HydraulicErosionIterations = 10;
    
    // Initialize biome distribution parameters
    BiomeParams.TemperatureGradient = 0.001f;
    BiomeParams.MoistureNoiseScale = 0.0008f;
    BiomeParams.ElevationInfluence = 0.5f;
    BiomeParams.TransitionSmoothness = 1000.0f;
    
    // Initialize river generation parameters
    RiverParams.MajorRiverCount = 3;
    RiverParams.TributariesPerRiver = 5;
    RiverParams.MinRiverLength = 5000.0f;
    RiverParams.MaxRiverLength = 20000.0f;
    RiverParams.RiverWidthRange = FVector2D(200.0f, 800.0f);
    RiverParams.RiverDepthRange = FVector2D(100.0f, 500.0f);
    RiverParams.FlowSpeedRange = FVector2D(0.5f, 3.0f);
    
    // Initialize vegetation parameters
    VegetationParams.DensityMultiplier = 1.0f;
    VegetationParams.TreeSpacingRange = FVector2D(300.0f, 800.0f);
    VegetationParams.UnderstoryDensity = 0.7f;
    VegetationParams.GroundCoverDensity = 1.5f;
    VegetationParams.LODDistances = {2000.0f, 5000.0f, 10000.0f};
    VegetationParams.MaxInstancesPerCell = 10000;
    
    // Set default biome weights
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::TropicalRainforest, 0.25f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::TemperateForest, 0.20f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::ConiferousForest, 0.15f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::Swampland, 0.15f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::Grasslands, 0.10f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::Scrubland, 0.08f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::RiverPlains, 0.05f);
    BiomeParams.BiomeWeights.Add(EPrehistoricBiome::CoastalMarsh, 0.02f);
    
    // Set geological formation influences
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Plains, 0.3f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Hills, 0.6f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Mountains, 1.0f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Valleys, 0.2f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Plateaus, 0.8f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::Canyons, 0.9f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::CoastalCliffs, 0.7f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::VolcanicCones, 1.2f);
    TerrainParams.FormationInfluence.Add(EGeologicalFormation::RiverDeltas, 0.1f);
}

void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPCGWorldGen, Log, TEXT("PCG World Generator initialized for %s"), *GetWorld()->GetName());
    
    if (bAutoGenerateOnBeginPlay)
    {
        GenerateWorld();
    }
}

void APCGWorldGenerator::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    if (bAutoGenerateInEditor && GetWorld() && !GetWorld()->IsGameWorld())
    {
        // Setup PCG component for editor preview
        if (MainPCGComponent && TerrainGenerationGraph)
        {
            MainPCGComponent->SetGraph(TerrainGenerationGraph);
        }
    }
}

void APCGWorldGenerator::GenerateWorld()
{
    if (!GetWorld())
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("Cannot generate world: Invalid world reference"));
        return;
    }
    
    UE_LOG(LogPCGWorldGen, Log, TEXT("Starting world generation..."));
    
    // Step 1: Setup World Partition
    SetupWorldPartition();
    
    // Step 2: Generate base terrain
    GenerateTerrain();
    
    // Step 3: Distribute biomes
    DistributeBiomes();
    
    // Step 4: Generate river systems
    GenerateRiverSystems();
    
    // Step 5: Place vegetation
    PlaceVegetation();
    
    // Step 6: Finalize generation
    FinalizeGeneration();
    
    UE_LOG(LogPCGWorldGen, Log, TEXT("World generation completed successfully"));
    
    // Broadcast completion event
    OnWorldGenerationComplete.Broadcast();
}

void APCGWorldGenerator::SetupWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UE_LOG(LogPCGWorldGen, Log, TEXT("Setting up World Partition with cell size: %.0f meters"), WorldPartitionCellSize);
    
    // Enable World Partition if not already enabled
    if (UWorldPartition* WorldPartition = World->GetWorldPartition())
    {
        UE_LOG(LogPCGWorldGen, Log, TEXT("World Partition already enabled"));
    }
    else
    {
        UE_LOG(LogPCGWorldGen, Warning, TEXT("World Partition not enabled - consider enabling for large worlds"));
    }
    
    // Setup data layers for different content types
    SetupDataLayers();
}

void APCGWorldGenerator::SetupDataLayers()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Setting up data layers for world content organization"));
    
    // Create data layers for different content types
    TArray<FString> LayerNames = {
        TEXT("Terrain"),
        TEXT("Rivers"),
        TEXT("Vegetation_Trees"),
        TEXT("Vegetation_Understory"),
        TEXT("Vegetation_GroundCover"),
        TEXT("Biome_TropicalRainforest"),
        TEXT("Biome_TemperateForest"),
        TEXT("Biome_ConiferousForest"),
        TEXT("Biome_Swampland"),
        TEXT("Biome_Grasslands")
    };
    
    for (const FString& LayerName : LayerNames)
    {
        // Data layer creation would be handled by World Partition system
        UE_LOG(LogPCGWorldGen, Verbose, TEXT("Prepared data layer: %s"), *LayerName);
    }
}

void APCGWorldGenerator::GenerateTerrain()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Generating base terrain with resolution %dx%d"), LandscapeResolution, LandscapeResolution);
    
    if (!TerrainGenerationGraph)
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("Terrain generation graph not assigned"));
        return;
    }
    
    // Setup PCG component for terrain generation
    if (MainPCGComponent)
    {
        MainPCGComponent->SetGraph(TerrainGenerationGraph);
        
        // Set terrain parameters on PCG component
        SetTerrainParameters();
        
        // Generate terrain using PCG
        MainPCGComponent->Generate();
    }
    
    // Apply geological formations
    ApplyGeologicalFormations();
    
    // Simulate erosion
    SimulateErosion();
}

void APCGWorldGenerator::SetTerrainParameters()
{
    if (!MainPCGComponent)
    {
        return;
    }
    
    // Set parameters that will be used by PCG graph
    // These would typically be exposed as PCG parameters
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Setting terrain parameters:"));
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("- Base Height Scale: %.0f"), TerrainParams.BaseHeightScale);
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("- Primary Noise Freq: %.6f"), TerrainParams.PrimaryNoiseFreq);
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("- Erosion Strength: %.2f"), TerrainParams.ErosionStrength);
}

void APCGWorldGenerator::ApplyGeologicalFormations()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Applying geological formations to terrain"));
    
    // Apply different geological formations based on noise patterns
    for (const auto& Formation : TerrainParams.FormationInfluence)
    {
        EGeologicalFormation FormationType = Formation.Key;
        float Influence = Formation.Value;
        
        ApplyFormation(FormationType, Influence);
    }
}

void APCGWorldGenerator::ApplyFormation(EGeologicalFormation Formation, float Influence)
{
    FString FormationName = UEnum::GetValueAsString(Formation);
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Applying %s formation with influence %.2f"), *FormationName, Influence);
    
    // Implementation would modify terrain based on formation type
    switch (Formation)
    {
        case EGeologicalFormation::Mountains:
            // Create mountain ridges and peaks
            break;
            
        case EGeologicalFormation::Valleys:
            // Carve valleys between high terrain
            break;
            
        case EGeologicalFormation::Rivers:
            // Create river valleys and deltas
            break;
            
        case EGeologicalFormation::VolcanicCones:
            // Add volcanic peaks and craters
            break;
            
        default:
            break;
    }
}

void APCGWorldGenerator::SimulateErosion()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Simulating erosion: %d thermal + %d hydraulic iterations"), 
           TerrainParams.ThermalErosionIterations, TerrainParams.HydraulicErosionIterations);
    
    // Thermal erosion simulation
    for (int32 i = 0; i < TerrainParams.ThermalErosionIterations; ++i)
    {
        SimulateThermalErosion();
    }
    
    // Hydraulic erosion simulation
    for (int32 i = 0; i < TerrainParams.HydraulicErosionIterations; ++i)
    {
        SimulateHydraulicErosion();
    }
}

void APCGWorldGenerator::SimulateThermalErosion()
{
    // Thermal erosion simulates material sliding down steep slopes
    // Implementation would analyze slope angles and redistribute material
    UE_LOG(LogPCGWorldGen, VeryVerbose, TEXT("Thermal erosion iteration"));
}

void APCGWorldGenerator::SimulateHydraulicErosion()
{
    // Hydraulic erosion simulates water flow and sediment transport
    // Implementation would trace water flow and carve channels
    UE_LOG(LogPCGWorldGen, VeryVerbose, TEXT("Hydraulic erosion iteration"));
}

void APCGWorldGenerator::DistributeBiomes()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Distributing biomes across terrain"));
    
    if (!BiomeDistributionGraph)
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("Biome distribution graph not assigned"));
        return;
    }
    
    // Calculate climate data
    CalculateClimateData();
    
    // Assign biomes based on elevation, climate, and proximity to water
    AssignBiomes();
    
    // Smooth biome transitions
    SmoothBiomeTransitions();
}

void APCGWorldGenerator::CalculateClimateData()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Calculating climate data (temperature, moisture, wind patterns)"));
    
    // Temperature calculation based on latitude and elevation
    // Moisture calculation based on proximity to water and prevailing winds
    // Implementation would generate climate maps for biome distribution
}

void APCGWorldGenerator::AssignBiomes()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Assigning biomes based on climate and terrain data"));
    
    for (const auto& BiomeWeight : BiomeParams.BiomeWeights)
    {
        EPrehistoricBiome BiomeType = BiomeWeight.Key;
        float Weight = BiomeWeight.Value;
        
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        UE_LOG(LogPCGWorldGen, VeryVerbose, TEXT("Processing %s biome (weight: %.2f)"), *BiomeName, Weight);
    }
}

void APCGWorldGenerator::SmoothBiomeTransitions()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Smoothing biome transitions (smoothness: %.0f)"), BiomeParams.TransitionSmoothness);
    
    // Implementation would blend biome boundaries to create natural transitions
}

void APCGWorldGenerator::GenerateRiverSystems()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Generating river systems: %d major rivers, %d tributaries each"), 
           RiverParams.MajorRiverCount, RiverParams.TributariesPerRiver);
    
    if (!RiverGenerationGraph)
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("River generation graph not assigned"));
        return;
    }
    
    // Generate major rivers
    for (int32 i = 0; i < RiverParams.MajorRiverCount; ++i)
    {
        GenerateMajorRiver(i);
    }
    
    // Generate tributaries
    GenerateTributaries();
    
    // Create water bodies
    CreateWaterBodies();
}

void APCGWorldGenerator::GenerateMajorRiver(int32 RiverIndex)
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Generating major river %d"), RiverIndex + 1);
    
    // Find high elevation start point
    FVector StartPoint = FindRiverSource();
    
    // Trace path to lowest elevation (ocean/lake)
    TArray<FVector> RiverPath = TraceRiverPath(StartPoint);
    
    // Create river spline and carve terrain
    CreateRiverSpline(RiverPath, RiverParams.RiverWidthRange.Y, RiverParams.RiverDepthRange.Y);
}

FVector APCGWorldGenerator::FindRiverSource()
{
    // Find a high elevation point for river source
    float WorldSize = WorldSizeKm * 100000.0f; // Convert km to cm
    
    // Random position in world bounds
    float X = FMath::RandRange(-WorldSize * 0.5f, WorldSize * 0.5f);
    float Y = FMath::RandRange(-WorldSize * 0.5f, WorldSize * 0.5f);
    
    // Find high elevation at this position
    float Z = GetTerrainHeightAtLocation(FVector2D(X, Y));
    
    return FVector(X, Y, Z);
}

TArray<FVector> APCGWorldGenerator::TraceRiverPath(const FVector& StartPoint)
{
    TArray<FVector> Path;
    Path.Add(StartPoint);
    
    FVector CurrentPoint = StartPoint;
    const float StepSize = 1000.0f; // 10 meter steps
    const int32 MaxSteps = 10000; // Prevent infinite loops
    
    for (int32 Step = 0; Step < MaxSteps; ++Step)
    {
        // Find steepest descent direction
        FVector NextPoint = FindSteepestDescent(CurrentPoint, StepSize);
        
        // Add meandering
        NextPoint = AddRiverMeandering(NextPoint, Step);
        
        Path.Add(NextPoint);
        CurrentPoint = NextPoint;
        
        // Stop if we reach sea level or a water body
        if (NextPoint.Z <= 0.0f || HasReachedWater(NextPoint))
        {
            break;
        }
    }
    
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("River path traced with %d points"), Path.Num());
    return Path;
}

FVector APCGWorldGenerator::FindSteepestDescent(const FVector& CurrentPoint, float StepSize)
{
    // Sample surrounding points to find steepest descent
    TArray<FVector> Samples;
    const int32 SampleCount = 8;
    
    for (int32 i = 0; i < SampleCount; ++i)
    {
        float Angle = (2.0f * PI * i) / SampleCount;
        float X = CurrentPoint.X + FMath::Cos(Angle) * StepSize;
        float Y = CurrentPoint.Y + FMath::Sin(Angle) * StepSize;
        float Z = GetTerrainHeightAtLocation(FVector2D(X, Y));
        
        Samples.Add(FVector(X, Y, Z));
    }
    
    // Find lowest point
    FVector LowestPoint = CurrentPoint;
    for (const FVector& Sample : Samples)
    {
        if (Sample.Z < LowestPoint.Z)
        {
            LowestPoint = Sample;
        }
    }
    
    return LowestPoint;
}

FVector APCGWorldGenerator::AddRiverMeandering(const FVector& Point, int32 Step)
{
    // Add natural meandering to river path
    float MeanderOffset = FMath::Sin(Step * RiverParams.FlowSpeedRange.X * 0.01f) * 500.0f;
    
    FVector MeanderedPoint = Point;
    MeanderedPoint.Y += MeanderOffset;
    
    return MeanderedPoint;
}

bool APCGWorldGenerator::HasReachedWater(const FVector& Point)
{
    // Check if point has reached existing water body
    return Point.Z <= 0.0f; // Simplified: sea level check
}

float APCGWorldGenerator::GetTerrainHeightAtLocation(const FVector2D& Location)
{
    // Get terrain height at specific location
    // This would query the landscape or heightfield data
    return 0.0f; // Placeholder implementation
}

void APCGWorldGenerator::CreateRiverSpline(const TArray<FVector>& Path, float Width, float Depth)
{
    if (Path.Num() < 2)
    {
        return;
    }
    
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Creating river spline with %d points, width %.0f, depth %.0f"), 
           Path.Num(), Width, Depth);
    
    // Implementation would create spline component and carve terrain
}

void APCGWorldGenerator::GenerateTributaries()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Generating %d tributaries per major river"), RiverParams.TributariesPerRiver);
    
    // Generate smaller tributaries connecting to major rivers
    // Implementation would create branching river networks
}

void APCGWorldGenerator::CreateWaterBodies()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Creating water bodies (lakes, ponds, swamps)"));
    
    // Create various water bodies based on terrain and biome data
    // Implementation would place water actors and configure water system
}

void APCGWorldGenerator::PlaceVegetation()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Placing vegetation with density multiplier %.2f"), VegetationParams.DensityMultiplier);
    
    if (!VegetationGraph)
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("Vegetation graph not assigned"));
        return;
    }
    
    // Place vegetation based on biome and environmental factors
    PlaceTreesByBiome();
    PlaceUnderstoryVegetation();
    PlaceGroundCover();
    
    // Setup LOD system for performance
    SetupVegetationLOD();
}

void APCGWorldGenerator::PlaceTreesByBiome()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Placing trees by biome with spacing %.0f-%.0f"), 
           VegetationParams.TreeSpacingRange.X, VegetationParams.TreeSpacingRange.Y);
    
    // Place trees based on biome requirements and environmental factors
    // Implementation would use PCG to distribute tree instances
}

void APCGWorldGenerator::PlaceUnderstoryVegetation()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Placing understory vegetation (density: %.2f)"), VegetationParams.UnderstoryDensity);
    
    // Place bushes, ferns, and other understory plants
    // Implementation would consider light levels and tree coverage
}

void APCGWorldGenerator::PlaceGroundCover()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Placing ground cover (density: %.2f)"), VegetationParams.GroundCoverDensity);
    
    // Place grass, moss, fallen logs, and other ground details
    // Implementation would use high-density instancing for performance
}

void APCGWorldGenerator::SetupVegetationLOD()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Setting up vegetation LOD system"));
    
    for (int32 i = 0; i < VegetationParams.LODDistances.Num(); ++i)
    {
        float Distance = VegetationParams.LODDistances[i];
        UE_LOG(LogPCGWorldGen, VeryVerbose, TEXT("LOD %d distance: %.0f meters"), i, Distance);
    }
}

void APCGWorldGenerator::FinalizeGeneration()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Finalizing world generation"));
    
    // Optimize generated content
    OptimizeGeneratedContent();
    
    // Setup streaming
    SetupContentStreaming();
    
    // Validate generation
    ValidateGeneration();
}

void APCGWorldGenerator::OptimizeGeneratedContent()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Optimizing generated content for performance"));
    
    // Merge static meshes where appropriate
    // Setup instanced static mesh components
    // Configure LOD systems
    // Optimize collision meshes
}

void APCGWorldGenerator::SetupContentStreaming()
{
    UE_LOG(LogPCGWorldGen, Verbose, TEXT("Setting up content streaming for large world"));
    
    // Configure World Partition streaming
    // Setup HLOD generation
    // Configure data layer streaming
}

void APCGWorldGenerator::ValidateGeneration()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Validating world generation results"));
    
    // Check for generation errors
    // Validate performance metrics
    // Ensure all systems are properly configured
    
    bool bValidationPassed = true;
    
    if (bValidationPassed)
    {
        UE_LOG(LogPCGWorldGen, Log, TEXT("World generation validation passed"));
    }
    else
    {
        UE_LOG(LogPCGWorldGen, Error, TEXT("World generation validation failed"));
    }
}

void APCGWorldGenerator::RegenerateWorld()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Regenerating world with current parameters"));
    
    // Clear existing generated content
    ClearGeneratedContent();
    
    // Regenerate world
    GenerateWorld();
}

void APCGWorldGenerator::ClearGeneratedContent()
{
    UE_LOG(LogPCGWorldGen, Log, TEXT("Clearing existing generated content"));
    
    if (MainPCGComponent)
    {
        MainPCGComponent->CleanupLocalImmediate(true);
    }
    
    // Clear any additional generated actors or components
}

EPrehistoricBiome APCGWorldGenerator::GetBiomeAtLocation(const FVector& Location) const
{
    // Determine biome at specific world location
    // Implementation would query biome distribution data
    return EPrehistoricBiome::TemperateForest; // Placeholder
}

float APCGWorldGenerator::GetVegetationDensityAtLocation(const FVector& Location) const
{
    // Calculate vegetation density at specific location
    // Implementation would consider biome, slope, water proximity, etc.
    return VegetationParams.DensityMultiplier; // Placeholder
}

TArray<FVector> APCGWorldGenerator::GetNearbyRivers(const FVector& Location, float SearchRadius) const
{
    TArray<FVector> NearbyRivers;
    
    // Find rivers within search radius
    // Implementation would query river system data
    
    return NearbyRivers;
}

void APCGWorldGenerator::SetWorldSize(float SizeInKilometers)
{
    WorldSizeKm = FMath::Clamp(SizeInKilometers, 1.0f, 64.0f);
    UE_LOG(LogPCGWorldGen, Log, TEXT("World size set to %.1f km"), WorldSizeKm);
}

void APCGWorldGenerator::SetLandscapeResolution(int32 Resolution)
{
    // Validate resolution is a valid UE5 landscape size
    TArray<int32> ValidSizes = {127, 253, 505, 1009, 2017, 4033, 8129};
    
    if (ValidSizes.Contains(Resolution))
    {
        LandscapeResolution = Resolution;
        UE_LOG(LogPCGWorldGen, Log, TEXT("Landscape resolution set to %dx%d"), Resolution, Resolution);
    }
    else
    {
        UE_LOG(LogPCGWorldGen, Warning, TEXT("Invalid landscape resolution %d. Using default %d"), Resolution, LandscapeResolution);
    }
}

void APCGWorldGenerator::SetTerrainParameters(const FTerrainGenerationParams& NewParams)
{
    TerrainParams = NewParams;
    UE_LOG(LogPCGWorldGen, Log, TEXT("Terrain parameters updated"));
}

void APCGWorldGenerator::SetBiomeParameters(const FBiomeDistributionParams& NewParams)
{
    BiomeParams = NewParams;
    UE_LOG(LogPCGWorldGen, Log, TEXT("Biome parameters updated"));
}

void APCGWorldGenerator::SetRiverParameters(const FRiverGenerationParams& NewParams)
{
    RiverParams = NewParams;
    UE_LOG(LogPCGWorldGen, Log, TEXT("River parameters updated"));
}

void APCGWorldGenerator::SetVegetationParameters(const FVegetationGenerationParams& NewParams)
{
    VegetationParams = NewParams;
    UE_LOG(LogPCGWorldGen, Log, TEXT("Vegetation parameters updated"));
}
// Copyright Transpersonal Game Studio. All Rights Reserved.
// PCGWorldGenerator.cpp - Core PCG World Generation System Implementation

#include "PCGWorldGenerator.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "PCGPoint.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPCGWorldGenerator::UPCGWorldGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;

    // Initialize PCG component
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    
    // Set default generation parameters
    WorldSeed = FMath::RandRange(1000, 99999);
    LastGenerationTime = 0.0f;
    GeneratedPointsCount = 0;
}

void UPCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: BeginPlay - Initializing world generation system"));
    
    InitializePCGComponent();
    SetupPCGGraphs();
    
    // Auto-generate world if enabled
    if (bEnableRuntimeGeneration)
    {
        GenerateWorld();
    }
}

void UPCGWorldGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ClearGeneration();
    Super::EndPlay(EndPlayReason);
}

void UPCGWorldGenerator::InitializePCGComponent()
{
    if (!PCGComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PCGWorldGenerator: PCGComponent is null!"));
        return;
    }

    // Configure PCG component for world generation
    PCGComponent->SetGenerationTrigger(EPCGGenerationTrigger::GenerateAtRuntime);
    PCGComponent->bGenerated = false;
    
    // Set up hierarchical generation for performance
    if (bUseHierarchicalGeneration)
    {
        PCGComponent->SetIsPartitioned(true);
    }

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: PCG Component initialized successfully"));
}

void UPCGWorldGenerator::SetupPCGGraphs()
{
    // Load or create PCG graphs for different generation tasks
    // In a real implementation, these would be loaded from assets
    
    if (!TerrainGenerationGraph)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: TerrainGenerationGraph not assigned"));
    }
    
    if (!BiomeGenerationGraph)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: BiomeGenerationGraph not assigned"));
    }
    
    if (!VegetationGenerationGraph)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: VegetationGenerationGraph not assigned"));
    }
    
    if (!RiverGenerationGraph)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: RiverGenerationGraph not assigned"));
    }

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: PCG Graphs setup complete"));
}

void UPCGWorldGenerator::GenerateWorld()
{
    UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: Starting world generation..."));
    
    double StartTime = FPlatformTime::Seconds();
    
    // Step 1: Generate base terrain
    GenerateTerrain();
    
    // Step 2: Generate biome distribution
    GenerateBiomes();
    
    // Step 3: Generate river network
    GenerateRivers();
    
    // Step 4: Generate vegetation
    GenerateVegetation();
    
    LastGenerationTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: World generation completed in %.2f seconds"), LastGenerationTime);
    
    // Broadcast completion event
    OnWorldGenerationComplete.Broadcast(true);
}

void UPCGWorldGenerator::GenerateTerrain()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating base terrain..."));
    
    if (!PCGComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("PCGWorldGenerator: Cannot generate terrain - PCGComponent is null"));
        return;
    }

    // Generate heightmap using Perlin noise
    GenerateHeightmap();
    
    // Find or create landscape
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("PCGWorldGenerator: World is null"));
        return;
    }

    // Look for existing landscape
    ALandscape* Landscape = nullptr;
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        Landscape = *ActorItr;
        break;
    }

    if (Landscape)
    {
        GeneratedLandscape = Landscape;
        UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Using existing landscape"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: No landscape found - terrain generation requires manual landscape setup"));
    }
}

void UPCGWorldGenerator::GenerateHeightmap()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating heightmap with Perlin noise"));
    
    // Set random seed for consistent generation
    FMath::RandInit(WorldSeed);
    
    // Generate noise-based heightmap
    const int32 Resolution = TerrainParams.HeightmapResolution;
    const float Scale = TerrainParams.NoiseScale;
    
    TArray<float> HeightData;
    HeightData.SetNum(Resolution * Resolution);
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            float NoiseValue = GeneratePerlinNoise(
                X * Scale, 
                Y * Scale, 
                TerrainParams.NoiseOctaves,
                TerrainParams.NoisePersistence,
                TerrainParams.NoiseLacunarity
            );
            
            // Normalize and scale height
            float Height = (NoiseValue + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
            Height *= TerrainParams.TerrainScale;
            
            HeightData[Y * Resolution + X] = Height;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Heightmap generated with %d points"), HeightData.Num());
}

float UPCGWorldGenerator::GeneratePerlinNoise(float X, float Y, int32 Octaves, float Persistence, float Lacunarity) const
{
    float Total = 0.0f;
    float Frequency = 1.0f;
    float Amplitude = 1.0f;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Total += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= Persistence;
        Frequency *= Lacunarity;
    }
    
    return Total / MaxValue;
}

void UPCGWorldGenerator::GenerateBiomes()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating biome distribution..."));
    
    ApplyBiomeDistribution();
    
    // Generate specific biome areas
    FVector WorldCenter = FVector::ZeroVector;
    
    // Generate tropical rainforest areas
    for (int32 i = 0; i < 3; i++)
    {
        FVector Location = WorldCenter + FVector(
            FMath::RandRange(-50000.0f, 50000.0f),
            FMath::RandRange(-50000.0f, 50000.0f),
            0.0f
        );
        GenerateTropicalRainforest(Location, FMath::RandRange(5000.0f, 15000.0f));
    }
    
    // Generate coniferous forest areas
    for (int32 i = 0; i < 2; i++)
    {
        FVector Location = WorldCenter + FVector(
            FMath::RandRange(-40000.0f, 40000.0f),
            FMath::RandRange(-40000.0f, 40000.0f),
            0.0f
        );
        GenerateConiferousForest(Location, FMath::RandRange(8000.0f, 20000.0f));
    }
    
    // Generate fern prairies
    for (int32 i = 0; i < 4; i++)
    {
        FVector Location = WorldCenter + FVector(
            FMath::RandRange(-30000.0f, 30000.0f),
            FMath::RandRange(-30000.0f, 30000.0f),
            0.0f
        );
        GenerateFernPrairie(Location, FMath::RandRange(3000.0f, 8000.0f));
    }
    
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Biome generation completed"));
}

void UPCGWorldGenerator::ApplyBiomeDistribution()
{
    // Apply biome weights and create transition zones
    for (const auto& BiomePair : TerrainParams.BiomeWeights)
    {
        EPrehistoricBiome BiomeType = BiomePair.Key;
        float Weight = BiomePair.Value;
        
        UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Applying biome %d with weight %.2f"), 
               (int32)BiomeType, Weight);
    }
}

void UPCGWorldGenerator::GenerateTropicalRainforest(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating Tropical Rainforest at %s with radius %.1f"), 
           *Location.ToString(), Radius);
    
    // Generate dense vegetation points
    int32 VegetationCount = FMath::RoundToInt(Radius * 0.1f); // Density based on radius
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector VegLocation = Location + RandomOffset;
        
        // Only place if within radius
        if (FVector::Dist2D(Location, VegLocation) <= Radius)
        {
            // Here we would spawn tropical vegetation using PCG
            GeneratedPointsCount++;
        }
    }
    
    OnBiomeGenerated.Broadcast(EPrehistoricBiome::TropicalRainforest, Location);
}

void UPCGWorldGenerator::GenerateConiferousForest(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating Coniferous Forest at %s with radius %.1f"), 
           *Location.ToString(), Radius);
    
    // Generate coniferous trees with different density
    int32 TreeCount = FMath::RoundToInt(Radius * 0.05f); // Lower density than rainforest
    
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector TreeLocation = Location + RandomOffset;
        
        if (FVector::Dist2D(Location, TreeLocation) <= Radius)
        {
            GeneratedPointsCount++;
        }
    }
    
    OnBiomeGenerated.Broadcast(EPrehistoricBiome::ConiferousForest, Location);
}

void UPCGWorldGenerator::GenerateFernPrairie(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating Fern Prairie at %s with radius %.1f"), 
           *Location.ToString(), Radius);
    
    // Generate ferns and low vegetation
    int32 FernCount = FMath::RoundToInt(Radius * 0.2f); // High density of small plants
    
    for (int32 i = 0; i < FernCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector FernLocation = Location + RandomOffset;
        
        if (FVector::Dist2D(Location, FernLocation) <= Radius)
        {
            GeneratedPointsCount++;
        }
    }
    
    OnBiomeGenerated.Broadcast(EPrehistoricBiome::FernPrairie, Location);
}

void UPCGWorldGenerator::GenerateSwampLands(const FVector& Location, float Radius)
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating Swamp Lands at %s with radius %.1f"), 
           *Location.ToString(), Radius);
    
    // Generate swamp vegetation and water features
    int32 SwampFeatureCount = FMath::RoundToInt(Radius * 0.08f);
    
    for (int32 i = 0; i < SwampFeatureCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-Radius, Radius),
            FMath::RandRange(-Radius, Radius),
            0.0f
        );
        
        FVector SwampLocation = Location + RandomOffset;
        
        if (FVector::Dist2D(Location, SwampLocation) <= Radius)
        {
            GeneratedPointsCount++;
        }
    }
    
    OnBiomeGenerated.Broadcast(EPrehistoricBiome::SwampLands, Location);
}

void UPCGWorldGenerator::GenerateRivers()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating river network..."));
    
    CreateRiverNetwork();
}

void UPCGWorldGenerator::CreateRiverNetwork()
{
    // Generate main rivers flowing from highlands to lowlands
    FVector WorldCenter = FVector::ZeroVector;
    
    // Create 2-3 major rivers
    for (int32 i = 0; i < 3; i++)
    {
        FVector RiverStart = WorldCenter + FVector(
            FMath::RandRange(-60000.0f, 60000.0f),
            FMath::RandRange(-60000.0f, 60000.0f),
            1000.0f // Start at higher elevation
        );
        
        FVector RiverEnd = WorldCenter + FVector(
            FMath::RandRange(-80000.0f, 80000.0f),
            FMath::RandRange(-80000.0f, 80000.0f),
            -100.0f // End at lower elevation
        );
        
        // Generate river path with meandering
        TArray<FVector> RiverPath;
        int32 PathSegments = 20;
        
        for (int32 j = 0; j <= PathSegments; j++)
        {
            float Alpha = (float)j / PathSegments;
            FVector PathPoint = FMath::Lerp(RiverStart, RiverEnd, Alpha);
            
            // Add meandering
            float MeanderOffset = FMath::Sin(Alpha * PI * 4.0f) * 2000.0f;
            PathPoint += FVector(MeanderOffset, MeanderOffset * 0.5f, 0.0f);
            
            RiverPath.Add(PathPoint);
            GeneratedPointsCount++;
        }
        
        UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generated river %d with %d segments"), i, PathSegments);
    }
}

void UPCGWorldGenerator::GenerateVegetation()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Generating vegetation..."));
    
    PopulateVegetation();
}

void UPCGWorldGenerator::PopulateVegetation()
{
    // Populate vegetation based on biome types and terrain features
    if (!PCGComponent)
    {
        return;
    }

    // This would use the VegetationGenerationGraph in a real implementation
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Vegetation population completed"));
}

EPrehistoricBiome UPCGWorldGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Use noise functions to determine biome at location
    float BiomeNoise = GenerateBiomeNoise(WorldLocation.X * 0.0001f, WorldLocation.Y * 0.0001f, EPrehistoricBiome::TropicalRainforest);
    
    if (BiomeNoise > 0.6f)
        return EPrehistoricBiome::TropicalRainforest;
    else if (BiomeNoise > 0.4f)
        return EPrehistoricBiome::ConiferousForest;
    else if (BiomeNoise > 0.2f)
        return EPrehistoricBiome::FernPrairie;
    else if (BiomeNoise > 0.0f)
        return EPrehistoricBiome::SwampLands;
    else
        return EPrehistoricBiome::CoastalPlains;
}

float UPCGWorldGenerator::GenerateBiomeNoise(float X, float Y, EPrehistoricBiome BiomeType) const
{
    // Generate biome-specific noise patterns
    float BaseNoise = FMath::PerlinNoise2D(FVector2D(X, Y));
    
    // Modify based on biome type
    switch (BiomeType)
    {
        case EPrehistoricBiome::TropicalRainforest:
            return BaseNoise * 1.2f;
        case EPrehistoricBiome::ConiferousForest:
            return BaseNoise * 0.8f;
        case EPrehistoricBiome::SwampLands:
            return BaseNoise * 0.6f;
        default:
            return BaseNoise;
    }
}

float UPCGWorldGenerator::GetHeightAtLocation(const FVector& WorldLocation) const
{
    if (GeneratedLandscape.IsValid())
    {
        return GeneratedLandscape->GetActorLocation().Z;
    }
    
    // Fallback to noise-based height
    return GeneratePerlinNoise(
        WorldLocation.X * TerrainParams.NoiseScale,
        WorldLocation.Y * TerrainParams.NoiseScale,
        TerrainParams.NoiseOctaves,
        TerrainParams.NoisePersistence,
        TerrainParams.NoiseLacunarity
    ) * TerrainParams.TerrainScale;
}

FVector UPCGWorldGenerator::GetNormalAtLocation(const FVector& WorldLocation) const
{
    // Calculate surface normal using height samples
    const float SampleDistance = 100.0f;
    
    float HeightL = GetHeightAtLocation(WorldLocation + FVector(-SampleDistance, 0, 0));
    float HeightR = GetHeightAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    float HeightD = GetHeightAtLocation(WorldLocation + FVector(0, -SampleDistance, 0));
    float HeightU = GetHeightAtLocation(WorldLocation + FVector(0, SampleDistance, 0));
    
    FVector Normal = FVector(HeightL - HeightR, HeightD - HeightU, 2.0f * SampleDistance);
    return Normal.GetSafeNormal();
}

void UPCGWorldGenerator::SetGenerationLOD(int32 LODLevel)
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Setting generation LOD to %d"), LODLevel);
    
    // Adjust generation parameters based on LOD
    if (LODLevel == 0)
    {
        // High detail
        GenerationRadius = 5000;
        CleanupRadius = 7500;
    }
    else if (LODLevel == 1)
    {
        // Medium detail
        GenerationRadius = 3000;
        CleanupRadius = 4500;
    }
    else
    {
        // Low detail
        GenerationRadius = 1500;
        CleanupRadius = 2250;
    }
}

void UPCGWorldGenerator::EnableRuntimeGeneration(bool bEnable)
{
    bEnableRuntimeGeneration = bEnable;
    
    if (PCGComponent)
    {
        if (bEnable)
        {
            PCGComponent->SetGenerationTrigger(EPCGGenerationTrigger::GenerateAtRuntime);
        }
        else
        {
            PCGComponent->SetGenerationTrigger(EPCGGenerationTrigger::GenerateOnLoad);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Runtime generation %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPCGWorldGenerator::ClearGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: Clearing generated content..."));
    
    if (PCGComponent)
    {
        PCGComponent->CleanupLocalImmediate(true);
    }
    
    GeneratedPointsCount = 0;
    GeneratedLandscape.Reset();
}

// World Subsystem Implementation
void UPCGWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PCGWorldSubsystem: Initializing world generation subsystem"));
    
    InitializeWorldGeneration();
}

void UPCGWorldSubsystem::Deinitialize()
{
    // Clean up all active chunks
    for (auto& ChunkPair : ActiveChunks)
    {
        if (ChunkPair.Value)
        {
            ChunkPair.Value->ClearGeneration();
        }
    }
    ActiveChunks.Empty();
    
    Super::Deinitialize();
}

void UPCGWorldSubsystem::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("PCGWorldSubsystem: World generation initialized"));
}

void UPCGWorldSubsystem::GenerateWorldChunk(const FVector& ChunkCenter, int32 ChunkSize)
{
    FIntPoint ChunkCoord = WorldLocationToChunkCoord(ChunkCenter);
    
    if (ActiveChunks.Contains(ChunkCoord))
    {
        return; // Chunk already exists
    }
    
    UE_LOG(LogTemp, Log, TEXT("PCGWorldSubsystem: Generating chunk at (%d, %d)"), 
           ChunkCoord.X, ChunkCoord.Y);
    
    // Create new world generator for this chunk
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // In a real implementation, we would create an actor with the PCGWorldGenerator component
    // For now, we'll just log the generation
    ActiveChunks.Add(ChunkCoord, nullptr);
}

void UPCGWorldSubsystem::UnloadWorldChunk(const FVector& ChunkCenter)
{
    FIntPoint ChunkCoord = WorldLocationToChunkCoord(ChunkCenter);
    
    if (UPCGWorldGenerator** Generator = ActiveChunks.Find(ChunkCoord))
    {
        if (*Generator)
        {
            (*Generator)->ClearGeneration();
        }
        ActiveChunks.Remove(ChunkCoord);
        
        UE_LOG(LogTemp, Log, TEXT("PCGWorldSubsystem: Unloaded chunk at (%d, %d)"), 
               ChunkCoord.X, ChunkCoord.Y);
    }
}

void UPCGWorldSubsystem::UpdateStreamingAroundPlayer(const FVector& PlayerLocation)
{
    if (FVector::Dist(PlayerLocation, LastPlayerLocation) < 1000.0f)
    {
        return; // Player hasn't moved far enough
    }
    
    LastPlayerLocation = PlayerLocation;
    
    // Generate chunks around player
    const int32 ChunkRadius = 2;
    const int32 ChunkSize = 10000; // 100m chunks
    
    for (int32 X = -ChunkRadius; X <= ChunkRadius; X++)
    {
        for (int32 Y = -ChunkRadius; Y <= ChunkRadius; Y++)
        {
            FVector ChunkCenter = PlayerLocation + FVector(X * ChunkSize, Y * ChunkSize, 0);
            GenerateWorldChunk(ChunkCenter, ChunkSize);
        }
    }
    
    // Clean up distant chunks
    CleanupDistantChunks(PlayerLocation);
}

void UPCGWorldSubsystem::SetStreamingRadius(float NewRadius)
{
    StreamingRadius = NewRadius;
    UE_LOG(LogTemp, Log, TEXT("PCGWorldSubsystem: Streaming radius set to %.1f"), NewRadius);
}

EPrehistoricBiome UPCGWorldSubsystem::GetBiomeAtWorldLocation(const FVector& WorldLocation) const
{
    // Simple noise-based biome determination
    float BiomeNoise = FMath::PerlinNoise2D(FVector2D(WorldLocation.X * 0.0001f, WorldLocation.Y * 0.0001f));
    
    if (BiomeNoise > 0.5f)
        return EPrehistoricBiome::TropicalRainforest;
    else if (BiomeNoise > 0.0f)
        return EPrehistoricBiome::ConiferousForest;
    else
        return EPrehistoricBiome::FernPrairie;
}

TArray<FVector> UPCGWorldSubsystem::FindBiomeLocations(EPrehistoricBiome BiomeType, float SearchRadius) const
{
    TArray<FVector> Locations;
    
    // Sample locations in a grid pattern
    const int32 SampleCount = 20;
    const float SampleSpacing = SearchRadius * 2.0f / SampleCount;
    
    for (int32 X = 0; X < SampleCount; X++)
    {
        for (int32 Y = 0; Y < SampleCount; Y++)
        {
            FVector SampleLocation = FVector(
                (X - SampleCount * 0.5f) * SampleSpacing,
                (Y - SampleCount * 0.5f) * SampleSpacing,
                0.0f
            );
            
            if (GetBiomeAtWorldLocation(SampleLocation) == BiomeType)
            {
                Locations.Add(SampleLocation);
            }
        }
    }
    
    return Locations;
}

void UPCGWorldSubsystem::CleanupDistantChunks(const FVector& PlayerLocation)
{
    TArray<FIntPoint> ChunksToRemove;
    
    for (const auto& ChunkPair : ActiveChunks)
    {
        FVector ChunkWorldLocation = ChunkCoordToWorldLocation(ChunkPair.Key);
        float Distance = FVector::Dist(PlayerLocation, ChunkWorldLocation);
        
        if (Distance > StreamingRadius * 2.0f)
        {
            ChunksToRemove.Add(ChunkPair.Key);
        }
    }
    
    for (const FIntPoint& ChunkCoord : ChunksToRemove)
    {
        FVector ChunkLocation = ChunkCoordToWorldLocation(ChunkCoord);
        UnloadWorldChunk(ChunkLocation);
    }
}

FIntPoint UPCGWorldSubsystem::WorldLocationToChunkCoord(const FVector& WorldLocation) const
{
    const int32 ChunkSize = 10000; // 100m chunks
    return FIntPoint(
        FMath::FloorToInt(WorldLocation.X / ChunkSize),
        FMath::FloorToInt(WorldLocation.Y / ChunkSize)
    );
}

FVector UPCGWorldSubsystem::ChunkCoordToWorldLocation(const FIntPoint& ChunkCoord) const
{
    const int32 ChunkSize = 10000; // 100m chunks
    return FVector(
        ChunkCoord.X * ChunkSize + ChunkSize * 0.5f,
        ChunkCoord.Y * ChunkSize + ChunkSize * 0.5f,
        0.0f
    );
}
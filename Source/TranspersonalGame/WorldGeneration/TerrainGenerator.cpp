#include "TerrainGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeInfo.h"
#include "LandscapeEditorModule.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "DrawDebugHelpers.h"

ATerrainGenerator::ATerrainGenerator()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // Create PCG Component for procedural generation
    PCGComponent = CreateDefaultSubobject<UPCGComponent>(TEXT("PCGComponent"));
    RootComponent = PCGComponent;

    // Default biome configurations for prehistoric world
    SetupBiomeConfigurations();

    // Set default world parameters
    WorldSizeKm = 10;
    ChunkSizeMeters = 2048;
    HeightmapScale = 100.0f;
    RandomSeed = 12345;
    BiomeTransitionDistance = 500.0f;
    bEnableBiomeBlending = true;
    bUseWorldPartition = true;
    PartitionCellSize = 12800;
}

void ATerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: BeginPlay - Initializing world generation system"));
    
    InitializePCGComponent();
    
    // Initialize random seed
    FMath::RandInit(RandomSeed);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Initialization complete"));
}

void ATerrainGenerator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up any ongoing generation
    bIsGenerating = false;
    
    Super::EndPlay(EndPlayReason);
}

#if WITH_EDITOR
void ATerrainGenerator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (PropertyChangedEvent.Property)
    {
        const FName PropertyName = PropertyChangedEvent.Property->GetFName();
        
        if (PropertyName == GET_MEMBER_NAME_CHECKED(ATerrainGenerator, RandomSeed))
        {
            FMath::RandInit(RandomSeed);
            UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Random seed updated to %d"), RandomSeed);
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(ATerrainGenerator, BiomeConfigurations))
        {
            UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Biome configurations updated"));
        }
    }
}
#endif

void ATerrainGenerator::InitializePCGComponent()
{
    if (!PCGComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: PCGComponent is null"));
        return;
    }

    // Configure PCG component for world generation
    PCGComponent->SetGenerationTrigger(EPCGComponentGenerationTrigger::GenerateAtRuntime);
    
    if (bUseWorldPartition)
    {
        PCGComponent->SetIsPartitioned(true);
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: PCG Component initialized"));
}

void ATerrainGenerator::SetupBiomeConfigurations()
{
    BiomeConfigurations.Empty();

    // Dense Forest - Primary biome for dinosaur habitat
    FBiomeSettings DenseForest;
    DenseForest.BiomeType = EBiomeType::DenseForest;
    DenseForest.Density = 1.0f;
    DenseForest.HeightVariation = 300.0f;
    DenseForest.Temperature = 28.0f;
    DenseForest.Humidity = 0.9f;
    BiomeConfigurations.Add(DenseForest);

    // Open Plains - Grazing areas for herbivores
    FBiomeSettings OpenPlains;
    OpenPlains.BiomeType = EBiomeType::OpenPlains;
    OpenPlains.Density = 0.3f;
    OpenPlains.HeightVariation = 100.0f;
    OpenPlains.Temperature = 25.0f;
    OpenPlains.Humidity = 0.5f;
    BiomeConfigurations.Add(OpenPlains);

    // Rocky Hills - Elevated terrain with sparse vegetation
    FBiomeSettings RockyHills;
    RockyHills.BiomeType = EBiomeType::RockyHills;
    RockyHills.Density = 0.4f;
    RockyHills.HeightVariation = 800.0f;
    RockyHills.Temperature = 20.0f;
    RockyHills.Humidity = 0.4f;
    BiomeConfigurations.Add(RockyHills);

    // River Valley - Water sources and lush vegetation
    FBiomeSettings RiverValley;
    RiverValley.BiomeType = EBiomeType::RiverValley;
    RiverValley.Density = 0.8f;
    RiverValley.HeightVariation = 150.0f;
    RiverValley.Temperature = 26.0f;
    RiverValley.Humidity = 1.0f;
    BiomeConfigurations.Add(RiverValley);

    // Swamp Lands - Dangerous wetland areas
    FBiomeSettings SwampLands;
    SwampLands.BiomeType = EBiomeType::SwampLands;
    SwampLands.Density = 0.9f;
    SwampLands.HeightVariation = 50.0f;
    SwampLands.Temperature = 30.0f;
    SwampLands.Humidity = 1.0f;
    BiomeConfigurations.Add(SwampLands);

    // Volcanic Region - Hazardous but resource-rich
    FBiomeSettings VolcanicRegion;
    VolcanicRegion.BiomeType = EBiomeType::VolcanicRegion;
    VolcanicRegion.Density = 0.2f;
    VolcanicRegion.HeightVariation = 1200.0f;
    VolcanicRegion.Temperature = 35.0f;
    VolcanicRegion.Humidity = 0.3f;
    BiomeConfigurations.Add(VolcanicRegion);

    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: %d biome configurations initialized"), BiomeConfigurations.Num());
}

void ATerrainGenerator::GenerateWorld()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: World generation already in progress"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Starting world generation - Size: %dx%d km"), WorldSizeKm, WorldSizeKm);
    
    bIsGenerating = true;
    LastGenerationTime = FPlatformTime::Seconds();
    
    // Clear existing chunks
    GeneratedChunks.Empty();
    ChunkIndexMap.Empty();

    // Calculate number of chunks needed
    const int32 ChunksPerSide = (WorldSizeKm * 1000) / ChunkSizeMeters;
    const int32 TotalChunks = ChunksPerSide * ChunksPerSide;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Generating %d chunks (%dx%d)"), TotalChunks, ChunksPerSide, ChunksPerSide);

    // Generate chunks in a grid pattern
    for (int32 X = 0; X < ChunksPerSide; X++)
    {
        for (int32 Y = 0; Y < ChunksPerSide; Y++)
        {
            const FVector2D ChunkCoords(X, Y);
            GenerateChunkAtLocation(ChunkCoords);
        }
    }

    bIsGenerating = false;
    const float GenerationTime = FPlatformTime::Seconds() - LastGenerationTime;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: World generation complete - Time: %.2f seconds, Chunks: %d"), 
           GenerationTime, GeneratedChunks.Num());
}

void ATerrainGenerator::GenerateChunkAtLocation(const FVector2D& ChunkCoords)
{
    if (IsChunkGenerated(ChunkCoords))
    {
        UE_LOG(LogTemp, Warning, TEXT("TerrainGenerator: Chunk (%d,%d) already generated"), 
               (int32)ChunkCoords.X, (int32)ChunkCoords.Y);
        return;
    }

    // Determine biome for this chunk
    const EBiomeType BiomeType = CalculateBiomeForLocation(ChunkCoords);
    const FBiomeSettings* BiomeSettings = BiomeConfigurations.FindByPredicate([BiomeType](const FBiomeSettings& Settings)
    {
        return Settings.BiomeType == BiomeType;
    });

    if (!BiomeSettings)
    {
        UE_LOG(LogTemp, Error, TEXT("TerrainGenerator: No biome settings found for biome type"));
        return;
    }

    // Create terrain chunk
    FTerrainChunk NewChunk;
    NewChunk.ChunkCoordinates = ChunkCoords;
    NewChunk.ChunkSize = ChunkSizeMeters;
    NewChunk.BiomeSettings = *BiomeSettings;
    NewChunk.bIsGenerated = false;

    // Generate the actual terrain
    GenerateTerrainChunk(ChunkCoords, *BiomeSettings);
    
    // Apply biome-specific features
    ApplyBiomeToChunk(NewChunk);
    
    NewChunk.bIsGenerated = true;
    
    // Store chunk data
    const int32 ChunkIndex = GeneratedChunks.Add(NewChunk);
    ChunkIndexMap.Add(ChunkCoords, ChunkIndex);

    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Generated chunk (%d,%d) with biome: %d"), 
           (int32)ChunkCoords.X, (int32)ChunkCoords.Y, (int32)BiomeType);
}

void ATerrainGenerator::GenerateTerrainChunk(const FVector2D& ChunkCoords, const FBiomeSettings& BiomeSettings)
{
    // Calculate world position for this chunk
    const FVector ChunkWorldLocation = ChunkCoordsToWorldLocation(ChunkCoords);
    
    // Generate heightmap data using noise functions
    const int32 HeightmapResolution = 513; // Standard UE5 landscape resolution
    TArray<uint16> HeightData;
    HeightData.SetNum(HeightmapResolution * HeightmapResolution);

    // Generate height values based on biome characteristics
    for (int32 Y = 0; Y < HeightmapResolution; Y++)
    {
        for (int32 X = 0; X < HeightmapResolution; X++)
        {
            const FVector2D LocalCoords(X, Y);
            const FVector2D WorldCoords = ChunkCoords + (LocalCoords / HeightmapResolution);
            
            // Calculate base height using multiple noise octaves
            float Height = 0.0f;
            
            // Base terrain height
            Height += CalculateNoiseAtLocation(WorldCoords, 0.001f, 4) * BiomeSettings.HeightVariation;
            
            // Add biome-specific height modulation
            switch (BiomeSettings.BiomeType)
            {
                case EBiomeType::RockyHills:
                    Height += CalculateNoiseAtLocation(WorldCoords, 0.005f, 2) * 200.0f;
                    break;
                case EBiomeType::RiverValley:
                    Height -= FMath::Abs(CalculateNoiseAtLocation(WorldCoords, 0.01f, 1)) * 100.0f;
                    break;
                case EBiomeType::VolcanicRegion:
                    Height += FMath::Pow(CalculateNoiseAtLocation(WorldCoords, 0.002f, 3), 2.0f) * 400.0f;
                    break;
            }
            
            // Convert to heightmap format (0-65535)
            const uint16 HeightValue = FMath::Clamp(
                (int32)((Height + 256.0f) * 128.0f), 
                0, 
                65535
            );
            
            HeightData[Y * HeightmapResolution + X] = HeightValue;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Generated heightmap for chunk (%d,%d)"), 
           (int32)ChunkCoords.X, (int32)ChunkCoords.Y);
}

void ATerrainGenerator::ApplyBiomeToChunk(FTerrainChunk& Chunk)
{
    // This would typically involve:
    // 1. Setting up PCG graphs for vegetation placement
    // 2. Configuring material parameters
    // 3. Placing biome-specific assets
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Applied biome %d to chunk (%d,%d)"), 
           (int32)Chunk.BiomeSettings.BiomeType,
           (int32)Chunk.ChunkCoordinates.X, 
           (int32)Chunk.ChunkCoordinates.Y);
}

EBiomeType ATerrainGenerator::CalculateBiomeForLocation(const FVector2D& Location) const
{
    // Calculate environmental factors
    const float Temperature = CalculateTemperature(Location);
    const float Humidity = CalculateHumidity(Location);
    const float Elevation = CalculateNoiseAtLocation(Location, 0.001f, 4);

    // Biome selection based on temperature, humidity, and elevation
    if (Elevation > 0.6f)
    {
        return EBiomeType::RockyHills;
    }
    else if (Elevation > 0.8f && Temperature > 30.0f)
    {
        return EBiomeType::VolcanicRegion;
    }
    else if (Humidity > 0.8f && Elevation < 0.2f)
    {
        if (Temperature > 28.0f)
        {
            return EBiomeType::SwampLands;
        }
        else
        {
            return EBiomeType::RiverValley;
        }
    }
    else if (Humidity > 0.6f && Temperature > 25.0f)
    {
        return EBiomeType::DenseForest;
    }
    else
    {
        return EBiomeType::OpenPlains;
    }
}

float ATerrainGenerator::CalculateNoiseAtLocation(const FVector2D& Location, float Scale, int32 Octaves) const
{
    float NoiseValue = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;

    for (int32 i = 0; i < Octaves; i++)
    {
        NoiseValue += FMath::PerlinNoise2D(Location * Frequency) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }

    return NoiseValue / MaxValue;
}

float ATerrainGenerator::CalculateTemperature(const FVector2D& Location) const
{
    // Base temperature with latitude variation
    const float LatitudeEffect = FMath::Cos(Location.Y * 0.001f) * 10.0f;
    const float NoiseVariation = CalculateNoiseAtLocation(Location, 0.0005f, 3) * 5.0f;
    
    return 25.0f + LatitudeEffect + NoiseVariation;
}

float ATerrainGenerator::CalculateHumidity(const FVector2D& Location) const
{
    // Humidity based on distance to water sources and elevation
    const float WaterInfluence = CalculateNoiseAtLocation(Location, 0.002f, 2) * 0.3f;
    const float ElevationEffect = -CalculateNoiseAtLocation(Location, 0.001f, 4) * 0.2f;
    
    return FMath::Clamp(0.6f + WaterInfluence + ElevationEffect, 0.0f, 1.0f);
}

void ATerrainGenerator::ClearGeneratedWorld()
{
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: Clearing generated world"));
    
    // Clean up landscape actors
    for (const FTerrainChunk& Chunk : GeneratedChunks)
    {
        if (Chunk.LandscapeActor.IsValid())
        {
            Chunk.LandscapeActor->Destroy();
        }
    }
    
    GeneratedChunks.Empty();
    ChunkIndexMap.Empty();
    bIsGenerating = false;
    
    UE_LOG(LogTemp, Log, TEXT("TerrainGenerator: World cleared"));
}

EBiomeType ATerrainGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    const FVector2D ChunkCoords = WorldLocationToChunkCoords(WorldLocation);
    return CalculateBiomeForLocation(ChunkCoords);
}

FBiomeSettings ATerrainGenerator::GetBiomeSettings(EBiomeType BiomeType) const
{
    const FBiomeSettings* Settings = BiomeConfigurations.FindByPredicate([BiomeType](const FBiomeSettings& Config)
    {
        return Config.BiomeType == BiomeType;
    });
    
    return Settings ? *Settings : FBiomeSettings();
}

float ATerrainGenerator::GetTerrainHeightAtLocation(const FVector& WorldLocation) const
{
    // This would query the actual landscape height
    // For now, return calculated height based on noise
    const FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    const FVector2D ChunkCoords = WorldLocationToChunkCoords(WorldLocation);
    
    const EBiomeType BiomeType = CalculateBiomeForLocation(ChunkCoords);
    const FBiomeSettings Settings = GetBiomeSettings(BiomeType);
    
    return CalculateNoiseAtLocation(Location2D, 0.001f, 4) * Settings.HeightVariation;
}

FVector ATerrainGenerator::GetTerrainNormalAtLocation(const FVector& WorldLocation) const
{
    // Calculate normal using height sampling
    const float SampleDistance = 100.0f;
    
    const float HeightCenter = GetTerrainHeightAtLocation(WorldLocation);
    const float HeightRight = GetTerrainHeightAtLocation(WorldLocation + FVector(SampleDistance, 0, 0));
    const float HeightUp = GetTerrainHeightAtLocation(WorldLocation + FVector(0, SampleDistance, 0));
    
    const FVector TangentX = FVector(SampleDistance, 0, HeightRight - HeightCenter).GetSafeNormal();
    const FVector TangentY = FVector(0, SampleDistance, HeightUp - HeightCenter).GetSafeNormal();
    
    return FVector::CrossProduct(TangentY, TangentX).GetSafeNormal();
}

void ATerrainGenerator::DebugDrawBiomes()
{
    if (!GetWorld())
        return;

    const int32 DebugGridSize = 20;
    const float CellSize = (WorldSizeKm * 1000.0f) / DebugGridSize;
    
    for (int32 X = 0; X < DebugGridSize; X++)
    {
        for (int32 Y = 0; Y < DebugGridSize; Y++)
        {
            const FVector2D GridLocation(X * CellSize, Y * CellSize);
            const EBiomeType BiomeType = CalculateBiomeForLocation(GridLocation);
            
            FColor BiomeColor = FColor::White;
            switch (BiomeType)
            {
                case EBiomeType::DenseForest: BiomeColor = FColor::Green; break;
                case EBiomeType::OpenPlains: BiomeColor = FColor::Yellow; break;
                case EBiomeType::RockyHills: BiomeColor = FColor::Silver; break;
                case EBiomeType::RiverValley: BiomeColor = FColor::Blue; break;
                case EBiomeType::SwampLands: BiomeColor = FColor::Purple; break;
                case EBiomeType::VolcanicRegion: BiomeColor = FColor::Red; break;
            }
            
            const FVector WorldPos(GridLocation.X, GridLocation.Y, 1000.0f);
            DrawDebugBox(GetWorld(), WorldPos, FVector(CellSize * 0.4f, CellSize * 0.4f, 50.0f), 
                        BiomeColor, false, 30.0f, 0, 10.0f);
        }
    }
}

void ATerrainGenerator::DebugDrawChunkBounds()
{
    if (!GetWorld())
        return;

    for (const FTerrainChunk& Chunk : GeneratedChunks)
    {
        const FVector ChunkCenter = ChunkCoordsToWorldLocation(Chunk.ChunkCoordinates);
        const FVector BoxExtent(ChunkSizeMeters * 0.5f, ChunkSizeMeters * 0.5f, 100.0f);
        
        const FColor ChunkColor = Chunk.bIsGenerated ? FColor::Green : FColor::Red;
        
        DrawDebugBox(GetWorld(), ChunkCenter, BoxExtent, ChunkColor, false, 10.0f, 0, 20.0f);
    }
}

FVector2D ATerrainGenerator::WorldLocationToChunkCoords(const FVector& WorldLocation) const
{
    return FVector2D(
        FMath::FloorToInt(WorldLocation.X / ChunkSizeMeters),
        FMath::FloorToInt(WorldLocation.Y / ChunkSizeMeters)
    );
}

FVector ATerrainGenerator::ChunkCoordsToWorldLocation(const FVector2D& ChunkCoords) const
{
    return FVector(
        ChunkCoords.X * ChunkSizeMeters + (ChunkSizeMeters * 0.5f),
        ChunkCoords.Y * ChunkSizeMeters + (ChunkSizeMeters * 0.5f),
        0.0f
    );
}

bool ATerrainGenerator::IsChunkGenerated(const FVector2D& ChunkCoords) const
{
    return ChunkIndexMap.Contains(ChunkCoords);
}

FTerrainChunk* ATerrainGenerator::FindChunk(const FVector2D& ChunkCoords)
{
    const int32* ChunkIndex = ChunkIndexMap.Find(ChunkCoords);
    return ChunkIndex ? &GeneratedChunks[*ChunkIndex] : nullptr;
}
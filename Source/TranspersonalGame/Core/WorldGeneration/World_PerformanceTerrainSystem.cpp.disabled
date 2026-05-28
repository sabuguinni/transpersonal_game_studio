#include "World_PerformanceTerrainSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "ProceduralMeshComponent.h"
#include "Engine/StaticMesh.h"

AWorld_PerformanceTerrainSystem::AWorld_PerformanceTerrainSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms for performance

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create terrain mesh component
    TerrainMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerrainMeshComponent"));
    TerrainMeshComponent->SetupAttachment(RootComponent);
    TerrainMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TerrainMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);

    // Initialize default values
    TerrainLODLevels = 3;
    StreamingDistance = 2000.0f;
    ChunkSize = 512.0f;
    bEnablePerformanceOptimizations = true;
    HeightScale = 100.0f;
    NoiseScale = 0.01f;
    NoiseOctaves = 4;
    NoisePersistence = 0.5f;
    BiomeTransitionDistance = 200.0f;

    // Initialize supported biomes
    SupportedBiomes.Add(World_BiomeType::Forest);
    SupportedBiomes.Add(World_BiomeType::Plains);
    SupportedBiomes.Add(World_BiomeType::Mountains);
    SupportedBiomes.Add(World_BiomeType::Desert);

    // Initialize performance tracking
    CurrentFrameTime = 0.0f;
    ActiveTerrainChunks = 0;
    RenderedVertices = 0;
    LastPerformanceUpdate = 0.0f;
    PerformanceUpdateInterval = 1.0f;
    MaxCachedChunks = 50;
}

void AWorld_PerformanceTerrainSystem::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Log, TEXT("World_PerformanceTerrainSystem: BeginPlay - Initializing high-performance terrain system"));

    // Initialize terrain chunks around spawn location
    FVector SpawnLocation = GetActorLocation();
    
    // Generate initial terrain chunks in a 3x3 grid
    for (int32 X = -1; X <= 1; X++)
    {
        for (int32 Y = -1; Y <= 1; Y++)
        {
            FVector ChunkLocation = SpawnLocation + FVector(X * ChunkSize, Y * ChunkSize, 0);
            GenerateTerrainChunk(ChunkLocation, 0); // Highest LOD for nearby chunks
        }
    }

    // Apply performance optimizations
    if (bEnablePerformanceOptimizations)
    {
        OptimizeTerrainPerformance();
    }

    UE_LOG(LogTemp, Log, TEXT("World_PerformanceTerrainSystem: Initialization complete with %d active chunks"), ActiveTerrainChunks);
}

void AWorld_PerformanceTerrainSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CurrentFrameTime = DeltaTime;

    // Update performance metrics periodically
    if (GetWorld()->GetTimeSeconds() - LastPerformanceUpdate > PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
    }

    // Get player location for LOD updates
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && bEnablePerformanceOptimizations)
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        
        // Update terrain LOD based on player distance
        UpdateTerrainLOD(PlayerLocation);
        
        // Stream terrain chunks based on player movement
        StreamTerrainChunks(PlayerLocation);
        
        // Cull distant terrain for performance
        CullDistantTerrain(PlayerLocation);
    }
}

void AWorld_PerformanceTerrainSystem::GenerateTerrainChunk(FVector ChunkLocation, int32 LODLevel)
{
    // Clamp LOD level
    LODLevel = FMath::Clamp(LODLevel, 0, TerrainLODLevels - 1);

    // Calculate chunk grid position
    FIntPoint ChunkCoord = FIntPoint(
        FMath::FloorToInt(ChunkLocation.X / ChunkSize),
        FMath::FloorToInt(ChunkLocation.Y / ChunkSize)
    );

    // Check if chunk already exists
    if (TerrainChunks.Contains(ChunkCoord))
    {
        return; // Chunk already generated
    }

    // Generate height map for this chunk
    TArray<float> HeightData;
    int32 ChunkResolution = 64 >> LODLevel; // Reduce resolution for higher LOD levels
    ChunkResolution = FMath::Max(ChunkResolution, 8); // Minimum resolution

    GenerateHeightMap(ChunkCoord.X, ChunkCoord.Y, HeightData);

    // Create terrain mesh
    CreateTerrainMesh(HeightData, ChunkResolution, LODLevel);

    // Store chunk information
    TerrainChunkLocations.Add(ChunkLocation);
    ChunkLODLevels.Add(LODLevel);
    ChunkVisibility.Add(true);
    ActiveTerrainChunks++;

    UE_LOG(LogTemp, Log, TEXT("Generated terrain chunk at (%f, %f) with LOD %d"), 
           ChunkLocation.X, ChunkLocation.Y, LODLevel);
}

void AWorld_PerformanceTerrainSystem::UpdateTerrainLOD(FVector PlayerLocation)
{
    for (int32 i = 0; i < TerrainChunkLocations.Num(); i++)
    {
        float Distance = FVector::Dist(PlayerLocation, TerrainChunkLocations[i]);
        
        // Determine appropriate LOD level based on distance
        int32 NewLODLevel = 0;
        if (Distance > StreamingDistance * 0.3f)
        {
            NewLODLevel = 1;
        }
        if (Distance > StreamingDistance * 0.6f)
        {
            NewLODLevel = 2;
        }
        if (Distance > StreamingDistance)
        {
            NewLODLevel = TerrainLODLevels - 1;
        }

        // Update LOD if changed
        if (NewLODLevel != ChunkLODLevels[i])
        {
            UpdateChunkLOD(i, Distance);
            ChunkLODLevels[i] = NewLODLevel;
        }
    }
}

void AWorld_PerformanceTerrainSystem::StreamTerrainChunks(FVector PlayerLocation)
{
    // Calculate which chunks should be loaded based on player position
    FIntPoint PlayerChunk = FIntPoint(
        FMath::FloorToInt(PlayerLocation.X / ChunkSize),
        FMath::FloorToInt(PlayerLocation.Y / ChunkSize)
    );

    int32 StreamingRadius = FMath::CeilToInt(StreamingDistance / ChunkSize);

    // Generate new chunks around player
    for (int32 X = PlayerChunk.X - StreamingRadius; X <= PlayerChunk.X + StreamingRadius; X++)
    {
        for (int32 Y = PlayerChunk.Y - StreamingRadius; Y <= PlayerChunk.Y + StreamingRadius; Y++)
        {
            FIntPoint ChunkCoord = FIntPoint(X, Y);
            
            if (!TerrainChunks.Contains(ChunkCoord))
            {
                FVector ChunkWorldLocation = FVector(X * ChunkSize, Y * ChunkSize, 0);
                float Distance = FVector::Dist(PlayerLocation, ChunkWorldLocation);
                
                if (Distance <= StreamingDistance)
                {
                    int32 LODLevel = Distance > StreamingDistance * 0.5f ? 1 : 0;
                    GenerateTerrainChunk(ChunkWorldLocation, LODLevel);
                }
            }
        }
    }

    // Unload distant chunks to save memory
    TArray<FIntPoint> ChunksToRemove;
    for (auto& ChunkPair : TerrainChunks)
    {
        FVector ChunkWorldLocation = FVector(ChunkPair.Key.X * ChunkSize, ChunkPair.Key.Y * ChunkSize, 0);
        float Distance = FVector::Dist(PlayerLocation, ChunkWorldLocation);
        
        if (Distance > StreamingDistance * 1.5f)
        {
            ChunksToRemove.Add(ChunkPair.Key);
        }
    }

    // Remove distant chunks
    for (const FIntPoint& ChunkCoord : ChunksToRemove)
    {
        if (UStaticMeshComponent* ChunkMesh = TerrainChunks[ChunkCoord])
        {
            ChunkMesh->DestroyComponent();
        }
        TerrainChunks.Remove(ChunkCoord);
        ActiveTerrainChunks--;
    }
}

void AWorld_PerformanceTerrainSystem::OptimizeTerrainPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("World_PerformanceTerrainSystem: Applying performance optimizations"));

    // Enable LOD system
    EnableLODSystem(true);

    // Set appropriate terrain quality based on performance targets
    SetTerrainQuality(2); // Medium quality for 60fps target

    // Optimize vertex count for performance
    OptimizeVertexCount(50000); // Target 50k vertices max

    UE_LOG(LogTemp, Log, TEXT("Performance optimizations applied successfully"));
}

void AWorld_PerformanceTerrainSystem::UpdatePerformanceMetrics()
{
    // Update active chunk count
    ActiveTerrainChunks = TerrainChunks.Num();

    // Calculate rendered vertices (approximate)
    RenderedVertices = ActiveTerrainChunks * 4096; // Rough estimate

    // Log performance metrics
    if (CurrentFrameTime > 0.0f)
    {
        float FPS = 1.0f / CurrentFrameTime;
        UE_LOG(LogTemp, VeryVerbose, TEXT("Terrain Performance: FPS=%.1f, Chunks=%d, Vertices=%d"), 
               FPS, ActiveTerrainChunks, RenderedVertices);
    }
}

void AWorld_PerformanceTerrainSystem::ApplyBiomeToTerrain(World_BiomeType BiomeType, FVector Location, float Radius)
{
    // Find chunks within the biome radius
    for (auto& ChunkPair : TerrainChunks)
    {
        FVector ChunkLocation = FVector(ChunkPair.Key.X * ChunkSize, ChunkPair.Key.Y * ChunkSize, 0);
        float Distance = FVector::Dist(Location, ChunkLocation);
        
        if (Distance <= Radius)
        {
            // Apply biome-specific material
            UMaterialInterface* BiomeMaterial = GetBiomeMaterial(BiomeType);
            if (BiomeMaterial && ChunkPair.Value)
            {
                ChunkPair.Value->SetMaterial(0, BiomeMaterial);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Applied biome %d to terrain at location (%f, %f, %f) with radius %f"), 
           (int32)BiomeType, Location.X, Location.Y, Location.Z, Radius);
}

World_BiomeType AWorld_PerformanceTerrainSystem::GetBiomeAtLocation(FVector Location)
{
    // Simple biome determination based on location
    // In a real implementation, this would use noise functions or biome maps
    
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X * 0.001f, Location.Y * 0.001f));
    
    if (NoiseValue > 0.3f)
    {
        return World_BiomeType::Mountains;
    }
    else if (NoiseValue > 0.0f)
    {
        return World_BiomeType::Forest;
    }
    else if (NoiseValue > -0.3f)
    {
        return World_BiomeType::Plains;
    }
    else
    {
        return World_BiomeType::Desert;
    }
}

float AWorld_PerformanceTerrainSystem::GetTerrainHeightAtLocation(FVector Location)
{
    // Calculate which chunk contains this location
    FIntPoint ChunkCoord = FIntPoint(
        FMath::FloorToInt(Location.X / ChunkSize),
        FMath::FloorToInt(Location.Y / ChunkSize)
    );

    // Check if we have cached height data for this chunk
    if (CachedHeightMaps.Contains(ChunkCoord))
    {
        const TArray<float>& HeightData = CachedHeightMaps[ChunkCoord];
        
        // Calculate local coordinates within the chunk
        float LocalX = FMath::Fmod(Location.X, ChunkSize) / ChunkSize;
        float LocalY = FMath::Fmod(Location.Y, ChunkSize) / ChunkSize;
        
        // Sample height from cached data (simplified bilinear interpolation)
        int32 Size = FMath::Sqrt(HeightData.Num());
        int32 X = FMath::Clamp(FMath::FloorToInt(LocalX * Size), 0, Size - 1);
        int32 Y = FMath::Clamp(FMath::FloorToInt(LocalY * Size), 0, Size - 1);
        
        return HeightData[Y * Size + X] * HeightScale;
    }

    // Generate height on the fly if not cached
    float NoiseValue = FMath::PerlinNoise2D(FVector2D(Location.X * NoiseScale, Location.Y * NoiseScale));
    return NoiseValue * HeightScale;
}

void AWorld_PerformanceTerrainSystem::GenerateHeightMap(int32 ChunkX, int32 ChunkY, TArray<float>& HeightData)
{
    int32 Size = 65; // Standard heightmap size
    HeightData.SetNum(Size * Size);

    // Generate base terrain using Perlin noise
    for (int32 Y = 0; Y < Size; Y++)
    {
        for (int32 X = 0; X < Size; X++)
        {
            float WorldX = (ChunkX * ChunkSize) + (X * ChunkSize / Size);
            float WorldY = (ChunkY * ChunkSize) + (Y * ChunkSize / Size);
            
            float Height = 0.0f;
            float Amplitude = 1.0f;
            float Frequency = NoiseScale;
            
            // Generate multiple octaves of noise
            for (int32 Octave = 0; Octave < NoiseOctaves; Octave++)
            {
                Height += FMath::PerlinNoise2D(FVector2D(WorldX * Frequency, WorldY * Frequency)) * Amplitude;
                Amplitude *= NoisePersistence;
                Frequency *= 2.0f;
            }
            
            HeightData[Y * Size + X] = Height;
        }
    }

    // Cache the height map
    FIntPoint ChunkCoord = FIntPoint(ChunkX, ChunkY);
    CachedHeightMaps.Add(ChunkCoord, HeightData);

    // Limit cache size
    if (CachedHeightMaps.Num() > MaxCachedChunks)
    {
        // Remove oldest cached chunk (simplified LRU)
        auto OldestChunk = CachedHeightMaps.begin();
        CachedHeightMaps.Remove(OldestChunk.Key());
    }
}

void AWorld_PerformanceTerrainSystem::CreateTerrainMesh(const TArray<float>& HeightData, int32 Size, int32 LODLevel)
{
    // This is a simplified version - in a real implementation, you would use ProceduralMeshComponent
    // or generate actual mesh data for the StaticMeshComponent
    
    UE_LOG(LogTemp, Log, TEXT("Creating terrain mesh with size %d and LOD %d"), Size, LODLevel);
    
    // For now, just ensure the component has a basic material
    if (TerrainMeshComponent && GrassMaterial)
    {
        TerrainMeshComponent->SetMaterial(0, GrassMaterial);
    }
}

void AWorld_PerformanceTerrainSystem::EnableLODSystem(bool bEnable)
{
    bEnablePerformanceOptimizations = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Terrain LOD system %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void AWorld_PerformanceTerrainSystem::SetTerrainQuality(int32 QualityLevel)
{
    QualityLevel = FMath::Clamp(QualityLevel, 1, 5);
    
    // Adjust parameters based on quality level
    switch (QualityLevel)
    {
        case 1: // Low quality
            TerrainLODLevels = 2;
            StreamingDistance = 1000.0f;
            break;
        case 2: // Medium quality
            TerrainLODLevels = 3;
            StreamingDistance = 2000.0f;
            break;
        case 3: // High quality
            TerrainLODLevels = 4;
            StreamingDistance = 3000.0f;
            break;
        case 4: // Ultra quality
            TerrainLODLevels = 5;
            StreamingDistance = 4000.0f;
            break;
        case 5: // Maximum quality
            TerrainLODLevels = 6;
            StreamingDistance = 5000.0f;
            break;
    }

    UE_LOG(LogTemp, Log, TEXT("Terrain quality set to level %d"), QualityLevel);
}

void AWorld_PerformanceTerrainSystem::CullDistantTerrain(FVector ViewerLocation)
{
    for (auto& ChunkPair : TerrainChunks)
    {
        FVector ChunkLocation = FVector(ChunkPair.Key.X * ChunkSize, ChunkPair.Key.Y * ChunkSize, 0);
        float Distance = FVector::Dist(ViewerLocation, ChunkLocation);
        
        bool bShouldBeVisible = Distance <= StreamingDistance * 1.2f;
        
        if (ChunkPair.Value)
        {
            ChunkPair.Value->SetVisibility(bShouldBeVisible);
        }
    }
}

void AWorld_PerformanceTerrainSystem::UpdateChunkLOD(int32 ChunkIndex, float DistanceToPlayer)
{
    // Update the LOD level for a specific chunk based on distance
    if (ChunkIndex >= 0 && ChunkIndex < ChunkLODLevels.Num())
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Updated chunk %d LOD based on distance %f"), ChunkIndex, DistanceToPlayer);
    }
}

void AWorld_PerformanceTerrainSystem::OptimizeVertexCount(int32 TargetVertexCount)
{
    // Optimize the total vertex count across all terrain chunks
    if (RenderedVertices > TargetVertexCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Terrain vertex count (%d) exceeds target (%d), optimizing..."), 
               RenderedVertices, TargetVertexCount);
        
        // Reduce LOD levels for distant chunks
        for (int32 i = 0; i < ChunkLODLevels.Num(); i++)
        {
            if (ChunkLODLevels[i] < TerrainLODLevels - 1)
            {
                ChunkLODLevels[i]++;
            }
        }
    }
}

UMaterialInterface* AWorld_PerformanceTerrainSystem::GetBiomeMaterial(World_BiomeType BiomeType)
{
    switch (BiomeType)
    {
        case World_BiomeType::Forest:
            return GrassMaterial;
        case World_BiomeType::Mountains:
            return RockMaterial;
        case World_BiomeType::Desert:
            return SandMaterial;
        case World_BiomeType::Tundra:
            return SnowMaterial;
        default:
            return GrassMaterial;
    }
}
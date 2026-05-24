#include "World_MassiveWorldSystem.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "LandscapeProxy.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"

AWorld_MassiveWorldSystem::AWorld_MassiveWorldSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize world parameters
    WorldSize = FVector2D(50000.0f, 50000.0f);
    ChunkSize = FVector2D(5000.0f, 5000.0f);
    StreamingRadius = 15000.0f;
    MaxActiveChunks = 16;
    
    // Biome configuration
    BiomeTransitionDistance = 2000.0f;
    BiomeNoiseScale = 0.0001f;
    BiomeBlendStrength = 0.5f;
    
    // Performance settings
    bEnableStreamingOptimization = true;
    bEnableLODSystem = true;
    bEnableAsyncGeneration = true;
    MaxGenerationTimePerFrame = 0.016f; // 16ms max per frame
    
    // Initialize biome definitions
    InitializeBiomeDefinitions();
}

void AWorld_MassiveWorldSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldSystem: Starting massive world generation"));
    
    // Initialize world generation
    InitializeWorldGeneration();
    
    // Start streaming system
    StartStreamingSystem();
    
    // Create initial world chunks
    GenerateInitialChunks();
}

void AWorld_MassiveWorldSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableStreamingOptimization)
    {
        UpdateStreamingSystem(DeltaTime);
    }
    
    // Update biome transitions
    UpdateBiomeTransitions(DeltaTime);
    
    // Process async generation queue
    ProcessAsyncGenerationQueue(DeltaTime);
}

void AWorld_MassiveWorldSystem::InitializeWorldGeneration()
{
    // Calculate total chunks needed
    int32 ChunksX = FMath::CeilToInt(WorldSize.X / ChunkSize.X);
    int32 ChunksY = FMath::CeilToInt(WorldSize.Y / ChunkSize.Y);
    
    TotalChunks = ChunksX * ChunksY;
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldSystem: Initializing %d total chunks (%dx%d)"), 
           TotalChunks, ChunksX, ChunksY);
    
    // Initialize chunk grid
    WorldChunks.Empty();
    WorldChunks.SetNum(TotalChunks);
    
    // Initialize biome noise
    BiomeNoise.SetSeed(FMath::Rand());
    BiomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    BiomeNoise.SetFrequency(BiomeNoiseScale);
    
    // Initialize height noise
    HeightNoise.SetSeed(FMath::Rand() + 1000);
    HeightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    HeightNoise.SetFrequency(0.0005f);
    
    // Initialize moisture noise
    MoistureNoise.SetSeed(FMath::Rand() + 2000);
    MoistureNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    MoistureNoise.SetFrequency(0.0003f);
}

void AWorld_MassiveWorldSystem::StartStreamingSystem()
{
    if (!bEnableStreamingOptimization) return;
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldSystem: Starting streaming system"));
    
    // Get player pawn for streaming center
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        LastPlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Initialize streaming state
    ActiveChunks.Empty();
    LoadingChunks.Empty();
    UnloadingChunks.Empty();
}

void AWorld_MassiveWorldSystem::GenerateInitialChunks()
{
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldSystem: Generating initial chunks"));
    
    // Get player location for initial chunk generation
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Generate chunks around player
    TArray<FIntPoint> InitialChunks = GetChunksInRadius(PlayerLocation, StreamingRadius * 0.5f);
    
    for (const FIntPoint& ChunkCoord : InitialChunks)
    {
        if (ActiveChunks.Num() < MaxActiveChunks)
        {
            GenerateChunk(ChunkCoord);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_MassiveWorldSystem: Generated %d initial chunks"), ActiveChunks.Num());
}

void AWorld_MassiveWorldSystem::UpdateStreamingSystem(float DeltaTime)
{
    // Get current player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector CurrentPlayerLocation = PlayerPawn->GetActorLocation();
    
    // Check if player has moved significantly
    float MovementDistance = FVector::Dist(CurrentPlayerLocation, LastPlayerLocation);
    if (MovementDistance < 1000.0f) return; // Only update if moved 1km
    
    LastPlayerLocation = CurrentPlayerLocation;
    
    // Get chunks that should be loaded
    TArray<FIntPoint> RequiredChunks = GetChunksInRadius(CurrentPlayerLocation, StreamingRadius);
    
    // Find chunks to load
    for (const FIntPoint& ChunkCoord : RequiredChunks)
    {
        if (!ActiveChunks.Contains(ChunkCoord) && !LoadingChunks.Contains(ChunkCoord))
        {
            if (ActiveChunks.Num() < MaxActiveChunks)
            {
                LoadingChunks.Add(ChunkCoord);
                QueueChunkGeneration(ChunkCoord);
            }
        }
    }
    
    // Find chunks to unload
    TArray<FIntPoint> ChunksToUnload;
    for (const FIntPoint& ActiveChunk : ActiveChunks)
    {
        if (!RequiredChunks.Contains(ActiveChunk))
        {
            ChunksToUnload.Add(ActiveChunk);
        }
    }
    
    // Unload distant chunks
    for (const FIntPoint& ChunkCoord : ChunksToUnload)
    {
        UnloadChunk(ChunkCoord);
    }
}

void AWorld_MassiveWorldSystem::UpdateBiomeTransitions(float DeltaTime)
{
    // Update biome transition effects
    for (auto& ChunkPair : ActiveChunks)
    {
        FWorld_ChunkData& Chunk = ChunkPair.Value;
        if (Chunk.bNeedsTransitionUpdate)
        {
            UpdateChunkBiomeTransition(ChunkPair.Key, Chunk);
            Chunk.bNeedsTransitionUpdate = false;
        }
    }
}

void AWorld_MassiveWorldSystem::ProcessAsyncGenerationQueue(float DeltaTime)
{
    if (!bEnableAsyncGeneration) return;
    
    float TimeUsed = 0.0f;
    
    while (GenerationQueue.Num() > 0 && TimeUsed < MaxGenerationTimePerFrame)
    {
        float StartTime = FPlatformTime::Seconds();
        
        FIntPoint ChunkCoord = GenerationQueue[0];
        GenerationQueue.RemoveAt(0);
        
        // Generate chunk synchronously (in small time slice)
        GenerateChunk(ChunkCoord);
        
        float EndTime = FPlatformTime::Seconds();
        TimeUsed += (EndTime - StartTime);
    }
}

TArray<FIntPoint> AWorld_MassiveWorldSystem::GetChunksInRadius(const FVector& Center, float Radius)
{
    TArray<FIntPoint> ChunksInRadius;
    
    // Convert world position to chunk coordinates
    FIntPoint CenterChunk = WorldToChunkCoord(Center);
    
    // Calculate chunk radius
    int32 ChunkRadius = FMath::CeilToInt(Radius / ChunkSize.X);
    
    // Iterate through chunks in radius
    for (int32 X = CenterChunk.X - ChunkRadius; X <= CenterChunk.X + ChunkRadius; X++)
    {
        for (int32 Y = CenterChunk.Y - ChunkRadius; Y <= CenterChunk.Y + ChunkRadius; Y++)
        {
            FIntPoint ChunkCoord(X, Y);
            FVector ChunkCenter = ChunkCoordToWorld(ChunkCoord);
            
            // Check if chunk is within radius
            float Distance = FVector::Dist2D(Center, ChunkCenter);
            if (Distance <= Radius)
            {
                ChunksInRadius.Add(ChunkCoord);
            }
        }
    }
    
    return ChunksInRadius;
}

void AWorld_MassiveWorldSystem::GenerateChunk(const FIntPoint& ChunkCoord)
{
    if (ActiveChunks.Contains(ChunkCoord)) return;
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldSystem: Generating chunk (%d, %d)"), ChunkCoord.X, ChunkCoord.Y);
    
    // Create chunk data
    FWorld_ChunkData ChunkData;
    ChunkData.ChunkCoord = ChunkCoord;
    ChunkData.WorldPosition = ChunkCoordToWorld(ChunkCoord);
    ChunkData.bIsLoaded = true;
    ChunkData.bNeedsTransitionUpdate = true;
    
    // Determine chunk biome
    ChunkData.PrimaryBiome = DetermineBiome(ChunkData.WorldPosition);
    
    // Generate terrain for chunk
    GenerateChunkTerrain(ChunkData);
    
    // Generate vegetation for chunk
    GenerateChunkVegetation(ChunkData);
    
    // Generate water features for chunk
    GenerateChunkWaterFeatures(ChunkData);
    
    // Generate landmarks for chunk
    GenerateChunkLandmarks(ChunkData);
    
    // Add to active chunks
    ActiveChunks.Add(ChunkCoord, ChunkData);
    LoadingChunks.Remove(ChunkCoord);
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldSystem: Chunk (%d, %d) generated successfully"), ChunkCoord.X, ChunkCoord.Y);
}

void AWorld_MassiveWorldSystem::UnloadChunk(const FIntPoint& ChunkCoord)
{
    if (!ActiveChunks.Contains(ChunkCoord)) return;
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldSystem: Unloading chunk (%d, %d)"), ChunkCoord.X, ChunkCoord.Y);
    
    FWorld_ChunkData& ChunkData = ActiveChunks[ChunkCoord];
    
    // Destroy all actors in chunk
    for (AActor* Actor : ChunkData.ChunkActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    // Remove from active chunks
    ActiveChunks.Remove(ChunkCoord);
    
    UE_LOG(LogTemp, Log, TEXT("World_MassiveWorldSystem: Chunk (%d, %d) unloaded"), ChunkCoord.X, ChunkCoord.Y);
}

void AWorld_MassiveWorldSystem::QueueChunkGeneration(const FIntPoint& ChunkCoord)
{
    if (!GenerationQueue.Contains(ChunkCoord))
    {
        GenerationQueue.Add(ChunkCoord);
    }
}

EWorld_BiomeType AWorld_MassiveWorldSystem::DetermineBiome(const FVector& WorldPosition)
{
    // Sample biome noise
    float BiomeValue = BiomeNoise.GetNoise(WorldPosition.X, WorldPosition.Y);
    float HeightValue = HeightNoise.GetNoise(WorldPosition.X, WorldPosition.Y);
    float MoistureValue = MoistureNoise.GetNoise(WorldPosition.X, WorldPosition.Y);
    
    // Normalize values
    BiomeValue = (BiomeValue + 1.0f) * 0.5f;
    HeightValue = (HeightValue + 1.0f) * 0.5f;
    MoistureValue = (MoistureValue + 1.0f) * 0.5f;
    
    // Determine biome based on height and moisture
    if (HeightValue > 0.8f)
    {
        return EWorld_BiomeType::Mountain;
    }
    else if (HeightValue < 0.2f)
    {
        if (MoistureValue > 0.6f)
        {
            return EWorld_BiomeType::Swamp;
        }
        else
        {
            return EWorld_BiomeType::Desert;
        }
    }
    else if (MoistureValue > 0.7f)
    {
        return EWorld_BiomeType::TropicalRainforest;
    }
    else if (MoistureValue > 0.4f)
    {
        return EWorld_BiomeType::TemperateForest;
    }
    else if (BiomeValue > 0.6f)
    {
        return EWorld_BiomeType::Volcanic;
    }
    else
    {
        return EWorld_BiomeType::Grassland;
    }
}

void AWorld_MassiveWorldSystem::GenerateChunkTerrain(FWorld_ChunkData& ChunkData)
{
    // Create landscape for chunk (simplified for performance)
    UClass* LandscapeClass = ALandscape::StaticClass();
    if (LandscapeClass)
    {
        ALandscape* ChunkLandscape = GetWorld()->SpawnActor<ALandscape>(
            LandscapeClass,
            ChunkData.WorldPosition,
            FRotator::ZeroRotator
        );
        
        if (ChunkLandscape)
        {
            ChunkLandscape->SetActorLabel(FString::Printf(TEXT("Landscape_Chunk_%d_%d"), ChunkData.ChunkCoord.X, ChunkData.ChunkCoord.Y));
            ChunkData.ChunkActors.Add(ChunkLandscape);
        }
    }
}

void AWorld_MassiveWorldSystem::GenerateChunkVegetation(FWorld_ChunkData& ChunkData)
{
    // Get biome definition
    if (!BiomeDefinitions.Contains(ChunkData.PrimaryBiome)) return;
    
    const FWorld_BiomeDefinition& BiomeDef = BiomeDefinitions[ChunkData.PrimaryBiome];
    
    // Generate vegetation based on biome
    int32 VegetationCount = FMath::RandRange(BiomeDef.MinVegetationDensity, BiomeDef.MaxVegetationDensity);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector VegPosition = ChunkData.WorldPosition + FVector(
            FMath::RandRange(-ChunkSize.X * 0.5f, ChunkSize.X * 0.5f),
            FMath::RandRange(-ChunkSize.Y * 0.5f, ChunkSize.Y * 0.5f),
            0.0f
        );
        
        // Create simple vegetation actor (cube for now)
        AActor* VegActor = GetWorld()->SpawnActor<AActor>(VegPosition, FRotator::ZeroRotator);
        if (VegActor)
        {
            UStaticMeshComponent* MeshComp = VegActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VegetationMesh"));
            VegActor->SetRootComponent(MeshComp);
            
            // Set random scale
            float Scale = FMath::RandRange(0.5f, 2.0f);
            VegActor->SetActorScale3D(FVector(Scale));
            
            VegActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%d_%d_%d"), ChunkData.ChunkCoord.X, ChunkData.ChunkCoord.Y, i));
            ChunkData.ChunkActors.Add(VegActor);
        }
    }
}

void AWorld_MassiveWorldSystem::GenerateChunkWaterFeatures(FWorld_ChunkData& ChunkData)
{
    // Generate water features based on biome and noise
    float WaterChance = 0.1f; // 10% chance for water features
    
    if (ChunkData.PrimaryBiome == EWorld_BiomeType::Swamp)
    {
        WaterChance = 0.8f;
    }
    else if (ChunkData.PrimaryBiome == EWorld_BiomeType::TropicalRainforest)
    {
        WaterChance = 0.3f;
    }
    
    if (FMath::RandRange(0.0f, 1.0f) < WaterChance)
    {
        // Create water feature
        FVector WaterPosition = ChunkData.WorldPosition + FVector(
            FMath::RandRange(-ChunkSize.X * 0.3f, ChunkSize.X * 0.3f),
            FMath::RandRange(-ChunkSize.Y * 0.3f, ChunkSize.Y * 0.3f),
            -50.0f
        );
        
        AActor* WaterActor = GetWorld()->SpawnActor<AActor>(WaterPosition, FRotator::ZeroRotator);
        if (WaterActor)
        {
            UStaticMeshComponent* MeshComp = WaterActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMesh"));
            WaterActor->SetRootComponent(MeshComp);
            
            // Scale water feature
            WaterActor->SetActorScale3D(FVector(10.0f, 10.0f, 1.0f));
            
            WaterActor->SetActorLabel(FString::Printf(TEXT("Water_%d_%d"), ChunkData.ChunkCoord.X, ChunkData.ChunkCoord.Y));
            ChunkData.ChunkActors.Add(WaterActor);
        }
    }
}

void AWorld_MassiveWorldSystem::GenerateChunkLandmarks(FWorld_ChunkData& ChunkData)
{
    // Generate landmarks based on biome
    float LandmarkChance = 0.05f; // 5% chance for landmarks
    
    if (ChunkData.PrimaryBiome == EWorld_BiomeType::Mountain)
    {
        LandmarkChance = 0.2f;
    }
    else if (ChunkData.PrimaryBiome == EWorld_BiomeType::Volcanic)
    {
        LandmarkChance = 0.15f;
    }
    
    if (FMath::RandRange(0.0f, 1.0f) < LandmarkChance)
    {
        // Create landmark
        FVector LandmarkPosition = ChunkData.WorldPosition + FVector(
            FMath::RandRange(-ChunkSize.X * 0.2f, ChunkSize.X * 0.2f),
            FMath::RandRange(-ChunkSize.Y * 0.2f, ChunkSize.Y * 0.2f),
            100.0f
        );
        
        AActor* LandmarkActor = GetWorld()->SpawnActor<AActor>(LandmarkPosition, FRotator::ZeroRotator);
        if (LandmarkActor)
        {
            UStaticMeshComponent* MeshComp = LandmarkActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LandmarkMesh"));
            LandmarkActor->SetRootComponent(MeshComp);
            
            // Scale landmark
            LandmarkActor->SetActorScale3D(FVector(5.0f, 5.0f, 10.0f));
            
            LandmarkActor->SetActorLabel(FString::Printf(TEXT("Landmark_%d_%d"), ChunkData.ChunkCoord.X, ChunkData.ChunkCoord.Y));
            ChunkData.ChunkActors.Add(LandmarkActor);
        }
    }
}

void AWorld_MassiveWorldSystem::UpdateChunkBiomeTransition(const FIntPoint& ChunkCoord, FWorld_ChunkData& ChunkData)
{
    // Update biome transition effects for smooth blending
    // This would involve updating materials, vegetation density, etc.
    // For now, just mark as updated
    ChunkData.bNeedsTransitionUpdate = false;
}

void AWorld_MassiveWorldSystem::InitializeBiomeDefinitions()
{
    // Grassland biome
    FWorld_BiomeDefinition GrasslandBiome;
    GrasslandBiome.BiomeType = EWorld_BiomeType::Grassland;
    GrasslandBiome.MinVegetationDensity = 20;
    GrasslandBiome.MaxVegetationDensity = 50;
    GrasslandBiome.PrimaryColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::Grassland, GrasslandBiome);
    
    // Forest biome
    FWorld_BiomeDefinition ForestBiome;
    ForestBiome.BiomeType = EWorld_BiomeType::TemperateForest;
    ForestBiome.MinVegetationDensity = 80;
    ForestBiome.MaxVegetationDensity = 150;
    ForestBiome.PrimaryColor = FLinearColor(0.1f, 0.6f, 0.1f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::TemperateForest, ForestBiome);
    
    // Desert biome
    FWorld_BiomeDefinition DesertBiome;
    DesertBiome.BiomeType = EWorld_BiomeType::Desert;
    DesertBiome.MinVegetationDensity = 2;
    DesertBiome.MaxVegetationDensity = 8;
    DesertBiome.PrimaryColor = FLinearColor(0.9f, 0.8f, 0.3f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::Desert, DesertBiome);
    
    // Mountain biome
    FWorld_BiomeDefinition MountainBiome;
    MountainBiome.BiomeType = EWorld_BiomeType::Mountain;
    MountainBiome.MinVegetationDensity = 10;
    MountainBiome.MaxVegetationDensity = 30;
    MountainBiome.PrimaryColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::Mountain, MountainBiome);
    
    // Tropical Rainforest biome
    FWorld_BiomeDefinition TropicalBiome;
    TropicalBiome.BiomeType = EWorld_BiomeType::TropicalRainforest;
    TropicalBiome.MinVegetationDensity = 150;
    TropicalBiome.MaxVegetationDensity = 300;
    TropicalBiome.PrimaryColor = FLinearColor(0.0f, 0.8f, 0.0f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::TropicalRainforest, TropicalBiome);
    
    // Swamp biome
    FWorld_BiomeDefinition SwampBiome;
    SwampBiome.BiomeType = EWorld_BiomeType::Swamp;
    SwampBiome.MinVegetationDensity = 60;
    SwampBiome.MaxVegetationDensity = 120;
    SwampBiome.PrimaryColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::Swamp, SwampBiome);
    
    // Volcanic biome
    FWorld_BiomeDefinition VolcanicBiome;
    VolcanicBiome.BiomeType = EWorld_BiomeType::Volcanic;
    VolcanicBiome.MinVegetationDensity = 5;
    VolcanicBiome.MaxVegetationDensity = 15;
    VolcanicBiome.PrimaryColor = FLinearColor(0.8f, 0.2f, 0.0f, 1.0f);
    BiomeDefinitions.Add(EWorld_BiomeType::Volcanic, VolcanicBiome);
}

FIntPoint AWorld_MassiveWorldSystem::WorldToChunkCoord(const FVector& WorldPosition)
{
    return FIntPoint(
        FMath::FloorToInt(WorldPosition.X / ChunkSize.X),
        FMath::FloorToInt(WorldPosition.Y / ChunkSize.Y)
    );
}

FVector AWorld_MassiveWorldSystem::ChunkCoordToWorld(const FIntPoint& ChunkCoord)
{
    return FVector(
        ChunkCoord.X * ChunkSize.X + ChunkSize.X * 0.5f,
        ChunkCoord.Y * ChunkSize.Y + ChunkSize.Y * 0.5f,
        0.0f
    );
}
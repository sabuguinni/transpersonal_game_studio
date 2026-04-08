/**
 * @file ProceduralWorldGenerator.cpp
 * @brief Implementation of procedural world generation with consciousness integration
 * @author Transpersonal Game Studio - Procedural World Generator
 * @date 2024
 */

#include "ProceduralWorldGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Async/AsyncWork.h"

UProceduralWorldGenerator::UProceduralWorldGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    RandomGenerator.Initialize(Seed);
}

void UProceduralWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    RandomGenerator.Initialize(Seed);
    LastChunkUpdateTime = 0.0f;
    
    // Generate initial chunks around player spawn
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        FVector2D PlayerChunk = FVector2D(
            FMath::FloorToInt(PlayerLocation.X / (ChunkSize * 100.0f)),
            FMath::FloorToInt(PlayerLocation.Y / (ChunkSize * 100.0f))
        );
        
        // Generate chunks in spiral pattern around player
        for (int32 Ring = 0; Ring <= 2; Ring++)
        {
            for (int32 Side = 0; Side < 4; Side++)
            {
                for (int32 Step = 0; Step < Ring * 2 + 1; Step++)
                {
                    FVector2D ChunkCoord = PlayerChunk;
                    
                    switch (Side)
                    {
                        case 0: ChunkCoord += FVector2D(Ring, -Ring + Step); break;
                        case 1: ChunkCoord += FVector2D(Ring - Step, Ring); break;
                        case 2: ChunkCoord += FVector2D(-Ring, Ring - Step); break;
                        case 3: ChunkCoord += FVector2D(-Ring + Step, -Ring); break;
                    }
                    
                    GenerateChunk(ChunkCoord);
                }
            }
        }
    }
}

void UProceduralWorldGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastChunkUpdateTime += DeltaTime;
    
    if (LastChunkUpdateTime >= ChunkUpdateInterval)
    {
        UpdateChunkPriorities();
        ProcessGenerationQueue();
        LastChunkUpdateTime = 0.0f;
    }
}

void UProceduralWorldGenerator::GenerateChunk(FVector2D ChunkCoordinate)
{
    if (LoadedChunks.Contains(ChunkCoordinate))
    {
        return; // Chunk already exists
    }
    
    if (LoadedChunks.Num() >= MaxActiveChunks)
    {
        // Find furthest chunk to unload
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            FVector2D PlayerChunk = FVector2D(
                PlayerLocation.X / (ChunkSize * 100.0f),
                PlayerLocation.Y / (ChunkSize * 100.0f)
            );
            
            float MaxDistance = 0.0f;
            FVector2D FarthestChunk;
            
            for (auto& ChunkPair : LoadedChunks)
            {
                float Distance = FVector2D::Distance(ChunkPair.Key, PlayerChunk);
                if (Distance > MaxDistance)
                {
                    MaxDistance = Distance;
                    FarthestChunk = ChunkPair.Key;
                }
            }
            
            if (MaxDistance > RenderDistance)
            {
                UnloadChunk(FarthestChunk);
            }
        }
    }
    
    // Create new chunk data
    FChunkData NewChunk;
    NewChunk.ChunkCoordinate = ChunkCoordinate;
    NewChunk.bIsLoaded = false;
    
    // Calculate world position for this chunk
    FVector ChunkWorldPos = FVector(
        ChunkCoordinate.X * ChunkSize * 100.0f,
        ChunkCoordinate.Y * ChunkSize * 100.0f,
        0.0f
    );
    
    // Generate terrain vertices
    NewChunk.TerrainVertices.Reserve((ChunkSize + 1) * (ChunkSize + 1));
    NewChunk.TerrainIndices.Reserve(ChunkSize * ChunkSize * 6);
    
    for (int32 Y = 0; Y <= ChunkSize; Y++)
    {
        for (int32 X = 0; X <= ChunkSize; X++)
        {
            FVector WorldPos = ChunkWorldPos + FVector(X * 100.0f, Y * 100.0f, 0.0f);
            float Height = GetHeightAtLocation(FVector2D(WorldPos.X, WorldPos.Y));
            
            // Apply consciousness influence
            float ConsciousnessModifier = CalculateConsciousnessResonance(WorldPos);
            Height += ConsciousnessModifier * ConsciousnessTerrainModifier * 100.0f;
            
            NewChunk.TerrainVertices.Add(FVector(X * 100.0f, Y * 100.0f, Height));
        }
    }
    
    // Generate indices for triangulation
    for (int32 Y = 0; Y < ChunkSize; Y++)
    {
        for (int32 X = 0; X < ChunkSize; X++)
        {
            int32 BottomLeft = Y * (ChunkSize + 1) + X;
            int32 BottomRight = BottomLeft + 1;
            int32 TopLeft = (Y + 1) * (ChunkSize + 1) + X;
            int32 TopRight = TopLeft + 1;
            
            // First triangle
            NewChunk.TerrainIndices.Add(BottomLeft);
            NewChunk.TerrainIndices.Add(TopLeft);
            NewChunk.TerrainIndices.Add(BottomRight);
            
            // Second triangle
            NewChunk.TerrainIndices.Add(BottomRight);
            NewChunk.TerrainIndices.Add(TopLeft);
            NewChunk.TerrainIndices.Add(TopRight);
        }
    }
    
    // Calculate biome for chunk center
    NewChunk.BiomeData = CalculateBiome(ChunkWorldPos + FVector(ChunkSize * 50.0f, ChunkSize * 50.0f, 0.0f));
    NewChunk.bIsLoaded = true;
    
    LoadedChunks.Add(ChunkCoordinate, NewChunk);
    
    UE_LOG(LogTemp, Log, TEXT("Generated chunk at (%f, %f)"), ChunkCoordinate.X, ChunkCoordinate.Y);
}

void UProceduralWorldGenerator::UnloadChunk(FVector2D ChunkCoordinate)
{
    if (LoadedChunks.Contains(ChunkCoordinate))
    {
        LoadedChunks.Remove(ChunkCoordinate);
        UE_LOG(LogTemp, Log, TEXT("Unloaded chunk at (%f, %f)"), ChunkCoordinate.X, ChunkCoordinate.Y);
    }
}

FBiomeParameters UProceduralWorldGenerator::CalculateBiome(FVector WorldLocation)
{
    FBiomeParameters Biome;
    
    // Generate base environmental parameters using noise
    float TempNoise = GeneratePerlinNoise(WorldLocation.X * 0.0001f, WorldLocation.Y * 0.0001f, 3);
    float HumidNoise = GeneratePerlinNoise(WorldLocation.X * 0.0002f + 1000.0f, WorldLocation.Y * 0.0002f + 1000.0f, 3);
    float ElevNoise = GenerateRidgedNoise(WorldLocation.X * 0.0005f, WorldLocation.Y * 0.0005f);
    
    Biome.Temperature = FMath::Clamp(TempNoise * 0.5f + 0.5f, 0.0f, 1.0f);
    Biome.Humidity = FMath::Clamp(HumidNoise * 0.5f + 0.5f, 0.0f, 1.0f);
    Biome.Elevation = FMath::Clamp(ElevNoise * 0.5f + 0.5f, 0.0f, 1.0f);
    
    // Calculate consciousness resonance
    Biome.ConsciousnessResonance = CalculateConsciousnessResonance(WorldLocation);
    
    // Determine aura color based on biome characteristics
    float Hue = (Biome.Temperature + Biome.ConsciousnessResonance) * 0.5f * 360.0f;
    float Saturation = FMath::Lerp(0.3f, 1.0f, Biome.Humidity);
    float Value = FMath::Lerp(0.5f, 1.0f, Biome.Elevation);
    
    Biome.AuraColor = FLinearColor::MakeFromHSV8(Hue, Saturation * 255, Value * 255);
    
    return Biome;
}

float UProceduralWorldGenerator::GetHeightAtLocation(FVector2D WorldLocation)
{
    float Height = 0.0f;
    
    // Base terrain using multiple octaves of Perlin noise
    Height += GeneratePerlinNoise(WorldLocation.X * NoiseScale, WorldLocation.Y * NoiseScale, 1) * HeightMultiplier * 0.5f;
    Height += GeneratePerlinNoise(WorldLocation.X * NoiseScale * 2.0f, WorldLocation.Y * NoiseScale * 2.0f, 1) * HeightMultiplier * 0.25f;
    Height += GeneratePerlinNoise(WorldLocation.X * NoiseScale * 4.0f, WorldLocation.Y * NoiseScale * 4.0f, 1) * HeightMultiplier * 0.125f;
    
    // Add ridged noise for mountain features
    float RidgedContribution = GenerateRidgedNoise(WorldLocation.X * NoiseScale * 0.5f, WorldLocation.Y * NoiseScale * 0.5f);
    Height += RidgedContribution * HeightMultiplier * 0.3f;
    
    // Add sacred geometry influence
    TArray<FVector> SacredPoints = CalculateSacredGeometryPoints(FVector(WorldLocation, 0.0f), 5000.0f);
    for (const FVector& Point : SacredPoints)
    {
        float Distance = FVector2D::Distance(WorldLocation, FVector2D(Point.X, Point.Y));
        if (Distance < 1000.0f)
        {
            float Influence = 1.0f - (Distance / 1000.0f);
            Height += FMath::Sin(Influence * PI) * 200.0f;
        }
    }
    
    return Height;
}

void UProceduralWorldGenerator::UpdateConsciousnessInfluence(FVector PlayerLocation, EConsciousnessState State, float Intensity)
{
    // Update chunks within consciousness influence radius
    FVector2D PlayerChunk = FVector2D(
        PlayerLocation.X / (ChunkSize * 100.0f),
        PlayerLocation.Y / (ChunkSize * 100.0f)
    );
    
    int32 InfluenceRadius = FMath::CeilToInt(ConsciousnessInfluenceRadius / (ChunkSize * 100.0f));
    
    for (int32 X = -InfluenceRadius; X <= InfluenceRadius; X++)
    {
        for (int32 Y = -InfluenceRadius; Y <= InfluenceRadius; Y++)
        {
            FVector2D ChunkCoord = PlayerChunk + FVector2D(X, Y);
            
            if (LoadedChunks.Contains(ChunkCoord))
            {
                FChunkData& Chunk = LoadedChunks[ChunkCoord];
                
                // Calculate distance-based influence
                float Distance = FVector2D::Distance(ChunkCoord, PlayerChunk) * ChunkSize * 100.0f;
                float InfluenceFactor = FMath::Clamp(1.0f - (Distance / ConsciousnessInfluenceRadius), 0.0f, 1.0f);
                
                // Apply consciousness state modifier
                float StateModifier = 1.0f;
                switch (State)
                {
                    case EConsciousnessState::Ordinary:
                        StateModifier = 0.1f;
                        break;
                    case EConsciousnessState::Meditative:
                        StateModifier = 0.5f;
                        break;
                    case EConsciousnessState::Transcendent:
                        StateModifier = 1.0f;
                        break;
                    case EConsciousnessState::Unity:
                        StateModifier = 1.5f;
                        break;
                }
                
                Chunk.ConsciousnessInfluence = InfluenceFactor * Intensity * StateModifier;
                
                // Trigger biome transition if influence is significant
                if (Chunk.ConsciousnessInfluence > 0.3f)
                {
                    FVector ChunkCenter = FVector(
                        ChunkCoord.X * ChunkSize * 100.0f + ChunkSize * 50.0f,
                        ChunkCoord.Y * ChunkSize * 100.0f + ChunkSize * 50.0f,
                        0.0f
                    );
                    
                    FBiomeParameters NewBiome = CalculateBiome(ChunkCenter);
                    RegisterBiomeTransition(ChunkCenter, Chunk.BiomeData, NewBiome);
                }
            }
        }
    }
}

float UProceduralWorldGenerator::GeneratePerlinNoise(float X, float Y, int32 Octaves)
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Value / MaxValue;
}

float UProceduralWorldGenerator::GenerateRidgedNoise(float X, float Y)
{
    float Noise = FMath::PerlinNoise2D(FVector2D(X, Y));
    return 1.0f - FMath::Abs(Noise);
}

float UProceduralWorldGenerator::CalculateConsciousnessResonance(FVector Location)
{
    // Use Fibonacci spiral and golden ratio for consciousness resonance calculation
    float GoldenRatio = 1.618033988749f;
    float Angle = FMath::Atan2(Location.Y, Location.X);
    float Distance = FVector2D(Location.X, Location.Y).Size();
    
    float SpiralValue = GetGoldenRatioSpiral(Angle, Distance * 0.001f);
    float VoronoiValue = GenerateVoronoiNoise(Location.X * 0.0001f, Location.Y * 0.0001f);
    
    return FMath::Clamp((SpiralValue + VoronoiValue) * 0.5f, 0.0f, 1.0f);
}

TArray<FVector> UProceduralWorldGenerator::CalculateSacredGeometryPoints(FVector Center, float Radius)
{
    TArray<FVector> Points;
    
    // Generate points using Fibonacci sphere distribution
    int32 NumPoints = 21; // Fibonacci number for sacred geometry
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector Point = GetFibonacciSpherePoint(i, NumPoints);
        Point = Center + Point * Radius;
        Points.Add(Point);
    }
    
    return Points;
}

float UProceduralWorldGenerator::GetGoldenRatioSpiral(float Angle, float Scale)
{
    float GoldenRatio = 1.618033988749f;
    float SpiralRadius = Scale * FMath::Pow(GoldenRatio, Angle / (2.0f * PI));
    return FMath::Sin(SpiralRadius) * 0.5f + 0.5f;
}

FVector UProceduralWorldGenerator::GetFibonacciSpherePoint(int32 Index, int32 Total)
{
    float GoldenAngle = PI * (3.0f - FMath::Sqrt(5.0f)); // Golden angle in radians
    
    float Y = 1.0f - (Index / float(Total - 1)) * 2.0f; // Y from 1 to -1
    float Radius = FMath::Sqrt(1.0f - Y * Y);
    
    float Theta = GoldenAngle * Index;
    
    float X = FMath::Cos(Theta) * Radius;
    float Z = FMath::Sin(Theta) * Radius;
    
    return FVector(X, Y, Z);
}

void UProceduralWorldGenerator::UpdateChunkPriorities()
{
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        LastPlayerLocation = PlayerLocation;
        
        // Update LOD for all loaded chunks
        for (auto& ChunkPair : LoadedChunks)
        {
            FVector ChunkCenter = FVector(
                ChunkPair.Key.X * ChunkSize * 100.0f + ChunkSize * 50.0f,
                ChunkPair.Key.Y * ChunkSize * 100.0f + ChunkSize * 50.0f,
                0.0f
            );
            
            float Distance = FVector::Distance(PlayerLocation, ChunkCenter);
            OptimizeChunkLOD(ChunkPair.Key, Distance);
        }
    }
}

void UProceduralWorldGenerator::ProcessGenerationQueue()
{
    // Process up to 2 chunks per frame to maintain performance
    int32 ProcessedChunks = 0;
    const int32 MaxChunksPerFrame = 2;
    
    while (ChunkGenerationQueue.Num() > 0 && ProcessedChunks < MaxChunksPerFrame)
    {
        FVector2D ChunkCoord = ChunkGenerationQueue[0];
        ChunkGenerationQueue.RemoveAt(0);
        
        if (!LoadedChunks.Contains(ChunkCoord))
        {
            if (bEnableAsyncGeneration)
            {
                StartAsyncChunkGeneration(ChunkCoord);
            }
            else
            {
                GenerateChunk(ChunkCoord);
            }
            ProcessedChunks++;
        }
    }
}

void UProceduralWorldGenerator::OptimizeChunkLOD(FVector2D ChunkCoordinate, float DistanceToPlayer)
{
    if (!LoadedChunks.Contains(ChunkCoordinate))
        return;
    
    FChunkData& Chunk = LoadedChunks[ChunkCoordinate];
    
    // Implement LOD based on distance
    if (DistanceToPlayer > RenderDistance * ChunkSize * 100.0f * 0.8f)
    {
        // Far LOD - reduce vertex density
        // Implementation would modify TerrainVertices array
    }
    else if (DistanceToPlayer > RenderDistance * ChunkSize * 100.0f * 0.5f)
    {
        // Medium LOD
        // Implementation would use medium vertex density
    }
    // Close LOD uses full detail (default)
}

void UProceduralWorldGenerator::RegisterBiomeTransition(FVector Location, FBiomeParameters FromBiome, FBiomeParameters ToBiome)
{
    // Smooth transition between biomes over time
    // This would typically trigger visual effects and gradual parameter changes
    UE_LOG(LogTemp, Log, TEXT("Biome transition registered at location (%f, %f, %f)"), 
           Location.X, Location.Y, Location.Z);
}

TArray<FVector> UProceduralWorldGenerator::GetSacredSiteLocations(float Radius)
{
    TArray<FVector> SacredSites;
    
    if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
    {
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        SacredSites = CalculateSacredGeometryPoints(PlayerLocation, Radius);
    }
    
    return SacredSites;
}

float UProceduralWorldGenerator::GenerateVoronoiNoise(float X, float Y)
{
    // Simple Voronoi noise implementation
    FVector2D Point = FVector2D(X, Y);
    FVector2D Cell = FVector2D(FMath::FloorToFloat(X), FMath::FloorToFloat(Y));
    
    float MinDistance = FLT_MAX;
    
    for (int32 i = -1; i <= 1; i++)
    {
        for (int32 j = -1; j <= 1; j++)
        {
            FVector2D Neighbor = Cell + FVector2D(i, j);
            FVector2D FeaturePoint = Neighbor + FVector2D(
                RandomGenerator.GetFraction(),
                RandomGenerator.GetFraction()
            );
            
            float Distance = FVector2D::Distance(Point, FeaturePoint);
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return FMath::Clamp(MinDistance, 0.0f, 1.0f);
}
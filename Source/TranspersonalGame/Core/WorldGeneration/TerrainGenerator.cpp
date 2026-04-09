// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "TerrainGenerator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "LandscapeStreamingProxy.h"
#include "Materials/MaterialInterface.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/KismetMathLibrary.h"
#include "Algo/Transform.h"

DEFINE_LOG_CATEGORY(LogTerrainGenerator);

UTerrainGenerator::UTerrainGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    
    // Initialize default world bounds (20km x 20km)
    WorldBounds = FVector2D(2000000.0f, 2000000.0f); // 20km in cm
}

void UTerrainGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("TerrainGenerator component initialized"));
}

void UTerrainGenerator::GenerateHeightmap(TArray<uint16>& OutHeightData, int32 Resolution, int32 Seed)
{
    UE_LOG(LogTerrainGenerator, Log, TEXT("Generating heightmap with resolution %d and seed %d"), Resolution, Seed);
    
    // Initialize heightmap data
    OutHeightData.SetNum(Resolution * Resolution);
    TArray<float> FloatHeightData;
    FloatHeightData.SetNum(Resolution * Resolution);
    
    // Generate base terrain using multiple noise layers
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 Index = Y * Resolution + X;
            float Height = 0.0f;
            
            // Convert to world coordinates
            float WorldX = (float(X) / float(Resolution - 1)) * WorldBounds.X - (WorldBounds.X * 0.5f);
            float WorldY = (float(Y) / float(Resolution - 1)) * WorldBounds.Y - (WorldBounds.Y * 0.5f);
            
            // Scale for noise generation
            float NoiseX = WorldX * 0.0001f; // Scale down for noise
            float NoiseY = WorldY * 0.0001f;
            
            // Base terrain layer - large scale features
            Height += GeneratePerlinNoise(NoiseX * 0.5f, NoiseY * 0.5f, Seed) * 1000.0f;
            
            // Medium scale features - hills and valleys
            Height += GeneratePerlinNoise(NoiseX * 2.0f, NoiseY * 2.0f, Seed + 1) * 300.0f;
            
            // Small scale features - surface detail
            Height += GeneratePerlinNoise(NoiseX * 8.0f, NoiseY * 8.0f, Seed + 2) * 50.0f;
            
            // Add some ridged noise for mountain ranges
            Height += GenerateRidgedNoise(NoiseX * 1.0f, NoiseY * 1.0f, Seed + 3) * 500.0f;
            
            // Ensure height is positive and within reasonable bounds
            Height = FMath::Max(Height, 0.0f);
            Height = FMath::Min(Height, 65535.0f); // Max height for 16-bit
            
            FloatHeightData[Index] = Height;
        }
    }
    
    // Convert float heights to 16-bit integers
    for (int32 i = 0; i < FloatHeightData.Num(); i++)
    {
        OutHeightData[i] = FMath::Clamp(FMath::RoundToInt(FloatHeightData[i]), 0, 65535);
    }
    
    // Cache the heightmap data
    CachedHeightData = FloatHeightData;
    HeightmapResolution = Resolution;
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Heightmap generation completed"));
}

void UTerrainGenerator::GenerateTerrainFromProfile(UTerrainProfile* Profile, int32 Resolution, int32 Seed)
{
    if (!Profile)
    {
        UE_LOG(LogTerrainGenerator, Warning, TEXT("No terrain profile provided"));
        return;
    }
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Generating terrain from profile"));
    
    // Initialize heightmap
    TArray<float> HeightData;
    HeightData.SetNum(Resolution * Resolution);
    
    // Initialize with base elevation
    for (int32 i = 0; i < HeightData.Num(); i++)
    {
        HeightData[i] = Profile->BaseElevation;
    }
    
    // Apply noise layers
    for (const FNoiseLayer& NoiseLayer : Profile->NoiseLayers)
    {
        if (NoiseLayer.bEnabled)
        {
            ApplyNoiseLayer(HeightData, NoiseLayer, Resolution, Seed);
        }
    }
    
    // Apply erosion steps
    for (const FErosionParams& ErosionStep : Profile->ErosionSteps)
    {
        ApplyErosion(HeightData, ErosionStep, Resolution);
    }
    
    // Clamp to profile's max elevation
    for (float& Height : HeightData)
    {
        Height = FMath::Clamp(Height, 0.0f, Profile->MaxElevation);
    }
    
    // Cache the result
    CachedHeightData = HeightData;
    HeightmapResolution = Resolution;
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Terrain generation from profile completed"));
}

void UTerrainGenerator::ApplyNoiseLayer(TArray<float>& HeightData, const FNoiseLayer& NoiseLayer, int32 Resolution, int32 Seed)
{
    UE_LOG(LogTerrainGenerator, VeryVerbose, TEXT("Applying noise layer: Type=%d, Frequency=%f, Amplitude=%f"), 
           (int32)NoiseLayer.NoiseType, NoiseLayer.Frequency, NoiseLayer.Amplitude);
    
    for (int32 Y = 0; Y < Resolution; Y++)
    {
        for (int32 X = 0; X < Resolution; X++)
        {
            int32 Index = Y * Resolution + X;
            
            // Convert to world coordinates
            float WorldX = (float(X) / float(Resolution - 1)) * WorldBounds.X - (WorldBounds.X * 0.5f);
            float WorldY = (float(Y) / float(Resolution - 1)) * WorldBounds.Y - (WorldBounds.Y * 0.5f);
            
            // Apply offset and frequency
            float NoiseX = (WorldX + NoiseLayer.Offset.X) * NoiseLayer.Frequency;
            float NoiseY = (WorldY + NoiseLayer.Offset.Y) * NoiseLayer.Frequency;
            
            float NoiseValue = 0.0f;
            
            // Generate noise based on type
            switch (NoiseLayer.NoiseType)
            {
                case ENoiseType::Perlin:
                    NoiseValue = GenerateFractalNoise(NoiseX, NoiseY, NoiseLayer.Octaves, 
                                                    NoiseLayer.Lacunarity, NoiseLayer.Persistence, Seed);
                    break;
                    
                case ENoiseType::Simplex:
                    NoiseValue = GenerateSimplexNoise(NoiseX, NoiseY, Seed);
                    break;
                    
                case ENoiseType::Ridged:
                    NoiseValue = GenerateRidgedNoise(NoiseX, NoiseY, Seed);
                    break;
                    
                case ENoiseType::Voronoi:
                    NoiseValue = GenerateVoronoiNoise(NoiseX, NoiseY, Seed);
                    break;
                    
                default:
                    NoiseValue = GeneratePerlinNoise(NoiseX, NoiseY, Seed);
                    break;
            }
            
            // Apply amplitude and add to height
            HeightData[Index] += NoiseValue * NoiseLayer.Amplitude;
        }
    }
}

void UTerrainGenerator::ApplyErosion(TArray<float>& HeightData, const FErosionParams& ErosionParams, int32 Resolution)
{
    UE_LOG(LogTerrainGenerator, Log, TEXT("Applying erosion: Type=%d, Strength=%f, Iterations=%d"), 
           (int32)ErosionParams.ErosionType, ErosionParams.Strength, ErosionParams.Iterations);
    
    switch (ErosionParams.ErosionType)
    {
        case EErosionType::Hydraulic:
            ApplyHydraulicErosion(HeightData, Resolution, ErosionParams);
            break;
            
        case EErosionType::Thermal:
            ApplyThermalErosion(HeightData, Resolution, ErosionParams);
            break;
            
        default:
            UE_LOG(LogTerrainGenerator, Warning, TEXT("Erosion type not implemented: %d"), (int32)ErosionParams.ErosionType);
            break;
    }
}

float UTerrainGenerator::GeneratePerlinNoise(float X, float Y, int32 Seed)
{
    return PerlinNoise2D(X, Y, Seed);
}

float UTerrainGenerator::GenerateSimplexNoise(float X, float Y, int32 Seed)
{
    return SimplexNoise2D(X, Y, Seed);
}

float UTerrainGenerator::GenerateRidgedNoise(float X, float Y, int32 Seed)
{
    float Noise = FMath::Abs(PerlinNoise2D(X, Y, Seed));
    return 1.0f - (Noise * 2.0f - 1.0f);
}

float UTerrainGenerator::GenerateVoronoiNoise(float X, float Y, int32 Seed)
{
    // Simple Voronoi implementation
    int32 CellX = FMath::FloorToInt(X);
    int32 CellY = FMath::FloorToInt(Y);
    
    float MinDistance = FLT_MAX;
    
    for (int32 OffsetY = -1; OffsetY <= 1; OffsetY++)
    {
        for (int32 OffsetX = -1; OffsetX <= 1; OffsetX++)
        {
            int32 NeighborX = CellX + OffsetX;
            int32 NeighborY = CellY + OffsetY;
            
            // Generate random point in cell
            FRandomStream Random(Seed + NeighborX * 374761393 + NeighborY * 668265263);
            float PointX = NeighborX + Random.FRand();
            float PointY = NeighborY + Random.FRand();
            
            float Distance = FVector2D::Distance(FVector2D(X, Y), FVector2D(PointX, PointY));
            MinDistance = FMath::Min(MinDistance, Distance);
        }
    }
    
    return FMath::Clamp(MinDistance, 0.0f, 1.0f);
}

float UTerrainGenerator::GenerateFractalNoise(float X, float Y, int32 Octaves, float Lacunarity, float Persistence, int32 Seed)
{
    float Result = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = 1.0f;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Result += PerlinNoise2D(X * Frequency, Y * Frequency, Seed + i) * Amplitude;
        MaxValue += Amplitude;
        
        Amplitude *= Persistence;
        Frequency *= Lacunarity;
    }
    
    return Result / MaxValue;
}

void UTerrainGenerator::ApplyHydraulicErosion(TArray<float>& HeightData, int32 Resolution, const FErosionParams& Params)
{
    UE_LOG(LogTerrainGenerator, Log, TEXT("Applying hydraulic erosion with %d iterations"), Params.Iterations);
    
    TArray<float> WaterMap;
    TArray<float> SedimentMap;
    WaterMap.SetNumZeroed(Resolution * Resolution);
    SedimentMap.SetNumZeroed(Resolution * Resolution);
    
    for (int32 Iteration = 0; Iteration < Params.Iterations; Iteration++)
    {
        // Add rain
        for (int32 i = 0; i < WaterMap.Num(); i++)
        {
            WaterMap[i] += Params.RainAmount;
        }
        
        // Simulate water flow and erosion
        for (int32 Y = 1; Y < Resolution - 1; Y++)
        {
            for (int32 X = 1; X < Resolution - 1; X++)
            {
                int32 Index = Y * Resolution + X;
                
                if (WaterMap[Index] <= 0.0f) continue;
                
                // Find steepest descent
                float CurrentHeight = HeightData[Index] + WaterMap[Index];
                float MaxDrop = 0.0f;
                int32 BestNeighbor = Index;
                
                // Check 8 neighbors
                for (int32 OffsetY = -1; OffsetY <= 1; OffsetY++)
                {
                    for (int32 OffsetX = -1; OffsetX <= 1; OffsetX++)
                    {
                        if (OffsetX == 0 && OffsetY == 0) continue;
                        
                        int32 NeighborIndex = (Y + OffsetY) * Resolution + (X + OffsetX);
                        float NeighborHeight = HeightData[NeighborIndex] + WaterMap[NeighborIndex];
                        float Drop = CurrentHeight - NeighborHeight;
                        
                        if (Drop > MaxDrop)
                        {
                            MaxDrop = Drop;
                            BestNeighbor = NeighborIndex;
                        }
                    }
                }
                
                // Flow water and erode
                if (MaxDrop > 0.0f && BestNeighbor != Index)
                {
                    float FlowAmount = FMath::Min(WaterMap[Index], MaxDrop * Params.Strength);
                    WaterMap[Index] -= FlowAmount;
                    WaterMap[BestNeighbor] += FlowAmount;
                    
                    // Erode terrain
                    float ErosionAmount = FlowAmount * Params.SedimentCapacity * 0.1f;
                    HeightData[Index] -= ErosionAmount;
                    SedimentMap[Index] += ErosionAmount;
                    
                    // Deposit sediment
                    float DepositAmount = SedimentMap[Index] * 0.1f;
                    HeightData[BestNeighbor] += DepositAmount;
                    SedimentMap[Index] -= DepositAmount;
                }
            }
        }
        
        // Evaporate water
        for (int32 i = 0; i < WaterMap.Num(); i++)
        {
            WaterMap[i] *= (1.0f - Params.EvaporationRate);
        }
    }
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Hydraulic erosion completed"));
}

void UTerrainGenerator::ApplyThermalErosion(TArray<float>& HeightData, int32 Resolution, const FErosionParams& Params)
{
    UE_LOG(LogTerrainGenerator, Log, TEXT("Applying thermal erosion with angle %f"), Params.ThermalAngle);
    
    float TalusAngle = FMath::Tan(FMath::DegreesToRadians(Params.ThermalAngle));
    
    for (int32 Iteration = 0; Iteration < Params.Iterations; Iteration++)
    {
        TArray<float> DeltaMap;
        DeltaMap.SetNumZeroed(Resolution * Resolution);
        
        for (int32 Y = 1; Y < Resolution - 1; Y++)
        {
            for (int32 X = 1; X < Resolution - 1; X++)
            {
                int32 Index = Y * Resolution + X;
                float CurrentHeight = HeightData[Index];
                
                float TotalDifference = 0.0f;
                int32 NeighborCount = 0;
                
                // Check 4 neighbors
                TArray<int32> Neighbors = {
                    Index - 1,           // Left
                    Index + 1,           // Right
                    Index - Resolution,  // Up
                    Index + Resolution   // Down
                };
                
                for (int32 NeighborIndex : Neighbors)
                {
                    if (NeighborIndex >= 0 && NeighborIndex < HeightData.Num())
                    {
                        float HeightDiff = CurrentHeight - HeightData[NeighborIndex];
                        if (HeightDiff > TalusAngle)
                        {
                            TotalDifference += HeightDiff;
                            NeighborCount++;
                        }
                    }
                }
                
                if (NeighborCount > 0)
                {
                    float MaterialToMove = TotalDifference * Params.Strength * 0.5f;
                    DeltaMap[Index] -= MaterialToMove;
                    
                    // Distribute to neighbors
                    for (int32 NeighborIndex : Neighbors)
                    {
                        if (NeighborIndex >= 0 && NeighborIndex < HeightData.Num())
                        {
                            float HeightDiff = CurrentHeight - HeightData[NeighborIndex];
                            if (HeightDiff > TalusAngle)
                            {
                                DeltaMap[NeighborIndex] += MaterialToMove / NeighborCount;
                            }
                        }
                    }
                }
            }
        }
        
        // Apply changes
        for (int32 i = 0; i < HeightData.Num(); i++)
        {
            HeightData[i] += DeltaMap[i];
        }
    }
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Thermal erosion completed"));
}

ALandscape* UTerrainGenerator::CreateLandscape(const TArray<uint16>& HeightData, int32 Resolution, const FVector& Location, const FVector& Scale)
{
    UE_LOG(LogTerrainGenerator, Log, TEXT("Creating landscape at location %s with scale %s"), 
           *Location.ToString(), *Scale.ToString());
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTerrainGenerator, Error, TEXT("No valid world found"));
        return nullptr;
    }
    
    // Create landscape actor
    ALandscape* Landscape = World->SpawnActor<ALandscape>(Location, FRotator::ZeroRotator);
    if (!Landscape)
    {
        UE_LOG(LogTerrainGenerator, Error, TEXT("Failed to spawn landscape actor"));
        return nullptr;
    }
    
    // Set landscape scale
    Landscape->SetActorScale3D(Scale);
    
    // Store reference
    GeneratedLandscape = Landscape;
    
    UE_LOG(LogTerrainGenerator, Log, TEXT("Landscape created successfully"));
    return Landscape;
}

float UTerrainGenerator::GetHeightAtPosition(const FVector& WorldPosition) const
{
    if (CachedHeightData.Num() == 0 || HeightmapResolution == 0)
    {
        return 0.0f;
    }
    
    // Convert world position to heightmap coordinates
    float NormalizedX = (WorldPosition.X + WorldBounds.X * 0.5f) / WorldBounds.X;
    float NormalizedY = (WorldPosition.Y + WorldBounds.Y * 0.5f) / WorldBounds.Y;
    
    // Clamp to valid range
    NormalizedX = FMath::Clamp(NormalizedX, 0.0f, 1.0f);
    NormalizedY = FMath::Clamp(NormalizedY, 0.0f, 1.0f);
    
    // Convert to heightmap indices
    float FloatX = NormalizedX * (HeightmapResolution - 1);
    float FloatY = NormalizedY * (HeightmapResolution - 1);
    
    int32 X = FMath::FloorToInt(FloatX);
    int32 Y = FMath::FloorToInt(FloatY);
    
    // Bilinear interpolation
    float FracX = FloatX - X;
    float FracY = FloatY - Y;
    
    int32 X1 = FMath::Min(X + 1, HeightmapResolution - 1);
    int32 Y1 = FMath::Min(Y + 1, HeightmapResolution - 1);
    
    float Height00 = CachedHeightData[Y * HeightmapResolution + X];
    float Height10 = CachedHeightData[Y * HeightmapResolution + X1];
    float Height01 = CachedHeightData[Y1 * HeightmapResolution + X];
    float Height11 = CachedHeightData[Y1 * HeightmapResolution + X1];
    
    float HeightX0 = FMath::Lerp(Height00, Height10, FracX);
    float HeightX1 = FMath::Lerp(Height01, Height11, FracX);
    
    return FMath::Lerp(HeightX0, HeightX1, FracY);
}

float UTerrainGenerator::GetSlopeAtPosition(const FVector& WorldPosition) const
{
    if (CachedHeightData.Num() == 0 || HeightmapResolution == 0)
    {
        return 0.0f;
    }
    
    // Sample heights around the position
    float SampleDistance = 100.0f; // 1 meter
    float HeightCenter = GetHeightAtPosition(WorldPosition);
    float HeightRight = GetHeightAtPosition(WorldPosition + FVector(SampleDistance, 0, 0));
    float HeightUp = GetHeightAtPosition(WorldPosition + FVector(0, SampleDistance, 0));
    
    // Calculate gradients
    float GradientX = (HeightRight - HeightCenter) / SampleDistance;
    float GradientY = (HeightUp - HeightCenter) / SampleDistance;
    
    // Calculate slope angle in degrees
    float SlopeRadians = FMath::Atan(FMath::Sqrt(GradientX * GradientX + GradientY * GradientY));
    return FMath::RadiansToDegrees(SlopeRadians);
}

FVector UTerrainGenerator::GetNormalAtPosition(const FVector& WorldPosition) const
{
    if (CachedHeightData.Num() == 0 || HeightmapResolution == 0)
    {
        return FVector::UpVector;
    }
    
    // Sample heights around the position
    float SampleDistance = 100.0f; // 1 meter
    float HeightCenter = GetHeightAtPosition(WorldPosition);
    float HeightRight = GetHeightAtPosition(WorldPosition + FVector(SampleDistance, 0, 0));
    float HeightUp = GetHeightAtPosition(WorldPosition + FVector(0, SampleDistance, 0));
    float HeightLeft = GetHeightAtPosition(WorldPosition + FVector(-SampleDistance, 0, 0));
    float HeightDown = GetHeightAtPosition(WorldPosition + FVector(0, -SampleDistance, 0));
    
    // Calculate gradients
    float GradientX = (HeightRight - HeightLeft) / (2.0f * SampleDistance);
    float GradientY = (HeightUp - HeightDown) / (2.0f * SampleDistance);
    
    // Calculate normal vector
    FVector Normal(-GradientX, -GradientY, 1.0f);
    return Normal.GetSafeNormal();
}

// Static noise functions implementation
float UTerrainGenerator::PerlinNoise2D(float X, float Y, int32 Seed)
{
    // Simple Perlin noise implementation
    int32 Xi = FMath::FloorToInt(X) & 255;
    int32 Yi = FMath::FloorToInt(Y) & 255;
    
    float Xf = X - FMath::FloorToFloat(X);
    float Yf = Y - FMath::FloorToFloat(Y);
    
    // Fade curves
    float U = Xf * Xf * (3.0f - 2.0f * Xf);
    float V = Yf * Yf * (3.0f - 2.0f * Yf);
    
    // Hash coordinates
    FRandomStream Random(Seed);
    Random.Initialize(Seed + Xi * 374761393 + Yi * 668265263);
    
    float A = Random.FRand() * 2.0f - 1.0f;
    Random.Initialize(Seed + (Xi + 1) * 374761393 + Yi * 668265263);
    float B = Random.FRand() * 2.0f - 1.0f;
    Random.Initialize(Seed + Xi * 374761393 + (Yi + 1) * 668265263);
    float C = Random.FRand() * 2.0f - 1.0f;
    Random.Initialize(Seed + (Xi + 1) * 374761393 + (Yi + 1) * 668265263);
    float D = Random.FRand() * 2.0f - 1.0f;
    
    // Interpolate
    float X1 = FMath::Lerp(A, B, U);
    float X2 = FMath::Lerp(C, D, U);
    
    return FMath::Lerp(X1, X2, V);
}

float UTerrainGenerator::SimplexNoise2D(float X, float Y, int32 Seed)
{
    // Simplified simplex noise - using Perlin for now
    return PerlinNoise2D(X * 1.414f, Y * 1.414f, Seed) * 0.707f;
}
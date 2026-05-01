#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    // Initialize default biome parameters
    WorldSizeKm = 16.0f;
    BiomeNoiseScale = 0.001f;
    TransitionZoneSize = 500.0f;
    bEnableBiomeBlending = true;
    BiomeUpdateFrequency = 1.0f;
    LastCacheUpdate = 0.0f;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system..."));
    
    // Initialize default biome parameters
    FEng_BiomeParameters GrasslandParams;
    GrasslandParams.BiomeType = EEng_BiomeType::Grassland;
    GrasslandParams.Temperature = 22.0f;
    GrasslandParams.Humidity = 0.4f;
    GrasslandParams.VegetationDensity = 0.3f;
    GrasslandParams.RockDensity = 0.1f;
    GrasslandParams.WaterLevel = 0.0f;
    GrasslandParams.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    GrasslandParams.FogDensity = 0.01f;
    BiomeParametersMap.Add(EEng_BiomeType::Grassland, GrasslandParams);
    
    FEng_BiomeParameters ForestParams;
    ForestParams.BiomeType = EEng_BiomeType::Forest;
    ForestParams.Temperature = 18.0f;
    ForestParams.Humidity = 0.8f;
    ForestParams.VegetationDensity = 0.9f;
    ForestParams.RockDensity = 0.2f;
    ForestParams.WaterLevel = 0.0f;
    ForestParams.FogColor = FLinearColor(0.4f, 0.6f, 0.5f, 1.0f);
    ForestParams.FogDensity = 0.03f;
    BiomeParametersMap.Add(EEng_BiomeType::Forest, ForestParams);
    
    FEng_BiomeParameters DesertParams;
    DesertParams.BiomeType = EEng_BiomeType::Desert;
    DesertParams.Temperature = 35.0f;
    DesertParams.Humidity = 0.1f;
    DesertParams.VegetationDensity = 0.05f;
    DesertParams.RockDensity = 0.4f;
    DesertParams.WaterLevel = 0.0f;
    DesertParams.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DesertParams.FogDensity = 0.005f;
    BiomeParametersMap.Add(EEng_BiomeType::Desert, DesertParams);
    
    FEng_BiomeParameters SwampParams;
    SwampParams.BiomeType = EEng_BiomeType::Swamp;
    SwampParams.Temperature = 25.0f;
    SwampParams.Humidity = 0.95f;
    SwampParams.VegetationDensity = 0.7f;
    SwampParams.RockDensity = 0.1f;
    SwampParams.WaterLevel = 50.0f;
    SwampParams.FogColor = FLinearColor(0.5f, 0.6f, 0.4f, 1.0f);
    SwampParams.FogDensity = 0.05f;
    BiomeParametersMap.Add(EEng_BiomeType::Swamp, SwampParams);
    
    FEng_BiomeParameters MountainParams;
    MountainParams.BiomeType = EEng_BiomeType::Mountains;
    MountainParams.Temperature = 5.0f;
    MountainParams.Humidity = 0.3f;
    MountainParams.VegetationDensity = 0.2f;
    MountainParams.RockDensity = 0.8f;
    MountainParams.WaterLevel = 0.0f;
    MountainParams.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    MountainParams.FogDensity = 0.02f;
    BiomeParametersMap.Add(EEng_BiomeType::Mountains, MountainParams);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d biome types"), BiomeParametersMap.Num());
}

void UEng_BiomeManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Shutting down biome system"));
    BiomeParametersMap.Empty();
    BiomeTransitions.Empty();
    BiomeCache.Empty();
    Super::Deinitialize();
}

EEng_BiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to grid coordinates
    FIntPoint GridCoord = FIntPoint(
        FMath::FloorToInt(WorldLocation.X / 1000.0f),
        FMath::FloorToInt(WorldLocation.Y / 1000.0f)
    );
    
    // Check cache first
    if (BiomeCache.Contains(GridCoord))
    {
        return BiomeCache[GridCoord];
    }
    
    // Calculate noise value for biome determination
    float NoiseValue = CalculateNoiseValue(WorldLocation, BiomeNoiseScale);
    float Elevation = WorldLocation.Z;
    
    EEng_BiomeType BiomeType = DetermineBiomeFromNoise(NoiseValue, Elevation);
    
    // Cache the result
    BiomeCache.Add(GridCoord, BiomeType);
    
    return BiomeType;
}

FEng_BiomeParameters UEng_BiomeManager::GetBiomeParameters(EEng_BiomeType BiomeType) const
{
    if (BiomeParametersMap.Contains(BiomeType))
    {
        return BiomeParametersMap[BiomeType];
    }
    
    // Return default grassland parameters if not found
    FEng_BiomeParameters DefaultParams;
    return DefaultParams;
}

FEng_BiomeParameters UEng_BiomeManager::GetBlendedBiomeParameters(const FVector& WorldLocation) const
{
    if (!bEnableBiomeBlending)
    {
        EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
        return GetBiomeParameters(BiomeType);
    }
    
    // Sample multiple points around the location for blending
    TArray<FVector> SamplePoints = {
        WorldLocation,
        WorldLocation + FVector(TransitionZoneSize, 0, 0),
        WorldLocation + FVector(-TransitionZoneSize, 0, 0),
        WorldLocation + FVector(0, TransitionZoneSize, 0),
        WorldLocation + FVector(0, -TransitionZoneSize, 0)
    };
    
    TMap<EEng_BiomeType, float> BiomeWeights;
    float TotalWeight = 0.0f;
    
    for (const FVector& SamplePoint : SamplePoints)
    {
        EEng_BiomeType BiomeType = GetBiomeAtLocation(SamplePoint);
        float Distance = FVector::Dist(WorldLocation, SamplePoint);
        float Weight = FMath::Max(0.1f, 1.0f - (Distance / TransitionZoneSize));
        
        if (BiomeWeights.Contains(BiomeType))
        {
            BiomeWeights[BiomeType] += Weight;
        }
        else
        {
            BiomeWeights.Add(BiomeType, Weight);
        }
        TotalWeight += Weight;
    }
    
    // Blend parameters based on weights
    FEng_BiomeParameters BlendedParams;
    bool bFirstBiome = true;
    
    for (const auto& BiomeWeight : BiomeWeights)
    {
        FEng_BiomeParameters BiomeParams = GetBiomeParameters(BiomeWeight.Key);
        float NormalizedWeight = BiomeWeight.Value / TotalWeight;
        
        if (bFirstBiome)
        {
            BlendedParams = BiomeParams;
            BlendedParams.Temperature *= NormalizedWeight;
            BlendedParams.Humidity *= NormalizedWeight;
            BlendedParams.VegetationDensity *= NormalizedWeight;
            BlendedParams.RockDensity *= NormalizedWeight;
            BlendedParams.WaterLevel *= NormalizedWeight;
            BlendedParams.FogDensity *= NormalizedWeight;
            bFirstBiome = false;
        }
        else
        {
            BlendedParams.Temperature += BiomeParams.Temperature * NormalizedWeight;
            BlendedParams.Humidity += BiomeParams.Humidity * NormalizedWeight;
            BlendedParams.VegetationDensity += BiomeParams.VegetationDensity * NormalizedWeight;
            BlendedParams.RockDensity += BiomeParams.RockDensity * NormalizedWeight;
            BlendedParams.WaterLevel += BiomeParams.WaterLevel * NormalizedWeight;
            BlendedParams.FogDensity += BiomeParams.FogDensity * NormalizedWeight;
        }
    }
    
    return BlendedParams;
}

void UEng_BiomeManager::SetBiomeParameters(EEng_BiomeType BiomeType, const FEng_BiomeParameters& Parameters)
{
    BiomeParametersMap.Add(BiomeType, Parameters);
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Updated parameters for biome type %d"), (int32)BiomeType);
}

bool UEng_BiomeManager::IsInTransitionZone(const FVector& WorldLocation) const
{
    // Sample nearby locations to detect biome changes
    EEng_BiomeType CenterBiome = GetBiomeAtLocation(WorldLocation);
    
    TArray<FVector> CheckPoints = {
        WorldLocation + FVector(TransitionZoneSize * 0.5f, 0, 0),
        WorldLocation + FVector(-TransitionZoneSize * 0.5f, 0, 0),
        WorldLocation + FVector(0, TransitionZoneSize * 0.5f, 0),
        WorldLocation + FVector(0, -TransitionZoneSize * 0.5f, 0)
    };
    
    for (const FVector& CheckPoint : CheckPoints)
    {
        if (GetBiomeAtLocation(CheckPoint) != CenterBiome)
        {
            return true;
        }
    }
    
    return false;
}

FEng_BiomeTransition UEng_BiomeManager::GetTransitionData(const FVector& WorldLocation) const
{
    FEng_BiomeTransition TransitionData;
    
    EEng_BiomeType CenterBiome = GetBiomeAtLocation(WorldLocation);
    TransitionData.FromBiome = CenterBiome;
    
    // Find the nearest different biome
    float MinDistance = FLT_MAX;
    EEng_BiomeType NearestDifferentBiome = CenterBiome;
    
    for (float Angle = 0.0f; Angle < 360.0f; Angle += 45.0f)
    {
        FVector Direction = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)),
            FMath::Sin(FMath::DegreesToRadians(Angle)),
            0.0f
        );
        
        for (float Distance = 100.0f; Distance <= TransitionZoneSize; Distance += 100.0f)
        {
            FVector TestLocation = WorldLocation + Direction * Distance;
            EEng_BiomeType TestBiome = GetBiomeAtLocation(TestLocation);
            
            if (TestBiome != CenterBiome && Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestDifferentBiome = TestBiome;
                break;
            }
        }
    }
    
    TransitionData.ToBiome = NearestDifferentBiome;
    TransitionData.TransitionDistance = MinDistance;
    TransitionData.BlendFactor = FMath::Clamp(MinDistance / TransitionZoneSize, 0.0f, 1.0f);
    
    return TransitionData;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeParameters Params = GetBlendedBiomeParameters(WorldLocation);
    
    // Add elevation-based temperature variation
    float ElevationEffect = -WorldLocation.Z * 0.01f; // -1°C per 100m elevation
    
    return Params.Temperature + ElevationEffect;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeParameters Params = GetBlendedBiomeParameters(WorldLocation);
    return Params.Humidity;
}

float UEng_BiomeManager::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeParameters Params = GetBlendedBiomeParameters(WorldLocation);
    return Params.VegetationDensity;
}

void UEng_BiomeManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validating biome configuration..."));
    
    int32 ValidBiomes = 0;
    for (const auto& BiomeEntry : BiomeParametersMap)
    {
        const FEng_BiomeParameters& Params = BiomeEntry.Value;
        
        if (Params.Temperature >= -50.0f && Params.Temperature <= 50.0f &&
            Params.Humidity >= 0.0f && Params.Humidity <= 1.0f &&
            Params.VegetationDensity >= 0.0f && Params.VegetationDensity <= 1.0f)
        {
            ValidBiomes++;
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid parameters for biome %d"), (int32)BiomeEntry.Key);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validation complete. %d/%d biomes valid"), ValidBiomes, BiomeParametersMap.Num());
}

void UEng_BiomeManager::GenerateDebugBiomeMap()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generating debug biome map..."));
    
    // Clear existing cache to force regeneration
    BiomeCache.Empty();
    
    // Sample a grid of locations and log biome types
    for (int32 X = -5; X <= 5; X++)
    {
        for (int32 Y = -5; Y <= 5; Y++)
        {
            FVector TestLocation = FVector(X * 1000.0f, Y * 1000.0f, 0.0f);
            EEng_BiomeType BiomeType = GetBiomeAtLocation(TestLocation);
            
            UE_LOG(LogTemp, Warning, TEXT("BiomeMap: (%d,%d) = Biome %d"), X, Y, (int32)BiomeType);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Debug map generation complete"));
}

float UEng_BiomeManager::CalculateNoiseValue(const FVector& Location, float Scale) const
{
    // Simple Perlin-like noise implementation
    float X = Location.X * Scale;
    float Y = Location.Y * Scale;
    
    // Basic noise function using sine waves
    float Noise1 = FMath::Sin(X * 0.1f) * FMath::Cos(Y * 0.1f);
    float Noise2 = FMath::Sin(X * 0.05f) * FMath::Cos(Y * 0.05f) * 0.5f;
    float Noise3 = FMath::Sin(X * 0.02f) * FMath::Cos(Y * 0.02f) * 0.25f;
    
    return (Noise1 + Noise2 + Noise3) / 1.75f;
}

EEng_BiomeType UEng_BiomeManager::DetermineBiomeFromNoise(float NoiseValue, float Elevation) const
{
    // Elevation-based biome selection
    if (Elevation > 500.0f)
    {
        return EEng_BiomeType::Mountains;
    }
    else if (Elevation < -50.0f)
    {
        return EEng_BiomeType::Swamp;
    }
    
    // Noise-based biome selection for normal elevations
    if (NoiseValue < -0.5f)
    {
        return EEng_BiomeType::Desert;
    }
    else if (NoiseValue < -0.2f)
    {
        return EEng_BiomeType::Grassland;
    }
    else if (NoiseValue < 0.2f)
    {
        return EEng_BiomeType::Forest;
    }
    else if (NoiseValue < 0.5f)
    {
        return EEng_BiomeType::Swamp;
    }
    else
    {
        return EEng_BiomeType::Lake;
    }
}

FEng_BiomeParameters UEng_BiomeManager::BlendBiomeParameters(const FEng_BiomeParameters& BiomeA, const FEng_BiomeParameters& BiomeB, float BlendFactor) const
{
    FEng_BiomeParameters BlendedParams;
    
    BlendedParams.Temperature = FMath::Lerp(BiomeA.Temperature, BiomeB.Temperature, BlendFactor);
    BlendedParams.Humidity = FMath::Lerp(BiomeA.Humidity, BiomeB.Humidity, BlendFactor);
    BlendedParams.VegetationDensity = FMath::Lerp(BiomeA.VegetationDensity, BiomeB.VegetationDensity, BlendFactor);
    BlendedParams.RockDensity = FMath::Lerp(BiomeA.RockDensity, BiomeB.RockDensity, BlendFactor);
    BlendedParams.WaterLevel = FMath::Lerp(BiomeA.WaterLevel, BiomeB.WaterLevel, BlendFactor);
    BlendedParams.FogDensity = FMath::Lerp(BiomeA.FogDensity, BiomeB.FogDensity, BlendFactor);
    
    // Blend colors
    BlendedParams.FogColor = FLinearColor(
        FMath::Lerp(BiomeA.FogColor.R, BiomeB.FogColor.R, BlendFactor),
        FMath::Lerp(BiomeA.FogColor.G, BiomeB.FogColor.G, BlendFactor),
        FMath::Lerp(BiomeA.FogColor.B, BiomeB.FogColor.B, BlendFactor),
        FMath::Lerp(BiomeA.FogColor.A, BiomeB.FogColor.A, BlendFactor)
    );
    
    // Choose dominant biome type
    BlendedParams.BiomeType = (BlendFactor < 0.5f) ? BiomeA.BiomeType : BiomeB.BiomeType;
    
    return BlendedParams;
}
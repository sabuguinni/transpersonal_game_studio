#include "BiomeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "PCGSubsystem.h"

UBiomeGeneratorComponent::UBiomeGeneratorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Set default biome configurations
    CachedBiomeData.Empty();
    
    // Forest biome
    FBiomeData ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.Density = 0.8f;
    ForestBiome.MinElevation = 50.0f;
    ForestBiome.MaxElevation = 800.0f;
    ForestBiome.WaterDistance = 200.0f;
    ForestBiome.Temperature = 22.0f;
    ForestBiome.Humidity = 0.7f;
    CachedBiomeData.Add(ForestBiome);
    
    // Grassland biome
    FBiomeData GrasslandBiome;
    GrasslandBiome.BiomeType = EBiomeType::Grassland;
    GrasslandBiome.Density = 0.4f;
    GrasslandBiome.MinElevation = 0.0f;
    GrasslandBiome.MaxElevation = 300.0f;
    GrasslandBiome.WaterDistance = 1000.0f;
    GrasslandBiome.Temperature = 25.0f;
    GrasslandBiome.Humidity = 0.4f;
    CachedBiomeData.Add(GrasslandBiome);
    
    // Swampland biome
    FBiomeData SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swampland;
    SwampBiome.Density = 0.9f;
    SwampBiome.MinElevation = -10.0f;
    SwampBiome.MaxElevation = 50.0f;
    SwampBiome.WaterDistance = 50.0f;
    SwampBiome.Temperature = 28.0f;
    SwampBiome.Humidity = 0.9f;
    CachedBiomeData.Add(SwampBiome);
    
    // Mountain biome
    FBiomeData MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.Density = 0.2f;
    MountainBiome.MinElevation = 800.0f;
    MountainBiome.MaxElevation = 2000.0f;
    MountainBiome.WaterDistance = 2000.0f;
    MountainBiome.Temperature = 15.0f;
    MountainBiome.Humidity = 0.3f;
    CachedBiomeData.Add(MountainBiome);
    
    // River biome
    FBiomeData RiverBiome;
    RiverBiome.BiomeType = EBiomeType::River;
    RiverBiome.Density = 0.6f;
    RiverBiome.MinElevation = -5.0f;
    RiverBiome.MaxElevation = 100.0f;
    RiverBiome.WaterDistance = 0.0f;
    RiverBiome.Temperature = 20.0f;
    RiverBiome.Humidity = 1.0f;
    CachedBiomeData.Add(RiverBiome);
    
    // Volcanic biome
    FBiomeData VolcanicBiome;
    VolcanicBiome.BiomeType = EBiomeType::Volcanic;
    VolcanicBiome.Density = 0.1f;
    VolcanicBiome.MinElevation = 1000.0f;
    VolcanicBiome.MaxElevation = 3000.0f;
    VolcanicBiome.WaterDistance = 5000.0f;
    VolcanicBiome.Temperature = 35.0f;
    VolcanicBiome.Humidity = 0.1f;
    CachedBiomeData.Add(VolcanicBiome);
}

void UBiomeGeneratorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Load biome data asset if specified
    if (BiomeData.IsValid())
    {
        UBiomeDataAsset* LoadedBiomeData = BiomeData.LoadSynchronous();
        if (LoadedBiomeData && LoadedBiomeData->BiomeConfigurations.Num() > 0)
        {
            CachedBiomeData = LoadedBiomeData->BiomeConfigurations;
            UE_LOG(LogTemp, Log, TEXT("BiomeGenerator: Loaded %d biome configurations"), CachedBiomeData.Num());
        }
    }
    
    // Auto-generate biomes on start
    GenerateBiomes();
}

void UBiomeGeneratorComponent::GenerateBiomes()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeGenerator: No valid world found"));
        return;
    }
    
    UPCGSubsystem* PCGSubsystem = UPCGSubsystem::GetInstance(World);
    if (!PCGSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("BiomeGenerator: PCG Subsystem not available"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeGenerator: Starting biome generation with %d biome types"), CachedBiomeData.Num());
    
    // Trigger PCG generation
    if (GetGraph())
    {
        PCGSubsystem->FlushCache();
        Generate();
        UE_LOG(LogTemp, Log, TEXT("BiomeGenerator: PCG generation triggered"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeGenerator: No PCG graph assigned"));
    }
}

EBiomeType UBiomeGeneratorComponent::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Convert world location to normalized coordinates
    float X = WorldLocation.X * 0.001f; // Scale down for noise sampling
    float Y = WorldLocation.Y * 0.001f;
    
    // Get elevation
    float Elevation = GetElevationAtLocation(WorldLocation);
    
    // Generate temperature based on latitude and elevation
    float Temperature = 30.0f - (FMath::Abs(Y) * 0.1f) - (Elevation * 0.01f);
    
    // Generate humidity based on noise and water distance
    float Humidity = GeneratePerlinNoise(X * 2.0f, Y * 2.0f, 0.05f, 3, 0.6f);
    
    // Calculate distance to nearest water (simplified)
    float WaterDistance = FMath::Max(0.0f, Elevation + 10.0f); // Approximate water distance
    
    return DetermineBiomeType(Elevation, Temperature, Humidity, WaterDistance);
}

float UBiomeGeneratorComponent::GetElevationAtLocation(FVector WorldLocation) const
{
    float X = WorldLocation.X * 0.001f;
    float Y = WorldLocation.Y * 0.001f;
    
    // Generate base terrain with multiple octaves of noise
    float BaseElevation = GeneratePerlinNoise(X, Y, 0.01f, 6, 0.5f) * 1000.0f;
    
    // Add mountain ridges
    float MountainNoise = GeneratePerlinNoise(X * 0.5f, Y * 0.5f, 0.005f, 4, 0.7f);
    if (MountainNoise > 0.6f)
    {
        BaseElevation += (MountainNoise - 0.6f) * 2000.0f;
    }
    
    // Add river valleys (negative elevation in certain areas)
    float RiverNoise = GeneratePerlinNoise(X * 3.0f, Y * 3.0f, 0.02f, 2, 0.3f);
    if (RiverNoise < 0.3f)
    {
        BaseElevation -= (0.3f - RiverNoise) * 200.0f;
    }
    
    return FMath::Max(-50.0f, BaseElevation); // Clamp minimum elevation
}

FBiomeData UBiomeGeneratorComponent::GetBiomeData(EBiomeType BiomeType) const
{
    for (const FBiomeData& BiomeData : CachedBiomeData)
    {
        if (BiomeData.BiomeType == BiomeType)
        {
            return BiomeData;
        }
    }
    
    // Return default forest biome if not found
    FBiomeData DefaultBiome;
    DefaultBiome.BiomeType = EBiomeType::Forest;
    return DefaultBiome;
}

float UBiomeGeneratorComponent::GeneratePerlinNoise(float X, float Y, float Scale, int32 Octaves, float Persistence) const
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    float MaxValue = 0.0f;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        MaxValue += Amplitude;
        Amplitude *= Persistence;
        Frequency *= 2.0f;
    }
    
    return Value / MaxValue;
}

EBiomeType UBiomeGeneratorComponent::DetermineBiomeType(float Elevation, float Temperature, float Humidity, float WaterDistance) const
{
    // River biome - near water at low elevation
    if (WaterDistance < 100.0f && Elevation < 50.0f)
    {
        return EBiomeType::River;
    }
    
    // Swampland - low elevation, high humidity, near water
    if (Elevation < 100.0f && Humidity > 0.7f && WaterDistance < 300.0f)
    {
        return EBiomeType::Swampland;
    }
    
    // Volcanic - very high elevation, high temperature
    if (Elevation > 1500.0f && Temperature > 30.0f)
    {
        return EBiomeType::Volcanic;
    }
    
    // Mountain - high elevation
    if (Elevation > 800.0f)
    {
        return EBiomeType::Mountain;
    }
    
    // Forest - moderate elevation, good humidity
    if (Elevation > 50.0f && Elevation < 800.0f && Humidity > 0.5f)
    {
        return EBiomeType::Forest;
    }
    
    // Default to grassland
    return EBiomeType::Grassland;
}
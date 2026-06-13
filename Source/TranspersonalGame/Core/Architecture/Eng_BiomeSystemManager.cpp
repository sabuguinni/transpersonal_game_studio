#include "Eng_BiomeSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape/Landscape.h"

UEng_BiomeSystemManager::UEng_BiomeSystemManager()
{
    BiomeCacheResolution = 100;
    BiomeUpdateFrequency = 5.0f;
    bEnableBiomeDebugDraw = false;
}

void UEng_BiomeSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initializing biome system"));
    
    InitializeDefaultBiomeConfigurations();
    CacheBiomeData();
    
    // Set up periodic biome cache updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BiomeUpdateTimer,
            this,
            &UEng_BiomeSystemManager::UpdateBiomeCache,
            BiomeUpdateFrequency,
            true
        );
    }
}

void UEng_BiomeSystemManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BiomeUpdateTimer);
    }
    
    BiomeCache.Empty();
    Super::Deinitialize();
}

EBiomeType UEng_BiomeSystemManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to cache coordinates
    FIntPoint CacheCoord(
        FMath::FloorToInt(WorldLocation.X / BiomeCacheResolution),
        FMath::FloorToInt(WorldLocation.Y / BiomeCacheResolution)
    );
    
    // Check cache first
    if (const EBiomeType* CachedBiome = BiomeCache.Find(CacheCoord))
    {
        return *CachedBiome;
    }
    
    // Calculate biome if not cached
    return CalculateBiomeFromEnvironment(WorldLocation);
}

FEng_BiomeConfiguration UEng_BiomeSystemManager::GetBiomeConfiguration(EBiomeType BiomeType) const
{
    if (const FEng_BiomeConfiguration* Config = BiomeConfigurations.Find(BiomeType))
    {
        return *Config;
    }
    
    // Return default configuration if not found
    return FEng_BiomeConfiguration();
}

void UEng_BiomeSystemManager::SetBiomeConfiguration(EBiomeType BiomeType, const FEng_BiomeConfiguration& Configuration)
{
    BiomeConfigurations.Add(BiomeType, Configuration);
    
    // Update cache after configuration change
    CacheBiomeData();
}

TArray<EBiomeType> UEng_BiomeSystemManager::GetAllBiomeTypes() const
{
    TArray<EBiomeType> BiomeTypes;
    BiomeConfigurations.GetKeys(BiomeTypes);
    return BiomeTypes;
}

float UEng_BiomeSystemManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    
    // Add some variation based on elevation and latitude
    float ElevationModifier = -WorldLocation.Z * 0.01f; // Higher = colder
    float LatitudeModifier = FMath::Abs(WorldLocation.Y) * 0.001f; // Further from equator = colder
    
    return Config.Temperature + ElevationModifier + LatitudeModifier;
}

float UEng_BiomeSystemManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    
    // Add variation based on proximity to water bodies
    // This is simplified - in a full implementation, we'd check for rivers/lakes
    float WaterProximityModifier = FMath::Sin(WorldLocation.X * 0.001f) * 10.0f;
    
    return FMath::Clamp(Config.Humidity + WaterProximityModifier, 0.0f, 100.0f);
}

bool UEng_BiomeSystemManager::IsLocationSuitableForDinosaur(const FVector& WorldLocation, TSubclassOf<APawn> DinosaurClass) const
{
    if (!DinosaurClass)
    {
        return false;
    }
    
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfiguration Config = GetBiomeConfiguration(BiomeType);
    
    return Config.AllowedDinosaurs.Contains(DinosaurClass);
}

void UEng_BiomeSystemManager::AddBiomeTransition(const FEng_BiomeTransition& Transition)
{
    BiomeTransitions.Add(Transition);
}

FEng_BiomeTransition UEng_BiomeSystemManager::GetBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const
{
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == FromBiome && Transition.ToBiome == ToBiome)
        {
            return Transition;
        }
    }
    
    // Return default transition if not found
    FEng_BiomeTransition DefaultTransition;
    DefaultTransition.FromBiome = FromBiome;
    DefaultTransition.ToBiome = ToBiome;
    return DefaultTransition;
}

float UEng_BiomeSystemManager::GetBiomeBlendFactor(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const
{
    FEng_BiomeTransition Transition = GetBiomeTransition(BiomeA, BiomeB);
    
    // Calculate distance-based blend factor
    // This is simplified - in practice, you'd use more sophisticated blending
    float DistanceFromCenter = FVector::Dist2D(WorldLocation, FVector::ZeroVector);
    float BlendDistance = Transition.TransitionDistance;
    
    if (BlendDistance <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(DistanceFromCenter / BlendDistance, 0.0f, 1.0f);
}

void UEng_BiomeSystemManager::GenerateBiomeMap(int32 WorldSizeKm)
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Generating biome map for %d km world"), WorldSizeKm);
    
    // Clear existing cache
    BiomeCache.Empty();
    
    // Generate biome map based on world size
    float WorldSizeCm = WorldSizeKm * 100000.0f; // Convert km to cm
    int32 GridSize = WorldSizeCm / BiomeCacheResolution;
    
    for (int32 X = -GridSize/2; X < GridSize/2; X++)
    {
        for (int32 Y = -GridSize/2; Y < GridSize/2; Y++)
        {
            FVector WorldLocation(X * BiomeCacheResolution, Y * BiomeCacheResolution, 0.0f);
            EBiomeType BiomeType = CalculateBiomeFromEnvironment(WorldLocation);
            
            FIntPoint CacheCoord(X, Y);
            BiomeCache.Add(CacheCoord, BiomeType);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Generated %d biome cache entries"), BiomeCache.Num());
}

void UEng_BiomeSystemManager::ApplyBiomeToRegion(const FVector& CenterLocation, float Radius, EBiomeType BiomeType)
{
    int32 GridRadius = FMath::CeilToInt(Radius / BiomeCacheResolution);
    FIntPoint CenterCoord(
        FMath::FloorToInt(CenterLocation.X / BiomeCacheResolution),
        FMath::FloorToInt(CenterLocation.Y / BiomeCacheResolution)
    );
    
    for (int32 X = -GridRadius; X <= GridRadius; X++)
    {
        for (int32 Y = -GridRadius; Y <= GridRadius; Y++)
        {
            FIntPoint CacheCoord(CenterCoord.X + X, CenterCoord.Y + Y);
            FVector WorldPos(CacheCoord.X * BiomeCacheResolution, CacheCoord.Y * BiomeCacheResolution, 0.0f);
            
            float Distance = FVector::Dist2D(WorldPos, CenterLocation);
            if (Distance <= Radius)
            {
                BiomeCache.Add(CacheCoord, BiomeType);
            }
        }
    }
}

void UEng_BiomeSystemManager::UpdateBiomeCache()
{
    // Periodic cache update - could be optimized to only update changed areas
    if (BiomeCache.Num() > 10000) // Only update if cache is large
    {
        UE_LOG(LogTemp, Log, TEXT("BiomeSystemManager: Updating biome cache (%d entries)"), BiomeCache.Num());
    }
}

void UEng_BiomeSystemManager::ClearBiomeCache()
{
    BiomeCache.Empty();
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Biome cache cleared"));
}

void UEng_BiomeSystemManager::DebugDrawBiomeBoundaries()
{
    if (!bEnableBiomeDebugDraw)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Draw biome boundaries for debugging
    for (const auto& CacheEntry : BiomeCache)
    {
        FVector WorldPos(CacheEntry.Key.X * BiomeCacheResolution, CacheEntry.Key.Y * BiomeCacheResolution, 100.0f);
        FColor BiomeColor = FColor::White;
        
        // Assign colors based on biome type
        switch (CacheEntry.Value)
        {
            case EBiomeType::Forest:
                BiomeColor = FColor::Green;
                break;
            case EBiomeType::Desert:
                BiomeColor = FColor::Yellow;
                break;
            case EBiomeType::Swamp:
                BiomeColor = FColor::Cyan;
                break;
            case EBiomeType::Mountains:
                BiomeColor = FColor::Magenta;
                break;
            case EBiomeType::Plains:
                BiomeColor = FColor::Orange;
                break;
            default:
                BiomeColor = FColor::White;
                break;
        }
        
        DrawDebugBox(World, WorldPos, FVector(BiomeCacheResolution/2), BiomeColor, false, 1.0f);
    }
}

void UEng_BiomeSystemManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Validating biome configurations"));
    
    int32 ValidConfigs = 0;
    for (const auto& ConfigPair : BiomeConfigurations)
    {
        const FEng_BiomeConfiguration& Config = ConfigPair.Value;
        
        // Validate temperature range
        if (Config.Temperature < -50.0f || Config.Temperature > 60.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid temperature for biome %d: %f"), (int32)ConfigPair.Key, Config.Temperature);
            continue;
        }
        
        // Validate humidity range
        if (Config.Humidity < 0.0f || Config.Humidity > 100.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid humidity for biome %d: %f"), (int32)ConfigPair.Key, Config.Humidity);
            continue;
        }
        
        ValidConfigs++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: %d/%d biome configurations are valid"), ValidConfigs, BiomeConfigurations.Num());
}

EBiomeType UEng_BiomeSystemManager::CalculateBiomeFromEnvironment(const FVector& WorldLocation) const
{
    // Simplified biome calculation based on location
    // In a full implementation, this would consider elevation, temperature, humidity, etc.
    
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    float Z = WorldLocation.Z;
    
    // Use noise-like functions to determine biome
    float NoiseValue = FMath::Sin(X * 0.0001f) + FMath::Cos(Y * 0.0001f);
    
    if (Z > 500.0f) // High elevation
    {
        return EBiomeType::Mountains;
    }
    else if (NoiseValue > 0.5f)
    {
        return EBiomeType::Forest;
    }
    else if (NoiseValue < -0.5f)
    {
        return EBiomeType::Desert;
    }
    else if (Z < 50.0f) // Low elevation
    {
        return EBiomeType::Swamp;
    }
    else
    {
        return EBiomeType::Plains;
    }
}

void UEng_BiomeSystemManager::InitializeDefaultBiomeConfigurations()
{
    // Forest biome
    FEng_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.Temperature = 20.0f;
    ForestConfig.Humidity = 70.0f;
    ForestConfig.Elevation = 200.0f;
    ForestConfig.MaxDinosaursPerBiome = 30;
    ForestConfig.VegetationDensity = 0.8f;
    ForestConfig.AmbientColor = FLinearColor(0.2f, 0.8f, 0.2f, 1.0f);
    ForestConfig.FogDensity = 0.03f;
    BiomeConfigurations.Add(EBiomeType::Forest, ForestConfig);
    
    // Desert biome
    FEng_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.Temperature = 40.0f;
    DesertConfig.Humidity = 20.0f;
    DesertConfig.Elevation = 100.0f;
    DesertConfig.MaxDinosaursPerBiome = 15;
    DesertConfig.VegetationDensity = 0.1f;
    DesertConfig.AmbientColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
    DesertConfig.FogDensity = 0.01f;
    BiomeConfigurations.Add(EBiomeType::Desert, DesertConfig);
    
    // Swamp biome
    FEng_BiomeConfiguration SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.Temperature = 25.0f;
    SwampConfig.Humidity = 90.0f;
    SwampConfig.Elevation = 50.0f;
    SwampConfig.MaxDinosaursPerBiome = 20;
    SwampConfig.VegetationDensity = 0.9f;
    SwampConfig.AmbientColor = FLinearColor(0.3f, 0.6f, 0.4f, 1.0f);
    SwampConfig.FogDensity = 0.05f;
    BiomeConfigurations.Add(EBiomeType::Swamp, SwampConfig);
    
    // Mountain biome
    FEng_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountains;
    MountainConfig.Temperature = 10.0f;
    MountainConfig.Humidity = 40.0f;
    MountainConfig.Elevation = 800.0f;
    MountainConfig.MaxDinosaursPerBiome = 10;
    MountainConfig.VegetationDensity = 0.3f;
    MountainConfig.AmbientColor = FLinearColor(0.6f, 0.6f, 0.8f, 1.0f);
    MountainConfig.FogDensity = 0.02f;
    BiomeConfigurations.Add(EBiomeType::Mountains, MountainConfig);
    
    // Plains biome
    FEng_BiomeConfiguration PlainsConfig;
    PlainsConfig.BiomeType = EBiomeType::Plains;
    PlainsConfig.Temperature = 22.0f;
    PlainsConfig.Humidity = 50.0f;
    PlainsConfig.Elevation = 150.0f;
    PlainsConfig.MaxDinosaursPerBiome = 40;
    PlainsConfig.VegetationDensity = 0.4f;
    PlainsConfig.AmbientColor = FLinearColor(0.7f, 0.8f, 0.3f, 1.0f);
    PlainsConfig.FogDensity = 0.015f;
    BiomeConfigurations.Add(EBiomeType::Plains, PlainsConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystemManager: Initialized %d default biome configurations"), BiomeConfigurations.Num());
}

void UEng_BiomeSystemManager::CacheBiomeData()
{
    // Initial cache population - in practice this would be more sophisticated
    if (BiomeCache.Num() == 0)
    {
        GenerateBiomeMap(16); // Default 16km world
    }
}
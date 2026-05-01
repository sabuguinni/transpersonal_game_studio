#include "Eng_BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UEng_BiomeManager::UEng_BiomeManager()
{
    // Initialize default values
    WorldSizeX = 20000.0f;
    WorldSizeY = 20000.0f;
    SwampRegionSize = 4000.0f;
    ForestRegionSize = 6000.0f;
    SavannaRegionSize = 5000.0f;
    DesertRegionSize = 3000.0f;
    MountainRegionSize = 2000.0f;
    WeatherSystem = nullptr;
    LastCacheCleanupTime = 0.0f;
}

void UEng_BiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::Initialize - Starting biome system initialization"));
    
    // Initialize biome configurations
    InitializeBiomeConfigs();
    
    // Setup biome transitions
    SetupBiomeTransitions();
    
    // Clear any existing cache
    BiomeLocationCache.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::Initialize - Biome system initialized with %d biome configs"), BiomeConfigs.Num());
}

void UEng_BiomeManager::Deinitialize()
{
    // Clear all registered biome zones
    RegisteredBiomeZones.Empty();
    
    // Clear cache
    BiomeLocationCache.Empty();
    
    // Clear weather system reference
    WeatherSystem = nullptr;
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::Deinitialize - Biome system shutdown complete"));
    
    Super::Deinitialize();
}

void UEng_BiomeManager::InitializeBiomeConfigs()
{
    // Clear existing configs
    BiomeConfigs.Empty();
    
    // SWAMP BIOME
    FEng_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.BiomeName = TEXT("Prehistoric Swamp");
    SwampConfig.TemperatureMin = 22.0f;
    SwampConfig.TemperatureMax = 32.0f;
    SwampConfig.HumidityLevel = 0.9f;
    SwampConfig.VegetationDensity = 0.8f;
    SwampConfig.FogColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    SwampConfig.FogDensity = 0.05f;
    BiomeConfigs.Add(EBiomeType::Swamp, SwampConfig);
    
    // FOREST BIOME
    FEng_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.BiomeName = TEXT("Dense Prehistoric Forest");
    ForestConfig.TemperatureMin = 15.0f;
    ForestConfig.TemperatureMax = 25.0f;
    ForestConfig.HumidityLevel = 0.7f;
    ForestConfig.VegetationDensity = 0.9f;
    ForestConfig.FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
    ForestConfig.FogDensity = 0.02f;
    BiomeConfigs.Add(EBiomeType::Forest, ForestConfig);
    
    // SAVANNA BIOME
    FEng_BiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.BiomeName = TEXT("Cretaceous Savanna");
    SavannaConfig.TemperatureMin = 20.0f;
    SavannaConfig.TemperatureMax = 35.0f;
    SavannaConfig.HumidityLevel = 0.4f;
    SavannaConfig.VegetationDensity = 0.5f;
    SavannaConfig.FogColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
    SavannaConfig.FogDensity = 0.01f;
    BiomeConfigs.Add(EBiomeType::Savanna, SavannaConfig);
    
    // DESERT BIOME
    FEng_BiomeConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.BiomeName = TEXT("Arid Desert");
    DesertConfig.TemperatureMin = 25.0f;
    DesertConfig.TemperatureMax = 45.0f;
    DesertConfig.HumidityLevel = 0.1f;
    DesertConfig.VegetationDensity = 0.2f;
    DesertConfig.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    DesertConfig.FogDensity = 0.005f;
    BiomeConfigs.Add(EBiomeType::Desert, DesertConfig);
    
    // SNOW MOUNTAIN BIOME
    FEng_BiomeConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::SnowMountain;
    MountainConfig.BiomeName = TEXT("Snow-Capped Mountains");
    MountainConfig.TemperatureMin = -5.0f;
    MountainConfig.TemperatureMax = 10.0f;
    MountainConfig.HumidityLevel = 0.6f;
    MountainConfig.VegetationDensity = 0.3f;
    MountainConfig.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MountainConfig.FogDensity = 0.03f;
    BiomeConfigs.Add(EBiomeType::SnowMountain, MountainConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::InitializeBiomeConfigs - Initialized %d biome configurations"), BiomeConfigs.Num());
}

void UEng_BiomeManager::SetupBiomeTransitions()
{
    BiomeTransitions.Empty();
    
    // Forest to Swamp transition
    FEng_BiomeTransition ForestToSwamp;
    ForestToSwamp.FromBiome = EBiomeType::Forest;
    ForestToSwamp.ToBiome = EBiomeType::Swamp;
    ForestToSwamp.TransitionDistance = 800.0f;
    ForestToSwamp.BlendFactor = 0.5f;
    BiomeTransitions.Add(ForestToSwamp);
    
    // Forest to Savanna transition
    FEng_BiomeTransition ForestToSavanna;
    ForestToSavanna.FromBiome = EBiomeType::Forest;
    ForestToSavanna.ToBiome = EBiomeType::Savanna;
    ForestToSavanna.TransitionDistance = 1200.0f;
    ForestToSavanna.BlendFactor = 0.6f;
    BiomeTransitions.Add(ForestToSavanna);
    
    // Savanna to Desert transition
    FEng_BiomeTransition SavannaToDesert;
    SavannaToDesert.FromBiome = EBiomeType::Savanna;
    SavannaToDesert.ToBiome = EBiomeType::Desert;
    SavannaToDesert.TransitionDistance = 1000.0f;
    SavannaToDesert.BlendFactor = 0.7f;
    BiomeTransitions.Add(SavannaToDesert);
    
    // Forest to Mountain transition
    FEng_BiomeTransition ForestToMountain;
    ForestToMountain.FromBiome = EBiomeType::Forest;
    ForestToMountain.ToBiome = EBiomeType::SnowMountain;
    ForestToMountain.TransitionDistance = 1500.0f;
    ForestToMountain.BlendFactor = 0.8f;
    BiomeTransitions.Add(ForestToMountain);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::SetupBiomeTransitions - Setup %d biome transitions"), BiomeTransitions.Num());
}

EBiomeType UEng_BiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Check cache first for performance
    if (BiomeLocationCache.Contains(WorldLocation))
    {
        return BiomeLocationCache[WorldLocation];
    }
    
    // Calculate biome based on world position
    EBiomeType BiomeType = CalculateBiomeFromWorldPosition(WorldLocation);
    
    // Cache the result
    if (BiomeLocationCache.Num() < MAX_CACHE_ENTRIES)
    {
        BiomeLocationCache.Add(WorldLocation, BiomeType);
    }
    
    return BiomeType;
}

EBiomeType UEng_BiomeManager::CalculateBiomeFromWorldPosition(const FVector& WorldLocation) const
{
    // Convert world location to normalized coordinates (0-1)
    float NormalizedX = (WorldLocation.X + WorldSizeX * 0.5f) / WorldSizeX;
    float NormalizedY = (WorldLocation.Y + WorldSizeY * 0.5f) / WorldSizeY;
    
    // Clamp to valid range
    NormalizedX = FMath::Clamp(NormalizedX, 0.0f, 1.0f);
    NormalizedY = FMath::Clamp(NormalizedY, 0.0f, 1.0f);
    
    // Simple biome distribution based on world quadrants and elevation
    float DistanceFromCenter = FVector2D(NormalizedX - 0.5f, NormalizedY - 0.5f).Size();
    float Elevation = WorldLocation.Z;
    
    // Mountain biome at high elevations
    if (Elevation > 1500.0f)
    {
        return EBiomeType::SnowMountain;
    }
    
    // Desert in the far corners
    if (DistanceFromCenter > 0.6f && (NormalizedX > 0.7f || NormalizedX < 0.3f))
    {
        return EBiomeType::Desert;
    }
    
    // Swamp in low-lying areas near water
    if (Elevation < 100.0f && NormalizedY < 0.4f)
    {
        return EBiomeType::Swamp;
    }
    
    // Savanna in open areas
    if (NormalizedY > 0.6f && DistanceFromCenter > 0.3f)
    {
        return EBiomeType::Savanna;
    }
    
    // Default to Forest
    return EBiomeType::Forest;
}

FEng_BiomeConfig UEng_BiomeManager::GetBiomeConfig(EBiomeType BiomeType) const
{
    if (BiomeConfigs.Contains(BiomeType))
    {
        return BiomeConfigs[BiomeType];
    }
    
    // Return default forest config if not found
    FEng_BiomeConfig DefaultConfig;
    UE_LOG(LogTemp, Warning, TEXT("UEng_BiomeManager::GetBiomeConfig - Biome type %d not found, returning default"), (int32)BiomeType);
    return DefaultConfig;
}

float UEng_BiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    
    // Add some variation based on location and time
    float BaseTemp = (Config.TemperatureMin + Config.TemperatureMax) * 0.5f;
    float Variation = FMath::Sin(WorldLocation.X * 0.001f) * FMath::Cos(WorldLocation.Y * 0.001f) * 3.0f;
    
    return BaseTemp + Variation;
}

float UEng_BiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    
    return Config.HumidityLevel;
}

bool UEng_BiomeManager::IsLocationInBiomeTransition(const FVector& WorldLocation) const
{
    // Check if location is near any biome border
    float MinDistanceToBorder = CalculateDistanceToBiomeBorder(WorldLocation, GetBiomeAtLocation(WorldLocation));
    
    return MinDistanceToBorder < 1000.0f; // 1km transition zone
}

FEng_BiomeTransition UEng_BiomeManager::GetBiomeTransitionData(const FVector& WorldLocation) const
{
    EBiomeType CurrentBiome = GetBiomeAtLocation(WorldLocation);
    
    // Find the closest transition
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        if (Transition.FromBiome == CurrentBiome || Transition.ToBiome == CurrentBiome)
        {
            return Transition;
        }
    }
    
    // Return default transition
    FEng_BiomeTransition DefaultTransition;
    return DefaultTransition;
}

float UEng_BiomeManager::CalculateDistanceToBiomeBorder(const FVector& Location, EBiomeType BiomeType) const
{
    // Simplified calculation - in a real implementation this would be more sophisticated
    float MinDistance = 10000.0f;
    
    // Sample points around the location to find biome changes
    for (int32 Angle = 0; Angle < 360; Angle += 45)
    {
        float RadianAngle = FMath::DegreesToRadians(Angle);
        for (float Distance = 100.0f; Distance < 2000.0f; Distance += 100.0f)
        {
            FVector TestLocation = Location + FVector(
                FMath::Cos(RadianAngle) * Distance,
                FMath::Sin(RadianAngle) * Distance,
                0.0f
            );
            
            if (GetBiomeAtLocation(TestLocation) != BiomeType)
            {
                MinDistance = FMath::Min(MinDistance, Distance);
                break;
            }
        }
    }
    
    return MinDistance;
}

void UEng_BiomeManager::RegisterBiomeZone(ABiomeZone* BiomeZone)
{
    if (BiomeZone && !RegisteredBiomeZones.Contains(BiomeZone))
    {
        RegisteredBiomeZones.Add(BiomeZone);
        UE_LOG(LogTemp, Log, TEXT("UEng_BiomeManager::RegisterBiomeZone - Registered biome zone: %s"), *BiomeZone->GetName());
    }
}

void UEng_BiomeManager::UnregisterBiomeZone(ABiomeZone* BiomeZone)
{
    if (BiomeZone)
    {
        RegisteredBiomeZones.Remove(BiomeZone);
        UE_LOG(LogTemp, Log, TEXT("UEng_BiomeManager::UnregisterBiomeZone - Unregistered biome zone: %s"), *BiomeZone->GetName());
    }
}

TArray<ABiomeZone*> UEng_BiomeManager::GetBiomeZonesInRadius(const FVector& Center, float Radius) const
{
    TArray<ABiomeZone*> ZonesInRadius;
    
    for (ABiomeZone* Zone : RegisteredBiomeZones)
    {
        if (Zone && FVector::Dist(Zone->GetActorLocation(), Center) <= Radius)
        {
            ZonesInRadius.Add(Zone);
        }
    }
    
    return ZonesInRadius;
}

void UEng_BiomeManager::UpdateWeatherForBiome(EBiomeType BiomeType, float DeltaTime)
{
    // Weather system integration - placeholder for now
    if (WeatherSystem)
    {
        // Update weather patterns based on biome characteristics
        FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
        // Weather system would use Config.HumidityLevel, temperature ranges, etc.
    }
}

bool UEng_BiomeManager::CanWeatherOccurInBiome(EWeatherType WeatherType, EBiomeType BiomeType) const
{
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    
    // Simple weather compatibility rules
    switch (WeatherType)
    {
    case EWeatherType::Rain:
        return Config.HumidityLevel > 0.3f;
    case EWeatherType::Snow:
        return BiomeType == EBiomeType::SnowMountain || Config.TemperatureMax < 5.0f;
    case EWeatherType::Sandstorm:
        return BiomeType == EBiomeType::Desert;
    case EWeatherType::Fog:
        return Config.HumidityLevel > 0.6f;
    default:
        return true;
    }
}

TArray<TSubclassOf<class ADinosaurBase>> UEng_BiomeManager::GetNativeDinosaursForBiome(EBiomeType BiomeType) const
{
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    return Config.NativeDinosaurs;
}

float UEng_BiomeManager::GetDinosaurSpawnChanceInBiome(TSubclassOf<class ADinosaurBase> DinosaurClass, EBiomeType BiomeType) const
{
    TArray<TSubclassOf<class ADinosaurBase>> NativeDinosaurs = GetNativeDinosaursForBiome(BiomeType);
    
    if (NativeDinosaurs.Contains(DinosaurClass))
    {
        return 1.0f; // 100% chance for native species
    }
    
    // Reduced chance for non-native species
    return 0.1f; // 10% chance
}

void UEng_BiomeManager::DebugPrintBiomeInfo(const FVector& Location) const
{
    EBiomeType BiomeType = GetBiomeAtLocation(Location);
    FEng_BiomeConfig Config = GetBiomeConfig(BiomeType);
    float Temperature = GetTemperatureAtLocation(Location);
    float Humidity = GetHumidityAtLocation(Location);
    bool InTransition = IsLocationInBiomeTransition(Location);
    
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME DEBUG INFO ==="));
    UE_LOG(LogTemp, Warning, TEXT("Location: %s"), *Location.ToString());
    UE_LOG(LogTemp, Warning, TEXT("Biome: %s"), *Config.BiomeName);
    UE_LOG(LogTemp, Warning, TEXT("Temperature: %.1f°C"), Temperature);
    UE_LOG(LogTemp, Warning, TEXT("Humidity: %.1f%%"), Humidity * 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("In Transition: %s"), InTransition ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("Vegetation Density: %.1f%%"), Config.VegetationDensity * 100.0f);
}

void UEng_BiomeManager::ValidateBiomeConfiguration() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME SYSTEM VALIDATION ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Biome Configs: %d"), BiomeConfigs.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total Transitions: %d"), BiomeTransitions.Num());
    UE_LOG(LogTemp, Warning, TEXT("Registered Zones: %d"), RegisteredBiomeZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Cache Entries: %d"), BiomeLocationCache.Num());
    
    // Validate each biome config
    for (const auto& BiomePair : BiomeConfigs)
    {
        const FEng_BiomeConfig& Config = BiomePair.Value;
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: Temp %.1f-%.1f°C, Humidity %.1f, Vegetation %.1f"), 
            *Config.BiomeName, Config.TemperatureMin, Config.TemperatureMax, 
            Config.HumidityLevel, Config.VegetationDensity);
    }
}

void UEng_BiomeManager::CleanupLocationCache() const
{
    if (BiomeLocationCache.Num() > MAX_CACHE_ENTRIES)
    {
        // Remove oldest entries (simplified - in production use LRU cache)
        BiomeLocationCache.Empty();
        UE_LOG(LogTemp, Log, TEXT("UEng_BiomeManager::CleanupLocationCache - Cache cleared"));
    }
}
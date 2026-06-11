#include "BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

UBiomeManager::UBiomeManager()
{
    BiomeTransitionSmoothness = 0.3f;
    BiomeResolution = 100;
    TemperatureVariation = 15.0f;
    HumidityVariation = 0.4f;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initializing biome system"));
    
    // Initialize default biome configurations
    InitializeDefaultBiomes();
    
    // Set up global environmental parameters
    GlobalEnvironmentalParams.BaseTemperature = 25.0f; // 25°C base
    GlobalEnvironmentalParams.BaseHumidity = 0.6f; // 60% base humidity
    GlobalEnvironmentalParams.SeasonalVariation = 0.2f;
    GlobalEnvironmentalParams.DayNightTemperatureDelta = 8.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized with %d biome types"), BiomeDefinitions.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeDefinitions.Empty();
    ActiveBiomeZones.Empty();
    BiomeCache.Empty();
    EnvironmentCache.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Deinitialized"));
    Super::Deinitialize();
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Convert world location to biome grid coordinates
    FIntVector GridCoord = FIntVector(
        FMath::FloorToInt(WorldLocation.X / BiomeResolution),
        FMath::FloorToInt(WorldLocation.Y / BiomeResolution),
        0
    );
    
    // Check cache first
    if (BiomeCache.Contains(GridCoord))
    {
        return BiomeCache[GridCoord];
    }
    
    // Calculate biome based on environmental parameters
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    float Humidity = GetHumidityAtLocation(WorldLocation);
    float Elevation = WorldLocation.Z;
    
    EEng_BiomeType BiomeType = CalculateBiomeFromParameters(Temperature, Humidity, Elevation);
    
    // Cache result
    BiomeCache.Add(GridCoord, BiomeType);
    
    return BiomeType;
}

FEng_BiomeData UBiomeManager::GetBiomeData(EEng_BiomeType BiomeType) const
{
    if (BiomeDefinitions.Contains(BiomeType))
    {
        return BiomeDefinitions[BiomeType];
    }
    
    // Return default biome data if not found
    FEng_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = EEng_BiomeType::Temperate_Forest;
    DefaultBiome.Temperature = 20.0f;
    DefaultBiome.Humidity = 0.7f;
    DefaultBiome.VegetationDensity = 0.6f;
    DefaultBiome.WaterAvailability = 0.5f;
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Biome type %d not found, returning default"), (int32)BiomeType);
    return DefaultBiome;
}

void UBiomeManager::RegisterBiomeZone(const FEng_BiomeZone& BiomeZone)
{
    ActiveBiomeZones.Add(BiomeZone);
    UpdateBiomeZoneCache();
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome zone at (%f, %f) with radius %f"), 
           BiomeZone.Center.X, BiomeZone.Center.Y, BiomeZone.Radius);
}

TArray<FEng_BiomeZone> UBiomeManager::GetBiomeZonesInRadius(const FVector& Center, float Radius) const
{
    TArray<FEng_BiomeZone> NearbyZones;
    
    for (const FEng_BiomeZone& Zone : ActiveBiomeZones)
    {
        float Distance = FVector::Dist2D(Center, Zone.Center);
        if (Distance <= (Radius + Zone.Radius))
        {
            NearbyZones.Add(Zone);
        }
    }
    
    return NearbyZones;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    float BaseTemp = GlobalEnvironmentalParams.BaseTemperature;
    
    // Add elevation-based cooling (6.5°C per 1000m)
    float ElevationCooling = (WorldLocation.Z / 1000.0f) * 6.5f;
    
    // Add noise variation
    float NoiseVariation = GenerateTemperatureNoise(WorldLocation) * TemperatureVariation;
    
    return BaseTemp - ElevationCooling + NoiseVariation;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    float BaseHumidity = GlobalEnvironmentalParams.BaseHumidity;
    
    // Add noise variation
    float NoiseVariation = GenerateHumidityNoise(WorldLocation) * HumidityVariation;
    
    return FMath::Clamp(BaseHumidity + NoiseVariation, 0.0f, 1.0f);
}

EEng_WeatherType UBiomeManager::GetWeatherAtLocation(const FVector& WorldLocation) const
{
    float Humidity = GetHumidityAtLocation(WorldLocation);
    float Temperature = GetTemperatureAtLocation(WorldLocation);
    
    // Simple weather determination based on environmental conditions
    if (Humidity > 0.8f && Temperature > 15.0f)
    {
        return EEng_WeatherType::Rainy;
    }
    else if (Temperature < 5.0f)
    {
        return EEng_WeatherType::Cold;
    }
    else if (Humidity < 0.3f)
    {
        return EEng_WeatherType::Dry;
    }
    
    return EEng_WeatherType::Clear;
}

bool UBiomeManager::IsInBiomeTransitionZone(const FVector& WorldLocation, float TransitionRadius) const
{
    EEng_BiomeType CenterBiome = GetBiomeAtLocation(WorldLocation);
    
    // Check surrounding points
    TArray<FVector> TestPoints = {
        WorldLocation + FVector(TransitionRadius, 0, 0),
        WorldLocation + FVector(-TransitionRadius, 0, 0),
        WorldLocation + FVector(0, TransitionRadius, 0),
        WorldLocation + FVector(0, -TransitionRadius, 0)
    };
    
    for (const FVector& TestPoint : TestPoints)
    {
        if (GetBiomeAtLocation(TestPoint) != CenterBiome)
        {
            return true;
        }
    }
    
    return false;
}

FEng_BiomeTransition UBiomeManager::CalculateBiomeTransition(const FVector& WorldLocation) const
{
    FEng_BiomeTransition Transition;
    Transition.PrimaryBiome = GetBiomeAtLocation(WorldLocation);
    Transition.SecondaryBiome = EEng_BiomeType::Temperate_Forest; // Default
    Transition.BlendFactor = 0.0f;
    
    if (IsInBiomeTransitionZone(WorldLocation))
    {
        // Find the secondary biome and calculate blend factor
        float MinDistance = MAX_FLT;
        EEng_BiomeType ClosestDifferentBiome = Transition.PrimaryBiome;
        
        // Sample surrounding area
        for (int32 i = 0; i < 8; ++i)
        {
            float Angle = (i / 8.0f) * 2.0f * PI;
            FVector SamplePoint = WorldLocation + FVector(
                FMath::Cos(Angle) * 1000.0f,
                FMath::Sin(Angle) * 1000.0f,
                0
            );
            
            EEng_BiomeType SampleBiome = GetBiomeAtLocation(SamplePoint);
            if (SampleBiome != Transition.PrimaryBiome)
            {
                float Distance = FVector::Dist2D(WorldLocation, SamplePoint);
                if (Distance < MinDistance)
                {
                    MinDistance = Distance;
                    ClosestDifferentBiome = SampleBiome;
                }
            }
        }
        
        Transition.SecondaryBiome = ClosestDifferentBiome;
        Transition.BlendFactor = FMath::Clamp(1.0f - (MinDistance / 1000.0f), 0.0f, 1.0f);
    }
    
    return Transition;
}

void UBiomeManager::ValidateBiomeConfiguration()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validating biome configuration..."));
    
    int32 ValidBiomes = 0;
    int32 InvalidBiomes = 0;
    
    for (const auto& BiomePair : BiomeDefinitions)
    {
        if (ValidateBiomeData(BiomePair.Value))
        {
            ValidBiomes++;
        }
        else
        {
            InvalidBiomes++;
            UE_LOG(LogTemp, Error, TEXT("BiomeManager: Invalid biome data for type %d"), (int32)BiomePair.Key);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Validation complete - %d valid, %d invalid biomes"), 
           ValidBiomes, InvalidBiomes);
}

void UBiomeManager::GenerateDebugBiomeMap()
{
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Generating debug biome map..."));
    
    // Generate a simple biome map for debugging
    const int32 MapSize = 20;
    const float SampleDistance = 1000.0f;
    
    for (int32 X = -MapSize; X <= MapSize; ++X)
    {
        FString RowString = TEXT("");
        for (int32 Y = -MapSize; Y <= MapSize; ++Y)
        {
            FVector SampleLocation = FVector(X * SampleDistance, Y * SampleDistance, 0);
            EEng_BiomeType BiomeType = GetBiomeAtLocation(SampleLocation);
            
            // Convert biome type to character for display
            TCHAR BiomeChar = TEXT('?');
            switch (BiomeType)
            {
                case EEng_BiomeType::Tropical_Rainforest: BiomeChar = TEXT('R'); break;
                case EEng_BiomeType::Temperate_Forest: BiomeChar = TEXT('F'); break;
                case EEng_BiomeType::Grassland: BiomeChar = TEXT('G'); break;
                case EEng_BiomeType::Desert: BiomeChar = TEXT('D'); break;
                case EEng_BiomeType::Tundra: BiomeChar = TEXT('T'); break;
                case EEng_BiomeType::Swamp: BiomeChar = TEXT('S'); break;
                case EEng_BiomeType::Mountain: BiomeChar = TEXT('M'); break;
                case EEng_BiomeType::Coastal: BiomeChar = TEXT('C'); break;
                default: BiomeChar = TEXT('?'); break;
            }
            
            RowString += BiomeChar;
        }
        UE_LOG(LogTemp, Warning, TEXT("BiomeMap[%2d]: %s"), X, *RowString);
    }
}

EEng_BiomeType UBiomeManager::CalculateBiomeFromParameters(float Temperature, float Humidity, float Elevation) const
{
    // Biome determination logic based on temperature, humidity, and elevation
    if (Elevation > 2000.0f)
    {
        return EEng_BiomeType::Mountain;
    }
    else if (Temperature > 25.0f && Humidity > 0.8f)
    {
        return EEng_BiomeType::Tropical_Rainforest;
    }
    else if (Temperature > 20.0f && Humidity > 0.6f)
    {
        return EEng_BiomeType::Temperate_Forest;
    }
    else if (Temperature > 15.0f && Humidity < 0.3f)
    {
        return EEng_BiomeType::Desert;
    }
    else if (Temperature < 5.0f)
    {
        return EEng_BiomeType::Tundra;
    }
    else if (Humidity > 0.9f)
    {
        return EEng_BiomeType::Swamp;
    }
    else if (Elevation < 50.0f)
    {
        return EEng_BiomeType::Coastal;
    }
    else
    {
        return EEng_BiomeType::Grassland;
    }
}

float UBiomeManager::GenerateTemperatureNoise(const FVector& Location) const
{
    // Simple noise generation for temperature variation
    float NoiseScale = 0.001f;
    float Noise = FMath::PerlinNoise2D(FVector2D(Location.X * NoiseScale, Location.Y * NoiseScale));
    return Noise; // Returns value between -1 and 1
}

float UBiomeManager::GenerateHumidityNoise(const FVector& Location) const
{
    // Simple noise generation for humidity variation
    float NoiseScale = 0.0015f;
    float Noise = FMath::PerlinNoise2D(FVector2D(Location.X * NoiseScale + 1000.0f, Location.Y * NoiseScale + 1000.0f));
    return Noise * 0.5f; // Returns value between -0.5 and 0.5
}

void UBiomeManager::InitializeDefaultBiomes()
{
    // Initialize default biome configurations
    FEng_BiomeData TropicalRainforest;
    TropicalRainforest.BiomeType = EEng_BiomeType::Tropical_Rainforest;
    TropicalRainforest.Temperature = 27.0f;
    TropicalRainforest.Humidity = 0.9f;
    TropicalRainforest.VegetationDensity = 0.95f;
    TropicalRainforest.WaterAvailability = 0.9f;
    BiomeDefinitions.Add(EEng_BiomeType::Tropical_Rainforest, TropicalRainforest);
    
    FEng_BiomeData TemperateForest;
    TemperateForest.BiomeType = EEng_BiomeType::Temperate_Forest;
    TemperateForest.Temperature = 15.0f;
    TemperateForest.Humidity = 0.7f;
    TemperateForest.VegetationDensity = 0.8f;
    TemperateForest.WaterAvailability = 0.6f;
    BiomeDefinitions.Add(EEng_BiomeType::Temperate_Forest, TemperateForest);
    
    FEng_BiomeData Grassland;
    Grassland.BiomeType = EEng_BiomeType::Grassland;
    Grassland.Temperature = 20.0f;
    Grassland.Humidity = 0.5f;
    Grassland.VegetationDensity = 0.4f;
    Grassland.WaterAvailability = 0.4f;
    BiomeDefinitions.Add(EEng_BiomeType::Grassland, Grassland);
    
    FEng_BiomeData Desert;
    Desert.BiomeType = EEng_BiomeType::Desert;
    Desert.Temperature = 35.0f;
    Desert.Humidity = 0.1f;
    Desert.VegetationDensity = 0.05f;
    Desert.WaterAvailability = 0.1f;
    BiomeDefinitions.Add(EEng_BiomeType::Desert, Desert);
    
    FEng_BiomeData Tundra;
    Tundra.BiomeType = EEng_BiomeType::Tundra;
    Tundra.Temperature = -5.0f;
    Tundra.Humidity = 0.3f;
    Tundra.VegetationDensity = 0.1f;
    Tundra.WaterAvailability = 0.2f;
    BiomeDefinitions.Add(EEng_BiomeType::Tundra, Tundra);
    
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.Temperature = 22.0f;
    Swamp.Humidity = 0.95f;
    Swamp.VegetationDensity = 0.7f;
    Swamp.WaterAvailability = 1.0f;
    BiomeDefinitions.Add(EEng_BiomeType::Swamp, Swamp);
    
    FEng_BiomeData Mountain;
    Mountain.BiomeType = EEng_BiomeType::Mountain;
    Mountain.Temperature = 5.0f;
    Mountain.Humidity = 0.4f;
    Mountain.VegetationDensity = 0.2f;
    Mountain.WaterAvailability = 0.3f;
    BiomeDefinitions.Add(EEng_BiomeType::Mountain, Mountain);
    
    FEng_BiomeData Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.Temperature = 18.0f;
    Coastal.Humidity = 0.8f;
    Coastal.VegetationDensity = 0.6f;
    Coastal.WaterAvailability = 0.8f;
    BiomeDefinitions.Add(EEng_BiomeType::Coastal, Coastal);
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialized %d default biomes"), BiomeDefinitions.Num());
}

void UBiomeManager::UpdateBiomeZoneCache()
{
    // Clear existing cache when biome zones change
    BiomeCache.Empty();
    EnvironmentCache.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Updated biome zone cache - %d active zones"), ActiveBiomeZones.Num());
}

bool UBiomeManager::ValidateBiomeData(const FEng_BiomeData& BiomeData) const
{
    // Validate biome data ranges
    if (BiomeData.Temperature < -50.0f || BiomeData.Temperature > 60.0f)
    {
        return false;
    }
    
    if (BiomeData.Humidity < 0.0f || BiomeData.Humidity > 1.0f)
    {
        return false;
    }
    
    if (BiomeData.VegetationDensity < 0.0f || BiomeData.VegetationDensity > 1.0f)
    {
        return false;
    }
    
    if (BiomeData.WaterAvailability < 0.0f || BiomeData.WaterAvailability > 1.0f)
    {
        return false;
    }
    
    return true;
}

void UBiomeManager::LogBiomeSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered biomes: %d"), BiomeDefinitions.Num());
    UE_LOG(LogTemp, Warning, TEXT("Active biome zones: %d"), ActiveBiomeZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Cached biome samples: %d"), BiomeCache.Num());
    UE_LOG(LogTemp, Warning, TEXT("Biome resolution: %d units"), BiomeResolution);
    UE_LOG(LogTemp, Warning, TEXT("Transition smoothness: %f"), BiomeTransitionSmoothness);
}
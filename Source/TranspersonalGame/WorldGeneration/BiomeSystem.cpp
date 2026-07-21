#include "BiomeSystem.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentPlayerBiome = EWorld_BiomeType::None;
    CurrentTimeOfDay = 8.0f;   // Start at 8am
    DayDurationSeconds = 600.0f; // 10 real minutes = 1 game day
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
    SetupDefaultWeather();
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AdvanceTimeOfDay(DeltaTime);
}

void ABiomeSystem::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // Forest biome
    FWorld_BiomeZone Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.Center = FVector(2000.0f, 2000.0f, 0.0f);
    Forest.Radius = 1800.0f;
    Forest.HeightOffset = 0.0f;
    Forest.DebugColor = FLinearColor(0.05f, 0.35f, 0.05f, 1.0f);
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.80f;
    Forest.VegetationDensity = 0.90f;
    BiomeZones.Add(Forest);

    // Plains biome
    FWorld_BiomeZone Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.Center = FVector(-2000.0f, 1500.0f, -50.0f);
    Plains.Radius = 2200.0f;
    Plains.HeightOffset = -50.0f;
    Plains.DebugColor = FLinearColor(0.45f, 0.55f, 0.10f, 1.0f);
    Plains.Temperature = 28.0f;
    Plains.Humidity = 0.30f;
    Plains.VegetationDensity = 0.35f;
    BiomeZones.Add(Plains);

    // Rocky Highland biome
    FWorld_BiomeZone Rocky;
    Rocky.BiomeType = EWorld_BiomeType::Rocky;
    Rocky.Center = FVector(2500.0f, -2000.0f, 200.0f);
    Rocky.Radius = 1600.0f;
    Rocky.HeightOffset = 200.0f;
    Rocky.DebugColor = FLinearColor(0.40f, 0.30f, 0.20f, 1.0f);
    Rocky.Temperature = 15.0f;
    Rocky.Humidity = 0.20f;
    Rocky.VegetationDensity = 0.15f;
    BiomeZones.Add(Rocky);

    // Swamp biome
    FWorld_BiomeZone Swamp;
    Swamp.BiomeType = EWorld_BiomeType::Swamp;
    Swamp.Center = FVector(-1500.0f, -2500.0f, -120.0f);
    Swamp.Radius = 1400.0f;
    Swamp.HeightOffset = -120.0f;
    Swamp.DebugColor = FLinearColor(0.10f, 0.25f, 0.10f, 1.0f);
    Swamp.Temperature = 30.0f;
    Swamp.Humidity = 0.95f;
    Swamp.VegetationDensity = 0.70f;
    BiomeZones.Add(Swamp);

    // Volcanic biome
    FWorld_BiomeZone Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.Center = FVector(500.0f, -500.0f, 350.0f);
    Volcanic.Radius = 1200.0f;
    Volcanic.HeightOffset = 350.0f;
    Volcanic.DebugColor = FLinearColor(0.35f, 0.08f, 0.02f, 1.0f);
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.10f;
    Volcanic.VegetationDensity = 0.05f;
    BiomeZones.Add(Volcanic);

    UE_LOG(LogTemp, Log, TEXT("ABiomeSystem: Initialized %d default biome zones"), BiomeZones.Num());
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(FVector WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::None;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }
    return ClosestBiome;
}

FWorld_BiomeZone ABiomeSystem::GetBiomeZoneData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return Zone;
        }
    }
    return FWorld_BiomeZone();
}

FWorld_BiomeWeather ABiomeSystem::GetWeatherForBiome(EWorld_BiomeType BiomeType) const
{
    const FWorld_BiomeWeather* Found = BiomeWeatherMap.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }
    return FWorld_BiomeWeather();
}

void ABiomeSystem::AdvanceTimeOfDay(float DeltaSeconds)
{
    if (DayDurationSeconds <= 0.0f) return;
    float HoursPerSecond = 24.0f / DayDurationSeconds;
    CurrentTimeOfDay += HoursPerSecond * DeltaSeconds;
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

bool ABiomeSystem::IsNightTime() const
{
    return (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f);
}

float ABiomeSystem::GetTemperatureAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeZone ZoneData = GetBiomeZoneData(Biome);
    // Modulate by time of day: cooler at night, hotter at noon
    float TimeModifier = FMath::Sin((CurrentTimeOfDay - 6.0f) / 24.0f * PI * 2.0f) * 5.0f;
    return ZoneData.Temperature + TimeModifier;
}

float ABiomeSystem::GetHumidityAtLocation(FVector WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeZone ZoneData = GetBiomeZoneData(Biome);
    return ZoneData.Humidity;
}

void ABiomeSystem::SetupDefaultWeather()
{
    FWorld_BiomeWeather ForestWeather;
    ForestWeather.RainIntensity = 0.3f;
    ForestWeather.FogDensity = 0.2f;
    ForestWeather.WindSpeed = 5.0f;
    ForestWeather.LightningChance = 0.05f;
    BiomeWeatherMap.Add(EWorld_BiomeType::Forest, ForestWeather);

    FWorld_BiomeWeather PlainsWeather;
    PlainsWeather.RainIntensity = 0.1f;
    PlainsWeather.FogDensity = 0.05f;
    PlainsWeather.WindSpeed = 12.0f;
    PlainsWeather.LightningChance = 0.02f;
    BiomeWeatherMap.Add(EWorld_BiomeType::Plains, PlainsWeather);

    FWorld_BiomeWeather RockyWeather;
    RockyWeather.RainIntensity = 0.05f;
    RockyWeather.FogDensity = 0.15f;
    RockyWeather.WindSpeed = 20.0f;
    RockyWeather.LightningChance = 0.10f;
    BiomeWeatherMap.Add(EWorld_BiomeType::Rocky, RockyWeather);

    FWorld_BiomeWeather SwampWeather;
    SwampWeather.RainIntensity = 0.5f;
    SwampWeather.FogDensity = 0.70f;
    SwampWeather.WindSpeed = 2.0f;
    SwampWeather.LightningChance = 0.08f;
    BiomeWeatherMap.Add(EWorld_BiomeType::Swamp, SwampWeather);

    FWorld_BiomeWeather VolcanicWeather;
    VolcanicWeather.RainIntensity = 0.0f;
    VolcanicWeather.FogDensity = 0.40f;  // Ash/smoke
    VolcanicWeather.WindSpeed = 8.0f;
    VolcanicWeather.LightningChance = 0.20f;
    BiomeWeatherMap.Add(EWorld_BiomeType::Volcanic, VolcanicWeather);

    UE_LOG(LogTemp, Log, TEXT("ABiomeSystem: Default weather configured for all 5 biomes"));
}

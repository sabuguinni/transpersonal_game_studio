// BiomeSystem.cpp
// Agent #5 — Procedural World Generator
// Manages biome zones, weather transitions, and environmental state for the prehistoric world.

#include "BiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 5.0f; // Weather ticks every 5 seconds

    InitializeBiomeDefaults();
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize weather timers for all biomes
    for (uint8 i = 0; i < static_cast<uint8>(EWorld_BiomeType::COUNT); ++i)
    {
        EWorld_BiomeType BiomeType = static_cast<EWorld_BiomeType>(i);
        WeatherTimers.Add(BiomeType, FMath::RandRange(MinWeatherDuration, MaxWeatherDuration));
        CurrentWeatherMap.Add(i, static_cast<uint8>(EWorld_WeatherState::Clear));
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeSystem] Initialized with %d biome types"), static_cast<int32>(EWorld_BiomeType::COUNT));
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickWeather(DeltaTime * 5.0f); // Scale by tick interval
}

// --- Biome Query ---

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Check registered biome zones
    for (const auto& Pair : BiomeZones)
    {
        if (Pair.Value.IsInsideOrOn(WorldLocation))
        {
            return Pair.Key;
        }
    }

    // Fallback: determine biome by altitude and distance from origin
    const float Altitude = WorldLocation.Z;
    const float DistFromOrigin = FVector2D(WorldLocation.X, WorldLocation.Y).Size();

    if (Altitude > 800.0f)
    {
        return EWorld_BiomeType::Volcanic;
    }
    else if (Altitude > 400.0f)
    {
        return EWorld_BiomeType::RockyHighlands;
    }
    else if (Altitude < -50.0f)
    {
        return EWorld_BiomeType::Swamp;
    }
    else if (DistFromOrigin < 500.0f)
    {
        return EWorld_BiomeType::RiverDelta;
    }
    else if (DistFromOrigin < 2000.0f)
    {
        return EWorld_BiomeType::Forest;
    }
    else
    {
        return EWorld_BiomeType::OpenPlains;
    }
}

FWorld_BiomeData ABiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    const uint8 Index = static_cast<uint8>(BiomeType);
    if (BiomeDataTable.IsValidIndex(Index))
    {
        return BiomeDataTable[Index];
    }

    // Return default forest data if index invalid
    FWorld_BiomeData Default;
    Default.BiomeType = EWorld_BiomeType::Forest;
    Default.BiomeName = TEXT("Unknown");
    return Default;
}

FWorld_BiomeData ABiomeSystem::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeData(GetBiomeAtLocation(WorldLocation));
}

// --- Weather ---

EWorld_WeatherState ABiomeSystem::GetCurrentWeather(EWorld_BiomeType BiomeType) const
{
    const uint8 Key = static_cast<uint8>(BiomeType);
    const uint8* Found = CurrentWeatherMap.Find(Key);
    if (Found)
    {
        return static_cast<EWorld_WeatherState>(*Found);
    }
    return EWorld_WeatherState::Clear;
}

void ABiomeSystem::SetWeather(EWorld_BiomeType BiomeType, EWorld_WeatherState NewWeather)
{
    const uint8 Key = static_cast<uint8>(BiomeType);
    CurrentWeatherMap.Add(Key, static_cast<uint8>(NewWeather));

    UE_LOG(LogTemp, Log, TEXT("[BiomeSystem] Weather changed for biome %d -> state %d"),
        static_cast<int32>(BiomeType), static_cast<int32>(NewWeather));
}

void ABiomeSystem::TickWeather(float DeltaTime)
{
    for (auto& Pair : WeatherTimers)
    {
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.0f)
        {
            // Transition to next weather
            EWorld_WeatherState NextWeather = PickNextWeather(Pair.Key);
            SetWeather(Pair.Key, NextWeather);

            // Reset timer
            Pair.Value = FMath::RandRange(MinWeatherDuration, MaxWeatherDuration);
        }
    }
}

// --- Biome Registration ---

void ABiomeSystem::RegisterBiomeZone(EWorld_BiomeType BiomeType, const FBox& ZoneBounds)
{
    BiomeZones.Add(BiomeType, ZoneBounds);
    UE_LOG(LogTemp, Log, TEXT("[BiomeSystem] Registered biome zone: type=%d, bounds=%s"),
        static_cast<int32>(BiomeType), *ZoneBounds.ToString());
}

// --- Debug ---

void ABiomeSystem::PrintBiomeDebugInfo() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BiomeSystem Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered zones: %d"), BiomeZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Biome data entries: %d"), BiomeDataTable.Num());

    for (const auto& Pair : CurrentWeatherMap)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Biome[%d] weather: %d"), Pair.Key, Pair.Value);
    }

    UE_LOG(LogTemp, Warning, TEXT("=============================="));
}

// --- Private ---

void ABiomeSystem::InitializeBiomeDefaults()
{
    BiomeDataTable.SetNum(static_cast<int32>(EWorld_BiomeType::COUNT));

    // Forest
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::Forest)];
        D.BiomeType = EWorld_BiomeType::Forest;
        D.BiomeName = TEXT("Prehistoric Forest");
        D.AmbientTemperature = 24.0f;
        D.Humidity = 0.85f;
        D.VegetationDensity = 0.9f;
        D.DinosaurSpawnRate = 1.2f;
        D.FogColor = FLinearColor(0.4f, 0.7f, 0.3f, 1.0f);
        D.FogDensity = 0.025f;
        D.PlayerStaminaDrainMultiplier = 1.1f;
        D.PlayerHungerDrainMultiplier = 1.0f;
        D.PlayerThirstDrainMultiplier = 0.9f;
    }

    // Rocky Highlands
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::RockyHighlands)];
        D.BiomeType = EWorld_BiomeType::RockyHighlands;
        D.BiomeName = TEXT("Rocky Highlands");
        D.AmbientTemperature = 15.0f;
        D.Humidity = 0.3f;
        D.VegetationDensity = 0.3f;
        D.DinosaurSpawnRate = 0.8f;
        D.FogColor = FLinearColor(0.7f, 0.6f, 0.4f, 1.0f);
        D.FogDensity = 0.01f;
        D.PlayerStaminaDrainMultiplier = 1.3f;
        D.PlayerHungerDrainMultiplier = 1.2f;
        D.PlayerThirstDrainMultiplier = 1.3f;
    }

    // River Delta
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::RiverDelta)];
        D.BiomeType = EWorld_BiomeType::RiverDelta;
        D.BiomeName = TEXT("River Delta");
        D.AmbientTemperature = 26.0f;
        D.Humidity = 0.95f;
        D.VegetationDensity = 0.75f;
        D.DinosaurSpawnRate = 1.5f;
        D.FogColor = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
        D.FogDensity = 0.035f;
        D.PlayerStaminaDrainMultiplier = 1.0f;
        D.PlayerHungerDrainMultiplier = 0.9f;
        D.PlayerThirstDrainMultiplier = 0.5f; // Water abundant
    }

    // Open Plains
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::OpenPlains)];
        D.BiomeType = EWorld_BiomeType::OpenPlains;
        D.BiomeName = TEXT("Open Plains");
        D.AmbientTemperature = 28.0f;
        D.Humidity = 0.4f;
        D.VegetationDensity = 0.5f;
        D.DinosaurSpawnRate = 1.8f; // Herds roam here
        D.FogColor = FLinearColor(0.8f, 0.8f, 0.5f, 1.0f);
        D.FogDensity = 0.005f;
        D.PlayerStaminaDrainMultiplier = 0.9f;
        D.PlayerHungerDrainMultiplier = 1.1f;
        D.PlayerThirstDrainMultiplier = 1.2f;
    }

    // Volcanic
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::Volcanic)];
        D.BiomeType = EWorld_BiomeType::Volcanic;
        D.BiomeName = TEXT("Volcanic Zone");
        D.AmbientTemperature = 45.0f;
        D.Humidity = 0.1f;
        D.VegetationDensity = 0.05f;
        D.DinosaurSpawnRate = 0.4f;
        D.FogColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
        D.FogDensity = 0.05f;
        D.PlayerStaminaDrainMultiplier = 1.5f;
        D.PlayerHungerDrainMultiplier = 1.3f;
        D.PlayerThirstDrainMultiplier = 1.8f;
    }

    // Swamp
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::Swamp)];
        D.BiomeType = EWorld_BiomeType::Swamp;
        D.BiomeName = TEXT("Swamp");
        D.AmbientTemperature = 29.0f;
        D.Humidity = 1.0f;
        D.VegetationDensity = 0.7f;
        D.DinosaurSpawnRate = 1.1f;
        D.FogColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);
        D.FogDensity = 0.06f;
        D.PlayerStaminaDrainMultiplier = 1.2f;
        D.PlayerHungerDrainMultiplier = 1.0f;
        D.PlayerThirstDrainMultiplier = 0.6f;
    }

    // Coastal
    {
        FWorld_BiomeData& D = BiomeDataTable[static_cast<uint8>(EWorld_BiomeType::Coastal)];
        D.BiomeType = EWorld_BiomeType::Coastal;
        D.BiomeName = TEXT("Coastal");
        D.AmbientTemperature = 22.0f;
        D.Humidity = 0.8f;
        D.VegetationDensity = 0.45f;
        D.DinosaurSpawnRate = 0.9f;
        D.FogColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
        D.FogDensity = 0.02f;
        D.PlayerStaminaDrainMultiplier = 1.0f;
        D.PlayerHungerDrainMultiplier = 0.95f;
        D.PlayerThirstDrainMultiplier = 0.7f;
    }
}

EWorld_WeatherState ABiomeSystem::PickNextWeather(EWorld_BiomeType BiomeType) const
{
    // Weighted weather transitions based on biome humidity
    const FWorld_BiomeData Data = GetBiomeData(BiomeType);
    const float Humidity = Data.Humidity;
    const float Rand = FMath::FRand();

    if (Rand < 0.4f)
    {
        return EWorld_WeatherState::Clear;
    }
    else if (Rand < 0.6f)
    {
        return EWorld_WeatherState::Overcast;
    }
    else if (Rand < 0.6f + Humidity * 0.25f)
    {
        return EWorld_WeatherState::Rain;
    }
    else if (Rand < 0.6f + Humidity * 0.35f)
    {
        return EWorld_WeatherState::Fog;
    }
    else
    {
        return EWorld_WeatherState::Storm;
    }
}

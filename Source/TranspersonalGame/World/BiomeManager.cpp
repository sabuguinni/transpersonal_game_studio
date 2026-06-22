// BiomeManager.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260622_002
// Full implementation of ABiomeManager — biome zone detection, player tracking,
// and environmental parameter queries for the MinPlayableMap.

#include "BiomeManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check every 0.5s, not every frame

    // Initialize with default MinPlayableMap biomes at construction
    InitializeDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    // Re-initialize in case editor values were changed
    if (BiomeConfigs.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    BiomeCheckTimer += DeltaTime;
    if (BiomeCheckTimer >= BiomeCheckInterval)
    {
        BiomeCheckTimer = 0.0f;
        CheckPlayerBiome();
    }
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeConfigs.Empty();
    SetupForestBiome();
    SetupPlainsBiome();
    SetupRockyRidgeBiome();
    SetupRiverValleyBiome();
}

void ABiomeManager::SetupForestBiome()
{
    FWorld_BiomeConfig Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.ZoneCenter = FVector(-3000.0f, -3000.0f, 50.0f);
    Forest.ZoneRadius = 2500.0f;
    Forest.AmbientTemperatureCelsius = 26.0f;  // Cooler under canopy
    Forest.Humidity = 0.85f;                    // High humidity — reduces thirst drain
    Forest.DangerLevel = 0.8f;                  // High — raptor territory
    Forest.MaxDinoPopulation = 6;               // Up to 6 raptors (3 nests x 2)
    Forest.FogDensityMultiplier = 1.8f;         // Dense fog under canopy
    BiomeConfigs.Add(Forest);
}

void ABiomeManager::SetupPlainsBiome()
{
    FWorld_BiomeConfig Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.ZoneCenter = FVector(3000.0f, 3000.0f, 30.0f);
    Plains.ZoneRadius = 3000.0f;
    Plains.AmbientTemperatureCelsius = 34.0f;   // Hot open plains
    Plains.Humidity = 0.25f;                     // Low humidity — increases thirst drain
    Plains.DangerLevel = 0.9f;                   // Very high — T-Rex territory
    Plains.MaxDinoPopulation = 2;                // 1-2 T-Rex (apex predator, low density)
    Plains.FogDensityMultiplier = 0.6f;          // Clear visibility on plains
    BiomeConfigs.Add(Plains);
}

void ABiomeManager::SetupRockyRidgeBiome()
{
    FWorld_BiomeConfig Rocky;
    Rocky.BiomeType = EWorld_BiomeType::RockyRidge;
    Rocky.ZoneCenter = FVector(3000.0f, -3000.0f, 100.0f);
    Rocky.ZoneRadius = 2000.0f;
    Rocky.AmbientTemperatureCelsius = 22.0f;    // Cooler at elevation
    Rocky.Humidity = 0.35f;                      // Moderate humidity
    Rocky.DangerLevel = 0.5f;                    // Moderate — good defensive position
    Rocky.MaxDinoPopulation = 3;                 // Small pterosaurs / herbivores
    Rocky.FogDensityMultiplier = 1.2f;           // Slight mist at ridge
    BiomeConfigs.Add(Rocky);
}

void ABiomeManager::SetupRiverValleyBiome()
{
    FWorld_BiomeConfig River;
    River.BiomeType = EWorld_BiomeType::RiverValley;
    River.ZoneCenter = FVector(-500.0f, 0.0f, -20.0f);
    River.ZoneRadius = 1500.0f;
    River.AmbientTemperatureCelsius = 24.0f;    // Cool near water
    River.Humidity = 0.95f;                      // Very high — near water source
    River.DangerLevel = 0.6f;                    // Moderate — animals gather at water
    River.MaxDinoPopulation = 4;                 // Mixed species at water source
    River.FogDensityMultiplier = 2.0f;           // Heavy morning mist near river
    BiomeConfigs.Add(River);
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistSq = FLT_MAX;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Unknown;

    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        float DistSq = FVector::DistSquaredXY(WorldLocation, Config.ZoneCenter);
        float RadiusSq = Config.ZoneRadius * Config.ZoneRadius;

        if (DistSq <= RadiusSq && DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestBiome = Config.BiomeType;
        }
    }

    return ClosestBiome;
}

bool ABiomeManager::GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeConfig& OutConfig) const
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            OutConfig = Config;
            return true;
        }
    }
    return false;
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeConfig Config;
    if (GetBiomeConfig(Biome, Config))
    {
        return Config.DangerLevel;
    }
    return 0.3f; // Default moderate danger outside defined zones
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeConfig Config;
    if (GetBiomeConfig(Biome, Config))
    {
        return Config.AmbientTemperatureCelsius;
    }
    return 28.0f; // Default Cretaceous ambient temperature
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeConfig Config;
    if (GetBiomeConfig(Biome, Config))
    {
        return Config.Humidity;
    }
    return 0.5f; // Default moderate humidity
}

void ABiomeManager::CheckPlayerBiome()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Skip if player hasn't moved significantly
    if (FVector::DistSquared(PlayerLoc, LastCheckedPlayerLocation) < 10000.0f) // 100 UU threshold
    {
        return;
    }
    LastCheckedPlayerLocation = PlayerLoc;

    EWorld_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);
    if (NewBiome != CurrentPlayerBiome)
    {
        EWorld_BiomeType OldBiome = CurrentPlayerBiome;
        CurrentPlayerBiome = NewBiome;
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
    }
}

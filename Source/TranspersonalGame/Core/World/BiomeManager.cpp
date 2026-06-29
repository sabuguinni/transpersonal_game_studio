// BiomeManager.cpp
// Procedural World Generator — Agent #5
// Manages biome zones, climate properties, and location-based queries.

#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    InitDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: %d biomes registered"), BiomeConfigs.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitDefaultBiomes()
{
    BiomeConfigs.Empty();

    // Forest biome — dense, humid, high foliage
    FWorld_BiomeConfig Forest;
    Forest.BiomeType            = EWorld_BiomeType::Forest;
    Forest.DisplayName          = TEXT("Dense Cretaceous Forest");
    Forest.AmbientTemperature   = 28.0f;
    Forest.Humidity             = 0.85f;
    Forest.FogDensityMultiplier = 1.8f;
    Forest.FoliageDensity       = 0.95f;
    Forest.DinosaurSpawnWeight  = 0.6f;
    Forest.bHasWater            = false;
    Forest.AmbientSoundTag      = FName("Biome_Forest");
    BiomeConfigs.Add(Forest);

    // Rocky Plains biome — dry, sparse
    FWorld_BiomeConfig Rocky;
    Rocky.BiomeType            = EWorld_BiomeType::RockyPlains;
    Rocky.DisplayName          = TEXT("Rocky Badlands");
    Rocky.AmbientTemperature   = 35.0f;
    Rocky.Humidity             = 0.2f;
    Rocky.FogDensityMultiplier = 0.4f;
    Rocky.FoliageDensity       = 0.15f;
    Rocky.DinosaurSpawnWeight  = 0.8f;
    Rocky.bHasWater            = false;
    Rocky.AmbientSoundTag      = FName("Biome_Rocky");
    BiomeConfigs.Add(Rocky);

    // Savanna biome — open, warm, moderate fauna
    FWorld_BiomeConfig Savanna;
    Savanna.BiomeType            = EWorld_BiomeType::Savanna;
    Savanna.DisplayName          = TEXT("Open Savanna");
    Savanna.AmbientTemperature   = 32.0f;
    Savanna.Humidity             = 0.35f;
    Savanna.FogDensityMultiplier = 0.6f;
    Savanna.FoliageDensity       = 0.3f;
    Savanna.DinosaurSpawnWeight  = 0.9f;
    Savanna.bHasWater            = false;
    Savanna.AmbientSoundTag      = FName("Biome_Savanna");
    BiomeConfigs.Add(Savanna);

    // River Valley biome — water, high humidity
    FWorld_BiomeConfig River;
    River.BiomeType            = EWorld_BiomeType::RiverValley;
    River.DisplayName          = TEXT("River Valley");
    River.AmbientTemperature   = 24.0f;
    River.Humidity             = 0.95f;
    River.FogDensityMultiplier = 2.5f;
    River.FoliageDensity       = 0.8f;
    River.DinosaurSpawnWeight  = 0.7f;
    River.bHasWater            = true;
    River.AmbientSoundTag      = FName("Biome_River");
    BiomeConfigs.Add(River);

    // Swamp biome — very humid, low visibility
    FWorld_BiomeConfig Swamp;
    Swamp.BiomeType            = EWorld_BiomeType::Swamp;
    Swamp.DisplayName          = TEXT("Prehistoric Swamp");
    Swamp.AmbientTemperature   = 22.0f;
    Swamp.Humidity             = 1.0f;
    Swamp.FogDensityMultiplier = 4.0f;
    Swamp.FoliageDensity       = 0.7f;
    Swamp.DinosaurSpawnWeight  = 0.5f;
    Swamp.bHasWater            = true;
    Swamp.AmbientSoundTag      = FName("Biome_Swamp");
    BiomeConfigs.Add(Swamp);

    // Volcanic biome — extreme heat, low fauna
    FWorld_BiomeConfig Volcano;
    Volcano.BiomeType            = EWorld_BiomeType::Volcano;
    Volcano.DisplayName          = TEXT("Volcanic Region");
    Volcano.AmbientTemperature   = 55.0f;
    Volcano.Humidity             = 0.1f;
    Volcano.FogDensityMultiplier = 3.0f;
    Volcano.FoliageDensity       = 0.05f;
    Volcano.DinosaurSpawnWeight  = 0.2f;
    Volcano.bHasWater            = false;
    Volcano.AmbientSoundTag      = FName("Biome_Volcano");
    BiomeConfigs.Add(Volcano);
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Simple sector-based biome detection using actor-relative position
    const FVector LocalPos = WorldLocation - GetActorLocation();
    const float Dist = LocalPos.Size2D();

    // Beyond detection radius → default Forest
    if (Dist > BiomeDetectionRadius)
    {
        return EWorld_BiomeType::Forest;
    }

    // Sector logic: divide world into angular sectors
    const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(LocalPos.Y, LocalPos.X));

    if (LocalPos.Y > 2000.0f)
    {
        // North: Forest + River Valley
        if (FMath::Abs(LocalPos.X) < 1500.0f)
            return EWorld_BiomeType::RiverValley;
        return EWorld_BiomeType::Forest;
    }
    else if (LocalPos.X > 1500.0f)
    {
        // East: Rocky Plains
        return EWorld_BiomeType::RockyPlains;
    }
    else if (LocalPos.Y < -1500.0f)
    {
        // South: Savanna
        return EWorld_BiomeType::Savanna;
    }
    else if (LocalPos.X < -1500.0f)
    {
        // West: Swamp
        return EWorld_BiomeType::Swamp;
    }

    // Centre: Forest
    return EWorld_BiomeType::Forest;
}

FWorld_BiomeConfig ABiomeManager::GetBiomeConfig(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }

    // Return default Forest config if not found
    FWorld_BiomeConfig Default;
    Default.BiomeType   = EWorld_BiomeType::Forest;
    Default.DisplayName = TEXT("Default");
    return Default;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeConfig Config = GetBiomeConfig(Biome);
    return Config.AmbientTemperature;
}

float ABiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeConfig Config = GetBiomeConfig(Biome);
    return Config.Humidity;
}

bool ABiomeManager::IsWaterBiome(const FVector& WorldLocation) const
{
    const EWorld_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FWorld_BiomeConfig Config = GetBiomeConfig(Biome);
    return Config.bHasWater;
}

void ABiomeManager::DrawBiomeDebugZones()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Origin = GetActorLocation();
    const float R = BiomeDetectionRadius;

    // Draw each biome zone as a coloured arc
    DrawDebugCircle(World, Origin + FVector(0, 2500, 100), 1500.0f, 32, FColor::Green,  true, 30.0f, 0, 8.0f, FVector(1,0,0), FVector(0,1,0));
    DrawDebugCircle(World, Origin + FVector(2200, 0, 100),  1500.0f, 32, FColor::Orange, true, 30.0f, 0, 8.0f, FVector(1,0,0), FVector(0,1,0));
    DrawDebugCircle(World, Origin + FVector(0,-2200, 100),  1500.0f, 32, FColor::Yellow, true, 30.0f, 0, 8.0f, FVector(1,0,0), FVector(0,1,0));
    DrawDebugCircle(World, Origin + FVector(-2200,0, 100),  1500.0f, 32, FColor::Blue,   true, 30.0f, 0, 8.0f, FVector(1,0,0), FVector(0,1,0));
    DrawDebugCircle(World, Origin + FVector(0, 2000, 100),  800.0f,  32, FColor::Cyan,   true, 30.0f, 0, 8.0f, FVector(1,0,0), FVector(0,1,0));

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Debug zones drawn (Forest=Green, Rocky=Orange, Savanna=Yellow, Swamp=Blue, River=Cyan)"));
}

#include "BiomeManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick at 10Hz, internal timer controls actual update rate
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitDefaultConfigs();

    // Seed default biome zones if none configured in editor
    if (BiomeZones.Num() == 0)
    {
        // Forest zone — near origin
        FEng_BiomeZoneEntry Forest;
        Forest.WorldCenter = FVector(0.0f, 0.0f, 0.0f);
        Forest.Radius = 3000.0f;
        Forest.BiomeType = EEng_BiomeType::Forest;
        Forest.Config = DefaultConfigs[EEng_BiomeType::Forest];
        BiomeZones.Add(Forest);

        // Plains zone — east
        FEng_BiomeZoneEntry Plains;
        Plains.WorldCenter = FVector(5000.0f, 0.0f, 0.0f);
        Plains.Radius = 4000.0f;
        Plains.BiomeType = EEng_BiomeType::Plains;
        Plains.Config = DefaultConfigs[EEng_BiomeType::Plains];
        BiomeZones.Add(Plains);

        // Swamp zone — north
        FEng_BiomeZoneEntry Swamp;
        Swamp.WorldCenter = FVector(-2000.0f, 4000.0f, 0.0f);
        Swamp.Radius = 2500.0f;
        Swamp.BiomeType = EEng_BiomeType::Swamp;
        Swamp.Config = DefaultConfigs[EEng_BiomeType::Swamp];
        BiomeZones.Add(Swamp);

        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Seeded 3 default biome zones"));
    }

    // Initial player biome update
    UpdatePlayerBiome();
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        TimeSinceLastUpdate = 0.0f;
        UpdatePlayerBiome();
    }
}

void ABiomeManager::RegisterBiomeZone(const FEng_BiomeZoneEntry& Zone)
{
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered zone %s at (%.0f, %.0f, %.0f) r=%.0f"),
        *UEnum::GetValueAsString(Zone.BiomeType),
        Zone.WorldCenter.X, Zone.WorldCenter.Y, Zone.WorldCenter.Z,
        Zone.Radius);
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EEng_BiomeType ClosestBiome = DefaultBiome;

    for (const FEng_BiomeZoneEntry& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.WorldCenter);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

FEng_BiomeConfig ABiomeManager::GetBiomeConfig(EEng_BiomeType BiomeType) const
{
    // Check zone overrides first
    for (const FEng_BiomeZoneEntry& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return Zone.Config;
        }
    }

    // Fall back to defaults
    if (DefaultConfigs.Contains(BiomeType))
    {
        return DefaultConfigs[BiomeType];
    }

    // Return empty config
    return FEng_BiomeConfig();
}

void ABiomeManager::UpdatePlayerBiome()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar)
    {
        return;
    }

    FVector PlayerLocation = PlayerChar->GetActorLocation();
    EEng_BiomeType NewBiome = GetBiomeAtLocation(PlayerLocation);

    if (NewBiome != CurrentPlayerBiome)
    {
        EEng_BiomeType OldBiome = CurrentPlayerBiome;
        CurrentPlayerBiome = NewBiome;

        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Player entered biome %s (was %s)"),
            *UEnum::GetValueAsString(NewBiome),
            *UEnum::GetValueAsString(OldBiome));

        // Fire Blueprint event for visual/audio transitions
        OnPlayerEnteredBiome(NewBiome, OldBiome);
    }
}

void ABiomeManager::InitDefaultConfigs()
{
    // Forest — dense Cretaceous jungle
    FEng_BiomeConfig Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.BiomeName = TEXT("Cretaceous Forest");
    Forest.TemperatureBase = 28.0f;
    Forest.HumidityBase = 0.85f;
    Forest.VegetationDensity = 0.95f;
    Forest.DinosaurSpawnWeight = 1.2f;
    Forest.FogDensityMultiplier = 1.5f;
    Forest.AmbientColorTint = FLinearColor(0.6f, 0.9f, 0.5f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Forest, Forest);

    // Plains — open grassland, high visibility
    FEng_BiomeConfig Plains;
    Plains.BiomeType = EEng_BiomeType::Plains;
    Plains.BiomeName = TEXT("Open Plains");
    Plains.TemperatureBase = 32.0f;
    Plains.HumidityBase = 0.35f;
    Plains.VegetationDensity = 0.3f;
    Plains.DinosaurSpawnWeight = 1.5f;  // Herds visible on plains
    Plains.FogDensityMultiplier = 0.6f;
    Plains.AmbientColorTint = FLinearColor(1.0f, 0.95f, 0.75f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Plains, Plains);

    // Swamp — dangerous, dense fog, ambush predators
    FEng_BiomeConfig Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.BiomeName = TEXT("Swamp / Wetlands");
    Swamp.TemperatureBase = 35.0f;
    Swamp.HumidityBase = 0.95f;
    Swamp.VegetationDensity = 0.8f;
    Swamp.DinosaurSpawnWeight = 0.9f;
    Swamp.FogDensityMultiplier = 2.5f;
    Swamp.AmbientColorTint = FLinearColor(0.5f, 0.7f, 0.4f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Swamp, Swamp);

    // Volcanic — extreme heat, low life, unique hazards
    FEng_BiomeConfig Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName = TEXT("Volcanic Region");
    Volcanic.TemperatureBase = 55.0f;
    Volcanic.HumidityBase = 0.1f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.DinosaurSpawnWeight = 0.3f;
    Volcanic.FogDensityMultiplier = 3.0f;  // Ash/smoke
    Volcanic.AmbientColorTint = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Volcanic, Volcanic);

    // Coastal — beaches, shallow water, diverse fauna
    FEng_BiomeConfig Coastal;
    Coastal.BiomeType = EEng_BiomeType::Coastal;
    Coastal.BiomeName = TEXT("Coastal / Beach");
    Coastal.TemperatureBase = 27.0f;
    Coastal.HumidityBase = 0.7f;
    Coastal.VegetationDensity = 0.4f;
    Coastal.DinosaurSpawnWeight = 1.0f;
    Coastal.FogDensityMultiplier = 0.8f;
    Coastal.AmbientColorTint = FLinearColor(0.8f, 0.95f, 1.0f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Coastal, Coastal);

    // Canyon — rocky terrain, ambush points, scarce water
    FEng_BiomeConfig Canyon;
    Canyon.BiomeType = EEng_BiomeType::Canyon;
    Canyon.BiomeName = TEXT("Canyon / Badlands");
    Canyon.TemperatureBase = 38.0f;
    Canyon.HumidityBase = 0.15f;
    Canyon.VegetationDensity = 0.1f;
    Canyon.DinosaurSpawnWeight = 0.7f;
    Canyon.FogDensityMultiplier = 0.4f;
    Canyon.AmbientColorTint = FLinearColor(1.0f, 0.75f, 0.5f, 1.0f);
    DefaultConfigs.Add(EEng_BiomeType::Canyon, Canyon);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d default biome configs"), DefaultConfigs.Num());
}

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bAutoInitializeOnBeginPlay = true;
    BiomeTransitionBlendRadius = 500.0f;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoInitializeOnBeginPlay && BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome zones"), BiomeZones.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // River Delta — center of map
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::RiverDelta;
        Zone.BiomeName = TEXT("River Delta");
        Zone.Center = FVector(0.0f, 0.0f, -180.0f);
        Zone.Radius = 1500.0f;
        Zone.AmbientTemperature = 28.0f;
        Zone.Humidity = 0.85f;
        Zone.DangerLevel = 0.4f;
        BiomeZones.Add(Zone);
    }

    // Jungle Lowland — northwest quadrant
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::JungleLowland;
        Zone.BiomeName = TEXT("Jungle Lowland");
        Zone.Center = FVector(-3000.0f, 2000.0f, 0.0f);
        Zone.Radius = 2500.0f;
        Zone.AmbientTemperature = 32.0f;
        Zone.Humidity = 0.95f;
        Zone.DangerLevel = 0.7f;
        BiomeZones.Add(Zone);
    }

    // Rocky Highland — east quadrant
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::RockyHighland;
        Zone.BiomeName = TEXT("Rocky Highland");
        Zone.Center = FVector(3600.0f, -2000.0f, 200.0f);
        Zone.Radius = 2000.0f;
        Zone.AmbientTemperature = 18.0f;
        Zone.Humidity = 0.25f;
        Zone.DangerLevel = 0.5f;
        BiomeZones.Add(Zone);
    }

    // Volcanic Badland — far east
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::VolcanicBadland;
        Zone.BiomeName = TEXT("Volcanic Badland");
        Zone.Center = FVector(5200.0f, -3200.0f, 0.0f);
        Zone.Radius = 1800.0f;
        Zone.AmbientTemperature = 55.0f;
        Zone.Humidity = 0.05f;
        Zone.DangerLevel = 0.9f;
        BiomeZones.Add(Zone);
    }

    // Open Plains — south quadrant
    {
        FWorld_BiomeZone Zone;
        Zone.BiomeType = EWorld_BiomeType::OpenPlains;
        Zone.BiomeName = TEXT("Open Plains");
        Zone.Center = FVector(0.0f, -4000.0f, 0.0f);
        Zone.Radius = 3000.0f;
        Zone.AmbientTemperature = 24.0f;
        Zone.Humidity = 0.35f;
        Zone.DangerLevel = 0.3f;
        BiomeZones.Add(Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Default biomes initialized — %d zones"), BiomeZones.Num());
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    int32 Idx = FindNearestBiomeIndex(WorldLocation);
    if (Idx >= 0 && Idx < BiomeZones.Num())
    {
        return BiomeZones[Idx].BiomeType;
    }
    return EWorld_BiomeType::Unknown;
}

bool ABiomeManager::GetBiomeZoneAtLocation(const FVector& WorldLocation, FWorld_BiomeZone& OutZone) const
{
    int32 Idx = FindNearestBiomeIndex(WorldLocation);
    if (Idx >= 0 && Idx < BiomeZones.Num())
    {
        const FWorld_BiomeZone& Zone = BiomeZones[Idx];
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius + BiomeTransitionBlendRadius)
        {
            OutZone = Zone;
            return true;
        }
    }
    return false;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.AmbientTemperature;
    }
    return 22.0f; // Default ambient temperature
}

float ABiomeManager::GetDangerLevelAtLocation(const FVector& WorldLocation) const
{
    FWorld_BiomeZone Zone;
    if (GetBiomeZoneAtLocation(WorldLocation, Zone))
    {
        return Zone.DangerLevel;
    }
    return 0.2f; // Default low danger
}

void ABiomeManager::RegisterBiomeZone(const FWorld_BiomeZone& Zone)
{
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered new biome zone '%s'"), *Zone.BiomeName);
}

TArray<FWorld_BiomeZone> ABiomeManager::GetAllBiomeZones() const
{
    return BiomeZones;
}

int32 ABiomeManager::FindNearestBiomeIndex(const FVector& WorldLocation) const
{
    if (BiomeZones.Num() == 0)
    {
        return -1;
    }

    int32 NearestIdx = 0;
    float NearestDist = FVector::Dist2D(WorldLocation, BiomeZones[0].Center);

    for (int32 i = 1; i < BiomeZones.Num(); i++)
    {
        float Dist = FVector::Dist2D(WorldLocation, BiomeZones[i].Center);
        if (Dist < NearestDist)
        {
            NearestDist = Dist;
            NearestIdx = i;
        }
    }

    return NearestIdx;
}

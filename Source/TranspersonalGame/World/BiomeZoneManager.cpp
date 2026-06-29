#include "BiomeZoneManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ABiomeZoneManager::ABiomeZoneManager()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}

void ABiomeZoneManager::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateOnBeginPlay && BiomeZones.Num() == 0)
    {
        GenerateDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeZoneManager: Initialized with %d biome zones"), BiomeZones.Num());
}

void ABiomeZoneManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeZoneManager::GenerateDefaultBiomes()
{
    BiomeZones.Empty();

    // Forest biome — SW quadrant
    FWorld_BiomeZone Forest;
    Forest.BiomeType = EWorld_BiomeType::Forest;
    Forest.Center = FVector(-600.0f, -300.0f, 0.0f);
    Forest.Radius = 600.0f;
    Forest.VegetationDensity = 0.9f;
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.8f;
    Forest.bHasWater = false;
    BiomeZones.Add(Forest);

    // Plains biome — NW quadrant
    FWorld_BiomeZone Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.Center = FVector(-500.0f, 500.0f, 0.0f);
    Plains.Radius = 700.0f;
    Plains.VegetationDensity = 0.3f;
    Plains.Temperature = 28.0f;
    Plains.Humidity = 0.4f;
    Plains.bHasWater = false;
    BiomeZones.Add(Plains);

    // Rocky Badlands — NE quadrant
    FWorld_BiomeZone Badlands;
    Badlands.BiomeType = EWorld_BiomeType::RockyBadlands;
    Badlands.Center = FVector(600.0f, -400.0f, 0.0f);
    Badlands.Radius = 600.0f;
    Badlands.VegetationDensity = 0.1f;
    Badlands.Temperature = 35.0f;
    Badlands.Humidity = 0.15f;
    Badlands.bHasWater = false;
    BiomeZones.Add(Badlands);

    // Volcanic zone — SE quadrant
    FWorld_BiomeZone Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.Center = FVector(700.0f, 700.0f, 0.0f);
    Volcanic.Radius = 500.0f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.bHasWater = false;
    BiomeZones.Add(Volcanic);

    // River corridor — central band
    FWorld_BiomeZone River;
    River.BiomeType = EWorld_BiomeType::River;
    River.Center = FVector(0.0f, 400.0f, 0.0f);
    River.Radius = 300.0f;
    River.VegetationDensity = 0.7f;
    River.Temperature = 20.0f;
    River.Humidity = 0.95f;
    River.bHasWater = true;
    BiomeZones.Add(River);

    UE_LOG(LogTemp, Log, TEXT("BiomeZoneManager: Generated %d default biome zones"), BiomeZones.Num());
}

EWorld_BiomeType ABiomeZoneManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float BestInfluence = 0.0f;
    EWorld_BiomeType BestBiome = EWorld_BiomeType::Plains;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Influence = GetBiomeInfluence(Zone, WorldLocation);
        if (Influence > BestInfluence)
        {
            BestInfluence = Influence;
            BestBiome = Zone.BiomeType;
        }
    }

    return BestBiome;
}

FWorld_BiomeZone ABiomeZoneManager::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    float BestInfluence = 0.0f;
    int32 BestIndex = 0;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        float Influence = GetBiomeInfluence(BiomeZones[i], WorldLocation);
        if (Influence > BestInfluence)
        {
            BestInfluence = Influence;
            BestIndex = i;
        }
    }

    if (BiomeZones.IsValidIndex(BestIndex))
    {
        return BiomeZones[BestIndex];
    }

    return FWorld_BiomeZone();
}

float ABiomeZoneManager::GetVegetationDensityAt(const FVector& WorldLocation) const
{
    float TotalInfluence = 0.0f;
    float WeightedDensity = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Influence = GetBiomeInfluence(Zone, WorldLocation);
        WeightedDensity += Zone.VegetationDensity * Influence;
        TotalInfluence += Influence;
    }

    return (TotalInfluence > 0.0f) ? (WeightedDensity / TotalInfluence) : 0.0f;
}

float ABiomeZoneManager::GetTemperatureAt(const FVector& WorldLocation) const
{
    float TotalInfluence = 0.0f;
    float WeightedTemp = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Influence = GetBiomeInfluence(Zone, WorldLocation);
        WeightedTemp += Zone.Temperature * Influence;
        TotalInfluence += Influence;
    }

    return (TotalInfluence > 0.0f) ? (WeightedTemp / TotalInfluence) : 25.0f;
}

bool ABiomeZoneManager::IsLocationNearWater(const FVector& WorldLocation, float SearchRadius) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (!Zone.bHasWater) continue;
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius + SearchRadius)
        {
            return true;
        }
    }
    return false;
}

void ABiomeZoneManager::RegisterBiomeZone(const FWorld_BiomeZone& Zone)
{
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeZoneManager: Registered biome zone '%s' at (%s)"),
        *BiomeTypeToString(Zone.BiomeType), *Zone.Center.ToString());
}

void ABiomeZoneManager::ClearAllBiomes()
{
    BiomeZones.Empty();
    UE_LOG(LogTemp, Log, TEXT("BiomeZoneManager: All biome zones cleared"));
}

void ABiomeZoneManager::DrawBiomeBoundaries(float Duration) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        FColor Color = FColor::Green;
        switch (Zone.BiomeType)
        {
            case EWorld_BiomeType::Forest:       Color = FColor(34, 139, 34);  break;
            case EWorld_BiomeType::Plains:       Color = FColor(154, 205, 50); break;
            case EWorld_BiomeType::RockyBadlands: Color = FColor(139, 90, 43); break;
            case EWorld_BiomeType::Volcanic:     Color = FColor(220, 50, 10);  break;
            case EWorld_BiomeType::River:        Color = FColor(30, 144, 255); break;
            default: break;
        }

        DrawDebugSphere(World, Zone.Center, Zone.Radius, 32, Color, false, Duration, 0, 5.0f);
        DrawDebugString(World, Zone.Center + FVector(0, 0, 50),
            BiomeTypeToString(Zone.BiomeType), nullptr, Color, Duration);
    }
}

FString ABiomeZoneManager::GetBiomeNameAtLocation(const FVector& WorldLocation) const
{
    return BiomeTypeToString(GetBiomeAtLocation(WorldLocation));
}

float ABiomeZoneManager::GetBiomeInfluence(const FWorld_BiomeZone& Zone, const FVector& Location) const
{
    float Dist = FVector::Dist2D(Location, Zone.Center);

    if (Dist >= Zone.Radius + BlendRadius)
    {
        return 0.0f;
    }

    if (Dist <= Zone.Radius - BlendRadius)
    {
        return 1.0f;
    }

    // Smooth blend in the border region
    float BlendFactor = (Zone.Radius + BlendRadius - Dist) / (2.0f * BlendRadius);
    return FMath::Clamp(BlendFactor, 0.0f, 1.0f);
}

FString ABiomeZoneManager::BiomeTypeToString(EWorld_BiomeType Type)
{
    switch (Type)
    {
        case EWorld_BiomeType::Forest:        return TEXT("Forest");
        case EWorld_BiomeType::Plains:        return TEXT("Plains");
        case EWorld_BiomeType::RockyBadlands: return TEXT("Rocky Badlands");
        case EWorld_BiomeType::Volcanic:      return TEXT("Volcanic");
        case EWorld_BiomeType::River:         return TEXT("River");
        default:                              return TEXT("Unknown");
    }
}

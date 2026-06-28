// BiomeManager.cpp
// Transpersonal Game Studio — Engine Architect #02
// Manages biome zones, transitions, and environmental parameters for the prehistoric world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    // Default biome transition parameters
    BiomeTransitionBlendRadius = 2000.0f;
    bEnableBiomeDebugDraw = false;
    CurrentDominantBiome = EEng_BiomeType::Jungle;
    BiomeUpdateInterval = 1.0f;
    bBiomesInitialized = false;
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeDefaultBiomes();
    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome zones"), BiomeZones.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeZones.Empty();
    Super::Deinitialize();
}

void UBiomeManager::InitializeDefaultBiomes()
{
    // Jungle biome — dense vegetation, high humidity, warm
    FEng_BiomeZone JungleBiome;
    JungleBiome.BiomeType = EEng_BiomeType::Jungle;
    JungleBiome.BiomeName = TEXT("Cretaceous Jungle");
    JungleBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    JungleBiome.Radius = 8000.0f;
    JungleBiome.Temperature = 32.0f;
    JungleBiome.Humidity = 0.85f;
    JungleBiome.FogDensity = 0.04f;
    JungleBiome.FogColor = FLinearColor(0.6f, 0.8f, 0.5f, 1.0f);
    JungleBiome.AmbientLightIntensity = 0.7f;
    JungleBiome.VegetationDensity = 0.9f;
    JungleBiome.bHasWater = true;
    JungleBiome.WaterLevel = -50.0f;
    JungleBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Velociraptor);
    JungleBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Spinosaurus);
    BiomeZones.Add(JungleBiome);

    // Plains biome — open grassland, moderate temperature
    FEng_BiomeZone PlainsBiome;
    PlainsBiome.BiomeType = EEng_BiomeType::Plains;
    PlainsBiome.BiomeName = TEXT("Prehistoric Savanna");
    PlainsBiome.CenterLocation = FVector(12000.0f, 0.0f, 0.0f);
    PlainsBiome.Radius = 10000.0f;
    PlainsBiome.Temperature = 28.0f;
    PlainsBiome.Humidity = 0.35f;
    PlainsBiome.FogDensity = 0.01f;
    PlainsBiome.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);
    PlainsBiome.AmbientLightIntensity = 1.0f;
    PlainsBiome.VegetationDensity = 0.4f;
    PlainsBiome.bHasWater = false;
    PlainsBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::TyrannosaurusRex);
    PlainsBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Triceratops);
    PlainsBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Brachiosaurus);
    BiomeZones.Add(PlainsBiome);

    // Swamp biome — murky water, high fog, dangerous
    FEng_BiomeZone SwampBiome;
    SwampBiome.BiomeType = EEng_BiomeType::Swamp;
    SwampBiome.BiomeName = TEXT("Primordial Swamp");
    SwampBiome.CenterLocation = FVector(-8000.0f, 5000.0f, -100.0f);
    SwampBiome.Radius = 6000.0f;
    SwampBiome.Temperature = 35.0f;
    SwampBiome.Humidity = 0.95f;
    SwampBiome.FogDensity = 0.08f;
    SwampBiome.FogColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
    SwampBiome.AmbientLightIntensity = 0.5f;
    SwampBiome.VegetationDensity = 0.7f;
    SwampBiome.bHasWater = true;
    SwampBiome.WaterLevel = 0.0f;
    SwampBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Spinosaurus);
    BiomeZones.Add(SwampBiome);

    // Volcanic biome — extreme heat, ash, dangerous terrain
    FEng_BiomeZone VolcanicBiome;
    VolcanicBiome.BiomeType = EEng_BiomeType::Volcanic;
    VolcanicBiome.BiomeName = TEXT("Volcanic Badlands");
    VolcanicBiome.CenterLocation = FVector(0.0f, -15000.0f, 500.0f);
    VolcanicBiome.Radius = 5000.0f;
    VolcanicBiome.Temperature = 55.0f;
    VolcanicBiome.Humidity = 0.1f;
    VolcanicBiome.FogDensity = 0.06f;
    VolcanicBiome.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    VolcanicBiome.AmbientLightIntensity = 0.8f;
    VolcanicBiome.VegetationDensity = 0.05f;
    VolcanicBiome.bHasWater = false;
    VolcanicBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Ankylosaurus);
    BiomeZones.Add(VolcanicBiome);

    // Coastal biome — beaches, cliffs, ocean edge
    FEng_BiomeZone CoastalBiome;
    CoastalBiome.BiomeType = EEng_BiomeType::Coastal;
    CoastalBiome.BiomeName = TEXT("Cretaceous Coast");
    CoastalBiome.CenterLocation = FVector(-5000.0f, -10000.0f, 0.0f);
    CoastalBiome.Radius = 7000.0f;
    CoastalBiome.Temperature = 26.0f;
    CoastalBiome.Humidity = 0.6f;
    CoastalBiome.FogDensity = 0.02f;
    CoastalBiome.FogColor = FLinearColor(0.7f, 0.85f, 0.9f, 1.0f);
    CoastalBiome.AmbientLightIntensity = 0.95f;
    CoastalBiome.VegetationDensity = 0.3f;
    CoastalBiome.bHasWater = true;
    CoastalBiome.WaterLevel = -200.0f;
    CoastalBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Pterodactyl);
    BiomeZones.Add(CoastalBiome);

    // Mountain biome — rocky, cold, high altitude
    FEng_BiomeZone MountainBiome;
    MountainBiome.BiomeType = EEng_BiomeType::Mountain;
    MountainBiome.BiomeName = TEXT("Highland Peaks");
    MountainBiome.CenterLocation = FVector(8000.0f, -8000.0f, 1000.0f);
    MountainBiome.Radius = 6000.0f;
    MountainBiome.Temperature = 12.0f;
    MountainBiome.Humidity = 0.3f;
    MountainBiome.FogDensity = 0.03f;
    MountainBiome.FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    MountainBiome.AmbientLightIntensity = 0.9f;
    MountainBiome.VegetationDensity = 0.2f;
    MountainBiome.bHasWater = false;
    MountainBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Pterodactyl);
    MountainBiome.DominantDinosaurs.Add(EEng_DinosaurSpecies::Stegosaurus);
    BiomeZones.Add(MountainBiome);
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = MAX_FLT;
    EEng_BiomeType DominantBiome = EEng_BiomeType::Jungle;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Distance < Zone.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            DominantBiome = Zone.BiomeType;
        }
    }

    return DominantBiome;
}

FEng_BiomeZone UBiomeManager::GetBiomeZoneAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistance = MAX_FLT;
    FEng_BiomeZone Result;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Distance < Zone.Radius && Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            Result = Zone;
        }
    }

    return Result;
}

float UBiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    // Blend temperatures from nearby biomes
    float TotalWeight = 0.0f;
    float BlendedTemp = 28.0f; // default

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Distance < Zone.Radius + BiomeTransitionBlendRadius)
        {
            float Weight = FMath::Max(0.0f, 1.0f - (Distance / (Zone.Radius + BiomeTransitionBlendRadius)));
            BlendedTemp += Zone.Temperature * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        BlendedTemp /= TotalWeight;
    }

    return BlendedTemp;
}

float UBiomeManager::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    float TotalWeight = 0.0f;
    float BlendedHumidity = 0.5f;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Distance < Zone.Radius + BiomeTransitionBlendRadius)
        {
            float Weight = FMath::Max(0.0f, 1.0f - (Distance / (Zone.Radius + BiomeTransitionBlendRadius)));
            BlendedHumidity += Zone.Humidity * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        BlendedHumidity /= TotalWeight;
    }

    return FMath::Clamp(BlendedHumidity, 0.0f, 1.0f);
}

bool UBiomeManager::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
            if (Distance < Zone.Radius)
            {
                return true;
            }
        }
    }
    return false;
}

TArray<EEng_DinosaurSpecies> UBiomeManager::GetDinosaursForBiome(EEng_BiomeType BiomeType) const
{
    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            return Zone.DominantDinosaurs;
        }
    }
    return TArray<EEng_DinosaurSpecies>();
}

void UBiomeManager::RegisterBiomeZone(const FEng_BiomeZone& NewZone)
{
    // Check for duplicate
    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        if (BiomeZones[i].BiomeType == NewZone.BiomeType &&
            FVector::Dist(BiomeZones[i].CenterLocation, NewZone.CenterLocation) < 100.0f)
        {
            BiomeZones[i] = NewZone; // Update existing
            return;
        }
    }
    BiomeZones.Add(NewZone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered new biome zone '%s'"), *NewZone.BiomeName);
}

int32 UBiomeManager::GetBiomeZoneCount() const
{
    return BiomeZones.Num();
}

FLinearColor UBiomeManager::GetFogColorAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeZone Zone = GetBiomeZoneAtLocation(WorldLocation);
    return Zone.FogColor;
}

float UBiomeManager::GetFogDensityAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeZone Zone = GetBiomeZoneAtLocation(WorldLocation);
    return Zone.FogDensity;
}

// BiomeSystem.cpp
// Agent #05 — Procedural World Generator
// Implementation of biome classification and zone management

#include "BiomeSystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"

// ============================================================
// UBiomeDetectorComponent
// ============================================================

UBiomeDetectorComponent::UBiomeDetectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = BiomeCheckInterval;
    CurrentBiome = EWorld_BiomeType::OpenPlains;
    BiomeTransitionAlpha = 1.0f;
    LastBiomeCheckTime = 0.0f;
}

void UBiomeDetectorComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UBiomeDetectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Query BiomeManager for current location
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector OwnerLocation = Owner->GetActorLocation();

    // Find BiomeManager in world
    for (TActorIterator<ABiomeManager> It(World); It; ++It)
    {
        ABiomeManager* Manager = *It;
        if (Manager)
        {
            EWorld_BiomeType NewBiome = Manager->GetBiomeAtLocation(OwnerLocation);
            if (NewBiome != CurrentBiome)
            {
                CurrentBiome = NewBiome;
                BiomeTransitionAlpha = 0.0f;
            }
            break;
        }
    }

    // Smooth transition
    if (BiomeTransitionAlpha < 1.0f)
    {
        BiomeTransitionAlpha = FMath::Min(1.0f, BiomeTransitionAlpha + DeltaTime * 0.5f);
    }
}

EWorld_BiomeType UBiomeDetectorComponent::GetCurrentBiome() const
{
    return CurrentBiome;
}

FWorld_BiomeConfig UBiomeDetectorComponent::GetCurrentBiomeConfig() const
{
    UWorld* World = GetWorld();
    if (!World) return FWorld_BiomeConfig();

    AActor* Owner = GetOwner();
    if (!Owner) return FWorld_BiomeConfig();

    FVector OwnerLocation = Owner->GetActorLocation();

    for (TActorIterator<ABiomeManager> It(World); It; ++It)
    {
        ABiomeManager* Manager = *It;
        if (Manager)
        {
            return Manager->GetBiomeConfigAtLocation(OwnerLocation);
        }
    }

    return FWorld_BiomeConfig();
}

float UBiomeDetectorComponent::GetDistanceToNearestWater() const
{
    UWorld* World = GetWorld();
    if (!World) return TNumericLimits<float>::Max();

    AActor* Owner = GetOwner();
    if (!Owner) return TNumericLimits<float>::Max();

    FVector OwnerLocation = Owner->GetActorLocation();
    float MinDist = TNumericLimits<float>::Max();

    for (TActorIterator<ABiomeManager> It(World); It; ++It)
    {
        ABiomeManager* Manager = *It;
        if (!Manager) continue;

        for (const FWorld_BiomeZone& Zone : Manager->GetAllBiomeZones())
        {
            if (Zone.Config.bHasWater)
            {
                float Dist = FVector::Dist(OwnerLocation, Zone.CenterLocation);
                MinDist = FMath::Min(MinDist, Dist);
            }
        }
    }

    return MinDist;
}

bool UBiomeDetectorComponent::IsInDangerousZone() const
{
    return CurrentBiome == EWorld_BiomeType::VolcanicBadlands ||
           CurrentBiome == EWorld_BiomeType::SwampWetlands;
}

// ============================================================
// ABiomeManager
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default config for fallback
    DefaultConfig.BiomeType = EWorld_BiomeType::OpenPlains;
    DefaultConfig.FogColor = FLinearColor(0.6f, 0.65f, 0.6f, 1.0f);
    DefaultConfig.FogDensity = 0.015f;
    DefaultConfig.AmbientTemperature = 25.0f;
    DefaultConfig.HumidityLevel = 0.5f;
    DefaultConfig.VegetationDensity = 0.5f;
    DefaultConfig.bHasWater = false;
    DefaultConfig.DinosaurSpawnMultiplier = 1.0f;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // Cretaceous Jungle — dense forest zone
    {
        FWorld_BiomeZone Zone;
        Zone.ZoneName = TEXT("CretaceousJungle");
        Zone.CenterLocation = FVector(-2500.0f, -2500.0f, 0.0f);
        Zone.Radius = 4000.0f;
        Zone.Config.BiomeType = EWorld_BiomeType::CretaceousJungle;
        Zone.Config.FogColor = FLinearColor(0.15f, 0.35f, 0.1f, 1.0f);
        Zone.Config.FogDensity = 0.045f;
        Zone.Config.AmbientTemperature = 32.0f;
        Zone.Config.HumidityLevel = 0.95f;
        Zone.Config.VegetationDensity = 2.0f;
        Zone.Config.bHasWater = false;
        Zone.Config.DinosaurSpawnMultiplier = 1.8f;
        Zone.Config.AmbientSoundCue = TEXT("JungleAmbient");
        BiomeZones.Add(Zone);
    }

    // Rocky Highlands
    {
        FWorld_BiomeZone Zone;
        Zone.ZoneName = TEXT("RockyHighlands");
        Zone.CenterLocation = FVector(3000.0f, 2000.0f, 300.0f);
        Zone.Radius = 3500.0f;
        Zone.Config.BiomeType = EWorld_BiomeType::RockyHighlands;
        Zone.Config.FogColor = FLinearColor(0.5f, 0.45f, 0.35f, 1.0f);
        Zone.Config.FogDensity = 0.01f;
        Zone.Config.AmbientTemperature = 18.0f;
        Zone.Config.HumidityLevel = 0.2f;
        Zone.Config.VegetationDensity = 0.3f;
        Zone.Config.bHasWater = false;
        Zone.Config.DinosaurSpawnMultiplier = 0.7f;
        Zone.Config.AmbientSoundCue = TEXT("WindRocksAmbient");
        BiomeZones.Add(Zone);
    }

    // River Valley
    {
        FWorld_BiomeZone Zone;
        Zone.ZoneName = TEXT("RiverValley");
        Zone.CenterLocation = FVector(0.0f, 3000.0f, -50.0f);
        Zone.Radius = 2500.0f;
        Zone.Config.BiomeType = EWorld_BiomeType::RiverValley;
        Zone.Config.FogColor = FLinearColor(0.2f, 0.4f, 0.55f, 1.0f);
        Zone.Config.FogDensity = 0.035f;
        Zone.Config.AmbientTemperature = 24.0f;
        Zone.Config.HumidityLevel = 0.85f;
        Zone.Config.VegetationDensity = 1.5f;
        Zone.Config.bHasWater = true;
        Zone.Config.DinosaurSpawnMultiplier = 1.4f;
        Zone.Config.AmbientSoundCue = TEXT("RiverAmbient");
        BiomeZones.Add(Zone);
    }

    // Open Plains
    {
        FWorld_BiomeZone Zone;
        Zone.ZoneName = TEXT("OpenPlains");
        Zone.CenterLocation = FVector(2000.0f, -2000.0f, 0.0f);
        Zone.Radius = 4500.0f;
        Zone.Config.BiomeType = EWorld_BiomeType::OpenPlains;
        Zone.Config.FogColor = FLinearColor(0.7f, 0.7f, 0.55f, 1.0f);
        Zone.Config.FogDensity = 0.008f;
        Zone.Config.AmbientTemperature = 30.0f;
        Zone.Config.HumidityLevel = 0.35f;
        Zone.Config.VegetationDensity = 0.6f;
        Zone.Config.bHasWater = false;
        Zone.Config.DinosaurSpawnMultiplier = 1.2f;
        Zone.Config.AmbientSoundCue = TEXT("PlainsAmbient");
        BiomeZones.Add(Zone);
    }

    // Swamp Wetlands
    {
        FWorld_BiomeZone Zone;
        Zone.ZoneName = TEXT("SwampWetlands");
        Zone.CenterLocation = FVector(-4000.0f, 0.0f, -80.0f);
        Zone.Radius = 2000.0f;
        Zone.Config.BiomeType = EWorld_BiomeType::SwampWetlands;
        Zone.Config.FogColor = FLinearColor(0.1f, 0.25f, 0.15f, 1.0f);
        Zone.Config.FogDensity = 0.06f;
        Zone.Config.AmbientTemperature = 35.0f;
        Zone.Config.HumidityLevel = 0.99f;
        Zone.Config.VegetationDensity = 1.8f;
        Zone.Config.bHasWater = true;
        Zone.Config.DinosaurSpawnMultiplier = 1.6f;
        Zone.Config.AmbientSoundCue = TEXT("SwampAmbient");
        BiomeZones.Add(Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome zones"), BiomeZones.Num());
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(FVector Location) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(Location, Zone.CenterLocation);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.Config.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeConfig ABiomeManager::GetBiomeConfigAtLocation(FVector Location) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    const FWorld_BiomeConfig* ClosestConfig = nullptr;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(Location, Zone.CenterLocation);
        if (Dist <= Zone.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestConfig = &Zone.Config;
        }
    }

    return ClosestConfig ? *ClosestConfig : DefaultConfig;
}

void ABiomeManager::RegisterBiomeZone(const FWorld_BiomeZone& Zone)
{
    BiomeZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered zone '%s'"), *Zone.ZoneName);
}

TArray<FWorld_BiomeZone> ABiomeManager::GetAllBiomeZones() const
{
    return BiomeZones;
}

int32 ABiomeManager::GetBiomeZoneCount() const
{
    return BiomeZones.Num();
}

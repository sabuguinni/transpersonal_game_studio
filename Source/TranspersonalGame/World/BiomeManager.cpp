#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager* ABiomeManager::CachedInstance = nullptr;

// ─── ABiomeManager ────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    InitializeDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    CachedInstance = this;
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitializeDefaultBiomes()
{
    // Jungle — hot, wet, dense
    FEng_BiomeData Jungle;
    Jungle.BiomeType        = EEng_BiomeType::Jungle;
    Jungle.Temperature      = 32.0f;
    Jungle.Humidity         = 0.9f;
    Jungle.FoliageDensity   = 0.95f;
    Jungle.DinosaurSpawnWeight = 1.5f;
    Jungle.FogColor         = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
    Jungle.FogDensity       = 0.04f;
    BiomeTable.Add(EEng_BiomeType::Jungle, Jungle);

    // Savanna — hot, dry, open
    FEng_BiomeData Savanna;
    Savanna.BiomeType       = EEng_BiomeType::Savanna;
    Savanna.Temperature     = 35.0f;
    Savanna.Humidity        = 0.2f;
    Savanna.FoliageDensity  = 0.2f;
    Savanna.DinosaurSpawnWeight = 2.0f;
    Savanna.FogColor        = FLinearColor(0.8f, 0.7f, 0.4f, 1.0f);
    Savanna.FogDensity      = 0.01f;
    BiomeTable.Add(EEng_BiomeType::Savanna, Savanna);

    // Swamp — warm, very wet
    FEng_BiomeData Swamp;
    Swamp.BiomeType         = EEng_BiomeType::Swamp;
    Swamp.Temperature       = 25.0f;
    Swamp.Humidity          = 0.95f;
    Swamp.FoliageDensity    = 0.7f;
    Swamp.DinosaurSpawnWeight = 1.2f;
    Swamp.FogColor          = FLinearColor(0.2f, 0.3f, 0.15f, 1.0f);
    Swamp.FogDensity        = 0.06f;
    BiomeTable.Add(EEng_BiomeType::Swamp, Swamp);

    // Volcanic — extreme heat, sparse
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType      = EEng_BiomeType::Volcanic;
    Volcanic.Temperature    = 55.0f;
    Volcanic.Humidity       = 0.05f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.DinosaurSpawnWeight = 0.5f;
    Volcanic.FogColor       = FLinearColor(0.6f, 0.2f, 0.1f, 1.0f);
    Volcanic.FogDensity     = 0.08f;
    BiomeTable.Add(EEng_BiomeType::Volcanic, Volcanic);

    // Coastal — moderate, humid
    FEng_BiomeData Coastal;
    Coastal.BiomeType       = EEng_BiomeType::Coastal;
    Coastal.Temperature     = 22.0f;
    Coastal.Humidity        = 0.7f;
    Coastal.FoliageDensity  = 0.4f;
    Coastal.DinosaurSpawnWeight = 1.0f;
    Coastal.FogColor        = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    Coastal.FogDensity      = 0.03f;
    BiomeTable.Add(EEng_BiomeType::Coastal, Coastal);

    // Forest — temperate, moderate
    FEng_BiomeData Forest;
    Forest.BiomeType        = EEng_BiomeType::Forest;
    Forest.Temperature      = 18.0f;
    Forest.Humidity         = 0.6f;
    Forest.FoliageDensity   = 0.8f;
    Forest.DinosaurSpawnWeight = 1.3f;
    Forest.FogColor         = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
    Forest.FogDensity       = 0.025f;
    BiomeTable.Add(EEng_BiomeType::Forest, Forest);

    // Grassland — open, moderate
    FEng_BiomeData Grassland;
    Grassland.BiomeType     = EEng_BiomeType::Grassland;
    Grassland.Temperature   = 20.0f;
    Grassland.Humidity      = 0.45f;
    Grassland.FoliageDensity = 0.3f;
    Grassland.DinosaurSpawnWeight = 1.8f;
    Grassland.FogColor      = FLinearColor(0.6f, 0.7f, 0.5f, 1.0f);
    Grassland.FogDensity    = 0.015f;
    BiomeTable.Add(EEng_BiomeType::Grassland, Grassland);

    // Desert — extreme heat, arid
    FEng_BiomeData Desert;
    Desert.BiomeType        = EEng_BiomeType::Desert;
    Desert.Temperature      = 45.0f;
    Desert.Humidity         = 0.05f;
    Desert.FoliageDensity   = 0.05f;
    Desert.DinosaurSpawnWeight = 0.6f;
    Desert.FogColor         = FLinearColor(0.9f, 0.8f, 0.5f, 1.0f);
    Desert.FogDensity       = 0.005f;
    BiomeTable.Add(EEng_BiomeType::Desert, Desert);
}

EEng_BiomeType ABiomeManager::SampleBiomeFromNoise(float X, float Y) const
{
    // Simple deterministic biome sampling using sine-based pseudo-noise
    // Real implementation would use FPerlinNoise or a noise texture
    float NX = X * BiomeNoiseScale + BiomeSeed * 0.001f;
    float NY = Y * BiomeNoiseScale + BiomeSeed * 0.002f;

    float NoiseA = FMath::Sin(NX * 3.7f + NY * 2.1f) * 0.5f + 0.5f;
    float NoiseB = FMath::Sin(NX * 1.3f - NY * 4.5f) * 0.5f + 0.5f;

    // Map 2D noise to biome index
    int32 BiomeIndex = FMath::FloorToInt((NoiseA * 0.6f + NoiseB * 0.4f) * 8.0f);
    BiomeIndex = FMath::Clamp(BiomeIndex, 0, 7);

    static const EEng_BiomeType BiomeOrder[] = {
        EEng_BiomeType::Grassland,
        EEng_BiomeType::Forest,
        EEng_BiomeType::Jungle,
        EEng_BiomeType::Swamp,
        EEng_BiomeType::Coastal,
        EEng_BiomeType::Savanna,
        EEng_BiomeType::Desert,
        EEng_BiomeType::Volcanic,
    };

    return BiomeOrder[BiomeIndex];
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    return SampleBiomeFromNoise(WorldLocation.X, WorldLocation.Y);
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeAtLocation(WorldLocation);
    if (const FEng_BiomeData* Data = BiomeTable.Find(BiomeType))
    {
        return *Data;
    }
    // Return default grassland if not found
    return FEng_BiomeData();
}

bool ABiomeManager::IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

void ABiomeManager::RegisterBiome(EEng_BiomeType BiomeType, const FEng_BiomeData& BiomeData)
{
    BiomeTable.Add(BiomeType, BiomeData);
}

void ABiomeManager::SetBiomeNoiseScale(float Scale)
{
    BiomeNoiseScale = FMath::Max(0.00001f, Scale);
}

ABiomeManager* ABiomeManager::GetInstance(UObject* WorldContextObject)
{
    if (CachedInstance)
    {
        return CachedInstance;
    }

    if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
    {
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(World, ABiomeManager::StaticClass(), Found);
        if (Found.Num() > 0)
        {
            CachedInstance = Cast<ABiomeManager>(Found[0]);
        }
    }

    return CachedInstance;
}

// ─── UBiomeQueryComponent ─────────────────────────────────────────────────────

UBiomeQueryComponent::UBiomeQueryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

EEng_BiomeType UBiomeQueryComponent::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    ABiomeManager* Manager = ABiomeManager::GetInstance(GetOwner());
    if (Manager)
    {
        return Manager->GetBiomeAtLocation(WorldLocation);
    }
    return EEng_BiomeType::Grassland;
}

FEng_BiomeData UBiomeQueryComponent::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    ABiomeManager* Manager = ABiomeManager::GetInstance(GetOwner());
    if (Manager)
    {
        return Manager->GetBiomeDataAtLocation(WorldLocation);
    }
    return FEng_BiomeData();
}

float UBiomeQueryComponent::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).Temperature;
}

float UBiomeQueryComponent::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).Humidity;
}

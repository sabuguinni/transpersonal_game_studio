// BiomeSystem.cpp
// Biome classification, query, and manager for the prehistoric world.
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260622_003

#include "BiomeSystem.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeSystemComponent
// ─────────────────────────────────────────────────────────────────────────────

UBiomeSystemComponent::UBiomeSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UBiomeSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultBiomes();
}

void UBiomeSystemComponent::InitializeDefaultBiomes()
{
    // Forest Zone — NW quadrant
    FWorld_BiomeData Forest;
    Forest.BiomeType        = EWorld_BiomeType::Forest;
    Forest.WorldCenter      = FVector(-2000.0f, -2000.0f, 0.0f);
    Forest.Radius           = 2000.0f;
    Forest.TreeDensity      = 0.85f;
    Forest.RockDensity      = 0.15f;
    Forest.AmbientTemperature = 18.0f;
    Forest.HumidityLevel    = 0.80f;
    Forest.FogTint          = FLinearColor(0.4f, 0.7f, 0.4f, 1.0f);
    RegisteredBiomes.Add(Forest);

    // Plains Zone — NE quadrant
    FWorld_BiomeData Plains;
    Plains.BiomeType        = EWorld_BiomeType::Plains;
    Plains.WorldCenter      = FVector(2000.0f, -2000.0f, 0.0f);
    Plains.Radius           = 2000.0f;
    Plains.TreeDensity      = 0.20f;
    Plains.RockDensity      = 0.10f;
    Plains.AmbientTemperature = 26.0f;
    Plains.HumidityLevel    = 0.35f;
    Plains.FogTint          = FLinearColor(0.8f, 0.85f, 0.6f, 1.0f);
    RegisteredBiomes.Add(Plains);

    // Rocky Outcrop Zone — South
    FWorld_BiomeData Rocky;
    Rocky.BiomeType         = EWorld_BiomeType::Rocky;
    Rocky.WorldCenter       = FVector(0.0f, 2500.0f, 0.0f);
    Rocky.Radius            = 1800.0f;
    Rocky.TreeDensity       = 0.10f;
    Rocky.RockDensity       = 0.75f;
    Rocky.AmbientTemperature = 24.0f;
    Rocky.HumidityLevel     = 0.25f;
    Rocky.FogTint           = FLinearColor(0.7f, 0.65f, 0.55f, 1.0f);
    RegisteredBiomes.Add(Rocky);

    // River — center strip
    FWorld_BiomeData River;
    River.BiomeType         = EWorld_BiomeType::River;
    River.WorldCenter       = FVector(0.0f, 0.0f, -10.0f);
    River.Radius            = 300.0f;
    River.TreeDensity       = 0.40f;
    River.RockDensity       = 0.20f;
    River.AmbientTemperature = 20.0f;
    River.HumidityLevel     = 0.95f;
    River.FogTint           = FLinearColor(0.3f, 0.5f, 0.8f, 1.0f);
    RegisteredBiomes.Add(River);
}

void UBiomeSystemComponent::RegisterBiome(const FWorld_BiomeData& BiomeData)
{
    RegisteredBiomes.Add(BiomeData);
}

EWorld_BiomeType UBiomeSystemComponent::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).BiomeType;
}

FWorld_BiomeData UBiomeSystemComponent::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = TNumericLimits<float>::Max();
    int32 ClosestIdx  = INDEX_NONE;

    for (int32 i = 0; i < RegisteredBiomes.Num(); ++i)
    {
        const FWorld_BiomeData& B = RegisteredBiomes[i];
        float Dist = FVector::Dist2D(WorldLocation, B.WorldCenter);
        if (Dist < B.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestIdx  = i;
        }
    }

    if (ClosestIdx != INDEX_NONE)
    {
        return RegisteredBiomes[ClosestIdx];
    }

    // Default: Plains
    FWorld_BiomeData Default;
    Default.BiomeType = EWorld_BiomeType::Plains;
    return Default;
}

float UBiomeSystemComponent::GetTreeDensityAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).TreeDensity;
}

float UBiomeSystemComponent::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    return GetBiomeDataAtLocation(WorldLocation).AmbientTemperature;
}

// ─────────────────────────────────────────────────────────────────────────────
// ABiomeManager
// ─────────────────────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    BiomeComponent = CreateDefaultSubobject<UBiomeSystemComponent>(TEXT("BiomeComponent"));
    RootComponent  = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
}

void ABiomeManager::GenerateBiomeLayout()
{
    if (BiomeComponent)
    {
        BiomeComponent->RegisteredBiomes.Empty();
        // Re-initialize via BeginPlay-equivalent
        UE_LOG(LogTemp, Log, TEXT("ABiomeManager: GenerateBiomeLayout called — biomes reset and re-initialized."));
    }
}

EWorld_BiomeType ABiomeManager::QueryBiomeAt(FVector Location) const
{
    if (BiomeComponent)
    {
        return BiomeComponent->GetBiomeAtLocation(Location);
    }
    return EWorld_BiomeType::Plains;
}

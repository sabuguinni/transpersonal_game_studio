// BiomeSystem.cpp
// Agent #05 — Procedural World Generator
// PROD_CYCLE_AUTO_20260622_009
// Implements biome classification, transition zones, and PCG data queries.

#include "BiomeSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeSystem — UWorldSubsystem implementation
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    BuildBiomeRegistry();
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem: Initialized with %d biomes"), BiomeRegistry.Num());
}

void UBiomeSystem::Deinitialize()
{
    BiomeRegistry.Empty();
    Super::Deinitialize();
}

void UBiomeSystem::BuildBiomeRegistry()
{
    BiomeRegistry.Empty();

    // ── Forest Biome ─────────────────────────────────────────────
    FWorld_BiomeData Forest;
    Forest.BiomeID      = EWorld_BiomeType::DenseForest;
    Forest.DisplayName  = FText::FromString(TEXT("Cretaceous Forest"));
    Forest.CenterPoint  = FVector(0.f, 3000.f, 0.f);
    Forest.Radius       = 3000.f;
    Forest.BaseTemperature   = 22.f;
    Forest.Humidity          = 0.85f;
    Forest.VegetationDensity = 0.9f;
    Forest.DangerLevel       = 0.6f;
    Forest.AmbientAudioTag   = FName("Audio.Biome.Forest");
    BiomeRegistry.Add(EWorld_BiomeType::DenseForest, Forest);

    // ── Open Plains Biome ────────────────────────────────────────
    FWorld_BiomeData Plains;
    Plains.BiomeID      = EWorld_BiomeType::OpenPlains;
    Plains.DisplayName  = FText::FromString(TEXT("Prehistoric Plains"));
    Plains.CenterPoint  = FVector(0.f, 0.f, 0.f);
    Plains.Radius       = 4000.f;
    Plains.BaseTemperature   = 28.f;
    Plains.Humidity          = 0.45f;
    Plains.VegetationDensity = 0.35f;
    Plains.DangerLevel       = 0.4f;
    Plains.AmbientAudioTag   = FName("Audio.Biome.Plains");
    BiomeRegistry.Add(EWorld_BiomeType::OpenPlains, Plains);

    // ── Rocky Highlands Biome ────────────────────────────────────
    FWorld_BiomeData Rocky;
    Rocky.BiomeID      = EWorld_BiomeType::RockyHighlands;
    Rocky.DisplayName  = FText::FromString(TEXT("Rocky Highlands"));
    Rocky.CenterPoint  = FVector(0.f, -3000.f, 200.f);
    Rocky.Radius       = 2500.f;
    Rocky.BaseTemperature   = 15.f;
    Rocky.Humidity          = 0.25f;
    Rocky.VegetationDensity = 0.15f;
    Rocky.DangerLevel       = 0.7f;
    Rocky.AmbientAudioTag   = FName("Audio.Biome.Rocky");
    BiomeRegistry.Add(EWorld_BiomeType::RockyHighlands, Rocky);

    // ── River Corridor Biome ─────────────────────────────────────
    FWorld_BiomeData River;
    River.BiomeID      = EWorld_BiomeType::RiverCorridor;
    River.DisplayName  = FText::FromString(TEXT("River Corridor"));
    River.CenterPoint  = FVector(3000.f, 500.f, -80.f);
    River.Radius       = 1200.f;
    River.BaseTemperature   = 20.f;
    River.Humidity          = 0.95f;
    River.VegetationDensity = 0.7f;
    River.DangerLevel       = 0.5f;
    River.AmbientAudioTag   = FName("Audio.Biome.River");
    BiomeRegistry.Add(EWorld_BiomeType::RiverCorridor, River);
}

EWorld_BiomeType UBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;
    float ClosestDist = MAX_FLT;

    for (const auto& Pair : BiomeRegistry)
    {
        const FWorld_BiomeData& Data = Pair.Value;
        const float Dist = FVector::Dist2D(WorldLocation, Data.CenterPoint);
        if (Dist < Data.Radius && Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Data.BiomeID;
        }
    }
    return ClosestBiome;
}

FWorld_BiomeData UBiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    if (const FWorld_BiomeData* Found = BiomeRegistry.Find(BiomeType))
    {
        return *Found;
    }
    // Return default (plains) if not found
    FWorld_BiomeData Default;
    Default.BiomeID = EWorld_BiomeType::OpenPlains;
    Default.DisplayName = FText::FromString(TEXT("Unknown"));
    return Default;
}

float UBiomeSystem::GetBlendWeightAtLocation(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    const FWorld_BiomeData* Data = BiomeRegistry.Find(BiomeType);
    if (!Data) return 0.f;

    const float Dist = FVector::Dist2D(WorldLocation, Data->CenterPoint);
    if (Dist >= Data->Radius) return 0.f;

    // Smooth blend: 1.0 at center, 0.0 at edge
    const float NormalizedDist = Dist / Data->Radius;
    return FMath::SmoothStep(0.f, 1.f, 1.f - NormalizedDist);
}

TArray<FWorld_BiomeData> UBiomeSystem::GetAllBiomes() const
{
    TArray<FWorld_BiomeData> Result;
    BiomeRegistry.GenerateValueArray(Result);
    return Result;
}

float UBiomeSystem::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    float TotalWeight = 0.f;
    float WeightedTemp = 0.f;

    for (const auto& Pair : BiomeRegistry)
    {
        const float W = GetBlendWeightAtLocation(WorldLocation, Pair.Key);
        WeightedTemp += Pair.Value.BaseTemperature * W;
        TotalWeight  += W;
    }
    return (TotalWeight > KINDA_SMALL_NUMBER) ? (WeightedTemp / TotalWeight) : 20.f;
}

float UBiomeSystem::GetHumidityAtLocation(const FVector& WorldLocation) const
{
    float TotalWeight = 0.f;
    float WeightedHumidity = 0.f;

    for (const auto& Pair : BiomeRegistry)
    {
        const float W = GetBlendWeightAtLocation(WorldLocation, Pair.Key);
        WeightedHumidity += Pair.Value.Humidity * W;
        TotalWeight      += W;
    }
    return (TotalWeight > KINDA_SMALL_NUMBER) ? (WeightedHumidity / TotalWeight) : 0.5f;
}

bool UBiomeSystem::IsInTransitionZone(const FVector& WorldLocation, float TransitionWidth) const
{
    int32 ActiveBiomes = 0;
    for (const auto& Pair : BiomeRegistry)
    {
        const float W = GetBlendWeightAtLocation(WorldLocation, Pair.Key);
        if (W > 0.1f) ActiveBiomes++;
    }
    return ActiveBiomes > 1;
}

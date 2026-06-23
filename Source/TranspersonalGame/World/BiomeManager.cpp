// BiomeManager.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260623_007
// Full implementation of ABiomeManager for the prehistoric survival game.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    // Default biome configuration
    TransitionBlendRadius = 2000.0f;
    BiomeUpdateInterval   = 1.0f;
    bDebugBiomeOverlay    = false;
    CurrentDominantBiome  = EEng_BiomeType::Grassland;
    bBiomesInitialized    = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();

    // Start periodic biome update timer
    GetWorldTimerManager().SetTimer(
        BiomeUpdateTimerHandle,
        this,
        &ABiomeManager::UpdateBiomeStates,
        BiomeUpdateInterval,
        true  // looping
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance active transitions
    for (FEng_BiomeTransition& Transition : ActiveTransitions)
    {
        if (Transition.bIsActive)
        {
            Transition.BlendAlpha = FMath::Clamp(
                Transition.BlendAlpha + DeltaTime / FMath::Max(Transition.TransitionDuration, 0.01f),
                0.0f, 1.0f
            );

            if (Transition.BlendAlpha >= 1.0f)
            {
                Transition.bIsActive = false;
                OnBiomeTransitionComplete.Broadcast(Transition.FromBiome, Transition.ToBiome);
            }
        }
    }

    // Remove completed transitions
    ActiveTransitions.RemoveAll([](const FEng_BiomeTransition& T) { return !T.bIsActive; });

#if WITH_EDITOR
    if (bDebugBiomeOverlay)
    {
        DrawDebugBiomeOverlay();
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// InitializeBiomes — populate default runtime data for all biome types
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::InitializeBiomes()
{
    if (bBiomesInitialized) return;

    // ── Grassland ──────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Grassland;
        Grassland.BiomeType           = EEng_BiomeType::Grassland;
        Grassland.DisplayName         = FText::FromString(TEXT("Grassland Plains"));
        Grassland.TemperatureRange    = FVector2D(18.0f, 32.0f);
        Grassland.HumidityRange       = FVector2D(0.3f, 0.6f);
        Grassland.FoliageDensity      = 0.6f;
        Grassland.PredatorSpawnWeight = 0.4f;
        Grassland.HerbivoreSpawnWeight= 0.8f;
        Grassland.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Grassland, Grassland);
    }

    // ── Forest ────────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Forest;
        Forest.BiomeType           = EEng_BiomeType::Forest;
        Forest.DisplayName         = FText::FromString(TEXT("Dense Forest"));
        Forest.TemperatureRange    = FVector2D(12.0f, 26.0f);
        Forest.HumidityRange       = FVector2D(0.6f, 0.9f);
        Forest.FoliageDensity      = 0.95f;
        Forest.PredatorSpawnWeight = 0.6f;
        Forest.HerbivoreSpawnWeight= 0.5f;
        Forest.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Forest, Forest);
    }

    // ── Desert ────────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Desert;
        Desert.BiomeType           = EEng_BiomeType::Desert;
        Desert.DisplayName         = FText::FromString(TEXT("Arid Desert"));
        Desert.TemperatureRange    = FVector2D(35.0f, 55.0f);
        Desert.HumidityRange       = FVector2D(0.0f, 0.15f);
        Desert.FoliageDensity      = 0.05f;
        Desert.PredatorSpawnWeight = 0.3f;
        Desert.HerbivoreSpawnWeight= 0.2f;
        Desert.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Desert, Desert);
    }

    // ── Swamp ─────────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Swamp;
        Swamp.BiomeType           = EEng_BiomeType::Swamp;
        Swamp.DisplayName         = FText::FromString(TEXT("Primordial Swamp"));
        Swamp.TemperatureRange    = FVector2D(22.0f, 34.0f);
        Swamp.HumidityRange       = FVector2D(0.85f, 1.0f);
        Swamp.FoliageDensity      = 0.8f;
        Swamp.PredatorSpawnWeight = 0.7f;
        Swamp.HerbivoreSpawnWeight= 0.3f;
        Swamp.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Swamp, Swamp);
    }

    // ── Volcanic ──────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Volcanic;
        Volcanic.BiomeType           = EEng_BiomeType::Volcanic;
        Volcanic.DisplayName         = FText::FromString(TEXT("Volcanic Badlands"));
        Volcanic.TemperatureRange    = FVector2D(40.0f, 80.0f);
        Volcanic.HumidityRange       = FVector2D(0.1f, 0.3f);
        Volcanic.FoliageDensity      = 0.02f;
        Volcanic.PredatorSpawnWeight = 0.2f;
        Volcanic.HerbivoreSpawnWeight= 0.05f;
        Volcanic.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Volcanic, Volcanic);
    }

    // ── Tundra ────────────────────────────────────────────────────────────
    {
        FEng_BiomeRuntimeData Tundra;
        Tundra.BiomeType           = EEng_BiomeType::Tundra;
        Tundra.DisplayName         = FText::FromString(TEXT("Frozen Tundra"));
        Tundra.TemperatureRange    = FVector2D(-20.0f, 5.0f);
        Tundra.HumidityRange       = FVector2D(0.2f, 0.5f);
        Tundra.FoliageDensity      = 0.1f;
        Tundra.PredatorSpawnWeight = 0.3f;
        Tundra.HerbivoreSpawnWeight= 0.4f;
        Tundra.bIsActive           = true;
        BiomeRuntimeData.Add(EEng_BiomeType::Tundra, Tundra);
    }

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biomes"), BiomeRuntimeData.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeAtLocation — returns the dominant biome at a world position
// Uses a simple noise-based approach until proper heightmap integration
// ─────────────────────────────────────────────────────────────────────────────

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (!bBiomesInitialized) return EEng_BiomeType::Grassland;

    // Normalize coordinates to [0,1] range over a 100km world
    const float WorldScale = 100000.0f;
    float NX = (WorldLocation.X / WorldScale) + 0.5f;
    float NY = (WorldLocation.Y / WorldScale) + 0.5f;

    // Simple deterministic biome assignment based on position bands
    // In production this will be replaced by PCG/heightmap sampling
    float Elevation = WorldLocation.Z;

    if (Elevation > 5000.0f)  return EEng_BiomeType::Volcanic;
    if (Elevation > 2000.0f)  return EEng_BiomeType::Tundra;

    // Use a pseudo-noise pattern based on position
    float Pattern = FMath::Sin(NX * 3.14159f * 2.0f) * FMath::Cos(NY * 3.14159f * 2.0f);

    if (Pattern > 0.5f)  return EEng_BiomeType::Forest;
    if (Pattern > 0.0f)  return EEng_BiomeType::Grassland;
    if (Pattern > -0.3f) return EEng_BiomeType::Swamp;
    return EEng_BiomeType::Desert;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeBlendWeights — returns blend weights for biomes at a location
// ─────────────────────────────────────────────────────────────────────────────

TMap<EEng_BiomeType, float> ABiomeManager::GetBiomeBlendWeights(FVector WorldLocation) const
{
    TMap<EEng_BiomeType, float> Weights;

    EEng_BiomeType Primary = GetBiomeAtLocation(WorldLocation);
    Weights.Add(Primary, 1.0f);

    // Check neighbouring sample points for blending
    const float SampleOffset = TransitionBlendRadius * 0.5f;
    TArray<FVector> SampleOffsets = {
        FVector(SampleOffset, 0, 0),
        FVector(-SampleOffset, 0, 0),
        FVector(0, SampleOffset, 0),
        FVector(0, -SampleOffset, 0)
    };

    for (const FVector& Offset : SampleOffsets)
    {
        EEng_BiomeType Neighbour = GetBiomeAtLocation(WorldLocation + Offset);
        if (Neighbour != Primary)
        {
            float& Weight = Weights.FindOrAdd(Neighbour);
            Weight += 0.25f;
        }
    }

    // Normalize weights
    float Total = 0.0f;
    for (auto& Pair : Weights) Total += Pair.Value;
    if (Total > 0.0f)
    {
        for (auto& Pair : Weights) Pair.Value /= Total;
    }

    return Weights;
}

// ─────────────────────────────────────────────────────────────────────────────
// TriggerBiomeTransition — initiates a blended transition between two biomes
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::TriggerBiomeTransition(EEng_BiomeType FromBiome, EEng_BiomeType ToBiome, float Duration)
{
    // Cancel any existing transition involving these biomes
    ActiveTransitions.RemoveAll([&](const FEng_BiomeTransition& T) {
        return T.FromBiome == FromBiome || T.ToBiome == ToBiome;
    });

    FEng_BiomeTransition NewTransition;
    NewTransition.FromBiome          = FromBiome;
    NewTransition.ToBiome            = ToBiome;
    NewTransition.TransitionDuration = FMath::Max(Duration, 0.1f);
    NewTransition.BlendAlpha         = 0.0f;
    NewTransition.bIsActive          = true;

    ActiveTransitions.Add(NewTransition);
    CurrentDominantBiome = ToBiome;

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition started %d -> %d (%.1fs)"),
        (int32)FromBiome, (int32)ToBiome, Duration);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeRuntimeData — returns the runtime data for a specific biome
// ─────────────────────────────────────────────────────────────────────────────

FEng_BiomeRuntimeData ABiomeManager::GetBiomeRuntimeData(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeRuntimeData* Data = BiomeRuntimeData.Find(BiomeType);
    if (Data) return *Data;

    // Return default grassland data as fallback
    FEng_BiomeRuntimeData Default;
    Default.BiomeType  = EEng_BiomeType::Grassland;
    Default.DisplayName= FText::FromString(TEXT("Unknown Biome"));
    Default.FoliageDensity = 0.5f;
    Default.bIsActive  = false;
    return Default;
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateBiomeStates — periodic update called by timer
// ─────────────────────────────────────────────────────────────────────────────

void ABiomeManager::UpdateBiomeStates()
{
    if (!bBiomesInitialized) return;

    // Update player-relative biome if we have a player
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        EEng_BiomeType PlayerBiome = GetBiomeAtLocation(PlayerPawn->GetActorLocation());
        if (PlayerBiome != CurrentDominantBiome)
        {
            TriggerBiomeTransition(CurrentDominantBiome, PlayerBiome, 5.0f);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DrawDebugBiomeOverlay — editor-only debug visualisation
// ─────────────────────────────────────────────────────────────────────────────

#if WITH_EDITOR
void ABiomeManager::DrawDebugBiomeOverlay()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const int32 GridSize   = 10;
    const float GridSpacing= 1000.0f;

    static const TMap<EEng_BiomeType, FColor> BiomeColors = {
        { EEng_BiomeType::Grassland, FColor::Green  },
        { EEng_BiomeType::Forest,    FColor(0, 100, 0) },
        { EEng_BiomeType::Desert,    FColor::Yellow  },
        { EEng_BiomeType::Swamp,     FColor(0, 128, 64) },
        { EEng_BiomeType::Volcanic,  FColor::Red     },
        { EEng_BiomeType::Tundra,    FColor::Cyan    }
    };

    for (int32 X = -GridSize; X <= GridSize; ++X)
    {
        for (int32 Y = -GridSize; Y <= GridSize; ++Y)
        {
            FVector Loc(X * GridSpacing, Y * GridSpacing, 0.0f);
            EEng_BiomeType Biome = GetBiomeAtLocation(Loc);
            const FColor* Color = BiomeColors.Find(Biome);
            FColor DrawColor = Color ? *Color : FColor::White;

            DrawDebugPoint(World, Loc, 8.0f, DrawColor, false, BiomeUpdateInterval + 0.1f);
        }
    }
}
#endif

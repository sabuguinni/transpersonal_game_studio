// BiomeManager.cpp
// Engine Architect #02 — Transpersonal Game Studio
// Biome system: classifies world positions into biomes, drives foliage/weather/audio.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update once per second — not every frame

    CurrentBiome      = EEng_BiomeType::Jungle;
    WorldSeed         = 42;
    BiomeBlendRadius  = 500.0f;
    bDebugDrawBiomes  = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomeMap();
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Initialized with seed %d"), WorldSeed);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────
void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update current biome based on player location
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
        EEng_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);
        if (NewBiome != CurrentBiome)
        {
            CurrentBiome = NewBiome;
            OnBiomeChanged(NewBiome);
        }
    }

    if (bDebugDrawBiomes)
    {
        DrawBiomeDebug();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// InitializeBiomeMap
// ─────────────────────────────────────────────────────────────────────────────
void ABiomeManager::InitializeBiomeMap()
{
    BiomeZones.Empty();

    // Define biome zones by world-space centre + radius
    // These match the MinPlayableMap layout (origin = PlayerStart)
    struct FBiomeZoneDef
    {
        FVector    Centre;
        float      Radius;
        EEng_BiomeType Type;
    };

    TArray<FBiomeZoneDef> Defs = {
        { FVector(   0,    0,  0),  800.f, EEng_BiomeType::Jungle      },
        { FVector( 900,  600,  0),  600.f, EEng_BiomeType::Savanna     },
        { FVector(-900,  700,  0),  550.f, EEng_BiomeType::Swamp       },
        { FVector( 600, -800,  0),  500.f, EEng_BiomeType::Volcanic    },
        { FVector(-700, -600,  0),  500.f, EEng_BiomeType::Riverbank   },
        { FVector(1200,  300,  0),  400.f, EEng_BiomeType::OpenPlains  },
        { FVector(-500, 1100,  0),  400.f, EEng_BiomeType::Forest      },
    };

    for (const FBiomeZoneDef& D : Defs)
    {
        FEng_BiomeZone Zone;
        Zone.Centre     = D.Centre;
        Zone.Radius     = D.Radius;
        Zone.BiomeType  = D.Type;
        Zone.Temperature = GetDefaultTemperature(D.Type);
        Zone.Humidity    = GetDefaultHumidity(D.Type);
        BiomeZones.Add(Zone);
    }

    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] %d biome zones registered"), BiomeZones.Num());
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBiomeAtLocation — returns the dominant biome at a world position
// ─────────────────────────────────────────────────────────────────────────────
EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float    BestDist = TNumericLimits<float>::Max();
    EEng_BiomeType Best = EEng_BiomeType::Jungle;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.Centre);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Best     = Zone.BiomeType;
        }
    }
    return Best;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetBlendedBiomeData — returns temperature/humidity blended across nearby zones
// ─────────────────────────────────────────────────────────────────────────────
FEng_BiomeBlendData ABiomeManager::GetBlendedBiomeData(const FVector& WorldLocation) const
{
    FEng_BiomeBlendData Result;
    Result.DominantBiome = GetBiomeAtLocation(WorldLocation);

    float TotalWeight = 0.f;
    float BlendedTemp = 0.f;
    float BlendedHum  = 0.f;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        float Dist   = FVector::Dist2D(WorldLocation, Zone.Centre);
        float Weight = FMath::Max(0.f, 1.f - (Dist / (Zone.Radius + BiomeBlendRadius)));
        Weight = Weight * Weight; // Quadratic falloff

        BlendedTemp  += Zone.Temperature * Weight;
        BlendedHum   += Zone.Humidity    * Weight;
        TotalWeight  += Weight;

        if (Weight > 0.1f)
        {
            Result.NearbyBiomes.AddUnique(Zone.BiomeType);
        }
    }

    if (TotalWeight > 0.f)
    {
        Result.BlendedTemperature = BlendedTemp / TotalWeight;
        Result.BlendedHumidity    = BlendedHum  / TotalWeight;
    }

    return Result;
}

// ─────────────────────────────────────────────────────────────────────────────
// OnBiomeChanged — called when player crosses a biome boundary
// ─────────────────────────────────────────────────────────────────────────────
void ABiomeManager::OnBiomeChanged(EEng_BiomeType NewBiome)
{
    UE_LOG(LogTemp, Log, TEXT("[BiomeManager] Player entered biome: %d"), (int32)NewBiome);
    // Blueprint event — notify audio/VFX agents via delegate
    OnBiomeChangedDelegate.Broadcast(NewBiome);
}

// ─────────────────────────────────────────────────────────────────────────────
// DrawBiomeDebug — debug visualisation (editor only)
// ─────────────────────────────────────────────────────────────────────────────
void ABiomeManager::DrawBiomeDebug()
{
#if WITH_EDITOR
    UWorld* W = GetWorld();
    if (!W) return;

    for (const FEng_BiomeZone& Zone : BiomeZones)
    {
        FColor DebugColor = FColor::Green;
        switch (Zone.BiomeType)
        {
            case EEng_BiomeType::Jungle:     DebugColor = FColor(0,   180,  0);   break;
            case EEng_BiomeType::Savanna:    DebugColor = FColor(200, 180,  0);   break;
            case EEng_BiomeType::Swamp:      DebugColor = FColor(0,   100, 50);   break;
            case EEng_BiomeType::Volcanic:   DebugColor = FColor(220,  50,  0);   break;
            case EEng_BiomeType::Riverbank:  DebugColor = FColor(0,   100, 200);  break;
            case EEng_BiomeType::OpenPlains: DebugColor = FColor(180, 220, 100);  break;
            case EEng_BiomeType::Forest:     DebugColor = FColor(0,   120,  60);  break;
            default: break;
        }
        DrawDebugCircle(W, Zone.Centre, Zone.Radius, 32, DebugColor, false, 1.1f, 0, 5.f);
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
float ABiomeManager::GetDefaultTemperature(EEng_BiomeType Biome) const
{
    switch (Biome)
    {
        case EEng_BiomeType::Jungle:     return 32.f;
        case EEng_BiomeType::Savanna:    return 38.f;
        case EEng_BiomeType::Swamp:      return 28.f;
        case EEng_BiomeType::Volcanic:   return 55.f;
        case EEng_BiomeType::Riverbank:  return 25.f;
        case EEng_BiomeType::OpenPlains: return 35.f;
        case EEng_BiomeType::Forest:     return 22.f;
        default:                         return 30.f;
    }
}

float ABiomeManager::GetDefaultHumidity(EEng_BiomeType Biome) const
{
    switch (Biome)
    {
        case EEng_BiomeType::Jungle:     return 0.90f;
        case EEng_BiomeType::Savanna:    return 0.30f;
        case EEng_BiomeType::Swamp:      return 0.95f;
        case EEng_BiomeType::Volcanic:   return 0.15f;
        case EEng_BiomeType::Riverbank:  return 0.80f;
        case EEng_BiomeType::OpenPlains: return 0.40f;
        case EEng_BiomeType::Forest:     return 0.70f;
        default:                         return 0.50f;
    }
}

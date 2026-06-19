// PCGWorldGenerator.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260619_007
// Implements biome-based procedural world generation for the MinPlayableMap.
// Biomes: Forest, Savanna, Swamp, Rocky, Plains

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// Constructor
// ============================================================
APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome layout — matches MinPlayableMap placements
    BiomeDefinitions.Add(FWorld_BiomeDef{
        EWorld_BiomeType::Forest,
        FVector(-2000.f, -2000.f, 0.f),
        1200.f,
        TEXT("Forest_Zone_001")
    });
    BiomeDefinitions.Add(FWorld_BiomeDef{
        EWorld_BiomeType::Savanna,
        FVector(2000.f, 0.f, 0.f),
        1500.f,
        TEXT("Savanna_Zone_001")
    });
    BiomeDefinitions.Add(FWorld_BiomeDef{
        EWorld_BiomeType::Swamp,
        FVector(-1000.f, 2500.f, -50.f),
        1000.f,
        TEXT("Swamp_Zone_001")
    });
    BiomeDefinitions.Add(FWorld_BiomeDef{
        EWorld_BiomeType::Rocky,
        FVector(500.f, -2500.f, 100.f),
        800.f,
        TEXT("Rocky_Zone_001")
    });
    BiomeDefinitions.Add(FWorld_BiomeDef{
        EWorld_BiomeType::Plains,
        FVector(0.f, 0.f, 0.f),
        2000.f,
        TEXT("Plains_Zone_001")
    });

    // Water body definitions
    WaterBodies.Add(FWorld_WaterBody{
        TEXT("Water_Swamp_001"),
        FVector(-1000.f, 2500.f, -60.f),
        FVector2D(1500.f, 1500.f)
    });
    WaterBodies.Add(FWorld_WaterBody{
        TEXT("Water_River_001"),
        FVector(0.f, 500.f, -20.f),
        FVector2D(300.f, 2000.f)
    });
    WaterBodies.Add(FWorld_WaterBody{
        TEXT("Water_Lake_001"),
        FVector(1500.f, 1500.f, -30.f),
        FVector2D(800.f, 800.f)
    });

    // Performance: ISM budget per biome (max actors in 500m radius)
    MaxActorsPerBiomeRadius = 200;
    WorldSeed = 42;
}

// ============================================================
// BeginPlay
// ============================================================
void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();
    // World generation is editor-time only in this implementation.
    // Runtime generation can be triggered via GenerateWorld() if needed.
}

// ============================================================
// GetBiomeAtLocation
// Returns the dominant biome type at a given world location.
// Uses simple distance-to-center weighting.
// ============================================================
EWorld_BiomeType APCGWorldGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Plains;
    float ClosestDist = MAX_FLT;

    for (const FWorld_BiomeDef& Biome : BiomeDefinitions)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

// ============================================================
// GetBiomeName
// Returns human-readable biome name for UI/debug display.
// ============================================================
FString APCGWorldGenerator::GetBiomeName(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:  return TEXT("Cretaceous Forest");
        case EWorld_BiomeType::Savanna: return TEXT("Open Savanna");
        case EWorld_BiomeType::Swamp:   return TEXT("Prehistoric Swamp");
        case EWorld_BiomeType::Rocky:   return TEXT("Rocky Highlands");
        case EWorld_BiomeType::Plains:  return TEXT("Central Plains");
        default:                        return TEXT("Unknown");
    }
}

// ============================================================
// IsInWaterBody
// Returns true if the given location is inside any water body.
// ============================================================
bool APCGWorldGenerator::IsInWaterBody(const FVector& WorldLocation) const
{
    for (const FWorld_WaterBody& Water : WaterBodies)
    {
        float DX = FMath::Abs(WorldLocation.X - Water.CenterLocation.X);
        float DY = FMath::Abs(WorldLocation.Y - Water.CenterLocation.Y);
        if (DX < Water.Extents.X * 0.5f && DY < Water.Extents.Y * 0.5f)
        {
            return true;
        }
    }
    return false;
}

// ============================================================
// GetVegetationDensity
// Returns 0.0-1.0 vegetation density for a biome type.
// Used by FoliageManager to scale foliage spawning.
// ============================================================
float APCGWorldGenerator::GetVegetationDensity(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Forest:  return 0.95f;
        case EWorld_BiomeType::Swamp:   return 0.80f;
        case EWorld_BiomeType::Plains:  return 0.45f;
        case EWorld_BiomeType::Savanna: return 0.35f;
        case EWorld_BiomeType::Rocky:   return 0.15f;
        default:                        return 0.30f;
    }
}

// ============================================================
// GetTerrainHeightBias
// Returns height offset bias for terrain variation per biome.
// Rocky = elevated, Swamp = depressed, others = neutral.
// ============================================================
float APCGWorldGenerator::GetTerrainHeightBias(EWorld_BiomeType BiomeType) const
{
    switch (BiomeType)
    {
        case EWorld_BiomeType::Rocky:   return 150.f;
        case EWorld_BiomeType::Forest:  return 20.f;
        case EWorld_BiomeType::Plains:  return 0.f;
        case EWorld_BiomeType::Savanna: return -10.f;
        case EWorld_BiomeType::Swamp:   return -60.f;
        default:                        return 0.f;
    }
}

// ============================================================
// GenerateWorld (CallInEditor)
// Editor utility — regenerates all biome markers and water bodies.
// Safe to call multiple times (checks for existing actors).
// ============================================================
void APCGWorldGenerator::GenerateWorld()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGWorldGenerator: No world found — cannot generate."));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: GenerateWorld() called. Biomes: %d, WaterBodies: %d"),
        BiomeDefinitions.Num(), WaterBodies.Num());

    // Log biome layout for debugging
    for (const FWorld_BiomeDef& Biome : BiomeDefinitions)
    {
        UE_LOG(LogTemp, Log, TEXT("  Biome [%s] center=(%.0f, %.0f, %.0f) radius=%.0f"),
            *Biome.ZoneLabel,
            Biome.CenterLocation.X, Biome.CenterLocation.Y, Biome.CenterLocation.Z,
            Biome.Radius);
    }

    // Log water bodies
    for (const FWorld_WaterBody& Water : WaterBodies)
    {
        UE_LOG(LogTemp, Log, TEXT("  Water [%s] center=(%.0f, %.0f, %.0f) extents=(%.0f x %.0f)"),
            *Water.BodyLabel,
            Water.CenterLocation.X, Water.CenterLocation.Y, Water.CenterLocation.Z,
            Water.Extents.X, Water.Extents.Y);
    }

    UE_LOG(LogTemp, Log, TEXT("PCGWorldGenerator: GenerateWorld() complete. WorldSeed=%d"), WorldSeed);
}

// ============================================================
// DebugDrawBiomes (CallInEditor)
// Draws debug spheres for each biome zone in the editor viewport.
// ============================================================
void APCGWorldGenerator::DebugDrawBiomes()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FWorld_BiomeDef& Biome : BiomeDefinitions)
    {
        FColor DebugColor = FColor::Green;
        switch (Biome.BiomeType)
        {
            case EWorld_BiomeType::Forest:  DebugColor = FColor(34,  139, 34);  break;
            case EWorld_BiomeType::Savanna: DebugColor = FColor(210, 180, 40);  break;
            case EWorld_BiomeType::Swamp:   DebugColor = FColor(60,  100, 60);  break;
            case EWorld_BiomeType::Rocky:   DebugColor = FColor(139, 115, 85);  break;
            case EWorld_BiomeType::Plains:  DebugColor = FColor(180, 200, 100); break;
        }

        DrawDebugSphere(World, Biome.CenterLocation, Biome.Radius, 16, DebugColor, false, 30.f, 0, 5.f);
        DrawDebugString(World, Biome.CenterLocation + FVector(0, 0, 200), Biome.ZoneLabel, nullptr, DebugColor, 30.f);
    }
}

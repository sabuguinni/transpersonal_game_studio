// Copyright Transpersonal Game Studio. All Rights Reserved.
// PCGWorldGenerator.cpp — Agent #5 Procedural World Generator
// Implements biome layout, foliage spawning, water body placement,
// NavMesh configuration, and terrain height variation logic.

#include "PCGWorldGenerator.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================
APCGWorldGenerator::APCGWorldGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome configuration
    WorldSeedValue = 42;
    bGenerateOnBeginPlay = true;
    WorldScale = 1.0f;
    BiomeBlendRadius = 2000.0f;
    WaterLevel = -150.0f;

    // Default biome weights
    BiomeWeights.Add(EWorld_BiomeType::ForestValley, 0.30f);
    BiomeWeights.Add(EWorld_BiomeType::OpenPlains, 0.35f);
    BiomeWeights.Add(EWorld_BiomeType::RockyHighlands, 0.20f);
    BiomeWeights.Add(EWorld_BiomeType::RiverValley, 0.15f);
}

// ============================================================
// BeginPlay
// ============================================================
void APCGWorldGenerator::BeginPlay()
{
    Super::BeginPlay();

    if (bGenerateOnBeginPlay)
    {
        GenerateWorld();
    }
}

// ============================================================
// GenerateWorld — master entry point
// ============================================================
void APCGWorldGenerator::GenerateWorld()
{
    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] GenerateWorld() START — Seed: %d"), WorldSeedValue);

    FMath::RandInit(WorldSeedValue);

    GenerateBiomeLayout();
    PlaceWaterBodies();
    SpawnFoliageForAllBiomes();
    ConfigureNavMesh();
    ApplyHeightVariation();

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] GenerateWorld() COMPLETE — %d biome zones registered"), BiomeZones.Num());
}

// ============================================================
// GenerateBiomeLayout
// ============================================================
void APCGWorldGenerator::GenerateBiomeLayout()
{
    BiomeZones.Empty();

    // Forest Valley — north quadrant
    FWorld_BiomeZone ForestZone;
    ForestZone.BiomeType = EWorld_BiomeType::ForestValley;
    ForestZone.CenterLocation = FVector(0.0f, 6000.0f, 0.0f);
    ForestZone.Radius = 4000.0f;
    ForestZone.FoliageDensity = 0.85f;
    ForestZone.bHasWater = false;
    ForestZone.BiomeName = TEXT("Northern Forest Valley");
    BiomeZones.Add(ForestZone);

    // Open Plains — east quadrant
    FWorld_BiomeZone PlainsZone;
    PlainsZone.BiomeType = EWorld_BiomeType::OpenPlains;
    PlainsZone.CenterLocation = FVector(11000.0f, 0.0f, 0.0f);
    PlainsZone.Radius = 6000.0f;
    PlainsZone.FoliageDensity = 0.25f;
    PlainsZone.bHasWater = false;
    PlainsZone.BiomeName = TEXT("Eastern Grazing Plains");
    BiomeZones.Add(PlainsZone);

    // Rocky Highlands — south-west quadrant
    FWorld_BiomeZone RockyZone;
    RockyZone.BiomeType = EWorld_BiomeType::RockyHighlands;
    RockyZone.CenterLocation = FVector(-5000.0f, -4500.0f, 200.0f);
    RockyZone.Radius = 3500.0f;
    RockyZone.FoliageDensity = 0.10f;
    RockyZone.bHasWater = false;
    RockyZone.BiomeName = TEXT("Volcanic Rocky Highlands");
    BiomeZones.Add(RockyZone);

    // River Valley — center-east
    FWorld_BiomeZone RiverZone;
    RiverZone.BiomeType = EWorld_BiomeType::RiverValley;
    RiverZone.CenterLocation = FVector(8000.0f, 0.0f, -150.0f);
    RiverZone.Radius = 2500.0f;
    RiverZone.FoliageDensity = 0.60f;
    RiverZone.bHasWater = true;
    RiverZone.BiomeName = TEXT("Central River Valley");
    BiomeZones.Add(RiverZone);

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] GenerateBiomeLayout() — %d biomes defined"), BiomeZones.Num());
}

// ============================================================
// PlaceWaterBodies
// ============================================================
void APCGWorldGenerator::PlaceWaterBodies()
{
    if (!GetWorld()) return;

    // Water body locations: river (elongated) + lake (circular)
    TArray<FVector> WaterLocations = {
        FVector(8000.0f, 0.0f, WaterLevel),
        FVector(5000.0f, -2000.0f, WaterLevel - 50.0f)
    };

    TArray<FVector> WaterScales = {
        FVector(40.0f, 8.0f, 0.05f),   // River — elongated
        FVector(15.0f, 15.0f, 0.05f)   // Lake — circular
    };

    TArray<FString> WaterLabels = {
        TEXT("WaterBody_MainRiver"),
        TEXT("WaterBody_ForestLake")
    };

    for (int32 i = 0; i < WaterLocations.Num(); i++)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AStaticMeshActor* WaterActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            WaterLocations[i],
            FRotator::ZeroRotator,
            SpawnParams
        );

        if (WaterActor)
        {
            WaterActor->SetActorLabel(WaterLabels[i]);
            WaterActor->SetActorScale3D(WaterScales[i]);

            UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] Water body placed: %s at %s"),
                *WaterLabels[i], *WaterLocations[i].ToString());
        }
    }
}

// ============================================================
// SpawnFoliageForBiome
// ============================================================
void APCGWorldGenerator::SpawnFoliageForBiome(const FWorld_BiomeZone& BiomeZone)
{
    if (!GetWorld()) return;

    int32 FoliageCount = FMath::RoundToInt(BiomeZone.FoliageDensity * 50.0f);

    for (int32 i = 0; i < FoliageCount; i++)
    {
        // Random position within biome radius
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Distance = FMath::RandRange(0.0f, BiomeZone.Radius);
        FVector SpawnLoc = BiomeZone.CenterLocation + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );

        // Line trace to find ground
        FHitResult HitResult;
        FVector TraceStart = SpawnLoc + FVector(0.0f, 0.0f, 2000.0f);
        FVector TraceEnd = SpawnLoc - FVector(0.0f, 0.0f, 2000.0f);

        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLoc.Z = HitResult.Location.Z;
        }

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

        AStaticMeshActor* FoliageActor = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLoc,
            FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f),
            SpawnParams
        );

        if (FoliageActor)
        {
            float Scale = FMath::RandRange(0.8f, 2.5f);
            FoliageActor->SetActorScale3D(FVector(Scale, Scale, Scale * 1.5f));
            FoliageActor->SetActorLabel(FString::Printf(TEXT("%s_Foliage_%d"), *BiomeZone.BiomeName, i));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] SpawnFoliageForBiome: %d actors in %s"),
        FoliageCount, *BiomeZone.BiomeName);
}

// ============================================================
// SpawnFoliageForAllBiomes
// ============================================================
void APCGWorldGenerator::SpawnFoliageForAllBiomes()
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        SpawnFoliageForBiome(Zone);
    }
}

// ============================================================
// ConfigureNavMesh
// ============================================================
void APCGWorldGenerator::ConfigureNavMesh()
{
    if (!GetWorld()) return;

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSys)
    {
        NavSys->Build();
        UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] NavMesh rebuild triggered"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PCGWorldGenerator] NavSystem not found — NavMesh rebuild skipped"));
    }
}

// ============================================================
// ApplyHeightVariation
// ============================================================
void APCGWorldGenerator::ApplyHeightVariation()
{
    // Height variation is applied via Perlin noise at runtime
    // Actual landscape deformation requires Landscape actor — 
    // this method logs the intended height profile for each biome
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float HeightOffset = 0.0f;
        switch (Zone.BiomeType)
        {
            case EWorld_BiomeType::RockyHighlands:  HeightOffset = 800.0f;  break;
            case EWorld_BiomeType::ForestValley:    HeightOffset = 100.0f;  break;
            case EWorld_BiomeType::OpenPlains:      HeightOffset = 0.0f;    break;
            case EWorld_BiomeType::RiverValley:     HeightOffset = -150.0f; break;
            default:                                HeightOffset = 0.0f;    break;
        }
        UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] Biome '%s' target height offset: %.0f cm"),
            *Zone.BiomeName, HeightOffset);
    }
}

// ============================================================
// GetBiomeAtLocation
// ============================================================
EWorld_BiomeType APCGWorldGenerator::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

// ============================================================
// GetFoliageDensityAtLocation
// ============================================================
float APCGWorldGenerator::GetFoliageDensityAtLocation(const FVector& WorldLocation) const
{
    float TotalWeight = 0.0f;
    float WeightedDensity = 0.0f;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Dist = FVector::Dist2D(WorldLocation, Zone.CenterLocation);
        if (Dist < Zone.Radius + BiomeBlendRadius)
        {
            float Weight = FMath::Max(0.0f, 1.0f - (Dist / (Zone.Radius + BiomeBlendRadius)));
            WeightedDensity += Zone.FoliageDensity * Weight;
            TotalWeight += Weight;
        }
    }

    return TotalWeight > 0.0f ? WeightedDensity / TotalWeight : 0.0f;
}

// ============================================================
// SetWorldSeed
// ============================================================
void APCGWorldGenerator::SetWorldSeed(int32 NewSeed)
{
    WorldSeedValue = NewSeed;
    FMath::RandInit(WorldSeedValue);
    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] World seed set to: %d"), WorldSeedValue);
}

// ============================================================
// RegenerateWorld (CallInEditor)
// ============================================================
void APCGWorldGenerator::RegenerateWorld()
{
    // Clear existing generated actors
    if (GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetActorLabel().Contains(TEXT("_Foliage_")))
            {
                Actor->Destroy();
            }
        }
    }

    GenerateWorld();
    UE_LOG(LogTemp, Log, TEXT("[PCGWorldGenerator] World regenerated with seed: %d"), WorldSeedValue);
}

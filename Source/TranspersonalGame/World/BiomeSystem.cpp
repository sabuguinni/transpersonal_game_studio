// BiomeSystem.cpp — Agent #5 Procedural World Generator
// Implements 6 prehistoric biome zones with PCG-driven terrain variation

#include "BiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ABiomeSystem::ABiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Update every 0.5s for performance

    BiomeBlendRadius = 500.0f;
    bDebugDrawBiomes = false;
    CurrentPlayerBiome = EWorld_BiomeType::OpenPlains;
}

void ABiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeBiomes();
    UE_LOG(LogTemp, Log, TEXT("ABiomeSystem: Initialized %d biomes"), Biomes.Num());
}

void ABiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player biome every tick interval
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        FVector PlayerLoc = PlayerPawn->GetActorLocation();
        EWorld_BiomeType NewBiome = GetBiomeAtLocation(PlayerLoc);
        if (NewBiome != CurrentPlayerBiome)
        {
            CurrentPlayerBiome = NewBiome;
            OnBiomeChanged(NewBiome);
        }
    }

    if (bDebugDrawBiomes)
    {
        DebugDrawBiomeBoundaries();
    }
}

void ABiomeSystem::InitializeBiomes()
{
    Biomes.Empty();

    // --- FOREST BIOME (SW quadrant) ---
    FWorld_BiomeData Forest;
    Forest.BiomeType = EWorld_BiomeType::DenseForest;
    Forest.Center = FVector(-1800.0f, -1800.0f, 0.0f);
    Forest.Radius = 2000.0f;
    Forest.FoliageDensity = 0.85f;
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.80f;
    Forest.FogColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
    Forest.FogDensity = 0.04f;
    Forest.BiomeName = TEXT("Cretaceous Jungle");
    Biomes.Add(Forest);

    // --- OPEN PLAINS BIOME (NW quadrant) ---
    FWorld_BiomeData Plains;
    Plains.BiomeType = EWorld_BiomeType::OpenPlains;
    Plains.Center = FVector(-1200.0f, 1600.0f, 0.0f);
    Plains.Radius = 2200.0f;
    Plains.FoliageDensity = 0.25f;
    Plains.Temperature = 28.0f;
    Plains.Humidity = 0.35f;
    Plains.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    Plains.FogDensity = 0.01f;
    Plains.BiomeName = TEXT("Prehistoric Savanna");
    Biomes.Add(Plains);

    // --- ROCKY HIGHLANDS BIOME (NE quadrant) ---
    FWorld_BiomeData Highlands;
    Highlands.BiomeType = EWorld_BiomeType::RockyHighlands;
    Highlands.Center = FVector(2000.0f, -1200.0f, 200.0f);
    Highlands.Radius = 1800.0f;
    Highlands.FoliageDensity = 0.15f;
    Highlands.Temperature = 15.0f;
    Highlands.Humidity = 0.20f;
    Highlands.FogColor = FLinearColor(0.6f, 0.6f, 0.65f, 1.0f);
    Highlands.FogDensity = 0.02f;
    Highlands.BiomeName = TEXT("Rocky Highlands");
    Biomes.Add(Highlands);

    // --- RIVER DELTA BIOME (center) ---
    FWorld_BiomeData River;
    River.BiomeType = EWorld_BiomeType::RiverDelta;
    River.Center = FVector(0.0f, 800.0f, -150.0f);
    River.Radius = 1200.0f;
    River.FoliageDensity = 0.55f;
    River.Temperature = 24.0f;
    River.Humidity = 0.90f;
    River.FogColor = FLinearColor(0.5f, 0.7f, 0.8f, 1.0f);
    River.FogDensity = 0.06f;
    River.BiomeName = TEXT("River Delta");
    Biomes.Add(River);

    // --- VOLCANIC ZONE BIOME (SE quadrant) ---
    FWorld_BiomeData Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::VolcanicZone;
    Volcanic.Center = FVector(2100.0f, 1600.0f, 0.0f);
    Volcanic.Radius = 1600.0f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.Temperature = 45.0f;
    Volcanic.Humidity = 0.15f;
    Volcanic.FogColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    Volcanic.FogDensity = 0.08f;
    Volcanic.BiomeName = TEXT("Volcanic Badlands");
    Biomes.Add(Volcanic);

    // --- SWAMPLAND BIOME (south) ---
    FWorld_BiomeData Swamp;
    Swamp.BiomeType = EWorld_BiomeType::Swampland;
    Swamp.Center = FVector(0.0f, 2300.0f, -150.0f);
    Swamp.Radius = 1500.0f;
    Swamp.FoliageDensity = 0.70f;
    Swamp.Temperature = 30.0f;
    Swamp.Humidity = 0.95f;
    Swamp.FogColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
    Swamp.FogDensity = 0.10f;
    Swamp.BiomeName = TEXT("Primordial Swamp");
    Biomes.Add(Swamp);
}

EWorld_BiomeType ABiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float ClosestDist = MAX_FLT;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;

    for (const FWorld_BiomeData& Biome : Biomes)
    {
        float Dist = FVector::Dist2D(WorldLocation, Biome.Center);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Biome.BiomeType;
        }
    }

    return ClosestBiome;
}

FWorld_BiomeData ABiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : Biomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }

    // Return default if not found
    FWorld_BiomeData Default;
    Default.BiomeType = EWorld_BiomeType::OpenPlains;
    Default.BiomeName = TEXT("Unknown");
    return Default;
}

float ABiomeSystem::GetBlendWeight(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    const FWorld_BiomeData* TargetBiome = nullptr;
    for (const FWorld_BiomeData& Biome : Biomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            TargetBiome = &Biome;
            break;
        }
    }

    if (!TargetBiome)
    {
        return 0.0f;
    }

    float Dist = FVector::Dist2D(WorldLocation, TargetBiome->Center);
    float BlendStart = TargetBiome->Radius - BiomeBlendRadius;
    float BlendEnd = TargetBiome->Radius;

    if (Dist <= BlendStart)
    {
        return 1.0f;
    }
    else if (Dist >= BlendEnd)
    {
        return 0.0f;
    }
    else
    {
        // Smooth blend in transition zone
        float Alpha = (Dist - BlendStart) / BiomeBlendRadius;
        return FMath::SmoothStep(1.0f, 0.0f, Alpha);
    }
}

void ABiomeSystem::OnBiomeChanged(EWorld_BiomeType NewBiome)
{
    FWorld_BiomeData BiomeData = GetBiomeData(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("ABiomeSystem: Player entered biome '%s' (Temp: %.1f°C, Humidity: %.0f%%)"),
        *BiomeData.BiomeName, BiomeData.Temperature, BiomeData.Humidity * 100.0f);

    // Broadcast biome change event for other systems (audio, weather, etc.)
    OnBiomeChangedDelegate.Broadcast(NewBiome, BiomeData);
}

void ABiomeSystem::DebugDrawBiomeBoundaries()
{
    UWorld* World = GetWorld();
    if (!World) return;

    static const TMap<EWorld_BiomeType, FColor> BiomeColors = {
        { EWorld_BiomeType::DenseForest,    FColor::Green },
        { EWorld_BiomeType::OpenPlains,     FColor::Yellow },
        { EWorld_BiomeType::RockyHighlands, FColor::Silver },
        { EWorld_BiomeType::RiverDelta,     FColor::Blue },
        { EWorld_BiomeType::VolcanicZone,   FColor::Red },
        { EWorld_BiomeType::Swampland,      FColor(0, 100, 0) },
    };

    for (const FWorld_BiomeData& Biome : Biomes)
    {
        const FColor* Color = BiomeColors.Find(Biome.BiomeType);
        FColor DrawColor = Color ? *Color : FColor::White;
        DrawDebugCircle(World, Biome.Center, Biome.Radius, 64, DrawColor, false, 0.6f, 0, 20.0f, FVector(1, 0, 0), FVector(0, 1, 0));
    }
}

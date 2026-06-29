#include "BiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// ABiomeManager Implementation
// Agent #05 — Procedural World Generator
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize with 3 default prehistoric biomes
    // These are set up with sensible defaults — call SetupDefaultPrehistoricBiomes()
    // in editor to populate with full prehistoric configs.
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    if (BiomeZones.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("ABiomeManager: No biome zones configured. Call SetupDefaultPrehistoricBiomes() in editor."));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("ABiomeManager: %d biome zones active."), BiomeZones.Num());
    }
}

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    int32 Index = FindBiomeIndexAtLocation(WorldLocation);
    if (Index >= 0 && Index < BiomeZones.Num())
    {
        return BiomeZones[Index].BiomeType;
    }
    return EWorld_BiomeType::None;
}

bool ABiomeManager::GetBiomeConfigAtLocation(FVector WorldLocation, FWorld_BiomeConfig& OutConfig) const
{
    int32 Index = FindBiomeIndexAtLocation(WorldLocation);
    if (Index >= 0 && Index < BiomeZones.Num())
    {
        OutConfig = BiomeZones[Index];
        return true;
    }
    return false;
}

TArray<FWorld_PCGSpawnRule> ABiomeManager::GetSpawnRulesForBiome(EWorld_BiomeType BiomeType) const
{
    TArray<FWorld_PCGSpawnRule> Result;
    for (const FWorld_PCGSpawnRule& Rule : SpawnRules)
    {
        if (Rule.TargetBiome == BiomeType)
        {
            Result.Add(Rule);
        }
    }
    return Result;
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    // Simple nearest-biome temperature — could be interpolated at biome edges
    int32 Index = FindBiomeIndexAtLocation(WorldLocation);
    if (Index >= 0 && Index < BiomeZones.Num())
    {
        return BiomeZones[Index].AmbientTemperature;
    }
    // Default prehistoric ambient temperature
    return 28.0f;
}

float ABiomeManager::GetDangerLevelAtLocation(FVector WorldLocation) const
{
    int32 Index = FindBiomeIndexAtLocation(WorldLocation);
    if (Index >= 0 && Index < BiomeZones.Num())
    {
        return BiomeZones[Index].DangerLevel;
    }
    return 0.1f;
}

void ABiomeManager::SetupDefaultPrehistoricBiomes()
{
    BiomeZones.Empty();
    SpawnRules.Empty();

    // --- JUNGLE BIOME ---
    FWorld_BiomeConfig Jungle;
    Jungle.BiomeType = EWorld_BiomeType::Jungle;
    Jungle.BiomeName = TEXT("Prehistoric Jungle");
    Jungle.Center = FVector(-3000.0f, 0.0f, 0.0f);
    Jungle.Radius = 350000.0f;       // 3.5km radius
    Jungle.VegetationDensity = 0.9f;
    Jungle.AmbientTemperature = 32.0f;
    Jungle.DangerLevel = 0.6f;       // Raptors hunt here
    Jungle.FogDensityMultiplier = 1.8f;
    Jungle.AmbientSoundTag = FName("Biome.Jungle");
    Jungle.bHasWater = true;
    BiomeZones.Add(Jungle);

    // --- PLAINS BIOME ---
    FWorld_BiomeConfig Plains;
    Plains.BiomeType = EWorld_BiomeType::Plains;
    Plains.BiomeName = TEXT("Open Plains");
    Plains.Center = FVector(0.0f, 3000.0f, 0.0f);
    Plains.Radius = 400000.0f;       // 4km radius
    Plains.VegetationDensity = 0.3f;
    Plains.AmbientTemperature = 26.0f;
    Plains.DangerLevel = 0.4f;       // Herbivore herds, occasional predator
    Plains.FogDensityMultiplier = 0.6f;
    Plains.AmbientSoundTag = FName("Biome.Plains");
    Plains.bHasWater = false;
    BiomeZones.Add(Plains);

    // --- VOLCANIC BIOME ---
    FWorld_BiomeConfig Volcanic;
    Volcanic.BiomeType = EWorld_BiomeType::Volcanic;
    Volcanic.BiomeName = TEXT("Volcanic Badlands");
    Volcanic.Center = FVector(3000.0f, -2000.0f, 0.0f);
    Volcanic.Radius = 300000.0f;     // 3km radius
    Volcanic.VegetationDensity = 0.1f;
    Volcanic.AmbientTemperature = 45.0f;
    Volcanic.DangerLevel = 0.85f;    // Apex predators, environmental hazards
    Volcanic.FogDensityMultiplier = 2.2f;
    Volcanic.AmbientSoundTag = FName("Biome.Volcanic");
    Volcanic.bHasWater = false;
    BiomeZones.Add(Volcanic);

    // --- RIVER BIOME ---
    FWorld_BiomeConfig River;
    River.BiomeType = EWorld_BiomeType::River;
    River.BiomeName = TEXT("River Valley");
    River.Center = FVector(-1000.0f, 1500.0f, 0.0f);
    River.Radius = 150000.0f;        // 1.5km radius
    River.VegetationDensity = 0.7f;
    River.AmbientTemperature = 24.0f;
    River.DangerLevel = 0.5f;        // Aquatic predators
    River.FogDensityMultiplier = 1.4f;
    River.AmbientSoundTag = FName("Biome.River");
    River.bHasWater = true;
    BiomeZones.Add(River);

    // --- PCG SPAWN RULES ---

    // Jungle vegetation
    FWorld_PCGSpawnRule JungleTree;
    JungleTree.TargetBiome = EWorld_BiomeType::Jungle;
    JungleTree.MeshPath = FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    JungleTree.SpawnProbability = 0.25f;
    JungleTree.ScaleRange = FVector2D(1.0f, 3.0f);
    JungleTree.CullDistance = 8000.0f;
    JungleTree.bCastShadows = false;
    SpawnRules.Add(JungleTree);

    FWorld_PCGSpawnRule JungleFern;
    JungleFern.TargetBiome = EWorld_BiomeType::Jungle;
    JungleFern.MeshPath = FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    JungleFern.SpawnProbability = 0.4f;
    JungleFern.ScaleRange = FVector2D(0.3f, 0.8f);
    JungleFern.CullDistance = 4000.0f;
    JungleFern.bCastShadows = false;
    SpawnRules.Add(JungleFern);

    // Plains vegetation (sparse)
    FWorld_PCGSpawnRule PlainsCycad;
    PlainsCycad.TargetBiome = EWorld_BiomeType::Plains;
    PlainsCycad.MeshPath = FSoftObjectPath(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    PlainsCycad.SpawnProbability = 0.05f;
    PlainsCycad.ScaleRange = FVector2D(0.5f, 1.5f);
    PlainsCycad.CullDistance = 12000.0f;
    PlainsCycad.bCastShadows = true;
    SpawnRules.Add(PlainsCycad);

    // Volcanic rocks
    FWorld_PCGSpawnRule VolcanicRock;
    VolcanicRock.TargetBiome = EWorld_BiomeType::Volcanic;
    VolcanicRock.MeshPath = FSoftObjectPath(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    VolcanicRock.SpawnProbability = 0.15f;
    VolcanicRock.ScaleRange = FVector2D(0.5f, 2.5f);
    VolcanicRock.CullDistance = 12000.0f;
    VolcanicRock.bCastShadows = true;
    SpawnRules.Add(VolcanicRock);

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Default prehistoric biomes configured (%d zones, %d spawn rules)."),
        BiomeZones.Num(), SpawnRules.Num());

#if WITH_EDITOR
    // Mark dirty so editor saves the changes
    MarkPackageDirty();
#endif
}

int32 ABiomeManager::FindBiomeIndexAtLocation(FVector WorldLocation) const
{
    float NearestDistSq = MAX_FLT;
    int32 NearestIndex = -1;

    for (int32 i = 0; i < BiomeZones.Num(); ++i)
    {
        const FWorld_BiomeConfig& Zone = BiomeZones[i];
        // Use 2D distance (ignore Z) for biome lookup
        FVector2D Loc2D(WorldLocation.X, WorldLocation.Y);
        FVector2D Center2D(Zone.Center.X, Zone.Center.Y);
        float DistSq = FVector2D::DistSquared(Loc2D, Center2D);
        float RadiusSq = Zone.Radius * Zone.Radius;

        if (DistSq <= RadiusSq && DistSq < NearestDistSq)
        {
            NearestDistSq = DistSq;
            NearestIndex = i;
        }
    }

    return NearestIndex;
}

#if WITH_EDITOR
void ABiomeManager::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Property changed — %d biome zones, %d spawn rules."),
        BiomeZones.Num(), SpawnRules.Num());
}
#endif

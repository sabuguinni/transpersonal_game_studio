#include "BiomeManager.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Update once per second — low cost

    // Initialize biome table in constructor so CDO is valid
    InitializeBiomeTable();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();

    // Re-initialize in case table was cleared
    if (BiomeTable.Num() == 0)
    {
        InitializeBiomeTable();
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome zones. WorldRadius=%.0f"),
        BiomeTable.Num(), WorldRadius);
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDebugDrawBiomes)
    {
        DebugDrawBiomeZones();
    }
}

void ABiomeManager::InitializeBiomeTable()
{
    BiomeTable.Empty();

    // Jungle — dense, humid, high dinosaur activity
    {
        FBiomeData Jungle;
        Jungle.BiomeType = EBiomeType::Jungle;
        Jungle.BiomeName = TEXT("Cretaceous Jungle");
        Jungle.FogColor = FLinearColor(0.4f, 0.7f, 0.3f, 1.0f);
        Jungle.FogDensity = 0.04f;
        Jungle.Temperature = 32.0f;
        Jungle.Humidity = 0.9f;
        Jungle.DinosaurSpawnMultiplier = 1.5f;
        Jungle.VegetationDensity = 0.95f;
        BiomeTable.Add(Jungle);
    }

    // Savanna — open, dry, good visibility
    {
        FBiomeData Savanna;
        Savanna.BiomeType = EBiomeType::Savanna;
        Savanna.BiomeName = TEXT("Cretaceous Savanna");
        Savanna.FogColor = FLinearColor(1.0f, 0.85f, 0.5f, 1.0f);
        Savanna.FogDensity = 0.01f;
        Savanna.Temperature = 35.0f;
        Savanna.Humidity = 0.25f;
        Savanna.DinosaurSpawnMultiplier = 1.2f;
        Savanna.VegetationDensity = 0.3f;
        BiomeTable.Add(Savanna);
    }

    // Swamp — low visibility, high humidity, dangerous
    {
        FBiomeData Swamp;
        Swamp.BiomeType = EBiomeType::Swamp;
        Swamp.BiomeName = TEXT("Cretaceous Swamp");
        Swamp.FogColor = FLinearColor(0.3f, 0.5f, 0.25f, 1.0f);
        Swamp.FogDensity = 0.08f;
        Swamp.Temperature = 28.0f;
        Swamp.Humidity = 1.0f;
        Swamp.DinosaurSpawnMultiplier = 1.8f;
        Swamp.VegetationDensity = 0.7f;
        BiomeTable.Add(Swamp);
    }

    // Volcanic Plains — extreme heat, low vegetation, unique fauna
    {
        FBiomeData Volcanic;
        Volcanic.BiomeType = EBiomeType::VolcanicPlains;
        Volcanic.BiomeName = TEXT("Volcanic Plains");
        Volcanic.FogColor = FLinearColor(0.8f, 0.4f, 0.1f, 1.0f);
        Volcanic.FogDensity = 0.05f;
        Volcanic.Temperature = 45.0f;
        Volcanic.Humidity = 0.1f;
        Volcanic.DinosaurSpawnMultiplier = 0.7f;
        Volcanic.VegetationDensity = 0.05f;
        BiomeTable.Add(Volcanic);
    }

    // River Delta — water access, moderate everything
    {
        FBiomeData River;
        River.BiomeType = EBiomeType::RiverDelta;
        River.BiomeName = TEXT("River Delta");
        River.FogColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
        River.FogDensity = 0.03f;
        River.Temperature = 26.0f;
        River.Humidity = 0.75f;
        River.DinosaurSpawnMultiplier = 1.3f;
        River.VegetationDensity = 0.6f;
        BiomeTable.Add(River);
    }

    // Forest Canopy — tall trees, filtered light, ambush danger
    {
        FBiomeData Forest;
        Forest.BiomeType = EBiomeType::ForestCanopy;
        Forest.BiomeName = TEXT("Forest Canopy");
        Forest.FogColor = FLinearColor(0.35f, 0.6f, 0.25f, 1.0f);
        Forest.FogDensity = 0.035f;
        Forest.Temperature = 27.0f;
        Forest.Humidity = 0.7f;
        Forest.DinosaurSpawnMultiplier = 1.4f;
        Forest.VegetationDensity = 0.85f;
        BiomeTable.Add(Forest);
    }

    // Coastal Flats — open, sea breeze, pterosaur territory
    {
        FBiomeData Coastal;
        Coastal.BiomeType = EBiomeType::CoastalFlats;
        Coastal.BiomeName = TEXT("Coastal Flats");
        Coastal.FogColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
        Coastal.FogDensity = 0.02f;
        Coastal.Temperature = 24.0f;
        Coastal.Humidity = 0.55f;
        Coastal.DinosaurSpawnMultiplier = 0.9f;
        Coastal.VegetationDensity = 0.2f;
        BiomeTable.Add(Coastal);
    }

    // Unknown fallback
    {
        FBiomeData Unknown;
        Unknown.BiomeType = EBiomeType::Unknown;
        Unknown.BiomeName = TEXT("Wilderness");
        Unknown.FogColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
        Unknown.FogDensity = 0.02f;
        Unknown.Temperature = 28.0f;
        Unknown.Humidity = 0.5f;
        Unknown.DinosaurSpawnMultiplier = 1.0f;
        Unknown.VegetationDensity = 0.4f;
        BiomeTable.Add(Unknown);
    }

    NumBiomeZones = BiomeTable.Num();
}

int32 ABiomeManager::ComputeBiomeSectorIndex(FVector WorldLocation) const
{
    if (BiomeTable.Num() == 0)
    {
        return 0;
    }

    // Use angle from world origin to determine biome sector
    // Each biome occupies an equal angular slice of the world
    float Angle = FMath::Atan2(WorldLocation.Y, WorldLocation.X); // -PI to PI
    float NormalizedAngle = (Angle + PI) / (2.0f * PI); // 0 to 1
    int32 SectorIndex = FMath::FloorToInt(NormalizedAngle * BiomeTable.Num());
    return FMath::Clamp(SectorIndex, 0, BiomeTable.Num() - 1);
}

EBiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    if (BiomeTable.Num() == 0)
    {
        return EBiomeType::Unknown;
    }

    int32 Index = ComputeBiomeSectorIndex(WorldLocation);
    return BiomeTable[Index].BiomeType;
}

FBiomeData ABiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    if (BiomeTable.Num() == 0)
    {
        FBiomeData Default;
        return Default;
    }

    int32 Index = ComputeBiomeSectorIndex(WorldLocation);
    return BiomeTable[Index];
}

FLinearColor ABiomeManager::GetBlendedFogColor(FVector WorldLocation) const
{
    if (BiomeTable.Num() < 2)
    {
        return FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    }

    int32 PrimaryIndex = ComputeBiomeSectorIndex(WorldLocation);
    int32 SecondaryIndex = (PrimaryIndex + 1) % BiomeTable.Num();

    // Distance from world origin determines blend factor
    float DistFromOrigin = FVector2D(WorldLocation.X, WorldLocation.Y).Size();
    float BlendAlpha = FMath::Clamp(
        FMath::Fmod(DistFromOrigin, BiomeBlendRadius) / BiomeBlendRadius,
        0.0f, 1.0f
    );

    return FLinearColor::LerpUsingHSV(
        BiomeTable[PrimaryIndex].FogColor,
        BiomeTable[SecondaryIndex].FogColor,
        BlendAlpha
    );
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.Temperature;
}

float ABiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.Humidity;
}

float ABiomeManager::GetDinosaurSpawnMultiplierAtLocation(FVector WorldLocation) const
{
    FBiomeData Data = GetBiomeDataAtLocation(WorldLocation);
    return Data.DinosaurSpawnMultiplier;
}

void ABiomeManager::DebugDrawBiomeZones() const
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World || BiomeTable.Num() == 0)
    {
        return;
    }

    float AngleStep = (2.0f * PI) / BiomeTable.Num();
    for (int32 i = 0; i < BiomeTable.Num(); i++)
    {
        float Angle = -PI + (i * AngleStep) + (AngleStep * 0.5f);
        float CenterX = FMath::Cos(Angle) * (WorldRadius * 0.5f);
        float CenterY = FMath::Sin(Angle) * (WorldRadius * 0.5f);
        FVector ZoneCenter(CenterX, CenterY, 200.0f);

        FColor DebugColor = FColor::MakeRandomSeededColor(i * 137);
        DrawDebugSphere(World, ZoneCenter, 300.0f, 12, DebugColor, false, 1.1f);
    }
#endif
}

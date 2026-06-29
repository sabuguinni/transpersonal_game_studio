#include "World/BiomeManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

// Static singleton cache
ABiomeManager* ABiomeManager::CachedInstance = nullptr;

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Populate default biome regions in constructor so CDO is valid
    InitialiseDefaultBiomes();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    CachedInstance = this;

    if (BiomeRegions.Num() == 0)
    {
        InitialiseDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Initialised with %d biome regions"), BiomeRegions.Num());
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ABiomeManager::InitialiseDefaultBiomes()
{
    BiomeRegions.Empty();

    // --- Savanna (central plains) ---
    FEng_BiomeData Savanna;
    Savanna.BiomeType = EEng_BiomeType::Savanna;
    Savanna.BiomeName = TEXT("Cretaceous Savanna");
    Savanna.TemperatureCelsius = 32.0f;
    Savanna.AnnualRainfallMM = 600.0f;
    Savanna.Humidity = 0.35f;
    Savanna.FogDensity = 0.01f;
    Savanna.SunIntensityMultiplier = 1.2f;
    Savanna.AmbientColorTint = FLinearColor(1.0f, 0.92f, 0.7f, 1.0f);
    Savanna.VegetationDensity = 0.4f;
    Savanna.PredatorSpawnWeight = 1.5f;
    Savanna.HerbivoreSpawnWeight = 2.0f;
    Savanna.BiomeBounds = FBox(FVector(-8000, -8000, -200), FVector(8000, 8000, 3000));
    BiomeRegions.Add(Savanna);

    // --- Jungle (north-east) ---
    FEng_BiomeData Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.BiomeName = TEXT("Dense Jungle");
    Jungle.TemperatureCelsius = 28.0f;
    Jungle.AnnualRainfallMM = 2500.0f;
    Jungle.Humidity = 0.9f;
    Jungle.FogDensity = 0.06f;
    Jungle.SunIntensityMultiplier = 0.6f;
    Jungle.AmbientColorTint = FLinearColor(0.5f, 0.85f, 0.4f, 1.0f);
    Jungle.VegetationDensity = 0.95f;
    Jungle.PredatorSpawnWeight = 1.2f;
    Jungle.HerbivoreSpawnWeight = 1.0f;
    Jungle.BiomeBounds = FBox(FVector(5000, 5000, -200), FVector(20000, 20000, 4000));
    BiomeRegions.Add(Jungle);

    // --- Swamp (south-west) ---
    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.BiomeName = TEXT("Primordial Swamp");
    Swamp.TemperatureCelsius = 26.0f;
    Swamp.AnnualRainfallMM = 1800.0f;
    Swamp.Humidity = 0.95f;
    Swamp.FogDensity = 0.12f;
    Swamp.SunIntensityMultiplier = 0.4f;
    Swamp.AmbientColorTint = FLinearColor(0.4f, 0.6f, 0.35f, 1.0f);
    Swamp.VegetationDensity = 0.85f;
    Swamp.PredatorSpawnWeight = 0.8f;
    Swamp.HerbivoreSpawnWeight = 0.6f;
    Swamp.BiomeBounds = FBox(FVector(-20000, -20000, -500), FVector(-5000, -5000, 500));
    BiomeRegions.Add(Swamp);

    // --- Volcanic (north-west) ---
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName = TEXT("Volcanic Badlands");
    Volcanic.TemperatureCelsius = 45.0f;
    Volcanic.AnnualRainfallMM = 150.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.FogDensity = 0.08f;
    Volcanic.SunIntensityMultiplier = 0.9f;
    Volcanic.AmbientColorTint = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.PredatorSpawnWeight = 0.5f;
    Volcanic.HerbivoreSpawnWeight = 0.2f;
    Volcanic.BiomeBounds = FBox(FVector(-20000, 5000, -200), FVector(-5000, 20000, 8000));
    BiomeRegions.Add(Volcanic);

    // --- Coastal Plain (east) ---
    FEng_BiomeData Coastal;
    Coastal.BiomeType = EEng_BiomeType::CoastalPlain;
    Coastal.BiomeName = TEXT("Coastal Floodplain");
    Coastal.TemperatureCelsius = 24.0f;
    Coastal.AnnualRainfallMM = 1200.0f;
    Coastal.Humidity = 0.7f;
    Coastal.FogDensity = 0.04f;
    Coastal.SunIntensityMultiplier = 1.0f;
    Coastal.AmbientColorTint = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    Coastal.VegetationDensity = 0.55f;
    Coastal.PredatorSpawnWeight = 0.7f;
    Coastal.HerbivoreSpawnWeight = 1.8f;
    Coastal.BiomeBounds = FBox(FVector(8000, -20000, -200), FVector(25000, 5000, 1000));
    BiomeRegions.Add(Coastal);

    // --- River Delta (south) ---
    FEng_BiomeData River;
    River.BiomeType = EEng_BiomeType::RiverDelta;
    River.BiomeName = TEXT("River Delta");
    River.TemperatureCelsius = 25.0f;
    River.AnnualRainfallMM = 1600.0f;
    River.Humidity = 0.8f;
    River.FogDensity = 0.05f;
    River.SunIntensityMultiplier = 0.85f;
    River.AmbientColorTint = FLinearColor(0.6f, 0.8f, 0.7f, 1.0f);
    River.VegetationDensity = 0.75f;
    River.PredatorSpawnWeight = 0.9f;
    River.HerbivoreSpawnWeight = 1.4f;
    River.BiomeBounds = FBox(FVector(-8000, -25000, -300), FVector(8000, -8000, 800));
    BiomeRegions.Add(River);
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    int32 Idx = FindNearestBiomeIndex(WorldLocation);
    if (BiomeRegions.IsValidIndex(Idx))
    {
        return BiomeRegions[Idx].BiomeType;
    }
    return EEng_BiomeType::Savanna;
}

FEng_BiomeData ABiomeManager::GetBiomeDataAtLocation(FVector WorldLocation) const
{
    int32 Idx = FindNearestBiomeIndex(WorldLocation);
    if (BiomeRegions.IsValidIndex(Idx))
    {
        return BiomeRegions[Idx];
    }
    // Return default savanna data if nothing found
    FEng_BiomeData Default;
    return Default;
}

FEng_BiomeData ABiomeManager::GetBlendedBiomeData(FVector WorldLocation) const
{
    if (BiomeRegions.Num() == 0)
    {
        return FEng_BiomeData();
    }

    // Find two nearest biomes and blend between them
    float BestDist1 = FLT_MAX, BestDist2 = FLT_MAX;
    int32 BestIdx1 = 0, BestIdx2 = -1;

    for (int32 i = 0; i < BiomeRegions.Num(); i++)
    {
        FVector Center = BiomeRegions[i].BiomeBounds.GetCenter();
        float Dist = FVector::Dist(WorldLocation, Center);
        if (Dist < BestDist1)
        {
            BestDist2 = BestDist1; BestIdx2 = BestIdx1;
            BestDist1 = Dist;     BestIdx1 = i;
        }
        else if (Dist < BestDist2)
        {
            BestDist2 = Dist; BestIdx2 = i;
        }
    }

    if (BestIdx2 < 0 || BestDist1 >= TransitionBlendDistance)
    {
        return BiomeRegions[BestIdx1];
    }

    // Blend alpha: 0 = fully biome1, 1 = fully biome2
    float Alpha = FMath::Clamp(BestDist1 / TransitionBlendDistance, 0.0f, 1.0f);

    const FEng_BiomeData& A = BiomeRegions[BestIdx1];
    const FEng_BiomeData& B = BiomeRegions[BestIdx2];

    FEng_BiomeData Blended;
    Blended.BiomeType = A.BiomeType; // dominant biome
    Blended.BiomeName = A.BiomeName;
    Blended.TemperatureCelsius   = FMath::Lerp(A.TemperatureCelsius,   B.TemperatureCelsius,   Alpha);
    Blended.AnnualRainfallMM     = FMath::Lerp(A.AnnualRainfallMM,     B.AnnualRainfallMM,     Alpha);
    Blended.Humidity             = FMath::Lerp(A.Humidity,             B.Humidity,             Alpha);
    Blended.FogDensity           = FMath::Lerp(A.FogDensity,           B.FogDensity,           Alpha);
    Blended.SunIntensityMultiplier = FMath::Lerp(A.SunIntensityMultiplier, B.SunIntensityMultiplier, Alpha);
    Blended.AmbientColorTint     = FLinearColor(
        FMath::Lerp(A.AmbientColorTint.R, B.AmbientColorTint.R, Alpha),
        FMath::Lerp(A.AmbientColorTint.G, B.AmbientColorTint.G, Alpha),
        FMath::Lerp(A.AmbientColorTint.B, B.AmbientColorTint.B, Alpha),
        1.0f
    );
    Blended.VegetationDensity    = FMath::Lerp(A.VegetationDensity,    B.VegetationDensity,    Alpha);
    Blended.PredatorSpawnWeight  = FMath::Lerp(A.PredatorSpawnWeight,  B.PredatorSpawnWeight,  Alpha);
    Blended.HerbivoreSpawnWeight = FMath::Lerp(A.HerbivoreSpawnWeight, B.HerbivoreSpawnWeight, Alpha);
    return Blended;
}

bool ABiomeManager::IsLocationInBiome(FVector WorldLocation, EEng_BiomeType BiomeType) const
{
    return GetBiomeAtLocation(WorldLocation) == BiomeType;
}

void ABiomeManager::RegisterBiomeRegion(const FEng_BiomeData& NewBiome)
{
    BiomeRegions.Add(NewBiome);
    UE_LOG(LogTemp, Log, TEXT("ABiomeManager: Registered new biome region '%s'"), *NewBiome.BiomeName);
}

void ABiomeManager::DrawBiomeBounds()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const TArray<FLinearColor> Colors = {
        FLinearColor::Yellow, FLinearColor::Green, FLinearColor(0.0f, 0.5f, 1.0f),
        FLinearColor::Red,    FLinearColor::Blue,  FLinearColor(0.0f, 1.0f, 0.5f)
    };

    for (int32 i = 0; i < BiomeRegions.Num(); i++)
    {
        const FEng_BiomeData& B = BiomeRegions[i];
        FColor DrawColor = Colors[i % Colors.Num()].ToFColor(true);
        DrawDebugBox(World, B.BiomeBounds.GetCenter(), B.BiomeBounds.GetExtent(), DrawColor, false, 10.0f, 0, 30.0f);
        DrawDebugString(World, B.BiomeBounds.GetCenter(), B.BiomeName, nullptr, DrawColor, 10.0f);
    }
}

ABiomeManager* ABiomeManager::GetInstance(UObject* WorldContextObject)
{
    if (CachedInstance)
    {
        return CachedInstance;
    }

    if (!WorldContextObject) return nullptr;
    UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
    if (!World) return nullptr;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(World, ABiomeManager::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        CachedInstance = Cast<ABiomeManager>(Found[0]);
    }
    return CachedInstance;
}

int32 ABiomeManager::FindNearestBiomeIndex(FVector WorldLocation) const
{
    int32 BestIdx = 0;
    float BestDist = FLT_MAX;

    for (int32 i = 0; i < BiomeRegions.Num(); i++)
    {
        // First check if location is inside bounds
        if (BiomeRegions[i].BiomeBounds.IsInsideOrOn(WorldLocation))
        {
            return i;
        }
        // Otherwise find nearest by center distance
        float Dist = FVector::Dist(WorldLocation, BiomeRegions[i].BiomeBounds.GetCenter());
        if (Dist < BestDist)
        {
            BestDist = Dist;
            BestIdx = i;
        }
    }
    return BestIdx;
}

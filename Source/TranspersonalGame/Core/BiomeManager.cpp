#include "BiomeManager.h"
#include "DrawDebugHelpers.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultBiomes();
}

void ABiomeManager::InitializeDefaultBiomes()
{
    BiomeMap.Empty();

    // --- Jungle (centre of map) ---
    FEng_BiomeData Jungle;
    Jungle.BiomeType        = EEng_BiomeType::Jungle;
    Jungle.WorldOrigin      = FVector(2000.0f, 2000.0f, 0.0f);
    Jungle.Radius           = 6000.0f;
    Jungle.Temperature      = 32.0f;
    Jungle.Humidity         = 0.9f;
    Jungle.FoliageDensity   = 1.0f;
    Jungle.FogColor         = FLinearColor(0.35f, 0.55f, 0.25f, 1.0f);
    Jungle.FogDensity       = 0.025f;
    Jungle.AllowedDinosaurSpecies = { "Velociraptor", "Compsognathus", "Dilophosaurus" };
    BiomeMap.Add(EEng_BiomeType::Jungle, Jungle);

    // --- Savanna (east) ---
    FEng_BiomeData Savanna;
    Savanna.BiomeType        = EEng_BiomeType::Savanna;
    Savanna.WorldOrigin      = FVector(8000.0f, 2000.0f, 0.0f);
    Savanna.Radius           = 5000.0f;
    Savanna.Temperature      = 38.0f;
    Savanna.Humidity         = 0.3f;
    Savanna.FoliageDensity   = 0.4f;
    Savanna.FogColor         = FLinearColor(0.7f, 0.6f, 0.3f, 1.0f);
    Savanna.FogDensity       = 0.01f;
    Savanna.AllowedDinosaurSpecies = { "TRex", "Triceratops", "Brachiosaurus", "Parasaurolophus" };
    BiomeMap.Add(EEng_BiomeType::Savanna, Savanna);

    // --- Swamp (south) ---
    FEng_BiomeData Swamp;
    Swamp.BiomeType        = EEng_BiomeType::Swamp;
    Swamp.WorldOrigin      = FVector(2000.0f, -4000.0f, 0.0f);
    Swamp.Radius           = 4000.0f;
    Swamp.Temperature      = 28.0f;
    Swamp.Humidity         = 1.0f;
    Swamp.FoliageDensity   = 0.8f;
    Swamp.FogColor         = FLinearColor(0.2f, 0.4f, 0.2f, 1.0f);
    Swamp.FogDensity       = 0.04f;
    Swamp.AllowedDinosaurSpecies = { "Spinosaurus", "Sarcosuchus", "Ankylosaurus" };
    BiomeMap.Add(EEng_BiomeType::Swamp, Swamp);

    // --- Volcanic (north-west) ---
    FEng_BiomeData Volcanic;
    Volcanic.BiomeType        = EEng_BiomeType::Volcanic;
    Volcanic.WorldOrigin      = FVector(-5000.0f, 5000.0f, 500.0f);
    Volcanic.Radius           = 3500.0f;
    Volcanic.Temperature      = 55.0f;
    Volcanic.Humidity         = 0.1f;
    Volcanic.FoliageDensity   = 0.1f;
    Volcanic.FogColor         = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
    Volcanic.FogDensity       = 0.035f;
    Volcanic.AllowedDinosaurSpecies = { "Pachycephalosaurus", "Protoceratops" };
    BiomeMap.Add(EEng_BiomeType::Volcanic, Volcanic);

    // --- Coastal (west) ---
    FEng_BiomeData Coastal;
    Coastal.BiomeType        = EEng_BiomeType::Coastal;
    Coastal.WorldOrigin      = FVector(-3000.0f, 0.0f, 0.0f);
    Coastal.Radius           = 4500.0f;
    Coastal.Temperature      = 26.0f;
    Coastal.Humidity         = 0.7f;
    Coastal.FoliageDensity   = 0.6f;
    Coastal.FogColor         = FLinearColor(0.4f, 0.55f, 0.65f, 1.0f);
    Coastal.FogDensity       = 0.02f;
    Coastal.AllowedDinosaurSpecies = { "Mosasaurus", "Pteranodon", "Tsintaosaurus" };
    BiomeMap.Add(EEng_BiomeType::Coastal, Coastal);

    // Sync to array for Blueprint visibility
    RegisteredBiomes.Empty();
    for (auto& Pair : BiomeMap)
    {
        RegisteredBiomes.Add(Pair.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biomes"), BiomeMap.Num());
}

void ABiomeManager::RegisterBiome(const FEng_BiomeData& BiomeData)
{
    BiomeMap.Add(BiomeData.BiomeType, BiomeData);

    // Keep array in sync
    bool bFound = false;
    for (FEng_BiomeData& Existing : RegisteredBiomes)
    {
        if (Existing.BiomeType == BiomeData.BiomeType)
        {
            Existing = BiomeData;
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        RegisteredBiomes.Add(BiomeData);
    }
}

EEng_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType ClosestBiome = EEng_BiomeType::Jungle;
    float ClosestDist = FLT_MAX;

    for (const auto& Pair : BiomeMap)
    {
        float Dist = FVector::Dist2D(WorldLocation, Pair.Value.WorldOrigin);
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = Pair.Key;
        }
    }

    return ClosestBiome;
}

bool ABiomeManager::GetBiomeData(EEng_BiomeType BiomeType, FEng_BiomeData& OutData) const
{
    const FEng_BiomeData* Found = BiomeMap.Find(BiomeType);
    if (Found)
    {
        OutData = *Found;
        return true;
    }
    return false;
}

float ABiomeManager::GetBlendWeightAtLocation(const FVector& WorldLocation, EEng_BiomeType BiomeType) const
{
    const FEng_BiomeData* Data = BiomeMap.Find(BiomeType);
    if (!Data)
    {
        return 0.0f;
    }

    float Dist = FVector::Dist2D(WorldLocation, Data->WorldOrigin);
    float InnerRadius = Data->Radius - BiomeTransitionWidth;

    if (Dist <= InnerRadius)
    {
        return 1.0f;
    }
    else if (Dist >= Data->Radius)
    {
        return 0.0f;
    }
    else
    {
        // Smooth blend in transition zone
        float Alpha = (Dist - InnerRadius) / BiomeTransitionWidth;
        return FMath::SmoothStep(0.0f, 1.0f, 1.0f - Alpha);
    }
}

TArray<FName> ABiomeManager::GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeData Data;
    if (GetBiomeData(Biome, Data))
    {
        return Data.AllowedDinosaurSpecies;
    }
    return TArray<FName>();
}

void ABiomeManager::DrawBiomeBoundaries()
{
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (!World) return;

    const TMap<EEng_BiomeType, FColor> BiomeColors = {
        { EEng_BiomeType::Jungle,   FColor::Green   },
        { EEng_BiomeType::Savanna,  FColor::Yellow  },
        { EEng_BiomeType::Swamp,    FColor(0, 100, 50) },
        { EEng_BiomeType::Volcanic, FColor::Red     },
        { EEng_BiomeType::Coastal,  FColor::Cyan    },
        { EEng_BiomeType::Forest,   FColor(0, 180, 0) },
    };

    for (const auto& Pair : BiomeMap)
    {
        const FColor* Color = BiomeColors.Find(Pair.Key);
        FColor DrawColor = Color ? *Color : FColor::White;

        DrawDebugCircle(
            World,
            Pair.Value.WorldOrigin,
            Pair.Value.Radius,
            64,
            DrawColor,
            false,
            10.0f,
            0,
            20.0f
        );

        DrawDebugString(
            World,
            Pair.Value.WorldOrigin + FVector(0, 0, 200),
            UEnum::GetValueAsString(Pair.Key),
            nullptr,
            DrawColor,
            10.0f
        );
    }
#endif
}

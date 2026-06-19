// BiomeManager.cpp
// Engine Architect #02 — P1 World Generation: Biome System Implementation
// Cycle: PROD_CYCLE_AUTO_20260619_010

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UBiomeManager::UBiomeManager()
{
    // Default biome thresholds
    TemperatureScale = 1.0f;
    MoistureScale = 1.0f;
    AltitudeScale = 1.0f;
    bBiomesInitialized = false;

    // Register default biomes
    RegisterDefaultBiomes();
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    RegisterDefaultBiomes();
    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biomes"), RegisteredBiomes.Num());
}

void UBiomeManager::Deinitialize()
{
    RegisteredBiomes.Empty();
    bBiomesInitialized = false;
    Super::Deinitialize();
}

void UBiomeManager::RegisterDefaultBiomes()
{
    RegisteredBiomes.Empty();

    // TROPICAL_JUNGLE — hot, wet, low altitude
    FEng_BiomeDefinition Jungle;
    Jungle.BiomeID = EEng_BiomeType::TropicalJungle;
    Jungle.BiomeName = FName("TropicalJungle");
    Jungle.DisplayName = FText::FromString("Tropical Jungle");
    Jungle.MinTemperature = 0.65f;
    Jungle.MaxTemperature = 1.0f;
    Jungle.MinMoisture = 0.70f;
    Jungle.MaxMoisture = 1.0f;
    Jungle.MinAltitude = 0.0f;
    Jungle.MaxAltitude = 0.35f;
    Jungle.FoliageDensity = 0.95f;
    Jungle.WaterBodyChance = 0.40f;
    Jungle.DangerLevel = 0.80f;
    Jungle.GroundColor = FLinearColor(0.05f, 0.25f, 0.05f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::TropicalJungle, Jungle);

    // SAVANNA — hot, dry, low altitude
    FEng_BiomeDefinition Savanna;
    Savanna.BiomeID = EEng_BiomeType::Savanna;
    Savanna.BiomeName = FName("Savanna");
    Savanna.DisplayName = FText::FromString("Savanna");
    Savanna.MinTemperature = 0.60f;
    Savanna.MaxTemperature = 1.0f;
    Savanna.MinMoisture = 0.10f;
    Savanna.MaxMoisture = 0.45f;
    Savanna.MinAltitude = 0.0f;
    Savanna.MaxAltitude = 0.40f;
    Savanna.FoliageDensity = 0.30f;
    Savanna.WaterBodyChance = 0.10f;
    Savanna.DangerLevel = 0.65f;
    Savanna.GroundColor = FLinearColor(0.55f, 0.45f, 0.15f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Savanna, Savanna);

    // TEMPERATE_FOREST — mild temp, moderate moisture
    FEng_BiomeDefinition TempForest;
    TempForest.BiomeID = EEng_BiomeType::TemperateForest;
    TempForest.BiomeName = FName("TemperateForest");
    TempForest.DisplayName = FText::FromString("Temperate Forest");
    TempForest.MinTemperature = 0.30f;
    TempForest.MaxTemperature = 0.65f;
    TempForest.MinMoisture = 0.40f;
    TempForest.MaxMoisture = 0.75f;
    TempForest.MinAltitude = 0.10f;
    TempForest.MaxAltitude = 0.60f;
    TempForest.FoliageDensity = 0.70f;
    TempForest.WaterBodyChance = 0.25f;
    TempForest.DangerLevel = 0.50f;
    TempForest.GroundColor = FLinearColor(0.15f, 0.35f, 0.10f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::TemperateForest, TempForest);

    // SWAMP — warm, very wet, very low altitude
    FEng_BiomeDefinition Swamp;
    Swamp.BiomeID = EEng_BiomeType::Swamp;
    Swamp.BiomeName = FName("Swamp");
    Swamp.DisplayName = FText::FromString("Swamp");
    Swamp.MinTemperature = 0.45f;
    Swamp.MaxTemperature = 0.80f;
    Swamp.MinMoisture = 0.80f;
    Swamp.MaxMoisture = 1.0f;
    Swamp.MinAltitude = 0.0f;
    Swamp.MaxAltitude = 0.15f;
    Swamp.FoliageDensity = 0.60f;
    Swamp.WaterBodyChance = 0.75f;
    Swamp.DangerLevel = 0.70f;
    Swamp.GroundColor = FLinearColor(0.10f, 0.20f, 0.08f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Swamp, Swamp);

    // VOLCANIC — extreme temp, dry, high altitude variation
    FEng_BiomeDefinition Volcanic;
    Volcanic.BiomeID = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName = FName("Volcanic");
    Volcanic.DisplayName = FText::FromString("Volcanic Region");
    Volcanic.MinTemperature = 0.75f;
    Volcanic.MaxTemperature = 1.0f;
    Volcanic.MinMoisture = 0.0f;
    Volcanic.MaxMoisture = 0.20f;
    Volcanic.MinAltitude = 0.50f;
    Volcanic.MaxAltitude = 1.0f;
    Volcanic.FoliageDensity = 0.05f;
    Volcanic.WaterBodyChance = 0.05f;
    Volcanic.DangerLevel = 1.0f;
    Volcanic.GroundColor = FLinearColor(0.20f, 0.05f, 0.02f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Volcanic, Volcanic);

    // COASTAL — moderate temp, high moisture, sea level
    FEng_BiomeDefinition Coastal;
    Coastal.BiomeID = EEng_BiomeType::Coastal;
    Coastal.BiomeName = FName("Coastal");
    Coastal.DisplayName = FText::FromString("Coastal");
    Coastal.MinTemperature = 0.35f;
    Coastal.MaxTemperature = 0.70f;
    Coastal.MinMoisture = 0.55f;
    Coastal.MaxMoisture = 0.90f;
    Coastal.MinAltitude = 0.0f;
    Coastal.MaxAltitude = 0.20f;
    Coastal.FoliageDensity = 0.45f;
    Coastal.WaterBodyChance = 0.90f;
    Coastal.DangerLevel = 0.40f;
    Coastal.GroundColor = FLinearColor(0.60f, 0.55f, 0.35f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Coastal, Coastal);

    // HIGHLAND — cool, moderate moisture, high altitude
    FEng_BiomeDefinition Highland;
    Highland.BiomeID = EEng_BiomeType::Highland;
    Highland.BiomeName = FName("Highland");
    Highland.DisplayName = FText::FromString("Highland");
    Highland.MinTemperature = 0.10f;
    Highland.MaxTemperature = 0.45f;
    Highland.MinMoisture = 0.25f;
    Highland.MaxMoisture = 0.65f;
    Highland.MinAltitude = 0.55f;
    Highland.MaxAltitude = 0.85f;
    Highland.FoliageDensity = 0.35f;
    Highland.WaterBodyChance = 0.15f;
    Highland.DangerLevel = 0.55f;
    Highland.GroundColor = FLinearColor(0.40f, 0.38f, 0.30f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Highland, Highland);

    // DESERT — hot, very dry, low-mid altitude
    FEng_BiomeDefinition Desert;
    Desert.BiomeID = EEng_BiomeType::Desert;
    Desert.BiomeName = FName("Desert");
    Desert.DisplayName = FText::FromString("Desert");
    Desert.MinTemperature = 0.70f;
    Desert.MaxTemperature = 1.0f;
    Desert.MinMoisture = 0.0f;
    Desert.MaxMoisture = 0.15f;
    Desert.MinAltitude = 0.05f;
    Desert.MaxAltitude = 0.50f;
    Desert.FoliageDensity = 0.05f;
    Desert.WaterBodyChance = 0.02f;
    Desert.DangerLevel = 0.75f;
    Desert.GroundColor = FLinearColor(0.75f, 0.60f, 0.30f, 1.0f);
    RegisteredBiomes.Add(EEng_BiomeType::Desert, Desert);
}

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    if (!bBiomesInitialized)
    {
        return EEng_BiomeType::TemperateForest;
    }

    float Temperature, Moisture, Altitude;
    SampleEnvironmentAt(WorldLocation, Temperature, Moisture, Altitude);

    return ClassifyBiome(Temperature, Moisture, Altitude);
}

FEng_BiomeDefinition UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(BiomeType);
    if (Found)
    {
        return *Found;
    }
    // Return default temperate forest if not found
    FEng_BiomeDefinition Default;
    Default.BiomeID = EEng_BiomeType::TemperateForest;
    Default.BiomeName = FName("Default");
    Default.DisplayName = FText::FromString("Unknown Biome");
    return Default;
}

TArray<EEng_BiomeType> UBiomeManager::GetAllBiomeTypes() const
{
    TArray<EEng_BiomeType> Keys;
    RegisteredBiomes.GetKeys(Keys);
    return Keys;
}

float UBiomeManager::GetBiomeDangerLevel(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(BiomeType);
    return Found ? Found->DangerLevel : 0.5f;
}

float UBiomeManager::GetFoliageDensityAt(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(Biome);
    return Found ? Found->FoliageDensity : 0.5f;
}

bool UBiomeManager::IsWaterBodyLocation(const FVector& WorldLocation) const
{
    EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(Biome);
    if (!Found) return false;

    // Use deterministic pseudo-random based on location
    float HashVal = FMath::Abs(FMath::Sin(WorldLocation.X * 0.001f + WorldLocation.Y * 0.0013f));
    return HashVal < Found->WaterBodyChance;
}

FLinearColor UBiomeManager::GetBiomeGroundColor(EEng_BiomeType BiomeType) const
{
    const FEng_BiomeDefinition* Found = RegisteredBiomes.Find(BiomeType);
    return Found ? Found->GroundColor : FLinearColor(0.3f, 0.3f, 0.3f, 1.0f);
}

void UBiomeManager::SampleEnvironmentAt(const FVector& WorldLocation, float& OutTemperature, float& OutMoisture, float& OutAltitude) const
{
    // Latitude-based temperature: hotter near equator (Y=0), cooler at extremes
    // World assumed to span -100000 to 100000 units in Y
    float NormalizedY = FMath::Clamp(FMath::Abs(WorldLocation.Y) / 100000.0f, 0.0f, 1.0f);
    float BaseTemp = 1.0f - NormalizedY;

    // Altitude reduces temperature
    float NormalizedZ = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);
    OutTemperature = FMath::Clamp((BaseTemp - NormalizedZ * 0.3f) * TemperatureScale, 0.0f, 1.0f);

    // Moisture: Perlin-like noise using sin/cos for determinism
    float MoistureNoise = (FMath::Sin(WorldLocation.X * 0.00008f) * 0.5f + 0.5f) *
                          (FMath::Cos(WorldLocation.Y * 0.00006f) * 0.5f + 0.5f);
    OutMoisture = FMath::Clamp(MoistureNoise * MoistureScale, 0.0f, 1.0f);

    // Altitude: normalized world Z
    OutAltitude = FMath::Clamp(NormalizedZ * AltitudeScale, 0.0f, 1.0f);
}

EEng_BiomeType UBiomeManager::ClassifyBiome(float Temperature, float Moisture, float Altitude) const
{
    // Score each biome by how well it matches the input parameters
    EEng_BiomeType BestMatch = EEng_BiomeType::TemperateForest;
    float BestScore = -1.0f;

    for (const auto& Pair : RegisteredBiomes)
    {
        const FEng_BiomeDefinition& Def = Pair.Value;

        // Check hard bounds
        if (Temperature < Def.MinTemperature || Temperature > Def.MaxTemperature) continue;
        if (Moisture < Def.MinMoisture || Moisture > Def.MaxMoisture) continue;
        if (Altitude < Def.MinAltitude || Altitude > Def.MaxAltitude) continue;

        // Score = how close to center of biome range (0..1 each axis)
        float TempCenter = (Def.MinTemperature + Def.MaxTemperature) * 0.5f;
        float MoistCenter = (Def.MinMoisture + Def.MaxMoisture) * 0.5f;
        float AltCenter = (Def.MinAltitude + Def.MaxAltitude) * 0.5f;

        float TempRange = FMath::Max(Def.MaxTemperature - Def.MinTemperature, 0.01f);
        float MoistRange = FMath::Max(Def.MaxMoisture - Def.MinMoisture, 0.01f);
        float AltRange = FMath::Max(Def.MaxAltitude - Def.MinAltitude, 0.01f);

        float Score = 1.0f - (FMath::Abs(Temperature - TempCenter) / TempRange +
                               FMath::Abs(Moisture - MoistCenter) / MoistRange +
                               FMath::Abs(Altitude - AltCenter) / AltRange) / 3.0f;

        if (Score > BestScore)
        {
            BestScore = Score;
            BestMatch = Pair.Key;
        }
    }

    return BestMatch;
}

FString UBiomeManager::GetBiomeDebugString(const FVector& WorldLocation) const
{
    float Temp, Moist, Alt;
    SampleEnvironmentAt(WorldLocation, Temp, Moist, Alt);
    EEng_BiomeType Biome = ClassifyBiome(Temp, Moist, Alt);
    FEng_BiomeDefinition Def = GetBiomeDefinition(Biome);

    return FString::Printf(TEXT("Biome:%s T:%.2f M:%.2f A:%.2f Danger:%.2f Foliage:%.2f"),
        *Def.DisplayName.ToString(), Temp, Moist, Alt, Def.DangerLevel, Def.FoliageDensity);
}

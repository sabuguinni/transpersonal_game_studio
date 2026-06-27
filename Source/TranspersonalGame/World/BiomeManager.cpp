#include "BiomeManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;
    SetupDefaultBiomeDefinitions();
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    if (!bBiomesInitialized)
    {
        InitializeBiomes();
    }
}

void ABiomeManager::InitializeBiomes()
{
    SetupDefaultBiomeDefinitions();
    TotalBiomeRegions = BiomeDefinitions.Num();
    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized %d biome types for prehistoric world."), TotalBiomeRegions);
}

void ABiomeManager::SetupDefaultBiomeDefinitions()
{
    BiomeDefinitions.Empty();

    // Jungle biome — dense, hot, humid, high danger
    {
        FEng_BiomeDefinition Jungle;
        Jungle.BiomeType = EEng_BiomeType::Jungle;
        Jungle.BiomeName = TEXT("Prehistoric Jungle");
        Jungle.Climate.TemperatureMin = 22.0f;
        Jungle.Climate.TemperatureMax = 38.0f;
        Jungle.Climate.HumidityPercent = 85.0f;
        Jungle.Climate.RainfallMM = 2500.0f;
        Jungle.Climate.WindSpeedKMH = 5.0f;
        Jungle.VegetationDensity = 0.95f;
        Jungle.DinosaurSpawnMultiplier = 1.5f;
        Jungle.DangerLevel = 0.8f;
        Jungle.FogColor = FLinearColor(0.2f, 0.5f, 0.2f, 1.0f);
        Jungle.FogDensity = 0.04f;
        BiomeDefinitions.Add(EEng_BiomeType::Jungle, Jungle);
    }

    // Savanna biome — open, warm, dry, medium danger
    {
        FEng_BiomeDefinition Savanna;
        Savanna.BiomeType = EEng_BiomeType::Savanna;
        Savanna.BiomeName = TEXT("Prehistoric Savanna");
        Savanna.Climate.TemperatureMin = 18.0f;
        Savanna.Climate.TemperatureMax = 42.0f;
        Savanna.Climate.HumidityPercent = 30.0f;
        Savanna.Climate.RainfallMM = 600.0f;
        Savanna.Climate.WindSpeedKMH = 25.0f;
        Savanna.VegetationDensity = 0.3f;
        Savanna.DinosaurSpawnMultiplier = 1.2f;
        Savanna.DangerLevel = 0.6f;
        Savanna.FogColor = FLinearColor(0.7f, 0.6f, 0.3f, 1.0f);
        Savanna.FogDensity = 0.01f;
        BiomeDefinitions.Add(EEng_BiomeType::Savanna, Savanna);
    }

    // Swamp biome — wet, murky, slow movement, high danger
    {
        FEng_BiomeDefinition Swamp;
        Swamp.BiomeType = EEng_BiomeType::Swamp;
        Swamp.BiomeName = TEXT("Prehistoric Swamp");
        Swamp.Climate.TemperatureMin = 18.0f;
        Swamp.Climate.TemperatureMax = 32.0f;
        Swamp.Climate.HumidityPercent = 95.0f;
        Swamp.Climate.RainfallMM = 1800.0f;
        Swamp.Climate.WindSpeedKMH = 3.0f;
        Swamp.VegetationDensity = 0.7f;
        Swamp.DinosaurSpawnMultiplier = 1.3f;
        Swamp.DangerLevel = 0.75f;
        Swamp.FogColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);
        Swamp.FogDensity = 0.06f;
        BiomeDefinitions.Add(EEng_BiomeType::Swamp, Swamp);
    }

    // Volcanic biome — extreme heat, toxic, very high danger
    {
        FEng_BiomeDefinition Volcanic;
        Volcanic.BiomeType = EEng_BiomeType::Volcanic;
        Volcanic.BiomeName = TEXT("Volcanic Badlands");
        Volcanic.Climate.TemperatureMin = 40.0f;
        Volcanic.Climate.TemperatureMax = 80.0f;
        Volcanic.Climate.HumidityPercent = 10.0f;
        Volcanic.Climate.RainfallMM = 50.0f;
        Volcanic.Climate.WindSpeedKMH = 30.0f;
        Volcanic.VegetationDensity = 0.05f;
        Volcanic.DinosaurSpawnMultiplier = 0.5f;
        Volcanic.DangerLevel = 0.95f;
        Volcanic.FogColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
        Volcanic.FogDensity = 0.08f;
        BiomeDefinitions.Add(EEng_BiomeType::Volcanic, Volcanic);
    }

    // Coastal biome — mild, breezy, medium danger
    {
        FEng_BiomeDefinition Coastal;
        Coastal.BiomeType = EEng_BiomeType::Coastal;
        Coastal.BiomeName = TEXT("Prehistoric Coast");
        Coastal.Climate.TemperatureMin = 16.0f;
        Coastal.Climate.TemperatureMax = 28.0f;
        Coastal.Climate.HumidityPercent = 70.0f;
        Coastal.Climate.RainfallMM = 900.0f;
        Coastal.Climate.WindSpeedKMH = 35.0f;
        Coastal.VegetationDensity = 0.4f;
        Coastal.DinosaurSpawnMultiplier = 0.8f;
        Coastal.DangerLevel = 0.4f;
        Coastal.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
        Coastal.FogDensity = 0.02f;
        BiomeDefinitions.Add(EEng_BiomeType::Coastal, Coastal);
    }

    // Forest biome — moderate, diverse, medium danger
    {
        FEng_BiomeDefinition Forest;
        Forest.BiomeType = EEng_BiomeType::Forest;
        Forest.BiomeName = TEXT("Ancient Forest");
        Forest.Climate.TemperatureMin = 10.0f;
        Forest.Climate.TemperatureMax = 25.0f;
        Forest.Climate.HumidityPercent = 65.0f;
        Forest.Climate.RainfallMM = 1100.0f;
        Forest.Climate.WindSpeedKMH = 10.0f;
        Forest.VegetationDensity = 0.8f;
        Forest.DinosaurSpawnMultiplier = 1.0f;
        Forest.DangerLevel = 0.5f;
        Forest.FogColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
        Forest.FogDensity = 0.025f;
        BiomeDefinitions.Add(EEng_BiomeType::Forest, Forest);
    }

    // Desert biome — extreme heat, dry, sparse, low danger
    {
        FEng_BiomeDefinition Desert;
        Desert.BiomeType = EEng_BiomeType::Desert;
        Desert.BiomeName = TEXT("Prehistoric Desert");
        Desert.Climate.TemperatureMin = 25.0f;
        Desert.Climate.TemperatureMax = 55.0f;
        Desert.Climate.HumidityPercent = 5.0f;
        Desert.Climate.RainfallMM = 100.0f;
        Desert.Climate.WindSpeedKMH = 40.0f;
        Desert.VegetationDensity = 0.05f;
        Desert.DinosaurSpawnMultiplier = 0.3f;
        Desert.DangerLevel = 0.45f;
        Desert.FogColor = FLinearColor(0.8f, 0.7f, 0.4f, 1.0f);
        Desert.FogDensity = 0.005f;
        BiomeDefinitions.Add(EEng_BiomeType::Desert, Desert);
    }

    // Tundra biome — cold, sparse, low danger
    {
        FEng_BiomeDefinition Tundra;
        Tundra.BiomeType = EEng_BiomeType::Tundra;
        Tundra.BiomeName = TEXT("Prehistoric Tundra");
        Tundra.Climate.TemperatureMin = -15.0f;
        Tundra.Climate.TemperatureMax = 5.0f;
        Tundra.Climate.HumidityPercent = 40.0f;
        Tundra.Climate.RainfallMM = 250.0f;
        Tundra.Climate.WindSpeedKMH = 50.0f;
        Tundra.VegetationDensity = 0.1f;
        Tundra.DinosaurSpawnMultiplier = 0.4f;
        Tundra.DangerLevel = 0.35f;
        Tundra.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
        Tundra.FogDensity = 0.015f;
        BiomeDefinitions.Add(EEng_BiomeType::Tundra, Tundra);
    }
}

float ABiomeManager::BiomeNoise(float X, float Y, int32 Seed) const
{
    // Deterministic pseudo-noise using sine waves
    float SeedF = static_cast<float>(Seed);
    float N = FMath::Sin(X * 0.0001f + SeedF * 0.1f) * FMath::Cos(Y * 0.00015f + SeedF * 0.07f);
    N += FMath::Sin(X * 0.00023f + Y * 0.00018f + SeedF * 0.05f) * 0.5f;
    N += FMath::Sin(X * 0.00007f - Y * 0.00009f) * 0.25f;
    return FMath::Clamp((N + 1.5f) / 3.0f, 0.0f, 1.0f);
}

EEng_BiomeType ABiomeManager::NoiseValueToBiome(float NoiseValue) const
{
    // Map 0-1 noise to biome types
    if (NoiseValue < 0.10f) return EEng_BiomeType::Tundra;
    if (NoiseValue < 0.20f) return EEng_BiomeType::Desert;
    if (NoiseValue < 0.35f) return EEng_BiomeType::Savanna;
    if (NoiseValue < 0.50f) return EEng_BiomeType::Coastal;
    if (NoiseValue < 0.65f) return EEng_BiomeType::Forest;
    if (NoiseValue < 0.75f) return EEng_BiomeType::Swamp;
    if (NoiseValue < 0.88f) return EEng_BiomeType::Jungle;
    return EEng_BiomeType::Volcanic;
}

FEng_BiomeSample ABiomeManager::SampleBiomeAtLocation(FVector WorldLocation) const
{
    FEng_BiomeSample Sample;

    float NoiseVal = BiomeNoise(WorldLocation.X, WorldLocation.Y, BiomeSeed);
    float NoiseVal2 = BiomeNoise(WorldLocation.X + 50000.0f, WorldLocation.Y + 30000.0f, BiomeSeed + 7);

    Sample.PrimaryBiome = NoiseValueToBiome(NoiseVal);
    Sample.SecondaryBiome = NoiseValueToBiome(NoiseVal2);
    Sample.BlendFactor = FMath::Frac(NoiseVal * 8.0f);  // 0-1 blend within biome band
    Sample.Altitude = WorldLocation.Z;

    // Get climate data
    if (BiomeDefinitions.Contains(Sample.PrimaryBiome))
    {
        const FEng_BiomeDefinition& Def = BiomeDefinitions[Sample.PrimaryBiome];
        float AltitudeFactor = FMath::Clamp(WorldLocation.Z / 5000.0f, 0.0f, 1.0f);
        Sample.Temperature = FMath::Lerp(Def.Climate.TemperatureMax, Def.Climate.TemperatureMin, AltitudeFactor);
        Sample.Humidity = Def.Climate.HumidityPercent;
    }

    return Sample;
}

EEng_BiomeType ABiomeManager::GetBiomeTypeAtLocation(FVector WorldLocation) const
{
    float NoiseVal = BiomeNoise(WorldLocation.X, WorldLocation.Y, BiomeSeed);
    return NoiseValueToBiome(NoiseVal);
}

FEng_BiomeDefinition ABiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType) const
{
    if (BiomeDefinitions.Contains(BiomeType))
    {
        return BiomeDefinitions[BiomeType];
    }
    return FEng_BiomeDefinition();
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    FEng_BiomeSample Sample = SampleBiomeAtLocation(WorldLocation);
    return Sample.Temperature;
}

float ABiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    FEng_BiomeSample Sample = SampleBiomeAtLocation(WorldLocation);
    return Sample.Humidity;
}

float ABiomeManager::GetDangerLevelAtLocation(FVector WorldLocation) const
{
    EEng_BiomeType BiomeType = GetBiomeTypeAtLocation(WorldLocation);
    if (BiomeDefinitions.Contains(BiomeType))
    {
        return BiomeDefinitions[BiomeType].DangerLevel;
    }
    return 0.5f;
}

void ABiomeManager::DebugLogBiomeAtPlayerLocation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager Debug: No player pawn found."));
        return;
    }

    FVector PlayerLoc = PlayerPawn->GetActorLocation();
    FEng_BiomeSample Sample = SampleBiomeAtLocation(PlayerLoc);

    UE_LOG(LogTemp, Log, TEXT("=== BIOME DEBUG at (%.0f, %.0f, %.0f) ==="), PlayerLoc.X, PlayerLoc.Y, PlayerLoc.Z);
    UE_LOG(LogTemp, Log, TEXT("  Primary Biome: %d"), (int32)Sample.PrimaryBiome);
    UE_LOG(LogTemp, Log, TEXT("  Secondary Biome: %d"), (int32)Sample.SecondaryBiome);
    UE_LOG(LogTemp, Log, TEXT("  Blend Factor: %.2f"), Sample.BlendFactor);
    UE_LOG(LogTemp, Log, TEXT("  Temperature: %.1f C"), Sample.Temperature);
    UE_LOG(LogTemp, Log, TEXT("  Humidity: %.1f%%"), Sample.Humidity);
    UE_LOG(LogTemp, Log, TEXT("  Danger Level: %.2f"), GetDangerLevelAtLocation(PlayerLoc));
}

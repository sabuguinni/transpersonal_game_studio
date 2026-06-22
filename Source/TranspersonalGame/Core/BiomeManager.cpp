#include "BiomeManager.h"
#include "Math/UnrealMathUtility.h"

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Default biome definitions for prehistoric world
    FEng_BiomeData Forest;
    Forest.BiomeType = EEng_BiomeType::Forest;
    Forest.BiomeName = TEXT("Prehistoric Forest");
    Forest.Temperature = 22.0f;
    Forest.Humidity = 0.75f;
    Forest.VegetationDensity = 0.9f;
    Forest.DinosaurSpawnWeight = 1.2f;
    Forest.FogColor = FLinearColor(0.3f, 0.5f, 0.3f, 1.0f);
    Forest.FogDensity = 0.03f;
    BiomeDefinitions.Add(Forest);

    FEng_BiomeData Plains;
    Plains.BiomeType = EEng_BiomeType::Plains;
    Plains.BiomeName = TEXT("Open Plains");
    Plains.Temperature = 28.0f;
    Plains.Humidity = 0.35f;
    Plains.VegetationDensity = 0.4f;
    Plains.DinosaurSpawnWeight = 1.5f;
    Plains.FogColor = FLinearColor(0.6f, 0.65f, 0.5f, 1.0f);
    Plains.FogDensity = 0.01f;
    BiomeDefinitions.Add(Plains);

    FEng_BiomeData Swamp;
    Swamp.BiomeType = EEng_BiomeType::Swamp;
    Swamp.BiomeName = TEXT("Prehistoric Swamp");
    Swamp.Temperature = 30.0f;
    Swamp.Humidity = 0.95f;
    Swamp.VegetationDensity = 0.8f;
    Swamp.DinosaurSpawnWeight = 0.8f;
    Swamp.FogColor = FLinearColor(0.4f, 0.5f, 0.3f, 1.0f);
    Swamp.FogDensity = 0.06f;
    BiomeDefinitions.Add(Swamp);

    FEng_BiomeData Volcanic;
    Volcanic.BiomeType = EEng_BiomeType::Volcanic;
    Volcanic.BiomeName = TEXT("Volcanic Badlands");
    Volcanic.Temperature = 55.0f;
    Volcanic.Humidity = 0.1f;
    Volcanic.VegetationDensity = 0.05f;
    Volcanic.DinosaurSpawnWeight = 0.3f;
    Volcanic.FogColor = FLinearColor(0.7f, 0.4f, 0.2f, 1.0f);
    Volcanic.FogDensity = 0.08f;
    BiomeDefinitions.Add(Volcanic);

    FEng_BiomeData Jungle;
    Jungle.BiomeType = EEng_BiomeType::Jungle;
    Jungle.BiomeName = TEXT("Dense Jungle");
    Jungle.Temperature = 35.0f;
    Jungle.Humidity = 0.9f;
    Jungle.VegetationDensity = 1.0f;
    Jungle.DinosaurSpawnWeight = 1.8f;
    Jungle.FogColor = FLinearColor(0.2f, 0.45f, 0.2f, 1.0f);
    Jungle.FogDensity = 0.04f;
    BiomeDefinitions.Add(Jungle);
}

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
}

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

FEng_BiomeData ABiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Check registered zones first (explicit designer-placed zones)
    float BestWeight = -1.0f;
    int32 BestZoneIdx = -1;

    for (int32 i = 0; i < RegisteredZones.Num(); ++i)
    {
        const FBiomeZone& Zone = RegisteredZones[i];
        float Dist = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Dist <= Zone.Radius)
        {
            // Weight by inverse distance (closer = stronger influence)
            float Weight = 1.0f - (Dist / Zone.Radius);
            if (Weight > BestWeight)
            {
                BestWeight = Weight;
                BestZoneIdx = i;
            }
        }
    }

    if (BestZoneIdx >= 0)
    {
        return RegisteredZones[BestZoneIdx].Data;
    }

    // Fallback: noise-based procedural biome
    float NoiseX = WorldLocation.X / BiomeScale;
    float NoiseY = WorldLocation.Y / BiomeScale;
    float TempNoise = SampleNoise(NoiseX, NoiseY);
    float HumidNoise = SampleNoise(NoiseX + 100.0f, NoiseY + 100.0f);

    float Temperature = FMath::Lerp(5.0f, 55.0f, TempNoise);
    float Humidity = FMath::Lerp(0.0f, 1.0f, HumidNoise);

    EEng_BiomeType Type = ClassifyBiome(Temperature, Humidity);

    // Find matching biome definition
    for (const FEng_BiomeData& Def : BiomeDefinitions)
    {
        if (Def.BiomeType == Type)
        {
            return Def;
        }
    }

    // Return default plains if no match
    FEng_BiomeData Default;
    Default.BiomeType = DefaultBiome;
    Default.BiomeName = TEXT("Default Plains");
    Default.Temperature = Temperature;
    Default.Humidity = Humidity;
    return Default;
}

EEng_BiomeType ABiomeManager::GetBiomeTypeAtLocation(FVector WorldLocation) const
{
    return GetBiomeAtLocation(WorldLocation).BiomeType;
}

float ABiomeManager::GetTemperatureAtLocation(FVector WorldLocation) const
{
    return GetBiomeAtLocation(WorldLocation).Temperature;
}

float ABiomeManager::GetHumidityAtLocation(FVector WorldLocation) const
{
    return GetBiomeAtLocation(WorldLocation).Humidity;
}

void ABiomeManager::RegisterBiomeZone(FVector Center, float Radius, FEng_BiomeData BiomeData)
{
    FBiomeZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = FMath::Max(Radius, 100.0f); // Minimum 1m radius
    NewZone.Data = BiomeData;
    RegisteredZones.Add(NewZone);
}

int32 ABiomeManager::GetBiomeZoneCount() const
{
    return RegisteredZones.Num();
}

float ABiomeManager::SampleNoise(float X, float Y) const
{
    // Simple deterministic pseudo-noise based on seed
    // Uses sin-based approximation for lightweight noise without external dependencies
    float S = static_cast<float>(BiomeSeed);
    float Val = FMath::Sin(X * 127.1f + S) * 43758.5453f;
    Val = Val - FMath::FloorToFloat(Val);
    float Val2 = FMath::Sin(Y * 311.7f + S + Val) * 43758.5453f;
    Val2 = Val2 - FMath::FloorToFloat(Val2);
    return FMath::Clamp((Val + Val2) * 0.5f, 0.0f, 1.0f);
}

EEng_BiomeType ABiomeManager::ClassifyBiome(float Temperature, float Humidity) const
{
    // Whittaker biome classification adapted for prehistoric world
    if (Temperature > 45.0f)
    {
        return EEng_BiomeType::Volcanic;
    }
    if (Temperature > 30.0f && Humidity > 0.7f)
    {
        return EEng_BiomeType::Jungle;
    }
    if (Temperature > 25.0f && Humidity > 0.5f)
    {
        return EEng_BiomeType::Swamp;
    }
    if (Temperature > 20.0f && Humidity > 0.6f)
    {
        return EEng_BiomeType::Forest;
    }
    if (Humidity < 0.2f)
    {
        return EEng_BiomeType::Desert;
    }
    if (Temperature < 10.0f)
    {
        return EEng_BiomeType::Tundra;
    }
    if (Humidity > 0.4f && Temperature > 15.0f)
    {
        return EEng_BiomeType::Coastal;
    }
    return EEng_BiomeType::Plains;
}

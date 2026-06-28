// BiomeManager.cpp — Full implementation of biome classification and query system
// Engine Architect #02 — PROD_CYCLE_AUTO_20260628_006
// Prehistoric survival game — NO spiritual content

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogBiomeManager, Log, All);

// ─────────────────────────────────────────────────────────────────────────────
// UBiomeManager — UWorldSubsystem implementation
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Register all biome definitions on startup
    RegisterDefaultBiomes();

    UE_LOG(LogBiomeManager, Log, TEXT("BiomeManager initialized — %d biomes registered"), BiomeRegistry.Num());
}

void UBiomeManager::Deinitialize()
{
    BiomeRegistry.Empty();
    Super::Deinitialize();
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome Registration
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::RegisterDefaultBiomes()
{
    // JUNGLE BIOME — hot, humid, dense vegetation
    {
        FEng_BiomeDefinition Jungle;
        Jungle.BiomeID        = EEng_BiomeType::Jungle;
        Jungle.DisplayName    = FText::FromString(TEXT("Prehistoric Jungle"));
        Jungle.MinTemperature = 25.0f;
        Jungle.MaxTemperature = 38.0f;
        Jungle.MinHumidity    = 0.75f;
        Jungle.MaxHumidity    = 1.0f;
        Jungle.MinAltitude    = 0.0f;
        Jungle.MaxAltitude    = 400.0f;
        Jungle.FoliageDensity = 0.9f;
        Jungle.DangerLevel    = 0.7f;
        Jungle.AmbientColor   = FLinearColor(0.1f, 0.35f, 0.05f, 1.0f);
        Jungle.FogDensity     = 0.04f;
        BiomeRegistry.Add(EEng_BiomeType::Jungle, Jungle);
    }

    // SAVANNA BIOME — warm, dry, open plains
    {
        FEng_BiomeDefinition Savanna;
        Savanna.BiomeID        = EEng_BiomeType::Savanna;
        Savanna.DisplayName    = FText::FromString(TEXT("Prehistoric Savanna"));
        Savanna.MinTemperature = 20.0f;
        Savanna.MaxTemperature = 40.0f;
        Savanna.MinHumidity    = 0.1f;
        Savanna.MaxHumidity    = 0.4f;
        Savanna.MinAltitude    = 0.0f;
        Savanna.MaxAltitude    = 300.0f;
        Savanna.FoliageDensity = 0.3f;
        Savanna.DangerLevel    = 0.5f;
        Savanna.AmbientColor   = FLinearColor(0.7f, 0.55f, 0.2f, 1.0f);
        Savanna.FogDensity     = 0.01f;
        BiomeRegistry.Add(EEng_BiomeType::Savanna, Savanna);
    }

    // SWAMP BIOME — cool, very humid, murky
    {
        FEng_BiomeDefinition Swamp;
        Swamp.BiomeID        = EEng_BiomeType::Swamp;
        Swamp.DisplayName    = FText::FromString(TEXT("Prehistoric Swamp"));
        Swamp.MinTemperature = 15.0f;
        Swamp.MaxTemperature = 28.0f;
        Swamp.MinHumidity    = 0.8f;
        Swamp.MaxHumidity    = 1.0f;
        Swamp.MinAltitude    = -20.0f;
        Swamp.MaxAltitude    = 50.0f;
        Swamp.FoliageDensity = 0.6f;
        Swamp.DangerLevel    = 0.8f;
        Swamp.AmbientColor   = FLinearColor(0.15f, 0.25f, 0.1f, 1.0f);
        Swamp.FogDensity     = 0.08f;
        BiomeRegistry.Add(EEng_BiomeType::Swamp, Swamp);
    }

    // VOLCANIC BIOME — extreme heat, low humidity, barren
    {
        FEng_BiomeDefinition Volcanic;
        Volcanic.BiomeID        = EEng_BiomeType::Volcanic;
        Volcanic.DisplayName    = FText::FromString(TEXT("Volcanic Badlands"));
        Volcanic.MinTemperature = 35.0f;
        Volcanic.MaxTemperature = 80.0f;
        Volcanic.MinHumidity    = 0.0f;
        Volcanic.MaxHumidity    = 0.15f;
        Volcanic.MinAltitude    = 200.0f;
        Volcanic.MaxAltitude    = 2000.0f;
        Volcanic.FoliageDensity = 0.05f;
        Volcanic.DangerLevel    = 1.0f;
        Volcanic.AmbientColor   = FLinearColor(0.6f, 0.15f, 0.02f, 1.0f);
        Volcanic.FogDensity     = 0.06f;
        BiomeRegistry.Add(EEng_BiomeType::Volcanic, Volcanic);
    }

    // COASTAL BIOME — temperate, moderate humidity, beaches
    {
        FEng_BiomeDefinition Coastal;
        Coastal.BiomeID        = EEng_BiomeType::Coastal;
        Coastal.DisplayName    = FText::FromString(TEXT("Prehistoric Coast"));
        Coastal.MinTemperature = 18.0f;
        Coastal.MaxTemperature = 30.0f;
        Coastal.MinHumidity    = 0.5f;
        Coastal.MaxHumidity    = 0.8f;
        Coastal.MinAltitude    = -5.0f;
        Coastal.MaxAltitude    = 80.0f;
        Coastal.FoliageDensity = 0.4f;
        Coastal.DangerLevel    = 0.4f;
        Coastal.AmbientColor   = FLinearColor(0.4f, 0.6f, 0.7f, 1.0f);
        Coastal.FogDensity     = 0.03f;
        BiomeRegistry.Add(EEng_BiomeType::Coastal, Coastal);
    }

    // HIGHLAND BIOME — cold, low humidity, rocky terrain
    {
        FEng_BiomeDefinition Highland;
        Highland.BiomeID        = EEng_BiomeType::Highland;
        Highland.DisplayName    = FText::FromString(TEXT("Highland Plateau"));
        Highland.MinTemperature = 0.0f;
        Highland.MaxTemperature = 18.0f;
        Highland.MinHumidity    = 0.2f;
        Highland.MaxHumidity    = 0.5f;
        Highland.MinAltitude    = 500.0f;
        Highland.MaxAltitude    = 3000.0f;
        Highland.FoliageDensity = 0.15f;
        Highland.DangerLevel    = 0.6f;
        Highland.AmbientColor   = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
        Highland.FogDensity     = 0.05f;
        BiomeRegistry.Add(EEng_BiomeType::Highland, Highland);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Biome Query — classify a world position into a biome
// ─────────────────────────────────────────────────────────────────────────────

EEng_BiomeType UBiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Sample altitude from Z coordinate (scaled: 1 UE unit = 1 cm, so /100 = metres)
    const float AltitudeMetres = WorldLocation.Z / 100.0f;

    // Sample humidity and temperature using Perlin-like noise approximation
    // (deterministic based on XY position — no random seed needed)
    const float NX = WorldLocation.X * 0.0001f;
    const float NY = WorldLocation.Y * 0.0001f;

    // Simple sinusoidal approximation for biome blending
    const float HumidityRaw   = (FMath::Sin(NX * 2.3f + 0.5f) * FMath::Cos(NY * 1.7f + 1.2f) + 1.0f) * 0.5f;
    const float TemperatureRaw = (FMath::Sin(NX * 1.1f - 0.8f) * FMath::Sin(NY * 2.9f + 0.3f) + 1.0f) * 0.5f;

    const float Humidity    = FMath::Clamp(HumidityRaw, 0.0f, 1.0f);
    const float Temperature = FMath::Lerp(0.0f, 50.0f, FMath::Clamp(TemperatureRaw, 0.0f, 1.0f));

    // Altitude-first classification
    if (AltitudeMetres > 500.0f)  return EEng_BiomeType::Highland;
    if (AltitudeMetres > 200.0f && Temperature > 35.0f) return EEng_BiomeType::Volcanic;
    if (AltitudeMetres < 0.0f)    return EEng_BiomeType::Coastal;

    // Humidity + temperature matrix
    if (Temperature > 25.0f && Humidity > 0.7f) return EEng_BiomeType::Jungle;
    if (Temperature > 15.0f && Humidity < 0.15f && AltitudeMetres < 50.0f) return EEng_BiomeType::Swamp;
    if (Temperature > 20.0f && Humidity < 0.4f) return EEng_BiomeType::Savanna;
    if (Humidity > 0.5f && AltitudeMetres < 80.0f) return EEng_BiomeType::Coastal;

    // Default fallback
    return EEng_BiomeType::Savanna;
}

bool UBiomeManager::GetBiomeDefinition(EEng_BiomeType BiomeType, FEng_BiomeDefinition& OutDefinition) const
{
    if (const FEng_BiomeDefinition* Found = BiomeRegistry.Find(BiomeType))
    {
        OutDefinition = *Found;
        return true;
    }
    return false;
}

FLinearColor UBiomeManager::GetBiomeAmbientColor(const FVector& WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(Biome, Def))
    {
        return Def.AmbientColor;
    }
    return FLinearColor::White;
}

float UBiomeManager::GetBiomeDangerLevel(const FVector& WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(Biome, Def))
    {
        return Def.DangerLevel;
    }
    return 0.5f;
}

float UBiomeManager::GetBiomeFoliageDensity(const FVector& WorldLocation) const
{
    const EEng_BiomeType Biome = GetBiomeAtLocation(WorldLocation);
    FEng_BiomeDefinition Def;
    if (GetBiomeDefinition(Biome, Def))
    {
        return Def.FoliageDensity;
    }
    return 0.5f;
}

TArray<EEng_BiomeType> UBiomeManager::GetAllRegisteredBiomes() const
{
    TArray<EEng_BiomeType> Keys;
    BiomeRegistry.GetKeys(Keys);
    return Keys;
}

// ─────────────────────────────────────────────────────────────────────────────
// Debug visualization
// ─────────────────────────────────────────────────────────────────────────────

void UBiomeManager::DebugDrawBiomeBoundaries(float Duration) const
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Sample a grid of points and draw colored spheres
    const float GridStep  = 500.0f;
    const float GridRange = 5000.0f;

    for (float X = -GridRange; X <= GridRange; X += GridStep)
    {
        for (float Y = -GridRange; Y <= GridRange; Y += GridStep)
        {
            const FVector SamplePos(X, Y, 0.0f);
            const EEng_BiomeType Biome = GetBiomeAtLocation(SamplePos);
            FEng_BiomeDefinition Def;
            FColor DebugColor = FColor::White;
            if (GetBiomeDefinition(Biome, Def))
            {
                DebugColor = Def.AmbientColor.ToFColor(true);
            }
            DrawDebugSphere(World, SamplePos, 50.0f, 6, DebugColor, false, Duration);
        }
    }
}

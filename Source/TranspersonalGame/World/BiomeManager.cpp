// BiomeManager.cpp — Engine Architect #02 — PROD_CYCLE_AUTO_20260630_001
// Manages biome detection, transitions, and environmental parameters for the prehistoric world.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UBiomeManager::UBiomeManager()
{
    // Default biome parameters
    CurrentBiome = EBiomeType::Forest;
    PreviousBiome = EBiomeType::Forest;
    BiomeTransitionAlpha = 1.0f;
    TransitionDuration = 5.0f;
    bIsTransitioning = false;

    // Temperature ranges per biome (Celsius)
    BiomeTemperatures.Add(EBiomeType::Forest,     FVector2D(18.0f, 28.0f));
    BiomeTemperatures.Add(EBiomeType::Jungle,     FVector2D(25.0f, 38.0f));
    BiomeTemperatures.Add(EBiomeType::Swamp,      FVector2D(20.0f, 32.0f));
    BiomeTemperatures.Add(EBiomeType::Desert,     FVector2D(35.0f, 55.0f));
    BiomeTemperatures.Add(EBiomeType::Volcanic,   FVector2D(45.0f, 80.0f));
    BiomeTemperatures.Add(EBiomeType::Coastal,    FVector2D(22.0f, 30.0f));
    BiomeTemperatures.Add(EBiomeType::Grassland,  FVector2D(15.0f, 30.0f));
    BiomeTemperatures.Add(EBiomeType::Mountain,   FVector2D(0.0f,  15.0f));
    BiomeTemperatures.Add(EBiomeType::River,      FVector2D(16.0f, 26.0f));
    BiomeTemperatures.Add(EBiomeType::Cave,       FVector2D(10.0f, 16.0f));

    // Humidity ranges (0-1)
    BiomeHumidity.Add(EBiomeType::Forest,    FVector2D(0.5f, 0.75f));
    BiomeHumidity.Add(EBiomeType::Jungle,    FVector2D(0.8f, 1.0f));
    BiomeHumidity.Add(EBiomeType::Swamp,     FVector2D(0.85f, 1.0f));
    BiomeHumidity.Add(EBiomeType::Desert,    FVector2D(0.05f, 0.2f));
    BiomeHumidity.Add(EBiomeType::Volcanic,  FVector2D(0.1f, 0.3f));
    BiomeHumidity.Add(EBiomeType::Coastal,   FVector2D(0.6f, 0.8f));
    BiomeHumidity.Add(EBiomeType::Grassland, FVector2D(0.3f, 0.55f));
    BiomeHumidity.Add(EBiomeType::Mountain,  FVector2D(0.4f, 0.65f));
    BiomeHumidity.Add(EBiomeType::River,     FVector2D(0.7f, 0.9f));
    BiomeHumidity.Add(EBiomeType::Cave,      FVector2D(0.6f, 0.85f));

    // Danger levels (0-10)
    BiomeDangerLevel.Add(EBiomeType::Forest,    4);
    BiomeDangerLevel.Add(EBiomeType::Jungle,    7);
    BiomeDangerLevel.Add(EBiomeType::Swamp,     6);
    BiomeDangerLevel.Add(EBiomeType::Desert,    5);
    BiomeDangerLevel.Add(EBiomeType::Volcanic,  9);
    BiomeDangerLevel.Add(EBiomeType::Coastal,   3);
    BiomeDangerLevel.Add(EBiomeType::Grassland, 3);
    BiomeDangerLevel.Add(EBiomeType::Mountain,  6);
    BiomeDangerLevel.Add(EBiomeType::River,     4);
    BiomeDangerLevel.Add(EBiomeType::Cave,      7);
}

void UBiomeManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager initialized"));
}

void UBiomeManager::Deinitialize()
{
    Super::Deinitialize();
}

EBiomeType UBiomeManager::GetBiomeAtLocation(FVector WorldLocation) const
{
    // Biome detection based on world coordinates using noise-like heuristics.
    // In production this will sample the PCG biome map texture.
    // For now: use coordinate-based zone assignment.

    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    float Z = WorldLocation.Z;

    // Elevation-based biomes
    if (Z > 8000.0f) return EBiomeType::Mountain;
    if (Z < -200.0f) return EBiomeType::Cave;

    // Distance from origin determines biome zones
    float DistFromOrigin = FMath::Sqrt(X * X + Y * Y);

    // Quadrant-based biome assignment (will be replaced by PCG texture sampling)
    if (X > 5000.0f && Y > 5000.0f)   return EBiomeType::Jungle;
    if (X > 5000.0f && Y < -5000.0f)  return EBiomeType::Desert;
    if (X < -5000.0f && Y > 5000.0f)  return EBiomeType::Swamp;
    if (X < -5000.0f && Y < -5000.0f) return EBiomeType::Volcanic;
    if (FMath::Abs(X) < 2000.0f && FMath::Abs(Y) < 500.0f) return EBiomeType::River;
    if (DistFromOrigin > 12000.0f)     return EBiomeType::Coastal;
    if (Z > 3000.0f)                   return EBiomeType::Mountain;
    if (DistFromOrigin < 3000.0f)      return EBiomeType::Grassland;

    return EBiomeType::Forest;
}

FBiomeEnvironmentData UBiomeManager::GetEnvironmentData(EBiomeType Biome) const
{
    FBiomeEnvironmentData Data;
    Data.BiomeType = Biome;

    // Temperature
    if (BiomeTemperatures.Contains(Biome))
    {
        FVector2D TempRange = BiomeTemperatures[Biome];
        Data.Temperature = (TempRange.X + TempRange.Y) * 0.5f;
    }

    // Humidity
    if (BiomeHumidity.Contains(Biome))
    {
        FVector2D HumRange = BiomeHumidity[Biome];
        Data.Humidity = (HumRange.X + HumRange.Y) * 0.5f;
    }

    // Danger
    if (BiomeDangerLevel.Contains(Biome))
    {
        Data.DangerLevel = BiomeDangerLevel[Biome];
    }

    // Biome-specific environment settings
    switch (Biome)
    {
    case EBiomeType::Forest:
        Data.FogDensity = 0.02f;
        Data.WindStrength = 0.4f;
        Data.AmbientSoundTag = FName("Forest_Ambient");
        Data.SkyColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f);
        break;
    case EBiomeType::Jungle:
        Data.FogDensity = 0.04f;
        Data.WindStrength = 0.2f;
        Data.AmbientSoundTag = FName("Jungle_Ambient");
        Data.SkyColor = FLinearColor(0.3f, 0.5f, 0.2f, 1.0f);
        break;
    case EBiomeType::Desert:
        Data.FogDensity = 0.005f;
        Data.WindStrength = 0.8f;
        Data.AmbientSoundTag = FName("Desert_Ambient");
        Data.SkyColor = FLinearColor(0.9f, 0.7f, 0.3f, 1.0f);
        break;
    case EBiomeType::Volcanic:
        Data.FogDensity = 0.08f;
        Data.WindStrength = 0.6f;
        Data.AmbientSoundTag = FName("Volcanic_Ambient");
        Data.SkyColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
        break;
    case EBiomeType::Swamp:
        Data.FogDensity = 0.06f;
        Data.WindStrength = 0.1f;
        Data.AmbientSoundTag = FName("Swamp_Ambient");
        Data.SkyColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f);
        break;
    case EBiomeType::Coastal:
        Data.FogDensity = 0.015f;
        Data.WindStrength = 1.0f;
        Data.AmbientSoundTag = FName("Coastal_Ambient");
        Data.SkyColor = FLinearColor(0.5f, 0.7f, 0.9f, 1.0f);
        break;
    case EBiomeType::Mountain:
        Data.FogDensity = 0.025f;
        Data.WindStrength = 1.2f;
        Data.AmbientSoundTag = FName("Mountain_Ambient");
        Data.SkyColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
        break;
    case EBiomeType::Grassland:
        Data.FogDensity = 0.01f;
        Data.WindStrength = 0.6f;
        Data.AmbientSoundTag = FName("Grassland_Ambient");
        Data.SkyColor = FLinearColor(0.6f, 0.8f, 0.4f, 1.0f);
        break;
    case EBiomeType::River:
        Data.FogDensity = 0.03f;
        Data.WindStrength = 0.3f;
        Data.AmbientSoundTag = FName("River_Ambient");
        Data.SkyColor = FLinearColor(0.4f, 0.6f, 0.7f, 1.0f);
        break;
    case EBiomeType::Cave:
        Data.FogDensity = 0.0f;
        Data.WindStrength = 0.0f;
        Data.AmbientSoundTag = FName("Cave_Ambient");
        Data.SkyColor = FLinearColor(0.05f, 0.05f, 0.1f, 1.0f);
        break;
    default:
        Data.FogDensity = 0.02f;
        Data.WindStrength = 0.4f;
        Data.AmbientSoundTag = FName("Default_Ambient");
        Data.SkyColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
        break;
    }

    return Data;
}

void UBiomeManager::TransitionToBiome(EBiomeType NewBiome, float Duration)
{
    if (NewBiome == CurrentBiome) return;

    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    TransitionDuration = FMath::Max(0.1f, Duration);
    BiomeTransitionAlpha = 0.0f;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transitioning from %d to %d over %.1f seconds"),
        (int32)PreviousBiome, (int32)NewBiome, TransitionDuration);
}

void UBiomeManager::UpdateTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    BiomeTransitionAlpha += DeltaTime / TransitionDuration;

    if (BiomeTransitionAlpha >= 1.0f)
    {
        BiomeTransitionAlpha = 1.0f;
        bIsTransitioning = false;
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Transition to biome %d complete"), (int32)CurrentBiome);
    }
}

float UBiomeManager::GetCurrentTemperature() const
{
    if (!BiomeTemperatures.Contains(CurrentBiome)) return 25.0f;

    FVector2D TempRange = BiomeTemperatures[CurrentBiome];
    float BaseTemp = (TempRange.X + TempRange.Y) * 0.5f;

    // Blend with previous biome during transition
    if (bIsTransitioning && BiomeTemperatures.Contains(PreviousBiome))
    {
        FVector2D PrevRange = BiomeTemperatures[PreviousBiome];
        float PrevTemp = (PrevRange.X + PrevRange.Y) * 0.5f;
        return FMath::Lerp(PrevTemp, BaseTemp, BiomeTransitionAlpha);
    }

    return BaseTemp;
}

float UBiomeManager::GetCurrentHumidity() const
{
    if (!BiomeHumidity.Contains(CurrentBiome)) return 0.5f;

    FVector2D HumRange = BiomeHumidity[CurrentBiome];
    float BaseHum = (HumRange.X + HumRange.Y) * 0.5f;

    if (bIsTransitioning && BiomeHumidity.Contains(PreviousBiome))
    {
        FVector2D PrevRange = BiomeHumidity[PreviousBiome];
        float PrevHum = (PrevRange.X + PrevRange.Y) * 0.5f;
        return FMath::Lerp(PrevHum, BaseHum, BiomeTransitionAlpha);
    }

    return BaseHum;
}

int32 UBiomeManager::GetCurrentDangerLevel() const
{
    if (!BiomeDangerLevel.Contains(CurrentBiome)) return 3;
    return BiomeDangerLevel[CurrentBiome];
}

FString UBiomeManager::GetBiomeName(EBiomeType Biome) const
{
    switch (Biome)
    {
    case EBiomeType::Forest:    return TEXT("Forest");
    case EBiomeType::Jungle:    return TEXT("Jungle");
    case EBiomeType::Swamp:     return TEXT("Swamp");
    case EBiomeType::Desert:    return TEXT("Desert");
    case EBiomeType::Volcanic:  return TEXT("Volcanic");
    case EBiomeType::Coastal:   return TEXT("Coastal");
    case EBiomeType::Grassland: return TEXT("Grassland");
    case EBiomeType::Mountain:  return TEXT("Mountain");
    case EBiomeType::River:     return TEXT("River");
    case EBiomeType::Cave:      return TEXT("Cave");
    default:                    return TEXT("Unknown");
    }
}

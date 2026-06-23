// BiomeManager.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260623_001
// Full implementation of the biome query, registration, and weather Markov chain.

#include "BiomeManager.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Check player biome every second

    // Seed default biome regions so the map is never empty
    // These are overwritten at BeginPlay by PCGWorldGenerator if present
    FEng_BiomeRuntimeData Grassland;
    Grassland.BiomeType = EBiomeType::Grassland;
    Grassland.MoistureLevel = 0.6f;
    Grassland.AmbientTemperature = 26.0f;
    Grassland.VegetationDensity = 0.75f;
    Grassland.BiomeBounds = FBox(FVector(-50000, -50000, -500), FVector(50000, 50000, 5000));
    Grassland.AllowedSpecies = { EDinosaurSpecies::Triceratops, EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Stegosaurus };
    BiomeRegions.Add(Grassland);
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: BeginPlay — %d biome regions registered"), BiomeRegions.Num());
    UpdatePlayerBiome();
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update player biome every tick (TickInterval=1s handles throttling)
    UpdatePlayerBiome();

    // Weather Markov chain
    WeatherTickAccumulator += DeltaTime;
    if (WeatherTickAccumulator >= WeatherTickInterval)
    {
        WeatherTickAccumulator = 0.0f;
        TickWeatherTransition();
    }

    // Debug draw
    if (bDrawBiomeBounds)
    {
        for (const FEng_BiomeRuntimeData& Region : BiomeRegions)
        {
            DrawDebugBox(GetWorld(), Region.BiomeBounds.GetCenter(),
                Region.BiomeBounds.GetExtent(), FColor::Green, false, 1.1f, 0, 5.0f);
        }
    }
}

// ─── Biome Query API ─────────────────────────────────────────────────────────

EBiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    // Iterate regions — first match wins (most specific region should be registered last)
    for (int32 i = BiomeRegions.Num() - 1; i >= 0; --i)
    {
        if (BiomeRegions[i].BiomeBounds.IsValid && BiomeRegions[i].BiomeBounds.IsInsideOrOn(WorldLocation))
        {
            return BiomeRegions[i].BiomeType;
        }
    }
    return EBiomeType::Grassland; // fallback
}

FEng_BiomeRuntimeData ABiomeManager::GetBiomeDataAtLocation(const FVector& WorldLocation) const
{
    for (int32 i = BiomeRegions.Num() - 1; i >= 0; --i)
    {
        if (BiomeRegions[i].BiomeBounds.IsValid && BiomeRegions[i].BiomeBounds.IsInsideOrOn(WorldLocation))
        {
            return BiomeRegions[i];
        }
    }
    // Return default grassland data
    FEng_BiomeRuntimeData Default;
    Default.BiomeType = EBiomeType::Grassland;
    Default.CurrentWeather = ActiveWeather;
    return Default;
}

EBiomeType ABiomeManager::GetPlayerCurrentBiome() const
{
    return CurrentPlayerBiome;
}

EEng_WeatherState ABiomeManager::GetCurrentWeather() const
{
    return ActiveWeather;
}

float ABiomeManager::GetTemperatureAtLocation(const FVector& WorldLocation) const
{
    FEng_BiomeRuntimeData Data = GetBiomeDataAtLocation(WorldLocation);
    float Temp = Data.AmbientTemperature;

    // Weather modifiers
    switch (ActiveWeather)
    {
    case EEng_WeatherState::Rain:   Temp -= 5.0f;  break;
    case EEng_WeatherState::Storm:  Temp -= 8.0f;  break;
    case EEng_WeatherState::Fog:    Temp -= 3.0f;  break;
    case EEng_WeatherState::Drought: Temp += 10.0f; break;
    default: break;
    }

    // Altitude modifier: -6.5°C per 1000m (lapse rate)
    const float AltitudeKm = FMath::Max(0.0f, WorldLocation.Z / 100000.0f);
    Temp -= AltitudeKm * 6.5f;

    return Temp;
}

// ─── Biome Registration ───────────────────────────────────────────────────────

void ABiomeManager::RegisterBiomeRegion(EBiomeType BiomeType, const FBox& Bounds,
    float VegetationDensity, float MoistureLevel)
{
    FEng_BiomeRuntimeData NewRegion;
    NewRegion.BiomeType = BiomeType;
    NewRegion.BiomeBounds = Bounds;
    NewRegion.VegetationDensity = FMath::Clamp(VegetationDensity, 0.0f, 1.0f);
    NewRegion.MoistureLevel = FMath::Clamp(MoistureLevel, 0.0f, 1.0f);
    NewRegion.AmbientTemperature = GetBaseTemperatureForBiome(BiomeType);
    NewRegion.CurrentWeather = ActiveWeather;

    // Default allowed species per biome
    switch (BiomeType)
    {
    case EBiomeType::Grassland:
        NewRegion.AllowedSpecies = { EDinosaurSpecies::Triceratops, EDinosaurSpecies::Brachiosaurus, EDinosaurSpecies::Stegosaurus };
        break;
    case EBiomeType::Forest:
        NewRegion.AllowedSpecies = { EDinosaurSpecies::Raptor, EDinosaurSpecies::Stegosaurus };
        break;
    case EBiomeType::Desert:
        NewRegion.AllowedSpecies = { EDinosaurSpecies::TRex };
        break;
    case EBiomeType::Swamp:
        NewRegion.AllowedSpecies = { EDinosaurSpecies::Brachiosaurus };
        break;
    default:
        NewRegion.AllowedSpecies = { EDinosaurSpecies::Raptor };
        break;
    }

    BiomeRegions.Add(NewRegion);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome %d at bounds center %s"),
        (int32)BiomeType, *Bounds.GetCenter().ToString());
}

void ABiomeManager::ClearAllBiomeRegions()
{
    BiomeRegions.Empty();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: All biome regions cleared"));
}

// ─── Weather Control ──────────────────────────────────────────────────────────

void ABiomeManager::ForceWeatherState(EEng_WeatherState NewWeather)
{
    if (NewWeather != ActiveWeather)
    {
        EEng_WeatherState Old = ActiveWeather;
        ActiveWeather = NewWeather;
        OnWeatherChanged.Broadcast(Old, NewWeather);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather forced to %d"), (int32)NewWeather);
    }
}

// ─── Debug ────────────────────────────────────────────────────────────────────

void ABiomeManager::PrintBiomeReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME MANAGER REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total regions: %d"), BiomeRegions.Num());
    UE_LOG(LogTemp, Warning, TEXT("Player biome: %d"), (int32)CurrentPlayerBiome);
    UE_LOG(LogTemp, Warning, TEXT("Active weather: %d"), (int32)ActiveWeather);
    for (const FEng_BiomeRuntimeData& R : BiomeRegions)
    {
        UE_LOG(LogTemp, Warning, TEXT("  Biome %d | Temp=%.1f°C | Moisture=%.2f | Veg=%.2f | Species=%d"),
            (int32)R.BiomeType, R.AmbientTemperature, R.MoistureLevel,
            R.VegetationDensity, R.AllowedSpecies.Num());
    }
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void ABiomeManager::UpdatePlayerBiome()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* Pawn = PC->GetPawn();
    if (!Pawn) return;

    EBiomeType NewBiome = GetBiomeAtLocation(Pawn->GetActorLocation());
    if (NewBiome != CurrentPlayerBiome)
    {
        EBiomeType OldBiome = CurrentPlayerBiome;
        CurrentPlayerBiome = NewBiome;
        OnBiomeChanged.Broadcast(OldBiome, NewBiome);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Player entered biome %d"), (int32)NewBiome);
    }
}

void ABiomeManager::TickWeatherTransition()
{
    EEng_WeatherState NewWeather = AdvanceWeatherMarkov(ActiveWeather, CurrentPlayerBiome);
    if (NewWeather != ActiveWeather)
    {
        EEng_WeatherState Old = ActiveWeather;
        ActiveWeather = NewWeather;
        OnWeatherChanged.Broadcast(Old, NewWeather);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather transitioned %d -> %d"), (int32)Old, (int32)NewWeather);
    }
}

EEng_WeatherState ABiomeManager::AdvanceWeatherMarkov(EEng_WeatherState Current, EBiomeType Biome) const
{
    // Simple Markov chain — transition probabilities differ per biome
    const float Roll = FMath::FRand();

    switch (Current)
    {
    case EEng_WeatherState::Clear:
        // Swamp/Forest more likely to go overcast; Desert stays clear
        if (Biome == EBiomeType::Swamp || Biome == EBiomeType::Forest)
            return Roll < 0.30f ? EEng_WeatherState::Overcast : EEng_WeatherState::Clear;
        if (Biome == EBiomeType::Desert)
            return Roll < 0.05f ? EEng_WeatherState::Drought : EEng_WeatherState::Clear;
        return Roll < 0.20f ? EEng_WeatherState::Overcast : EEng_WeatherState::Clear;

    case EEng_WeatherState::Overcast:
        if (Roll < 0.40f) return EEng_WeatherState::Rain;
        if (Roll < 0.55f) return EEng_WeatherState::Clear;
        return EEng_WeatherState::Overcast;

    case EEng_WeatherState::Rain:
        if (Roll < 0.20f) return EEng_WeatherState::Storm;
        if (Roll < 0.45f) return EEng_WeatherState::Overcast;
        return EEng_WeatherState::Rain;

    case EEng_WeatherState::Storm:
        if (Roll < 0.50f) return EEng_WeatherState::Rain;
        if (Roll < 0.70f) return EEng_WeatherState::Overcast;
        return EEng_WeatherState::Storm;

    case EEng_WeatherState::Fog:
        if (Roll < 0.60f) return EEng_WeatherState::Clear;
        return EEng_WeatherState::Fog;

    case EEng_WeatherState::Drought:
        if (Roll < 0.15f) return EEng_WeatherState::Clear;
        return EEng_WeatherState::Drought;

    default:
        return EEng_WeatherState::Clear;
    }
}

float ABiomeManager::GetBaseTemperatureForBiome(EBiomeType Biome) const
{
    switch (Biome)
    {
    case EBiomeType::Grassland:  return 26.0f;
    case EBiomeType::Forest:     return 22.0f;
    case EBiomeType::Desert:     return 42.0f;
    case EBiomeType::Swamp:      return 30.0f;
    case EBiomeType::Tundra:     return  2.0f;
    case EBiomeType::Volcanic:   return 55.0f;
    case EBiomeType::Coastal:    return 24.0f;
    default:                     return 25.0f;
    }
}

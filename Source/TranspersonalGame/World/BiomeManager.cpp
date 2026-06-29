// BiomeManager.cpp
// Agent #5 — Procedural World Generator
// Implementation of biome management, weather system, and day/night cycle

#include "BiomeManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// Constructor
// ============================================================

ABiomeManager::ABiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 Hz — sufficient for weather/day-night

    // Initialize default biome
    DefaultBiome.BiomeType = EWorld_BiomeType::Plains;
    DefaultBiome.BiomeName = TEXT("Open Plains");
    DefaultBiome.AmbientTemperature = 28.0f;
    DefaultBiome.Humidity = 0.4f;
    DefaultBiome.VegetationDensity = 0.35f;
    DefaultBiome.PredatorSpawnWeight = 0.25f;
    DefaultBiome.HerbivoreSpawnWeight = 0.65f;
    DefaultBiome.FogDensity = 0.01f;
    DefaultBiome.SkyTint = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    DefaultBiome.BiomeRadius = 0.0f; // Infinite — used as fallback
    DefaultBiome.BiomeCenter = FVector::ZeroVector;

    CurrentTimeOfDay = 8.0f; // Start at 8 AM
    CurrentWeather = EWorld_WeatherState::Clear;
    TargetWeather = EWorld_WeatherState::Clear;
    WeatherBlendAlpha = 1.0f;
    bWeatherTransitioning = false;
}

// ============================================================
// BeginPlay
// ============================================================

void ABiomeManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultBiomes();
    ApplySunRotation(CurrentTimeOfDay);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Initialized with %d biome zones. Time: %.1f:00"), RegisteredBiomes.Num(), CurrentTimeOfDay);
}

// ============================================================
// Tick
// ============================================================

void ABiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bDayNightCycleActive)
    {
        UpdateDayNightCycle(DeltaTime);
    }

    if (bWeatherTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

// ============================================================
// Biome Query
// ============================================================

EWorld_BiomeType ABiomeManager::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    return GetBiomePropertiesAtLocation(WorldLocation).BiomeType;
}

FWorld_BiomeProperties ABiomeManager::GetBiomePropertiesAtLocation(const FVector& WorldLocation) const
{
    float ClosestDistSq = MAX_FLT;
    const FWorld_BiomeProperties* ClosestBiome = nullptr;

    for (const FWorld_BiomeProperties& Biome : RegisteredBiomes)
    {
        if (Biome.BiomeRadius <= 0.0f)
        {
            continue; // Skip infinite biomes in search
        }

        const float DistSq = FVector::DistSquaredXY(WorldLocation, Biome.BiomeCenter);
        const float RadiusSq = Biome.BiomeRadius * Biome.BiomeRadius;

        if (DistSq <= RadiusSq && DistSq < ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            ClosestBiome = &Biome;
        }
    }

    return ClosestBiome ? *ClosestBiome : DefaultBiome;
}

EWorld_WeatherState ABiomeManager::GetCurrentWeather() const
{
    return CurrentWeather;
}

FWorld_WeatherProperties ABiomeManager::GetCurrentWeatherProperties() const
{
    return CurrentWeatherProps;
}

// ============================================================
// Biome Registration
// ============================================================

void ABiomeManager::RegisterBiomeZone(const FWorld_BiomeProperties& BiomeProps)
{
    RegisteredBiomes.Add(BiomeProps);
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Registered biome '%s' at (%.0f, %.0f) radius %.0f"),
        *BiomeProps.BiomeName, BiomeProps.BiomeCenter.X, BiomeProps.BiomeCenter.Y, BiomeProps.BiomeRadius);
}

void ABiomeManager::ClearAllBiomes()
{
    RegisteredBiomes.Empty();
    UE_LOG(LogTemp, Log, TEXT("BiomeManager: All biome zones cleared."));
}

// ============================================================
// Weather Control
// ============================================================

void ABiomeManager::TriggerWeatherTransition(EWorld_WeatherState NewWeather, float TransitionDuration)
{
    if (NewWeather == CurrentWeather && !bWeatherTransitioning)
    {
        return;
    }

    TargetWeather = NewWeather;
    WeatherTransitionDuration = FMath::Max(TransitionDuration, 1.0f);
    WeatherTransitionElapsed = 0.0f;
    WeatherBlendAlpha = 0.0f;
    bWeatherTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather transition started -> %d over %.1fs"),
        (int32)NewWeather, TransitionDuration);
}

void ABiomeManager::SetWeatherImmediate(EWorld_WeatherState NewWeather)
{
    CurrentWeather = NewWeather;
    TargetWeather = NewWeather;
    WeatherBlendAlpha = 1.0f;
    bWeatherTransitioning = false;

    // Update weather properties immediately
    CurrentWeatherProps.WeatherState = NewWeather;
    switch (NewWeather)
    {
    case EWorld_WeatherState::Clear:
        CurrentWeatherProps.RainIntensity = 0.0f;
        CurrentWeatherProps.WindSpeed = 3.0f;
        CurrentWeatherProps.LightningFrequency = 0.0f;
        break;
    case EWorld_WeatherState::Overcast:
        CurrentWeatherProps.RainIntensity = 0.0f;
        CurrentWeatherProps.WindSpeed = 8.0f;
        CurrentWeatherProps.LightningFrequency = 0.0f;
        break;
    case EWorld_WeatherState::Rain:
        CurrentWeatherProps.RainIntensity = 0.6f;
        CurrentWeatherProps.WindSpeed = 15.0f;
        CurrentWeatherProps.LightningFrequency = 0.0f;
        break;
    case EWorld_WeatherState::Storm:
        CurrentWeatherProps.RainIntensity = 1.0f;
        CurrentWeatherProps.WindSpeed = 35.0f;
        CurrentWeatherProps.LightningFrequency = 0.3f;
        break;
    case EWorld_WeatherState::Fog:
        CurrentWeatherProps.RainIntensity = 0.1f;
        CurrentWeatherProps.WindSpeed = 2.0f;
        CurrentWeatherProps.LightningFrequency = 0.0f;
        break;
    default:
        break;
    }
}

// ============================================================
// Day/Night Cycle
// ============================================================

float ABiomeManager::GetTimeOfDay() const
{
    return CurrentTimeOfDay;
}

void ABiomeManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Fmod(FMath::Abs(Hours), 24.0f);
    ApplySunRotation(CurrentTimeOfDay);
}

// ============================================================
// Private — Update Methods
// ============================================================

void ABiomeManager::UpdateDayNightCycle(float DeltaTime)
{
    // Advance time — DayNightSpeedMultiplier = 60 means 1 real second = 60 game seconds
    const float HoursPerSecond = DayNightSpeedMultiplier / 3600.0f;
    CurrentTimeOfDay += DeltaTime * HoursPerSecond;

    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Apply sun rotation every tick (at 10Hz this is fine)
    ApplySunRotation(CurrentTimeOfDay);
}

void ABiomeManager::UpdateWeatherTransition(float DeltaTime)
{
    WeatherTransitionElapsed += DeltaTime;
    WeatherBlendAlpha = FMath::Clamp(WeatherTransitionElapsed / WeatherTransitionDuration, 0.0f, 1.0f);

    if (WeatherBlendAlpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        bWeatherTransitioning = false;
        SetWeatherImmediate(CurrentWeather);
        UE_LOG(LogTemp, Log, TEXT("BiomeManager: Weather transition complete -> %d"), (int32)CurrentWeather);
    }
}

void ABiomeManager::ApplySunRotation(float TimeOfDay)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);

    if (DirectionalLights.Num() == 0)
    {
        return;
    }

    ADirectionalLight* Sun = Cast<ADirectionalLight>(DirectionalLights[0]);
    if (!Sun)
    {
        return;
    }

    // Convert time of day to sun pitch angle
    // 6:00 = sunrise (pitch = 0), 12:00 = noon (pitch = -90), 18:00 = sunset (pitch = 0), 0:00 = midnight (pitch = 90 below horizon)
    const float NormalizedTime = (TimeOfDay - 6.0f) / 24.0f; // 0 at 6AM
    const float SunAngleDeg = NormalizedTime * 360.0f;
    const float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngleDeg)) * -90.0f;

    // Enforce CAP: never let sun go above -30° pitch during daytime
    const float ClampedPitch = FMath::Min(SunPitch, -30.0f);

    FRotator CurrentRot = Sun->GetActorRotation();
    Sun->SetActorRotation(FRotator(ClampedPitch, CurrentRot.Yaw, CurrentRot.Roll));
}

void ABiomeManager::InitializeDefaultBiomes()
{
    // Only initialize if no biomes are already registered (e.g., set in editor)
    if (RegisteredBiomes.Num() > 0)
    {
        return;
    }

    // River Valley — center of the map
    FWorld_BiomeProperties River = GetDefaultPropertiesForBiome(EWorld_BiomeType::River);
    River.BiomeCenter = FVector(0.0f, 0.0f, -200.0f);
    River.BiomeRadius = 200000.0f; // 2km radius
    RegisteredBiomes.Add(River);

    // Dense Forest — north
    FWorld_BiomeProperties Forest = GetDefaultPropertiesForBiome(EWorld_BiomeType::Forest);
    Forest.BiomeCenter = FVector(300000.0f, 300000.0f, 0.0f);
    Forest.BiomeRadius = 350000.0f;
    RegisteredBiomes.Add(Forest);

    // Open Plains — east
    FWorld_BiomeProperties Plains = GetDefaultPropertiesForBiome(EWorld_BiomeType::Plains);
    Plains.BiomeCenter = FVector(500000.0f, 0.0f, 0.0f);
    Plains.BiomeRadius = 400000.0f;
    RegisteredBiomes.Add(Plains);

    // Rocky Highlands — south
    FWorld_BiomeProperties Rocky = GetDefaultPropertiesForBiome(EWorld_BiomeType::Rocky);
    Rocky.BiomeCenter = FVector(0.0f, -400000.0f, 20000.0f);
    Rocky.BiomeRadius = 300000.0f;
    RegisteredBiomes.Add(Rocky);

    // Volcanic Region — west
    FWorld_BiomeProperties Volcanic = GetDefaultPropertiesForBiome(EWorld_BiomeType::Volcanic);
    Volcanic.BiomeCenter = FVector(-500000.0f, 0.0f, 50000.0f);
    Volcanic.BiomeRadius = 350000.0f;
    RegisteredBiomes.Add(Volcanic);

    UE_LOG(LogTemp, Log, TEXT("BiomeManager: Default biomes initialized (%d zones)."), RegisteredBiomes.Num());
}

FWorld_BiomeProperties ABiomeManager::GetDefaultPropertiesForBiome(EWorld_BiomeType BiomeType) const
{
    FWorld_BiomeProperties Props;
    Props.BiomeType = BiomeType;

    switch (BiomeType)
    {
    case EWorld_BiomeType::River:
        Props.BiomeName = TEXT("River Valley");
        Props.AmbientTemperature = 24.0f;
        Props.Humidity = 0.85f;
        Props.VegetationDensity = 0.7f;
        Props.PredatorSpawnWeight = 0.2f;
        Props.HerbivoreSpawnWeight = 0.75f;
        Props.FogDensity = 0.04f;
        Props.SkyTint = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        break;

    case EWorld_BiomeType::Forest:
        Props.BiomeName = TEXT("Dense Jungle");
        Props.AmbientTemperature = 30.0f;
        Props.Humidity = 0.9f;
        Props.VegetationDensity = 0.95f;
        Props.PredatorSpawnWeight = 0.45f;
        Props.HerbivoreSpawnWeight = 0.5f;
        Props.FogDensity = 0.06f;
        Props.SkyTint = FLinearColor(0.7f, 1.0f, 0.7f, 1.0f);
        break;

    case EWorld_BiomeType::Plains:
        Props.BiomeName = TEXT("Open Plains");
        Props.AmbientTemperature = 32.0f;
        Props.Humidity = 0.3f;
        Props.VegetationDensity = 0.3f;
        Props.PredatorSpawnWeight = 0.35f;
        Props.HerbivoreSpawnWeight = 0.6f;
        Props.FogDensity = 0.005f;
        Props.SkyTint = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        break;

    case EWorld_BiomeType::Rocky:
        Props.BiomeName = TEXT("Rocky Highlands");
        Props.AmbientTemperature = 18.0f;
        Props.Humidity = 0.2f;
        Props.VegetationDensity = 0.15f;
        Props.PredatorSpawnWeight = 0.5f;
        Props.HerbivoreSpawnWeight = 0.35f;
        Props.FogDensity = 0.02f;
        Props.SkyTint = FLinearColor(0.9f, 0.85f, 0.75f, 1.0f);
        break;

    case EWorld_BiomeType::Volcanic:
        Props.BiomeName = TEXT("Volcanic Region");
        Props.AmbientTemperature = 55.0f;
        Props.Humidity = 0.1f;
        Props.VegetationDensity = 0.05f;
        Props.PredatorSpawnWeight = 0.6f;
        Props.HerbivoreSpawnWeight = 0.15f;
        Props.FogDensity = 0.08f;
        Props.SkyTint = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
        break;

    case EWorld_BiomeType::Swamp:
        Props.BiomeName = TEXT("Swampland");
        Props.AmbientTemperature = 27.0f;
        Props.Humidity = 0.95f;
        Props.VegetationDensity = 0.8f;
        Props.PredatorSpawnWeight = 0.55f;
        Props.HerbivoreSpawnWeight = 0.4f;
        Props.FogDensity = 0.1f;
        Props.SkyTint = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);
        break;

    default:
        Props.BiomeName = TEXT("Unknown");
        break;
    }

    return Props;
}

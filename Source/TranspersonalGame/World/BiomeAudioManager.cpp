// BiomeAudioManager.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_004
// Implementation of biome-aware environmental audio management.

#include "BiomeAudioManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ABiomeAudioManager::ABiomeAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check biome every 0.5s for performance

    InitDefaultAudioLayers();
}

void ABiomeAudioManager::BeginPlay()
{
    Super::BeginPlay();

    // Detect initial biome
    CurrentBiome = DetectPlayerBiome();
    PreviousBiome = CurrentBiome;
    WeatherTimer = 0.0f;
    TransitionAlpha = 1.0f;
}

void ABiomeAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateBiomeDetection();
    UpdateWeatherCycle(DeltaTime);
}

EWorld_BiomeType ABiomeAudioManager::DetectPlayerBiome() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return EWorld_BiomeType::Unknown;
    }

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC)
    {
        return EWorld_BiomeType::Unknown;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        return EWorld_BiomeType::Unknown;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Simple zone detection based on world position quadrants
    // Matches the biome zone light positions set in UE5 Python:
    // Forest:    (-3000, -3000)
    // Plains:    ( 3000, -3000)
    // Rocky:     (-3000,  3000)
    // Riverbank: ( 3000,  3000)

    const float ZoneRadius = 3000.0f;

    if (PlayerLocation.X < 0.0f && PlayerLocation.Y < 0.0f)
    {
        if (FVector::Dist2D(PlayerLocation, FVector(-3000.0f, -3000.0f, 0.0f)) < ZoneRadius)
        {
            return EWorld_BiomeType::Forest;
        }
    }
    else if (PlayerLocation.X > 0.0f && PlayerLocation.Y < 0.0f)
    {
        if (FVector::Dist2D(PlayerLocation, FVector(3000.0f, -3000.0f, 0.0f)) < ZoneRadius)
        {
            return EWorld_BiomeType::Plains;
        }
    }
    else if (PlayerLocation.X < 0.0f && PlayerLocation.Y > 0.0f)
    {
        if (FVector::Dist2D(PlayerLocation, FVector(-3000.0f, 3000.0f, 0.0f)) < ZoneRadius)
        {
            return EWorld_BiomeType::Rocky;
        }
    }
    else if (PlayerLocation.X > 0.0f && PlayerLocation.Y > 0.0f)
    {
        if (FVector::Dist2D(PlayerLocation, FVector(3000.0f, 3000.0f, 0.0f)) < ZoneRadius)
        {
            return EWorld_BiomeType::Riverbank;
        }
    }

    // Default to Plains if not in a specific zone
    return EWorld_BiomeType::Plains;
}

void ABiomeAudioManager::SetWeatherState(EWorld_WeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        TransitionAlpha = 0.0f; // Reset transition
    }
}

FWorld_BiomeAudioLayer ABiomeAudioManager::GetActiveAudioLayer() const
{
    for (const FWorld_BiomeAudioLayer& Layer : BiomeAudioLayers)
    {
        if (Layer.BiomeType == CurrentBiome && Layer.WeatherState == CurrentWeather)
        {
            return Layer;
        }
    }

    // Fallback: return first layer matching biome regardless of weather
    for (const FWorld_BiomeAudioLayer& Layer : BiomeAudioLayers)
    {
        if (Layer.BiomeType == CurrentBiome)
        {
            return Layer;
        }
    }

    // Final fallback: return default layer
    FWorld_BiomeAudioLayer Default;
    Default.BiomeType = EWorld_BiomeType::Plains;
    Default.WeatherState = EWorld_WeatherState::Clear;
    Default.AmbientVolume = 0.5f;
    return Default;
}

bool ABiomeAudioManager::HasAudioLayerForCurrentState() const
{
    for (const FWorld_BiomeAudioLayer& Layer : BiomeAudioLayers)
    {
        if (Layer.BiomeType == CurrentBiome && Layer.WeatherState == CurrentWeather)
        {
            return true;
        }
    }
    return false;
}

void ABiomeAudioManager::InitDefaultAudioLayers()
{
    BiomeAudioLayers.Empty();

    // Forest — Clear
    FWorld_BiomeAudioLayer ForestClear;
    ForestClear.BiomeType = EWorld_BiomeType::Forest;
    ForestClear.WeatherState = EWorld_WeatherState::Clear;
    ForestClear.AmbientVolume = 0.9f;
    ForestClear.WindIntensity = 0.3f;
    ForestClear.bPlayInsectChorus = true;
    ForestClear.bPlayBirdCalls = true;
    BiomeAudioLayers.Add(ForestClear);

    // Forest — Rain
    FWorld_BiomeAudioLayer ForestRain;
    ForestRain.BiomeType = EWorld_BiomeType::Forest;
    ForestRain.WeatherState = EWorld_WeatherState::Rain;
    ForestRain.AmbientVolume = 0.7f;
    ForestRain.WindIntensity = 0.6f;
    ForestRain.RainIntensity = 0.8f;
    ForestRain.bPlayInsectChorus = false;
    ForestRain.bPlayBirdCalls = false;
    BiomeAudioLayers.Add(ForestRain);

    // Plains — Clear
    FWorld_BiomeAudioLayer PlainsClear;
    PlainsClear.BiomeType = EWorld_BiomeType::Plains;
    PlainsClear.WeatherState = EWorld_WeatherState::Clear;
    PlainsClear.AmbientVolume = 0.6f;
    PlainsClear.WindIntensity = 0.7f;
    PlainsClear.bPlayBirdCalls = true;
    BiomeAudioLayers.Add(PlainsClear);

    // Plains — Storm
    FWorld_BiomeAudioLayer PlainsStorm;
    PlainsStorm.BiomeType = EWorld_BiomeType::Plains;
    PlainsStorm.WeatherState = EWorld_WeatherState::Storm;
    PlainsStorm.AmbientVolume = 0.4f;
    PlainsStorm.WindIntensity = 1.0f;
    PlainsStorm.RainIntensity = 1.0f;
    BiomeAudioLayers.Add(PlainsStorm);

    // Rocky — Clear
    FWorld_BiomeAudioLayer RockyClear;
    RockyClear.BiomeType = EWorld_BiomeType::Rocky;
    RockyClear.WeatherState = EWorld_WeatherState::Clear;
    RockyClear.AmbientVolume = 0.4f;
    RockyClear.WindIntensity = 0.8f;
    BiomeAudioLayers.Add(RockyClear);

    // Riverbank — Clear
    FWorld_BiomeAudioLayer RiverbankClear;
    RiverbankClear.BiomeType = EWorld_BiomeType::Riverbank;
    RiverbankClear.WeatherState = EWorld_WeatherState::Clear;
    RiverbankClear.AmbientVolume = 0.8f;
    RiverbankClear.WindIntensity = 0.2f;
    RiverbankClear.bPlayWaterFlow = true;
    RiverbankClear.bPlayInsectChorus = true;
    BiomeAudioLayers.Add(RiverbankClear);

    // Riverbank — Fog
    FWorld_BiomeAudioLayer RiverbankFog;
    RiverbankFog.BiomeType = EWorld_BiomeType::Riverbank;
    RiverbankFog.WeatherState = EWorld_WeatherState::Fog;
    RiverbankFog.AmbientVolume = 0.5f;
    RiverbankFog.WindIntensity = 0.1f;
    RiverbankFog.bPlayWaterFlow = true;
    BiomeAudioLayers.Add(RiverbankFog);
}

void ABiomeAudioManager::UpdateWeatherCycle(float DeltaTime)
{
    WeatherTimer += DeltaTime;

    if (WeatherTimer >= WeatherChangePeriod)
    {
        WeatherTimer = 0.0f;

        // Cycle through weather states naturally
        EWorld_WeatherState NextWeather = EWorld_WeatherState::Clear;
        switch (CurrentWeather)
        {
            case EWorld_WeatherState::Clear:
                NextWeather = (FMath::RandBool()) ? EWorld_WeatherState::Overcast : EWorld_WeatherState::Clear;
                break;
            case EWorld_WeatherState::Overcast:
                NextWeather = (FMath::RandBool()) ? EWorld_WeatherState::Rain : EWorld_WeatherState::Clear;
                break;
            case EWorld_WeatherState::Rain:
                NextWeather = (FMath::RandRange(0, 2) == 0) ? EWorld_WeatherState::Storm : EWorld_WeatherState::Overcast;
                break;
            case EWorld_WeatherState::Storm:
                NextWeather = EWorld_WeatherState::Overcast;
                break;
            case EWorld_WeatherState::Fog:
                NextWeather = EWorld_WeatherState::Clear;
                break;
            default:
                NextWeather = EWorld_WeatherState::Clear;
                break;
        }

        SetWeatherState(NextWeather);
    }

    // Update transition alpha
    if (TransitionAlpha < 1.0f)
    {
        TransitionAlpha = FMath::Min(1.0f, TransitionAlpha + DeltaTime / AudioTransitionSpeed);
    }
}

void ABiomeAudioManager::UpdateBiomeDetection()
{
    EWorld_BiomeType Detected = DetectPlayerBiome();
    if (Detected != CurrentBiome)
    {
        PreviousBiome = CurrentBiome;
        CurrentBiome = Detected;
        TransitionAlpha = 0.0f; // Start audio crossfade
    }
}

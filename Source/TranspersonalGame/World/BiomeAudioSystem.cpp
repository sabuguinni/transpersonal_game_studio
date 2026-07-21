// BiomeAudioSystem.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_004
// Full implementation of biome-driven environmental audio system

#include "BiomeAudioSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ABiomeAudioSystem::ABiomeAudioSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz — audio crossfade needs smooth update

    // Create dual-channel ambient system (A/B crossfade)
    AmbientChannelA = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientChannelA"));
    AmbientChannelA->SetupAttachment(RootComponent);
    AmbientChannelA->bAutoActivate = false;
    AmbientChannelA->VolumeMultiplier = 0.0f;

    AmbientChannelB = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientChannelB"));
    AmbientChannelB->SetupAttachment(RootComponent);
    AmbientChannelB->bAutoActivate = false;
    AmbientChannelB->VolumeMultiplier = 0.0f;

    // Weather overlay channel
    WeatherChannel = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherChannel"));
    WeatherChannel->SetupAttachment(RootComponent);
    WeatherChannel->bAutoActivate = false;
    WeatherChannel->VolumeMultiplier = 0.0f;

    // Default biome configs (no sound assets assigned — populated in editor)
    // Forest biome
    FWorld_BiomeAudioConfig ForestConfig;
    ForestConfig.BiomeType = EWorld_BiomeType::Forest;
    ForestConfig.VolumeMultiplier = 1.0f;
    ForestConfig.CrossfadeDuration = 4.0f;
    BiomeConfigs.Add(ForestConfig);

    // Plains biome
    FWorld_BiomeAudioConfig PlainsConfig;
    PlainsConfig.BiomeType = EWorld_BiomeType::Plains;
    PlainsConfig.VolumeMultiplier = 0.7f;
    PlainsConfig.CrossfadeDuration = 3.0f;
    BiomeConfigs.Add(PlainsConfig);

    // Rocky biome
    FWorld_BiomeAudioConfig RockyConfig;
    RockyConfig.BiomeType = EWorld_BiomeType::Rocky;
    RockyConfig.VolumeMultiplier = 0.5f;
    RockyConfig.CrossfadeDuration = 2.5f;
    BiomeConfigs.Add(RockyConfig);

    // River biome
    FWorld_BiomeAudioConfig RiverConfig;
    RiverConfig.BiomeType = EWorld_BiomeType::River;
    RiverConfig.VolumeMultiplier = 1.2f;
    RiverConfig.CrossfadeDuration = 3.5f;
    BiomeConfigs.Add(RiverConfig);
}

void ABiomeAudioSystem::BeginPlay()
{
    Super::BeginPlay();

    // Start both channels silent
    if (AmbientChannelA) AmbientChannelA->SetVolumeMultiplier(0.0f);
    if (AmbientChannelB) AmbientChannelB->SetVolumeMultiplier(0.0f);
    if (WeatherChannel)  WeatherChannel->SetVolumeMultiplier(0.0f);

    // Initial biome query
    BiomeQueryTimer = BiomeQueryInterval; // Force immediate query on first tick
}

void ABiomeAudioSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Biome query on interval
    TickBiomeQuery(DeltaTime);

    // Crossfade update
    if (bCrossfading && CrossfadeDuration > 0.0f)
    {
        CrossfadeTimer += DeltaTime;
        float Alpha = FMath::Clamp(CrossfadeTimer / CrossfadeDuration, 0.0f, 1.0f);

        // Smooth S-curve crossfade
        float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);

        if (bChannelAActive)
        {
            // A is new (fading in), B is old (fading out)
            if (AmbientChannelA) AmbientChannelA->SetVolumeMultiplier(SmoothAlpha);
            if (AmbientChannelB) AmbientChannelB->SetVolumeMultiplier(1.0f - SmoothAlpha);
        }
        else
        {
            // B is new (fading in), A is old (fading out)
            if (AmbientChannelB) AmbientChannelB->SetVolumeMultiplier(SmoothAlpha);
            if (AmbientChannelA) AmbientChannelA->SetVolumeMultiplier(1.0f - SmoothAlpha);
        }

        if (Alpha >= 1.0f)
        {
            bCrossfading = false;
            // Stop the old channel
            if (bChannelAActive && AmbientChannelB) AmbientChannelB->Stop();
            if (!bChannelAActive && AmbientChannelA) AmbientChannelA->Stop();
        }
    }
}

void ABiomeAudioSystem::TickBiomeQuery(float DeltaTime)
{
    BiomeQueryTimer += DeltaTime;
    if (BiomeQueryTimer < BiomeQueryInterval) return;
    BiomeQueryTimer = 0.0f;

    // Get player location
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
    EWorld_BiomeType DetectedBiome = QueryBiomeAtLocation(PlayerLoc);

    if (DetectedBiome != CurrentBiome)
    {
        SetBiome(DetectedBiome);
    }
}

EWorld_BiomeType ABiomeAudioSystem::QueryBiomeAtLocation(FVector WorldLocation) const
{
    // Simple distance-based biome detection using known biome zone positions
    // In production this would sample landscape layer weights via ALandscape API
    // For now: use zone proximity matching the BiomeZone_ actors in the map

    UWorld* World = GetWorld();
    if (!World) return EWorld_BiomeType::Unknown;

    // Check proximity to named biome zone actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    float ClosestDist = BiomeSampleRadius;
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::Plains; // Default to plains

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString Label = Actor->GetActorLabel();

        EWorld_BiomeType ZoneBiome = EWorld_BiomeType::Unknown;
        if (Label.Contains("Forest"))  ZoneBiome = EWorld_BiomeType::Forest;
        else if (Label.Contains("Plains")) ZoneBiome = EWorld_BiomeType::Plains;
        else if (Label.Contains("Rocky"))  ZoneBiome = EWorld_BiomeType::Rocky;
        else if (Label.Contains("River"))  ZoneBiome = EWorld_BiomeType::River;
        else if (Label.Contains("Swamp"))  ZoneBiome = EWorld_BiomeType::Swamp;
        else if (Label.Contains("Volcanic")) ZoneBiome = EWorld_BiomeType::Volcanic;

        if (ZoneBiome == EWorld_BiomeType::Unknown) continue;

        float Dist = FVector::Dist(WorldLocation, Actor->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            ClosestBiome = ZoneBiome;
        }
    }

    return ClosestBiome;
}

bool ABiomeAudioSystem::GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeAudioConfig& OutConfig) const
{
    for (const FWorld_BiomeAudioConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            OutConfig = Config;
            return true;
        }
    }
    return false;
}

void ABiomeAudioSystem::SetBiome(EWorld_BiomeType NewBiome)
{
    if (NewBiome == CurrentBiome) return;

    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;

    FWorld_BiomeAudioConfig Config;
    if (!GetBiomeConfig(NewBiome, Config)) return;

    CrossfadeToNewBiome(Config);
}

void ABiomeAudioSystem::CrossfadeToNewBiome(const FWorld_BiomeAudioConfig& Config)
{
    // Select which sound to play (night variant if available and it's night)
    USoundBase* SoundToPlay = nullptr;
    if (bIsNight && Config.NightLoop)
    {
        SoundToPlay = Config.NightLoop;
    }
    else
    {
        SoundToPlay = Config.AmbientLoop;
    }

    if (!SoundToPlay) return;

    // Toggle channels: A/B crossfade
    bChannelAActive = !bChannelAActive;
    CrossfadeDuration = Config.CrossfadeDuration;
    CrossfadeTimer = 0.0f;
    bCrossfading = true;

    UAudioComponent* NewChannel = bChannelAActive ? AmbientChannelA : AmbientChannelB;
    if (NewChannel)
    {
        NewChannel->SetSound(SoundToPlay);
        NewChannel->SetVolumeMultiplier(0.0f);
        NewChannel->Play();
    }
}

void ABiomeAudioSystem::SetWeather(EWorld_WeatherState NewWeather)
{
    if (NewWeather == CurrentWeather) return;
    CurrentWeather = NewWeather;
    UpdateWeatherAudio();
}

void ABiomeAudioSystem::UpdateWeatherAudio()
{
    if (!WeatherChannel) return;

    // Get rain sound from current biome config
    FWorld_BiomeAudioConfig Config;
    bool bHasConfig = GetBiomeConfig(CurrentBiome, Config);

    bool bShouldPlayRain = (CurrentWeather == EWorld_WeatherState::Rain || CurrentWeather == EWorld_WeatherState::Storm);

    if (bShouldPlayRain && bHasConfig && Config.RainOverlay)
    {
        float TargetVolume = (CurrentWeather == EWorld_WeatherState::Storm) ? 1.5f : 1.0f;
        WeatherChannel->SetSound(Config.RainOverlay);
        WeatherChannel->SetVolumeMultiplier(TargetVolume);
        if (!WeatherChannel->IsPlaying()) WeatherChannel->Play();
    }
    else
    {
        WeatherChannel->Stop();
    }
}

void ABiomeAudioSystem::SetNight(bool bNight)
{
    if (bNight == bIsNight) return;
    bIsNight = bNight;

    // Re-trigger current biome audio with night variant
    EWorld_BiomeType SavedBiome = CurrentBiome;
    CurrentBiome = EWorld_BiomeType::Unknown; // Force re-trigger
    SetBiome(SavedBiome);
}

#include "World_EnvironmentalAudio.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY_STATIC(LogEnvironmentalAudio, Log, All);

AWorld_EnvironmentalAudio::AWorld_EnvironmentalAudio()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create audio components for different environmental layers
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = false;
    
    WeatherAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
    WeatherAudioComponent->SetupAttachment(RootComponent);
    WeatherAudioComponent->bAutoActivate = false;
    
    BiomeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BiomeAudio"));
    BiomeAudioComponent->SetupAttachment(RootComponent);
    BiomeAudioComponent->bAutoActivate = false;
    
    WildlifeAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WildlifeAudio"));
    WildlifeAudioComponent->SetupAttachment(RootComponent);
    WildlifeAudioComponent->bAutoActivate = false;
    
    // Initialize default values
    CurrentBiome = EWorld_BiomeType::Savana;
    CurrentWeatherType = EWorld_WeatherType::Clear;
    CurrentTimeOfDay = EWorld_TimeOfDay::Day;
    
    AudioUpdateInterval = 5.0f;
    BiomeTransitionDuration = 3.0f;
    WeatherTransitionDuration = 2.0f;
    
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    
    // Default volume settings
    AmbientVolume = 0.7f;
    WeatherVolume = 0.8f;
    BiomeVolume = 0.6f;
    WildlifeVolume = 0.5f;
}

void AWorld_EnvironmentalAudio::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Environmental Audio System initialized"));
    
    // Start the audio update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            AudioUpdateTimerHandle,
            this,
            &AWorld_EnvironmentalAudio::UpdateEnvironmentalAudio,
            AudioUpdateInterval,
            true
        );
    }
    
    // Initialize with current biome audio
    SetBiomeAudio(CurrentBiome);
    SetWeatherAudio(CurrentWeatherType);
}

void AWorld_EnvironmentalAudio::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle audio transitions
    if (bIsTransitioning)
    {
        UpdateAudioTransition(DeltaTime);
    }
}

void AWorld_EnvironmentalAudio::SetBiomeAudio(EWorld_BiomeType NewBiome)
{
    if (CurrentBiome == NewBiome && !bIsTransitioning)
    {
        return;
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Transitioning to biome audio: %d"), (int32)NewBiome);
    
    PreviousBiome = CurrentBiome;
    CurrentBiome = NewBiome;
    
    StartBiomeTransition();
}

void AWorld_EnvironmentalAudio::SetWeatherAudio(EWorld_WeatherType NewWeather)
{
    if (CurrentWeatherType == NewWeather)
    {
        return;
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Setting weather audio: %d"), (int32)NewWeather);
    
    CurrentWeatherType = NewWeather;
    
    // Apply weather audio immediately
    ApplyWeatherAudio();
}

void AWorld_EnvironmentalAudio::SetTimeOfDayAudio(EWorld_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay == NewTimeOfDay)
    {
        return;
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Setting time of day audio: %d"), (int32)NewTimeOfDay);
    
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Adjust audio based on time of day
    ApplyTimeOfDayAudio();
}

void AWorld_EnvironmentalAudio::StartBiomeTransition()
{
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    
    // Start transition timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            BiomeTransitionTimerHandle,
            this,
            &AWorld_EnvironmentalAudio::CompleteBiomeTransition,
            BiomeTransitionDuration,
            false
        );
    }
}

void AWorld_EnvironmentalAudio::UpdateAudioTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionProgress += DeltaTime / BiomeTransitionDuration;
    TransitionProgress = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);
    
    // Smooth transition curve
    float SmoothProgress = FMath::SmoothStep(0.0f, 1.0f, TransitionProgress);
    
    // Fade out old biome audio and fade in new biome audio
    if (BiomeAudioComponent)
    {
        float NewVolume = BiomeVolume * SmoothProgress;
        BiomeAudioComponent->SetVolumeMultiplier(NewVolume);
    }
}

void AWorld_EnvironmentalAudio::CompleteBiomeTransition()
{
    bIsTransitioning = false;
    TransitionProgress = 1.0f;
    
    // Apply final biome audio
    ApplyBiomeAudio();
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Biome transition completed"));
}

void AWorld_EnvironmentalAudio::ApplyBiomeAudio()
{
    if (!BiomeAudioComponent)
    {
        return;
    }
    
    // Set biome-specific audio based on current biome
    switch (CurrentBiome)
    {
        case EWorld_BiomeType::Pantano:
            // Swamp sounds: water bubbling, insects, frogs
            BiomeAudioComponent->SetVolumeMultiplier(BiomeVolume * 0.8f);
            break;
            
        case EWorld_BiomeType::Floresta:
            // Forest sounds: wind through trees, bird calls, rustling leaves
            BiomeAudioComponent->SetVolumeMultiplier(BiomeVolume * 0.9f);
            break;
            
        case EWorld_BiomeType::Savana:
            // Savanna sounds: wind through grass, distant animal calls
            BiomeAudioComponent->SetVolumeMultiplier(BiomeVolume * 0.7f);
            break;
            
        case EWorld_BiomeType::Deserto:
            // Desert sounds: wind, sand movement, sparse wildlife
            BiomeAudioComponent->SetVolumeMultiplier(BiomeVolume * 0.5f);
            break;
            
        case EWorld_BiomeType::Montanha:
            // Mountain sounds: wind, echoes, sparse sounds
            BiomeAudioComponent->SetVolumeMultiplier(BiomeVolume * 0.6f);
            break;
    }
    
    if (!BiomeAudioComponent->IsPlaying())
    {
        BiomeAudioComponent->Play();
    }
}

void AWorld_EnvironmentalAudio::ApplyWeatherAudio()
{
    if (!WeatherAudioComponent)
    {
        return;
    }
    
    switch (CurrentWeatherType)
    {
        case EWorld_WeatherType::Clear:
            WeatherAudioComponent->Stop();
            break;
            
        case EWorld_WeatherType::Rain:
            WeatherAudioComponent->SetVolumeMultiplier(WeatherVolume * 0.8f);
            if (!WeatherAudioComponent->IsPlaying())
            {
                WeatherAudioComponent->Play();
            }
            break;
            
        case EWorld_WeatherType::Storm:
            WeatherAudioComponent->SetVolumeMultiplier(WeatherVolume * 1.0f);
            if (!WeatherAudioComponent->IsPlaying())
            {
                WeatherAudioComponent->Play();
            }
            break;
            
        case EWorld_WeatherType::Fog:
            WeatherAudioComponent->SetVolumeMultiplier(WeatherVolume * 0.3f);
            if (!WeatherAudioComponent->IsPlaying())
            {
                WeatherAudioComponent->Play();
            }
            break;
            
        case EWorld_WeatherType::Wind:
            WeatherAudioComponent->SetVolumeMultiplier(WeatherVolume * 0.6f);
            if (!WeatherAudioComponent->IsPlaying())
            {
                WeatherAudioComponent->Play();
            }
            break;
    }
}

void AWorld_EnvironmentalAudio::ApplyTimeOfDayAudio()
{
    if (!AmbientAudioComponent || !WildlifeAudioComponent)
    {
        return;
    }
    
    switch (CurrentTimeOfDay)
    {
        case EWorld_TimeOfDay::Dawn:
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * 0.6f);
            WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume * 0.8f);
            break;
            
        case EWorld_TimeOfDay::Day:
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * 1.0f);
            WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume * 1.0f);
            break;
            
        case EWorld_TimeOfDay::Dusk:
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * 0.7f);
            WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume * 0.6f);
            break;
            
        case EWorld_TimeOfDay::Night:
            AmbientAudioComponent->SetVolumeMultiplier(AmbientVolume * 0.4f);
            WildlifeAudioComponent->SetVolumeMultiplier(WildlifeVolume * 0.3f);
            break;
    }
    
    if (!AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    
    if (!WildlifeAudioComponent->IsPlaying())
    {
        WildlifeAudioComponent->Play();
    }
}

void AWorld_EnvironmentalAudio::UpdateEnvironmentalAudio()
{
    // This function is called periodically to update environmental audio
    // based on player location and world state
    
    UE_LOG(LogEnvironmentalAudio, VeryVerbose, TEXT("Updating environmental audio"));
    
    // Here we would typically:
    // 1. Check player location to determine current biome
    // 2. Query weather system for current weather
    // 3. Check time of day from world state
    // 4. Apply appropriate audio changes
    
    // For now, just ensure audio components are playing if they should be
    if (AmbientAudioComponent && !AmbientAudioComponent->IsPlaying())
    {
        AmbientAudioComponent->Play();
    }
    
    if (BiomeAudioComponent && !BiomeAudioComponent->IsPlaying())
    {
        ApplyBiomeAudio();
    }
}

void AWorld_EnvironmentalAudio::PlayRandomWildlifeSound()
{
    if (!WildlifeAudioComponent)
    {
        return;
    }
    
    // Play random wildlife sound based on current biome
    // This would typically load and play from a collection of wildlife sounds
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("Playing random wildlife sound for biome: %d"), (int32)CurrentBiome);
    
    if (!WildlifeAudioComponent->IsPlaying())
    {
        WildlifeAudioComponent->Play();
    }
}

void AWorld_EnvironmentalAudio::StopAllEnvironmentalAudio()
{
    if (AmbientAudioComponent)
    {
        AmbientAudioComponent->Stop();
    }
    
    if (WeatherAudioComponent)
    {
        WeatherAudioComponent->Stop();
    }
    
    if (BiomeAudioComponent)
    {
        BiomeAudioComponent->Stop();
    }
    
    if (WildlifeAudioComponent)
    {
        WildlifeAudioComponent->Stop();
    }
    
    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AudioUpdateTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(BiomeTransitionTimerHandle);
    }
    
    UE_LOG(LogEnvironmentalAudio, Log, TEXT("All environmental audio stopped"));
}
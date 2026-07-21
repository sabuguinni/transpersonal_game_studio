#include "Audio_WeatherSoundSystem.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UAudio_WeatherSoundSystem::UAudio_WeatherSoundSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize weather state
    CurrentWeatherType = EWeatherType::Clear;
    CurrentWeatherIntensity = 0.0f;
    bIsTransitioning = false;
    TransitionDuration = 5.0f;
    TransitionTimer = 0.0f;

    // Audio settings
    MaxAudioDistance = 5000.0f;
    VolumeMultiplier = 1.0f;
    ThunderRandomInterval = 10.0f;
    ThunderTimer = 0.0f;

    // Create audio components
    RainAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("RainAudioComponent"));
    ThunderAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ThunderAudioComponent"));
    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudioComponent"));
    FogAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("FogAudioComponent"));

    if (RainAudioComponent)
    {
        RainAudioComponent->bAutoActivate = false;
        RainAudioComponent->SetVolumeMultiplier(0.0f);
    }

    if (ThunderAudioComponent)
    {
        ThunderAudioComponent->bAutoActivate = false;
        ThunderAudioComponent->SetVolumeMultiplier(0.0f);
    }

    if (WindAudioComponent)
    {
        WindAudioComponent->bAutoActivate = false;
        WindAudioComponent->SetVolumeMultiplier(0.0f);
    }

    if (FogAudioComponent)
    {
        FogAudioComponent->bAutoActivate = false;
        FogAudioComponent->SetVolumeMultiplier(0.0f);
    }
}

void UAudio_WeatherSoundSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize audio components with default settings
    UpdateAudioComponentVolumes();
}

void UAudio_WeatherSoundSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateWeatherAudio(DeltaTime);

    if (bIsTransitioning)
    {
        ProcessWeatherTransition(DeltaTime);
    }

    // Handle thunder timing for storms
    if (CurrentWeatherType == EWeatherType::Storm && CurrentWeatherIntensity > 0.3f)
    {
        ThunderTimer += DeltaTime;
        if (ThunderTimer >= ThunderRandomInterval)
        {
            PlayThunderSound();
            ThunderTimer = 0.0f;
            // Randomize next thunder interval
            ThunderRandomInterval = FMath::RandRange(8.0f, 20.0f);
        }
    }
}

void UAudio_WeatherSoundSystem::SetWeatherType(EWeatherType NewWeatherType)
{
    if (NewWeatherType != CurrentWeatherType)
    {
        StartWeatherTransition(NewWeatherType, TransitionDuration);
    }
}

void UAudio_WeatherSoundSystem::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateAudioComponentVolumes();
}

void UAudio_WeatherSoundSystem::StartWeatherTransition(EWeatherType TargetWeather, float TransitionDur)
{
    if (TargetWeather == CurrentWeatherType)
        return;

    StartWeatherType = CurrentWeatherType;
    TargetWeatherType = TargetWeather;
    TransitionDuration = FMath::Max(TransitionDur, 0.1f);
    TransitionTimer = 0.0f;
    bIsTransitioning = true;

    UE_LOG(LogTemp, Log, TEXT("Starting weather transition from %d to %d over %.1f seconds"), 
           (int32)StartWeatherType, (int32)TargetWeatherType, TransitionDuration);
}

void UAudio_WeatherSoundSystem::StopAllWeatherSounds()
{
    if (RainAudioComponent && RainAudioComponent->IsPlaying())
    {
        RainAudioComponent->Stop();
    }

    if (ThunderAudioComponent && ThunderAudioComponent->IsPlaying())
    {
        ThunderAudioComponent->Stop();
    }

    if (WindAudioComponent && WindAudioComponent->IsPlaying())
    {
        WindAudioComponent->Stop();
    }

    if (FogAudioComponent && FogAudioComponent->IsPlaying())
    {
        FogAudioComponent->Stop();
    }

    CurrentWeatherIntensity = 0.0f;
    bIsTransitioning = false;
}

void UAudio_WeatherSoundSystem::UpdateWeatherAudio(float DeltaTime)
{
    // Update audio based on current weather type and intensity
    switch (CurrentWeatherType)
    {
        case EWeatherType::Clear:
            // Only gentle wind for clear weather
            if (WindAudioComponent && WindGentleSound)
            {
                if (!WindAudioComponent->IsPlaying())
                {
                    WindAudioComponent->SetSound(WindGentleSound);
                    WindAudioComponent->Play();
                }
            }
            break;

        case EWeatherType::Rain:
            // Rain sounds based on intensity
            if (RainAudioComponent)
            {
                USoundCue* RainSound = (CurrentWeatherIntensity > 0.5f) ? RainHeavySound : RainLightSound;
                if (RainSound && (!RainAudioComponent->IsPlaying() || RainAudioComponent->GetSound() != RainSound))
                {
                    RainAudioComponent->SetSound(RainSound);
                    RainAudioComponent->Play();
                }
            }
            break;

        case EWeatherType::Storm:
            // Heavy rain + strong wind + thunder
            if (RainAudioComponent && RainHeavySound)
            {
                if (!RainAudioComponent->IsPlaying())
                {
                    RainAudioComponent->SetSound(RainHeavySound);
                    RainAudioComponent->Play();
                }
            }
            
            if (WindAudioComponent && WindStrongSound)
            {
                if (!WindAudioComponent->IsPlaying())
                {
                    WindAudioComponent->SetSound(WindStrongSound);
                    WindAudioComponent->Play();
                }
            }
            break;

        case EWeatherType::Fog:
            // Muffled ambience for fog
            if (FogAudioComponent && FogAmbienceSound)
            {
                if (!FogAudioComponent->IsPlaying())
                {
                    FogAudioComponent->SetSound(FogAmbienceSound);
                    FogAudioComponent->Play();
                }
            }
            break;
    }

    UpdateAudioComponentVolumes();
}

void UAudio_WeatherSoundSystem::ProcessWeatherTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime;
    float TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentWeatherType = TargetWeatherType;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Weather transition completed to %d"), (int32)CurrentWeatherType);
    }
    else
    {
        // Blend between weather types during transition
        float BlendFactor = CalculateWeatherBlend(StartWeatherType, TargetWeatherType, TransitionProgress);
        
        // Update intensity based on blend
        if (TargetWeatherType != EWeatherType::Clear)
        {
            CurrentWeatherIntensity = FMath::Lerp(0.0f, 1.0f, BlendFactor);
        }
        else
        {
            CurrentWeatherIntensity = FMath::Lerp(1.0f, 0.0f, BlendFactor);
        }
    }
}

void UAudio_WeatherSoundSystem::PlayThunderSound()
{
    if (!ThunderAudioComponent)
        return;

    // Choose thunder sound based on intensity
    USoundCue* ThunderSound = (CurrentWeatherIntensity > 0.7f) ? ThunderCloseSound : ThunderDistantSound;
    
    if (ThunderSound)
    {
        ThunderAudioComponent->SetSound(ThunderSound);
        ThunderAudioComponent->Play();
        
        UE_LOG(LogTemp, Log, TEXT("Playing thunder sound - Intensity: %.2f"), CurrentWeatherIntensity);
    }
}

void UAudio_WeatherSoundSystem::UpdateAudioComponentVolumes()
{
    float BaseVolume = VolumeMultiplier * CurrentWeatherIntensity;

    if (RainAudioComponent)
    {
        float RainVolume = (CurrentWeatherType == EWeatherType::Rain || CurrentWeatherType == EWeatherType::Storm) 
                          ? BaseVolume : 0.0f;
        RainAudioComponent->SetVolumeMultiplier(RainVolume);
    }

    if (WindAudioComponent)
    {
        float WindVolume = BaseVolume * 0.7f; // Wind is always present but quieter
        if (CurrentWeatherType == EWeatherType::Storm)
        {
            WindVolume = BaseVolume; // Full volume for storms
        }
        WindAudioComponent->SetVolumeMultiplier(WindVolume);
    }

    if (FogAudioComponent)
    {
        float FogVolume = (CurrentWeatherType == EWeatherType::Fog) ? BaseVolume * 0.5f : 0.0f;
        FogAudioComponent->SetVolumeMultiplier(FogVolume);
    }

    if (ThunderAudioComponent)
    {
        float ThunderVolume = (CurrentWeatherType == EWeatherType::Storm) ? VolumeMultiplier : 0.0f;
        ThunderAudioComponent->SetVolumeMultiplier(ThunderVolume);
    }
}

float UAudio_WeatherSoundSystem::CalculateWeatherBlend(EWeatherType WeatherA, EWeatherType WeatherB, float BlendFactor)
{
    // Smooth transition curve
    float SmoothBlend = FMath::SmoothStep(0.0f, 1.0f, BlendFactor);
    
    // Adjust blend based on weather types
    if (WeatherA == EWeatherType::Clear || WeatherB == EWeatherType::Clear)
    {
        // Faster fade in/out for clear weather
        SmoothBlend = FMath::Pow(SmoothBlend, 0.7f);
    }
    
    return SmoothBlend;
}
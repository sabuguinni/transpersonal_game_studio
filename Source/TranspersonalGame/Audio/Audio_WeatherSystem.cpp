#include "Audio_WeatherSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UAudio_WeatherSystem::UAudio_WeatherSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    CurrentWeather = EAudio_WeatherType::Clear;
    TargetWeather = EAudio_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    WeatherIntensity = 1.0f;
    CurrentTransitionTime = 0.0f;
    bIsTransitioning = false;
    
    // Thunder settings
    ThunderMinInterval = 5.0f;
    ThunderMaxInterval = 20.0f;
    ThunderVolume = 0.8f;
    ThunderTimer = 0.0f;
    NextThunderTime = 0.0f;
}

void UAudio_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioComponents();
    ScheduleNextThunder();
    
    // Set initial weather
    SetWeatherType(CurrentWeather, true);
}

void UAudio_WeatherSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateWeatherTransition(DeltaTime);
    UpdateThunderSystem(DeltaTime);
}

void UAudio_WeatherSystem::InitializeAudioComponents()
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Create primary weather audio component
    PrimaryWeatherAudio = NewObject<UAudioComponent>(Owner);
    if (PrimaryWeatherAudio)
    {
        PrimaryWeatherAudio->SetupAttachment(Owner->GetRootComponent());
        PrimaryWeatherAudio->bAutoActivate = false;
        PrimaryWeatherAudio->SetVolumeMultiplier(0.0f);
        Owner->AddInstanceComponent(PrimaryWeatherAudio);
    }
    
    // Create secondary weather audio component for layering
    SecondaryWeatherAudio = NewObject<UAudioComponent>(Owner);
    if (SecondaryWeatherAudio)
    {
        SecondaryWeatherAudio->SetupAttachment(Owner->GetRootComponent());
        SecondaryWeatherAudio->bAutoActivate = false;
        SecondaryWeatherAudio->SetVolumeMultiplier(0.0f);
        Owner->AddInstanceComponent(SecondaryWeatherAudio);
    }
    
    // Create thunder audio component
    ThunderAudio = NewObject<UAudioComponent>(Owner);
    if (ThunderAudio)
    {
        ThunderAudio->SetupAttachment(Owner->GetRootComponent());
        ThunderAudio->bAutoActivate = false;
        ThunderAudio->SetVolumeMultiplier(ThunderVolume);
        Owner->AddInstanceComponent(ThunderAudio);
    }
}

void UAudio_WeatherSystem::SetWeatherType(EAudio_WeatherType NewWeatherType, bool bImmediate)
{
    if (NewWeatherType == CurrentWeather && !bIsTransitioning)
    {
        return;
    }
    
    TargetWeather = NewWeatherType;
    
    if (bImmediate)
    {
        CurrentWeather = TargetWeather;
        bIsTransitioning = false;
        CurrentTransitionTime = 0.0f;
        
        // Immediately play the new weather sound
        PlayWeatherSound(CurrentWeather, PrimaryWeatherAudio);
        
        if (SecondaryWeatherAudio && SecondaryWeatherAudio->IsPlaying())
        {
            StopWeatherSound(SecondaryWeatherAudio, 0.1f);
        }
    }
    else
    {
        bIsTransitioning = true;
        CurrentTransitionTime = 0.0f;
        
        // Start playing the target weather on secondary component
        PlayWeatherSound(TargetWeather, SecondaryWeatherAudio);
    }
}

void UAudio_WeatherSystem::SetWeatherIntensity(float NewIntensity)
{
    WeatherIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    
    // Update volume of currently playing sounds
    if (PrimaryWeatherAudio && PrimaryWeatherAudio->IsPlaying())
    {
        float Volume = CalculateVolumeForWeather(CurrentWeather);
        PrimaryWeatherAudio->SetVolumeMultiplier(Volume);
    }
    
    if (SecondaryWeatherAudio && SecondaryWeatherAudio->IsPlaying())
    {
        float Volume = CalculateVolumeForWeather(TargetWeather);
        SecondaryWeatherAudio->SetVolumeMultiplier(Volume);
    }
}

void UAudio_WeatherSystem::TriggerThunder(bool bRandomDelay)
{
    if (!ThunderAudio || ThunderSounds.Num() == 0)
    {
        return;
    }
    
    // Select random thunder sound
    int32 ThunderIndex = FMath::RandRange(0, ThunderSounds.Num() - 1);
    USoundCue* ThunderSound = ThunderSounds[ThunderIndex].LoadSynchronous();
    
    if (ThunderSound)
    {
        if (bRandomDelay)
        {
            float Delay = FMath::RandRange(0.5f, 3.0f);
            GetWorld()->GetTimerManager().SetTimer(
                FTimerHandle(),
                [this, ThunderSound]()
                {
                    ThunderAudio->SetSound(ThunderSound);
                    ThunderAudio->Play();
                },
                Delay,
                false
            );
        }
        else
        {
            ThunderAudio->SetSound(ThunderSound);
            ThunderAudio->Play();
        }
    }
    
    ScheduleNextThunder();
}

void UAudio_WeatherSystem::SetThunderEnabled(bool bEnabled)
{
    if (!bEnabled)
    {
        ThunderTimer = 0.0f;
        NextThunderTime = 0.0f;
        
        if (ThunderAudio && ThunderAudio->IsPlaying())
        {
            ThunderAudio->Stop();
        }
    }
    else
    {
        ScheduleNextThunder();
    }
}

void UAudio_WeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    CurrentTransitionTime += DeltaTime * WeatherTransitionSpeed;
    
    if (CurrentTransitionTime >= 1.0f)
    {
        // Transition complete
        bIsTransitioning = false;
        CurrentTransitionTime = 0.0f;
        
        // Swap audio components
        if (PrimaryWeatherAudio && PrimaryWeatherAudio->IsPlaying())
        {
            StopWeatherSound(PrimaryWeatherAudio, 0.1f);
        }
        
        // Swap the components
        UAudioComponent* Temp = PrimaryWeatherAudio;
        PrimaryWeatherAudio = SecondaryWeatherAudio;
        SecondaryWeatherAudio = Temp;
        
        CurrentWeather = TargetWeather;
    }
    else
    {
        // Update volumes during transition
        float TransitionAlpha = CurrentTransitionTime;
        
        if (PrimaryWeatherAudio)
        {
            float CurrentVolume = CalculateVolumeForWeather(CurrentWeather) * (1.0f - TransitionAlpha);
            PrimaryWeatherAudio->SetVolumeMultiplier(CurrentVolume);
        }
        
        if (SecondaryWeatherAudio)
        {
            float TargetVolume = CalculateVolumeForWeather(TargetWeather) * TransitionAlpha;
            SecondaryWeatherAudio->SetVolumeMultiplier(TargetVolume);
        }
    }
}

void UAudio_WeatherSystem::UpdateThunderSystem(float DeltaTime)
{
    // Only play thunder during storms
    if (CurrentWeather != EAudio_WeatherType::Storm && TargetWeather != EAudio_WeatherType::Storm)
    {
        return;
    }
    
    ThunderTimer += DeltaTime;
    
    if (ThunderTimer >= NextThunderTime && NextThunderTime > 0.0f)
    {
        TriggerThunder(true);
        ThunderTimer = 0.0f;
    }
}

void UAudio_WeatherSystem::PlayWeatherSound(EAudio_WeatherType WeatherType, UAudioComponent* AudioComponent)
{
    if (!AudioComponent)
    {
        return;
    }
    
    const FAudio_WeatherSoundData* SoundData = WeatherSounds.Find(WeatherType);
    if (!SoundData || !SoundData->WeatherSound.IsValid())
    {
        return;
    }
    
    USoundCue* WeatherSound = SoundData->WeatherSound.LoadSynchronous();
    if (WeatherSound)
    {
        AudioComponent->SetSound(WeatherSound);
        AudioComponent->bAutoActivate = SoundData->bLooping;
        
        float Volume = CalculateVolumeForWeather(WeatherType);
        AudioComponent->SetVolumeMultiplier(Volume);
        
        AudioComponent->Play();
    }
}

void UAudio_WeatherSystem::StopWeatherSound(UAudioComponent* AudioComponent, float FadeTime)
{
    if (AudioComponent && AudioComponent->IsPlaying())
    {
        if (FadeTime > 0.0f)
        {
            AudioComponent->FadeOut(FadeTime, 0.0f);
        }
        else
        {
            AudioComponent->Stop();
        }
    }
}

float UAudio_WeatherSystem::CalculateVolumeForWeather(EAudio_WeatherType WeatherType) const
{
    const FAudio_WeatherSoundData* SoundData = WeatherSounds.Find(WeatherType);
    if (!SoundData)
    {
        return 0.0f;
    }
    
    return SoundData->BaseVolume * WeatherIntensity;
}

void UAudio_WeatherSystem::ScheduleNextThunder()
{
    if (CurrentWeather == EAudio_WeatherType::Storm || TargetWeather == EAudio_WeatherType::Storm)
    {
        NextThunderTime = FMath::RandRange(ThunderMinInterval, ThunderMaxInterval);
        ThunderTimer = 0.0f;
    }
    else
    {
        NextThunderTime = 0.0f;
    }
}
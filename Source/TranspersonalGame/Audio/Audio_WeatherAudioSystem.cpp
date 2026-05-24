#include "Audio_WeatherAudioSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

AAudio_WeatherAudioSystem::AAudio_WeatherAudioSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio components
    PrimaryWeatherAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("PrimaryWeatherAudio"));
    RootComponent = PrimaryWeatherAudio;
    PrimaryWeatherAudio->bAutoActivate = false;
    PrimaryWeatherAudio->SetVolumeMultiplier(0.0f);

    SecondaryWeatherAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("SecondaryWeatherAudio"));
    SecondaryWeatherAudio->SetupAttachment(RootComponent);
    SecondaryWeatherAudio->bAutoActivate = false;
    SecondaryWeatherAudio->SetVolumeMultiplier(0.0f);

    TransitionAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("TransitionAudio"));
    TransitionAudio->SetupAttachment(RootComponent);
    TransitionAudio->bAutoActivate = false;
    TransitionAudio->SetVolumeMultiplier(0.0f);

    EmergencyAlertAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EmergencyAlertAudio"));
    EmergencyAlertAudio->SetupAttachment(RootComponent);
    EmergencyAlertAudio->bAutoActivate = false;
    EmergencyAlertAudio->SetVolumeMultiplier(0.0f);

    // Initialize default values
    CurrentWeatherType = EAudio_WeatherType::Clear;
    CurrentIntensity = 0.0f;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    GlobalWeatherVolume = 0.7f;
    MaxAudibleDistance = 5000.0f;
    AlertVolume = 0.9f;
    DistanceFromShelter = 1000.0f;
    RecentLightningStrikes = 0;
    CurrentWindDirection = FVector::ForwardVector;
}

void AAudio_WeatherAudioSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherConfigs();
    
    // Start with clear weather
    SetWeatherType(EAudio_WeatherType::Clear, 0.0f);
    
    // Set up periodic intensity updates
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            IntensityUpdateTimer,
            this,
            &AAudio_WeatherAudioSystem::UpdateWeatherAudio,
            0.1f,
            true
        );
    }
}

void AAudio_WeatherAudioSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllWeatherAudio();
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
    }
    
    Super::EndPlay(EndPlayReason);
}

void AAudio_WeatherAudioSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsTransitioning)
    {
        HandleWeatherTransition();
    }
    
    UpdateEnvironmentalFactors();
}

void AAudio_WeatherAudioSystem::SetWeatherType(EAudio_WeatherType NewWeatherType, float TransitionTime)
{
    if (NewWeatherType == CurrentWeatherType && !bIsTransitioning)
    {
        return;
    }
    
    if (TransitionTime <= 0.0f)
    {
        // Immediate change
        CurrentWeatherType = NewWeatherType;
        bIsTransitioning = false;
        UpdateWeatherAudio();
    }
    else
    {
        // Gradual transition
        StartWeatherTransition(CurrentWeatherType, NewWeatherType, TransitionTime);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weather Audio: Transitioning to %s over %.1f seconds"), 
           *UEnum::GetValueAsString(NewWeatherType), TransitionTime);
}

void AAudio_WeatherAudioSystem::SetWeatherIntensity(float Intensity)
{
    CurrentIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateWeatherAudio();
}

void AAudio_WeatherAudioSystem::StartWeatherTransition(EAudio_WeatherType FromWeather, EAudio_WeatherType ToWeather, float Duration)
{
    TransitionFromWeather = FromWeather;
    TransitionToWeather = ToWeather;
    TransitionDuration = Duration;
    TransitionTimer = 0.0f;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    // Play transition sound if available
    if (WeatherConfigs.Contains(ToWeather))
    {
        const FAudio_WeatherAudioConfig& Config = WeatherConfigs[ToWeather];
        if (Config.TransitionSound.IsValid())
        {
            USoundCue* TransitionSoundCue = Config.TransitionSound.LoadSynchronous();
            if (TransitionSoundCue && TransitionAudio)
            {
                TransitionAudio->SetSound(TransitionSoundCue);
                TransitionAudio->SetVolumeMultiplier(GlobalWeatherVolume * 0.6f);
                TransitionAudio->Play();
            }
        }
    }
}

void AAudio_WeatherAudioSystem::EmergencyWeatherAlert(EAudio_WeatherType WeatherType)
{
    if (!EmergencyAlertAudio)
    {
        return;
    }
    
    USoundCue* AlertSound = nullptr;
    
    switch (WeatherType)
    {
        case EAudio_WeatherType::Thunderstorm:
            if (ThunderstormAlert.IsValid())
            {
                AlertSound = ThunderstormAlert.LoadSynchronous();
            }
            break;
        case EAudio_WeatherType::WindStorm:
            if (WindStormAlert.IsValid())
            {
                AlertSound = WindStormAlert.LoadSynchronous();
            }
            break;
        case EAudio_WeatherType::Sandstorm:
            if (SandstormAlert.IsValid())
            {
                AlertSound = SandstormAlert.LoadSynchronous();
            }
            break;
        default:
            break;
    }
    
    if (AlertSound)
    {
        EmergencyAlertAudio->SetSound(AlertSound);
        EmergencyAlertAudio->SetVolumeMultiplier(AlertVolume);
        EmergencyAlertAudio->Play();
        
        UE_LOG(LogTemp, Warning, TEXT("Weather Emergency Alert: %s"), 
               *UEnum::GetValueAsString(WeatherType));
    }
}

void AAudio_WeatherAudioSystem::StopAllWeatherAudio()
{
    if (PrimaryWeatherAudio)
    {
        PrimaryWeatherAudio->Stop();
    }
    
    if (SecondaryWeatherAudio)
    {
        SecondaryWeatherAudio->Stop();
    }
    
    if (TransitionAudio)
    {
        TransitionAudio->Stop();
    }
    
    if (EmergencyAlertAudio)
    {
        EmergencyAlertAudio->Stop();
    }
    
    bIsTransitioning = false;
}

void AAudio_WeatherAudioSystem::UpdateWindDirection(FVector WindDirection)
{
    CurrentWindDirection = WindDirection.GetSafeNormal();
    UpdateWeatherAudio();
}

void AAudio_WeatherAudioSystem::SetDistanceFromShelter(float Distance)
{
    DistanceFromShelter = FMath::Max(0.0f, Distance);
    UpdateWeatherAudio();
}

void AAudio_WeatherAudioSystem::TriggerLightningStrike(FVector StrikeLocation)
{
    RecentLightningStrikes++;
    ProcessLightningAudio(StrikeLocation);
    
    // Reset lightning counter after 30 seconds
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            LightningResetTimer,
            [this]() { RecentLightningStrikes = FMath::Max(0, RecentLightningStrikes - 1); },
            30.0f,
            false
        );
    }
}

void AAudio_WeatherAudioSystem::InitializeWeatherConfigs()
{
    // Initialize default configurations for each weather type
    for (int32 i = 0; i < (int32)EAudio_WeatherType::Sandstorm + 1; i++)
    {
        EAudio_WeatherType WeatherType = (EAudio_WeatherType)i;
        FAudio_WeatherAudioConfig Config;
        
        switch (WeatherType)
        {
            case EAudio_WeatherType::Clear:
                Config.BaseVolume = 0.2f;
                Config.IntensityMultiplier = 0.5f;
                break;
            case EAudio_WeatherType::LightRain:
                Config.BaseVolume = 0.4f;
                Config.IntensityMultiplier = 0.8f;
                break;
            case EAudio_WeatherType::HeavyRain:
                Config.BaseVolume = 0.7f;
                Config.IntensityMultiplier = 1.2f;
                break;
            case EAudio_WeatherType::Thunderstorm:
                Config.BaseVolume = 0.8f;
                Config.IntensityMultiplier = 1.5f;
                break;
            case EAudio_WeatherType::WindStorm:
                Config.BaseVolume = 0.6f;
                Config.IntensityMultiplier = 1.3f;
                break;
            case EAudio_WeatherType::Fog:
                Config.BaseVolume = 0.3f;
                Config.IntensityMultiplier = 0.7f;
                break;
            case EAudio_WeatherType::Sandstorm:
                Config.BaseVolume = 0.9f;
                Config.IntensityMultiplier = 1.4f;
                break;
        }
        
        WeatherConfigs.Add(WeatherType, Config);
    }
}

void AAudio_WeatherAudioSystem::UpdateWeatherAudio()
{
    if (!WeatherConfigs.Contains(CurrentWeatherType))
    {
        return;
    }
    
    const FAudio_WeatherAudioConfig& Config = WeatherConfigs[CurrentWeatherType];
    
    // Calculate final volume based on intensity and environmental factors
    float FinalVolume = Config.BaseVolume * GlobalWeatherVolume;
    FinalVolume *= (Config.IntensityMultiplier * CurrentIntensity);
    
    // Adjust for distance from shelter
    if (DistanceFromShelter < 500.0f)
    {
        FinalVolume *= 0.3f; // Muffled when in shelter
    }
    else if (DistanceFromShelter < 1000.0f)
    {
        float ShelterFactor = (DistanceFromShelter - 500.0f) / 500.0f;
        FinalVolume *= FMath::Lerp(0.3f, 1.0f, ShelterFactor);
    }
    
    // Apply volume to primary audio component
    if (PrimaryWeatherAudio && Config.AmbientSound.IsValid())
    {
        USoundCue* AmbientSoundCue = Config.AmbientSound.LoadSynchronous();
        if (AmbientSoundCue)
        {
            if (PrimaryWeatherAudio->GetSound() != AmbientSoundCue)
            {
                PrimaryWeatherAudio->SetSound(AmbientSoundCue);
            }
            
            PrimaryWeatherAudio->SetVolumeMultiplier(FinalVolume);
            
            if (!PrimaryWeatherAudio->IsPlaying() && FinalVolume > 0.01f)
            {
                PrimaryWeatherAudio->Play();
            }
        }
    }
    
    // Handle intensity-based secondary audio
    if (SecondaryWeatherAudio && Config.IntensitySound.IsValid() && CurrentIntensity > 0.5f)
    {
        USoundCue* IntensitySoundCue = Config.IntensitySound.LoadSynchronous();
        if (IntensitySoundCue)
        {
            if (SecondaryWeatherAudio->GetSound() != IntensitySoundCue)
            {
                SecondaryWeatherAudio->SetSound(IntensitySoundCue);
            }
            
            float IntensityVolume = FinalVolume * (CurrentIntensity - 0.5f) * 2.0f;
            SecondaryWeatherAudio->SetVolumeMultiplier(IntensityVolume);
            
            if (!SecondaryWeatherAudio->IsPlaying() && IntensityVolume > 0.01f)
            {
                SecondaryWeatherAudio->Play();
            }
        }
    }
    else if (SecondaryWeatherAudio->IsPlaying())
    {
        SecondaryWeatherAudio->Stop();
    }
}

void AAudio_WeatherAudioSystem::HandleWeatherTransition()
{
    if (!bIsTransitioning)
    {
        return;
    }
    
    TransitionTimer += GetWorld()->GetDeltaSeconds();
    TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    if (TransitionProgress >= 1.0f)
    {
        // Transition complete
        CurrentWeatherType = TransitionToWeather;
        bIsTransitioning = false;
        TransitionProgress = 1.0f;
        
        if (TransitionAudio && TransitionAudio->IsPlaying())
        {
            TransitionAudio->Stop();
        }
    }
    
    UpdateWeatherAudio();
}

void AAudio_WeatherAudioSystem::ProcessLightningAudio(FVector StrikeLocation)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate distance to lightning strike
    float Distance = FVector::Dist(GetActorLocation(), StrikeLocation);
    
    // Play thunder sound with distance-based delay and volume
    float SoundDelay = Distance / 343.0f; // Speed of sound in m/s
    float VolumeMultiplier = FMath::Clamp(1.0f - (Distance / MaxAudibleDistance), 0.0f, 1.0f);
    
    if (VolumeMultiplier > 0.01f)
    {
        // Schedule thunder sound
        FTimerHandle ThunderTimer;
        GetWorld()->GetTimerManager().SetTimer(
            ThunderTimer,
            [this, VolumeMultiplier]()
            {
                if (EmergencyAlertAudio && ThunderstormAlert.IsValid())
                {
                    USoundCue* ThunderSound = ThunderstormAlert.LoadSynchronous();
                    if (ThunderSound)
                    {
                        EmergencyAlertAudio->SetSound(ThunderSound);
                        EmergencyAlertAudio->SetVolumeMultiplier(VolumeMultiplier * AlertVolume);
                        EmergencyAlertAudio->Play();
                    }
                }
            },
            SoundDelay,
            false
        );
    }
}

void AAudio_WeatherAudioSystem::UpdateEnvironmentalFactors()
{
    // Update environmental factors that affect audio
    if (GetWorld())
    {
        // Check for nearby players to adjust shelter distance
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float PlayerDistance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            if (PlayerDistance > MaxAudibleDistance)
            {
                // Player too far, reduce all audio
                GlobalWeatherVolume = FMath::Lerp(GlobalWeatherVolume, 0.1f, 0.1f);
            }
            else
            {
                // Player in range, restore normal volume
                GlobalWeatherVolume = FMath::Lerp(GlobalWeatherVolume, 0.7f, 0.1f);
            }
        }
    }
}

void AAudio_WeatherAudioSystem::CalculateWeatherVolume()
{
    // Additional volume calculations based on time of day, season, etc.
    // This can be expanded with more complex environmental factors
}
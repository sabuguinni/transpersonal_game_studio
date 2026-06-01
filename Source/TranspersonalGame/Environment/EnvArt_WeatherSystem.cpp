#include "EnvArt_WeatherSystem.h"
#include "Components/SceneComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_WeatherSystem::AEnvArt_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    // Create directional light for sun
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetCastShadows(true);

    // Create sky light for ambient lighting
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);

    // Create particle systems
    RainParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainParticles"));
    RainParticles->SetupAttachment(RootComponent);
    RainParticles->SetAutoActivate(false);

    DustParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    DustParticles->SetupAttachment(RootComponent);
    DustParticles->SetAutoActivate(false);

    // Create audio component
    WeatherAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
    WeatherAudio->SetupAttachment(RootComponent);
    WeatherAudio->SetAutoActivate(false);

    // Initialize default values
    CurrentWeather = EEnvArt_WeatherType::Clear;
    TargetWeather = EEnvArt_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
    WeatherChangeInterval = 300.0f; // 5 minutes
    bAutoWeatherChange = true;
    
    TimeOfDay = 12.0f; // Noon
    DayDurationMinutes = 20.0f; // 20 minute day cycle
    bAutoTimeProgression = true;
    
    WeatherTransitionProgress = 0.0f;
    LastWeatherChangeTime = 0.0f;

    // Initialize weather presets
    InitializeWeatherPresets();
}

void AEnvArt_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find height fog in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Set initial weather settings
    if (WeatherPresets.Contains(CurrentWeather))
    {
        CurrentSettings = WeatherPresets[CurrentWeather];
        TargetSettings = CurrentSettings;
        ApplyWeatherSettings(CurrentSettings);
    }

    // Start weather change timer if auto change is enabled
    if (bAutoWeatherChange)
    {
        GetWorldTimerManager().SetTimer(WeatherChangeTimer, this, &AEnvArt_WeatherSystem::OnWeatherChangeTimer, 
                                       WeatherChangeInterval, true);
    }

    UpdateSunPosition();
}

void AEnvArt_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoTimeProgression)
    {
        UpdateTimeOfDay(DeltaTime);
    }

    UpdateWeatherTransition(DeltaTime);
    UpdateSunPosition();
    UpdateLighting();
    UpdateFog();
    UpdateParticles();
    UpdateAudio();
}

void AEnvArt_WeatherSystem::SetWeather(EEnvArt_WeatherType NewWeather, bool bInstant)
{
    if (NewWeather == CurrentWeather)
        return;

    TargetWeather = NewWeather;
    
    if (WeatherPresets.Contains(TargetWeather))
    {
        TargetSettings = WeatherPresets[TargetWeather];
    }

    if (bInstant)
    {
        CurrentWeather = TargetWeather;
        CurrentSettings = TargetSettings;
        WeatherTransitionProgress = 0.0f;
        ApplyWeatherSettings(CurrentSettings);
    }
    else
    {
        WeatherTransitionProgress = 0.0f;
    }
}

void AEnvArt_WeatherSystem::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Fmod(NewTime, 24.0f);
    if (TimeOfDay < 0.0f)
    {
        TimeOfDay += 24.0f;
    }
    UpdateSunPosition();
}

void AEnvArt_WeatherSystem::TriggerRandomWeatherChange()
{
    EEnvArt_WeatherType NewWeather = GetRandomWeatherType();
    SetWeather(NewWeather, false);
}

void AEnvArt_WeatherSystem::UpdateWeatherTransition(float DeltaTime)
{
    if (CurrentWeather != TargetWeather)
    {
        WeatherTransitionProgress += DeltaTime * WeatherTransitionSpeed;
        
        if (WeatherTransitionProgress >= 1.0f)
        {
            // Transition complete
            CurrentWeather = TargetWeather;
            CurrentSettings = TargetSettings;
            WeatherTransitionProgress = 0.0f;
        }
        else
        {
            // Lerp between current and target settings
            FEnvArt_WeatherSettings StartSettings = WeatherPresets.Contains(CurrentWeather) ? 
                WeatherPresets[CurrentWeather] : CurrentSettings;
            CurrentSettings = LerpWeatherSettings(StartSettings, TargetSettings, WeatherTransitionProgress);
        }
        
        ApplyWeatherSettings(CurrentSettings);
    }
}

void AEnvArt_WeatherSystem::UpdateTimeOfDay(float DeltaTime)
{
    float HoursPerSecond = 24.0f / (DayDurationMinutes * 60.0f);
    TimeOfDay += HoursPerSecond * DeltaTime;
    
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
}

void AEnvArt_WeatherSystem::UpdateSunPosition()
{
    if (!SunLight)
        return;

    // Convert time of day to sun angle
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f; // 6 AM = 0 degrees, 6 PM = 180 degrees
    
    // Calculate sun rotation
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle - 90.0f; // Adjust for proper sun arc
    SunRotation.Yaw = 0.0f;
    SunRotation.Roll = 0.0f;
    
    SunLight->SetWorldRotation(SunRotation);
    
    // Adjust intensity based on time of day
    float SunIntensityMultiplier = 1.0f;
    if (TimeOfDay < 6.0f || TimeOfDay > 18.0f)
    {
        // Night time - very dim
        SunIntensityMultiplier = 0.1f;
    }
    else if (TimeOfDay < 8.0f || TimeOfDay > 16.0f)
    {
        // Dawn/dusk - reduced intensity
        float DawnDuskFactor = FMath::Min(
            FMath::Abs(TimeOfDay - 6.0f) / 2.0f,
            FMath::Abs(18.0f - TimeOfDay) / 2.0f
        );
        SunIntensityMultiplier = FMath::Lerp(0.1f, 1.0f, DawnDuskFactor);
    }
    
    SunLight->SetIntensity(CurrentSettings.SunIntensity * SunIntensityMultiplier);
}

void AEnvArt_WeatherSystem::UpdateLighting()
{
    if (SunLight)
    {
        SunLight->SetLightColor(CurrentSettings.SunColor);
    }
    
    if (SkyLight)
    {
        SkyLight->SetIntensity(CurrentSettings.SkyLightIntensity);
    }
}

void AEnvArt_WeatherSystem::UpdateFog()
{
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(CurrentSettings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(CurrentSettings.FogColor);
    }
}

void AEnvArt_WeatherSystem::UpdateParticles()
{
    // Update rain particles
    bool bShouldRain = (CurrentWeather == EEnvArt_WeatherType::Rain || 
                       CurrentWeather == EEnvArt_WeatherType::Storm);
    
    if (RainParticles)
    {
        if (bShouldRain && !RainParticles->IsActive())
        {
            RainParticles->Activate();
        }
        else if (!bShouldRain && RainParticles->IsActive())
        {
            RainParticles->Deactivate();
        }
    }
    
    // Update dust particles
    bool bShouldDust = (CurrentWeather == EEnvArt_WeatherType::Dust);
    
    if (DustParticles)
    {
        if (bShouldDust && !DustParticles->IsActive())
        {
            DustParticles->Activate();
        }
        else if (!bShouldDust && DustParticles->IsActive())
        {
            DustParticles->Deactivate();
        }
    }
}

void AEnvArt_WeatherSystem::UpdateAudio()
{
    if (!WeatherAudio)
        return;

    USoundCue* TargetSound = nullptr;
    
    switch (CurrentWeather)
    {
        case EEnvArt_WeatherType::Rain:
        case EEnvArt_WeatherType::Storm:
            TargetSound = RainSound;
            break;
        case EEnvArt_WeatherType::Dust:
            TargetSound = WindSound;
            break;
        default:
            TargetSound = nullptr;
            break;
    }
    
    if (TargetSound && WeatherAudio->GetSound() != TargetSound)
    {
        WeatherAudio->SetSound(TargetSound);
        WeatherAudio->Play();
    }
    else if (!TargetSound && WeatherAudio->IsPlaying())
    {
        WeatherAudio->Stop();
    }
}

void AEnvArt_WeatherSystem::ApplyWeatherSettings(const FEnvArt_WeatherSettings& Settings)
{
    CurrentSettings = Settings;
}

void AEnvArt_WeatherSystem::InitializeWeatherPresets()
{
    // Clear Sky
    FEnvArt_WeatherSettings ClearSettings;
    ClearSettings.SunIntensity = 3.0f;
    ClearSettings.SunColor = FLinearColor::White;
    ClearSettings.SkyLightIntensity = 1.0f;
    ClearSettings.FogDensity = 0.01f;
    ClearSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    ClearSettings.WindStrength = 0.5f;
    ClearSettings.Temperature = 25.0f;
    ClearSettings.Humidity = 0.4f;
    WeatherPresets.Add(EEnvArt_WeatherType::Clear, ClearSettings);

    // Cloudy
    FEnvArt_WeatherSettings CloudySettings;
    CloudySettings.SunIntensity = 2.0f;
    CloudySettings.SunColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    CloudySettings.SkyLightIntensity = 0.8f;
    CloudySettings.FogDensity = 0.02f;
    CloudySettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    CloudySettings.WindStrength = 1.0f;
    CloudySettings.Temperature = 22.0f;
    CloudySettings.Humidity = 0.6f;
    WeatherPresets.Add(EEnvArt_WeatherType::Cloudy, CloudySettings);

    // Rain
    FEnvArt_WeatherSettings RainSettings;
    RainSettings.SunIntensity = 1.0f;
    RainSettings.SunColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
    RainSettings.SkyLightIntensity = 0.5f;
    RainSettings.FogDensity = 0.05f;
    RainSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    RainSettings.WindStrength = 2.0f;
    RainSettings.Temperature = 18.0f;
    RainSettings.Humidity = 0.9f;
    WeatherPresets.Add(EEnvArt_WeatherType::Rain, RainSettings);

    // Storm
    FEnvArt_WeatherSettings StormSettings;
    StormSettings.SunIntensity = 0.5f;
    StormSettings.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormSettings.SkyLightIntensity = 0.3f;
    StormSettings.FogDensity = 0.08f;
    StormSettings.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
    StormSettings.WindStrength = 4.0f;
    StormSettings.Temperature = 15.0f;
    StormSettings.Humidity = 1.0f;
    WeatherPresets.Add(EEnvArt_WeatherType::Storm, StormSettings);

    // Fog
    FEnvArt_WeatherSettings FogSettings;
    FogSettings.SunIntensity = 1.5f;
    FogSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    FogSettings.SkyLightIntensity = 0.6f;
    FogSettings.FogDensity = 0.15f;
    FogSettings.FogColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);
    FogSettings.WindStrength = 0.2f;
    FogSettings.Temperature = 20.0f;
    FogSettings.Humidity = 0.8f;
    WeatherPresets.Add(EEnvArt_WeatherType::Fog, FogSettings);

    // Dust Storm
    FEnvArt_WeatherSettings DustSettings;
    DustSettings.SunIntensity = 2.0f;
    DustSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DustSettings.SkyLightIntensity = 0.7f;
    DustSettings.FogDensity = 0.1f;
    DustSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    DustSettings.WindStrength = 3.0f;
    DustSettings.Temperature = 35.0f;
    DustSettings.Humidity = 0.1f;
    WeatherPresets.Add(EEnvArt_WeatherType::Dust, DustSettings);
}

void AEnvArt_WeatherSystem::OnWeatherChangeTimer()
{
    TriggerRandomWeatherChange();
}

FEnvArt_WeatherSettings AEnvArt_WeatherSystem::LerpWeatherSettings(const FEnvArt_WeatherSettings& A, const FEnvArt_WeatherSettings& B, float Alpha)
{
    FEnvArt_WeatherSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.WindStrength = FMath::Lerp(A.WindStrength, B.WindStrength, Alpha);
    Result.Temperature = FMath::Lerp(A.Temperature, B.Temperature, Alpha);
    Result.Humidity = FMath::Lerp(A.Humidity, B.Humidity, Alpha);
    
    return Result;
}

EEnvArt_WeatherType AEnvArt_WeatherSystem::GetRandomWeatherType()
{
    TArray<EEnvArt_WeatherType> WeatherTypes = {
        EEnvArt_WeatherType::Clear,
        EEnvArt_WeatherType::Cloudy,
        EEnvArt_WeatherType::Rain,
        EEnvArt_WeatherType::Storm,
        EEnvArt_WeatherType::Fog,
        EEnvArt_WeatherType::Dust
    };
    
    int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
    return WeatherTypes[RandomIndex];
}
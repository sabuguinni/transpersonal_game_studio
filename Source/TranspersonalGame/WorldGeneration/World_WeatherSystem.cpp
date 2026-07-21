#include "World_WeatherSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

AWorld_WeatherSystem::AWorld_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle components
    RainParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RainParticles"));
    RainParticles->SetupAttachment(RootComponent);
    RainParticles->bAutoActivate = false;

    SnowParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SnowParticles"));
    SnowParticles->SetupAttachment(RootComponent);
    SnowParticles->bAutoActivate = false;

    SandstormParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SandstormParticles"));
    SandstormParticles->SetupAttachment(RootComponent);
    SandstormParticles->bAutoActivate = false;

    // Create audio component
    WeatherAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WeatherAudio"));
    WeatherAudio->SetupAttachment(RootComponent);
    WeatherAudio->bAutoActivate = false;

    // Initialize weather data
    CurrentWeather = FWorld_WeatherData();
    ClimateZone = EWorld_ClimateZone::Temperate;
    WeatherCycleDuration = 300.0f;
    WeatherTransitionSpeed = 0.1f;
    bEnableRandomWeather = true;
    bEnableSeasonalChanges = true;

    // Initialize timers
    WeatherTimer = 0.0f;
    TransitionTimer = 0.0f;
    bIsTransitioning = false;

    // Initialize references
    FogActor = nullptr;
    SunLight = nullptr;
    RainSound = nullptr;
    StormSound = nullptr;
    WindSound = nullptr;
}

void AWorld_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find fog and lighting actors in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Initialize weather system
    if (bEnableRandomWeather)
    {
        GenerateRandomWeather();
    }
    
    ApplyWeatherEffects();
}

void AWorld_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableRandomWeather)
    {
        UpdateWeatherCycle(DeltaTime);
    }

    if (bIsTransitioning)
    {
        ProcessWeatherTransition(DeltaTime);
    }
}

void AWorld_WeatherSystem::SetWeather(EWorld_WeatherType NewWeatherType, float NewIntensity)
{
    FWorld_WeatherData NewWeather;
    NewWeather.WeatherType = NewWeatherType;
    NewWeather.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);

    // Set appropriate values based on weather type
    switch (NewWeatherType)
    {
        case EWorld_WeatherType::Clear:
            NewWeather.WindSpeed = 5.0f;
            NewWeather.Humidity = 30.0f;
            NewWeather.Visibility = 1.0f;
            break;
        case EWorld_WeatherType::Cloudy:
            NewWeather.WindSpeed = 10.0f;
            NewWeather.Humidity = 60.0f;
            NewWeather.Visibility = 0.8f;
            break;
        case EWorld_WeatherType::Rain:
            NewWeather.WindSpeed = 15.0f;
            NewWeather.Humidity = 90.0f;
            NewWeather.Visibility = 0.6f;
            break;
        case EWorld_WeatherType::Storm:
            NewWeather.WindSpeed = 30.0f;
            NewWeather.Humidity = 95.0f;
            NewWeather.Visibility = 0.3f;
            break;
        case EWorld_WeatherType::Fog:
            NewWeather.WindSpeed = 2.0f;
            NewWeather.Humidity = 95.0f;
            NewWeather.Visibility = 0.2f;
            break;
        case EWorld_WeatherType::Sandstorm:
            NewWeather.WindSpeed = 40.0f;
            NewWeather.Humidity = 10.0f;
            NewWeather.Visibility = 0.1f;
            break;
        case EWorld_WeatherType::Snow:
            NewWeather.WindSpeed = 12.0f;
            NewWeather.Humidity = 80.0f;
            NewWeather.Visibility = 0.5f;
            NewWeather.Temperature = -5.0f;
            break;
    }

    StartWeatherTransition(NewWeather);
}

void AWorld_WeatherSystem::StartWeatherTransition(const FWorld_WeatherData& NewWeather)
{
    TargetWeather = NewWeather;
    bIsTransitioning = true;
    TransitionTimer = 0.0f;
}

void AWorld_WeatherSystem::SetClimateZone(EWorld_ClimateZone NewClimateZone)
{
    ClimateZone = NewClimateZone;
    
    if (bEnableRandomWeather)
    {
        GenerateRandomWeather();
    }
}

void AWorld_WeatherSystem::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }

    float FogDensity = 0.02f;
    float FogHeight = 1000.0f;
    FLinearColor FogColor = FLinearColor::Gray;

    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Clear:
            FogDensity = 0.005f;
            FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
            break;
        case EWorld_WeatherType::Fog:
            FogDensity = 0.1f * CurrentWeather.Intensity;
            FogHeight = 500.0f;
            FogColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
            break;
        case EWorld_WeatherType::Storm:
            FogDensity = 0.05f;
            FogColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
            break;
        case EWorld_WeatherType::Sandstorm:
            FogDensity = 0.08f * CurrentWeather.Intensity;
            FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
            break;
        default:
            FogDensity = 0.02f;
            break;
    }

    FogActor->GetComponent()->SetFogDensity(FogDensity);
    FogActor->GetComponent()->SetFogHeightFalloff(FogHeight);
    FogActor->GetComponent()->SetFogInscatteringColor(FogColor);
}

void AWorld_WeatherSystem::UpdateLightingSettings()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }

    float LightIntensity = 3.0f;
    FLinearColor LightColor = FLinearColor::White;

    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Clear:
            LightIntensity = 3.0f;
            LightColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
            break;
        case EWorld_WeatherType::Cloudy:
            LightIntensity = 2.0f;
            LightColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            break;
        case EWorld_WeatherType::Storm:
            LightIntensity = 1.0f;
            LightColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
            break;
        case EWorld_WeatherType::Rain:
            LightIntensity = 1.5f;
            LightColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
            break;
        case EWorld_WeatherType::Fog:
            LightIntensity = 1.2f;
            LightColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            break;
        case EWorld_WeatherType::Sandstorm:
            LightIntensity = 0.8f;
            LightColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            break;
        default:
            break;
    }

    SunLight->GetLightComponent()->SetIntensity(LightIntensity);
    SunLight->GetLightComponent()->SetLightColor(LightColor);
}

void AWorld_WeatherSystem::UpdateParticleEffects()
{
    // Deactivate all particles first
    RainParticles->Deactivate();
    SnowParticles->Deactivate();
    SandstormParticles->Deactivate();

    // Activate appropriate particle system
    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Rain:
        case EWorld_WeatherType::Storm:
            RainParticles->Activate();
            break;
        case EWorld_WeatherType::Snow:
            SnowParticles->Activate();
            break;
        case EWorld_WeatherType::Sandstorm:
            SandstormParticles->Activate();
            break;
        default:
            break;
    }
}

void AWorld_WeatherSystem::UpdateAudioEffects()
{
    if (!WeatherAudio)
    {
        return;
    }

    WeatherAudio->Stop();

    USoundCue* SoundToPlay = nullptr;
    float Volume = CurrentWeather.Intensity;

    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Rain:
            SoundToPlay = RainSound;
            break;
        case EWorld_WeatherType::Storm:
            SoundToPlay = StormSound;
            Volume *= 1.5f;
            break;
        case EWorld_WeatherType::Sandstorm:
        case EWorld_WeatherType::Snow:
            SoundToPlay = WindSound;
            break;
        default:
            break;
    }

    if (SoundToPlay)
    {
        WeatherAudio->SetSound(SoundToPlay);
        WeatherAudio->SetVolumeMultiplier(Volume);
        WeatherAudio->Play();
    }
}

void AWorld_WeatherSystem::GenerateRandomWeather()
{
    FWorld_WeatherData NewWeather = GenerateWeatherForClimate();
    StartWeatherTransition(NewWeather);
}

bool AWorld_WeatherSystem::IsStormActive() const
{
    return CurrentWeather.WeatherType == EWorld_WeatherType::Storm ||
           CurrentWeather.WeatherType == EWorld_WeatherType::Sandstorm;
}

float AWorld_WeatherSystem::GetVisibilityMultiplier() const
{
    return CurrentWeather.Visibility;
}

FVector AWorld_WeatherSystem::GetWindDirection() const
{
    // Generate wind direction based on weather type
    FVector WindDir = FVector::ForwardVector;
    
    switch (CurrentWeather.WeatherType)
    {
        case EWorld_WeatherType::Storm:
        case EWorld_WeatherType::Sandstorm:
            // Variable wind direction for storms
            WindDir = FVector(
                FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f),
                FMath::Cos(GetWorld()->GetTimeSeconds() * 0.3f),
                0.0f
            ).GetSafeNormal();
            break;
        default:
            WindDir = FVector(1.0f, 0.2f, 0.0f).GetSafeNormal();
            break;
    }

    return WindDir;
}

float AWorld_WeatherSystem::GetWindStrength() const
{
    return CurrentWeather.WindSpeed * CurrentWeather.Intensity;
}

void AWorld_WeatherSystem::UpdateWeatherCycle(float DeltaTime)
{
    WeatherTimer += DeltaTime;
    
    if (WeatherTimer >= WeatherCycleDuration)
    {
        WeatherTimer = 0.0f;
        GenerateRandomWeather();
    }
}

void AWorld_WeatherSystem::ProcessWeatherTransition(float DeltaTime)
{
    TransitionTimer += DeltaTime * WeatherTransitionSpeed;
    
    if (TransitionTimer >= 1.0f)
    {
        // Transition complete
        CurrentWeather = TargetWeather;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        
        ApplyWeatherEffects();
    }
    else
    {
        // Interpolate weather values
        float Alpha = TransitionTimer;
        
        CurrentWeather.Intensity = FMath::Lerp(CurrentWeather.Intensity, TargetWeather.Intensity, Alpha);
        CurrentWeather.WindSpeed = FMath::Lerp(CurrentWeather.WindSpeed, TargetWeather.WindSpeed, Alpha);
        CurrentWeather.Humidity = FMath::Lerp(CurrentWeather.Humidity, TargetWeather.Humidity, Alpha);
        CurrentWeather.Temperature = FMath::Lerp(CurrentWeather.Temperature, TargetWeather.Temperature, Alpha);
        CurrentWeather.Visibility = FMath::Lerp(CurrentWeather.Visibility, TargetWeather.Visibility, Alpha);
        
        // Update weather type when halfway through transition
        if (Alpha >= 0.5f && CurrentWeather.WeatherType != TargetWeather.WeatherType)
        {
            CurrentWeather.WeatherType = TargetWeather.WeatherType;
            UpdateParticleEffects();
            UpdateAudioEffects();
        }
        
        // Update visual effects during transition
        UpdateFogSettings();
        UpdateLightingSettings();
    }
}

FWorld_WeatherData AWorld_WeatherSystem::GenerateWeatherForClimate() const
{
    FWorld_WeatherData NewWeather;
    
    // Generate weather based on climate zone
    TArray<EWorld_WeatherType> PossibleWeather;
    
    switch (ClimateZone)
    {
        case EWorld_ClimateZone::Tropical:
            PossibleWeather.Add(EWorld_WeatherType::Clear);
            PossibleWeather.Add(EWorld_WeatherType::Cloudy);
            PossibleWeather.Add(EWorld_WeatherType::Rain);
            PossibleWeather.Add(EWorld_WeatherType::Storm);
            break;
        case EWorld_ClimateZone::Temperate:
            PossibleWeather.Add(EWorld_WeatherType::Clear);
            PossibleWeather.Add(EWorld_WeatherType::Cloudy);
            PossibleWeather.Add(EWorld_WeatherType::Rain);
            PossibleWeather.Add(EWorld_WeatherType::Fog);
            break;
        case EWorld_ClimateZone::Arid:
            PossibleWeather.Add(EWorld_WeatherType::Clear);
            PossibleWeather.Add(EWorld_WeatherType::Sandstorm);
            break;
        case EWorld_ClimateZone::Polar:
            PossibleWeather.Add(EWorld_WeatherType::Cloudy);
            PossibleWeather.Add(EWorld_WeatherType::Snow);
            PossibleWeather.Add(EWorld_WeatherType::Fog);
            break;
        case EWorld_ClimateZone::Mountain:
            PossibleWeather.Add(EWorld_WeatherType::Clear);
            PossibleWeather.Add(EWorld_WeatherType::Cloudy);
            PossibleWeather.Add(EWorld_WeatherType::Fog);
            PossibleWeather.Add(EWorld_WeatherType::Snow);
            break;
    }
    
    if (PossibleWeather.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, PossibleWeather.Num() - 1);
        NewWeather.WeatherType = PossibleWeather[RandomIndex];
    }
    
    // Generate random intensity and other parameters
    NewWeather.Intensity = FMath::RandRange(0.3f, 1.0f);
    
    return NewWeather;
}

void AWorld_WeatherSystem::ApplyWeatherEffects()
{
    UpdateFogSettings();
    UpdateLightingSettings();
    UpdateParticleEffects();
    UpdateAudioEffects();
}

void AWorld_WeatherSystem::CleanupWeatherEffects()
{
    if (RainParticles)
    {
        RainParticles->Deactivate();
    }
    
    if (SnowParticles)
    {
        SnowParticles->Deactivate();
    }
    
    if (SandstormParticles)
    {
        SandstormParticles->Deactivate();
    }
    
    if (WeatherAudio)
    {
        WeatherAudio->Stop();
    }
}
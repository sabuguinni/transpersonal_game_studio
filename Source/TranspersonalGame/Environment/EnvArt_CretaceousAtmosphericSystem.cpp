#include "EnvArt_CretaceousAtmosphericSystem.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"

AEnvArt_CretaceousAtmosphericSystem::AEnvArt_CretaceousAtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle system components
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticleComponent"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = true;

    PollenParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("PollenParticleComponent"));
    PollenParticleComponent->SetupAttachment(RootComponent);
    PollenParticleComponent->bAutoActivate = true;

    SporeParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SporeParticleComponent"));
    SporeParticleComponent->SetupAttachment(RootComponent);
    SporeParticleComponent->bAutoActivate = true;

    // Create audio components
    WindAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudioComponent"));
    WindAudioComponent->SetupAttachment(RootComponent);
    WindAudioComponent->bAutoActivate = true;

    BirdCallsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BirdCallsAudioComponent"));
    BirdCallsAudioComponent->SetupAttachment(RootComponent);
    BirdCallsAudioComponent->bAutoActivate = true;

    DistantRoarsAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DistantRoarsAudioComponent"));
    DistantRoarsAudioComponent->SetupAttachment(RootComponent);
    DistantRoarsAudioComponent->bAutoActivate = true;

    // Initialize default values
    TimeOfDay = 12.0f; // Noon
    bAutoAdvanceTime = true;
    TimeAdvanceSpeed = 1.0f;
    WeatherIntensity = 0.5f;
    bIsStormy = false;
    HumidityLevel = 0.6f;
    LastTimeUpdate = 0.0f;
    bSystemInitialized = false;

    // Initialize atmospheric data with Cretaceous period appropriate values
    ParticleData.ParticleIntensity = 0.7f;
    ParticleData.WindStrength = 0.4f;
    ParticleData.WindDirection = FVector(1.0f, 0.3f, 0.1f);

    LightingData.SunAngle = 45.0f;
    LightingData.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    LightingData.SunIntensity = 10.0f;
    LightingData.AtmosphericFogDensity = 0.03f;
    LightingData.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    LightingData.VolumetricFogExtinction = 0.15f;

    AmbientSoundData.AmbientVolume = 0.6f;
    AmbientSoundData.SoundRange = 8000.0f;
}

void AEnvArt_CretaceousAtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the atmospheric system
    SetGoldenHourLighting();
    UpdateAtmosphericParticles(ParticleData.ParticleIntensity);
    UpdateAmbientSounds(AmbientSoundData.AmbientVolume);

    bSystemInitialized = true;

    UE_LOG(LogTemp, Warning, TEXT("Cretaceous Atmospheric System initialized at location: %s"), 
           *GetActorLocation().ToString());
}

void AEnvArt_CretaceousAtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSystemInitialized)
        return;

    // Auto-advance time if enabled
    if (bAutoAdvanceTime)
    {
        TimeOfDay += (DeltaTime * TimeAdvanceSpeed) / 3600.0f; // Convert seconds to hours
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay = 0.0f;
        }
    }

    // Update lighting based on time of day
    float TimeSinceLastUpdate = GetWorld()->GetTimeSeconds() - LastTimeUpdate;
    if (TimeSinceLastUpdate >= 1.0f) // Update every second
    {
        UpdateLightingBasedOnTime();
        UpdateParticleSystemsBasedOnWeather();
        UpdateAmbientAudioBasedOnConditions();
        LastTimeUpdate = GetWorld()->GetTimeSeconds();
    }
}

void AEnvArt_CretaceousAtmosphericSystem::SetGoldenHourLighting()
{
    // Golden hour lighting (warm, low angle sun)
    LightingData.SunAngle = 20.0f;
    LightingData.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    LightingData.SunIntensity = 6.0f;
    LightingData.AtmosphericFogDensity = 0.04f;
    LightingData.FogColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Applied golden hour lighting"));
}

void AEnvArt_CretaceousAtmosphericSystem::SetMidDayLighting()
{
    // Midday lighting (bright, high angle sun)
    LightingData.SunAngle = 80.0f;
    LightingData.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    LightingData.SunIntensity = 12.0f;
    LightingData.AtmosphericFogDensity = 0.02f;
    LightingData.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Applied midday lighting"));
}

void AEnvArt_CretaceousAtmosphericSystem::SetDuskLighting()
{
    // Dusk lighting (cool, low angle sun)
    LightingData.SunAngle = 10.0f;
    LightingData.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    LightingData.SunIntensity = 3.0f;
    LightingData.AtmosphericFogDensity = 0.05f;
    LightingData.FogColor = FLinearColor(0.9f, 0.7f, 0.8f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("Applied dusk lighting"));
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateAtmosphericParticles(float Intensity)
{
    if (!DustParticleComponent || !PollenParticleComponent || !SporeParticleComponent)
        return;

    // Update dust particles
    DustParticleComponent->SetFloatParameter(TEXT("SpawnRate"), Intensity * 50.0f);
    DustParticleComponent->SetVectorParameter(TEXT("WindDirection"), ParticleData.WindDirection);

    // Update pollen particles (more active during certain times)
    float PollenIntensity = Intensity * (HumidityLevel > 0.5f ? 1.5f : 0.8f);
    PollenParticleComponent->SetFloatParameter(TEXT("SpawnRate"), PollenIntensity * 30.0f);

    // Update spore particles (more active in humid conditions)
    float SporeIntensity = Intensity * HumidityLevel;
    SporeParticleComponent->SetFloatParameter(TEXT("SpawnRate"), SporeIntensity * 20.0f);

    UE_LOG(LogTemp, Log, TEXT("Updated atmospheric particles - Intensity: %f"), Intensity);
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateAmbientSounds(float Volume)
{
    if (!WindAudioComponent || !BirdCallsAudioComponent || !DistantRoarsAudioComponent)
        return;

    // Update wind sound based on weather
    float WindVolume = Volume * (bIsStormy ? 1.5f : 0.8f);
    WindAudioComponent->SetVolumeMultiplier(WindVolume);

    // Update bird calls (quieter during storms or at night)
    float BirdVolume = Volume * (bIsStormy ? 0.3f : 1.0f);
    if (TimeOfDay < 6.0f || TimeOfDay > 20.0f) // Night time
    {
        BirdVolume *= 0.2f;
    }
    BirdCallsAudioComponent->SetVolumeMultiplier(BirdVolume);

    // Update distant roars (more prominent at dawn/dusk)
    float RoarVolume = Volume * 0.7f;
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f))
    {
        RoarVolume *= 1.8f; // Dawn/dusk activity
    }
    DistantRoarsAudioComponent->SetVolumeMultiplier(RoarVolume);

    UE_LOG(LogTemp, Log, TEXT("Updated ambient sounds - Volume: %f"), Volume);
}

void AEnvArt_CretaceousAtmosphericSystem::SetWeatherCondition(bool bStormy, float Intensity)
{
    bIsStormy = bStormy;
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Update humidity based on weather
    HumidityLevel = bIsStormy ? FMath::Clamp(HumidityLevel + 0.3f, 0.0f, 1.0f) : 
                               FMath::Clamp(HumidityLevel - 0.1f, 0.2f, 1.0f);

    // Update atmospheric effects
    UpdateAtmosphericParticles(ParticleData.ParticleIntensity * (bIsStormy ? 1.5f : 1.0f));
    UpdateAmbientSounds(AmbientSoundData.AmbientVolume);

    UE_LOG(LogTemp, Warning, TEXT("Weather condition changed - Stormy: %s, Intensity: %f"), 
           bIsStormy ? TEXT("True") : TEXT("False"), Intensity);
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateLightingBasedOnTime();

    UE_LOG(LogTemp, Log, TEXT("Time of day updated to: %f"), TimeOfDay);
}

FLinearColor AEnvArt_CretaceousAtmosphericSystem::GetCurrentSkyColor() const
{
    return CalculateSkyColorForTime(TimeOfDay);
}

float AEnvArt_CretaceousAtmosphericSystem::GetCurrentLightIntensity() const
{
    return CalculateLightIntensityForTime(TimeOfDay);
}

void AEnvArt_CretaceousAtmosphericSystem::SpawnAtmosphericEffectAtLocation(const FVector& Location, float Radius)
{
    // Create temporary particle effect at specified location
    if (DustParticleComponent && DustParticleComponent->GetTemplate())
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DustParticleComponent->GetTemplate(),
            Location,
            FRotator::ZeroRotator,
            FVector(Radius / 100.0f),
            true
        );

        UE_LOG(LogTemp, Log, TEXT("Spawned atmospheric effect at location: %s with radius: %f"), 
               *Location.ToString(), Radius);
    }
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateLightingBasedOnTime()
{
    // Calculate lighting properties based on time of day
    FLinearColor NewSkyColor = CalculateSkyColorForTime(TimeOfDay);
    float NewLightIntensity = CalculateLightIntensityForTime(TimeOfDay);

    // Update lighting data
    LightingData.SunColor = NewSkyColor;
    LightingData.SunIntensity = NewLightIntensity;
    LightingData.SunAngle = ((TimeOfDay - 6.0f) / 12.0f) * 180.0f; // Sun arc from 6 AM to 6 PM

    // Update fog based on time and weather
    float FogDensityMultiplier = bIsStormy ? 1.5f : 1.0f;
    LightingData.AtmosphericFogDensity = 0.02f + (WeatherIntensity * 0.03f) * FogDensityMultiplier;
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateParticleSystemsBasedOnWeather()
{
    // Adjust particle intensity based on weather and time
    float BaseIntensity = ParticleData.ParticleIntensity;
    float WeatherMultiplier = bIsStormy ? 2.0f : 1.0f;
    float TimeMultiplier = (TimeOfDay >= 6.0f && TimeOfDay <= 18.0f) ? 1.0f : 0.5f; // Less particles at night

    float FinalIntensity = BaseIntensity * WeatherMultiplier * TimeMultiplier;
    UpdateAtmosphericParticles(FinalIntensity);

    // Update wind direction based on weather
    if (bIsStormy)
    {
        ParticleData.WindDirection = FVector(
            FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f),
            FMath::Cos(GetWorld()->GetTimeSeconds() * 0.3f),
            0.2f
        ).GetSafeNormal();
        ParticleData.WindStrength = 0.8f + (WeatherIntensity * 0.4f);
    }
    else
    {
        ParticleData.WindDirection = FVector(1.0f, 0.3f, 0.1f).GetSafeNormal();
        ParticleData.WindStrength = 0.3f + (WeatherIntensity * 0.2f);
    }
}

void AEnvArt_CretaceousAtmosphericSystem::UpdateAmbientAudioBasedOnConditions()
{
    // Calculate volume based on time, weather, and humidity
    float BaseVolume = AmbientSoundData.AmbientVolume;
    float TimeMultiplier = 1.0f;
    
    // Quieter at night
    if (TimeOfDay < 6.0f || TimeOfDay > 20.0f)
    {
        TimeMultiplier = 0.6f;
    }
    // More active at dawn/dusk
    else if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f))
    {
        TimeMultiplier = 1.3f;
    }

    float WeatherMultiplier = bIsStormy ? 1.2f : 1.0f;
    float FinalVolume = BaseVolume * TimeMultiplier * WeatherMultiplier;

    UpdateAmbientSounds(FinalVolume);
}

FLinearColor AEnvArt_CretaceousAtmosphericSystem::CalculateSkyColorForTime(float Time) const
{
    // Dawn (5-7 AM): Orange/Pink
    if (Time >= 5.0f && Time <= 7.0f)
    {
        float Alpha = (Time - 5.0f) / 2.0f;
        return FLinearColor::LerpUsingHSV(
            FLinearColor(1.0f, 0.6f, 0.4f, 1.0f), // Dawn orange
            FLinearColor(1.0f, 1.0f, 0.9f, 1.0f), // Morning yellow
            Alpha
        );
    }
    // Day (7 AM - 5 PM): Blue/White
    else if (Time >= 7.0f && Time <= 17.0f)
    {
        return FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Daylight blue-white
    }
    // Dusk (5-7 PM): Orange/Red
    else if (Time >= 17.0f && Time <= 19.0f)
    {
        float Alpha = (Time - 17.0f) / 2.0f;
        return FLinearColor::LerpUsingHSV(
            FLinearColor(1.0f, 1.0f, 0.9f, 1.0f), // Evening yellow
            FLinearColor(1.0f, 0.5f, 0.3f, 1.0f), // Dusk orange
            Alpha
        );
    }
    // Night (7 PM - 5 AM): Dark blue
    else
    {
        return FLinearColor(0.2f, 0.3f, 0.6f, 1.0f); // Night blue
    }
}

float AEnvArt_CretaceousAtmosphericSystem::CalculateLightIntensityForTime(float Time) const
{
    // Dawn (5-7 AM): Gradual increase
    if (Time >= 5.0f && Time <= 7.0f)
    {
        float Alpha = (Time - 5.0f) / 2.0f;
        return FMath::Lerp(2.0f, 8.0f, Alpha);
    }
    // Day (7 AM - 5 PM): Full intensity
    else if (Time >= 7.0f && Time <= 17.0f)
    {
        return 10.0f + (WeatherIntensity * 2.0f);
    }
    // Dusk (5-7 PM): Gradual decrease
    else if (Time >= 17.0f && Time <= 19.0f)
    {
        float Alpha = (Time - 17.0f) / 2.0f;
        return FMath::Lerp(8.0f, 1.0f, Alpha);
    }
    // Night (7 PM - 5 AM): Minimal light
    else
    {
        return 0.5f;
    }
}

FVector AEnvArt_CretaceousAtmosphericSystem::CalculateWindDirectionForWeather() const
{
    if (bIsStormy)
    {
        // Variable wind during storms
        float Time = GetWorld()->GetTimeSeconds();
        return FVector(
            FMath::Sin(Time * 0.7f) * 0.8f,
            FMath::Cos(Time * 0.5f) * 0.6f,
            FMath::Sin(Time * 0.3f) * 0.3f
        ).GetSafeNormal();
    }
    else
    {
        // Steady wind in calm weather
        return FVector(1.0f, 0.2f, 0.1f).GetSafeNormal();
    }
}
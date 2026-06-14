#include "EnvArt_AtmosphericManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create atmospheric particle system component
    AtmosphericParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AtmosphericParticles"));
    AtmosphericParticles->SetupAttachment(RootComponent);

    // Create ambient audio component
    AmbientAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudioComponent"));
    AmbientAudioComponent->SetupAttachment(RootComponent);
    AmbientAudioComponent->bAutoActivate = true;

    // Initialize default atmospheric settings
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AtmosphericSettings.SunIntensity = 3.5f;
    AtmosphericSettings.SunElevation = -30.0f;
    AtmosphericSettings.SunAzimuth = 45.0f;
    AtmosphericSettings.FogDensity = 0.1f;
    AtmosphericSettings.FogHeight = 200.0f;
    AtmosphericSettings.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    AtmosphericSettings.ParticleDensity = 50.0f;
    AtmosphericSettings.WindStrength = 1.0f;

    CurrentTimeOfDay = 12.0f; // Noon
    CurrentWeatherIntensity = 0.5f; // Moderate weather
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();

    // Find the directional light in the scene
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Apply initial golden hour lighting
    ApplyGoldenHourLighting();

    // Start ambient audio if sound cue is assigned
    if (ForestAmbientSound && AmbientAudioComponent)
    {
        AmbientAudioComponent->SetSound(ForestAmbientSound);
        AmbientAudioComponent->Play();
    }
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateLightingBasedOnTime();
    UpdateFogBasedOnWeather();
    UpdateParticleEffects();
    UpdateAmbientAudio();
}

void AEnvArt_AtmosphericManager::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
    UpdateLightingBasedOnTime();
}

void AEnvArt_AtmosphericManager::SetWeatherIntensity(float Intensity)
{
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateFogBasedOnWeather();
}

void AEnvArt_AtmosphericManager::SetFogDensity(float Density)
{
    AtmosphericSettings.FogDensity = FMath::Clamp(Density, 0.0f, 1.0f);
    UpdateFogBasedOnWeather();
}

void AEnvArt_AtmosphericManager::SetWindStrength(float Strength)
{
    AtmosphericSettings.WindStrength = FMath::Clamp(Strength, 0.0f, 5.0f);
    UpdateParticleEffects();
}

void AEnvArt_AtmosphericManager::ApplyGoldenHourLighting()
{
    if (SunLight)
    {
        // Set golden hour rotation (low angle, warm light)
        FRotator GoldenHourRotation(AtmosphericSettings.SunElevation, AtmosphericSettings.SunAzimuth, 0.0f);
        SunLight->SetActorRotation(GoldenHourRotation);

        // Configure light properties
        UDirectionalLightComponent* LightComponent = SunLight->GetDirectionalLightComponent();
        if (LightComponent)
        {
            LightComponent->SetLightColor(AtmosphericSettings.SunColor);
            LightComponent->SetIntensity(AtmosphericSettings.SunIntensity);
            LightComponent->SetCastVolumetricShadow(true);
            LightComponent->SetVolumetricScatteringIntensity(1.5f);
        }
    }
}

void AEnvArt_AtmosphericManager::ApplyMidnightLighting()
{
    if (SunLight)
    {
        // Set midnight rotation (very low angle, cool light)
        FRotator MidnightRotation(-80.0f, 180.0f, 0.0f);
        SunLight->SetActorRotation(MidnightRotation);

        // Configure night light properties
        UDirectionalLightComponent* LightComponent = SunLight->GetDirectionalLightComponent();
        if (LightComponent)
        {
            FLinearColor MoonColor(0.3f, 0.4f, 0.6f, 1.0f);
            LightComponent->SetLightColor(MoonColor);
            LightComponent->SetIntensity(0.5f);
        }
    }
}

void AEnvArt_AtmosphericManager::CreateVolumetricFogZone(FVector Location, FVector Scale)
{
    // Spawn volumetric cloud actor for fog effect
    AVolumetricCloud* FogActor = GetWorld()->SpawnActor<AVolumetricCloud>(Location, FRotator::ZeroRotator);
    if (FogActor)
    {
        FogActor->SetActorScale3D(Scale);
        VolumetricFogActor = FogActor;

        UVolumetricCloudComponent* CloudComponent = FogActor->GetVolumetricCloudComponent();
        if (CloudComponent)
        {
            CloudComponent->SetLayerBottomAltitude(0.1f);
            CloudComponent->SetLayerHeight(AtmosphericSettings.FogHeight / 100.0f);
            CloudComponent->SetAlbedo(AtmosphericSettings.FogColor);
        }
    }
}

void AEnvArt_AtmosphericManager::SpawnAtmosphericParticles(FVector Location, float Density)
{
    if (AtmosphericParticles)
    {
        AtmosphericParticles->SetWorldLocation(Location);
        // Configure particle density and wind effects
        // Note: Specific particle system configuration would depend on the assigned particle system asset
    }
}

void AEnvArt_AtmosphericManager::UpdateLightingBasedOnTime()
{
    if (!SunLight) return;

    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f - 90.0f; // 6 AM = 0 degrees, 12 PM = 90 degrees
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;

    // Update sun rotation
    FRotator NewRotation(SunElevation, AtmosphericSettings.SunAzimuth, 0.0f);
    SunLight->SetActorRotation(NewRotation);

    // Update light color and intensity based on time
    UDirectionalLightComponent* LightComponent = SunLight->GetDirectionalLightComponent();
    if (LightComponent)
    {
        if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 18.0f) // Daytime
        {
            float DayProgress = (CurrentTimeOfDay - 6.0f) / 12.0f;
            float WarmnessFactor = 1.0f - FMath::Abs(DayProgress - 0.5f) * 2.0f; // Peak warmth at noon
            
            FLinearColor DayColor = FLinearColor::LerpUsingHSV(
                FLinearColor(1.0f, 0.6f, 0.4f, 1.0f), // Warm sunrise/sunset
                FLinearColor(1.0f, 1.0f, 0.9f, 1.0f), // Cool noon
                WarmnessFactor
            );
            
            LightComponent->SetLightColor(DayColor);
            LightComponent->SetIntensity(FMath::Lerp(1.0f, 5.0f, WarmnessFactor));
        }
        else // Nighttime
        {
            FLinearColor NightColor(0.2f, 0.3f, 0.5f, 1.0f);
            LightComponent->SetLightColor(NightColor);
            LightComponent->SetIntensity(0.1f);
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateFogBasedOnWeather()
{
    // Update fog density based on weather intensity and time of day
    float EffectiveFogDensity = AtmosphericSettings.FogDensity * CurrentWeatherIntensity;
    
    // Increase fog during early morning and evening
    if (CurrentTimeOfDay < 8.0f || CurrentTimeOfDay > 18.0f)
    {
        EffectiveFogDensity *= 1.5f;
    }

    // Apply fog settings to volumetric fog actor if available
    if (VolumetricFogActor)
    {
        AVolumetricCloud* CloudActor = Cast<AVolumetricCloud>(VolumetricFogActor);
        if (CloudActor)
        {
            UVolumetricCloudComponent* CloudComponent = CloudActor->GetVolumetricCloudComponent();
            if (CloudComponent)
            {
                CloudComponent->SetLayerHeight(AtmosphericSettings.FogHeight / 100.0f * EffectiveFogDensity);
            }
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateParticleEffects()
{
    if (AtmosphericParticles)
    {
        // Update particle effects based on wind strength and weather
        float EffectiveParticleDensity = AtmosphericSettings.ParticleDensity * CurrentWeatherIntensity;
        
        // Configure particle system parameters
        // Note: Specific implementation would depend on the particle system asset
    }
}

void AEnvArt_AtmosphericManager::UpdateAmbientAudio()
{
    if (AmbientAudioComponent)
    {
        // Adjust ambient audio volume based on weather and time
        float VolumeMultiplier = 0.5f + (CurrentWeatherIntensity * 0.5f);
        
        // Quieter at night
        if (CurrentTimeOfDay < 6.0f || CurrentTimeOfDay > 20.0f)
        {
            VolumeMultiplier *= 0.6f;
        }
        
        AmbientAudioComponent->SetVolumeMultiplier(VolumeMultiplier);
        
        // Switch between day and night ambient sounds
        if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 20.0f)
        {
            if (ForestAmbientSound && AmbientAudioComponent->GetSound() != ForestAmbientSound)
            {
                AmbientAudioComponent->SetSound(ForestAmbientSound);
                AmbientAudioComponent->Play();
            }
        }
    }
}
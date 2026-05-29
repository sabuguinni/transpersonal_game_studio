#include "EnvArt_AtmosphericManager.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
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
    SunLight->SetCastVolumetricShadow(true);

    // Create fog component
    AtmosphericFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphericFog"));
    AtmosphericFog->SetupAttachment(RootComponent);
    AtmosphericFog->SetFogDensity(0.02f);
    AtmosphericFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));

    // Create particle system for dust/pollen
    DustParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    DustParticles->SetupAttachment(RootComponent);
    DustParticles->SetAutoActivate(true);

    // Create ambient audio
    AmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("AmbientAudio"));
    AmbientAudio->SetupAttachment(RootComponent);
    AmbientAudio->SetVolumeMultiplier(0.5f);
    AmbientAudio->bAutoActivate = true;

    // Initialize default settings
    AtmosphericSettings.SunColor = FLinearColor::White;
    AtmosphericSettings.SunIntensity = 3.0f;
    AtmosphericSettings.SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    AtmosphericSettings.ParticleSpawnRate = 10.0f;
    AtmosphericSettings.AmbientVolume = 0.5f;
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial atmosphere settings
    LoadAtmospherePreset(CurrentAtmosphereType);
    ApplyGoldenHourLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphericManager: Initialized with atmosphere type %d"), (int32)CurrentAtmosphereType);
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    TimeOfDay += (DeltaTime / DayDuration) * 24.0f;
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateParticleEffects();
}

void AEnvArt_AtmosphericManager::SetAtmosphereType(EEnvArt_AtmosphereType NewType)
{
    CurrentAtmosphereType = NewType;
    LoadAtmospherePreset(NewType);
    
    UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphericManager: Changed atmosphere to type %d"), (int32)NewType);
}

void AEnvArt_AtmosphericManager::UpdateTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
}

void AEnvArt_AtmosphericManager::ApplyGoldenHourLighting()
{
    // Golden hour is around 6-8 AM and 6-8 PM
    float GoldenHourIntensity = 2.5f;
    FLinearColor GoldenColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    if (SunLight)
    {
        SunLight->SetIntensity(GoldenHourIntensity);
        SunLight->SetLightColor(GoldenColor);
        SunLight->SetRelativeRotation(FRotator(-15.0f, 45.0f, 0.0f));
    }
    
    // Update fog for golden hour
    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogInscatteringColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
        AtmosphericFog->SetFogDensity(0.015f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphericManager: Applied golden hour lighting"));
}

void AEnvArt_AtmosphericManager::SpawnVolumetricFog(FVector Location, float Radius)
{
    // Create localized fog effect
    if (AtmosphericFog)
    {
        AtmosphericFog->SetWorldLocation(Location);
        AtmosphericFog->SetFogDensity(AtmosphericSettings.FogDensity * 1.5f);
        
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphericManager: Spawned volumetric fog at location %s"), *Location.ToString());
    }
}

void AEnvArt_AtmosphericManager::AddAmbientParticles(FVector Location, float Intensity)
{
    if (DustParticles)
    {
        DustParticles->SetWorldLocation(Location);
        // Adjust particle spawn rate based on intensity
        float NewSpawnRate = AtmosphericSettings.ParticleSpawnRate * Intensity;
        
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphericManager: Added ambient particles at %s with intensity %f"), *Location.ToString(), Intensity);
    }
}

void AEnvArt_AtmosphericManager::UpdateSunPosition()
{
    if (SunLight)
    {
        // Calculate sun position based on time of day
        float SunAngle = (TimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at sunrise
        FRotator NewRotation = FRotator(SunAngle, AtmosphericSettings.SunRotation.Yaw, 0.0f);
        SunLight->SetRelativeRotation(NewRotation);
        
        // Adjust intensity based on sun height
        float IntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.1f, 1.0f);
        SunLight->SetIntensity(AtmosphericSettings.SunIntensity * IntensityMultiplier);
    }
}

void AEnvArt_AtmosphericManager::UpdateFogSettings()
{
    if (AtmosphericFog)
    {
        // Adjust fog based on time of day
        float FogMultiplier = 1.0f;
        if (TimeOfDay >= 6.0f && TimeOfDay <= 8.0f) // Morning fog
        {
            FogMultiplier = 1.5f;
        }
        else if (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f) // Evening fog
        {
            FogMultiplier = 1.3f;
        }
        
        AtmosphericFog->SetFogDensity(AtmosphericSettings.FogDensity * FogMultiplier);
    }
}

void AEnvArt_AtmosphericManager::UpdateParticleEffects()
{
    if (DustParticles)
    {
        // Adjust particle effects based on atmosphere type and time
        float ParticleMultiplier = 1.0f;
        
        switch (CurrentAtmosphereType)
        {
            case EEnvArt_AtmosphereType::Forest:
                ParticleMultiplier = 0.8f; // Less dust in forest
                break;
            case EEnvArt_AtmosphereType::Desert:
                ParticleMultiplier = 2.0f; // More dust in desert
                break;
            case EEnvArt_AtmosphereType::Savanna:
                ParticleMultiplier = 1.2f; // Moderate dust
                break;
            case EEnvArt_AtmosphereType::Swamp:
                ParticleMultiplier = 0.5f; // Humid, less particles
                break;
            case EEnvArt_AtmosphereType::Mountain:
                ParticleMultiplier = 1.5f; // Wind-blown particles
                break;
        }
        
        // Apply time-based variation (more particles during day)
        if (TimeOfDay >= 10.0f && TimeOfDay <= 16.0f)
        {
            ParticleMultiplier *= 1.3f;
        }
    }
}

void AEnvArt_AtmosphericManager::LoadAtmospherePreset(EEnvArt_AtmosphereType AtmosphereType)
{
    switch (AtmosphereType)
    {
        case EEnvArt_AtmosphereType::Forest:
            AtmosphericSettings.SunColor = FLinearColor(0.9f, 1.0f, 0.8f, 1.0f);
            AtmosphericSettings.SunIntensity = 2.5f;
            AtmosphericSettings.FogColor = FLinearColor(0.7f, 0.9f, 0.7f, 1.0f);
            AtmosphericSettings.FogDensity = 0.025f;
            break;
            
        case EEnvArt_AtmosphereType::Desert:
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            AtmosphericSettings.SunIntensity = 4.0f;
            AtmosphericSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            AtmosphericSettings.FogDensity = 0.01f;
            break;
            
        case EEnvArt_AtmosphereType::Savanna:
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            AtmosphericSettings.SunIntensity = 3.5f;
            AtmosphericSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
            AtmosphericSettings.FogDensity = 0.015f;
            break;
            
        case EEnvArt_AtmosphereType::Swamp:
            AtmosphericSettings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);
            AtmosphericSettings.SunIntensity = 2.0f;
            AtmosphericSettings.FogColor = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);
            AtmosphericSettings.FogDensity = 0.04f;
            break;
            
        case EEnvArt_AtmosphereType::Mountain:
            AtmosphericSettings.SunColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
            AtmosphericSettings.SunIntensity = 3.8f;
            AtmosphericSettings.FogColor = FLinearColor(0.8f, 0.8f, 1.0f, 1.0f);
            AtmosphericSettings.FogDensity = 0.008f;
            break;
    }
    
    // Apply the loaded settings
    if (SunLight)
    {
        SunLight->SetLightColor(AtmosphericSettings.SunColor);
        SunLight->SetIntensity(AtmosphericSettings.SunIntensity);
    }
    
    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogInscatteringColor(AtmosphericSettings.FogColor);
        AtmosphericFog->SetFogDensity(AtmosphericSettings.FogDensity);
    }
    
    if (AmbientAudio)
    {
        AmbientAudio->SetVolumeMultiplier(AtmosphericSettings.AmbientVolume);
    }
}
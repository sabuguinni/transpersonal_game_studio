#include "EnvArt_VolcanicAtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

AEnvArt_VolcanicAtmosphereManager::AEnvArt_VolcanicAtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle system components
    AshParticleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("AshParticleSystem"));
    AshParticleSystem->SetupAttachment(RootComponent);
    AshParticleSystem->bAutoActivate = true;

    SteamVentSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("SteamVentSystem"));
    SteamVentSystem->SetupAttachment(RootComponent);
    SteamVentSystem->bAutoActivate = true;
    SteamVentSystem->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

    // Create audio components
    VolcanicAmbientAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("VolcanicAmbientAudio"));
    VolcanicAmbientAudio->SetupAttachment(RootComponent);
    VolcanicAmbientAudio->bAutoActivate = true;

    WindAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("WindAudio"));
    WindAudio->SetupAttachment(RootComponent);
    WindAudio->bAutoActivate = true;

    // Initialize settings
    CurrentTimeOfDay = TimeOfDay;
    AtmosphereUpdateTimer = 0.0f;
}

void AEnvArt_VolcanicAtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find world lighting actors
    FindWorldLightingActors();
    
    // Apply initial atmospheric settings
    UpdateAtmosphericLighting();
    UpdateFogSettings();
    UpdateParticleEffects();
}

void AEnvArt_VolcanicAtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update time of day if dynamic
    if (bDynamicTimeOfDay)
    {
        UpdateTimeOfDayEffects(DeltaTime);
    }

    // Update atmosphere periodically
    AtmosphereUpdateTimer += DeltaTime;
    if (AtmosphereUpdateTimer >= 1.0f) // Update every second
    {
        UpdateAtmosphericLighting();
        UpdateFogSettings();
        AtmosphereUpdateTimer = 0.0f;
    }
}

void AEnvArt_VolcanicAtmosphereManager::SetVolcanicIntensity(EEnvArt_VolcanicIntensity NewIntensity)
{
    VolcanicIntensity = NewIntensity;
    
    // Adjust atmospheric settings based on intensity
    switch (VolcanicIntensity)
    {
        case EEnvArt_VolcanicIntensity::Dormant:
            AtmosphericSettings.FogDensity = 0.01f;
            AtmosphericSettings.AshParticleIntensity = 0.2f;
            AtmosphericSettings.AmbientVolume = 0.3f;
            break;
            
        case EEnvArt_VolcanicIntensity::LowActivity:
            AtmosphericSettings.FogDensity = 0.015f;
            AtmosphericSettings.AshParticleIntensity = 0.5f;
            AtmosphericSettings.AmbientVolume = 0.5f;
            break;
            
        case EEnvArt_VolcanicIntensity::Moderate:
            AtmosphericSettings.FogDensity = 0.02f;
            AtmosphericSettings.AshParticleIntensity = 1.0f;
            AtmosphericSettings.AmbientVolume = 0.7f;
            break;
            
        case EEnvArt_VolcanicIntensity::HighActivity:
            AtmosphericSettings.FogDensity = 0.03f;
            AtmosphericSettings.AshParticleIntensity = 1.5f;
            AtmosphericSettings.AmbientVolume = 0.9f;
            break;
            
        case EEnvArt_VolcanicIntensity::Erupting:
            AtmosphericSettings.FogDensity = 0.05f;
            AtmosphericSettings.AshParticleIntensity = 2.0f;
            AtmosphericSettings.AmbientVolume = 1.0f;
            break;
    }
    
    // Apply changes immediately
    UpdateFogSettings();
    UpdateParticleEffects();
}

void AEnvArt_VolcanicAtmosphereManager::UpdateAtmosphericLighting()
{
    if (!SunLight)
    {
        FindWorldLightingActors();
        return;
    }

    UDirectionalLightComponent* LightComponent = SunLight->GetComponent<UDirectionalLightComponent>();
    if (LightComponent)
    {
        // Apply time-based lighting
        FLinearColor SunColor = GetSunColorForTime(CurrentTimeOfDay);
        float SunIntensity = GetSunIntensityForTime(CurrentTimeOfDay);
        FRotator SunRotation = GetSunRotationForTime(CurrentTimeOfDay);

        // Modify colors based on volcanic intensity
        float VolcanicTint = static_cast<float>(VolcanicIntensity) / 4.0f; // 0-1 range
        SunColor.R = FMath::Lerp(SunColor.R, 1.0f, VolcanicTint * 0.3f);
        SunColor.G = FMath::Lerp(SunColor.G, 0.6f, VolcanicTint * 0.4f);
        SunColor.B = FMath::Lerp(SunColor.B, 0.4f, VolcanicTint * 0.5f);

        LightComponent->SetLightColor(SunColor);
        LightComponent->SetIntensity(SunIntensity);
        SunLight->SetActorRotation(SunRotation);
    }
}

void AEnvArt_VolcanicAtmosphereManager::UpdateFogSettings()
{
    if (!WorldFog)
    {
        FindWorldLightingActors();
        return;
    }

    UExponentialHeightFogComponent* FogComponent = WorldFog->GetComponent<UExponentialHeightFogComponent>();
    if (FogComponent)
    {
        FogComponent->SetFogDensity(AtmosphericSettings.FogDensity);
        FogComponent->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
        FogComponent->SetFogInscatteringColor(AtmosphericSettings.FogColor);
        FogComponent->SetVolumetricFog(true);
        FogComponent->SetVolumetricFogScatteringDistribution(0.2f);
    }
}

void AEnvArt_VolcanicAtmosphereManager::UpdateParticleEffects()
{
    if (AshParticleSystem && AshParticleSystem->Template)
    {
        // Scale particle emission based on intensity
        float EmissionRate = AtmosphericSettings.AshParticleIntensity * 100.0f;
        AshParticleSystem->SetFloatParameter(TEXT("EmissionRate"), EmissionRate);
    }

    if (SteamVentSystem && SteamVentSystem->Template)
    {
        // Steam intensity based on volcanic activity
        float SteamIntensity = static_cast<float>(VolcanicIntensity) / 4.0f;
        SteamVentSystem->SetFloatParameter(TEXT("SteamIntensity"), SteamIntensity);
    }

    // Update audio volumes
    if (VolcanicAmbientAudio)
    {
        VolcanicAmbientAudio->SetVolumeMultiplier(AtmosphericSettings.AmbientVolume);
    }

    if (WindAudio)
    {
        float WindVolume = AtmosphericSettings.AmbientVolume * 0.6f;
        WindAudio->SetVolumeMultiplier(WindVolume);
    }
}

void AEnvArt_VolcanicAtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    CurrentTimeOfDay = TimeOfDay;
    UpdateAtmosphericLighting();
}

FLinearColor AEnvArt_VolcanicAtmosphereManager::CalculateAtmosphericColor(float Distance) const
{
    // Calculate atmospheric scattering based on distance and fog density
    float ScatteringFactor = 1.0f - FMath::Exp(-Distance * AtmosphericSettings.FogDensity * 0.001f);
    FLinearColor BaseColor = FLinearColor::White;
    
    return FLinearColor::LerpUsingHSV(BaseColor, AtmosphericSettings.FogColor, ScatteringFactor);
}

float AEnvArt_VolcanicAtmosphereManager::CalculateVisibilityAtDistance(float Distance) const
{
    // Calculate visibility reduction due to volcanic atmosphere
    float VisibilityFactor = FMath::Exp(-Distance * AtmosphericSettings.FogDensity * 0.002f);
    return FMath::Clamp(VisibilityFactor, 0.1f, 1.0f);
}

void AEnvArt_VolcanicAtmosphereManager::RefreshAtmosphere()
{
    FindWorldLightingActors();
    UpdateAtmosphericLighting();
    UpdateFogSettings();
    UpdateParticleEffects();
}

void AEnvArt_VolcanicAtmosphereManager::FindWorldLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    // Find exponential height fog
    if (!WorldFog)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            WorldFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void AEnvArt_VolcanicAtmosphereManager::UpdateTimeOfDayEffects(float DeltaTime)
{
    if (DayDurationMinutes <= 0.0f) return;

    // Advance time of day
    float TimeAdvancement = (DeltaTime / 60.0f) * (24.0f / DayDurationMinutes);
    CurrentTimeOfDay += TimeAdvancement;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

FLinearColor AEnvArt_VolcanicAtmosphereManager::GetSunColorForTime(float Time) const
{
    // Golden hour colors (sunrise/sunset)
    if ((Time >= 5.0f && Time <= 7.0f) || (Time >= 17.0f && Time <= 19.0f))
    {
        return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    }
    // Midday
    else if (Time >= 10.0f && Time <= 16.0f)
    {
        return FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    }
    // Night
    else if (Time <= 4.0f || Time >= 20.0f)
    {
        return FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    }
    // Transition periods
    else
    {
        return FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    }
}

float AEnvArt_VolcanicAtmosphereManager::GetSunIntensityForTime(float Time) const
{
    // Peak intensity at midday
    if (Time >= 11.0f && Time <= 15.0f)
    {
        return 4.0f;
    }
    // Golden hour intensity
    else if ((Time >= 6.0f && Time <= 8.0f) || (Time >= 16.0f && Time <= 18.0f))
    {
        return 3.5f;
    }
    // Night time
    else if (Time <= 5.0f || Time >= 19.0f)
    {
        return 0.5f;
    }
    // Transition periods
    else
    {
        return 2.5f;
    }
}

FRotator AEnvArt_VolcanicAtmosphereManager::GetSunRotationForTime(float Time) const
{
    // Calculate sun position based on time of day
    float SunAngle = ((Time - 6.0f) / 12.0f) * 180.0f; // 6 AM = horizon, 6 PM = horizon
    float Pitch = -90.0f + SunAngle; // -90 = straight down, 0 = horizon, 90 = straight up
    
    // Clamp pitch to reasonable values
    Pitch = FMath::Clamp(Pitch, -85.0f, 85.0f);
    
    return FRotator(Pitch, 45.0f, 0.0f); // Keep consistent yaw for lighting direction
}
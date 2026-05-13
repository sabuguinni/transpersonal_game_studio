#include "Light_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default time to noon
    CurrentTime.Hours = 12.0f;
    CurrentTime.Minutes = 0.0f;
    CurrentTime.TimeScale = 1.0f;

    // Initialize default weather
    CurrentWeather = ELight_WeatherState::Clear;
    TargetWeather = ELight_WeatherState::Clear;
    WeatherTransitionSpeed = 1.0f;

    // Initialize Cretaceous atmospheric settings
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    AtmosphereSettings.SunIntensity = 5.0f;
    AtmosphereSettings.RayleighScattering = 0.0331f;
    AtmosphereSettings.MieScattering = 0.003996f;
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();

    // Find atmospheric actors in the level
    FindAtmosphericActors();

    // Apply Cretaceous atmosphere correction
    SetCretaceousAtmosphere();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time
        CurrentTime.Minutes += DeltaTime * CurrentTime.TimeScale;
        if (CurrentTime.Minutes >= 60.0f)
        {
            CurrentTime.Minutes = 0.0f;
            CurrentTime.Hours += 1.0f;
            if (CurrentTime.Hours >= 24.0f)
            {
                CurrentTime.Hours = 0.0f;
            }
        }

        // Update sun position and lighting
        UpdateSunPosition();
        UpdateAtmosphericScattering();
    }

    // Update weather transitions
    if (bWeatherTransitioning)
    {
        TransitionWeather(DeltaTime);
    }

    // Update effects based on current weather
    UpdateWeatherEffects();
    UpdateFogSettings();
    UpdatePostProcessing();
}

void ALight_AtmosphereManager::SetTimeOfDay(float Hours, float Minutes)
{
    CurrentTime.Hours = FMath::Clamp(Hours, 0.0f, 23.99f);
    CurrentTime.Minutes = FMath::Clamp(Minutes, 0.0f, 59.99f);

    UpdateSunPosition();
    UpdateAtmosphericScattering();
}

void ALight_AtmosphereManager::SetWeather(ELight_WeatherState NewWeather)
{
    if (NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        bWeatherTransitioning = true;
        WeatherTransitionProgress = 0.0f;

        // Store current settings for smooth transition
        PreviousSettings = AtmosphereSettings;

        // Set target settings based on weather
        switch (NewWeather)
        {
        case ELight_WeatherState::Clear:
            TargetSettings.SunIntensity = 5.0f;
            TargetSettings.FogDensity = 0.02f;
            TargetSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
            break;
        case ELight_WeatherState::Overcast:
            TargetSettings.SunIntensity = 2.0f;
            TargetSettings.FogDensity = 0.05f;
            TargetSettings.FogColor = FLinearColor(0.4f, 0.4f, 0.4f, 1.0f);
            break;
        case ELight_WeatherState::Rainy:
            TargetSettings.SunIntensity = 1.0f;
            TargetSettings.FogDensity = 0.08f;
            TargetSettings.FogColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
            break;
        case ELight_WeatherState::Stormy:
            TargetSettings.SunIntensity = 0.5f;
            TargetSettings.FogDensity = 0.12f;
            TargetSettings.FogColor = FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
            break;
        case ELight_WeatherState::Foggy:
            TargetSettings.SunIntensity = 1.5f;
            TargetSettings.FogDensity = 0.15f;
            TargetSettings.FogColor = FLinearColor(0.6f, 0.6f, 0.6f, 1.0f);
            break;
        }
    }
}

void ALight_AtmosphereManager::ApplyAtmosphericCorrection()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(AtmosphereSettings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(AtmosphereSettings.SunColor);
    }

    if (SkyAtmosphere && SkyAtmosphere->GetComponent())
    {
        SkyAtmosphere->GetComponent()->SetRayleighScatteringScale(AtmosphereSettings.RayleighScattering);
        SkyAtmosphere->GetComponent()->SetMieScatteringScale(AtmosphereSettings.MieScattering);
    }

    if (VolumetricClouds)
    {
        // Hide volumetric clouds for clear Cretaceous atmosphere
        VolumetricClouds->SetActorHiddenInGame(true);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(AtmosphereSettings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(AtmosphereSettings.FogColor);
    }
}

void ALight_AtmosphereManager::SetCretaceousAtmosphere()
{
    // Set optimal Cretaceous period atmospheric conditions
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm amber sunlight
    AtmosphereSettings.SunIntensity = 5.0f; // Bright tropical sun
    AtmosphereSettings.RayleighScattering = 0.0331f; // Clear blue sky
    AtmosphereSettings.MieScattering = 0.003996f; // Minimal haze
    AtmosphereSettings.FogDensity = 0.02f; // Light atmospheric fog
    AtmosphereSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f); // Blue-tinted fog

    ApplyAtmosphericCorrection();

    UE_LOG(LogTemp, Warning, TEXT("Cretaceous atmosphere applied - tropical lighting restored"));
}

void ALight_AtmosphereManager::FindAtmosphericActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light (sun)
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }

        // Find sky atmosphere
        TArray<AActor*> SkyAtmospheres;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
        if (SkyAtmospheres.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
        }

        // Find volumetric clouds
        TArray<AActor*> VolumetricCloudActors;
        UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), VolumetricCloudActors);
        if (VolumetricCloudActors.Num() > 0)
        {
            VolumetricClouds = Cast<AVolumetricCloud>(VolumetricCloudActors[0]);
        }

        // Find height fog
        TArray<AActor*> HeightFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
        if (HeightFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
        }

        // Find post process volume
        TArray<AActor*> PostProcessVolumes;
        UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
        if (PostProcessVolumes.Num() > 0)
        {
            PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
        }
    }
}

void ALight_AtmosphereManager::FixApocalypticAtmosphere()
{
    SetCretaceousAtmosphere();
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    float SunAngle = CalculateSunAngle();
    
    // Calculate sun rotation based on time
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // East-West movement could be added here
    SunRotation.Roll = 0.0f;

    SunLight->SetActorRotation(SunRotation);

    // Update sun color and intensity based on angle
    if (UDirectionalLightComponent* LightComp = SunLight->GetLightComponent())
    {
        FLinearColor SunColor = CalculateSunColor(SunAngle);
        float SunIntensity = CalculateSunIntensity(SunAngle);

        LightComp->SetLightColor(SunColor);
        LightComp->SetIntensity(SunIntensity);
    }
}

void ALight_AtmosphereManager::UpdateAtmosphericScattering()
{
    if (!SkyAtmosphere || !SkyAtmosphere->GetComponent()) return;

    float SunAngle = CalculateSunAngle();
    
    // Adjust atmospheric scattering based on sun angle
    float ScatteringMultiplier = FMath::Lerp(0.5f, 1.0f, FMath::Abs(FMath::Sin(FMath::DegreesToRadians(SunAngle))));
    
    SkyAtmosphere->GetComponent()->SetRayleighScatteringScale(AtmosphereSettings.RayleighScattering * ScatteringMultiplier);
}

void ALight_AtmosphereManager::UpdateWeatherEffects()
{
    // Weather-specific effects can be implemented here
    // For now, we'll focus on basic atmospheric changes
}

void ALight_AtmosphereManager::UpdateFogSettings()
{
    if (!HeightFog || !HeightFog->GetComponent()) return;

    HeightFog->GetComponent()->SetFogDensity(AtmosphereSettings.FogDensity);
    HeightFog->GetComponent()->SetFogInscatteringColor(AtmosphereSettings.FogColor);
}

void ALight_AtmosphereManager::UpdatePostProcessing()
{
    // Post-processing effects based on time of day and weather
    // Can be expanded for more advanced visual effects
}

float ALight_AtmosphereManager::CalculateSunAngle() const
{
    float TotalMinutes = CurrentTime.Hours * 60.0f + CurrentTime.Minutes;
    float DayProgress = TotalMinutes / (24.0f * 60.0f); // 0-1 through the day
    
    // Sun angle from -90 (sunrise) to 90 (sunset), with 0 at noon
    return FMath::Lerp(-90.0f, 90.0f, DayProgress);
}

FLinearColor ALight_AtmosphereManager::CalculateSunColor(float SunAngle) const
{
    float AbsAngle = FMath::Abs(SunAngle);
    
    if (AbsAngle > 85.0f) // Sunrise/sunset
    {
        return FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Orange-red
    }
    else if (AbsAngle > 70.0f) // Early morning/late afternoon
    {
        return FLinearColor(1.0f, 0.9f, 0.7f, 1.0f); // Warm yellow
    }
    else // Midday
    {
        return AtmosphereSettings.SunColor; // Standard warm white
    }
}

float ALight_AtmosphereManager::CalculateSunIntensity(float SunAngle) const
{
    float AbsAngle = FMath::Abs(SunAngle);
    
    if (AbsAngle > 90.0f) // Night
    {
        return 0.1f; // Minimal moonlight
    }
    else if (AbsAngle > 85.0f) // Twilight
    {
        return FMath::Lerp(0.1f, AtmosphereSettings.SunIntensity * 0.3f, (90.0f - AbsAngle) / 5.0f);
    }
    else // Day
    {
        return FMath::Lerp(AtmosphereSettings.SunIntensity * 0.3f, AtmosphereSettings.SunIntensity, (85.0f - AbsAngle) / 85.0f);
    }
}

void ALight_AtmosphereManager::TransitionWeather(float DeltaTime)
{
    WeatherTransitionProgress += DeltaTime * WeatherTransitionSpeed;
    
    if (WeatherTransitionProgress >= 1.0f)
    {
        // Transition complete
        WeatherTransitionProgress = 1.0f;
        bWeatherTransitioning = false;
        CurrentWeather = TargetWeather;
        AtmosphereSettings = TargetSettings;
    }
    else
    {
        // Interpolate between previous and target settings
        float Alpha = WeatherTransitionProgress;
        
        AtmosphereSettings.SunIntensity = FMath::Lerp(PreviousSettings.SunIntensity, TargetSettings.SunIntensity, Alpha);
        AtmosphereSettings.FogDensity = FMath::Lerp(PreviousSettings.FogDensity, TargetSettings.FogDensity, Alpha);
        AtmosphereSettings.FogColor = FLinearColor::LerpUsingHSV(PreviousSettings.FogColor, TargetSettings.FogColor, Alpha);
    }
    
    ApplyAtmosphericCorrection();
}
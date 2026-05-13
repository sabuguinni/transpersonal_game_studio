#include "AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UAtmosphericLightingManager::UAtmosphericLightingManager()
{
    // Set default Cretaceous period settings
    CurrentTime.Hour = 12.0f;
    CurrentTime.Minute = 0.0f;
    CurrentTime.TimeSpeed = 1.0f;

    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.FogDensity = 0.1f;
    CurrentWeather.WindStrength = 0.5f;

    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunAngle = 45.0f;
    AtmosphericSettings.RayleighScattering = 0.0331f;
    AtmosphericSettings.MieScattering = 0.004f;

    bTimePaused = false;
    bWeatherTransitioning = false;
}

void UAtmosphericLightingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Initializing lighting subsystem"));
    
    // Find existing lighting components in the world
    FindLightingComponents();
    
    // Apply Cretaceous period defaults
    SetCretaceousDefaults();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Initialization complete"));
}

void UAtmosphericLightingManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Deinitializing"));
    
    // Clear component references
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    VolumetricClouds = nullptr;
    HeightFog = nullptr;
    
    Super::Deinitialize();
}

void UAtmosphericLightingManager::Tick(float DeltaTime)
{
    if (!bTimePaused)
    {
        // Update time progression
        CurrentTime.Minute += DeltaTime * CurrentTime.TimeSpeed;
        if (CurrentTime.Minute >= 60.0f)
        {
            CurrentTime.Hour += 1.0f;
            CurrentTime.Minute = 0.0f;
            if (CurrentTime.Hour >= 24.0f)
            {
                CurrentTime.Hour = 0.0f;
            }
        }
        
        // Update sun position and lighting
        UpdateSunPosition();
        UpdateSunLighting();
    }
    
    // Update weather effects
    UpdateWeatherEffects(DeltaTime);
    
    // Update atmospheric components
    UpdateSkyAtmosphere();
    UpdateVolumetricClouds();
    UpdateHeightFog();
}

bool UAtmosphericLightingManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UAtmosphericLightingManager::SetTimeOfDay(float Hour, float Minute)
{
    CurrentTime.Hour = FMath::Clamp(Hour, 0.0f, 23.99f);
    CurrentTime.Minute = FMath::Clamp(Minute, 0.0f, 59.99f);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Time set to %02d:%02d"), 
           (int32)CurrentTime.Hour, (int32)CurrentTime.Minute);
}

void UAtmosphericLightingManager::SetTimeSpeed(float Speed)
{
    CurrentTime.TimeSpeed = FMath::Max(0.0f, Speed);
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Time speed set to %f"), Speed);
}

void UAtmosphericLightingManager::PauseTimeProgression(bool bPause)
{
    bTimePaused = bPause;
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Time progression %s"), 
           bPause ? TEXT("paused") : TEXT("resumed"));
}

void UAtmosphericLightingManager::SetWeatherState(const FLight_WeatherState& NewWeather)
{
    CurrentWeather = NewWeather;
    bWeatherTransitioning = false;
    WeatherTransitionProgress = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Weather state updated"));
}

void UAtmosphericLightingManager::TransitionToWeather(const FLight_WeatherState& TargetWeatherState, float TransitionTime)
{
    TargetWeather = TargetWeatherState;
    WeatherTransitionTime = FMath::Max(0.1f, TransitionTime);
    WeatherTransitionProgress = 0.0f;
    bWeatherTransitioning = true;
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Starting weather transition over %f seconds"), TransitionTime);
}

void UAtmosphericLightingManager::UpdateAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    AtmosphericSettings = Settings;
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Atmospheric settings updated"));
}

void UAtmosphericLightingManager::SetCretaceousDefaults()
{
    // Set tropical Cretaceous period lighting
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm golden sun
    AtmosphericSettings.SunIntensity = 5.0f; // Bright tropical sun
    AtmosphericSettings.RayleighScattering = 0.0331f; // Clear blue sky
    AtmosphericSettings.MieScattering = 0.004f; // Minimal haze
    
    // Set mild tropical weather
    CurrentWeather.CloudCoverage = 0.3f; // Scattered clouds
    CurrentWeather.RainIntensity = 0.0f; // No rain by default
    CurrentWeather.FogDensity = 0.05f; // Light morning mist
    CurrentWeather.WindStrength = 0.4f; // Gentle breeze
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Applied Cretaceous period defaults"));
}

void UAtmosphericLightingManager::RefreshLightingComponents()
{
    FindLightingComponents();
    ValidateComponents();
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Refreshed lighting components"));
}

void UAtmosphericLightingManager::ApplyLightingPreset(const FString& PresetName)
{
    if (PresetName == TEXT("Dawn"))
    {
        SetTimeOfDay(6.0f, 0.0f);
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        AtmosphericSettings.SunIntensity = 3.0f;
    }
    else if (PresetName == TEXT("Noon"))
    {
        SetTimeOfDay(12.0f, 0.0f);
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        AtmosphericSettings.SunIntensity = 5.0f;
    }
    else if (PresetName == TEXT("Sunset"))
    {
        SetTimeOfDay(18.0f, 0.0f);
        AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
        AtmosphericSettings.SunIntensity = 2.0f;
    }
    else if (PresetName == TEXT("Night"))
    {
        SetTimeOfDay(0.0f, 0.0f);
        AtmosphericSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
        AtmosphericSettings.SunIntensity = 0.5f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Applied lighting preset: %s"), *PresetName);
}

void UAtmosphericLightingManager::TriggerLightningStrike(FVector Location)
{
    // TODO: Implement lightning effect
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Lightning strike at %s"), *Location.ToString());
}

void UAtmosphericLightingManager::SetVolumetricFogEnabled(bool bEnabled)
{
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetVisibility(bEnabled);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Volumetric fog %s"), 
               bEnabled ? TEXT("enabled") : TEXT("disabled"));
    }
}

void UAtmosphericLightingManager::UpdateSunPosition()
{
    if (!SunLight)
        return;
    
    // Calculate sun angle based on time of day
    float TimeOfDayNormalized = (CurrentTime.Hour + CurrentTime.Minute / 60.0f) / 24.0f;
    float SunAngleRadians = (TimeOfDayNormalized - 0.5f) * PI; // -90 to +90 degrees
    
    // Calculate sun rotation
    FRotator SunRotation;
    SunRotation.Pitch = FMath::RadiansToDegrees(SunAngleRadians);
    SunRotation.Yaw = 0.0f; // East-West movement
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
    AtmosphericSettings.SunAngle = SunRotation.Pitch;
}

void UAtmosphericLightingManager::UpdateSunLighting()
{
    if (!SunLight || !SunLight->GetLightComponent())
        return;
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    // Update sun color and intensity based on time and weather
    FLinearColor FinalSunColor = CalculateSunColor();
    float FinalSunIntensity = CalculateSunIntensity();
    
    LightComp->SetLightColor(FinalSunColor);
    LightComp->SetIntensity(FinalSunIntensity);
}

void UAtmosphericLightingManager::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere || !SkyAtmosphere->GetComponent())
        return;
    
    USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
    SkyComp->SetRayleighScatteringScale(AtmosphericSettings.RayleighScattering);
    SkyComp->SetMieScatteringScale(AtmosphericSettings.MieScattering);
}

void UAtmosphericLightingManager::UpdateWeatherEffects(float DeltaTime)
{
    if (bWeatherTransitioning)
    {
        WeatherTransitionProgress += DeltaTime / WeatherTransitionTime;
        
        if (WeatherTransitionProgress >= 1.0f)
        {
            CurrentWeather = TargetWeather;
            bWeatherTransitioning = false;
            WeatherTransitionProgress = 0.0f;
        }
        else
        {
            // Interpolate weather values
            float Alpha = WeatherTransitionProgress;
            CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, Alpha);
            CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, Alpha);
            CurrentWeather.FogDensity = FMath::Lerp(CurrentWeather.FogDensity, TargetWeather.FogDensity, Alpha);
            CurrentWeather.WindStrength = FMath::Lerp(CurrentWeather.WindStrength, TargetWeather.WindStrength, Alpha);
        }
    }
}

void UAtmosphericLightingManager::UpdateVolumetricClouds()
{
    if (!VolumetricClouds || !VolumetricClouds->GetComponent())
        return;
    
    // Hide volumetric clouds for clean Cretaceous atmosphere
    VolumetricClouds->SetActorHiddenInGame(true);
}

void UAtmosphericLightingManager::UpdateHeightFog()
{
    if (!HeightFog || !HeightFog->GetComponent())
        return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    FogComp->SetFogDensity(CurrentWeather.FogDensity * 0.02f); // Scale fog density
}

void UAtmosphericLightingManager::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find directional light (sun)
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break; // Use first directional light found
    }
    
    // Find sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphere = *ActorItr;
        break;
    }
    
    // Find volumetric clouds
    for (TActorIterator<AVolumetricCloud> ActorItr(World); ActorItr; ++ActorItr)
    {
        VolumetricClouds = *ActorItr;
        break;
    }
    
    // Find height fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        HeightFog = *ActorItr;
        break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Found components - Sun: %s, Sky: %s, Clouds: %s, Fog: %s"),
           SunLight ? TEXT("Yes") : TEXT("No"),
           SkyAtmosphere ? TEXT("Yes") : TEXT("No"),
           VolumetricClouds ? TEXT("Yes") : TEXT("No"),
           HeightFog ? TEXT("Yes") : TEXT("No"));
}

void UAtmosphericLightingManager::ValidateComponents()
{
    if (!SunLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: No DirectionalLight found in world"));
    }
    
    if (!SkyAtmosphere)
    {
        UE_LOG(LogTemp, Warning, TEXT("AtmosphericLightingManager: No SkyAtmosphere found in world"));
    }
}

float UAtmosphericLightingManager::CalculateSunAngle() const
{
    float TimeOfDayNormalized = (CurrentTime.Hour + CurrentTime.Minute / 60.0f) / 24.0f;
    return (TimeOfDayNormalized - 0.5f) * 180.0f; // -90 to +90 degrees
}

FLinearColor UAtmosphericLightingManager::CalculateSunColor() const
{
    float SunAngle = CalculateSunAngle();
    
    // Adjust color based on sun angle (warmer at low angles)
    if (FMath::Abs(SunAngle) > 80.0f)
    {
        // Very low sun - warm orange/red
        return FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    }
    else if (FMath::Abs(SunAngle) > 60.0f)
    {
        // Low sun - warm yellow
        return FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    }
    else
    {
        // High sun - neutral warm white
        return AtmosphericSettings.SunColor;
    }
}

float UAtmosphericLightingManager::CalculateSunIntensity() const
{
    float SunAngle = CalculateSunAngle();
    
    // Reduce intensity at low sun angles
    float AngleFactor = FMath::Clamp(1.0f - (FMath::Abs(SunAngle) / 90.0f), 0.1f, 1.0f);
    float WeatherFactor = 1.0f - (CurrentWeather.CloudCoverage * 0.5f);
    
    return AtmosphericSettings.SunIntensity * AngleFactor * WeatherFactor;
}

FLinearColor UAtmosphericLightingManager::InterpolateWeatherColor(const FLinearColor& Clear, const FLinearColor& Stormy, float Factor) const
{
    return FLinearColor::LerpUsingHSV(Clear, Stormy, Factor);
}
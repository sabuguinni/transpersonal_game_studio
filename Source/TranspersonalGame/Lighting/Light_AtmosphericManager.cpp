#include "Light_AtmosphericManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericManager::ALight_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create sun light component
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f));
    SunLight->SetCastShadows(true);
    SunLight->SetMobility(EComponentMobility::Movable);

    // Create sky light component
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetMobility(EComponentMobility::Movable);

    // Create sky atmosphere component
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create height fog component
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogMaxOpacity(1.0f);

    // Create volumetric clouds component
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Initialize default weather presets
    WeatherPresets.Empty();
    
    // Clear weather
    FLight_WeatherState ClearWeather;
    ClearWeather.CloudCoverage = 0.1f;
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.RainIntensity = 0.0f;
    ClearWeather.SkyTint = FLinearColor(0.5f, 0.7f, 1.0f);
    ClearWeather.WindStrength = 0.3f;
    WeatherPresets.Add(ClearWeather);

    // Cloudy weather
    FLight_WeatherState CloudyWeather;
    CloudyWeather.CloudCoverage = 0.7f;
    CloudyWeather.FogDensity = 0.03f;
    CloudyWeather.RainIntensity = 0.0f;
    CloudyWeather.SkyTint = FLinearColor(0.4f, 0.5f, 0.7f);
    CloudyWeather.WindStrength = 0.6f;
    WeatherPresets.Add(CloudyWeather);

    // Stormy weather
    FLight_WeatherState StormyWeather;
    StormyWeather.CloudCoverage = 0.9f;
    StormyWeather.FogDensity = 0.05f;
    StormyWeather.RainIntensity = 0.8f;
    StormyWeather.SkyTint = FLinearColor(0.3f, 0.3f, 0.5f);
    StormyWeather.WindStrength = 1.0f;
    WeatherPresets.Add(StormyWeather);

    // Initialize biome weather settings
    BiomeWeatherSettings.Empty();
    
    // Savanna - hot and dry
    FLight_WeatherState SavannaWeather;
    SavannaWeather.CloudCoverage = 0.2f;
    SavannaWeather.FogDensity = 0.005f;
    SavannaWeather.SkyTint = FLinearColor(0.8f, 0.7f, 0.5f);
    BiomeWeatherSettings.Add(EBiomeType::Savanna, SavannaWeather);

    // Swamp - humid and foggy
    FLight_WeatherState SwampWeather;
    SwampWeather.CloudCoverage = 0.6f;
    SwampWeather.FogDensity = 0.08f;
    SwampWeather.SkyTint = FLinearColor(0.4f, 0.6f, 0.5f);
    BiomeWeatherSettings.Add(EBiomeType::Swamp, SwampWeather);

    // Forest - moderate with filtered light
    FLight_WeatherState ForestWeather;
    ForestWeather.CloudCoverage = 0.4f;
    ForestWeather.FogDensity = 0.02f;
    ForestWeather.SkyTint = FLinearColor(0.3f, 0.7f, 0.4f);
    BiomeWeatherSettings.Add(EBiomeType::Forest, ForestWeather);

    // Desert - clear and harsh
    FLight_WeatherState DesertWeather;
    DesertWeather.CloudCoverage = 0.1f;
    DesertWeather.FogDensity = 0.001f;
    DesertWeather.SkyTint = FLinearColor(0.9f, 0.8f, 0.6f);
    BiomeWeatherSettings.Add(EBiomeType::Desert, DesertWeather);

    // Mountain - variable with altitude effects
    FLight_WeatherState MountainWeather;
    MountainWeather.CloudCoverage = 0.5f;
    MountainWeather.FogDensity = 0.04f;
    MountainWeather.SkyTint = FLinearColor(0.6f, 0.7f, 0.9f);
    BiomeWeatherSettings.Add(EBiomeType::Mountain, MountainWeather);

    // Set initial state
    CurrentWeather = ClearWeather;
    TargetWeather = ClearWeather;
}

void ALight_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize lighting system
    UpdateSunPosition();
    UpdateLightingParameters();
    UpdateAtmosphericScattering();
    UpdateVolumetricClouds();
    UpdateHeightFog();
}

void ALight_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        TimeOfDay.CurrentHour += (DeltaTime / TimeOfDay.DayDuration) * 24.0f;
        if (TimeOfDay.CurrentHour >= 24.0f)
        {
            TimeOfDay.CurrentHour -= 24.0f;
        }

        UpdateSunPosition();
        UpdateLightingParameters();
    }

    if (bEnableWeatherSystem)
    {
        UpdateWeatherTransition(DeltaTime);
    }

    UpdateAtmosphericScattering();
    UpdateVolumetricClouds();
    UpdateHeightFog();
}

void ALight_AtmosphericManager::SetTimeOfDay(float Hour)
{
    TimeOfDay.CurrentHour = FMath::Clamp(Hour, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateLightingParameters();
}

void ALight_AtmosphericManager::SetDayDuration(float Duration)
{
    TimeOfDay.DayDuration = FMath::Max(Duration, 60.0f); // Minimum 1 minute
}

void ALight_AtmosphericManager::SetWeatherState(const FLight_WeatherState& NewWeather)
{
    CurrentWeather = NewWeather;
    TargetWeather = NewWeather;
    bIsTransitioning = false;
}

void ALight_AtmosphericManager::TransitionToWeather(const FLight_WeatherState& TargetState, float TransitionTime)
{
    TargetWeather = TargetState;
    WeatherTransitionDuration = FMath::Max(TransitionTime, 0.1f);
    WeatherTransitionTimer = 0.0f;
    bIsTransitioning = true;
}

void ALight_AtmosphericManager::SetRandomWeather()
{
    if (WeatherPresets.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, WeatherPresets.Num() - 1);
        TransitionToWeather(WeatherPresets[RandomIndex], 10.0f);
    }
}

void ALight_AtmosphericManager::SetBiomeWeather(EBiomeType BiomeType)
{
    if (BiomeWeatherSettings.Contains(BiomeType))
    {
        TransitionToWeather(BiomeWeatherSettings[BiomeType], 5.0f);
    }
}

void ALight_AtmosphericManager::UpdateWeatherForBiome(EBiomeType BiomeType, const FLight_WeatherState& WeatherState)
{
    BiomeWeatherSettings.Add(BiomeType, WeatherState);
}

void ALight_AtmosphericManager::SetSunIntensity(float Intensity)
{
    TimeOfDay.SunIntensity = FMath::Max(Intensity, 0.0f);
    if (SunLight)
    {
        SunLight->SetIntensity(TimeOfDay.SunIntensity);
    }
}

void ALight_AtmosphericManager::SetSunColor(const FLinearColor& Color)
{
    TimeOfDay.SunColor = Color;
    if (SunLight)
    {
        SunLight->SetLightColor(TimeOfDay.SunColor);
    }
}

void ALight_AtmosphericManager::SetFogDensity(float Density)
{
    CurrentWeather.FogDensity = FMath::Max(Density, 0.0f);
    if (HeightFog)
    {
        HeightFog->SetFogDensity(CurrentWeather.FogDensity);
    }
}

void ALight_AtmosphericManager::SetCloudCoverage(float Coverage)
{
    CurrentWeather.CloudCoverage = FMath::Clamp(Coverage, 0.0f, 1.0f);
}

void ALight_AtmosphericManager::ResetToDefaults()
{
    TimeOfDay.CurrentHour = 12.0f;
    TimeOfDay.DayDuration = 1200.0f;
    TimeOfDay.SunIntensity = 3.0f;
    TimeOfDay.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);

    if (WeatherPresets.Num() > 0)
    {
        SetWeatherState(WeatherPresets[0]); // Clear weather
    }
}

void ALight_AtmosphericManager::SaveCurrentSettings()
{
    // Implementation would save to game instance or save game object
    UE_LOG(LogTemp, Log, TEXT("Atmospheric settings saved"));
}

void ALight_AtmosphericManager::LoadSavedSettings()
{
    // Implementation would load from game instance or save game object
    UE_LOG(LogTemp, Log, TEXT("Atmospheric settings loaded"));
}

void ALight_AtmosphericManager::UpdateSunPosition()
{
    if (!SunLight) return;

    FRotator SunRotation = CalculateSunRotation(TimeOfDay.CurrentHour);
    SunLight->SetWorldRotation(SunRotation);
}

void ALight_AtmosphericManager::UpdateLightingParameters()
{
    if (!SunLight || !SkyLight) return;

    float SunIntensity = CalculateSunIntensity(TimeOfDay.CurrentHour);
    FLinearColor SunColor = CalculateSunColor(TimeOfDay.CurrentHour);

    SunLight->SetIntensity(SunIntensity * TimeOfDay.SunIntensity);
    SunLight->SetLightColor(SunColor);

    // Update sky light intensity based on time of day
    float SkyIntensity = FMath::Clamp(SunIntensity * 0.3f + 0.1f, 0.1f, 1.0f);
    SkyLight->SetIntensity(SkyIntensity);
}

void ALight_AtmosphericManager::UpdateWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning) return;

    WeatherTransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(WeatherTransitionTimer / WeatherTransitionDuration, 0.0f, 1.0f);

    // Interpolate weather parameters
    CurrentWeather.CloudCoverage = FMath::Lerp(CurrentWeather.CloudCoverage, TargetWeather.CloudCoverage, Alpha * WeatherTransitionSpeed);
    CurrentWeather.FogDensity = FMath::Lerp(CurrentWeather.FogDensity, TargetWeather.FogDensity, Alpha * WeatherTransitionSpeed);
    CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, Alpha * WeatherTransitionSpeed);
    CurrentWeather.SkyTint = FMath::Lerp(CurrentWeather.SkyTint, TargetWeather.SkyTint, Alpha * WeatherTransitionSpeed);
    CurrentWeather.WindStrength = FMath::Lerp(CurrentWeather.WindStrength, TargetWeather.WindStrength, Alpha * WeatherTransitionSpeed);

    if (Alpha >= 1.0f)
    {
        CurrentWeather = TargetWeather;
        bIsTransitioning = false;
    }
}

void ALight_AtmosphericManager::UpdateAtmosphericScattering()
{
    if (!SkyAtmosphere) return;

    // Update atmospheric parameters based on weather and time
    // This would typically involve setting material parameters or component properties
}

void ALight_AtmosphericManager::UpdateVolumetricClouds()
{
    if (!VolumetricClouds) return;

    // Update cloud parameters based on current weather
    // This would involve setting cloud density, coverage, etc.
}

void ALight_AtmosphericManager::UpdateHeightFog()
{
    if (!HeightFog) return;

    HeightFog->SetFogDensity(CurrentWeather.FogDensity);
    
    // Adjust fog color based on time of day and weather
    FLinearColor FogColor = CurrentWeather.SkyTint;
    if (IsNightTime())
    {
        FogColor = FogColor * 0.3f; // Darker fog at night
    }
    
    HeightFog->SetFogInscatteringColor(FogColor);
}

FLinearColor ALight_AtmosphericManager::CalculateSunColor(float Hour) const
{
    // Sunrise/sunset colors
    if (Hour >= 5.0f && Hour <= 7.0f)
    {
        // Sunrise - orange/red tints
        float Factor = (Hour - 5.0f) / 2.0f;
        return FMath::Lerp(FLinearColor(1.0f, 0.4f, 0.2f), TimeOfDay.SunColor, Factor);
    }
    else if (Hour >= 17.0f && Hour <= 19.0f)
    {
        // Sunset - orange/red tints
        float Factor = (19.0f - Hour) / 2.0f;
        return FMath::Lerp(TimeOfDay.SunColor, FLinearColor(1.0f, 0.4f, 0.2f), Factor);
    }
    else if (Hour >= 7.0f && Hour <= 17.0f)
    {
        // Daytime - normal sun color
        return TimeOfDay.SunColor;
    }
    else
    {
        // Nighttime - moon color
        return FLinearColor(0.2f, 0.3f, 0.5f);
    }
}

float ALight_AtmosphericManager::CalculateSunIntensity(float Hour) const
{
    if (Hour >= 6.0f && Hour <= 18.0f)
    {
        // Daytime - use sine curve for smooth transition
        float NormalizedHour = (Hour - 6.0f) / 12.0f; // 0 to 1
        float Angle = NormalizedHour * PI;
        return FMath::Sin(Angle) * TimeOfDay.SunIntensity;
    }
    else
    {
        // Nighttime - moon intensity
        return TimeOfDay.MoonIntensity;
    }
}

FRotator ALight_AtmosphericManager::CalculateSunRotation(float Hour) const
{
    // Calculate sun position based on time of day
    float SunAngle = ((Hour - 6.0f) / 12.0f) * 180.0f; // 6 AM = 0°, 6 PM = 180°
    
    // Clamp to reasonable range
    SunAngle = FMath::Clamp(SunAngle, -10.0f, 190.0f);
    
    return FRotator(-SunAngle, 0.0f, 0.0f);
}
#include "LightingController.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ALightingController::ALightingController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create Sun Light (Primary Directional Light)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetIntensity(10.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f)); // Warm sunlight
    SunLight->SetCastShadows(true);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);

    // Create Moon Light (Secondary Directional Light)
    MoonLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("MoonLight"));
    MoonLight->SetupAttachment(RootComponent);
    MoonLight->SetMobility(EComponentMobility::Movable);
    MoonLight->SetIntensity(0.5f);
    MoonLight->SetLightColor(FLinearColor(0.7f, 0.8f, 1.0f, 1.0f)); // Cool moonlight
    MoonLight->SetCastShadows(true);
    MoonLight->SetAtmosphereSunLight(true);
    MoonLight->SetAtmosphereSunLightIndex(1);

    // Create Sky Atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create Volumetric Clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Create Sky Light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
    SkyLight->SetRealTimeCapture(true);

    // Create Height Fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentTimeOfDay = 12.0f; // Start at noon
    TimeSpeed = 1.0f;
    bAutoProgressTime = true;
    CurrentWeather = EWeatherState::Clear;
    TargetWeather = EWeatherState::Clear;
    WeatherTransitionSpeed = 5.0f;
    CurrentMood = EMoodState::Peaceful;
    MoodIntensity = 0.5f;
}

void ALightingController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultPresets();
    UpdateLighting(0.0f);
}

void ALightingController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoProgressTime)
    {
        CurrentTimeOfDay += (DeltaTime * TimeSpeed) / 3600.0f; // Convert seconds to hours
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }
    
    UpdateLighting(DeltaTime);
}

void ALightingController::UpdateLighting(float DeltaTime)
{
    // Check for time of day changes
    ETimeOfDay NewTimeOfDay = GetCurrentTimeOfDayEnum();
    if (NewTimeOfDay != LastTimeOfDay)
    {
        LastTimeOfDay = NewTimeOfDay;
        OnTimeOfDayChanged.Broadcast(NewTimeOfDay);
    }

    // Update weather transition
    if (CurrentWeather != TargetWeather)
    {
        WeatherTransitionAlpha += DeltaTime / (WeatherTransitionSpeed * 60.0f);
        if (WeatherTransitionAlpha >= 1.0f)
        {
            WeatherTransitionAlpha = 1.0f;
            CurrentWeather = TargetWeather;
            OnWeatherChanged.Broadcast(CurrentWeather);
        }
    }

    // Update lighting transitions
    if (bIsTransitioning)
    {
        TransitionAlpha += DeltaTime * TransitionSpeed;
        if (TransitionAlpha >= 1.0f)
        {
            TransitionAlpha = 1.0f;
            bIsTransitioning = false;
            CurrentPreset = TargetPreset;
        }
    }

    // Calculate and apply current lighting state
    FLightingPreset FinalPreset = CalculateCurrentPreset();
    ApplyPreset(FinalPreset);

    // Update individual systems
    UpdateSunPosition();
    UpdateMoonPosition();
    UpdateSkyAtmosphere();
    UpdateClouds();
    UpdateFog();
}

void ALightingController::UpdateSunPosition()
{
    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 6 AM = 0°, 6 PM = 180°
    
    // Create rotation for sun
    FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);

    // Adjust sun intensity based on angle
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(FMath::Max(0.0f, 90.0f + SunAngle)));
    float SunIntensity = FMath::Max(0.0f, SunHeight) * CurrentPreset.SunIntensity;
    SunLight->SetIntensity(SunIntensity);

    // Adjust sun color based on time
    FLinearColor SunColor = CurrentPreset.SunColor;
    if (SunHeight < 0.2f && SunHeight > 0.0f) // Sunrise/Sunset
    {
        float SunsetFactor = 1.0f - (SunHeight / 0.2f);
        SunColor = FLinearColor::LerpUsingHSV(SunColor, FLinearColor(1.0f, 0.4f, 0.1f, 1.0f), SunsetFactor);
    }
    SunLight->SetLightColor(SunColor);
}

void ALightingController::UpdateMoonPosition()
{
    // Moon is opposite to sun
    float MoonAngle = (CurrentTimeOfDay - 18.0f) * 15.0f; // 6 PM = 0°, 6 AM = 180°
    if (MoonAngle < 0) MoonAngle += 360.0f;
    
    FRotator MoonRotation = FRotator(-MoonAngle, 0.0f, 0.0f);
    MoonLight->SetWorldRotation(MoonRotation);

    // Moon intensity based on sun position (stronger at night)
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(FMath::Max(0.0f, 90.0f + (CurrentTimeOfDay - 6.0f) * 15.0f)));
    float MoonIntensity = FMath::Max(0.0f, 1.0f - FMath::Max(0.0f, SunHeight)) * 0.5f;
    MoonLight->SetIntensity(MoonIntensity);
}

void ALightingController::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphere) return;

    // Update sky atmosphere based on current preset
    SkyAtmosphere->SetSkyLuminanceFactor(CurrentPreset.SkyLuminanceFactor);
    
    // Adjust atmosphere based on weather
    float WeatherFactor = 1.0f;
    switch (CurrentWeather)
    {
        case EWeatherState::Overcast:
            WeatherFactor = 0.6f;
            break;
        case EWeatherState::Storm:
            WeatherFactor = 0.3f;
            break;
        case EWeatherState::Fog:
            WeatherFactor = 0.4f;
            break;
        default:
            break;
    }
    
    SkyAtmosphere->SetSkyLuminanceFactor(CurrentPreset.SkyLuminanceFactor * WeatherFactor);
}

void ALightingController::UpdateClouds()
{
    if (!VolumetricClouds) return;

    // Update cloud properties based on weather
    float CloudCoverage = CurrentPreset.CloudCoverage;
    float CloudOpacity = CurrentPreset.CloudOpacity;

    switch (CurrentWeather)
    {
        case EWeatherState::PartlyCloudy:
            CloudCoverage = 0.4f;
            CloudOpacity = 0.6f;
            break;
        case EWeatherState::Overcast:
            CloudCoverage = 0.9f;
            CloudOpacity = 0.8f;
            break;
        case EWeatherState::Storm:
            CloudCoverage = 1.0f;
            CloudOpacity = 0.95f;
            break;
        case EWeatherState::Clear:
            CloudCoverage = 0.1f;
            CloudOpacity = 0.3f;
            break;
        default:
            break;
    }

    // Apply weather transition
    if (CurrentWeather != TargetWeather && WeatherTransitionAlpha < 1.0f)
    {
        float TargetCoverage = CloudCoverage;
        float TargetOpacity = CloudOpacity;
        
        // Get target weather values
        switch (TargetWeather)
        {
            case EWeatherState::PartlyCloudy:
                TargetCoverage = 0.4f;
                TargetOpacity = 0.6f;
                break;
            case EWeatherState::Overcast:
                TargetCoverage = 0.9f;
                TargetOpacity = 0.8f;
                break;
            case EWeatherState::Storm:
                TargetCoverage = 1.0f;
                TargetOpacity = 0.95f;
                break;
            case EWeatherState::Clear:
                TargetCoverage = 0.1f;
                TargetOpacity = 0.3f;
                break;
        }
        
        CloudCoverage = FMath::Lerp(CloudCoverage, TargetCoverage, WeatherTransitionAlpha);
        CloudOpacity = FMath::Lerp(CloudOpacity, TargetOpacity, WeatherTransitionAlpha);
    }

    // Note: Actual cloud material parameters would be set here
    // This requires the cloud material to expose these parameters
}

void ALightingController::UpdateFog()
{
    if (!HeightFog) return;

    // Update fog based on current preset and mood
    FLinearColor FogColor = CurrentPreset.FogInscatteringColor;
    float FogDensity = CurrentPreset.FogDensity;

    // Adjust fog based on mood
    switch (CurrentMood)
    {
        case EMoodState::Threatening:
            FogColor = FLinearColor::LerpUsingHSV(FogColor, FLinearColor(0.6f, 0.4f, 0.3f, 1.0f), MoodIntensity * 0.5f);
            FogDensity *= (1.0f + MoodIntensity * 0.5f);
            break;
        case EMoodState::Dangerous:
            FogColor = FLinearColor::LerpUsingHSV(FogColor, FLinearColor(0.7f, 0.3f, 0.2f, 1.0f), MoodIntensity * 0.7f);
            FogDensity *= (1.0f + MoodIntensity * 0.8f);
            break;
        case EMoodState::Terror:
            FogColor = FLinearColor::LerpUsingHSV(FogColor, FLinearColor(0.5f, 0.2f, 0.1f, 1.0f), MoodIntensity);
            FogDensity *= (1.0f + MoodIntensity * 1.2f);
            break;
        default:
            break;
    }

    HeightFog->SetFogInscatteringColor(FogColor);
    HeightFog->SetFogDensity(FogDensity);
    HeightFog->SetFogHeightFalloff(CurrentPreset.FogHeightFalloff);
}

FLightingPreset ALightingController::CalculateCurrentPreset() const
{
    // Get base preset from time of day
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    FLightingPreset BasePreset = TimePresets.Contains(CurrentTimeEnum) ? 
        TimePresets[CurrentTimeEnum] : FLightingPreset();

    // Blend with weather preset
    if (WeatherPresets.Contains(CurrentWeather))
    {
        BasePreset = BlendPresets(BasePreset, WeatherPresets[CurrentWeather], 0.5f);
    }

    // Blend with mood preset
    if (MoodPresets.Contains(CurrentMood))
    {
        BasePreset = BlendPresets(BasePreset, MoodPresets[CurrentMood], MoodIntensity * 0.3f);
    }

    // Apply transition if active
    if (bIsTransitioning)
    {
        BasePreset = BlendPresets(CurrentPreset, TargetPreset, TransitionAlpha);
    }

    return BasePreset;
}

FLightingPreset ALightingController::BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha) const
{
    FLightingPreset Result;
    
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunSourceAngle = FMath::Lerp(A.SunSourceAngle, B.SunSourceAngle, Alpha);
    
    Result.SkyColor = FLinearColor::LerpUsingHSV(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyLuminanceFactor = FMath::Lerp(A.SkyLuminanceFactor, B.SkyLuminanceFactor, Alpha);
    
    Result.FogInscatteringColor = FLinearColor::LerpUsingHSV(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.CloudOpacity = FMath::Lerp(A.CloudOpacity, B.CloudOpacity, Alpha);
    
    Result.Exposure = FMath::Lerp(A.Exposure, B.Exposure, Alpha);
    Result.ColorGrading = FLinearColor::LerpUsingHSV(A.ColorGrading, B.ColorGrading, Alpha);
    Result.Contrast = FMath::Lerp(A.Contrast, B.Contrast, Alpha);
    Result.Saturation = FMath::Lerp(A.Saturation, B.Saturation, Alpha);
    
    return Result;
}

void ALightingController::ApplyPreset(const FLightingPreset& Preset)
{
    CurrentPreset = Preset;
    
    // Apply sun properties
    SunLight->SetLightColor(Preset.SunColor);
    SunLight->SetSourceAngle(Preset.SunSourceAngle);
    
    // Sky light updates automatically with real-time capture
    SkyLight->SetIntensity(Preset.SkyLuminanceFactor);
    
    // Note: Post-process settings would be applied to the global post-process volume here
    // This requires a reference to the post-process volume or using console commands
}

void ALightingController::InitializeDefaultPresets()
{
    // Initialize Time of Day presets
    FLightingPreset DawnPreset;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Warm orange
    DawnPreset.SunIntensity = 3.0f;
    DawnPreset.SkyLuminanceFactor = 0.5f;
    DawnPreset.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnPreset.FogDensity = 0.05f;
    DawnPreset.CloudCoverage = 0.3f;
    DawnPreset.Exposure = 0.8f;
    DawnPreset.Saturation = 1.2f;
    TimePresets.Add(ETimeOfDay::Dawn, DawnPreset);

    FLightingPreset NoonPreset;
    NoonPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Bright white-yellow
    NoonPreset.SunIntensity = 15.0f;
    NoonPreset.SkyLuminanceFactor = 1.0f;
    NoonPreset.FogInscatteringColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    NoonPreset.FogDensity = 0.02f;
    NoonPreset.CloudCoverage = 0.2f;
    NoonPreset.Exposure = 1.0f;
    NoonPreset.Saturation = 1.0f;
    TimePresets.Add(ETimeOfDay::Noon, NoonPreset);

    FLightingPreset DuskPreset;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f); // Deep orange-red
    DuskPreset.SunIntensity = 2.0f;
    DuskPreset.SkyLuminanceFactor = 0.3f;
    DuskPreset.FogInscatteringColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
    DuskPreset.FogDensity = 0.08f;
    DuskPreset.CloudCoverage = 0.4f;
    DuskPreset.Exposure = 0.6f;
    DuskPreset.Saturation = 1.3f;
    TimePresets.Add(ETimeOfDay::Dusk, DuskPreset);

    FLightingPreset NightPreset;
    NightPreset.SunColor = FLinearColor(0.1f, 0.1f, 0.2f, 1.0f); // Very dim blue
    NightPreset.SunIntensity = 0.0f;
    NightPreset.SkyLuminanceFactor = 0.1f;
    NightPreset.FogInscatteringColor = FLinearColor(0.2f, 0.2f, 0.4f, 1.0f);
    NightPreset.FogDensity = 0.1f;
    NightPreset.CloudCoverage = 0.6f;
    NightPreset.Exposure = 0.4f;
    NightPreset.Saturation = 0.8f;
    TimePresets.Add(ETimeOfDay::Night, NightPreset);

    // Initialize Weather presets
    FLightingPreset StormPreset;
    StormPreset.SunIntensity = 0.3f; // Very dark
    StormPreset.SkyLuminanceFactor = 0.2f;
    StormPreset.FogDensity = 0.15f;
    StormPreset.CloudCoverage = 1.0f;
    StormPreset.CloudOpacity = 0.95f;
    StormPreset.Exposure = 0.3f;
    StormPreset.Contrast = 1.4f;
    StormPreset.Saturation = 0.6f;
    WeatherPresets.Add(EWeatherState::Storm, StormPreset);

    // Initialize Mood presets
    FLightingPreset ThreatPreset;
    ThreatPreset.FogInscatteringColor = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f); // Brownish
    ThreatPreset.FogDensity = 0.06f;
    ThreatPreset.Contrast = 1.2f;
    ThreatPreset.Saturation = 0.9f;
    MoodPresets.Add(EMoodState::Threatening, ThreatPreset);

    FLightingPreset TerrorPreset;
    TerrorPreset.FogInscatteringColor = FLinearColor(0.5f, 0.2f, 0.1f, 1.0f); // Dark red
    TerrorPreset.FogDensity = 0.12f;
    TerrorPreset.Exposure = 0.5f;
    TerrorPreset.Contrast = 1.5f;
    TerrorPreset.Saturation = 0.7f;
    MoodPresets.Add(EMoodState::Terror, TerrorPreset);
}

// Blueprint Functions Implementation
void ALightingController::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
}

void ALightingController::SetWeather(EWeatherState NewWeather, bool bInstant)
{
    TargetWeather = NewWeather;
    if (bInstant)
    {
        CurrentWeather = NewWeather;
        WeatherTransitionAlpha = 1.0f;
        OnWeatherChanged.Broadcast(NewWeather);
    }
    else
    {
        WeatherTransitionAlpha = 0.0f;
    }
}

void ALightingController::SetMood(EMoodState NewMood, float Intensity)
{
    CurrentMood = NewMood;
    MoodIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    OnMoodChanged.Broadcast(NewMood);
}

void ALightingController::TransitionToPreset(const FLightingPreset& Preset, float TransitionTime)
{
    TargetPreset = Preset;
    TransitionSpeed = TransitionTime > 0.0f ? 1.0f / TransitionTime : 999.0f;
    TransitionAlpha = 0.0f;
    bIsTransitioning = true;
}

ETimeOfDay ALightingController::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ETimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ETimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f)
        return ETimeOfDay::Night;
    else
        return ETimeOfDay::DeepNight;
}

float ALightingController::GetSunAngle() const
{
    return (CurrentTimeOfDay - 6.0f) * 15.0f;
}

bool ALightingController::IsNightTime() const
{
    return CurrentTimeOfDay >= 20.0f || CurrentTimeOfDay < 5.0f;
}

bool ALightingController::IsDaytime() const
{
    return CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 18.0f;
}
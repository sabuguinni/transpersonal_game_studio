#include "LightingSystemCore.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Kismet/KismetMathLibrary.h"

ALightingSystemCore::ALightingSystemCore()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create lighting components
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->bRealTimeCapture = true;

    // Create atmosphere components
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);

    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
    PostProcessComponent->SetupAttachment(RootComponent);
    PostProcessComponent->bUnbound = true;

    // Initialize default presets
    InitializeDefaultPresets();
}

void ALightingSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial lighting state
    UpdateLighting();
}

void ALightingSystemCore::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update time progression
    if (bEnableTimeProgression)
    {
        float TimeIncrement = (24.0f / (DayDurationInMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }

    // Handle transitions
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        CurrentPreset = BlendPresets(CurrentPreset, TargetPreset, Alpha);
        
        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            CurrentPreset = TargetPreset;
        }
    }

    UpdateLighting();
}

void ALightingSystemCore::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting();
}

void ALightingSystemCore::SetWeatherState(EWeatherState NewWeather)
{
    if (CurrentWeather != NewWeather)
    {
        CurrentWeather = NewWeather;
        
        if (WeatherPresets.Contains(NewWeather))
        {
            TransitionToPreset(WeatherPresets[NewWeather], 3.0f);
        }
    }
}

void ALightingSystemCore::SetEmotionalTone(EEmotionalTone NewTone)
{
    if (CurrentEmotionalTone != NewTone)
    {
        CurrentEmotionalTone = NewTone;
        ApplyEmotionalTone();
    }
}

void ALightingSystemCore::TransitionToPreset(const FLightingPreset& TargetPreset, float TransitionTime)
{
    this->TargetPreset = TargetPreset;
    TransitionDuration = TransitionTime;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
}

ETimeOfDay ALightingSystemCore::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f)
        return ETimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

void ALightingSystemCore::SetDangerLighting(bool bIsDangerous)
{
    if (bIsDangerous)
    {
        SetEmotionalTone(EEmotionalTone::Terrifying);
    }
    else
    {
        SetEmotionalTone(EEmotionalTone::Tense);
    }
}

void ALightingSystemCore::SetSafeLighting()
{
    SetEmotionalTone(EEmotionalTone::Peaceful);
}

void ALightingSystemCore::UpdateLighting()
{
    UpdateSunPosition();
    UpdateAtmosphere();
    UpdateFog();
    UpdatePostProcess();
}

void ALightingSystemCore::UpdateSunPosition()
{
    if (!SunLight) return;

    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunAzimuth = (CurrentTimeOfDay / 24.0f) * 360.0f;

    // Set sun rotation
    FRotator SunRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
    SunLight->SetWorldRotation(SunRotation);

    // Adjust intensity based on sun elevation
    float IntensityMultiplier = FMath::Max(0.1f, FMath::Sin(FMath::DegreesToRadians(SunElevation + 10.0f)));
    SunLight->SetIntensity(CurrentPreset.SunIntensity * IntensityMultiplier);

    // Adjust color temperature based on time
    FLinearColor SunColor = CurrentPreset.SunColor;
    if (SunElevation < 10.0f) // Dawn/Dusk
    {
        SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Warm orange
    }
    else if (SunElevation < 30.0f) // Early morning/Late afternoon
    {
        SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm yellow
    }
    
    SunLight->SetLightColor(SunColor);
}

void ALightingSystemCore::UpdateAtmosphere()
{
    if (!SkyAtmosphere) return;

    // Update atmosphere based on current preset
    SkyAtmosphere->SetRayleighScatteringScale(CurrentPreset.AtmosphereHaziness);
    
    // Adjust atmosphere based on weather
    switch (CurrentWeather)
    {
        case EWeatherState::Clear:
            SkyAtmosphere->SetMieScatteringScale(0.003f);
            break;
        case EWeatherState::Overcast:
            SkyAtmosphere->SetMieScatteringScale(0.008f);
            break;
        case EWeatherState::Storm:
            SkyAtmosphere->SetMieScatteringScale(0.015f);
            break;
        default:
            SkyAtmosphere->SetMieScatteringScale(0.005f);
            break;
    }
}

void ALightingSystemCore::UpdateFog()
{
    if (!HeightFog) return;

    HeightFog->SetFogInscatteringColor(CurrentPreset.FogColor);
    HeightFog->SetFogDensity(CurrentPreset.FogDensity);
    HeightFog->SetFogHeightFalloff(CurrentPreset.FogHeightFalloff);

    // Emotional fog adjustments
    switch (CurrentEmotionalTone)
    {
        case EEmotionalTone::Terrifying:
            HeightFog->SetFogDensity(CurrentPreset.FogDensity * 2.0f);
            HeightFog->SetFogInscatteringColor(FLinearColor(0.3f, 0.3f, 0.4f, 1.0f));
            break;
        case EEmotionalTone::Mysterious:
            HeightFog->SetFogDensity(CurrentPreset.FogDensity * 1.5f);
            HeightFog->SetFogInscatteringColor(FLinearColor(0.4f, 0.4f, 0.6f, 1.0f));
            break;
        case EEmotionalTone::Peaceful:
            HeightFog->SetFogDensity(CurrentPreset.FogDensity * 0.7f);
            break;
    }
}

void ALightingSystemCore::UpdatePostProcess()
{
    if (!PostProcessComponent) return;

    // Update post-process settings based on current preset
    PostProcessComponent->Settings.bOverride_ColorContrast = true;
    PostProcessComponent->Settings.ColorContrast = FVector4(CurrentPreset.Contrast, CurrentPreset.Contrast, CurrentPreset.Contrast, 1.0f);

    PostProcessComponent->Settings.bOverride_ColorSaturation = true;
    PostProcessComponent->Settings.ColorSaturation = FVector4(CurrentPreset.Saturation, CurrentPreset.Saturation, CurrentPreset.Saturation, 1.0f);

    PostProcessComponent->Settings.bOverride_ColorGamma = true;
    PostProcessComponent->Settings.ColorGamma = FVector4(CurrentPreset.ColorGrading.R, CurrentPreset.ColorGrading.G, CurrentPreset.ColorGrading.B, 1.0f);
}

void ALightingSystemCore::ApplyEmotionalTone()
{
    if (!EmotionalPresets.Contains(CurrentEmotionalTone)) return;

    FLightingPreset EmotionalPreset = EmotionalPresets[CurrentEmotionalTone];
    
    // Blend current preset with emotional adjustments
    CurrentPreset.Contrast = FMath::Lerp(CurrentPreset.Contrast, EmotionalPreset.Contrast, 0.5f);
    CurrentPreset.Saturation = FMath::Lerp(CurrentPreset.Saturation, EmotionalPreset.Saturation, 0.5f);
    CurrentPreset.ShadowIntensity = FMath::Lerp(CurrentPreset.ShadowIntensity, EmotionalPreset.ShadowIntensity, 0.7f);
}

FLightingPreset ALightingSystemCore::GetCurrentPreset() const
{
    ETimeOfDay CurrentTimeEnum = GetCurrentTimeOfDayEnum();
    
    if (TimeOfDayPresets.Contains(CurrentTimeEnum))
    {
        return TimeOfDayPresets[CurrentTimeEnum];
    }
    
    return FLightingPreset();
}

FLightingPreset ALightingSystemCore::BlendPresets(const FLightingPreset& A, const FLightingPreset& B, float Alpha) const
{
    FLightingPreset Result;
    
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SkyColor = FMath::Lerp(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.AtmosphereHaziness = FMath::Lerp(A.AtmosphereHaziness, B.AtmosphereHaziness, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.ShadowIntensity = FMath::Lerp(A.ShadowIntensity, B.ShadowIntensity, Alpha);
    Result.Contrast = FMath::Lerp(A.Contrast, B.Contrast, Alpha);
    Result.Saturation = FMath::Lerp(A.Saturation, B.Saturation, Alpha);
    Result.ColorGrading = FMath::Lerp(A.ColorGrading, B.ColorGrading, Alpha);
    
    return Result;
}

void ALightingSystemCore::InitializeDefaultPresets()
{
    // Dawn Preset (5:00-7:00) - Mysterious, low visibility
    FLightingPreset DawnPreset;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnPreset.SunIntensity = 1.5f;
    DawnPreset.SkyColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DawnPreset.SkyIntensity = 0.3f;
    DawnPreset.FogColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
    DawnPreset.FogDensity = 0.08f;
    DawnPreset.FogHeightFalloff = 0.1f;
    DawnPreset.AtmosphereHaziness = 0.8f;
    DawnPreset.CloudCoverage = 0.6f;
    DawnPreset.ShadowIntensity = 0.9f;
    DawnPreset.Contrast = 1.4f;
    DawnPreset.Saturation = 0.7f;
    DawnPreset.ColorGrading = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Dawn, DawnPreset);

    // Morning Preset (7:00-11:00) - Tense but clearer
    FLightingPreset MorningPreset;
    MorningPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningPreset.SunIntensity = 2.5f;
    MorningPreset.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    MorningPreset.SkyIntensity = 0.6f;
    MorningPreset.FogColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
    MorningPreset.FogDensity = 0.04f;
    MorningPreset.FogHeightFalloff = 0.2f;
    MorningPreset.AtmosphereHaziness = 0.5f;
    MorningPreset.CloudCoverage = 0.4f;
    MorningPreset.ShadowIntensity = 0.8f;
    MorningPreset.Contrast = 1.2f;
    MorningPreset.Saturation = 0.9f;
    MorningPreset.ColorGrading = FLinearColor(1.0f, 0.98f, 0.95f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Morning, MorningPreset);

    // Midday Preset (11:00-14:00) - Harsh, high contrast
    FLightingPreset MiddayPreset;
    MiddayPreset.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    MiddayPreset.SunIntensity = 4.0f;
    MiddayPreset.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    MiddayPreset.SkyIntensity = 1.0f;
    MiddayPreset.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    MiddayPreset.FogDensity = 0.02f;
    MiddayPreset.FogHeightFalloff = 0.3f;
    MiddayPreset.AtmosphereHaziness = 0.3f;
    MiddayPreset.CloudCoverage = 0.3f;
    MiddayPreset.ShadowIntensity = 0.7f;
    MiddayPreset.Contrast = 1.5f;
    MiddayPreset.Saturation = 1.1f;
    MiddayPreset.ColorGrading = FLinearColor(1.0f, 1.0f, 0.98f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Midday, MiddayPreset);

    // Afternoon Preset (14:00-17:00) - Golden but ominous
    FLightingPreset AfternoonPreset;
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AfternoonPreset.SunIntensity = 3.0f;
    AfternoonPreset.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    AfternoonPreset.SkyIntensity = 0.8f;
    AfternoonPreset.FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    AfternoonPreset.FogDensity = 0.03f;
    AfternoonPreset.FogHeightFalloff = 0.25f;
    AfternoonPreset.AtmosphereHaziness = 0.4f;
    AfternoonPreset.CloudCoverage = 0.5f;
    AfternoonPreset.ShadowIntensity = 0.8f;
    AfternoonPreset.Contrast = 1.3f;
    AfternoonPreset.Saturation = 1.0f;
    AfternoonPreset.ColorGrading = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Afternoon, AfternoonPreset);

    // Dusk Preset (17:00-19:00) - Most dangerous time
    FLightingPreset DuskPreset;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskPreset.SunIntensity = 1.0f;
    DuskPreset.SkyColor = FLinearColor(0.8f, 0.3f, 0.2f, 1.0f);
    DuskPreset.SkyIntensity = 0.4f;
    DuskPreset.FogColor = FLinearColor(0.5f, 0.3f, 0.3f, 1.0f);
    DuskPreset.FogDensity = 0.06f;
    DuskPreset.FogHeightFalloff = 0.15f;
    DuskPreset.AtmosphereHaziness = 0.7f;
    DuskPreset.CloudCoverage = 0.7f;
    DuskPreset.ShadowIntensity = 0.95f;
    DuskPreset.Contrast = 1.6f;
    DuskPreset.Saturation = 0.8f;
    DuskPreset.ColorGrading = FLinearColor(1.0f, 0.8f, 0.7f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Dusk, DuskPreset);

    // Night Preset (19:00-5:00) - Maximum fear
    FLightingPreset NightPreset;
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightPreset.SunIntensity = 0.1f;
    NightPreset.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightPreset.SkyIntensity = 0.1f;
    NightPreset.FogColor = FLinearColor(0.2f, 0.3f, 0.4f, 1.0f);
    NightPreset.FogDensity = 0.1f;
    NightPreset.FogHeightFalloff = 0.1f;
    NightPreset.AtmosphereHaziness = 0.9f;
    NightPreset.CloudCoverage = 0.8f;
    NightPreset.ShadowIntensity = 1.0f;
    NightPreset.Contrast = 2.0f;
    NightPreset.Saturation = 0.6f;
    NightPreset.ColorGrading = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(ETimeOfDay::Night, NightPreset);

    // Initialize emotional presets
    InitializeEmotionalPresets();
}

void ALightingSystemCore::InitializeEmotionalPresets()
{
    // Peaceful - Rare safe moments
    FLightingPreset PeacefulPreset;
    PeacefulPreset.Contrast = 1.0f;
    PeacefulPreset.Saturation = 1.2f;
    PeacefulPreset.ShadowIntensity = 0.6f;
    PeacefulPreset.ColorGrading = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Peaceful, PeacefulPreset);

    // Tense - Default state
    FLightingPreset TensePreset;
    TensePreset.Contrast = 1.3f;
    TensePreset.Saturation = 0.9f;
    TensePreset.ShadowIntensity = 0.8f;
    TensePreset.ColorGrading = FLinearColor(0.98f, 0.95f, 0.92f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Tense, TensePreset);

    // Terrifying - Immediate danger
    FLightingPreset TerrifyingPreset;
    TerrifyingPreset.Contrast = 2.0f;
    TerrifyingPreset.Saturation = 0.7f;
    TerrifyingPreset.ShadowIntensity = 1.0f;
    TerrifyingPreset.ColorGrading = FLinearColor(0.9f, 0.85f, 0.8f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Terrifying, TerrifyingPreset);

    // Mysterious - Unknown presence
    FLightingPreset MysteriousPreset;
    MysteriousPreset.Contrast = 1.5f;
    MysteriousPreset.Saturation = 0.8f;
    MysteriousPreset.ShadowIntensity = 0.9f;
    MysteriousPreset.ColorGrading = FLinearColor(0.95f, 0.9f, 1.0f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Mysterious, MysteriousPreset);

    // Melancholy - Loss/sadness
    FLightingPreset MelancholyPreset;
    MelancholyPreset.Contrast = 1.1f;
    MelancholyPreset.Saturation = 0.6f;
    MelancholyPreset.ShadowIntensity = 0.7f;
    MelancholyPreset.ColorGrading = FLinearColor(0.9f, 0.9f, 0.95f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Melancholy, MelancholyPreset);

    // Wonder - Discovery moments
    FLightingPreset WonderPreset;
    WonderPreset.Contrast = 1.2f;
    WonderPreset.Saturation = 1.3f;
    WonderPreset.ShadowIntensity = 0.5f;
    WonderPreset.ColorGrading = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    EmotionalPresets.Add(EEmotionalTone::Wonder, WonderPreset);
}
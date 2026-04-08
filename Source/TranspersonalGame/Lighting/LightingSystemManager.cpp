#include "LightingSystemManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

ALightingSystemManager::ALightingSystemManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create Sun Light (Directional Light)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(10.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetCastShadows(true);
    SunLight->SetMobility(EComponentMobility::Movable);
    SunLight->SetAtmosphereSunLight(true);
    SunLight->SetAtmosphereSunLightIndex(0);

    // Create Sky Light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor::White);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetRealTimeCapture(true);

    // Create Sky Atmosphere
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create Volumetric Clouds
    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Create Height Fog
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.447f, 0.638f, 1.0f));

    // Create Post Process Component
    PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
    PostProcessComponent->SetupAttachment(RootComponent);
    PostProcessComponent->bUnbound = true;

    // Initialize default presets
    InitializeDefaultPresets();
}

void ALightingSystemManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial lighting setup
    UpdateLightingComponents();
}

void ALightingSystemManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoTimeProgression)
    {
        UpdateTimeProgression(DeltaTime);
    }

    if (bAutoWeatherProgression)
    {
        UpdateWeatherProgression(DeltaTime);
    }

    UpdateLightingComponents();
}

void ALightingSystemManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLightingComponents();
}

ETimeOfDay ALightingSystemManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ETimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ETimeOfDay::Dusk;
    else if (CurrentTimeOfDay >= 20.0f && CurrentTimeOfDay < 23.0f)
        return ETimeOfDay::Night;
    else
        return ETimeOfDay::DeepNight;
}

void ALightingSystemManager::SetDayDuration(float NewDurationInMinutes)
{
    DayDurationInMinutes = FMath::Max(NewDurationInMinutes, 1.0f);
}

void ALightingSystemManager::SetWeatherState(EWeatherState NewWeatherState)
{
    CurrentWeatherState = NewWeatherState;
    bIsTransitioningWeather = false;
    UpdateLightingComponents();
}

void ALightingSystemManager::TransitionToWeather(EWeatherState NewWeatherState, float TransitionDuration)
{
    if (NewWeatherState != CurrentWeatherState)
    {
        TargetWeatherState = NewWeatherState;
        WeatherTransitionDuration = TransitionDuration;
        WeatherTransitionTimer = 0.0f;
        bIsTransitioningWeather = true;
    }
}

void ALightingSystemManager::SetEmotionalTone(EEmotionalTone NewTone, float Intensity)
{
    CurrentEmotionalTone = NewTone;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    bIsTransitioningEmotion = false;
    UpdateLightingComponents();
}

void ALightingSystemManager::BlendToEmotionalTone(EEmotionalTone NewTone, float Intensity, float BlendDuration)
{
    if (NewTone != CurrentEmotionalTone)
    {
        TargetEmotionalTone = NewTone;
        TargetEmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        EmotionalTransitionDuration = BlendDuration;
        EmotionalTransitionTimer = 0.0f;
        bIsTransitioningEmotion = true;
    }
}

void ALightingSystemManager::TriggerThreatLighting(float ThreatLevelParam, float Duration)
{
    if (!bThreatActive)
    {
        // Store current state
        PreThreatPreset = GetCurrentTimePreset();
        
        // Apply threat modifications
        ThreatLevel = FMath::Clamp(ThreatLevelParam, 0.0f, 1.0f);
        ThreatDuration = Duration;
        ThreatTimer = 0.0f;
        bThreatActive = true;
        
        // Force emotional tone to threatening
        SetEmotionalTone(EEmotionalTone::Threatening, ThreatLevel);
    }
}

void ALightingSystemManager::RestoreNormalLighting(float RestoreDuration)
{
    if (bThreatActive)
    {
        bThreatActive = false;
        BlendToEmotionalTone(EEmotionalTone::Peaceful, 0.5f, RestoreDuration);
    }
}

void ALightingSystemManager::UpdateTimeProgression(float DeltaTime)
{
    float TimeIncrement = (24.0f / (DayDurationInMinutes * 60.0f)) * DeltaTime;
    CurrentTimeOfDay += TimeIncrement;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }
}

void ALightingSystemManager::UpdateWeatherProgression(float DeltaTime)
{
    if (bIsTransitioningWeather)
    {
        WeatherTransitionTimer += DeltaTime;
        float TransitionAlpha = WeatherTransitionTimer / WeatherTransitionDuration;
        
        if (TransitionAlpha >= 1.0f)
        {
            CurrentWeatherState = TargetWeatherState;
            bIsTransitioningWeather = false;
        }
    }
    
    if (bIsTransitioningEmotion)
    {
        EmotionalTransitionTimer += DeltaTime;
        float TransitionAlpha = EmotionalTransitionTimer / EmotionalTransitionDuration;
        
        if (TransitionAlpha >= 1.0f)
        {
            CurrentEmotionalTone = TargetEmotionalTone;
            EmotionalIntensity = TargetEmotionalIntensity;
            bIsTransitioningEmotion = false;
        }
        else
        {
            // Smooth interpolation for emotional intensity
            float CurrentIntensity = FMath::Lerp(EmotionalIntensity, TargetEmotionalIntensity, TransitionAlpha);
            EmotionalIntensity = CurrentIntensity;
        }
    }
    
    // Update threat system
    if (bThreatActive)
    {
        ThreatTimer += DeltaTime;
        if (ThreatTimer >= ThreatDuration)
        {
            RestoreNormalLighting(3.0f);
        }
    }
}

void ALightingSystemManager::UpdateLightingComponents()
{
    // Get current presets
    FLightingPreset TimePreset = GetCurrentTimePreset();
    FLightingPreset WeatherPreset = GetCurrentWeatherPreset();
    FLightingPreset EmotionalPreset = GetCurrentEmotionalPreset();
    
    // Blend presets based on priorities
    FLightingPreset FinalPreset = TimePreset;
    
    // Apply weather modifications
    BlendLightingPresets(FinalPreset, WeatherPreset, 0.7f);
    
    // Apply emotional tone modifications
    BlendLightingPresets(FinalPreset, EmotionalPreset, EmotionalIntensity);
    
    // Apply threat modifications if active
    if (bThreatActive)
    {
        FLightingPreset ThreatPreset;
        ThreatPreset.SunColor = FLinearColor(1.0f, 0.3f, 0.1f); // Reddish threatening light
        ThreatPreset.SunIntensity = FinalPreset.SunIntensity * (1.0f + ThreatLevel * 0.5f);
        ThreatPreset.FogColor = FLinearColor(0.8f, 0.2f, 0.1f);
        ThreatPreset.FogDensity = FinalPreset.FogDensity * (1.0f + ThreatLevel);
        ThreatPreset.ColorTemperature = 3000.0f; // Warmer, more threatening
        
        BlendLightingPresets(FinalPreset, ThreatPreset, ThreatLevel);
    }
    
    // Apply final preset to components
    ApplyLightingPreset(FinalPreset);
}

void ALightingSystemManager::ApplyLightingPreset(const FLightingPreset& Preset, float BlendWeight)
{
    if (!SunLight || !SkyLight || !HeightFog) return;
    
    // Update Sun Light
    float SunAngle = CalculateSunAngle(CurrentTimeOfDay);
    FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
    SunLight->SetWorldRotation(SunRotation);
    SunLight->SetLightColor(Preset.SunColor);
    SunLight->SetIntensity(Preset.SunIntensity);
    
    // Update Sky Light
    SkyLight->SetLightColor(Preset.SkyLightColor);
    SkyLight->SetIntensity(Preset.SkyLightIntensity);
    
    // Update Height Fog
    HeightFog->SetFogInscatteringColor(Preset.FogColor);
    HeightFog->SetFogDensity(Preset.FogDensity);
    HeightFog->SetFogHeightFalloff(Preset.FogHeightFalloff);
    
    // Update Post Process
    if (PostProcessComponent)
    {
        FPostProcessSettings& Settings = PostProcessComponent->Settings;
        Settings.bOverride_WhiteTemp = true;
        Settings.WhiteTemp = Preset.ColorTemperature;
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = Preset.Exposure;
    }
}

float ALightingSystemManager::CalculateSunAngle(float TimeOfDay) const
{
    // Convert time to angle (0-360 degrees)
    // Sunrise at 6:00 (90 degrees), Sunset at 18:00 (-90 degrees)
    float NormalizedTime = (TimeOfDay - 6.0f) / 12.0f; // Normalize to 0-1 for day portion
    float SunAngle = FMath::Cos(NormalizedTime * PI) * 90.0f;
    
    return SunAngle;
}

void ALightingSystemManager::InitializeDefaultPresets()
{
    // Initialize Time of Day presets
    FLightingPreset DawnPreset;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
    DawnPreset.SunIntensity = 5.0f;
    DawnPreset.SkyLightColor = FLinearColor(0.8f, 0.9f, 1.0f);
    DawnPreset.SkyLightIntensity = 0.3f;
    DawnPreset.FogColor = FLinearColor(1.0f, 0.9f, 0.8f);
    DawnPreset.FogDensity = 0.05f;
    DawnPreset.ColorTemperature = 3500.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Dawn, DawnPreset);
    
    FLightingPreset MiddayPreset;
    MiddayPreset.SunColor = FLinearColor::White;
    MiddayPreset.SunIntensity = 15.0f;
    MiddayPreset.SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f);
    MiddayPreset.SkyLightIntensity = 1.0f;
    MiddayPreset.FogColor = FLinearColor(0.7f, 0.8f, 1.0f);
    MiddayPreset.FogDensity = 0.02f;
    MiddayPreset.ColorTemperature = 6500.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Midday, MiddayPreset);
    
    FLightingPreset DuskPreset;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
    DuskPreset.SunIntensity = 8.0f;
    DuskPreset.SkyLightColor = FLinearColor(0.6f, 0.4f, 0.8f);
    DuskPreset.SkyLightIntensity = 0.4f;
    DuskPreset.FogColor = FLinearColor(0.8f, 0.5f, 0.3f);
    DuskPreset.FogDensity = 0.04f;
    DuskPreset.ColorTemperature = 2800.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Dusk, DuskPreset);
    
    FLightingPreset NightPreset;
    NightPreset.SunColor = FLinearColor(0.1f, 0.2f, 0.4f);
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SkyLightColor = FLinearColor(0.2f, 0.3f, 0.6f);
    NightPreset.SkyLightIntensity = 0.1f;
    NightPreset.FogColor = FLinearColor(0.1f, 0.2f, 0.4f);
    NightPreset.FogDensity = 0.03f;
    NightPreset.ColorTemperature = 4000.0f;
    TimeOfDayPresets.Add(ETimeOfDay::Night, NightPreset);
    
    // Initialize Emotional Tone presets
    FLightingPreset ThreateningPreset;
    ThreateningPreset.SunColor = FLinearColor(1.0f, 0.4f, 0.2f);
    ThreateningPreset.SunIntensity = 12.0f;
    ThreateningPreset.FogColor = FLinearColor(0.6f, 0.3f, 0.2f);
    ThreateningPreset.FogDensity = 0.06f;
    ThreateningPreset.ColorTemperature = 3000.0f;
    EmotionalPresets.Add(EEmotionalTone::Threatening, ThreateningPreset);
    
    FLightingPreset MysteriousPreset;
    MysteriousPreset.SunColor = FLinearColor(0.8f, 0.8f, 1.0f);
    MysteriousPreset.SunIntensity = 6.0f;
    MysteriousPreset.FogColor = FLinearColor(0.5f, 0.6f, 0.8f);
    MysteriousPreset.FogDensity = 0.08f;
    MysteriousPreset.ColorTemperature = 7000.0f;
    EmotionalPresets.Add(EEmotionalTone::Mysterious, MysteriousPreset);
}
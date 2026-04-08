#include "LightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ALightingManager::ALightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default presets
    InitializeDefaultPresets();
}

void ALightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    
    // Set initial lighting state
    SetTimeOfDay(CurrentTimeOfDay);
}

void ALightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateTimeOfDay(DeltaTime);
    
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentPreset = TargetPreset;
        }
        
        InterpolateLighting(CurrentPreset, TargetPreset, TransitionProgress);
    }
}

void ALightingManager::InitializeLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find or create sun light
    SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(World, ADirectionalLight::StaticClass()));
    if (!SunLight)
    {
        SunLight = World->SpawnActor<ADirectionalLight>();
        SunLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        SunLight->GetLightComponent()->SetAtmosphereSunLight(true);
        SunLight->GetLightComponent()->SetCastShadows(true);
        SunLight->GetLightComponent()->SetCastCloudShadows(true);
        SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
    }
    
    // Find or create sky light
    SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(World, ASkyLight::StaticClass()));
    if (!SkyLight)
    {
        SkyLight = World->SpawnActor<ASkyLight>();
        SkyLight->GetLightComponent()->SetMobility(EComponentMobility::Movable);
        SkyLight->GetLightComponent()->bRealTimeCapture = true;
        SkyLight->GetLightComponent()->SetCloudAmbientOcclusion(true);
    }
    
    // Find or create sky atmosphere
    SkyAtmosphere = Cast<ASkyAtmosphere>(UGameplayStatics::GetActorOfClass(World, ASkyAtmosphere::StaticClass()));
    if (!SkyAtmosphere)
    {
        SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>();
    }
    
    // Find or create volumetric clouds
    VolumetricClouds = Cast<AVolumetricCloud>(UGameplayStatics::GetActorOfClass(World, AVolumetricCloud::StaticClass()));
    if (!VolumetricClouds)
    {
        VolumetricClouds = World->SpawnActor<AVolumetricCloud>();
    }
    
    // Find or create height fog
    HeightFog = Cast<AExponentialHeightFog>(UGameplayStatics::GetActorOfClass(World, AExponentialHeightFog::StaticClass()));
    if (!HeightFog)
    {
        HeightFog = World->SpawnActor<AExponentialHeightFog>();
        HeightFog->GetComponent()->SetVolumetricFog(true);
    }
}

void ALightingManager::UpdateTimeOfDay(float DeltaTime)
{
    if (TimeOfDaySpeed > 0.0f)
    {
        CurrentTimeOfDay += DeltaTime * TimeOfDaySpeed;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        // Update sun rotation based on time
        if (SunLight)
        {
            float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 6 AM = 0 degrees
            FRotator SunRotation = FRotator(-SunAngle, 30.0f, 0.0f);
            SunLight->SetActorRotation(SunRotation);
        }
        
        // Apply time-based lighting changes
        ETimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
        if (TimeOfDayPresets.Contains(TimeEnum))
        {
            FLightingPreset BasePreset = TimeOfDayPresets[TimeEnum];
            
            // Blend with weather and threat modifiers
            if (bEnableDynamicWeather && WeatherPresets.Contains(CurrentWeather))
            {
                BasePreset = BlendPresets(BasePreset, WeatherPresets[CurrentWeather], 0.7f);
            }
            
            if (bEnableThreatLighting && ThreatPresets.Contains(CurrentThreatLevel))
            {
                BasePreset = BlendPresets(BasePreset, ThreatPresets[CurrentThreatLevel], 0.5f);
            }
            
            ApplyLightingPreset(BasePreset);
        }
    }
}

void ALightingManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
}

void ALightingManager::SetWeatherState(EWeatherState NewWeather)
{
    CurrentWeather = NewWeather;
}

void ALightingManager::SetThreatLevel(EThreatLevel NewThreatLevel)
{
    CurrentThreatLevel = NewThreatLevel;
}

void ALightingManager::TransitionToPreset(const FLightingPreset& TargetPreset, float Duration)
{
    this->TargetPreset = TargetPreset;
    TransitionDuration = Duration;
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
}

ETimeOfDay ALightingManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
        return ETimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 11.0f)
        return ETimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ETimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ETimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 21.0f)
        return ETimeOfDay::Dusk;
    else
        return ETimeOfDay::Night;
}

FLightingPreset ALightingManager::GetCurrentLightingState() const
{
    return CurrentPreset;
}

void ALightingManager::ApplyLightingPreset(const FLightingPreset& Preset)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetLightColor(Preset.SunColor);
        SunLight->GetLightComponent()->SetIntensity(Preset.SunIntensity);
    }
    
    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetLightColor(Preset.SkyColor);
    }
    
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(Preset.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(Preset.FogColor);
    }
    
    CurrentPreset = Preset;
}

void ALightingManager::InterpolateLighting(const FLightingPreset& From, const FLightingPreset& To, float Alpha)
{
    FLightingPreset InterpolatedPreset;
    
    InterpolatedPreset.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedPreset.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedPreset.SkyColor = FMath::Lerp(From.SkyColor, To.SkyColor, Alpha);
    InterpolatedPreset.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedPreset.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);
    InterpolatedPreset.CloudCoverage = FMath::Lerp(From.CloudCoverage, To.CloudCoverage, Alpha);
    InterpolatedPreset.AtmosphericPerspective = FMath::Lerp(From.AtmosphericPerspective, To.AtmosphericPerspective, Alpha);
    
    ApplyLightingPreset(InterpolatedPreset);
}

FLightingPreset ALightingManager::BlendPresets(const FLightingPreset& Base, const FLightingPreset& Overlay, float Weight)
{
    FLightingPreset BlendedPreset;
    
    BlendedPreset.SunColor = FMath::Lerp(Base.SunColor, Overlay.SunColor, Weight);
    BlendedPreset.SunIntensity = FMath::Lerp(Base.SunIntensity, Overlay.SunIntensity, Weight);
    BlendedPreset.SkyColor = FMath::Lerp(Base.SkyColor, Overlay.SkyColor, Weight);
    BlendedPreset.FogDensity = FMath::Lerp(Base.FogDensity, Overlay.FogDensity, Weight);
    BlendedPreset.FogColor = FMath::Lerp(Base.FogColor, Overlay.FogColor, Weight);
    BlendedPreset.CloudCoverage = FMath::Lerp(Base.CloudCoverage, Overlay.CloudCoverage, Weight);
    BlendedPreset.AtmosphericPerspective = FMath::Lerp(Base.AtmosphericPerspective, Overlay.AtmosphericPerspective, Weight);
    
    return BlendedPreset;
}

void ALightingManager::InitializeDefaultPresets()
{
    // Dawn - Soft, warm light with high contrast shadows
    FLightingPreset DawnPreset;
    DawnPreset.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnPreset.SunIntensity = 3.0f;
    DawnPreset.SkyColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    DawnPreset.FogDensity = 0.05f;
    DawnPreset.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DawnPreset.CloudCoverage = 0.3f;
    TimeOfDayPresets.Add(ETimeOfDay::Dawn, DawnPreset);
    
    // Morning - Clear, bright light with moderate shadows
    FLightingPreset MorningPreset;
    MorningPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningPreset.SunIntensity = 8.0f;
    MorningPreset.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    MorningPreset.FogDensity = 0.02f;
    MorningPreset.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningPreset.CloudCoverage = 0.4f;
    TimeOfDayPresets.Add(ETimeOfDay::Morning, MorningPreset);
    
    // Noon - Harsh, bright light with sharp shadows
    FLightingPreset NoonPreset;
    NoonPreset.SunColor = FLinearColor::White;
    NoonPreset.SunIntensity = 12.0f;
    NoonPreset.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    NoonPreset.FogDensity = 0.01f;
    NoonPreset.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    NoonPreset.CloudCoverage = 0.2f;
    TimeOfDayPresets.Add(ETimeOfDay::Noon, NoonPreset);
    
    // Afternoon - Warm, golden light
    FLightingPreset AfternoonPreset;
    AfternoonPreset.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonPreset.SunIntensity = 10.0f;
    AfternoonPreset.SkyColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    AfternoonPreset.FogDensity = 0.015f;
    AfternoonPreset.FogColor = FLinearColor(0.8f, 0.8f, 0.7f, 1.0f);
    AfternoonPreset.CloudCoverage = 0.3f;
    TimeOfDayPresets.Add(ETimeOfDay::Afternoon, AfternoonPreset);
    
    // Dusk - Dramatic, orange-red light with long shadows
    FLightingPreset DuskPreset;
    DuskPreset.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskPreset.SunIntensity = 4.0f;
    DuskPreset.SkyColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    DuskPreset.FogDensity = 0.04f;
    DuskPreset.FogColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskPreset.CloudCoverage = 0.5f;
    TimeOfDayPresets.Add(ETimeOfDay::Dusk, DuskPreset);
    
    // Night - Cool, blue moonlight with deep shadows
    FLightingPreset NightPreset;
    NightPreset.SunColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightPreset.SunIntensity = 0.5f;
    NightPreset.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightPreset.FogDensity = 0.03f;
    NightPreset.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    NightPreset.CloudCoverage = 0.6f;
    TimeOfDayPresets.Add(ETimeOfDay::Night, NightPreset);
    
    // Weather Presets
    FLightingPreset StormPreset;
    StormPreset.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormPreset.SunIntensity = 2.0f;
    StormPreset.SkyColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    StormPreset.FogDensity = 0.08f;
    StormPreset.FogColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    StormPreset.CloudCoverage = 0.9f;
    WeatherPresets.Add(EWeatherState::Storm, StormPreset);
    
    // Threat Level Presets
    FLightingPreset TerrorPreset;
    TerrorPreset.SunColor = FLinearColor(0.8f, 0.4f, 0.4f, 1.0f);
    TerrorPreset.SunIntensity = 0.7f;
    TerrorPreset.SkyColor = FLinearColor(0.4f, 0.2f, 0.2f, 1.0f);
    TerrorPreset.FogDensity = 0.06f;
    TerrorPreset.FogColor = FLinearColor(0.5f, 0.3f, 0.3f, 1.0f);
    TerrorPreset.CloudCoverage = 0.8f;
    ThreatPresets.Add(EThreatLevel::Terror, TerrorPreset);
}
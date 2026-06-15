#include "EnvArt_AtmosphericLighting.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEnvArt_AtmosphericLighting::UEnvArt_AtmosphericLighting()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    SunLight = nullptr;
    SkyLight = nullptr;
    HeightFog = nullptr;
}

void UEnvArt_AtmosphericLighting::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTimeOfDayPresets();
    InitializeWeatherModifiers();
    FindLightingActors();
    
    // Apply initial lighting settings
    if (TimeOfDayPresets.Contains(CurrentTimeOfDay))
    {
        ApplyLightingSettings(TimeOfDayPresets[CurrentTimeOfDay]);
    }
}

void UEnvArt_AtmosphericLighting::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsTransitioning)
    {
        TransitionTimer += DeltaTime;
        float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
        
        // Interpolate between start and target settings
        FEnvArt_LightingSettings InterpolatedSettings;
        InterpolatedSettings.SunIntensity = FMath::Lerp(TransitionStartSettings.SunIntensity, TransitionTargetSettings.SunIntensity, Alpha);
        InterpolatedSettings.SunColor = FMath::Lerp(TransitionStartSettings.SunColor, TransitionTargetSettings.SunColor, Alpha);
        InterpolatedSettings.SunRotation = FMath::Lerp(TransitionStartSettings.SunRotation, TransitionTargetSettings.SunRotation, Alpha);
        InterpolatedSettings.SkyLightIntensity = FMath::Lerp(TransitionStartSettings.SkyLightIntensity, TransitionTargetSettings.SkyLightIntensity, Alpha);
        InterpolatedSettings.SkyLightColor = FMath::Lerp(TransitionStartSettings.SkyLightColor, TransitionTargetSettings.SkyLightColor, Alpha);
        InterpolatedSettings.FogDensity = FMath::Lerp(TransitionStartSettings.FogDensity, TransitionTargetSettings.FogDensity, Alpha);
        InterpolatedSettings.FogHeightFalloff = FMath::Lerp(TransitionStartSettings.FogHeightFalloff, TransitionTargetSettings.FogHeightFalloff, Alpha);
        InterpolatedSettings.FogInscatteringColor = FMath::Lerp(TransitionStartSettings.FogInscatteringColor, TransitionTargetSettings.FogInscatteringColor, Alpha);
        InterpolatedSettings.bVolumetricFog = TransitionTargetSettings.bVolumetricFog;
        
        ApplySettingsToActors(InterpolatedSettings);
        CurrentSettings = InterpolatedSettings;
        
        if (Alpha >= 1.0f)
        {
            bIsTransitioning = false;
            TransitionTimer = 0.0f;
        }
    }
}

void UEnvArt_AtmosphericLighting::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (TimeOfDayPresets.Contains(NewTimeOfDay))
    {
        FEnvArt_LightingSettings NewSettings = TimeOfDayPresets[NewTimeOfDay];
        
        // Apply weather modifiers
        if (WeatherModifiers.Contains(CurrentWeather))
        {
            const FEnvArt_LightingSettings& WeatherMod = WeatherModifiers[CurrentWeather];
            NewSettings.SunIntensity *= WeatherMod.SunIntensity;
            NewSettings.FogDensity += WeatherMod.FogDensity;
            NewSettings.FogHeightFalloff *= WeatherMod.FogHeightFalloff;
        }
        
        ApplyLightingSettings(NewSettings);
    }
}

void UEnvArt_AtmosphericLighting::SetWeather(EEnvArt_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    // Reapply current time of day with new weather modifiers
    SetTimeOfDay(CurrentTimeOfDay);
}

void UEnvArt_AtmosphericLighting::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    CurrentSettings = Settings;
    ApplySettingsToActors(Settings);
}

void UEnvArt_AtmosphericLighting::TransitionToTimeOfDay(EEnvArt_TimeOfDay TargetTime, float TransitionDuration)
{
    if (!TimeOfDayPresets.Contains(TargetTime))
    {
        return;
    }
    
    CurrentTimeOfDay = TargetTime;
    this->TransitionDuration = TransitionDuration;
    TransitionTimer = 0.0f;
    TransitionStartSettings = CurrentSettings;
    TransitionTargetSettings = TimeOfDayPresets[TargetTime];
    
    // Apply weather modifiers to target
    if (WeatherModifiers.Contains(CurrentWeather))
    {
        const FEnvArt_LightingSettings& WeatherMod = WeatherModifiers[CurrentWeather];
        TransitionTargetSettings.SunIntensity *= WeatherMod.SunIntensity;
        TransitionTargetSettings.FogDensity += WeatherMod.FogDensity;
        TransitionTargetSettings.FogHeightFalloff *= WeatherMod.FogHeightFalloff;
    }
    
    bIsTransitioning = true;
}

void UEnvArt_AtmosphericLighting::CreateGoldenHourLighting()
{
    FEnvArt_LightingSettings GoldenHour;
    GoldenHour.SunIntensity = 4.0f;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    GoldenHour.SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
    GoldenHour.SkyLightIntensity = 0.8f;
    GoldenHour.SkyLightColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    GoldenHour.FogDensity = 0.015f;
    GoldenHour.FogHeightFalloff = 0.15f;
    GoldenHour.bVolumetricFog = true;
    GoldenHour.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    ApplyLightingSettings(GoldenHour);
}

void UEnvArt_AtmosphericLighting::CreateMysticalForestLighting()
{
    FEnvArt_LightingSettings Mystical;
    Mystical.SunIntensity = 2.0f;
    Mystical.SunColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    Mystical.SunRotation = FRotator(-60.0f, 120.0f, 0.0f);
    Mystical.SkyLightIntensity = 1.2f;
    Mystical.SkyLightColor = FLinearColor(0.4f, 0.6f, 0.9f, 1.0f);
    Mystical.FogDensity = 0.08f;
    Mystical.FogHeightFalloff = 0.1f;
    Mystical.bVolumetricFog = true;
    Mystical.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    
    ApplyLightingSettings(Mystical);
}

void UEnvArt_AtmosphericLighting::CreateVolcanicAtmosphere()
{
    FEnvArt_LightingSettings Volcanic;
    Volcanic.SunIntensity = 1.5f;
    Volcanic.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    Volcanic.SunRotation = FRotator(-30.0f, 90.0f, 0.0f);
    Volcanic.SkyLightIntensity = 0.6f;
    Volcanic.SkyLightColor = FLinearColor(0.8f, 0.4f, 0.3f, 1.0f);
    Volcanic.FogDensity = 0.12f;
    Volcanic.FogHeightFalloff = 0.05f;
    Volcanic.bVolumetricFog = true;
    Volcanic.FogInscatteringColor = FLinearColor(0.9f, 0.5f, 0.3f, 1.0f);
    
    ApplyLightingSettings(Volcanic);
}

void UEnvArt_AtmosphericLighting::RefreshLightingReferences()
{
    FindLightingActors();
}

FEnvArt_LightingSettings UEnvArt_AtmosphericLighting::GetCurrentLightingSettings() const
{
    return CurrentSettings;
}

void UEnvArt_AtmosphericLighting::InitializeTimeOfDayPresets()
{
    // Dawn
    FEnvArt_LightingSettings Dawn;
    Dawn.SunIntensity = 2.0f;
    Dawn.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    Dawn.SunRotation = FRotator(-10.0f, 75.0f, 0.0f);
    Dawn.SkyLightIntensity = 0.5f;
    Dawn.SkyLightColor = FLinearColor(0.8f, 0.6f, 0.9f, 1.0f);
    Dawn.FogDensity = 0.05f;
    Dawn.FogHeightFalloff = 0.2f;
    Dawn.bVolumetricFog = true;
    Dawn.FogInscatteringColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Dawn, Dawn);
    
    // Morning
    FEnvArt_LightingSettings Morning;
    Morning.SunIntensity = 3.0f;
    Morning.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    Morning.SunRotation = FRotator(-30.0f, 60.0f, 0.0f);
    Morning.SkyLightIntensity = 0.8f;
    Morning.SkyLightColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    Morning.FogDensity = 0.03f;
    Morning.FogHeightFalloff = 0.25f;
    Morning.bVolumetricFog = true;
    Morning.FogInscatteringColor = FLinearColor(0.9f, 0.9f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Morning, Morning);
    
    // Midday
    FEnvArt_LightingSettings Midday;
    Midday.SunIntensity = 5.0f;
    Midday.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    Midday.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    Midday.SkyLightIntensity = 1.0f;
    Midday.SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    Midday.FogDensity = 0.01f;
    Midday.FogHeightFalloff = 0.3f;
    Midday.bVolumetricFog = false;
    Midday.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Midday, Midday);
    
    // Afternoon (Default)
    FEnvArt_LightingSettings Afternoon;
    Afternoon.SunIntensity = 3.5f;
    Afternoon.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    Afternoon.SunRotation = FRotator(-45.0f, 45.0f, 0.0f);
    Afternoon.SkyLightIntensity = 1.0f;
    Afternoon.SkyLightColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    Afternoon.FogDensity = 0.02f;
    Afternoon.FogHeightFalloff = 0.2f;
    Afternoon.bVolumetricFog = true;
    Afternoon.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Afternoon, Afternoon);
    
    // Dusk
    FEnvArt_LightingSettings Dusk;
    Dusk.SunIntensity = 2.5f;
    Dusk.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    Dusk.SunRotation = FRotator(-5.0f, 105.0f, 0.0f);
    Dusk.SkyLightIntensity = 0.6f;
    Dusk.SkyLightColor = FLinearColor(0.9f, 0.5f, 0.7f, 1.0f);
    Dusk.FogDensity = 0.04f;
    Dusk.FogHeightFalloff = 0.15f;
    Dusk.bVolumetricFog = true;
    Dusk.FogInscatteringColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Dusk, Dusk);
    
    // Night
    FEnvArt_LightingSettings Night;
    Night.SunIntensity = 0.1f;
    Night.SunColor = FLinearColor(0.3f, 0.3f, 0.8f, 1.0f);
    Night.SunRotation = FRotator(10.0f, 180.0f, 0.0f);
    Night.SkyLightIntensity = 0.3f;
    Night.SkyLightColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f);
    Night.FogDensity = 0.06f;
    Night.FogHeightFalloff = 0.1f;
    Night.bVolumetricFog = true;
    Night.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.9f, 1.0f);
    TimeOfDayPresets.Add(EEnvArt_TimeOfDay::Night, Night);
}

void UEnvArt_AtmosphericLighting::InitializeWeatherModifiers()
{
    // Clear weather (no modifiers)
    FEnvArt_LightingSettings Clear;
    Clear.SunIntensity = 1.0f;
    Clear.FogDensity = 0.0f;
    Clear.FogHeightFalloff = 1.0f;
    WeatherModifiers.Add(EEnvArt_WeatherType::Clear, Clear);
    
    // Overcast
    FEnvArt_LightingSettings Overcast;
    Overcast.SunIntensity = 0.6f;
    Overcast.FogDensity = 0.02f;
    Overcast.FogHeightFalloff = 0.8f;
    WeatherModifiers.Add(EEnvArt_WeatherType::Overcast, Overcast);
    
    // Foggy
    FEnvArt_LightingSettings Foggy;
    Foggy.SunIntensity = 0.3f;
    Foggy.FogDensity = 0.1f;
    Foggy.FogHeightFalloff = 0.5f;
    WeatherModifiers.Add(EEnvArt_WeatherType::Foggy, Foggy);
    
    // Stormy
    FEnvArt_LightingSettings Stormy;
    Stormy.SunIntensity = 0.2f;
    Stormy.FogDensity = 0.05f;
    Stormy.FogHeightFalloff = 0.6f;
    WeatherModifiers.Add(EEnvArt_WeatherType::Stormy, Stormy);
    
    // Dusty
    FEnvArt_LightingSettings Dusty;
    Dusty.SunIntensity = 0.7f;
    Dusty.FogDensity = 0.08f;
    Dusty.FogHeightFalloff = 0.3f;
    WeatherModifiers.Add(EEnvArt_WeatherType::Dusty, Dusty);
}

void UEnvArt_AtmosphericLighting::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }
    
    // Find height fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void UEnvArt_AtmosphericLighting::ApplySettingsToActors(const FEnvArt_LightingSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        SunLight->SetActorRotation(Settings.SunRotation);
    }
    
    // Apply sky light settings
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        SkyComp->SetIntensity(Settings.SkyLightIntensity);
        SkyComp->SetLightColor(Settings.SkyLightColor);
    }
    
    // Apply fog settings
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetVolumetricFog(Settings.bVolumetricFog);
        FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
    }
}
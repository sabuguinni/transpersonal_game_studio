#include "EnvArt_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default atmosphere settings for golden hour
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f);
    AtmosphereSettings.SunIntensity = 3.0f;
    AtmosphereSettings.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogHeightFalloff = 0.2f;
    AtmosphereSettings.FogMaxOpacity = 0.6f;
    AtmosphereSettings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f);
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindAtmosphereActors();
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
    
    if (bEnableWeatherTransitions)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Convert enum to float for smooth transitions
    switch (NewTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            CurrentTimeOfDayFloat = 0.2f;
            break;
        case EEnvArt_TimeOfDay::Morning:
            CurrentTimeOfDayFloat = 0.3f;
            break;
        case EEnvArt_TimeOfDay::Noon:
            CurrentTimeOfDayFloat = 0.5f;
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            CurrentTimeOfDayFloat = 0.65f;
            break;
        case EEnvArt_TimeOfDay::Dusk:
            CurrentTimeOfDayFloat = 0.8f;
            break;
        case EEnvArt_TimeOfDay::Night:
            CurrentTimeOfDayFloat = 0.0f;
            break;
    }
    
    AtmosphereSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::SetWeatherType(EEnvArt_WeatherType NewWeather)
{
    if (bEnableWeatherTransitions && NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        WeatherTransitionTimer = 0.0f;
    }
    else
    {
        CurrentWeather = NewWeather;
        AtmosphereSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
        ApplyAtmosphereSettings(AtmosphereSettings);
    }
}

void AEnvArt_AtmosphereManager::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    UpdateLighting();
    UpdateFog();
}

void AEnvArt_AtmosphereManager::UpdateLighting()
{
    if (!SunLight)
    {
        FindAtmosphereActors();
    }
    
    if (SunLight)
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetDirectionalLightComponent();
        if (LightComponent)
        {
            LightComponent->SetLightColor(AtmosphereSettings.SunColor);
            LightComponent->SetIntensity(AtmosphereSettings.SunIntensity);
        }
        
        SunLight->SetActorRotation(AtmosphereSettings.SunRotation);
    }
}

void AEnvArt_AtmosphereManager::UpdateFog()
{
    if (!AtmosphereFog)
    {
        FindAtmosphereActors();
    }
    
    if (AtmosphereFog)
    {
        UExponentialHeightFogComponent* FogComponent = AtmosphereFog->GetComponent();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(AtmosphereSettings.FogDensity);
            FogComponent->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
            FogComponent->SetFogMaxOpacity(AtmosphereSettings.FogMaxOpacity);
            FogComponent->SetFogInscatteringColor(AtmosphereSettings.FogInscatteringColor);
        }
    }
}

void AEnvArt_AtmosphereManager::FindAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find fog actor
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AtmosphereFog = Cast<AExponentialHeightFog>(FogActors[0]);
    }
}

void AEnvArt_AtmosphereManager::CreateGoldenHourAtmosphere()
{
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f);
    AtmosphereSettings.SunIntensity = 3.0f;
    AtmosphereSettings.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogHeightFalloff = 0.2f;
    AtmosphereSettings.FogMaxOpacity = 0.6f;
    AtmosphereSettings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f);
    
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    CurrentWeather = EEnvArt_WeatherType::Clear;
    
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::CreateMistyForestAtmosphere()
{
    AtmosphereSettings.SunColor = FLinearColor(0.8f, 0.9f, 1.0f);
    AtmosphereSettings.SunIntensity = 1.5f;
    AtmosphereSettings.SunRotation = FRotator(-30.0f, 120.0f, 0.0f);
    AtmosphereSettings.FogDensity = 0.08f;
    AtmosphereSettings.FogHeightFalloff = 0.1f;
    AtmosphereSettings.FogMaxOpacity = 0.9f;
    AtmosphereSettings.FogInscatteringColor = FLinearColor(0.6f, 0.7f, 0.8f);
    
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    CurrentWeather = EEnvArt_WeatherType::Foggy;
    
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::CreateStormyAtmosphere()
{
    AtmosphereSettings.SunColor = FLinearColor(0.4f, 0.4f, 0.5f);
    AtmosphereSettings.SunIntensity = 0.8f;
    AtmosphereSettings.SunRotation = FRotator(-45.0f, 200.0f, 0.0f);
    AtmosphereSettings.FogDensity = 0.05f;
    AtmosphereSettings.FogHeightFalloff = 0.3f;
    AtmosphereSettings.FogMaxOpacity = 0.8f;
    AtmosphereSettings.FogInscatteringColor = FLinearColor(0.3f, 0.3f, 0.4f);
    
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    CurrentWeather = EEnvArt_WeatherType::Stormy;
    
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    // Progress time based on day duration
    float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
    CurrentTimeOfDayFloat += TimeIncrement;
    
    if (CurrentTimeOfDayFloat >= 1.0f)
    {
        CurrentTimeOfDayFloat -= 1.0f;
    }
    
    // Update time of day enum based on float value
    if (CurrentTimeOfDayFloat < 0.15f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Night;
    else if (CurrentTimeOfDayFloat < 0.25f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Dawn;
    else if (CurrentTimeOfDayFloat < 0.4f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    else if (CurrentTimeOfDayFloat < 0.6f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Noon;
    else if (CurrentTimeOfDayFloat < 0.75f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDayFloat < 0.9f)
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    else
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Night;
    
    // Update atmosphere settings
    AtmosphereSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
    ApplyAtmosphereSettings(AtmosphereSettings);
}

void AEnvArt_AtmosphereManager::UpdateWeatherTransition(float DeltaTime)
{
    if (TargetWeather != CurrentWeather)
    {
        WeatherTransitionTimer += DeltaTime;
        
        if (WeatherTransitionTimer >= WeatherTransitionTime)
        {
            CurrentWeather = TargetWeather;
            WeatherTransitionTimer = 0.0f;
            
            AtmosphereSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
            ApplyAtmosphereSettings(AtmosphereSettings);
        }
    }
}

FEnvArt_AtmosphereSettings AEnvArt_AtmosphereManager::GetSettingsForTimeAndWeather(EEnvArt_TimeOfDay TimeOfDay, EEnvArt_WeatherType Weather)
{
    FEnvArt_AtmosphereSettings Settings;
    
    // Base settings by time of day
    switch (TimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f);
            Settings.SunIntensity = 1.5f;
            Settings.SunRotation = FRotator(-10.0f, 80.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Morning:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
            Settings.SunIntensity = 4.0f;
            Settings.SunRotation = FRotator(-30.0f, 100.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Noon:
            Settings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f);
            Settings.SunIntensity = 5.0f;
            Settings.SunRotation = FRotator(-80.0f, 180.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f);
            Settings.SunIntensity = 3.0f;
            Settings.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
            Settings.SunIntensity = 1.0f;
            Settings.SunRotation = FRotator(-5.0f, 280.0f, 0.0f);
            break;
        case EEnvArt_TimeOfDay::Night:
            Settings.SunColor = FLinearColor(0.2f, 0.3f, 0.5f);
            Settings.SunIntensity = 0.1f;
            Settings.SunRotation = FRotator(10.0f, 0.0f, 0.0f);
            break;
    }
    
    // Modify based on weather
    switch (Weather)
    {
        case EEnvArt_WeatherType::Clear:
            Settings.FogDensity = 0.01f;
            Settings.FogMaxOpacity = 0.3f;
            Settings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f);
            break;
        case EEnvArt_WeatherType::Overcast:
            Settings.SunIntensity *= 0.6f;
            Settings.FogDensity = 0.03f;
            Settings.FogMaxOpacity = 0.7f;
            Settings.FogInscatteringColor = FLinearColor(0.6f, 0.6f, 0.7f);
            break;
        case EEnvArt_WeatherType::Foggy:
            Settings.SunIntensity *= 0.3f;
            Settings.FogDensity = 0.08f;
            Settings.FogMaxOpacity = 0.9f;
            Settings.FogInscatteringColor = FLinearColor(0.5f, 0.6f, 0.7f);
            break;
        case EEnvArt_WeatherType::Rainy:
            Settings.SunIntensity *= 0.4f;
            Settings.FogDensity = 0.05f;
            Settings.FogMaxOpacity = 0.8f;
            Settings.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.6f);
            break;
        case EEnvArt_WeatherType::Stormy:
            Settings.SunIntensity *= 0.2f;
            Settings.SunColor = FLinearColor(0.4f, 0.4f, 0.5f);
            Settings.FogDensity = 0.06f;
            Settings.FogMaxOpacity = 0.9f;
            Settings.FogInscatteringColor = FLinearColor(0.3f, 0.3f, 0.4f);
            break;
    }
    
    Settings.FogHeightFalloff = 0.2f;
    
    return Settings;
}
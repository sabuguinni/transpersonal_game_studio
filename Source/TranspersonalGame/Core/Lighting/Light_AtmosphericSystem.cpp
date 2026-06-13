#include "Light_AtmosphericSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    // Initialize Cretaceous period defaults
    AtmosphericSettings.SunIntensity = 8.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);
    AtmosphericSettings.SunTemperature = 5800.0f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogInscatteringColor = FLinearColor(0.71f, 0.63f, 0.47f, 1.0f);
    AtmosphericSettings.SkyLightIntensity = 2.0f;
    AtmosphericSettings.WhiteTemperature = 5600.0f;
    AtmosphericSettings.ColorSaturation = 1.15f;
    AtmosphericSettings.ColorContrast = 1.1f;
    
    CurrentTimeOfDayFloat = 14.0f; // Start at 2 PM
    DayNightCycleDuration = 1200.0f; // 20 minutes
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and link existing lighting actors
    FindAndLinkLightingActors();
    
    // Apply initial atmospheric settings
    ApplyAtmosphericSettings(AtmosphericSettings);
    
    // Start day/night cycle if enabled
    if (bEnableDayNightCycle)
    {
        StartDayNightCycle();
    }
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        DayNightCycleTimer += DeltaTime;
        
        // Calculate current time of day (0-24 hours)
        float CycleProgress = DayNightCycleTimer / DayNightCycleDuration;
        CurrentTimeOfDayFloat = FMath::Fmod(CycleProgress * 24.0f, 24.0f);
        
        // Update lighting based on time
        UpdateLightingForTimeOfDay();
    }
    
    if (bIsTransitioningWeather)
    {
        WeatherTransitionTimer += DeltaTime;
        WeatherTransitionProgress = FMath::Clamp(WeatherTransitionTimer / WeatherTransitionDuration, 0.0f, 1.0f);
        
        UpdateWeatherEffects();
        
        if (WeatherTransitionProgress >= 1.0f)
        {
            CurrentWeather = TargetWeatherType;
            bIsTransitioningWeather = false;
            WeatherTransitionTimer = 0.0f;
        }
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Convert enum to float time
    switch (NewTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentTimeOfDayFloat = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentTimeOfDayFloat = 9.0f;
            break;
        case ELight_TimeOfDay::Noon:
            CurrentTimeOfDayFloat = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentTimeOfDayFloat = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentTimeOfDayFloat = 18.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentTimeOfDayFloat = 22.0f;
            break;
    }
    
    UpdateLightingForTimeOfDay();
}

void ALight_AtmosphericSystem::SetWeatherType(ELight_WeatherType NewWeatherType)
{
    if (NewWeatherType != CurrentWeather)
    {
        TransitionToWeather(NewWeatherType, 30.0f);
    }
}

void ALight_AtmosphericSystem::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings)
{
    AtmosphericSettings = NewSettings;
    
    // Apply to DirectionalLight (Sun)
    if (SunLight.IsValid())
    {
        if (UDirectionalLightComponent* LightComp = SunLight->GetComponent<UDirectionalLightComponent>())
        {
            LightComp->SetIntensity(NewSettings.SunIntensity);
            LightComp->SetLightColor(NewSettings.SunColor);
            LightComp->SetTemperature(NewSettings.SunTemperature);
        }
    }
    
    // Apply to SkyLight
    if (SkyLight.IsValid())
    {
        if (USkyLightComponent* SkyComp = SkyLight->GetComponent<USkyLightComponent>())
        {
            SkyComp->SetIntensity(NewSettings.SkyLightIntensity);
        }
    }
    
    // Apply to ExponentialHeightFog
    if (AtmosphericFog.IsValid())
    {
        if (UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent<UExponentialHeightFogComponent>())
        {
            FogComp->SetFogDensity(NewSettings.FogDensity);
            FogComp->SetFogHeightFalloff(NewSettings.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(NewSettings.FogInscatteringColor);
        }
    }
    
    // Apply to PostProcessVolume
    if (PostProcessVolume.IsValid())
    {
        if (UPostProcessComponent* PPComp = PostProcessVolume->GetComponent<UPostProcessComponent>())
        {
            FPostProcessSettings& Settings = PPComp->Settings;
            Settings.bOverride_WhiteTemp = true;
            Settings.WhiteTemp = NewSettings.WhiteTemperature;
            Settings.bOverride_ColorSaturation = true;
            Settings.ColorSaturation = FVector4(1.0f, 1.0f, 1.0f, NewSettings.ColorSaturation);
            Settings.bOverride_ColorContrast = true;
            Settings.ColorContrast = FVector4(1.0f, 1.0f, 1.0f, NewSettings.ColorContrast);
        }
    }
}

void ALight_AtmosphericSystem::StartDayNightCycle()
{
    bEnableDayNightCycle = true;
    DayNightCycleTimer = 0.0f;
}

void ALight_AtmosphericSystem::StopDayNightCycle()
{
    bEnableDayNightCycle = false;
}

void ALight_AtmosphericSystem::TransitionToWeather(ELight_WeatherType TargetWeather, float TransitionDuration)
{
    if (TargetWeather != CurrentWeather)
    {
        TargetWeatherType = TargetWeather;
        WeatherTransitionDuration = TransitionDuration;
        WeatherTransitionTimer = 0.0f;
        WeatherTransitionProgress = 0.0f;
        bIsTransitioningWeather = true;
    }
}

void ALight_AtmosphericSystem::FindAndLinkLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find DirectionalLight
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }
    
    // Find SkyLight
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
    }
    
    // Find ExponentialHeightFog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FogActors[0]);
    }
    
    // Find PostProcessVolume
    TArray<AActor*> PPVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PPVolumes);
    if (PPVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PPVolumes[0]);
    }
}

void ALight_AtmosphericSystem::CreateCretaceousLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // This function would spawn all necessary lighting actors
    // Implementation would create DirectionalLight, SkyLight, etc.
    // For now, we assume they exist and use FindAndLinkLightingActors
    FindAndLinkLightingActors();
    ApplyAtmosphericSettings(AtmosphericSettings);
}

void ALight_AtmosphericSystem::EnableLumenGlobalIllumination()
{
    // Enable Lumen via console commands
    if (UWorld* World = GetWorld())
    {
        if (UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.DynamicGlobalIlluminationMethod 1")))
        {
            UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ReflectionMethod 1"));
            UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.DiffuseColorBoost 1.2"));
            UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.FinalGatherQuality 4"));
            UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.MaxTraceDistance 20000"));
        }
    }
}

float ALight_AtmosphericSystem::GetNormalizedTimeOfDay() const
{
    return CurrentTimeOfDayFloat / 24.0f;
}

FRotator ALight_AtmosphericSystem::CalculateSunRotation(float TimeOfDay) const
{
    // Calculate sun position based on time of day
    float SunAngle = (TimeOfDay - 6.0f) / 12.0f * 180.0f - 90.0f; // -90 to +90 degrees
    float SunAzimuth = 135.0f; // Southeast direction for dramatic lighting
    
    return FRotator(SunAngle, SunAzimuth, 0.0f);
}

FLight_AtmosphericSettings ALight_AtmosphericSystem::GetSettingsForTimeAndWeather(ELight_TimeOfDay TimeOfDay, ELight_WeatherType Weather) const
{
    FLight_AtmosphericSettings Settings = AtmosphericSettings;
    
    // Adjust settings based on time of day
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            Settings.SunIntensity = 4.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            Settings.SunTemperature = 5200.0f;
            break;
        case ELight_TimeOfDay::Morning:
            Settings.SunIntensity = 6.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunTemperature = 5500.0f;
            break;
        case ELight_TimeOfDay::Noon:
            Settings.SunIntensity = 10.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
            Settings.SunTemperature = 6000.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            Settings.SunIntensity = 8.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.92f, 0.71f, 1.0f);
            Settings.SunTemperature = 5800.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            Settings.SunIntensity = 3.0f;
            Settings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
            Settings.SunTemperature = 4800.0f;
            break;
        case ELight_TimeOfDay::Night:
            Settings.SunIntensity = 0.1f;
            Settings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
            Settings.SunTemperature = 4000.0f;
            break;
    }
    
    // Adjust settings based on weather
    switch (Weather)
    {
        case ELight_WeatherType::Clear:
            // Use base settings
            break;
        case ELight_WeatherType::Hazy:
            Settings.FogDensity *= 2.0f;
            Settings.SunIntensity *= 0.8f;
            break;
        case ELight_WeatherType::Overcast:
            Settings.SunIntensity *= 0.5f;
            Settings.SkyLightIntensity *= 1.5f;
            Settings.FogDensity *= 3.0f;
            break;
        case ELight_WeatherType::Stormy:
            Settings.SunIntensity *= 0.3f;
            Settings.SkyLightIntensity *= 0.7f;
            Settings.FogDensity *= 4.0f;
            Settings.ColorSaturation *= 0.8f;
            break;
        case ELight_WeatherType::Foggy:
            Settings.FogDensity *= 8.0f;
            Settings.SunIntensity *= 0.4f;
            Settings.SkyLightIntensity *= 0.6f;
            break;
    }
    
    return Settings;
}

void ALight_AtmosphericSystem::UpdateLightingForTimeOfDay()
{
    // Calculate current time of day enum
    ELight_TimeOfDay TimeEnum = ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDayFloat >= 5.0f && CurrentTimeOfDayFloat < 8.0f)
        TimeEnum = ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDayFloat >= 8.0f && CurrentTimeOfDayFloat < 11.0f)
        TimeEnum = ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDayFloat >= 11.0f && CurrentTimeOfDayFloat < 14.0f)
        TimeEnum = ELight_TimeOfDay::Noon;
    else if (CurrentTimeOfDayFloat >= 14.0f && CurrentTimeOfDayFloat < 17.0f)
        TimeEnum = ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDayFloat >= 17.0f && CurrentTimeOfDayFloat < 20.0f)
        TimeEnum = ELight_TimeOfDay::Dusk;
    else
        TimeEnum = ELight_TimeOfDay::Night;
    
    CurrentTimeOfDay = TimeEnum;
    
    // Get settings for current time and weather
    FLight_AtmosphericSettings NewSettings = GetSettingsForTimeAndWeather(TimeEnum, CurrentWeather);
    
    // Apply settings
    ApplyAtmosphericSettings(NewSettings);
    
    // Update sun rotation
    if (SunLight.IsValid())
    {
        FRotator SunRotation = CalculateSunRotation(CurrentTimeOfDayFloat);
        SunLight->SetActorRotation(SunRotation);
    }
}

void ALight_AtmosphericSystem::UpdateWeatherEffects()
{
    if (!bIsTransitioningWeather) return;
    
    // Get current and target weather settings
    FLight_AtmosphericSettings CurrentSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, CurrentWeather);
    FLight_AtmosphericSettings TargetSettings = GetSettingsForTimeAndWeather(CurrentTimeOfDay, TargetWeatherType);
    
    // Interpolate between settings
    InterpolateLightingSettings(CurrentSettings, TargetSettings, WeatherTransitionProgress);
}

void ALight_AtmosphericSystem::InterpolateLightingSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    FLight_AtmosphericSettings InterpolatedSettings;
    
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.SunTemperature = FMath::Lerp(From.SunTemperature, To.SunTemperature, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    InterpolatedSettings.FogInscatteringColor = FMath::Lerp(From.FogInscatteringColor, To.FogInscatteringColor, Alpha);
    InterpolatedSettings.SkyLightIntensity = FMath::Lerp(From.SkyLightIntensity, To.SkyLightIntensity, Alpha);
    InterpolatedSettings.WhiteTemperature = FMath::Lerp(From.WhiteTemperature, To.WhiteTemperature, Alpha);
    InterpolatedSettings.ColorSaturation = FMath::Lerp(From.ColorSaturation, To.ColorSaturation, Alpha);
    InterpolatedSettings.ColorContrast = FMath::Lerp(From.ColorContrast, To.ColorContrast, Alpha);
    
    ApplyAtmosphericSettings(InterpolatedSettings);
}
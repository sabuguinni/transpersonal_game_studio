#include "Light_AtmosphericController.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphericController::ALight_AtmosphericController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize preset settings for Cretaceous period
    
    // Dawn settings - soft golden light
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunTemperature = 4500.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DawnSettings.BloomIntensity = 0.8f;
    DawnSettings.VignetteIntensity = 0.3f;
    DawnSettings.WhiteTemperature = 5000.0f;

    // Noon settings - bright warm light
    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    NoonSettings.SunTemperature = 5500.0f;
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    NoonSettings.BloomIntensity = 0.675f;
    NoonSettings.VignetteIntensity = 0.4f;
    NoonSettings.WhiteTemperature = 6500.0f;

    // Dusk settings - orange-red light
    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunTemperature = 3500.0f;
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskSettings.BloomIntensity = 1.0f;
    DuskSettings.VignetteIntensity = 0.5f;
    DuskSettings.WhiteTemperature = 4000.0f;

    // Night settings - cool moonlight
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.SunTemperature = 8000.0f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogInscatteringColor = FLinearColor(0.3f, 0.4f, 0.7f, 1.0f);
    NightSettings.BloomIntensity = 0.3f;
    NightSettings.VignetteIntensity = 0.6f;
    NightSettings.WhiteTemperature = 8500.0f;

    // Set default to noon
    CurrentSettings = NoonSettings;
}

void ALight_AtmosphericController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find atmospheric components in the level
    FindAtmosphericComponents();
    
    // Apply initial settings
    ApplyAtmosphericSettings(CurrentSettings);
}

void ALight_AtmosphericController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }

        // Update atmospheric settings based on time
        UpdateSunPosition();
        UpdateAtmosphericFog();
        UpdatePostProcessing();
    }

    // Update weather effects
    if (bIsStormy && StormTimer > 0.0f)
    {
        StormTimer -= DeltaTime;
        UpdateWeatherEffects();
        
        if (StormTimer <= 0.0f)
        {
            ClearWeather();
        }
    }
}

void ALight_AtmosphericController::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphericFog();
    UpdatePostProcessing();
}

void ALight_AtmosphericController::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    UpdateWeatherEffects();
}

void ALight_AtmosphericController::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings)
{
    CurrentSettings = NewSettings;

    // Apply to sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(NewSettings.SunIntensity);
        LightComp->SetLightColor(NewSettings.SunColor);
        LightComp->SetTemperature(NewSettings.SunTemperature);
        LightComp->SetUseTemperature(true);
        LightComp->SetCastVolumetricShadow(true);
    }

    // Apply to fog
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(NewSettings.FogDensity);
        FogComp->SetFogHeightFalloff(NewSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(NewSettings.FogInscatteringColor);
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.2f);
    }

    // Apply to post process volume
    if (PostProcessVolume)
    {
        FPostProcessSettings& Settings = PostProcessVolume->Settings;
        Settings.bOverride_BloomIntensity = true;
        Settings.BloomIntensity = NewSettings.BloomIntensity;
        Settings.bOverride_VignetteIntensity = true;
        Settings.VignetteIntensity = NewSettings.VignetteIntensity;
        Settings.bOverride_WhiteTemp = true;
        Settings.WhiteTemp = NewSettings.WhiteTemperature;
    }
}

ELight_TimeOfDay ALight_AtmosphericController::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericController::TriggerStorm(float Duration)
{
    bIsStormy = true;
    StormTimer = Duration;
    StormDuration = Duration;
    WeatherIntensity = 1.0f;
}

void ALight_AtmosphericController::ClearWeather()
{
    bIsStormy = false;
    StormTimer = 0.0f;
    WeatherIntensity = 0.0f;
}

void ALight_AtmosphericController::UpdateSunPosition()
{
    if (!SunLight) return;

    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
    
    // Convert to radians
    float SunAngleRad = FMath::DegreesToRadians(SunAngle);
    
    // Calculate sun direction
    FVector SunDirection = FVector(
        FMath::Cos(SunAngleRad),
        0.0f,
        FMath::Sin(SunAngleRad)
    );

    // Set sun rotation
    FRotator SunRotation = SunDirection.Rotation();
    SunLight->SetActorRotation(SunRotation);

    // Interpolate lighting settings based on time of day
    FLight_AtmosphericSettings TargetSettings;
    
    ELight_TimeOfDay TimeOfDay = GetCurrentTimeOfDay();
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            TargetSettings = DawnSettings;
            break;
        case ELight_TimeOfDay::Morning:
            TargetSettings = InterpolateSettings(DawnSettings, NoonSettings, (CurrentTimeOfDay - 7.0f) / 4.0f);
            break;
        case ELight_TimeOfDay::Noon:
            TargetSettings = NoonSettings;
            break;
        case ELight_TimeOfDay::Afternoon:
            TargetSettings = InterpolateSettings(NoonSettings, DuskSettings, (CurrentTimeOfDay - 15.0f) / 3.0f);
            break;
        case ELight_TimeOfDay::Dusk:
            TargetSettings = DuskSettings;
            break;
        case ELight_TimeOfDay::Night:
            TargetSettings = NightSettings;
            break;
    }

    ApplyAtmosphericSettings(TargetSettings);
}

void ALight_AtmosphericController::UpdateAtmosphericFog()
{
    if (!AtmosphericFog) return;

    // Additional fog adjustments based on weather
    if (bIsStormy)
    {
        float StormProgress = 1.0f - (StormTimer / StormDuration);
        float StormFogDensity = CurrentSettings.FogDensity * (1.0f + WeatherIntensity * 2.0f);
        
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(StormFogDensity);
        }
    }
}

void ALight_AtmosphericController::UpdatePostProcessing()
{
    if (!PostProcessVolume) return;

    // Weather-based post-processing adjustments
    if (bIsStormy)
    {
        FPostProcessSettings& Settings = PostProcessVolume->Settings;
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(0.8f, 0.8f, 0.8f, 1.0f) * (1.0f - WeatherIntensity * 0.3f);
    }
}

void ALight_AtmosphericController::UpdateWeatherEffects()
{
    // Weather effects implementation
    if (bIsStormy)
    {
        // Modify lighting intensity based on storm
        float StormIntensityMod = 1.0f - (WeatherIntensity * 0.5f);
        
        if (SunLight && SunLight->GetLightComponent())
        {
            float ModifiedIntensity = CurrentSettings.SunIntensity * StormIntensityMod;
            SunLight->GetLightComponent()->SetIntensity(ModifiedIntensity);
        }
    }
}

void ALight_AtmosphericController::FindAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find atmospheric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    // Find sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find post process volume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

FLight_AtmosphericSettings ALight_AtmosphericController::InterpolateSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha)
{
    FLight_AtmosphericSettings Result;
    
    Alpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    
    Result.SunIntensity = FMath::Lerp(SettingsA.SunIntensity, SettingsB.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(SettingsA.SunColor, SettingsB.SunColor, Alpha);
    Result.SunTemperature = FMath::Lerp(SettingsA.SunTemperature, SettingsB.SunTemperature, Alpha);
    Result.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(SettingsA.FogHeightFalloff, SettingsB.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(SettingsA.FogInscatteringColor, SettingsB.FogInscatteringColor, Alpha);
    Result.BloomIntensity = FMath::Lerp(SettingsA.BloomIntensity, SettingsB.BloomIntensity, Alpha);
    Result.VignetteIntensity = FMath::Lerp(SettingsA.VignetteIntensity, SettingsB.VignetteIntensity, Alpha);
    Result.WhiteTemperature = FMath::Lerp(SettingsA.WhiteTemperature, SettingsB.WhiteTemperature, Alpha);
    
    return Result;
}
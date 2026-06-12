#include "Light_AtmosphericSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/Engine.h"
#include "Kismet/KismetMathLibrary.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light component (sun)
    DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
    DirectionalLight->SetupAttachment(RootComponent);
    DirectionalLight->SetIntensity(5.0f);
    DirectionalLight->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f));
    DirectionalLight->SetCastShadows(true);
    DirectionalLight->SetCastVolumetricShadow(true);

    // Create sky light component
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.5f);
    SkyLight->SetSourceType(SLS_CapturedScene);
    SkyLight->SetRealTimeCapture(true);

    // Create atmospheric fog component
    AtmosphericFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphericFog"));
    AtmosphericFog->SetupAttachment(RootComponent);
    AtmosphericFog->SetFogDensity(0.02f);
    AtmosphericFog->SetFogHeightFalloff(0.2f);
    AtmosphericFog->SetVolumetricFog(true);

    // Create post process component
    PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
    PostProcess->SetupAttachment(RootComponent);
    PostProcess->bUnbound = true;

    // Initialize day/night cycle settings
    DayDurationMinutes = 20.0f;
    CurrentTimeOfDay = 12.0f;
    bEnableDayNightCycle = true;

    // Configure Cretaceous period atmospheric presets
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.8f;
    DawnSettings.FogDensity = 0.03f;

    DaySettings.SunIntensity = 5.0f;
    DaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    DaySettings.SkyLightIntensity = 1.5f;
    DaySettings.FogDensity = 0.02f;

    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.6f;
    DuskSettings.FogDensity = 0.025f;

    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    NightSettings.FogDensity = 0.015f;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    ConfigureCretaceousAtmosphere();
    EnableLumenGlobalIllumination();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Fmod(TimeHours, 24.0f);
    if (CurrentTimeOfDay < 0.0f)
    {
        CurrentTimeOfDay += 24.0f;
    }

    UpdateSunPosition();

    // Determine current atmospheric settings based on time
    FLight_AtmosphericSettings CurrentSettings;
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f) // Dawn
    {
        float Alpha = (CurrentTimeOfDay - 5.0f) / 2.0f;
        CurrentSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 10.0f) // Morning
    {
        float Alpha = (CurrentTimeOfDay - 7.0f) / 3.0f;
        CurrentSettings = InterpolateSettings(DawnSettings, DaySettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay < 16.0f) // Day
    {
        CurrentSettings = DaySettings;
    }
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 19.0f) // Afternoon to Dusk
    {
        float Alpha = (CurrentTimeOfDay - 16.0f) / 3.0f;
        CurrentSettings = InterpolateSettings(DaySettings, DuskSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 21.0f) // Dusk to Night
    {
        float Alpha = (CurrentTimeOfDay - 19.0f) / 2.0f;
        CurrentSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else // Night
    {
        CurrentSettings = NightSettings;
    }

    ApplyAtmosphericSettings(CurrentSettings);
}

void ALight_AtmosphericSystem::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (DirectionalLight)
    {
        DirectionalLight->SetIntensity(Settings.SunIntensity);
        DirectionalLight->SetLightColor(Settings.SunColor);
    }

    if (SkyLight)
    {
        SkyLight->SetIntensity(Settings.SkyLightIntensity);
    }

    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogDensity(Settings.FogDensity);
        AtmosphericFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
        AtmosphericFog->SetVolumetricFog(Settings.bVolumetricFog);
    }
}

ELight_TimeOfDay ALight_AtmosphericSystem::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 12.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 16.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 19.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 21.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphericSystem::EnableLumenGlobalIllumination()
{
    if (PostProcess)
    {
        PostProcess->Settings.bOverride_DynamicGlobalIlluminationMethod = true;
        PostProcess->Settings.DynamicGlobalIlluminationMethod = EDynamicGlobalIlluminationMethod::Lumen;
        PostProcess->Settings.bOverride_ReflectionMethod = true;
        PostProcess->Settings.ReflectionMethod = EReflectionMethod::Lumen;
        PostProcess->Settings.bOverride_BloomIntensity = true;
        PostProcess->Settings.BloomIntensity = 0.675f;
    }
}

void ALight_AtmosphericSystem::ConfigureCretaceousAtmosphere()
{
    // Configure for warm, humid Cretaceous climate
    if (DirectionalLight)
    {
        DirectionalLight->SetIntensity(5.0f);
        DirectionalLight->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f)); // Warm golden light
        DirectionalLight->SetTemperature(5500.0f); // Slightly warm sun
    }

    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogDensity(0.02f); // Higher humidity
        AtmosphericFog->SetFogInscatteringColor(FLinearColor(0.9f, 0.85f, 0.7f, 1.0f)); // Warm scattered light
        AtmosphericFog->SetVolumetricFog(true);
        AtmosphericFog->SetVolumetricFogScatteringDistribution(0.2f); // More forward scattering
    }

    if (SkyLight)
    {
        SkyLight->SetIntensity(1.5f);
        SkyLight->SetLightColor(FLinearColor(0.95f, 0.9f, 0.8f, 1.0f)); // Slightly warm sky light
    }
}

void ALight_AtmosphericSystem::UpdateDayNightCycle(float DeltaTime)
{
    float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
    CurrentTimeOfDay += TimeIncrement;
    
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    SetTimeOfDay(CurrentTimeOfDay);
}

FLight_AtmosphericSettings ALight_AtmosphericSystem::InterpolateSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha)
{
    FLight_AtmosphericSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.bVolumetricFog = Alpha > 0.5f ? B.bVolumetricFog : A.bVolumetricFog;
    
    return Result;
}

void ALight_AtmosphericSystem::UpdateSunPosition()
{
    if (DirectionalLight)
    {
        // Calculate sun angle based on time of day
        float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to start at horizon
        float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
        float SunAzimuth = (CurrentTimeOfDay / 24.0f) * 360.0f;

        FRotator SunRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
        DirectionalLight->SetWorldRotation(SunRotation);
    }
}
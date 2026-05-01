#include "Light_DayNightCycle.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_DayNightCycle::ALight_DayNightCycle()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    // Initialize default settings
    CycleDurationMinutes = 20.0f;
    CurrentTimeOfDay = 0.5f; // Start at noon
    bAutoAdvanceTime = true;
    TimeMultiplier = 1.0f;

    // Setup dawn settings (6 AM)
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Orange dawn light
    DawnSettings.SunTemperature = 3000.0f;
    DawnSettings.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnSettings.FogDensity = 0.05f;

    // Setup midday settings (12 PM)
    MiddaySettings.SunIntensity = 4.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Bright white
    MiddaySettings.SunTemperature = 5500.0f;
    MiddaySettings.SunRotation = FRotator(-80.0f, 90.0f, 0.0f);
    MiddaySettings.FogDensity = 0.01f;

    // Setup dusk settings (6 PM)
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f); // Orange/red sunset
    DuskSettings.SunTemperature = 2500.0f;
    DuskSettings.SunRotation = FRotator(10.0f, 270.0f, 0.0f);
    DuskSettings.FogDensity = 0.04f;

    // Setup night settings (12 AM)
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.8f, 1.0f); // Blue moonlight
    NightSettings.SunTemperature = 4000.0f;
    NightSettings.SunRotation = FRotator(80.0f, 270.0f, 0.0f);
    NightSettings.FogDensity = 0.03f;

    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    VolumetricFog = nullptr;
}

void ALight_DayNightCycle::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Apply initial lighting settings
    ApplyCurrentLightingSettings();
}

void ALight_DayNightCycle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime && CycleDurationMinutes > 0.0f)
    {
        // Advance time based on cycle duration
        float TimeIncrement = (DeltaTime * TimeMultiplier) / (CycleDurationMinutes * 60.0f);
        CurrentTimeOfDay += TimeIncrement;
        
        // Wrap around at 1.0 (full day)
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }
        
        // Update lighting based on new time
        UpdateLighting();
    }
}

void ALight_DayNightCycle::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateLighting();
}

ELight_TimeOfDay ALight_DayNightCycle::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay < 0.125f || CurrentTimeOfDay >= 0.875f)
        return ELight_TimeOfDay::Night;
    else if (CurrentTimeOfDay < 0.25f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay < 0.375f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay < 0.625f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay < 0.75f)
        return ELight_TimeOfDay::Afternoon;
    else
        return ELight_TimeOfDay::Dusk;
}

void ALight_DayNightCycle::SetCycleDuration(float NewDurationMinutes)
{
    CycleDurationMinutes = FMath::Max(0.1f, NewDurationMinutes);
}

void ALight_DayNightCycle::PauseTimeAdvancement()
{
    bAutoAdvanceTime = false;
}

void ALight_DayNightCycle::ResumeTimeAdvancement()
{
    bAutoAdvanceTime = true;
}

void ALight_DayNightCycle::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find volumetric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumetricFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_DayNightCycle::ApplyCurrentLightingSettings()
{
    UpdateLighting();
}

void ALight_DayNightCycle::UpdateLighting()
{
    FLight_LightingSettings CurrentSettings = GetInterpolatedSettings(CurrentTimeOfDay);
    ApplyLightingSettings(CurrentSettings);
}

FLight_LightingSettings ALight_DayNightCycle::GetInterpolatedSettings(float TimeValue) const
{
    FLight_LightingSettings Result;
    
    if (TimeValue < 0.25f) // Dawn to Morning
    {
        float Alpha = TimeValue / 0.25f;
        if (TimeValue < 0.125f) // Night to Dawn
        {
            Alpha = TimeValue / 0.125f;
            Result.SunIntensity = FMath::Lerp(NightSettings.SunIntensity, DawnSettings.SunIntensity, Alpha);
            Result.SunColor = FMath::Lerp(NightSettings.SunColor, DawnSettings.SunColor, Alpha);
            Result.SunTemperature = FMath::Lerp(NightSettings.SunTemperature, DawnSettings.SunTemperature, Alpha);
            Result.FogDensity = FMath::Lerp(NightSettings.FogDensity, DawnSettings.FogDensity, Alpha);
        }
        else // Dawn to Midday
        {
            Alpha = (TimeValue - 0.125f) / 0.125f;
            Result.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, MiddaySettings.SunIntensity, Alpha);
            Result.SunColor = FMath::Lerp(DawnSettings.SunColor, MiddaySettings.SunColor, Alpha);
            Result.SunTemperature = FMath::Lerp(DawnSettings.SunTemperature, MiddaySettings.SunTemperature, Alpha);
            Result.FogDensity = FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, Alpha);
        }
    }
    else if (TimeValue < 0.75f) // Midday to Afternoon
    {
        float Alpha = (TimeValue - 0.25f) / 0.5f;
        Result.SunIntensity = FMath::Lerp(MiddaySettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
        Result.SunColor = FMath::Lerp(MiddaySettings.SunColor, DuskSettings.SunColor, Alpha);
        Result.SunTemperature = FMath::Lerp(MiddaySettings.SunTemperature, DuskSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(MiddaySettings.FogDensity, DuskSettings.FogDensity, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (TimeValue - 0.75f) / 0.25f;
        Result.SunIntensity = FMath::Lerp(DuskSettings.SunIntensity, NightSettings.SunIntensity, Alpha);
        Result.SunColor = FMath::Lerp(DuskSettings.SunColor, NightSettings.SunColor, Alpha);
        Result.SunTemperature = FMath::Lerp(DuskSettings.SunTemperature, NightSettings.SunTemperature, Alpha);
        Result.FogDensity = FMath::Lerp(DuskSettings.FogDensity, NightSettings.FogDensity, Alpha);
    }

    // Calculate sun rotation based on time
    float SunAngle = CalculateSunAngle(TimeValue);
    Result.SunRotation = FRotator(SunAngle, 90.0f, 0.0f);

    return Result;
}

void ALight_DayNightCycle::ApplyLightingSettings(const FLight_LightingSettings& Settings)
{
    // Apply to directional light (sun)
    if (SunLight)
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(Settings.SunIntensity);
            LightComp->SetLightColor(Settings.SunColor);
            LightComp->SetTemperature(Settings.SunTemperature);
            LightComp->SetUseTemperature(true);
        }
        
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    // Apply to sky atmosphere
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent();
        if (SkyComp)
        {
            SkyComp->SetAtmosphereHeight(Settings.AtmosphereHeight);
            SkyComp->SetMultiScatteringFactor(Settings.MultiScatteringFactor);
        }
    }

    // Apply to volumetric fog
    if (VolumetricFog)
    {
        UExponentialHeightFogComponent* FogComp = VolumetricFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        }
    }
}

float ALight_DayNightCycle::CalculateSunAngle(float TimeValue) const
{
    // Convert time to sun angle (-90 to +90 degrees)
    // 0.0 = midnight (sun below horizon)
    // 0.5 = noon (sun overhead)
    // 1.0 = midnight again
    
    float SunAngle = FMath::Sin(TimeValue * 2.0f * PI) * 90.0f;
    return SunAngle;
}
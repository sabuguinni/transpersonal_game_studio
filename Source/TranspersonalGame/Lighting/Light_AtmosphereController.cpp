#include "Light_AtmosphereController.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereController::ALight_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    TimeOfDay = 12.0f;
    DayDurationMinutes = 20.0f;
    bEnableTimeProgression = true;
    
    InitializeAtmospherePresets();
}

void ALight_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingActors();
    UpdateSunPosition();
    
    FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    ApplyAtmosphereSettings(CurrentSettings);
}

void ALight_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableTimeProgression)
    {
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        TimeOfDay += TimeIncrement;
        
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay = 0.0f;
        }
        
        UpdateSunPosition();
        
        FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
        ApplyAtmosphereSettings(CurrentSettings);
    }
}

void ALight_AtmosphereController::InitializeAtmospherePresets()
{
    // Dawn (5-7 AM) - Soft orange light
    FLight_AtmosphereSettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SkyLightIntensity = 0.5f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Dawn, DawnSettings);
    
    // Morning (7-11 AM) - Warm growing light
    FLight_AtmosphereSettings MorningSettings;
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SkyLightIntensity = 0.8f;
    MorningSettings.FogDensity = 0.025f;
    MorningSettings.FogColor = FLinearColor(0.9f, 0.85f, 0.8f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Morning, MorningSettings);
    
    // Midday (11 AM-2 PM) - Bright Cretaceous sun
    FLight_AtmosphereSettings MiddaySettings;
    MiddaySettings.SunIntensity = 6.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SkyLightIntensity = 1.2f;
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon (2-6 PM) - Golden hour approaching
    FLight_AtmosphereSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 4.5f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.SkyLightIntensity = 0.9f;
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.8f, 0.75f, 0.7f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Dusk (6-8 PM) - Dramatic golden light
    FLight_AtmosphereSettings DuskSettings;
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SkyLightIntensity = 0.4f;
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Dusk, DuskSettings);
    
    // Night (8 PM-5 AM) - Moonlight and stars
    FLight_AtmosphereSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SkyLightIntensity = 0.2f;
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    AtmospherePresets.Add(ELight_TimeOfDay::Night, NightSettings);
}

void ALight_AtmosphereController::InitializeLightingActors()
{
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }
    
    if (!SkyLightActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
        }
    }
    
    if (!FogActor)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        }
    }
}

void ALight_AtmosphereController::SetTimeOfDay(float NewTime)
{
    TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
    
    FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    ApplyAtmosphereSettings(CurrentSettings);
}

ELight_TimeOfDay ALight_AtmosphereController::GetCurrentTimeOfDayEnum() const
{
    if (TimeOfDay >= 5.0f && TimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (TimeOfDay >= 7.0f && TimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (TimeOfDay >= 11.0f && TimeOfDay < 14.0f)
        return ELight_TimeOfDay::Midday;
    else if (TimeOfDay >= 14.0f && TimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (TimeOfDay >= 18.0f && TimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphereController::ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
    }
    
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        SkyLightActor->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
    }
    
    if (FogActor && FogActor->GetComponent())
    {
        FogActor->GetComponent()->SetFogDensity(Settings.FogDensity);
        FogActor->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogActor->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_AtmosphereController::UpdateSunPosition()
{
    if (!SunLight)
        return;
    
    float SunAngle = GetSunAngle();
    
    // Calculate sun rotation based on time of day
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // Sun moves east to west
    SunRotation.Roll = 0.0f;
    
    SunLight->SetActorRotation(SunRotation);
}

float ALight_AtmosphereController::GetSunAngle() const
{
    // Convert time of day to sun angle
    // 6 AM = -90 degrees (sunrise)
    // 12 PM = 0 degrees (noon, directly overhead)
    // 6 PM = 90 degrees (sunset)
    float NormalizedTime = (TimeOfDay - 6.0f) / 12.0f; // Normalize to 0-1 for 6AM-6PM
    float SunAngle = FMath::Lerp(-90.0f, 90.0f, NormalizedTime);
    
    return SunAngle;
}

FLight_AtmosphereSettings ALight_AtmosphereController::GetCurrentAtmosphereSettings() const
{
    ELight_TimeOfDay CurrentPeriod = GetCurrentTimeOfDayEnum();
    
    if (AtmospherePresets.Contains(CurrentPeriod))
    {
        return AtmospherePresets[CurrentPeriod];
    }
    
    // Fallback to midday settings
    if (AtmospherePresets.Contains(ELight_TimeOfDay::Midday))
    {
        return AtmospherePresets[ELight_TimeOfDay::Midday];
    }
    
    // Ultimate fallback
    return FLight_AtmosphereSettings();
}

FLight_AtmosphereSettings ALight_AtmosphereController::InterpolateAtmosphereSettings(ELight_TimeOfDay TimeA, ELight_TimeOfDay TimeB, float Alpha) const
{
    if (!AtmospherePresets.Contains(TimeA) || !AtmospherePresets.Contains(TimeB))
    {
        return FLight_AtmosphereSettings();
    }
    
    const FLight_AtmosphereSettings& SettingsA = AtmospherePresets[TimeA];
    const FLight_AtmosphereSettings& SettingsB = AtmospherePresets[TimeB];
    
    FLight_AtmosphereSettings Result;
    Result.SunIntensity = FMath::Lerp(SettingsA.SunIntensity, SettingsB.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(SettingsA.SunColor, SettingsB.SunColor, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(SettingsA.SkyLightIntensity, SettingsB.SkyLightIntensity, Alpha);
    Result.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(SettingsA.FogHeightFalloff, SettingsB.FogHeightFalloff, Alpha);
    Result.FogColor = FMath::Lerp(SettingsA.FogColor, SettingsB.FogColor, Alpha);
    
    return Result;
}
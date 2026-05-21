#include "Light_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings for Cretaceous period
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MiddaySettings.SunIntensity = 3.5f;
    MiddaySettings.SunRotation = FRotator(-35.0f, 45.0f, 0.0f);
    MiddaySettings.RayleighScattering = 0.8f;
    MiddaySettings.MieScattering = 0.6f;
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.FogColor = FLinearColor(0.9f, 0.85f, 0.7f, 1.0f);

    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunRotation = FRotator(-75.0f, 90.0f, 0.0f);
    DawnSettings.RayleighScattering = 1.2f;
    DawnSettings.MieScattering = 0.8f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);

    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunRotation = FRotator(-75.0f, 270.0f, 0.0f);
    DuskSettings.RayleighScattering = 1.0f;
    DuskSettings.MieScattering = 0.9f;
    DuskSettings.FogDensity = 0.03f;
    DuskSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunRotation = FRotator(-120.0f, 180.0f, 0.0f);
    NightSettings.RayleighScattering = 0.4f;
    NightSettings.MieScattering = 0.3f;
    NightSettings.FogDensity = 0.015f;
    NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();

    FindLightingActors();
    EnableCretaceousAtmosphere();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        float TimeIncrement = (24.0f / (DayDurationInMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }

        UpdateLighting();
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting();
}

void ALight_AtmosphereManager::SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    if (SkyAtmosphere && SkyAtmosphere->GetComponent())
    {
        SkyAtmosphere->GetComponent()->SetRayleighScatteringScale(Settings.RayleighScattering);
        SkyAtmosphere->GetComponent()->SetMieScatteringScale(Settings.MieScattering);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_AtmosphereManager::EnableCretaceousAtmosphere()
{
    FindLightingActors();

    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetAtmosphereSunLight(true);
        SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
    }

    // Apply current time settings
    UpdateLighting();
}

void ALight_AtmosphereManager::UpdateLighting()
{
    FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    SetAtmosphereSettings(CurrentSettings);
    
    UpdateSunPosition();
    UpdateAtmosphereScattering();
    UpdateFogSettings();
    UpdatePostProcessing();
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

FLight_AtmosphereSettings ALight_AtmosphereManager::GetCurrentAtmosphereSettings() const
{
    float DayProgress = GetDayNightProgress();
    
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
    {
        // Dawn transition
        float Alpha = (CurrentTimeOfDay - 5.0f) / 2.0f;
        return InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
    {
        // Morning transition
        float Alpha = (CurrentTimeOfDay - 7.0f) / 4.0f;
        return InterpolateSettings(DawnSettings, MiddaySettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
    {
        // Midday
        return MiddaySettings;
    }
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
    {
        // Afternoon transition
        float Alpha = (CurrentTimeOfDay - 15.0f) / 3.0f;
        return InterpolateSettings(MiddaySettings, DuskSettings, Alpha);
    }
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
    {
        // Dusk transition
        float Alpha = (CurrentTimeOfDay - 18.0f) / 2.0f;
        return InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    else
    {
        // Night
        return NightSettings;
    }
}

void ALight_AtmosphereManager::FindLightingActors()
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

    // Find sky atmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }

    // Find height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }

    // Find post process volume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
    }
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    // Calculate sun position based on time of day
    float SunAngle = (CurrentTimeOfDay / 24.0f) * 360.0f - 90.0f; // Start at dawn
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunAzimuth = SunAngle;

    FRotator NewRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
    SunLight->SetActorRotation(NewRotation);
}

void ALight_AtmosphereManager::UpdateAtmosphereScattering()
{
    if (!SkyAtmosphere) return;

    FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    
    if (SkyAtmosphere->GetComponent())
    {
        SkyAtmosphere->GetComponent()->SetRayleighScatteringScale(CurrentSettings.RayleighScattering);
        SkyAtmosphere->GetComponent()->SetMieScatteringScale(CurrentSettings.MieScattering);
    }
}

void ALight_AtmosphereManager::UpdateFogSettings()
{
    if (!HeightFog) return;

    FLight_AtmosphereSettings CurrentSettings = GetCurrentAtmosphereSettings();
    
    if (HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(CurrentSettings.FogDensity);
        HeightFog->GetComponent()->SetFogInscatteringColor(CurrentSettings.FogColor);
    }
}

void ALight_AtmosphereManager::UpdatePostProcessing()
{
    if (!PostProcessVolume) return;

    // Update post-process settings based on time of day
    ELight_TimeOfDay TimeOfDay = GetCurrentTimeOfDay();
    
    switch (TimeOfDay)
    {
    case ELight_TimeOfDay::Dawn:
        PostProcessVolume->Settings.ColorGradingGlobalSaturation = FVector4(1.2f, 1.0f, 0.8f, 1.0f);
        break;
    case ELight_TimeOfDay::Midday:
        PostProcessVolume->Settings.ColorGradingGlobalSaturation = FVector4(1.1f, 1.0f, 0.9f, 1.0f);
        break;
    case ELight_TimeOfDay::Dusk:
        PostProcessVolume->Settings.ColorGradingGlobalSaturation = FVector4(1.3f, 0.9f, 0.7f, 1.0f);
        break;
    case ELight_TimeOfDay::Night:
        PostProcessVolume->Settings.ColorGradingGlobalSaturation = FVector4(0.8f, 0.9f, 1.2f, 1.0f);
        break;
    default:
        PostProcessVolume->Settings.ColorGradingGlobalSaturation = FVector4(1.1f, 1.0f, 0.9f, 1.0f);
        break;
    }
}

FLight_AtmosphereSettings ALight_AtmosphereManager::InterpolateSettings(const FLight_AtmosphereSettings& A, const FLight_AtmosphereSettings& B, float Alpha) const
{
    FLight_AtmosphereSettings Result;
    
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.RayleighScattering = FMath::Lerp(A.RayleighScattering, B.RayleighScattering, Alpha);
    Result.MieScattering = FMath::Lerp(A.MieScattering, B.MieScattering, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    
    return Result;
}

float ALight_AtmosphereManager::GetDayNightProgress() const
{
    return CurrentTimeOfDay / 24.0f;
}
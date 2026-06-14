#include "Light_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings for Cretaceous period
    MiddaySettings.SunIntensity = 8.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.SunTemperature = 5800.0f;
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.VolumetricScattering = 2.0f;
    MiddaySettings.AtmosphereColor = FLinearColor(1.0f, 0.86f, 0.71f, 1.0f);

    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunTemperature = 4500.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.VolumetricScattering = 3.0f;
    DawnSettings.AtmosphereColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);

    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunTemperature = 3500.0f;
    DuskSettings.FogDensity = 0.03f;
    DuskSettings.VolumetricScattering = 4.0f;
    DuskSettings.AtmosphereColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);

    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunTemperature = 6500.0f;
    NightSettings.FogDensity = 0.01f;
    NightSettings.VolumetricScattering = 1.0f;
    NightSettings.AtmosphereColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);

    CurrentTimeOfDay = 12.0f; // Start at midday
    DayDurationMinutes = 20.0f;
    bEnableDayNightCycle = true;

    SunLight = nullptr;
    SkyLightActor = nullptr;
    AtmosphericFog = nullptr;
    SkyAtmosphereActor = nullptr;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    EnableCretaceousLighting();
    UpdateLightingComponents();
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Advance time of day
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }

        UpdateSunPosition();
        InterpolateLightingSettings();
    }
}

void ALight_AtmosphereManager::FindLightingActors()
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
        SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
    }

    // Find ExponentialHeightFog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FogActors[0]);
    }

    // Find SkyAtmosphere
    TArray<AActor*> AtmosphereActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), AtmosphereActors);
    if (AtmosphereActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(AtmosphereActors[0]);
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
    UpdateSunPosition();
    InterpolateLightingSettings();
}

void ALight_AtmosphereManager::SetAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetTemperature(Settings.SunTemperature);
        LightComp->SetVolumetricScatteringIntensity(Settings.VolumetricScattering);
    }

    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.AtmosphereColor);
    }
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 12.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 16.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay < 19.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 19.0f && CurrentTimeOfDay < 22.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphereManager::EnableCretaceousLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Enable Lumen and volumetric features
    if (UKismetSystemLibrary* SystemLib = UKismetSystemLibrary::StaticClass()->GetDefaultObject<UKismetSystemLibrary>())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.DynamicGlobalIlluminationMethod 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ReflectionMethod 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.Virtual.Enable 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.VolumetricFog 1"));
    }
}

void ALight_AtmosphereManager::UpdateLightingComponents()
{
    FLight_AtmosphereSettings CurrentSettings = GetInterpolatedSettings();
    SetAtmosphereSettings(CurrentSettings);
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    float SunAngle = GetSunAngle();
    
    // Calculate sun rotation based on time of day
    float Pitch = SunAngle - 90.0f; // Convert to pitch (negative = below horizon)
    float Yaw = 30.0f; // Fixed azimuth for consistent lighting direction
    
    FRotator SunRotation(Pitch, Yaw, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ALight_AtmosphereManager::InterpolateLightingSettings()
{
    FLight_AtmosphereSettings InterpolatedSettings = GetInterpolatedSettings();
    SetAtmosphereSettings(InterpolatedSettings);
}

FLight_AtmosphereSettings ALight_AtmosphereManager::GetInterpolatedSettings() const
{
    FLight_AtmosphereSettings Result;
    
    ELight_TimeOfDay TimeOfDay = GetCurrentTimeOfDay();
    
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            Result = DawnSettings;
            break;
        case ELight_TimeOfDay::Morning:
            // Interpolate between Dawn and Midday
            {
                float Alpha = (CurrentTimeOfDay - 8.0f) / 4.0f; // 8-12 hours
                Result.SunIntensity = FMath::Lerp(DawnSettings.SunIntensity, MiddaySettings.SunIntensity, Alpha);
                Result.SunColor = FMath::Lerp(DawnSettings.SunColor, MiddaySettings.SunColor, Alpha);
                Result.SunTemperature = FMath::Lerp(DawnSettings.SunTemperature, MiddaySettings.SunTemperature, Alpha);
                Result.FogDensity = FMath::Lerp(DawnSettings.FogDensity, MiddaySettings.FogDensity, Alpha);
                Result.VolumetricScattering = FMath::Lerp(DawnSettings.VolumetricScattering, MiddaySettings.VolumetricScattering, Alpha);
                Result.AtmosphereColor = FMath::Lerp(DawnSettings.AtmosphereColor, MiddaySettings.AtmosphereColor, Alpha);
            }
            break;
        case ELight_TimeOfDay::Midday:
            Result = MiddaySettings;
            break;
        case ELight_TimeOfDay::Afternoon:
            // Interpolate between Midday and Dusk
            {
                float Alpha = (CurrentTimeOfDay - 16.0f) / 3.0f; // 16-19 hours
                Result.SunIntensity = FMath::Lerp(MiddaySettings.SunIntensity, DuskSettings.SunIntensity, Alpha);
                Result.SunColor = FMath::Lerp(MiddaySettings.SunColor, DuskSettings.SunColor, Alpha);
                Result.SunTemperature = FMath::Lerp(MiddaySettings.SunTemperature, DuskSettings.SunTemperature, Alpha);
                Result.FogDensity = FMath::Lerp(MiddaySettings.FogDensity, DuskSettings.FogDensity, Alpha);
                Result.VolumetricScattering = FMath::Lerp(MiddaySettings.VolumetricScattering, DuskSettings.VolumetricScattering, Alpha);
                Result.AtmosphereColor = FMath::Lerp(MiddaySettings.AtmosphereColor, DuskSettings.AtmosphereColor, Alpha);
            }
            break;
        case ELight_TimeOfDay::Dusk:
            Result = DuskSettings;
            break;
        case ELight_TimeOfDay::Night:
            Result = NightSettings;
            break;
    }
    
    return Result;
}

float ALight_AtmosphereManager::GetSunAngle() const
{
    // Convert time of day (0-24) to sun angle (0-180 degrees)
    // 6 AM = 0°, 12 PM = 90°, 6 PM = 180°
    float NormalizedTime = (CurrentTimeOfDay - 6.0f) / 12.0f; // 6 AM to 6 PM = 0 to 1
    return FMath::Clamp(NormalizedTime * 180.0f, 0.0f, 180.0f);
}
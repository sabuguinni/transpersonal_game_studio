#include "Light_DynamicAtmosphereController.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_DynamicAtmosphereController::ALight_DynamicAtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    DayDurationMinutes = 2.0f;
    CurrentTimeOfDay = 12.0f;
    bEnableDynamicCycle = true;
    bStartAtRandomTime = false;
    bEnableWeatherSystem = true;
    CloudCoverage = 0.3f;
    RainIntensity = 0.0f;

    // Initialize atmospheric presets
    InitializeAtmosphericPresets();
}

void ALight_DynamicAtmosphereController::BeginPlay()
{
    Super::BeginPlay();

    // Find and assign lighting actors automatically
    FindAndAssignLightingActors();

    // Set random start time if enabled
    if (bStartAtRandomTime)
    {
        CurrentTimeOfDay = FMath::RandRange(0.0f, 24.0f);
    }

    // Apply initial atmospheric settings
    UpdateAtmosphericLighting();
}

void ALight_DynamicAtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_DynamicAtmosphereController::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_DynamicAtmosphereController::SetTimeOfDayEnum(ELight_TimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            SetTimeOfDay(6.0f);
            break;
        case ELight_TimeOfDay::Morning:
            SetTimeOfDay(9.0f);
            break;
        case ELight_TimeOfDay::Noon:
            SetTimeOfDay(12.0f);
            break;
        case ELight_TimeOfDay::Afternoon:
            SetTimeOfDay(15.0f);
            break;
        case ELight_TimeOfDay::Dusk:
            SetTimeOfDay(18.0f);
            break;
        case ELight_TimeOfDay::Night:
            SetTimeOfDay(0.0f);
            break;
    }
}

void ALight_DynamicAtmosphereController::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
    }

    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetFogHeightFalloff(Settings.FogHeightFalloff);
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
    }
}

void ALight_DynamicAtmosphereController::SetWeatherConditions(float NewCloudCoverage, float NewRainIntensity)
{
    CloudCoverage = FMath::Clamp(NewCloudCoverage, 0.0f, 1.0f);
    RainIntensity = FMath::Clamp(NewRainIntensity, 0.0f, 1.0f);

    // Apply weather effects to atmospheric settings
    UpdateAtmosphericLighting();
}

void ALight_DynamicAtmosphereController::FindAndAssignLightingActors()
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

    // Find SkyAtmosphere
    TArray<AActor*> SkyAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
    if (SkyAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
    }

    // Find ExponentialHeightFog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
    }

    // Find PostProcessVolume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
    }
}

ELight_TimeOfDay ALight_DynamicAtmosphereController::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f)
        return ELight_TimeOfDay::Noon;
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

float ALight_DynamicAtmosphereController::GetSunAngleFromTime(float TimeOfDay) const
{
    // Convert 24-hour time to sun angle (-90 to 90 degrees)
    // Sun rises at 6:00 (-90°) and sets at 18:00 (+90°)
    float NormalizedTime = (TimeOfDay - 6.0f) / 12.0f; // 0 to 1 for day
    return FMath::Lerp(-90.0f, 90.0f, NormalizedTime);
}

void ALight_DynamicAtmosphereController::UpdateDayNightCycle(float DeltaTime)
{
    // Update time of day
    float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
    CurrentTimeOfDay += TimeIncrement;

    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    UpdateSunPosition();
    UpdateAtmosphericLighting();
}

void ALight_DynamicAtmosphereController::UpdateSunPosition()
{
    if (!SunLight) return;

    float SunAngle = GetSunAngleFromTime(CurrentTimeOfDay);
    
    // Set sun rotation based on time
    FRotator SunRotation = FRotator(SunAngle, 0.0f, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ALight_DynamicAtmosphereController::UpdateAtmosphericLighting()
{
    ELight_TimeOfDay CurrentPeriod = GetCurrentTimeOfDayEnum();
    FLight_AtmosphericSettings CurrentSettings;

    // Interpolate between atmospheric presets based on time
    switch (CurrentPeriod)
    {
        case ELight_TimeOfDay::Dawn:
            InterpolateAtmosphericSettings(NightSettings, DawnSettings, 
                FMath::GetMappedRangeValueClamped(FVector2D(5.0f, 8.0f), FVector2D(0.0f, 1.0f), CurrentTimeOfDay), CurrentSettings);
            break;
        case ELight_TimeOfDay::Morning:
            InterpolateAtmosphericSettings(DawnSettings, NoonSettings, 
                FMath::GetMappedRangeValueClamped(FVector2D(8.0f, 11.0f), FVector2D(0.0f, 1.0f), CurrentTimeOfDay), CurrentSettings);
            break;
        case ELight_TimeOfDay::Noon:
            CurrentSettings = NoonSettings;
            break;
        case ELight_TimeOfDay::Afternoon:
            InterpolateAtmosphericSettings(NoonSettings, DuskSettings, 
                FMath::GetMappedRangeValueClamped(FVector2D(14.0f, 17.0f), FVector2D(0.0f, 1.0f), CurrentTimeOfDay), CurrentSettings);
            break;
        case ELight_TimeOfDay::Dusk:
            InterpolateAtmosphericSettings(DuskSettings, NightSettings, 
                FMath::GetMappedRangeValueClamped(FVector2D(17.0f, 20.0f), FVector2D(0.0f, 1.0f), CurrentTimeOfDay), CurrentSettings);
            break;
        case ELight_TimeOfDay::Night:
            CurrentSettings = NightSettings;
            break;
    }

    // Apply weather modifications
    if (bEnableWeatherSystem)
    {
        CurrentSettings.SunIntensity *= (1.0f - CloudCoverage * 0.7f);
        CurrentSettings.FogDensity += RainIntensity * 0.05f;
    }

    ApplyAtmosphericSettings(CurrentSettings);
}

void ALight_DynamicAtmosphereController::InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& SettingsA, const FLight_AtmosphericSettings& SettingsB, float Alpha, FLight_AtmosphericSettings& OutSettings)
{
    OutSettings.SunIntensity = FMath::Lerp(SettingsA.SunIntensity, SettingsB.SunIntensity, Alpha);
    OutSettings.SunColor = FMath::Lerp(SettingsA.SunColor, SettingsB.SunColor, Alpha);
    OutSettings.SunAngle = FMath::Lerp(SettingsA.SunAngle, SettingsB.SunAngle, Alpha);
    OutSettings.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    OutSettings.FogHeightFalloff = FMath::Lerp(SettingsA.FogHeightFalloff, SettingsB.FogHeightFalloff, Alpha);
    OutSettings.FogColor = FMath::Lerp(SettingsA.FogColor, SettingsB.FogColor, Alpha);
}

void ALight_DynamicAtmosphereController::InitializeAtmosphericPresets()
{
    // Dawn settings - soft warm light
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunAngle = -45.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogHeightFalloff = 0.15f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    // Noon settings - bright clear light
    NoonSettings.SunIntensity = 8.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    NoonSettings.SunAngle = 0.0f;
    NoonSettings.FogDensity = 0.015f;
    NoonSettings.FogHeightFalloff = 0.25f;
    NoonSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Dusk settings - warm orange light
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DuskSettings.SunAngle = 45.0f;
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogHeightFalloff = 0.18f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    // Night settings - cool moonlight
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    NightSettings.SunAngle = 90.0f;
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogHeightFalloff = 0.12f;
    NightSettings.FogColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
}
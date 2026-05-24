#include "EnvArt_AtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light component (sun)
    SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLightComponent->SetupAttachment(RootComponent);
    SunLightComponent->SetIntensity(3.0f);
    SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
    SunLightComponent->SetCastShadows(true);
    SunLightComponent->SetCastVolumetricShadow(true);

    // Create sky light component
    SkyLightComponent = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLightComponent->SetupAttachment(RootComponent);
    SkyLightComponent->SetIntensity(1.0f);
    SkyLightComponent->SetLightColor(FLinearColor(0.4f, 0.7f, 1.0f, 1.0f));
    SkyLightComponent->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);

    // Create exponential height fog component
    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    FogComponent->SetupAttachment(RootComponent);
    FogComponent->SetFogDensity(0.02f);
    FogComponent->SetFogHeightFalloff(0.2f);
    FogComponent->SetFogMaxOpacity(1.0f);
    FogComponent->SetStartDistance(0.0f);
    FogComponent->SetFogCutoffDistance(0.0f);

    // Initialize default settings
    GoldenHourSettings.SunAngle = 15.0f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
    GoldenHourSettings.SunIntensity = 4.0f;
    GoldenHourSettings.SkyColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    GoldenHourSettings.FogDensity = 0.03f;
    GoldenHourSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    GoldenHourSettings.FogHeight = 150.0f;

    MidDaySettings.SunAngle = 80.0f;
    MidDaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    MidDaySettings.SunIntensity = 5.0f;
    MidDaySettings.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    MidDaySettings.FogDensity = 0.015f;
    MidDaySettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MidDaySettings.FogHeight = 200.0f;

    DuskSettings.SunAngle = -10.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SkyColor = FLinearColor(0.8f, 0.4f, 0.6f, 1.0f);
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.8f, 1.0f);
    DuskSettings.FogHeight = 100.0f;

    // Initialize volumetric fog settings
    VolumetricFogSettings.bEnableVolumetricFog = true;
    VolumetricFogSettings.VolumetricFogScatteringDistribution = 0.2f;
    VolumetricFogSettings.VolumetricFogAlbedo = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    VolumetricFogSettings.VolumetricFogExtinctionScale = 1.0f;
    VolumetricFogSettings.ViewDistance = 6000.0f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Apply initial golden hour settings
    SetGoldenHourLighting();
    UpdateVolumetricFog();
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoCycleDayNight && DayCycleDuration > 0.0f)
    {
        CurrentTimeOfDay += DeltaTime / DayCycleDuration;
        if (CurrentTimeOfDay > 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }
        UpdateLightingBasedOnTime();
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float TimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDay, 0.0f, 1.0f);
    UpdateLightingBasedOnTime();
}

void AEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    ApplyTimeOfDaySettings(GoldenHourSettings);
    CurrentTimeOfDay = 0.25f; // Golden hour time
}

void AEnvArt_AtmosphereManager::SetMidDayLighting()
{
    ApplyTimeOfDaySettings(MidDaySettings);
    CurrentTimeOfDay = 0.5f; // Midday time
}

void AEnvArt_AtmosphereManager::SetDuskLighting()
{
    ApplyTimeOfDaySettings(DuskSettings);
    CurrentTimeOfDay = 0.75f; // Dusk time
}

void AEnvArt_AtmosphereManager::ApplyTimeOfDaySettings(const FEnvArt_TimeOfDaySettings& Settings)
{
    if (SunLightComponent)
    {
        SunLightComponent->SetIntensity(Settings.SunIntensity);
        SunLightComponent->SetLightColor(Settings.SunColor);
        
        // Set sun rotation based on angle
        FRotator SunRotation = FRotator(-Settings.SunAngle, 0.0f, 0.0f);
        SunLightComponent->SetWorldRotation(SunRotation);
    }

    if (SkyLightComponent)
    {
        SkyLightComponent->SetLightColor(Settings.SkyColor);
        SkyLightComponent->RecaptureSky();
    }

    if (FogComponent)
    {
        FogComponent->SetFogDensity(Settings.FogDensity);
        FogComponent->SetFogInscatteringColor(Settings.FogColor);
        FogComponent->SetFogHeightFalloff(0.2f / Settings.FogHeight);
    }
}

void AEnvArt_AtmosphereManager::UpdateVolumetricFog()
{
    if (FogComponent)
    {
        FogComponent->SetVolumetricFog(VolumetricFogSettings.bEnableVolumetricFog);
        FogComponent->SetVolumetricFogScatteringDistribution(VolumetricFogSettings.VolumetricFogScatteringDistribution);
        FogComponent->SetVolumetricFogAlbedo(VolumetricFogSettings.VolumetricFogAlbedo);
        FogComponent->SetVolumetricFogExtinctionScale(VolumetricFogSettings.VolumetricFogExtinctionScale);
        FogComponent->SetVolumetricFogDistance(VolumetricFogSettings.ViewDistance);
    }
}

void AEnvArt_AtmosphereManager::EnableForestAtmosphere()
{
    // Forest-specific atmosphere: dappled light, green-tinted fog
    FEnvArt_TimeOfDaySettings ForestSettings = GoldenHourSettings;
    ForestSettings.SunIntensity *= 0.7f; // Reduce intensity for forest canopy
    ForestSettings.FogColor = FLinearColor(0.8f, 1.0f, 0.8f, 1.0f); // Green tint
    ForestSettings.FogDensity = 0.04f; // Denser fog
    ApplyTimeOfDaySettings(ForestSettings);

    // Enable volumetric fog for light shafts
    VolumetricFogSettings.bEnableVolumetricFog = true;
    VolumetricFogSettings.VolumetricFogScatteringDistribution = 0.3f;
    UpdateVolumetricFog();
}

void AEnvArt_AtmosphereManager::EnableVolcanicAtmosphere()
{
    // Volcanic atmosphere: orange/red tints, heavy fog
    FEnvArt_TimeOfDaySettings VolcanicSettings = GoldenHourSettings;
    VolcanicSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Orange sun
    VolcanicSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.4f, 1.0f); // Red sky
    VolcanicSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f); // Sulfurous fog
    VolcanicSettings.FogDensity = 0.06f; // Heavy fog
    ApplyTimeOfDaySettings(VolcanicSettings);

    // Thick volumetric fog
    VolumetricFogSettings.VolumetricFogExtinctionScale = 1.5f;
    VolumetricFogSettings.VolumetricFogAlbedo = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    UpdateVolumetricFog();
}

void AEnvArt_AtmosphereManager::EnableRiverValleyAtmosphere()
{
    // River valley atmosphere: misty, cool blues and greens
    FEnvArt_TimeOfDaySettings ValleySettings = GoldenHourSettings;
    ValleySettings.SkyColor = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f); // Cool blue sky
    ValleySettings.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f); // Cool mist
    ValleySettings.FogDensity = 0.025f; // Light mist
    ValleySettings.FogHeight = 300.0f; // Higher fog layer
    ApplyTimeOfDaySettings(ValleySettings);

    // Soft volumetric fog
    VolumetricFogSettings.VolumetricFogScatteringDistribution = 0.1f;
    VolumetricFogSettings.VolumetricFogExtinctionScale = 0.8f;
    UpdateVolumetricFog();
}

void AEnvArt_AtmosphereManager::UpdateLightingBasedOnTime()
{
    FEnvArt_TimeOfDaySettings CurrentSettings;
    
    if (CurrentTimeOfDay < 0.25f)
    {
        // Dawn to Golden Hour
        float Alpha = CurrentTimeOfDay / 0.25f;
        CurrentSettings = InterpolateTimeOfDaySettings(DuskSettings, GoldenHourSettings, Alpha);
    }
    else if (CurrentTimeOfDay < 0.5f)
    {
        // Golden Hour to Midday
        float Alpha = (CurrentTimeOfDay - 0.25f) / 0.25f;
        CurrentSettings = InterpolateTimeOfDaySettings(GoldenHourSettings, MidDaySettings, Alpha);
    }
    else if (CurrentTimeOfDay < 0.75f)
    {
        // Midday to Dusk
        float Alpha = (CurrentTimeOfDay - 0.5f) / 0.25f;
        CurrentSettings = InterpolateTimeOfDaySettings(MidDaySettings, DuskSettings, Alpha);
    }
    else
    {
        // Dusk to Dawn
        float Alpha = (CurrentTimeOfDay - 0.75f) / 0.25f;
        CurrentSettings = InterpolateTimeOfDaySettings(DuskSettings, DuskSettings, Alpha); // Night stays at dusk settings
    }
    
    ApplyTimeOfDaySettings(CurrentSettings);
}

FEnvArt_TimeOfDaySettings AEnvArt_AtmosphereManager::InterpolateTimeOfDaySettings(const FEnvArt_TimeOfDaySettings& SettingsA, const FEnvArt_TimeOfDaySettings& SettingsB, float Alpha)
{
    FEnvArt_TimeOfDaySettings Result;
    
    Result.SunAngle = FMath::Lerp(SettingsA.SunAngle, SettingsB.SunAngle, Alpha);
    Result.SunColor = FMath::Lerp(SettingsA.SunColor, SettingsB.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(SettingsA.SunIntensity, SettingsB.SunIntensity, Alpha);
    Result.SkyColor = FMath::Lerp(SettingsA.SkyColor, SettingsB.SkyColor, Alpha);
    Result.FogDensity = FMath::Lerp(SettingsA.FogDensity, SettingsB.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(SettingsA.FogColor, SettingsB.FogColor, Alpha);
    Result.FogHeight = FMath::Lerp(SettingsA.FogHeight, SettingsB.FogHeight, Alpha);
    
    return Result;
}
#include "EnvArt_AtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light component for sun
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f));
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);

    // Create sky atmosphere component
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphere->SetupAttachment(RootComponent);

    // Create height fog component
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));

    // Initialize default settings
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    DayDurationMinutes = 20.0f;
    bAutoAdvanceTime = true;
    TimeAccumulator = 0.0f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmospherePresets();
    SetCretaceousLighting();
    ApplyAtmosphereSettings(GetSettingsForTimeOfDay(CurrentTimeOfDay));
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bAutoAdvanceTime)
    {
        UpdateTimeOfDay(DeltaTime);
    }
}

void AEnvArt_AtmosphereManager::InitializeAtmospherePresets()
{
    // Dawn - soft orange light
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    // Morning - warm golden light
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunAngle = 35.0f;
    MorningSettings.FogDensity = 0.025f;
    MorningSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);

    // Midday - bright white light
    MiddaySettings.SunIntensity = 4.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MiddaySettings.SunAngle = 75.0f;
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Afternoon - warm light
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.SunAngle = 45.0f;
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);

    // Dusk - orange-red light
    DuskSettings.SunIntensity = 1.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
    DuskSettings.SunAngle = 10.0f;
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);

    // Night - cool moonlight
    NightSettings.SunIntensity = 0.3f;
    NightSettings.SunColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    NightSettings.SunAngle = -15.0f;
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    ApplyAtmosphereSettings(GetSettingsForTimeOfDay(NewTimeOfDay));
}

void AEnvArt_AtmosphereManager::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetIntensity(Settings.SunIntensity);
        SunLight->SetLightColor(Settings.SunColor);
        
        // Set sun rotation based on angle
        FRotator SunRotation = FRotator(-Settings.SunAngle, 0.0f, 0.0f);
        SunLight->SetWorldRotation(SunRotation);
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }

    if (SkyAtmosphere)
    {
        // Link sky atmosphere to directional light
        SkyAtmosphere->SetAtmosphereLightDirection(SunLight);
    }
}

void AEnvArt_AtmosphereManager::SetCretaceousLighting()
{
    // Set tropical Cretaceous atmosphere - warm, humid, clear skies
    if (SunLight)
    {
        SunLight->SetIntensity(3.5f);
        SunLight->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f, 1.0f)); // Warm tropical light
        SunLight->SetTemperature(5500.0f); // Slightly warm temperature
    }

    if (HeightFog)
    {
        HeightFog->SetFogDensity(0.02f); // Light atmospheric haze
        HeightFog->SetFogInscatteringColor(FLinearColor(0.85f, 0.9f, 1.0f, 1.0f)); // Clear blue-white
        HeightFog->SetFogHeightFalloff(0.15f); // Gentle falloff for tropical atmosphere
        HeightFog->SetStartDistance(1000.0f); // Start fog at distance for clear foreground
    }

    if (SkyAtmosphere)
    {
        // Configure for clear Cretaceous skies
        SkyAtmosphere->SetAtmosphereLightDirection(SunLight);
    }

    UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphereManager: Cretaceous lighting applied"));
}

void AEnvArt_AtmosphereManager::EnableVolumetricFog(bool bEnable)
{
    if (HeightFog)
    {
        HeightFog->SetVolumetricFog(bEnable);
        if (bEnable)
        {
            HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
            HeightFog->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f, 1.0f));
        }
    }
}

FEnvArt_AtmosphereSettings AEnvArt_AtmosphereManager::GetCurrentAtmosphereSettings() const
{
    return GetSettingsForTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereManager::UpdateTimeOfDay(float DeltaTime)
{
    TimeAccumulator += DeltaTime;
    
    // Calculate time progression (each time period is 1/6 of day duration)
    float TimePerPeriod = (DayDurationMinutes * 60.0f) / 6.0f;
    
    if (TimeAccumulator >= TimePerPeriod)
    {
        TimeAccumulator = 0.0f;
        
        // Advance to next time of day
        int32 CurrentIndex = static_cast<int32>(CurrentTimeOfDay);
        CurrentIndex = (CurrentIndex + 1) % 6;
        CurrentTimeOfDay = static_cast<EEnvArt_TimeOfDay>(CurrentIndex);
        
        ApplyAtmosphereSettings(GetSettingsForTimeOfDay(CurrentTimeOfDay));
        
        UE_LOG(LogTemp, Warning, TEXT("EnvArt_AtmosphereManager: Time advanced to %d"), CurrentIndex);
    }
}

FEnvArt_AtmosphereSettings AEnvArt_AtmosphereManager::GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            return DawnSettings;
        case EEnvArt_TimeOfDay::Morning:
            return MorningSettings;
        case EEnvArt_TimeOfDay::Midday:
            return MiddaySettings;
        case EEnvArt_TimeOfDay::Afternoon:
            return AfternoonSettings;
        case EEnvArt_TimeOfDay::Dusk:
            return DuskSettings;
        case EEnvArt_TimeOfDay::Night:
            return NightSettings;
        default:
            return MorningSettings;
    }
}

void AEnvArt_AtmosphereManager::InterpolateAtmosphereSettings(const FEnvArt_AtmosphereSettings& From, const FEnvArt_AtmosphereSettings& To, float Alpha)
{
    FEnvArt_AtmosphereSettings InterpolatedSettings;
    
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FLinearColor::LerpUsingHSV(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.SunAngle = FMath::Lerp(From.SunAngle, To.SunAngle, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.FogColor = FLinearColor::LerpUsingHSV(From.FogColor, To.FogColor, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    
    ApplyAtmosphereSettings(InterpolatedSettings);
}
#include "Light_AtmosphereManager.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DirectionalLight.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create sun directional light component
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    RootComponent = SunLight;
    
    // Create sky atmosphere component
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    
    // Create atmospheric fog component
    AtmosphericFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("AtmosphericFog"));

    // Initialize with Cretaceous tropical settings
    ApplyCretaceousAtmosphere();
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreatePostProcessVolume();
    ApplyCretaceousAtmosphere();
    UpdateSunPosition();
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
        UpdateAtmosphereSettings();
        UpdateFogSettings();
        UpdatePostProcessSettings();
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateAtmosphereSettings();
    UpdateFogSettings();
}

void ALight_AtmosphereManager::SetWeatherIntensity(float Intensity)
{
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    bIsStormy = WeatherIntensity > 0.7f;
    UpdateAtmosphereSettings();
    UpdateFogSettings();
}

void ALight_AtmosphereManager::ApplyCretaceousAtmosphere()
{
    if (SunLight)
    {
        SunLight->SetIntensity(AtmosphereSettings.SunIntensity);
        SunLight->SetLightColor(AtmosphereSettings.SunColor);
        SunLight->SetCastShadows(true);
        SunLight->SetCastVolumetricShadow(true);
    }

    if (SkyAtmosphere)
    {
        SkyAtmosphere->SetRayleighScatteringScale(AtmosphereSettings.RayleighScattering);
        SkyAtmosphere->SetMieScatteringScale(AtmosphereSettings.MieScattering);
        SkyAtmosphere->SetRayleighExponentialDistribution(8.0f);
        SkyAtmosphere->SetMieExponentialDistribution(1.2f);
        SkyAtmosphere->SetOtherAbsorptionScale(1.0f);
        SkyAtmosphere->SetArtDirection(FLinearColor(0.0f, 0.0f, 0.0f));
    }

    if (AtmosphericFog)
    {
        AtmosphericFog->SetFogDensity(AtmosphereSettings.FogDensity);
        AtmosphericFog->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
        AtmosphericFog->SetFogInscatteringColor(AtmosphereSettings.FogColor);
        AtmosphericFog->SetDirectionalInscatteringExponent(4.0f);
        AtmosphericFog->SetDirectionalInscatteringStartDistance(11000.0f);
        AtmosphericFog->SetDirectionalInscatteringColor(FLinearColor(1.0f, 0.95f, 0.8f, 1.0f));
    }
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDayEnum() const
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

void ALight_AtmosphereManager::SetAtmospherePreset(ELight_TimeOfDay TimeOfDay)
{
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            CurrentTimeOfDay = 6.0f;
            break;
        case ELight_TimeOfDay::Morning:
            CurrentTimeOfDay = 9.0f;
            break;
        case ELight_TimeOfDay::Noon:
            CurrentTimeOfDay = 12.0f;
            break;
        case ELight_TimeOfDay::Afternoon:
            CurrentTimeOfDay = 15.0f;
            break;
        case ELight_TimeOfDay::Dusk:
            CurrentTimeOfDay = 19.0f;
            break;
        case ELight_TimeOfDay::Night:
            CurrentTimeOfDay = 23.0f;
            break;
    }
    
    UpdateSunPosition();
    UpdateAtmosphereSettings();
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;

    FRotator SunRotation = CalculateSunRotation();
    SunLight->SetWorldRotation(SunRotation);
    SunLight->SetIntensity(CalculateSunIntensity());
    SunLight->SetLightColor(CalculateSunColor());
}

void ALight_AtmosphereManager::UpdateAtmosphereSettings()
{
    if (!SkyAtmosphere) return;

    // Adjust atmosphere based on time of day and weather
    float TimeBasedScattering = AtmosphereSettings.RayleighScattering;
    float WeatherModifier = 1.0f + (WeatherIntensity * 0.5f);
    
    SkyAtmosphere->SetRayleighScatteringScale(TimeBasedScattering * WeatherModifier);
    
    // Increase mie scattering during storms for hazy effect
    float MieScale = AtmosphereSettings.MieScattering * (1.0f + WeatherIntensity * 2.0f);
    SkyAtmosphere->SetMieScatteringScale(MieScale);
}

void ALight_AtmosphereManager::UpdateFogSettings()
{
    if (!AtmosphericFog) return;

    float BaseFogDensity = AtmosphereSettings.FogDensity;
    float WeatherFogMultiplier = 1.0f + (WeatherIntensity * 3.0f);
    
    AtmosphericFog->SetFogDensity(BaseFogDensity * WeatherFogMultiplier);
    
    // Adjust fog color based on time of day
    FLinearColor FogColor = AtmosphereSettings.FogColor;
    ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
    
    switch (TimeEnum)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Dusk:
            FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Warm orange
            break;
        case ELight_TimeOfDay::Night:
            FogColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f); // Cool blue
            break;
        default:
            FogColor = AtmosphereSettings.FogColor; // Default tropical blue
            break;
    }
    
    AtmosphericFog->SetFogInscatteringColor(FogColor);
}

float ALight_AtmosphereManager::CalculateSunIntensity() const
{
    // Sun intensity based on time of day
    float SunAngle = (CurrentTimeOfDay - 12.0f) * 15.0f; // Convert to degrees from noon
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(90.0f - FMath::Abs(SunAngle)));
    
    float BaseIntensity = AtmosphereSettings.SunIntensity;
    float IntensityMultiplier = FMath::Max(0.1f, SunHeight);
    
    // Reduce intensity during storms
    float WeatherMultiplier = 1.0f - (WeatherIntensity * 0.6f);
    
    return BaseIntensity * IntensityMultiplier * WeatherMultiplier;
}

FLinearColor ALight_AtmosphereManager::CalculateSunColor() const
{
    ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
    FLinearColor BaseColor = AtmosphereSettings.SunColor;
    
    switch (TimeEnum)
    {
        case ELight_TimeOfDay::Dawn:
            return FLinearColor(1.0f, 0.7f, 0.5f, 1.0f); // Warm orange
        case ELight_TimeOfDay::Dusk:
            return FLinearColor(1.0f, 0.6f, 0.4f, 1.0f); // Deep orange
        case ELight_TimeOfDay::Night:
            return FLinearColor(0.4f, 0.5f, 0.8f, 1.0f); // Cool moonlight
        default:
            return BaseColor; // Warm tropical daylight
    }
}

FRotator ALight_AtmosphereManager::CalculateSunRotation() const
{
    // Calculate sun position based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 0 degrees at 6 AM (sunrise)
    float Pitch = SunAngle - 90.0f; // Convert to UE4 pitch (negative = up)
    
    // Clamp to prevent sun going below horizon too much
    Pitch = FMath::Clamp(Pitch, -85.0f, 85.0f);
    
    return FRotator(Pitch, 0.0f, 0.0f);
}

void ALight_AtmosphereManager::FindOrCreatePostProcessVolume()
{
    // Try to find existing post-process volume
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
    else
    {
        // Create new post-process volume
        PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
        if (PostProcessVolume)
        {
            PostProcessVolume->bUnbound = true;
            PostProcessVolume->SetActorLabel(TEXT("CretaceousPostProcess"));
        }
    }
}

void ALight_AtmosphereManager::UpdatePostProcessSettings()
{
    if (!PostProcessVolume) return;

    FPostProcessSettings& Settings = PostProcessVolume->Settings;
    
    // Adjust exposure based on time of day
    ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
    float ExposureBias = 0.5f;
    
    switch (TimeEnum)
    {
        case ELight_TimeOfDay::Dawn:
        case ELight_TimeOfDay::Dusk:
            ExposureBias = 0.3f; // Slightly darker for dramatic effect
            break;
        case ELight_TimeOfDay::Night:
            ExposureBias = -0.5f; // Much darker
            break;
        default:
            ExposureBias = 0.5f; // Bright tropical daylight
            break;
    }
    
    // Adjust for weather
    ExposureBias -= WeatherIntensity * 0.3f;
    
    Settings.bOverride_AutoExposureMethod = true;
    Settings.AutoExposureMethod = AEM_Manual;
    Settings.bOverride_AutoExposureBias = true;
    Settings.AutoExposureBias = ExposureBias;
    
    // Bloom settings for tropical atmosphere
    Settings.bOverride_BloomIntensity = true;
    Settings.BloomIntensity = 0.675f + (WeatherIntensity * 0.2f);
    Settings.bOverride_BloomThreshold = true;
    Settings.BloomThreshold = 1.0f;
}
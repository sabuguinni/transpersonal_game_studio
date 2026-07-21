#include "EnvironmentAtmosphereSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

AEnvironmentAtmosphereSystem::AEnvironmentAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize sun light
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor::White);
    SunLight->SetCastShadows(true);

    // Initialize sky light
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetSourceType(SLS_CapturedScene);

    // Initialize sky dome
    SkyDome = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SkyDome"));
    SkyDome->SetupAttachment(RootComponent);

    // Initialize default values
    CurrentTimeOfDay = 12.0f; // Noon
    DayDurationInMinutes = 20.0f; // 20 minute day cycle
    bEnableTimeProgression = true;

    // Default lighting colors
    SunriseColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    NoonColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    SunsetColor = FLinearColor(1.0f, 0.4f, 0.2f, 1.0f);
    NightColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);

    MaxSunIntensity = 8.0f;
    MinSunIntensity = 0.1f;

    // Default fog settings
    FogDensity = 0.02f;
    FogHeightFalloff = 0.2f;
    FogInscatteringColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
    FogMaxOpacity = 1.0f;

    // Default weather
    CurrentWeather = EEnvArt_WeatherType::Clear;
    WeatherTransitionSpeed = 1.0f;
}

void AEnvironmentAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeAtmosphere();
}

void AEnvironmentAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableTimeProgression)
    {
        UpdateTimeProgression(DeltaTime);
    }

    UpdateSunPosition();
    UpdateLightingColors();
    UpdateFogSettings();
}

void AEnvironmentAtmosphereSystem::InitializeAtmosphere()
{
    // Set initial sun position and lighting
    UpdateSunPosition();
    UpdateLightingColors();

    // Initialize biome-specific atmosphere settings
    FEnvArt_AtmosphereSettings SavannaSettings;
    SavannaSettings.FogDensity = 0.01f;
    SavannaSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    SavannaSettings.SkyTint = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    BiomeAtmosphereSettings.Add(EEnvArt_BiomeType::Savanna, SavannaSettings);

    FEnvArt_AtmosphereSettings ForestSettings;
    ForestSettings.FogDensity = 0.05f;
    ForestSettings.FogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);
    ForestSettings.SkyTint = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);
    BiomeAtmosphereSettings.Add(EEnvArt_BiomeType::Forest, ForestSettings);

    FEnvArt_AtmosphereSettings DesertSettings;
    DesertSettings.FogDensity = 0.005f;
    DesertSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DesertSettings.SkyTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    BiomeAtmosphereSettings.Add(EEnvArt_BiomeType::Desert, DesertSettings);
}

void AEnvironmentAtmosphereSystem::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateLightingColors();
}

void AEnvironmentAtmosphereSystem::SetWeather(EEnvArt_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    // Weather transition logic would be implemented here
}

void AEnvironmentAtmosphereSystem::ApplyBiomeAtmosphere(EEnvArt_BiomeType BiomeType)
{
    if (BiomeAtmosphereSettings.Contains(BiomeType))
    {
        FEnvArt_AtmosphereSettings Settings = BiomeAtmosphereSettings[BiomeType];
        FogDensity = Settings.FogDensity;
        FogInscatteringColor = Settings.FogColor;
        
        if (SkyLight)
        {
            SkyLight->SetLightColor(Settings.SkyTint);
        }
    }
}

void AEnvironmentAtmosphereSystem::UpdateSunPosition()
{
    if (SunLight)
    {
        FRotator SunRotation = CalculateSunRotation();
        SunLight->SetWorldRotation(SunRotation);
    }
}

void AEnvironmentAtmosphereSystem::UpdateLightingColors()
{
    if (SunLight)
    {
        FLinearColor SunColor = CalculateSunColor();
        float SunIntensity = CalculateSunIntensity();
        
        SunLight->SetLightColor(SunColor);
        SunLight->SetIntensity(SunIntensity);
    }
}

void AEnvironmentAtmosphereSystem::UpdateFogSettings()
{
    // Update exponential height fog in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AExponentialHeightFog> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AExponentialHeightFog* FogActor = *ActorIterator;
            if (FogActor && FogActor->GetComponent())
            {
                FogActor->GetComponent()->SetFogDensity(FogDensity);
                FogActor->GetComponent()->SetFogHeightFalloff(FogHeightFalloff);
                FogActor->GetComponent()->SetFogInscatteringColor(FogInscatteringColor);
                FogActor->GetComponent()->SetFogMaxOpacity(FogMaxOpacity);
            }
        }
    }
}

void AEnvironmentAtmosphereSystem::UpdateTimeProgression(float DeltaTime)
{
    if (DayDurationInMinutes > 0.0f)
    {
        float TimeIncrement = (DeltaTime / 60.0f) * (24.0f / DayDurationInMinutes);
        CurrentTimeOfDay += TimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
    }
}

FLinearColor AEnvironmentAtmosphereSystem::CalculateSunColor() const
{
    // Calculate sun color based on time of day
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
    {
        // Sunrise
        float Alpha = (CurrentTimeOfDay - 5.0f) / 2.0f;
        return FMath::Lerp(NightColor, SunriseColor, Alpha);
    }
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
    {
        // Morning to noon
        float Alpha = (CurrentTimeOfDay - 7.0f) / 4.0f;
        return FMath::Lerp(SunriseColor, NoonColor, Alpha);
    }
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 13.0f)
    {
        // Noon
        return NoonColor;
    }
    else if (CurrentTimeOfDay >= 13.0f && CurrentTimeOfDay < 17.0f)
    {
        // Afternoon
        return NoonColor;
    }
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 19.0f)
    {
        // Sunset
        float Alpha = (CurrentTimeOfDay - 17.0f) / 2.0f;
        return FMath::Lerp(NoonColor, SunsetColor, Alpha);
    }
    else
    {
        // Night
        return NightColor;
    }
}

float AEnvironmentAtmosphereSystem::CalculateSunIntensity() const
{
    // Calculate sun intensity based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // Convert to degrees (6am = 0°, 6pm = 180°)
    float SunHeight = FMath::Sin(FMath::DegreesToRadians(SunAngle));
    
    if (SunHeight > 0.0f)
    {
        return FMath::Lerp(MinSunIntensity, MaxSunIntensity, SunHeight);
    }
    else
    {
        return MinSunIntensity;
    }
}

FRotator AEnvironmentAtmosphereSystem::CalculateSunRotation() const
{
    // Calculate sun rotation based on time of day
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 6am = 0°, 6pm = 180°
    
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle - 90.0f; // Adjust for directional light orientation
    SunRotation.Yaw = 0.0f;
    SunRotation.Roll = 0.0f;
    
    return SunRotation;
}
#include "AtmosphericLightingSystem.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AAtmosphericLightingSystem::AAtmosphericLightingSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Initialize lighting components
    SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLightComponent->SetupAttachment(RootComponent);
    SunLightComponent->SetIntensity(5.0f);
    SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
    SunLightComponent->SetCastShadows(true);
    SunLightComponent->SetMobility(EComponentMobility::Movable);

    AtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("Atmosphere"));
    AtmosphereComponent->SetupAttachment(RootComponent);

    CloudComponent = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("Clouds"));
    CloudComponent->SetupAttachment(RootComponent);

    FogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("Fog"));
    FogComponent->SetupAttachment(RootComponent);
    FogComponent->SetFogDensity(0.02f);
    FogComponent->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));

    // Initialize default settings for Cretaceous atmosphere
    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    NoonSettings.SunAngle = 60.0f;
    NoonSettings.AtmosphereScattering = 0.0331f;
    NoonSettings.FogDensity = 0.02f;
    NoonSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.AtmosphereScattering = 0.05f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.SunAngle = 10.0f;
    DuskSettings.AtmosphereScattering = 0.06f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SunAngle = -30.0f;
    NightSettings.AtmosphereScattering = 0.02f;
    NightSettings.FogDensity = 0.01f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);

    // Initialize weather settings
    CurrentWeather.CloudCoverage = 0.3f;
    CurrentWeather.CloudDensity = 0.5f;
    CurrentWeather.bIsRaining = false;
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.WindStrength = 1.0f;

    // Initialize cached references
    WorldSun = nullptr;
    WorldAtmosphere = nullptr;
    WorldClouds = nullptr;
    WorldFog = nullptr;
}

void AAtmosphericLightingSystem::BeginPlay()
{
    Super::BeginPlay();

    // Find and cache world lighting actors
    FindAndCacheLightingActors();

    // Apply initial Cretaceous atmosphere settings
    ApplyCretaceousAtmosphere();

    // Set initial time of day
    SetTimeOfDay(TimeOfDay);
}

void AAtmosphericLightingSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        // Update time of day
        float TimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        TimeOfDay += TimeIncrement;
        
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }

        // Update lighting based on time
        UpdateSunPosition();
        UpdateAtmosphereSettings();
        UpdateFogSettings();
        UpdateCloudSettings();
    }
}

void AAtmosphericLightingSystem::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    
    if (!bEnableDayNightCycle)
    {
        UpdateSunPosition();
        UpdateAtmosphereSettings();
        UpdateFogSettings();
        UpdateCloudSettings();
    }
}

void AAtmosphericLightingSystem::SetDayNightCycleEnabled(bool bEnabled)
{
    bEnableDayNightCycle = bEnabled;
}

void AAtmosphericLightingSystem::SetWeatherSettings(const FLight_WeatherSettings& NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateCloudSettings();
}

void AAtmosphericLightingSystem::StartRain(float Intensity)
{
    CurrentWeather.bIsRaining = true;
    CurrentWeather.RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    CurrentWeather.CloudCoverage = FMath::Max(CurrentWeather.CloudCoverage, 0.7f);
    CurrentWeather.CloudDensity = FMath::Max(CurrentWeather.CloudDensity, 0.8f);
    UpdateCloudSettings();
    UpdateFogSettings();
}

void AAtmosphericLightingSystem::StopRain()
{
    CurrentWeather.bIsRaining = false;
    CurrentWeather.RainIntensity = 0.0f;
    UpdateCloudSettings();
    UpdateFogSettings();
}

void AAtmosphericLightingSystem::RegisterInteriorLight(APointLight* Light)
{
    if (Light && !InteriorLights.Contains(Light))
    {
        InteriorLights.Add(Light);
        UpdateInteriorLighting();
    }
}

void AAtmosphericLightingSystem::UpdateInteriorLighting()
{
    for (APointLight* Light : InteriorLights)
    {
        if (Light && Light->GetLightComponent())
        {
            Light->GetLightComponent()->SetIntensity(InteriorLightIntensity);
            Light->GetLightComponent()->SetLightColor(FireLightColor);
            Light->GetLightComponent()->SetAttenuationRadius(1000.0f);
        }
    }
}

void AAtmosphericLightingSystem::ApplyCretaceousAtmosphere()
{
    // Apply tropical Cretaceous atmosphere settings
    if (SunLightComponent)
    {
        SunLightComponent->SetIntensity(5.0f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
        SunLightComponent->SetTemperature(5500.0f);
    }

    if (AtmosphereComponent)
    {
        AtmosphereComponent->SetRayleighScatteringScale(0.0331f);
        AtmosphereComponent->SetMieScatteringScale(0.003996f);
        AtmosphereComponent->SetRayleighExponentialDistribution(8.0f);
        AtmosphereComponent->SetMieExponentialDistribution(1.2f);
    }

    if (FogComponent)
    {
        FogComponent->SetFogDensity(0.02f);
        FogComponent->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
        FogComponent->SetFogHeightFalloff(0.2f);
        FogComponent->SetFogMaxOpacity(1.0f);
    }

    if (CloudComponent)
    {
        CloudComponent->SetLayerBottomAltitude(1.5f);
        CloudComponent->SetLayerHeight(4.0f);
        CloudComponent->SetTracingStartMaxDistance(350.0f);
        CloudComponent->SetTracingMaxDistance(50.0f);
    }
}

void AAtmosphericLightingSystem::FixAtmosphericSettings()
{
    // Find and fix all atmospheric actors in the world
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
        {
            if (UDirectionalLightComponent* LightComp = DirLight->GetComponent())
            {
                LightComp->SetIntensity(5.0f);
                LightComp->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f, 1.0f));
            }
        }
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ASkyAtmosphere* SkyAtmo = Cast<ASkyAtmosphere>(Actor))
        {
            if (USkyAtmosphereComponent* AtmoComp = SkyAtmo->GetComponent())
            {
                AtmoComp->SetRayleighScatteringScale(0.0331f);
            }
        }
    }

    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (AVolumetricCloud* Cloud = Cast<AVolumetricCloud>(Actor))
        {
            Cloud->SetActorHiddenInGame(true);
        }
    }
}

void AAtmosphericLightingSystem::UpdateSunPosition()
{
    float SunAngle = GetSunAngleFromTime(TimeOfDay);
    
    if (SunLightComponent)
    {
        FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
        SunLightComponent->SetWorldRotation(SunRotation);
    }

    if (WorldSun && WorldSun->GetLightComponent())
    {
        FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
        WorldSun->SetActorRotation(SunRotation);
    }
}

void AAtmosphericLightingSystem::UpdateAtmosphereSettings()
{
    FLight_TimeOfDaySettings CurrentSettings = GetCurrentTimeSettings();
    
    if (SunLightComponent)
    {
        SunLightComponent->SetIntensity(CurrentSettings.SunIntensity);
        SunLightComponent->SetLightColor(CurrentSettings.SunColor);
    }

    if (WorldSun && WorldSun->GetLightComponent())
    {
        WorldSun->GetLightComponent()->SetIntensity(CurrentSettings.SunIntensity);
        WorldSun->GetLightComponent()->SetLightColor(CurrentSettings.SunColor);
    }

    if (AtmosphereComponent)
    {
        AtmosphereComponent->SetRayleighScatteringScale(CurrentSettings.AtmosphereScattering);
    }

    if (WorldAtmosphere && WorldAtmosphere->GetComponent())
    {
        WorldAtmosphere->GetComponent()->SetRayleighScatteringScale(CurrentSettings.AtmosphereScattering);
    }
}

void AAtmosphericLightingSystem::UpdateFogSettings()
{
    FLight_TimeOfDaySettings CurrentSettings = GetCurrentTimeSettings();
    
    float FogDensityMultiplier = CurrentWeather.bIsRaining ? (1.0f + CurrentWeather.RainIntensity) : 1.0f;
    
    if (FogComponent)
    {
        FogComponent->SetFogDensity(CurrentSettings.FogDensity * FogDensityMultiplier);
        FogComponent->SetFogInscatteringColor(CurrentSettings.FogColor);
    }

    if (WorldFog && WorldFog->GetComponent())
    {
        WorldFog->GetComponent()->SetFogDensity(CurrentSettings.FogDensity * FogDensityMultiplier);
        WorldFog->GetComponent()->SetFogInscatteringColor(CurrentSettings.FogColor);
    }
}

void AAtmosphericLightingSystem::UpdateCloudSettings()
{
    if (CloudComponent)
    {
        CloudComponent->SetLayerBottomAltitude(1.5f);
        CloudComponent->SetLayerHeight(4.0f + (CurrentWeather.CloudCoverage * 2.0f));
    }

    if (WorldClouds)
    {
        bool bShouldHideClouds = !CurrentWeather.bIsRaining && CurrentWeather.CloudCoverage < 0.5f;
        WorldClouds->SetActorHiddenInGame(bShouldHideClouds);
    }
}

void AAtmosphericLightingSystem::InterpolateLightingSettings(float Alpha, const FLight_TimeOfDaySettings& SettingsA, const FLight_TimeOfDaySettings& SettingsB)
{
    // This function would interpolate between two lighting settings based on Alpha
    // Implementation would blend all the lighting parameters smoothly
}

float AAtmosphericLightingSystem::GetSunAngleFromTime(float Time) const
{
    // Convert time (0-24) to sun angle (-90 to 90 degrees)
    // Noon (12) = 90 degrees (overhead)
    // Midnight (0/24) = -90 degrees (below horizon)
    float NormalizedTime = (Time - 12.0f) / 12.0f; // -1 to 1
    return NormalizedTime * 90.0f;
}

FLight_TimeOfDaySettings AAtmosphericLightingSystem::GetCurrentTimeSettings() const
{
    // Determine which time period we're in and interpolate accordingly
    if (TimeOfDay >= 5.0f && TimeOfDay < 8.0f)
    {
        // Dawn period
        return DawnSettings;
    }
    else if (TimeOfDay >= 8.0f && TimeOfDay < 17.0f)
    {
        // Day period
        return NoonSettings;
    }
    else if (TimeOfDay >= 17.0f && TimeOfDay < 20.0f)
    {
        // Dusk period
        return DuskSettings;
    }
    else
    {
        // Night period
        return NightSettings;
    }
}

void AAtmosphericLightingSystem::FindAndCacheLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;
    
    // Find DirectionalLight
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        WorldSun = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        WorldAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find VolumetricCloud
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        WorldClouds = Cast<AVolumetricCloud>(FoundActors[0]);
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        WorldFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}
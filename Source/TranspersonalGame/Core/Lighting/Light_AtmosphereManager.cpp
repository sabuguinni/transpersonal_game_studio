#include "Light_AtmosphereManager.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create atmosphere components
    SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    RootComponent = SkyAtmosphere;

    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);

    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);

    VolumetricClouds = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricClouds->SetupAttachment(RootComponent);

    // Initialize default weather settings
    ClearWeatherSettings.SunIntensity = 10.0f;
    ClearWeatherSettings.SunColor = FLinearColor::White;
    ClearWeatherSettings.FogDensity = 0.01f;
    ClearWeatherSettings.CloudCoverage = 0.2f;
    ClearWeatherSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);

    CloudyWeatherSettings.SunIntensity = 6.0f;
    CloudyWeatherSettings.SunColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    CloudyWeatherSettings.FogDensity = 0.03f;
    CloudyWeatherSettings.CloudCoverage = 0.7f;
    CloudyWeatherSettings.FogColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);

    StormyWeatherSettings.SunIntensity = 3.0f;
    StormyWeatherSettings.SunColor = FLinearColor(0.7f, 0.7f, 0.6f, 1.0f);
    StormyWeatherSettings.FogDensity = 0.08f;
    StormyWeatherSettings.CloudCoverage = 0.9f;
    StormyWeatherSettings.FogColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);

    CurrentSettings = ClearWeatherSettings;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindOrCreateSunLight();
    ApplyAtmosphereSettings(CurrentSettings);
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }
}

void ALight_AtmosphereManager::FindOrCreateSunLight()
{
    // Find existing directional light
    for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }

    if (!SunLight)
    {
        // Create new directional light if none exists
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SunLight");
        SunLight = GetWorld()->SpawnActor<ADirectionalLight>(SpawnParams);
        
        if (SunLight)
        {
            SunLight->SetActorLocation(FVector(0, 0, 500));
            SunLight->SetActorRotation(FRotator(-45, 45, 0));
        }
    }
}

void ALight_AtmosphereManager::SetWeatherState(ELight_WeatherState NewWeatherState)
{
    CurrentWeatherState = NewWeatherState;
    
    FLight_AtmosphereSettings NewSettings;
    switch (NewWeatherState)
    {
        case ELight_WeatherState::Clear:
            NewSettings = ClearWeatherSettings;
            break;
        case ELight_WeatherState::Cloudy:
        case ELight_WeatherState::Overcast:
            NewSettings = CloudyWeatherSettings;
            break;
        case ELight_WeatherState::Foggy:
        case ELight_WeatherState::Stormy:
            NewSettings = StormyWeatherSettings;
            break;
        default:
            NewSettings = ClearWeatherSettings;
            break;
    }
    
    ApplyAtmosphereSettings(NewSettings);
}

void ALight_AtmosphereManager::TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionTime)
{
    CurrentWeatherState = TargetWeather;
    
    switch (TargetWeather)
    {
        case ELight_WeatherState::Clear:
            TargetSettings = ClearWeatherSettings;
            break;
        case ELight_WeatherState::Cloudy:
        case ELight_WeatherState::Overcast:
            TargetSettings = CloudyWeatherSettings;
            break;
        case ELight_WeatherState::Foggy:
        case ELight_WeatherState::Stormy:
            TargetSettings = StormyWeatherSettings;
            break;
        default:
            TargetSettings = ClearWeatherSettings;
            break;
    }
    
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    TransitionDuration = TransitionTime;
}

void ALight_AtmosphereManager::UpdateAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    ApplyAtmosphereSettings(Settings);
    CurrentSettings = Settings;
}

void ALight_AtmosphereManager::ApplyAtmosphereSettings(const FLight_AtmosphereSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        SunLight->GetLightComponent()->SetIntensity(Settings.SunIntensity);
        SunLight->GetLightComponent()->SetLightColor(Settings.SunColor);
    }

    // Apply fog settings
    if (HeightFog)
    {
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
    }

    // Apply sky light settings
    if (SkyLight)
    {
        SkyLight->SetIntensity(Settings.SunIntensity * 0.3f);
        SkyLight->RecaptureSky();
    }

    // Apply volumetric cloud settings
    if (VolumetricClouds)
    {
        // Note: UVolumetricCloudComponent doesn't have direct coverage property
        // This would need to be set via material parameters in a real implementation
    }
}

void ALight_AtmosphereManager::UpdateTransition(float DeltaTime)
{
    TransitionProgress += DeltaTime / TransitionDuration;
    
    if (TransitionProgress >= 1.0f)
    {
        TransitionProgress = 1.0f;
        bIsTransitioning = false;
        CurrentSettings = TargetSettings;
    }
    
    FLight_AtmosphereSettings InterpolatedSettings = LerpSettings(CurrentSettings, TargetSettings, TransitionProgress);
    ApplyAtmosphereSettings(InterpolatedSettings);
    
    if (!bIsTransitioning)
    {
        CurrentSettings = TargetSettings;
    }
}

FLight_AtmosphereSettings ALight_AtmosphereManager::LerpSettings(const FLight_AtmosphereSettings& A, const FLight_AtmosphereSettings& B, float Alpha)
{
    FLight_AtmosphereSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    
    return Result;
}
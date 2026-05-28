#include "DynamicLightingManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

void UDynamicLightingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default settings
    CurrentTimeSettings.TimeOfDay = 12.0f;
    CurrentTimeSettings.SunColor = FLinearColor::White;
    CurrentTimeSettings.SunIntensity = 10.0f;
    CurrentTimeSettings.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f);
    CurrentTimeSettings.FogDensity = 0.02f;
    CurrentTimeSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);

    CurrentWeatherSettings.WeatherType = EWeatherType::Clear;
    CurrentWeatherSettings.CloudCoverage = 0.3f;
    CurrentWeatherSettings.RainIntensity = 0.0f;
    CurrentWeatherSettings.WindStrength = 1.0f;

    UE_LOG(LogTemp, Log, TEXT("DynamicLightingManager initialized"));
}

bool UDynamicLightingManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UDynamicLightingManager::Tick(float DeltaTime)
{
    if (!bActorsFound)
    {
        FindLightingActors();
    }

    if (bDynamicTimeEnabled)
    {
        AccumulatedTime += DeltaTime;
        
        // Progress time: 1 real minute = 1 game hour by default
        if (AccumulatedTime >= (60.0f / TimeProgressionSpeed))
        {
            AccumulatedTime = 0.0f;
            CurrentTimeSettings.TimeOfDay += 1.0f;
            
            if (CurrentTimeSettings.TimeOfDay >= 24.0f)
            {
                CurrentTimeSettings.TimeOfDay = 0.0f;
            }
            
            UpdateSunPosition();
            UpdateSkyLighting();
            UpdateFogSettings();
        }
    }

    UpdateWeatherEffects();
}

void UDynamicLightingManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeSettings.TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateSkyLighting();
    UpdateFogSettings();
}

void UDynamicLightingManager::SetWeather(EWeatherType NewWeather)
{
    CurrentWeatherSettings.WeatherType = NewWeather;
    
    switch (NewWeather)
    {
        case EWeatherType::Clear:
            CurrentWeatherSettings.CloudCoverage = 0.2f;
            CurrentWeatherSettings.RainIntensity = 0.0f;
            break;
        case EWeatherType::Cloudy:
            CurrentWeatherSettings.CloudCoverage = 0.7f;
            CurrentWeatherSettings.RainIntensity = 0.0f;
            break;
        case EWeatherType::Rainy:
            CurrentWeatherSettings.CloudCoverage = 0.9f;
            CurrentWeatherSettings.RainIntensity = 0.8f;
            break;
        case EWeatherType::Stormy:
            CurrentWeatherSettings.CloudCoverage = 1.0f;
            CurrentWeatherSettings.RainIntensity = 1.0f;
            break;
    }
    
    UpdateWeatherEffects();
}

void UDynamicLightingManager::EnableDynamicTimeProgression(bool bEnable)
{
    bDynamicTimeEnabled = bEnable;
}

void UDynamicLightingManager::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }

    // Convert time of day to sun angle
    float SunAngle = (CurrentTimeSettings.TimeOfDay - 6.0f) * 15.0f; // 6AM = 0 degrees
    float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    float SunYaw = (CurrentTimeSettings.TimeOfDay / 24.0f) * 360.0f;

    FRotator SunRotation(SunPitch, SunYaw, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Adjust sun intensity based on time
    float IntensityMultiplier = 1.0f;
    if (CurrentTimeSettings.TimeOfDay < 6.0f || CurrentTimeSettings.TimeOfDay > 18.0f)
    {
        // Night time - very dim
        IntensityMultiplier = 0.1f;
    }
    else if (CurrentTimeSettings.TimeOfDay < 8.0f || CurrentTimeSettings.TimeOfDay > 16.0f)
    {
        // Dawn/Dusk - moderate
        IntensityMultiplier = 0.5f;
    }

    SunLight->GetLightComponent()->SetIntensity(CurrentTimeSettings.SunIntensity * IntensityMultiplier);

    // Adjust sun color based on time
    FLinearColor SunColor = CurrentTimeSettings.SunColor;
    if (CurrentTimeSettings.TimeOfDay < 8.0f || CurrentTimeSettings.TimeOfDay > 16.0f)
    {
        // Warmer colors during dawn/dusk
        SunColor = FLinearColor(1.0f, 0.7f, 0.4f);
    }
    
    SunLight->GetLightComponent()->SetLightColor(SunColor);
}

void UDynamicLightingManager::UpdateSkyLighting()
{
    if (!SkyLightActor || !SkyLightActor->GetLightComponent())
    {
        return;
    }

    // Adjust sky light intensity based on time of day
    float SkyIntensity = 1.0f;
    if (CurrentTimeSettings.TimeOfDay < 6.0f || CurrentTimeSettings.TimeOfDay > 18.0f)
    {
        SkyIntensity = 0.2f; // Night
    }
    else if (CurrentTimeSettings.TimeOfDay < 8.0f || CurrentTimeSettings.TimeOfDay > 16.0f)
    {
        SkyIntensity = 0.6f; // Dawn/Dusk
    }

    SkyLightActor->GetLightComponent()->SetIntensity(SkyIntensity);
    SkyLightActor->GetLightComponent()->RecaptureSky();
}

void UDynamicLightingManager::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }

    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
    
    // Adjust fog based on time and weather
    float FogDensity = CurrentTimeSettings.FogDensity;
    FLinearColor FogColor = CurrentTimeSettings.FogColor;

    // Increase fog at dawn/dusk
    if (CurrentTimeSettings.TimeOfDay < 8.0f || CurrentTimeSettings.TimeOfDay > 16.0f)
    {
        FogDensity *= 1.5f;
        FogColor = FLinearColor(0.9f, 0.8f, 0.7f); // Warmer fog
    }

    // Weather affects fog
    if (CurrentWeatherSettings.WeatherType == EWeatherType::Rainy || 
        CurrentWeatherSettings.WeatherType == EWeatherType::Stormy)
    {
        FogDensity *= 2.0f;
        FogColor = FLinearColor(0.6f, 0.7f, 0.8f); // Cooler, denser fog
    }

    FogComponent->SetFogDensity(FogDensity);
    FogComponent->SetFogInscatteringColor(FogColor);
}

void UDynamicLightingManager::UpdateWeatherEffects()
{
    if (!CloudActor || !CloudActor->GetVolumetricCloudComponent())
    {
        return;
    }

    UVolumetricCloudComponent* CloudComponent = CloudActor->GetVolumetricCloudComponent();
    
    // Update cloud coverage based on weather
    CloudComponent->SetLayerBottomAltitude(1.0f);
    CloudComponent->SetLayerHeight(4.0f);
    
    // Adjust cloud density based on weather type
    float CloudDensity = CurrentWeatherSettings.CloudCoverage;
    CloudComponent->SetTracingMaxDistance(CloudDensity * 50.0f);
}

void UDynamicLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }

    // Find sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    if (SkyLights.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(SkyLights[0]);
    }

    // Find fog actor
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    }

    // Find cloud actor
    TArray<AActor*> CloudActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), CloudActors);
    if (CloudActors.Num() > 0)
    {
        CloudActor = Cast<AVolumetricCloud>(CloudActors[0]);
    }

    // Find atmosphere actor
    TArray<AActor*> AtmosphereActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), AtmosphereActors);
    if (AtmosphereActors.Num() > 0)
    {
        AtmosphereActor = Cast<ASkyAtmosphere>(AtmosphereActors[0]);
    }

    bActorsFound = (SunLight != nullptr && SkyLightActor != nullptr && FogActor != nullptr);
    
    if (bActorsFound)
    {
        UE_LOG(LogTemp, Log, TEXT("DynamicLightingManager found all lighting actors"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("DynamicLightingManager missing some lighting actors"));
    }
}
#include "Light_DynamicLightingSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULight_DynamicLightingSystem::ULight_DynamicLightingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void ULight_DynamicLightingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Initialize with current settings
    UpdateLighting();
}

void ULight_DynamicLightingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoProgressTime)
    {
        // Progress time of day
        CurrentTimeSettings.TimeOfDay += (TimeSpeed * DeltaTime / 60.0f); // Convert to minutes
        
        // Wrap around 24 hours
        if (CurrentTimeSettings.TimeOfDay >= 24.0f)
        {
            CurrentTimeSettings.TimeOfDay -= 24.0f;
        }
        
        // Update lighting if time has changed significantly
        if (FMath::Abs(CurrentTimeSettings.TimeOfDay - PreviousTimeOfDay) > 0.1f)
        {
            UpdateLighting();
            PreviousTimeOfDay = CurrentTimeSettings.TimeOfDay;
        }
    }
    
    // Update weather effects if weather changed
    if (CurrentWeatherSettings.WeatherType != PreviousWeatherType)
    {
        UpdateWeatherEffects();
        PreviousWeatherType = CurrentWeatherSettings.WeatherType;
    }
}

void ULight_DynamicLightingSystem::FindLightingActors()
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
    
    // Find fog
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
    }
    
    bLightingActorsFound = (SunLight != nullptr) || (SkyLightActor != nullptr) || (FogActor != nullptr);
    
    UE_LOG(LogTemp, Warning, TEXT("Lighting System: Found %d directional lights, %d sky lights, %d fog actors"), 
           DirectionalLights.Num(), SkyLights.Num(), FogActors.Num());
}

void ULight_DynamicLightingSystem::SetTimeOfDay(float NewTime)
{
    CurrentTimeSettings.TimeOfDay = FMath::Clamp(NewTime, 0.0f, 24.0f);
    UpdateLighting();
}

void ULight_DynamicLightingSystem::SetWeatherType(EWeatherType NewWeatherType)
{
    CurrentWeatherSettings.WeatherType = NewWeatherType;
    UpdateWeatherEffects();
}

void ULight_DynamicLightingSystem::UpdateLighting()
{
    if (!bLightingActorsFound)
    {
        FindLightingActors();
    }
    
    UpdateSunPosition();
    UpdateSunLighting();
    UpdateSkyLighting();
    UpdateFogSettings();
}

void ULight_DynamicLightingSystem::UpdateSunPosition()
{
    if (!SunLight)
    {
        return;
    }
    
    float SunElevation = CalculateSunElevation(CurrentTimeSettings.TimeOfDay);
    float SunAzimuth = (CurrentTimeSettings.TimeOfDay / 24.0f) * 360.0f - 90.0f; // East to West
    
    FRotator SunRotation = FRotator(SunElevation, SunAzimuth, 0.0f);
    SunLight->SetActorRotation(SunRotation);
}

void ULight_DynamicLightingSystem::UpdateSunLighting()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }
    
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    // Calculate sun intensity based on elevation
    float SunElevation = CalculateSunElevation(CurrentTimeSettings.TimeOfDay);
    float IntensityMultiplier = FMath::Clamp(SunElevation / 90.0f, 0.0f, 1.0f);
    
    // Night time (sun below horizon)
    if (SunElevation < 0.0f)
    {
        IntensityMultiplier = 0.0f;
    }
    
    // Apply curve if available
    if (SunIntensityCurve)
    {
        float NormalizedTime = CurrentTimeSettings.TimeOfDay / 24.0f;
        IntensityMultiplier *= SunIntensityCurve->GetFloatValue(NormalizedTime);
    }
    
    LightComp->SetIntensity(CurrentTimeSettings.SunIntensity * IntensityMultiplier);
    LightComp->SetLightColor(GetSunColorForTime(CurrentTimeSettings.TimeOfDay));
}

void ULight_DynamicLightingSystem::UpdateSkyLighting()
{
    if (!SkyLightActor || !SkyLightActor->GetLightComponent())
    {
        return;
    }
    
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    
    // Update sky light intensity based on time of day
    float SunElevation = CalculateSunElevation(CurrentTimeSettings.TimeOfDay);
    float SkyIntensity = FMath::Clamp((SunElevation + 30.0f) / 120.0f, 0.1f, 1.0f);
    
    SkyComp->SetIntensity(SkyIntensity);
    
    // Recapture sky light for dynamic changes
    SkyComp->RecaptureSky();
}

void ULight_DynamicLightingSystem::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    
    // Apply fog density from curve or settings
    float FogDensity = CurrentTimeSettings.FogDensity;
    if (FogDensityCurve)
    {
        float NormalizedTime = CurrentTimeSettings.TimeOfDay / 24.0f;
        FogDensity *= FogDensityCurve->GetFloatValue(NormalizedTime);
    }
    
    FogComp->SetFogDensity(FogDensity);
    FogComp->SetFogInscatteringColor(CurrentTimeSettings.FogColor);
}

void ULight_DynamicLightingSystem::UpdateWeatherEffects()
{
    // Modify lighting based on weather
    switch (CurrentWeatherSettings.WeatherType)
    {
        case EWeatherType::Clear:
            CurrentTimeSettings.SunIntensity = 3.0f;
            CurrentTimeSettings.FogDensity = 0.02f;
            break;
            
        case EWeatherType::Cloudy:
            CurrentTimeSettings.SunIntensity = 1.5f;
            CurrentTimeSettings.FogDensity = 0.05f;
            break;
            
        case EWeatherType::Overcast:
            CurrentTimeSettings.SunIntensity = 0.8f;
            CurrentTimeSettings.FogDensity = 0.08f;
            break;
            
        case EWeatherType::Rainy:
            CurrentTimeSettings.SunIntensity = 0.5f;
            CurrentTimeSettings.FogDensity = 0.12f;
            CurrentTimeSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f);
            break;
            
        case EWeatherType::Stormy:
            CurrentTimeSettings.SunIntensity = 0.3f;
            CurrentTimeSettings.FogDensity = 0.15f;
            CurrentTimeSettings.FogColor = FLinearColor(0.4f, 0.5f, 0.6f);
            break;
            
        case EWeatherType::Foggy:
            CurrentTimeSettings.SunIntensity = 0.7f;
            CurrentTimeSettings.FogDensity = 0.25f;
            break;
    }
    
    UpdateLighting();
}

float ULight_DynamicLightingSystem::CalculateSunElevation(float TimeOfDay)
{
    // Simple sun elevation calculation
    // 6 AM = 0°, 12 PM = 90°, 6 PM = 0°, 12 AM = -90°
    float TimeRadians = (TimeOfDay - 6.0f) * (PI / 12.0f);
    return FMath::Sin(TimeRadians) * 90.0f;
}

FLinearColor ULight_DynamicLightingSystem::GetSunColorForTime(float TimeOfDay)
{
    // Dawn/Dusk colors (5-7 AM, 5-7 PM)
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 17.0f && TimeOfDay <= 19.0f))
    {
        return FLinearColor(1.0f, 0.7f, 0.4f); // Orange
    }
    
    // Day colors (7 AM - 5 PM)
    if (TimeOfDay >= 7.0f && TimeOfDay <= 17.0f)
    {
        return FLinearColor(1.0f, 0.95f, 0.8f); // Warm white
    }
    
    // Night colors
    return FLinearColor(0.3f, 0.4f, 0.6f); // Cool blue
}

FLinearColor ULight_DynamicLightingSystem::GetSkyColorForTime(float TimeOfDay)
{
    // Similar to sun color but for sky
    if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 17.0f && TimeOfDay <= 19.0f))
    {
        return FLinearColor(0.8f, 0.5f, 0.3f);
    }
    
    if (TimeOfDay >= 7.0f && TimeOfDay <= 17.0f)
    {
        return FLinearColor(0.2f, 0.5f, 1.0f);
    }
    
    return FLinearColor(0.1f, 0.2f, 0.4f);
}
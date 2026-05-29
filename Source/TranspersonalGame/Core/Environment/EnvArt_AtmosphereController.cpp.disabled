#include "EnvArt_AtmosphereController.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Set default atmosphere settings for Cretaceous period
    AtmosphereSettings.SunIntensity = 3.5f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AtmosphereSettings.SunAngle = 35.0f; // Golden hour angle
    AtmosphereSettings.FogDensity = 0.015f;
    AtmosphereSettings.FogColor = FLinearColor(0.85f, 0.9f, 0.95f, 1.0f);
    AtmosphereSettings.FogHeightFalloff = 0.15f;
    
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    TimeOfDaySpeed = 0.1f; // Slow time progression
    CurrentTimeValue = 0.6f; // Start at afternoon
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing lighting actors in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    // Apply initial Cretaceous atmosphere
    SetCretaceousAtmosphere();
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    CurrentTimeValue += (TimeOfDaySpeed * DeltaTime) / 86400.0f; // 24 hours in seconds
    if (CurrentTimeValue >= 1.0f)
    {
        CurrentTimeValue = 0.0f;
    }
    
    // Update time of day enum based on current value
    if (CurrentTimeValue < 0.1f || CurrentTimeValue > 0.9f)
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Night;
    }
    else if (CurrentTimeValue < 0.2f)
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Dawn;
    }
    else if (CurrentTimeValue < 0.4f)
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    }
    else if (CurrentTimeValue < 0.6f)
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Noon;
    }
    else if (CurrentTimeValue < 0.8f)
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    }
    else
    {
        CurrentTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    }
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Set corresponding time value
    switch (NewTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            CurrentTimeValue = 0.15f;
            break;
        case EEnvArt_TimeOfDay::Morning:
            CurrentTimeValue = 0.3f;
            break;
        case EEnvArt_TimeOfDay::Noon:
            CurrentTimeValue = 0.5f;
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            CurrentTimeValue = 0.7f;
            break;
        case EEnvArt_TimeOfDay::Dusk:
            CurrentTimeValue = 0.85f;
            break;
        case EEnvArt_TimeOfDay::Night:
            CurrentTimeValue = 0.0f;
            break;
    }
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereController::UpdateAtmosphere()
{
    UpdateSunPosition();
    UpdateFogSettings();
    UpdateSkyLightSettings();
}

void AEnvArt_AtmosphereController::SetGoldenHourLighting()
{
    AtmosphereSettings.SunIntensity = 4.0f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f); // Warm golden color
    AtmosphereSettings.SunAngle = 25.0f; // Low angle for golden hour
    AtmosphereSettings.FogDensity = 0.025f; // More atmospheric fog
    AtmosphereSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f); // Warm fog
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereController::SetCretaceousAtmosphere()
{
    // Cretaceous period had higher CO2 levels, warmer climate, and lush vegetation
    AtmosphereSettings.SunIntensity = 3.5f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f); // Slightly warm sunlight
    AtmosphereSettings.SunAngle = 45.0f; // Tropical latitude angle
    AtmosphereSettings.FogDensity = 0.02f; // Humid atmosphere
    AtmosphereSettings.FogColor = FLinearColor(0.8f, 0.9f, 0.95f, 1.0f); // Slightly blue-tinted humid air
    AtmosphereSettings.FogHeightFalloff = 0.1f; // Dense low-lying humidity
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereController::UpdateSunPosition()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }
    
    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    
    // Calculate sun position based on time of day
    float SunElevation = FMath::Sin(CurrentTimeValue * PI) * 90.0f; // 0-90 degrees
    float SunAzimuth = (CurrentTimeValue - 0.25f) * 360.0f; // East to West
    
    // Apply atmosphere settings
    SunElevation = FMath::Max(AtmosphereSettings.SunAngle, SunElevation);
    
    FRotator SunRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
    SunLight->SetActorRotation(SunRotation);
    
    // Update sun intensity and color based on time
    float IntensityMultiplier = FMath::Max(0.1f, FMath::Sin(CurrentTimeValue * PI));
    LightComponent->SetIntensity(AtmosphereSettings.SunIntensity * IntensityMultiplier);
    LightComponent->SetLightColor(AtmosphereSettings.SunColor);
}

void AEnvArt_AtmosphereController::UpdateFogSettings()
{
    if (!HeightFog || !HeightFog->GetComponent())
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
    
    // Apply atmosphere fog settings
    FogComponent->SetFogDensity(AtmosphereSettings.FogDensity);
    FogComponent->SetFogInscatteringColor(AtmosphereSettings.FogColor);
    FogComponent->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
    
    // Adjust fog based on time of day
    float TimeMultiplier = 1.0f;
    if (CurrentTimeOfDay == EEnvArt_TimeOfDay::Dawn || CurrentTimeOfDay == EEnvArt_TimeOfDay::Dusk)
    {
        TimeMultiplier = 1.5f; // More atmospheric during golden hours
    }
    else if (CurrentTimeOfDay == EEnvArt_TimeOfDay::Night)
    {
        TimeMultiplier = 0.7f; // Less visible fog at night
    }
    
    FogComponent->SetFogDensity(AtmosphereSettings.FogDensity * TimeMultiplier);
}

void AEnvArt_AtmosphereController::UpdateSkyLightSettings()
{
    if (!SkyLight || !SkyLight->GetLightComponent())
    {
        return;
    }
    
    USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
    
    // Update sky light intensity based on time of day
    float SkyIntensity = 1.0f;
    switch (CurrentTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
        case EEnvArt_TimeOfDay::Dusk:
            SkyIntensity = 0.8f;
            break;
        case EEnvArt_TimeOfDay::Night:
            SkyIntensity = 0.3f;
            break;
        case EEnvArt_TimeOfDay::Noon:
            SkyIntensity = 1.2f;
            break;
        default:
            SkyIntensity = 1.0f;
            break;
    }
    
    SkyComponent->SetIntensity(SkyIntensity);
    SkyComponent->RecaptureSky();
}
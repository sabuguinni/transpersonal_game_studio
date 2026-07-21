#include "Light_WeatherSystem.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ALight_WeatherSystem::ALight_WeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings
    CurrentTime = 12.0f; // Start at midday
    TimeScale = 1.0f;
    WeatherTransitionSpeed = 1.0f;
    CurrentWeather = ELight_WeatherType::Clear;
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
}

void ALight_WeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherPresets();
    FindLightingActors();
    UpdateLighting();
}

void ALight_WeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time progression
    CurrentTime += DeltaTime * TimeScale / 3600.0f; // Convert seconds to hours
    if (CurrentTime >= 24.0f)
    {
        CurrentTime -= 24.0f;
    }
    
    // Update time of day enum
    CurrentTimeOfDay = GetTimeOfDayFromHour(CurrentTime);
    
    // Handle weather transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        if (TransitionProgress >= 1.0f)
        {
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
            CurrentSettings = TargetSettings;
        }
        else
        {
            // Interpolate between current and target settings
            CurrentSettings.SunIntensity = FMath::Lerp(CurrentSettings.SunIntensity, TargetSettings.SunIntensity, TransitionProgress);
            CurrentSettings.SunColor = FMath::Lerp(CurrentSettings.SunColor, TargetSettings.SunColor, TransitionProgress);
            CurrentSettings.FogDensity = FMath::Lerp(CurrentSettings.FogDensity, TargetSettings.FogDensity, TransitionProgress);
            CurrentSettings.FogColor = FMath::Lerp(CurrentSettings.FogColor, TargetSettings.FogColor, TransitionProgress);
            CurrentSettings.CloudCoverage = FMath::Lerp(CurrentSettings.CloudCoverage, TargetSettings.CloudCoverage, TransitionProgress);
        }
    }
    
    UpdateLighting();
}

void ALight_WeatherSystem::SetWeather(ELight_WeatherType NewWeather)
{
    if (WeatherPresets.Contains(NewWeather))
    {
        CurrentWeather = NewWeather;
        CurrentSettings = WeatherPresets[NewWeather];
        UpdateLighting();
    }
}

void ALight_WeatherSystem::SetTimeOfDay(float Hour)
{
    CurrentTime = FMath::Clamp(Hour, 0.0f, 24.0f);
    CurrentTimeOfDay = GetTimeOfDayFromHour(CurrentTime);
    UpdateLighting();
}

void ALight_WeatherSystem::TransitionToWeather(ELight_WeatherType TargetWeather, float Duration)
{
    if (WeatherPresets.Contains(TargetWeather))
    {
        TargetSettings = WeatherPresets[TargetWeather];
        TransitionDuration = Duration;
        TransitionProgress = 0.0f;
        bIsTransitioning = true;
        CurrentWeather = TargetWeather;
    }
}

void ALight_WeatherSystem::UpdateLighting()
{
    UpdateSunPosition();
    UpdateAtmosphericFog();
    UpdateSkyLight();
}

ELight_TimeOfDay ALight_WeatherSystem::GetTimeOfDayFromHour(float Hour) const
{
    if (Hour >= 5.0f && Hour < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (Hour >= 7.0f && Hour < 10.0f)
        return ELight_TimeOfDay::Morning;
    else if (Hour >= 10.0f && Hour < 14.0f)
        return ELight_TimeOfDay::Midday;
    else if (Hour >= 14.0f && Hour < 17.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (Hour >= 17.0f && Hour < 19.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_WeatherSystem::InitializeWeatherPresets()
{
    // Clear weather
    FLight_WeatherSettings ClearWeather;
    ClearWeather.SunIntensity = 5.0f;
    ClearWeather.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    ClearWeather.FogDensity = 0.02f;
    ClearWeather.FogColor = FLinearColor(0.7f, 0.63f, 0.47f, 1.0f);
    ClearWeather.CloudCoverage = 0.2f;
    ClearWeather.WindStrength = 1.0f;
    WeatherPresets.Add(ELight_WeatherType::Clear, ClearWeather);
    
    // Overcast weather
    FLight_WeatherSettings OvercastWeather;
    OvercastWeather.SunIntensity = 2.0f;
    OvercastWeather.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastWeather.FogDensity = 0.05f;
    OvercastWeather.FogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    OvercastWeather.CloudCoverage = 0.8f;
    OvercastWeather.WindStrength = 1.5f;
    WeatherPresets.Add(ELight_WeatherType::Overcast, OvercastWeather);
    
    // Storm weather
    FLight_WeatherSettings StormWeather;
    StormWeather.SunIntensity = 0.5f;
    StormWeather.SunColor = FLinearColor(0.4f, 0.5f, 0.7f, 1.0f);
    StormWeather.FogDensity = 0.1f;
    StormWeather.FogColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
    StormWeather.CloudCoverage = 1.0f;
    StormWeather.WindStrength = 3.0f;
    WeatherPresets.Add(ELight_WeatherType::Storm, StormWeather);
    
    // Fog weather
    FLight_WeatherSettings FogWeather;
    FogWeather.SunIntensity = 1.0f;
    FogWeather.SunColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    FogWeather.FogDensity = 0.2f;
    FogWeather.FogColor = FLinearColor(0.8f, 0.8f, 0.75f, 1.0f);
    FogWeather.CloudCoverage = 0.6f;
    FogWeather.WindStrength = 0.5f;
    WeatherPresets.Add(ELight_WeatherType::Fog, FogWeather);
    
    // Dust storm weather
    FLight_WeatherSettings DustWeather;
    DustWeather.SunIntensity = 0.8f;
    DustWeather.SunColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    DustWeather.FogDensity = 0.15f;
    DustWeather.FogColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    DustWeather.CloudCoverage = 0.4f;
    DustWeather.WindStrength = 4.0f;
    WeatherPresets.Add(ELight_WeatherType::Dust, DustWeather);
    
    // Set initial settings
    CurrentSettings = WeatherPresets[CurrentWeather];
}

void ALight_WeatherSystem::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
}

void ALight_WeatherSystem::UpdateSunPosition()
{
    if (!SunLight) return;
    
    UDirectionalLightComponent* LightComp = SunLight->GetComponent();
    if (!LightComp) return;
    
    // Calculate sun angle based on time of day
    float SunAngle = (CurrentTime - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    // Clamp elevation to prevent sun going below horizon
    SunElevation = FMath::Clamp(SunElevation, -10.0f, 90.0f);
    
    // Set sun rotation
    FRotator SunRotation(SunElevation, SunAngle, 0.0f);
    SunLight->SetActorRotation(SunRotation);
    
    // Adjust intensity based on time of day
    float TimeBasedIntensity = 1.0f;
    if (CurrentTimeOfDay == ELight_TimeOfDay::Dawn || CurrentTimeOfDay == ELight_TimeOfDay::Dusk)
    {
        TimeBasedIntensity = 0.6f;
    }
    else if (CurrentTimeOfDay == ELight_TimeOfDay::Night)
    {
        TimeBasedIntensity = 0.1f;
    }
    
    LightComp->SetIntensity(CurrentSettings.SunIntensity * TimeBasedIntensity);
    LightComp->SetLightColor(CurrentSettings.SunColor);
}

void ALight_WeatherSystem::UpdateAtmosphericFog()
{
    if (!FogActor) return;
    
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    if (!FogComp) return;
    
    FogComp->SetFogDensity(CurrentSettings.FogDensity);
    FogComp->SetFogInscatteringColor(CurrentSettings.FogColor);
    FogComp->SetFogHeightFalloff(0.2f);
}

void ALight_WeatherSystem::UpdateSkyLight()
{
    if (!SkyLightActor) return;
    
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (!SkyComp) return;
    
    // Adjust sky light intensity based on weather
    float SkyIntensity = 1.0f;
    if (CurrentWeather == ELight_WeatherType::Storm)
    {
        SkyIntensity = 0.3f;
    }
    else if (CurrentWeather == ELight_WeatherType::Overcast)
    {
        SkyIntensity = 0.6f;
    }
    
    SkyComp->SetIntensity(SkyIntensity);
}
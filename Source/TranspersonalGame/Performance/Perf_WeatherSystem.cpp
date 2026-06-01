#include "Perf_WeatherSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UPerf_WeatherSystem::UPerf_WeatherSystem()
{
    InitializeWeatherPresets();
}

void UPerf_WeatherSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeWeatherPresets();
    CurrentWeatherSettings = ClearWeatherPreset;
    TargetWeatherSettings = ClearWeatherPreset;
    
    // Find lighting actors in the world
    FindLightingActors();
    
    // Set up weather update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(WeatherUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UPerf_WeatherSystem::UpdateWeatherEffects),
            0.1f, true);
    }
}

void UPerf_WeatherSystem::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(WeatherUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UPerf_WeatherSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UPerf_WeatherSystem::SetWeatherType(EPerf_WeatherType NewWeatherType, float TransitionDuration)
{
    if (NewWeatherType == CurrentWeatherSettings.WeatherType)
    {
        return;
    }

    TargetWeatherSettings = GetWeatherPreset(NewWeatherType);
    this->TransitionDuration = TransitionDuration;
    TransitionTimer = 0.0f;
    bIsTransitioning = true;
}

void UPerf_WeatherSystem::SetWeatherIntensity(float NewIntensity)
{
    CurrentWeatherSettings.Intensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    TargetWeatherSettings.Intensity = CurrentWeatherSettings.Intensity;
    ApplyWeatherSettings(CurrentWeatherSettings);
}

void UPerf_WeatherSystem::UpdateWeatherEffects()
{
    if (bIsTransitioning)
    {
        TickWeatherTransition(0.1f);
    }
    
    // Apply current weather settings to lighting actors
    ApplyWeatherSettings(CurrentWeatherSettings);
}

void UPerf_WeatherSystem::RandomizeWeather()
{
    // Choose random weather type
    int32 RandomWeatherIndex = FMath::RandRange(0, 4);
    EPerf_WeatherType RandomWeatherType = static_cast<EPerf_WeatherType>(RandomWeatherIndex);
    
    // Set random transition duration between 3-8 seconds
    float RandomTransitionDuration = FMath::RandRange(3.0f, 8.0f);
    
    SetWeatherType(RandomWeatherType, RandomTransitionDuration);
}

void UPerf_WeatherSystem::RefreshWeatherReferences()
{
    FindLightingActors();
    ApplyWeatherSettings(CurrentWeatherSettings);
}

void UPerf_WeatherSystem::InitializeWeatherPresets()
{
    // Clear weather preset
    ClearWeatherPreset.WeatherType = EPerf_WeatherType::Clear;
    ClearWeatherPreset.Intensity = 1.0f;
    ClearWeatherPreset.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    ClearWeatherPreset.SunIntensity = 3.0f;
    ClearWeatherPreset.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    ClearWeatherPreset.FogDensity = 0.005f;
    ClearWeatherPreset.SkyLightColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    ClearWeatherPreset.SkyLightIntensity = 1.0f;

    // Cloudy weather preset
    CloudyWeatherPreset.WeatherType = EPerf_WeatherType::Cloudy;
    CloudyWeatherPreset.Intensity = 0.7f;
    CloudyWeatherPreset.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    CloudyWeatherPreset.SunIntensity = 2.0f;
    CloudyWeatherPreset.FogColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    CloudyWeatherPreset.FogDensity = 0.015f;
    CloudyWeatherPreset.SkyLightColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
    CloudyWeatherPreset.SkyLightIntensity = 0.7f;

    // Rainy weather preset
    RainyWeatherPreset.WeatherType = EPerf_WeatherType::Rainy;
    RainyWeatherPreset.Intensity = 0.5f;
    RainyWeatherPreset.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    RainyWeatherPreset.SunIntensity = 1.5f;
    RainyWeatherPreset.FogColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    RainyWeatherPreset.FogDensity = 0.025f;
    RainyWeatherPreset.SkyLightColor = FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
    RainyWeatherPreset.SkyLightIntensity = 0.5f;

    // Stormy weather preset
    StormyWeatherPreset.WeatherType = EPerf_WeatherType::Stormy;
    StormyWeatherPreset.Intensity = 0.3f;
    StormyWeatherPreset.SunColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
    StormyWeatherPreset.SunIntensity = 1.0f;
    StormyWeatherPreset.FogColor = FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
    StormyWeatherPreset.FogDensity = 0.04f;
    StormyWeatherPreset.SkyLightColor = FLinearColor(0.1f, 0.1f, 0.2f, 1.0f);
    StormyWeatherPreset.SkyLightIntensity = 0.3f;

    // Foggy weather preset
    FoggyWeatherPreset.WeatherType = EPerf_WeatherType::Foggy;
    FoggyWeatherPreset.Intensity = 0.4f;
    FoggyWeatherPreset.SunColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    FoggyWeatherPreset.SunIntensity = 1.8f;
    FoggyWeatherPreset.FogColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    FoggyWeatherPreset.FogDensity = 0.08f;
    FoggyWeatherPreset.SkyLightColor = FLinearColor(0.5f, 0.5f, 0.6f, 1.0f);
    FoggyWeatherPreset.SkyLightIntensity = 0.6f;
}

void UPerf_WeatherSystem::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        if (DirectionalLights.Num() > 0)
        {
            DirectionalLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        }

        // Find sky light
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLight = Cast<ASkyLight>(SkyLights[0]);
        }

        // Find height fog
        TArray<AActor*> HeightFogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
        if (HeightFogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
        }
    }
}

void UPerf_WeatherSystem::ApplyWeatherSettings(const FPerf_WeatherSettings& Settings)
{
    // Apply directional light settings
    if (DirectionalLight.IsValid())
    {
        if (UDirectionalLightComponent* LightComp = DirectionalLight->GetLightComponent())
        {
            LightComp->SetLightColor(Settings.SunColor);
            LightComp->SetIntensity(Settings.SunIntensity * Settings.Intensity);
        }
    }

    // Apply sky light settings
    if (SkyLight.IsValid())
    {
        if (USkyLightComponent* SkyLightComp = SkyLight->GetLightComponent())
        {
            SkyLightComp->SetLightColor(Settings.SkyLightColor);
            SkyLightComp->SetIntensity(Settings.SkyLightIntensity * Settings.Intensity);
        }
    }

    // Apply height fog settings
    if (HeightFog.IsValid())
    {
        if (UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent())
        {
            FogComp->SetFogInscatteringColor(Settings.FogColor);
            FogComp->SetFogDensity(Settings.FogDensity * Settings.Intensity);
            FogComp->SetStartDistance(Settings.FogStartDistance);
        }
    }
}

FPerf_WeatherSettings UPerf_WeatherSystem::GetWeatherPreset(EPerf_WeatherType WeatherType) const
{
    switch (WeatherType)
    {
        case EPerf_WeatherType::Clear:
            return ClearWeatherPreset;
        case EPerf_WeatherType::Cloudy:
            return CloudyWeatherPreset;
        case EPerf_WeatherType::Rainy:
            return RainyWeatherPreset;
        case EPerf_WeatherType::Stormy:
            return StormyWeatherPreset;
        case EPerf_WeatherType::Foggy:
            return FoggyWeatherPreset;
        default:
            return ClearWeatherPreset;
    }
}

void UPerf_WeatherSystem::TickWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    // Interpolate weather settings
    CurrentWeatherSettings.Intensity = FMath::Lerp(CurrentWeatherSettings.Intensity, TargetWeatherSettings.Intensity, Alpha);
    CurrentWeatherSettings.SunColor = FMath::Lerp(CurrentWeatherSettings.SunColor, TargetWeatherSettings.SunColor, Alpha);
    CurrentWeatherSettings.SunIntensity = FMath::Lerp(CurrentWeatherSettings.SunIntensity, TargetWeatherSettings.SunIntensity, Alpha);
    CurrentWeatherSettings.FogColor = FMath::Lerp(CurrentWeatherSettings.FogColor, TargetWeatherSettings.FogColor, Alpha);
    CurrentWeatherSettings.FogDensity = FMath::Lerp(CurrentWeatherSettings.FogDensity, TargetWeatherSettings.FogDensity, Alpha);
    CurrentWeatherSettings.SkyLightColor = FMath::Lerp(CurrentWeatherSettings.SkyLightColor, TargetWeatherSettings.SkyLightColor, Alpha);
    CurrentWeatherSettings.SkyLightIntensity = FMath::Lerp(CurrentWeatherSettings.SkyLightIntensity, TargetWeatherSettings.SkyLightIntensity, Alpha);

    // Check if transition is complete
    if (Alpha >= 1.0f)
    {
        CurrentWeatherSettings = TargetWeatherSettings;
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
    }
}
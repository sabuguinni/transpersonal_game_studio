#include "World_DynamicWeatherController.h"
#include "Engine/World.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/WindDirectionalSource.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/WindDirectionalSourceComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_DynamicWeatherController::AWorld_DynamicWeatherController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default weather state
    CurrentWeatherState.CurrentWeather = EWorld_WeatherType::Clear;
    CurrentWeatherState.WeatherIntensity = 0.5f;
    CurrentWeatherState.TransitionSpeed = 1.0f;
    CurrentWeatherState.WindStrength = 0.3f;
    CurrentWeatherState.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    CurrentWeatherState.FogDensity = 0.02f;
    CurrentWeatherState.CloudCoverage = 0.4f;
    CurrentWeatherState.SunIntensity = 3.0f;

    // Initialize weather transition
    WeatherTransition.TargetWeather = EWorld_WeatherType::Clear;
    WeatherTransition.TransitionDuration = 30.0f;
    WeatherTransition.CurrentProgress = 0.0f;
    WeatherTransition.bIsTransitioning = false;

    // Initialize weather control
    bEnableAutomaticWeatherChanges = true;
    WeatherChangeInterval = 300.0f; // 5 minutes
    TimeSinceLastWeatherChange = 0.0f;
}

void AWorld_DynamicWeatherController::BeginPlay()
{
    Super::BeginPlay();

    // Initialize weather presets
    InitializeWeatherPresets();

    // Find environmental actors in the level
    FindEnvironmentalActors();

    // Apply initial weather state
    ApplyWeatherState(CurrentWeatherState);

    UE_LOG(LogTemp, Log, TEXT("Dynamic Weather Controller initialized"));
}

void AWorld_DynamicWeatherController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update weather transition if active
    if (WeatherTransition.bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }

    // Handle automatic weather changes
    if (bEnableAutomaticWeatherChanges)
    {
        TimeSinceLastWeatherChange += DeltaTime;
        if (TimeSinceLastWeatherChange >= WeatherChangeInterval)
        {
            TriggerAutomaticWeatherChange();
            TimeSinceLastWeatherChange = 0.0f;
        }
    }
}

void AWorld_DynamicWeatherController::SetWeather(EWorld_WeatherType NewWeather, float TransitionDuration)
{
    if (!IsValidWeatherType(NewWeather))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid weather type requested"));
        return;
    }

    if (CurrentWeatherState.CurrentWeather == NewWeather)
    {
        UE_LOG(LogTemp, Log, TEXT("Weather already set to requested type"));
        return;
    }

    EWorld_WeatherType OldWeather = CurrentWeatherState.CurrentWeather;

    // Start weather transition
    WeatherTransition.TargetWeather = NewWeather;
    WeatherTransition.TransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    WeatherTransition.CurrentProgress = 0.0f;
    WeatherTransition.bIsTransitioning = true;

    // Trigger transition started event
    OnWeatherTransitionStarted(NewWeather);

    LogWeatherChange(OldWeather, NewWeather);
}

void AWorld_DynamicWeatherController::SetWeatherImmediate(EWorld_WeatherType NewWeather)
{
    if (!IsValidWeatherType(NewWeather))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid weather type for immediate change"));
        return;
    }

    EWorld_WeatherType OldWeather = CurrentWeatherState.CurrentWeather;

    // Stop any ongoing transition
    WeatherTransition.bIsTransitioning = false;

    // Apply new weather immediately
    if (WeatherPresets.Contains(NewWeather))
    {
        CurrentWeatherState = WeatherPresets[NewWeather];
        CurrentWeatherState.CurrentWeather = NewWeather;
        ApplyWeatherState(CurrentWeatherState);

        // Trigger weather changed event
        OnWeatherChanged(OldWeather, NewWeather);
        OnWeatherTransitionCompleted(NewWeather);

        LogWeatherChange(OldWeather, NewWeather);
    }
}

EWorld_WeatherType AWorld_DynamicWeatherController::GetCurrentWeather() const
{
    return CurrentWeatherState.CurrentWeather;
}

float AWorld_DynamicWeatherController::GetWeatherIntensity() const
{
    return CurrentWeatherState.WeatherIntensity;
}

bool AWorld_DynamicWeatherController::IsWeatherTransitioning() const
{
    return WeatherTransition.bIsTransitioning;
}

void AWorld_DynamicWeatherController::UpdateFogSettings()
{
    if (FogActor.IsValid())
    {
        AExponentialHeightFog* Fog = FogActor.Get();
        if (Fog && Fog->GetComponent())
        {
            UExponentialHeightFogComponent* FogComponent = Fog->GetComponent();
            FogComponent->SetFogDensity(CurrentWeatherState.FogDensity);
            FogComponent->SetFogInscatteringColor(CurrentWeatherState.FogColor);
            FogComponent->SetFogHeightFalloff(0.2f);
        }
    }
}

void AWorld_DynamicWeatherController::UpdateWindSettings()
{
    if (WindActor.IsValid())
    {
        AWindDirectionalSource* Wind = WindActor.Get();
        if (Wind && Wind->GetComponent())
        {
            UWindDirectionalSourceComponent* WindComponent = Wind->GetComponent();
            WindComponent->SetStrength(CurrentWeatherState.WindStrength);
            WindComponent->SetSpeed(CurrentWeatherState.WindStrength * 0.6f);
        }
    }
}

void AWorld_DynamicWeatherController::UpdateLightingSettings()
{
    if (SunActor.IsValid())
    {
        ADirectionalLight* Sun = SunActor.Get();
        if (Sun && Sun->GetLightComponent())
        {
            UDirectionalLightComponent* LightComponent = Sun->GetLightComponent();
            LightComponent->SetIntensity(CurrentWeatherState.SunIntensity);
            
            // Adjust light color based on weather
            FLinearColor LightColor = FLinearColor::White;
            switch (CurrentWeatherState.CurrentWeather)
            {
                case EWorld_WeatherType::Stormy:
                    LightColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
                    break;
                case EWorld_WeatherType::Foggy:
                    LightColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
                    break;
                case EWorld_WeatherType::Overcast:
                    LightColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
                    break;
                default:
                    LightColor = FLinearColor::White;
                    break;
            }
            LightComponent->SetLightColor(LightColor);
        }
    }
}

void AWorld_DynamicWeatherController::UpdateCloudSettings()
{
    // Cloud settings would be applied to volumetric clouds or sky atmosphere
    // This is a placeholder for future cloud system integration
    UE_LOG(LogTemp, Log, TEXT("Cloud coverage set to: %f"), CurrentWeatherState.CloudCoverage);
}

void AWorld_DynamicWeatherController::InitializeWeatherPresets()
{
    // Clear Weather
    FWorld_WeatherState ClearWeather;
    ClearWeather.CurrentWeather = EWorld_WeatherType::Clear;
    ClearWeather.WeatherIntensity = 0.3f;
    ClearWeather.WindStrength = 0.2f;
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.CloudCoverage = 0.2f;
    ClearWeather.SunIntensity = 4.0f;
    ClearWeather.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    WeatherPresets.Add(EWorld_WeatherType::Clear, ClearWeather);

    // Overcast Weather
    FWorld_WeatherState OvercastWeather;
    OvercastWeather.CurrentWeather = EWorld_WeatherType::Overcast;
    OvercastWeather.WeatherIntensity = 0.6f;
    OvercastWeather.WindStrength = 0.4f;
    OvercastWeather.FogDensity = 0.03f;
    OvercastWeather.CloudCoverage = 0.8f;
    OvercastWeather.SunIntensity = 2.0f;
    OvercastWeather.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    WeatherPresets.Add(EWorld_WeatherType::Overcast, OvercastWeather);

    // Stormy Weather
    FWorld_WeatherState StormyWeather;
    StormyWeather.CurrentWeather = EWorld_WeatherType::Stormy;
    StormyWeather.WeatherIntensity = 0.9f;
    StormyWeather.WindStrength = 0.8f;
    StormyWeather.FogDensity = 0.05f;
    StormyWeather.CloudCoverage = 0.95f;
    StormyWeather.SunIntensity = 1.0f;
    StormyWeather.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
    WeatherPresets.Add(EWorld_WeatherType::Stormy, StormyWeather);

    // Foggy Weather
    FWorld_WeatherState FoggyWeather;
    FoggyWeather.CurrentWeather = EWorld_WeatherType::Foggy;
    FoggyWeather.WeatherIntensity = 0.7f;
    FoggyWeather.WindStrength = 0.1f;
    FoggyWeather.FogDensity = 0.08f;
    FoggyWeather.CloudCoverage = 0.6f;
    FoggyWeather.SunIntensity = 1.5f;
    FoggyWeather.FogColor = FLinearColor(0.7f, 0.8f, 0.8f, 1.0f);
    WeatherPresets.Add(EWorld_WeatherType::Foggy, FoggyWeather);

    UE_LOG(LogTemp, Log, TEXT("Weather presets initialized"));
}

void AWorld_DynamicWeatherController::FindEnvironmentalActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find fog actor
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FogActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Found fog actor: %s"), *FogActor->GetName());
    }

    // Find wind actor
    TArray<AActor*> WindActors;
    UGameplayStatics::GetAllActorsOfClass(World, AWindDirectionalSource::StaticClass(), WindActors);
    if (WindActors.Num() > 0)
    {
        WindActor = Cast<AWindDirectionalSource>(WindActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Found wind actor: %s"), *WindActor->GetName());
    }

    // Find directional light (sun)
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    if (LightActors.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(LightActors[0]);
        UE_LOG(LogTemp, Log, TEXT("Found sun actor: %s"), *SunActor->GetName());
    }
}

void AWorld_DynamicWeatherController::UpdateWeatherTransition(float DeltaTime)
{
    if (!WeatherTransition.bIsTransitioning)
    {
        return;
    }

    // Update transition progress
    WeatherTransition.CurrentProgress += DeltaTime / WeatherTransition.TransitionDuration;

    if (WeatherTransition.CurrentProgress >= 1.0f)
    {
        // Transition complete
        WeatherTransition.CurrentProgress = 1.0f;
        WeatherTransition.bIsTransitioning = false;

        EWorld_WeatherType OldWeather = CurrentWeatherState.CurrentWeather;

        // Apply target weather state
        if (WeatherPresets.Contains(WeatherTransition.TargetWeather))
        {
            CurrentWeatherState = WeatherPresets[WeatherTransition.TargetWeather];
            CurrentWeatherState.CurrentWeather = WeatherTransition.TargetWeather;
        }

        ApplyWeatherState(CurrentWeatherState);

        // Trigger events
        OnWeatherChanged(OldWeather, WeatherTransition.TargetWeather);
        OnWeatherTransitionCompleted(WeatherTransition.TargetWeather);
    }
    else
    {
        // Interpolate between current and target weather
        if (WeatherPresets.Contains(WeatherTransition.TargetWeather))
        {
            FWorld_WeatherState TargetState = WeatherPresets[WeatherTransition.TargetWeather];
            FWorld_WeatherState InterpolatedState = InterpolateWeatherStates(CurrentWeatherState, TargetState, WeatherTransition.CurrentProgress);
            ApplyWeatherState(InterpolatedState);
        }
    }
}

void AWorld_DynamicWeatherController::ApplyWeatherState(const FWorld_WeatherState& WeatherState)
{
    CurrentWeatherState = WeatherState;

    // Update all environmental systems
    UpdateFogSettings();
    UpdateWindSettings();
    UpdateLightingSettings();
    UpdateCloudSettings();
}

FWorld_WeatherState AWorld_DynamicWeatherController::InterpolateWeatherStates(const FWorld_WeatherState& From, const FWorld_WeatherState& To, float Alpha)
{
    FWorld_WeatherState Result = From;

    Result.WeatherIntensity = FMath::Lerp(From.WeatherIntensity, To.WeatherIntensity, Alpha);
    Result.WindStrength = FMath::Lerp(From.WindStrength, To.WindStrength, Alpha);
    Result.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    Result.CloudCoverage = FMath::Lerp(From.CloudCoverage, To.CloudCoverage, Alpha);
    Result.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    Result.FogColor = FMath::Lerp(From.FogColor, To.FogColor, Alpha);

    return Result;
}

EWorld_WeatherType AWorld_DynamicWeatherController::GetRandomWeatherType() const
{
    TArray<EWorld_WeatherType> WeatherTypes = {
        EWorld_WeatherType::Clear,
        EWorld_WeatherType::Overcast,
        EWorld_WeatherType::Stormy,
        EWorld_WeatherType::Foggy
    };

    int32 RandomIndex = FMath::RandRange(0, WeatherTypes.Num() - 1);
    return WeatherTypes[RandomIndex];
}

void AWorld_DynamicWeatherController::TriggerAutomaticWeatherChange()
{
    EWorld_WeatherType NewWeather = GetRandomWeatherType();
    
    // Avoid changing to the same weather
    if (NewWeather == CurrentWeatherState.CurrentWeather)
    {
        // Try to get a different weather type
        for (int32 i = 0; i < 5; i++)
        {
            NewWeather = GetRandomWeatherType();
            if (NewWeather != CurrentWeatherState.CurrentWeather)
            {
                break;
            }
        }
    }

    float TransitionDuration = FMath::RandRange(20.0f, 60.0f);
    SetWeather(NewWeather, TransitionDuration);

    UE_LOG(LogTemp, Log, TEXT("Automatic weather change triggered: %d"), (int32)NewWeather);
}

float AWorld_DynamicWeatherController::GetWeatherTransitionAlpha() const
{
    return WeatherTransition.bIsTransitioning ? WeatherTransition.CurrentProgress : 1.0f;
}

bool AWorld_DynamicWeatherController::IsValidWeatherType(EWorld_WeatherType WeatherType) const
{
    return WeatherPresets.Contains(WeatherType);
}

void AWorld_DynamicWeatherController::LogWeatherChange(EWorld_WeatherType OldWeather, EWorld_WeatherType NewWeather) const
{
    UE_LOG(LogTemp, Log, TEXT("Weather changing from %d to %d"), (int32)OldWeather, (int32)NewWeather);
}
#include "Light_DynamicWeatherSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ALight_DynamicWeatherSystem::ALight_DynamicWeatherSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    CurrentWeatherState = ELight_WeatherState::Clear;
    WeatherTransitionDuration = 30.0f;
    WeatherChangeProbability = 0.1f;
    bAutoWeatherChanges = true;
    WeatherUpdateInterval = 60.0f;
    CurrentTransitionTime = 0.0f;
    bIsTransitioning = false;
    TargetWeatherState = ELight_WeatherState::Clear;
}

void ALight_DynamicWeatherSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherPresets();
    FindLightingActors();
    
    if (bAutoWeatherChanges)
    {
        GetWorldTimerManager().SetTimer(WeatherUpdateTimer, this, 
            &ALight_DynamicWeatherSystem::UpdateWeatherSystem, WeatherUpdateInterval, true);
    }
    
    // Apply initial weather state
    if (WeatherPresets.Contains(CurrentWeatherState))
    {
        ApplyWeatherSettings(WeatherPresets[CurrentWeatherState]);
    }
}

void ALight_DynamicWeatherSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsTransitioning)
    {
        ProcessWeatherTransition(DeltaTime);
    }
}

void ALight_DynamicWeatherSystem::SetWeatherState(ELight_WeatherState NewWeatherState)
{
    if (CurrentWeatherState == NewWeatherState)
    {
        return;
    }
    
    ELight_WeatherState OldState = CurrentWeatherState;
    CurrentWeatherState = NewWeatherState;
    bIsTransitioning = false;
    CurrentTransitionTime = 0.0f;
    
    if (WeatherPresets.Contains(CurrentWeatherState))
    {
        ApplyWeatherSettings(WeatherPresets[CurrentWeatherState]);
    }
    
    OnWeatherStateChanged(OldState, NewWeatherState);
}

void ALight_DynamicWeatherSystem::TransitionToWeatherState(ELight_WeatherState NewWeatherState, float TransitionTime)
{
    if (CurrentWeatherState == NewWeatherState || bIsTransitioning)
    {
        return;
    }
    
    TargetWeatherState = NewWeatherState;
    WeatherTransitionDuration = FMath::Max(TransitionTime, 1.0f);
    CurrentTransitionTime = 0.0f;
    bIsTransitioning = true;
    
    // Store current settings as start point
    if (WeatherPresets.Contains(CurrentWeatherState))
    {
        StartWeatherSettings = WeatherPresets[CurrentWeatherState];
    }
    
    if (WeatherPresets.Contains(TargetWeatherState))
    {
        TargetWeatherSettings = WeatherPresets[TargetWeatherState];
    }
    
    OnWeatherTransitionStarted(CurrentWeatherState, TargetWeatherState);
}

void ALight_DynamicWeatherSystem::ForceWeatherUpdate()
{
    UpdateWeatherSystem();
}

void ALight_DynamicWeatherSystem::UpdateWeatherSystem()
{
    if (!bAutoWeatherChanges || bIsTransitioning)
    {
        return;
    }
    
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    if (RandomValue < WeatherChangeProbability)
    {
        ELight_WeatherState NewWeatherState = SelectRandomWeatherState();
        if (NewWeatherState != CurrentWeatherState)
        {
            TransitionToWeatherState(NewWeatherState, WeatherTransitionDuration);
        }
    }
}

void ALight_DynamicWeatherSystem::ProcessWeatherTransition(float DeltaTime)
{
    CurrentTransitionTime += DeltaTime;
    float Alpha = FMath::Clamp(CurrentTransitionTime / WeatherTransitionDuration, 0.0f, 1.0f);
    
    // Smooth transition curve
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    FLight_WeatherSettings InterpolatedSettings = LerpWeatherSettings(StartWeatherSettings, TargetWeatherSettings, Alpha);
    ApplyWeatherSettings(InterpolatedSettings);
    
    if (CurrentTransitionTime >= WeatherTransitionDuration)
    {
        bIsTransitioning = false;
        CurrentTransitionTime = 0.0f;
        
        ELight_WeatherState OldState = CurrentWeatherState;
        CurrentWeatherState = TargetWeatherState;
        
        OnWeatherStateChanged(OldState, CurrentWeatherState);
        OnWeatherTransitionCompleted(CurrentWeatherState);
    }
}

void ALight_DynamicWeatherSystem::ApplyWeatherSettings(const FLight_WeatherSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
    }
    
    // Apply fog settings
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = AtmosphericFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
    
    // Apply sky atmosphere settings
    if (SkyAtmosphere && SkyAtmosphere->GetAtmosphereComponent())
    {
        USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetAtmosphereComponent();
        // Apply atmospheric perspective and other sky settings
    }
}

void ALight_DynamicWeatherSystem::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find directional light (sun)
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find atmospheric fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    // Find sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    // Find post process volume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ALight_DynamicWeatherSystem::InitializeWeatherPresets()
{
    // Clear weather
    FLight_WeatherSettings ClearWeather;
    ClearWeather.SunIntensity = 5.0f;
    ClearWeather.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    ClearWeather.FogDensity = 0.01f;
    ClearWeather.FogColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    ClearWeather.CloudCoverage = 0.2f;
    ClearWeather.AtmosphericPerspective = 1.0f;
    WeatherPresets.Add(ELight_WeatherState::Clear, ClearWeather);
    
    // Overcast weather
    FLight_WeatherSettings OvercastWeather;
    OvercastWeather.SunIntensity = 2.5f;
    OvercastWeather.SunColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
    OvercastWeather.FogDensity = 0.03f;
    OvercastWeather.FogColor = FLinearColor(0.7f, 0.75f, 0.8f, 1.0f);
    OvercastWeather.CloudCoverage = 0.8f;
    OvercastWeather.AtmosphericPerspective = 1.5f;
    WeatherPresets.Add(ELight_WeatherState::Overcast, OvercastWeather);
    
    // Storm weather
    FLight_WeatherSettings StormWeather;
    StormWeather.SunIntensity = 1.0f;
    StormWeather.SunColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);
    StormWeather.FogDensity = 0.05f;
    StormWeather.FogColor = FLinearColor(0.4f, 0.45f, 0.5f, 1.0f);
    StormWeather.CloudCoverage = 1.0f;
    StormWeather.AtmosphericPerspective = 2.0f;
    WeatherPresets.Add(ELight_WeatherState::Storm, StormWeather);
    
    // Fog weather
    FLight_WeatherSettings FogWeather;
    FogWeather.SunIntensity = 3.0f;
    FogWeather.SunColor = FLinearColor(0.9f, 0.9f, 0.95f, 1.0f);
    FogWeather.FogDensity = 0.08f;
    FogWeather.FogColor = FLinearColor(0.85f, 0.85f, 0.9f, 1.0f);
    FogWeather.CloudCoverage = 0.6f;
    FogWeather.AtmosphericPerspective = 3.0f;
    WeatherPresets.Add(ELight_WeatherState::Fog, FogWeather);
    
    // Rain weather
    FLight_WeatherSettings RainWeather;
    RainWeather.SunIntensity = 1.5f;
    RainWeather.SunColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    RainWeather.FogDensity = 0.04f;
    RainWeather.FogColor = FLinearColor(0.6f, 0.65f, 0.7f, 1.0f);
    RainWeather.CloudCoverage = 0.9f;
    RainWeather.AtmosphericPerspective = 1.8f;
    WeatherPresets.Add(ELight_WeatherState::Rain, RainWeather);
}

ELight_WeatherState ALight_DynamicWeatherSystem::SelectRandomWeatherState()
{
    TArray<ELight_WeatherState> WeatherStates = {
        ELight_WeatherState::Clear,
        ELight_WeatherState::Overcast,
        ELight_WeatherState::Storm,
        ELight_WeatherState::Fog,
        ELight_WeatherState::Rain
    };
    
    // Remove current state from selection
    WeatherStates.Remove(CurrentWeatherState);
    
    if (WeatherStates.Num() > 0)
    {
        int32 RandomIndex = FMath::RandRange(0, WeatherStates.Num() - 1);
        return WeatherStates[RandomIndex];
    }
    
    return CurrentWeatherState;
}

FLight_WeatherSettings ALight_DynamicWeatherSystem::LerpWeatherSettings(const FLight_WeatherSettings& A, const FLight_WeatherSettings& B, float Alpha)
{
    FLight_WeatherSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.AtmosphericPerspective = FMath::Lerp(A.AtmosphericPerspective, B.AtmosphericPerspective, Alpha);
    
    return Result;
}
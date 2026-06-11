#include "CretaceousLightingManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ACretaceousLightingManager::ACretaceousLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    DayDurationMinutes = 24.0f;
    CurrentTimeOfDay = 12.0f;
    bEnableDayNightCycle = true;
    CurrentTimePhase = ELight_TimeOfDay::Noon;
    CurrentWeatherState = ELight_WeatherState::Clear;
    bEnableWeatherTransitions = true;
    WeatherTransitionDurationMinutes = 5.0f;

    // Initialize atmospheric settings
    CurrentAtmosphericSettings = FLight_AtmosphericSettings();
}

void ACretaceousLightingManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize the lighting system
    InitializeLightingSystem();
    
    // Load presets
    LoadLightingPresets();
    
    // Apply initial settings
    UpdateAtmosphericLighting();
}

void ACretaceousLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }

    if (bIsTransitioning)
    {
        UpdateWeatherTransition(DeltaTime);
    }
}

void ACretaceousLightingManager::InitializeLightingSystem()
{
    // Find existing lighting actors in the world
    FindLightingActors();
    
    // Create missing lighting actors if needed
    CreateMissingLightingActors();
    
    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Lighting system initialized"));
}

void ACretaceousLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Found DirectionalLight"));
    }

    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Found SkyAtmosphere"));
    }

    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Found ExponentialHeightFog"));
    }
}

void ACretaceousLightingManager::CreateMissingLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create DirectionalLight if missing
    if (!SunLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("CretaceousSunLight");
        SunLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector::ZeroVector, FRotator(-45.0f, 0.0f, 0.0f), SpawnParams);
        if (SunLight)
        {
            UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Created DirectionalLight"));
        }
    }

    // Create SkyAtmosphere if missing
    if (!SkyAtmosphereActor)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("CretaceousSkyAtmosphere");
        SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
        if (SkyAtmosphereActor)
        {
            UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Created SkyAtmosphere"));
        }
    }

    // Create ExponentialHeightFog if missing
    if (!FogActor)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("CretaceousFog");
        FogActor = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector(0.0f, 0.0f, 100.0f), FRotator::ZeroRotator, SpawnParams);
        if (FogActor)
        {
            UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Created ExponentialHeightFog"));
        }
    }
}

void ACretaceousLightingManager::LoadLightingPresets()
{
    // Initialize Time of Day presets
    TimeOfDayPresets.Empty();

    // Dawn (6:00)
    FLight_AtmosphericSettings DawnSettings;
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.Temperature = 3500.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogHeightFalloff = 0.15f;
    DawnSettings.FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dawn, DawnSettings);

    // Morning (9:00)
    FLight_AtmosphericSettings MorningSettings;
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    MorningSettings.Temperature = 4500.0f;
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogHeightFalloff = 0.2f;
    MorningSettings.FogInscatteringColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Morning, MorningSettings);

    // Noon (12:00)
    FLight_AtmosphericSettings NoonSettings;
    NoonSettings.SunIntensity = 6.0f;
    NoonSettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
    NoonSettings.Temperature = 5500.0f;
    NoonSettings.FogDensity = 0.015f;
    NoonSettings.FogHeightFalloff = 0.25f;
    NoonSettings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Noon, NoonSettings);

    // Afternoon (15:00)
    FLight_AtmosphericSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 5.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.92f, 0.8f, 1.0f);
    AfternoonSettings.Temperature = 5000.0f;
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogHeightFalloff = 0.2f;
    AfternoonSettings.FogInscatteringColor = FLinearColor(0.8f, 0.85f, 1.0f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk (18:00)
    FLight_AtmosphericSettings DuskSettings;
    DuskSettings.SunIntensity = 3.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.Temperature = 3000.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogHeightFalloff = 0.15f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Dusk, DuskSettings);

    // Night (21:00)
    FLight_AtmosphericSettings NightSettings;
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.6f, 0.7f, 1.0f, 1.0f);
    NightSettings.Temperature = 4000.0f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.FogHeightFalloff = 0.1f;
    NightSettings.FogInscatteringColor = FLinearColor(0.4f, 0.5f, 0.8f, 1.0f);
    TimeOfDayPresets.Add(ELight_TimeOfDay::Night, NightSettings);

    // Initialize Weather presets
    WeatherPresets.Empty();

    // Clear Weather
    FLight_AtmosphericSettings ClearSettings = CurrentAtmosphericSettings;
    WeatherPresets.Add(ELight_WeatherState::Clear, ClearSettings);

    // Cloudy Weather
    FLight_AtmosphericSettings CloudySettings = CurrentAtmosphericSettings;
    CloudySettings.SunIntensity *= 0.7f;
    CloudySettings.FogDensity *= 1.5f;
    WeatherPresets.Add(ELight_WeatherState::Cloudy, CloudySettings);

    // Overcast Weather
    FLight_AtmosphericSettings OvercastSettings = CurrentAtmosphericSettings;
    OvercastSettings.SunIntensity *= 0.4f;
    OvercastSettings.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
    OvercastSettings.FogDensity *= 2.0f;
    WeatherPresets.Add(ELight_WeatherState::Overcast, OvercastSettings);

    // Foggy Weather
    FLight_AtmosphericSettings FoggySettings = CurrentAtmosphericSettings;
    FoggySettings.SunIntensity *= 0.3f;
    FoggySettings.FogDensity *= 5.0f;
    FoggySettings.FogHeightFalloff *= 0.5f;
    WeatherPresets.Add(ELight_WeatherState::Foggy, FoggySettings);

    // Storm Weather
    FLight_AtmosphericSettings StormSettings = CurrentAtmosphericSettings;
    StormSettings.SunIntensity *= 0.2f;
    StormSettings.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
    StormSettings.FogDensity *= 3.0f;
    WeatherPresets.Add(ELight_WeatherState::Storm, StormSettings);

    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Loaded lighting presets"));
}

void ACretaceousLightingManager::UpdateAtmosphericLighting()
{
    ApplyAtmosphericSettings(CurrentAtmosphericSettings);
}

void ACretaceousLightingManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    CurrentAtmosphericSettings = Settings;

    // Apply sun light settings
    ApplySunLightSettings(Settings);

    // Apply fog settings
    ApplyFogSettings(Settings);

    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Applied atmospheric settings"));
}

void ACretaceousLightingManager::ApplySunLightSettings(const FLight_AtmosphericSettings& Settings)
{
    if (!SunLight)
    {
        return;
    }

    UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
    if (!LightComponent)
    {
        return;
    }

    LightComponent->SetIntensity(Settings.SunIntensity);
    LightComponent->SetLightColor(Settings.SunColor);
    LightComponent->SetTemperature(Settings.Temperature);
    LightComponent->SetCastShadows(true);
}

void ACretaceousLightingManager::ApplyFogSettings(const FLight_AtmosphericSettings& Settings)
{
    if (!FogActor)
    {
        return;
    }

    UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
    if (!FogComponent)
    {
        return;
    }

    FogComponent->SetFogDensity(Settings.FogDensity);
    FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
    FogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
}

void ACretaceousLightingManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    CalculateTimePhase();
    
    // Apply settings for current time phase
    if (TimeOfDayPresets.Contains(CurrentTimePhase))
    {
        ApplyAtmosphericSettings(TimeOfDayPresets[CurrentTimePhase]);
    }
}

void ACretaceousLightingManager::SetTimePhase(ELight_TimeOfDay NewPhase)
{
    CurrentTimePhase = NewPhase;
    
    if (TimeOfDayPresets.Contains(NewPhase))
    {
        ApplyAtmosphericSettings(TimeOfDayPresets[NewPhase]);
    }
}

void ACretaceousLightingManager::SetWeatherState(ELight_WeatherState NewWeatherState)
{
    CurrentWeatherState = NewWeatherState;
    
    if (WeatherPresets.Contains(NewWeatherState))
    {
        ApplyAtmosphericSettings(WeatherPresets[NewWeatherState]);
    }
}

void ACretaceousLightingManager::TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration)
{
    if (CurrentWeatherState == TargetWeather)
    {
        return;
    }

    if (!WeatherPresets.Contains(TargetWeather))
    {
        return;
    }

    TransitionStartSettings = CurrentAtmosphericSettings;
    TransitionTargetSettings = WeatherPresets[TargetWeather];
    TransitionTimer = 0.0f;
    TransitionDuration = FMath::Max(TransitionDuration, 0.1f);
    bIsTransitioning = true;
    TransitionProgress = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Starting weather transition to %d"), (int32)TargetWeather);
}

void ACretaceousLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    // Convert day duration from minutes to seconds
    float DayDurationSeconds = DayDurationMinutes * 60.0f;
    
    // Calculate time progression
    float TimeIncrement = (24.0f / DayDurationSeconds) * DeltaTime;
    CurrentTimeOfDay += TimeIncrement;
    
    // Wrap around 24 hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay -= 24.0f;
    }

    // Update time phase
    ELight_TimeOfDay PreviousPhase = CurrentTimePhase;
    CalculateTimePhase();

    // Apply new settings if phase changed
    if (PreviousPhase != CurrentTimePhase && TimeOfDayPresets.Contains(CurrentTimePhase))
    {
        ApplyAtmosphericSettings(TimeOfDayPresets[CurrentTimePhase]);
    }
}

void ACretaceousLightingManager::UpdateWeatherTransition(float DeltaTime)
{
    if (!bIsTransitioning)
    {
        return;
    }

    TransitionTimer += DeltaTime;
    TransitionProgress = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);

    // Interpolate between start and target settings
    InterpolateAtmosphericSettings(TransitionStartSettings, TransitionTargetSettings, TransitionProgress);

    // Check if transition is complete
    if (TransitionProgress >= 1.0f)
    {
        bIsTransitioning = false;
        TransitionProgress = 1.0f;
        ApplyAtmosphericSettings(TransitionTargetSettings);
        UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Weather transition complete"));
    }
}

void ACretaceousLightingManager::CalculateTimePhase()
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 8.0f)
    {
        CurrentTimePhase = ELight_TimeOfDay::Dawn;
    }
    else if (CurrentTimeOfDay >= 8.0f && CurrentTimeOfDay < 11.0f)
    {
        CurrentTimePhase = ELight_TimeOfDay::Morning;
    }
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 14.0f)
    {
        CurrentTimePhase = ELight_TimeOfDay::Noon;
    }
    else if (CurrentTimeOfDay >= 14.0f && CurrentTimeOfDay < 17.0f)
    {
        CurrentTimePhase = ELight_TimeOfDay::Afternoon;
    }
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay < 20.0f)
    {
        CurrentTimePhase = ELight_TimeOfDay::Dusk;
    }
    else
    {
        CurrentTimePhase = ELight_TimeOfDay::Night;
    }
}

void ACretaceousLightingManager::InterpolateAtmosphericSettings(const FLight_AtmosphericSettings& From, const FLight_AtmosphericSettings& To, float Alpha)
{
    FLight_AtmosphericSettings InterpolatedSettings;
    
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.Temperature = FMath::Lerp(From.Temperature, To.Temperature, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.FogHeightFalloff = FMath::Lerp(From.FogHeightFalloff, To.FogHeightFalloff, Alpha);
    InterpolatedSettings.FogInscatteringColor = FMath::Lerp(From.FogInscatteringColor, To.FogInscatteringColor, Alpha);

    ApplyAtmosphericSettings(InterpolatedSettings);
}

void ACretaceousLightingManager::SaveCurrentLightingState()
{
    // This would save current lighting state to a file or game save
    // Implementation depends on the game's save system
    UE_LOG(LogTemp, Log, TEXT("CretaceousLightingManager: Lighting state saved"));
}
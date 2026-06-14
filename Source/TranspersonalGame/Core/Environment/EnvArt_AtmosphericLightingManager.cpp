#include "EnvArt_AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

AEnvArt_AtmosphericLightingManager::AEnvArt_AtmosphericLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default lighting presets
    InitializeDefaultLightingPresets();
}

void AEnvArt_AtmosphericLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find and cache lighting actors in the world
    FindAndCacheLightingActors();
    
    // Apply initial lighting settings
    UpdateLightingForCurrentTime();
}

void AEnvArt_AtmosphericLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Handle time progression
    if (bAutoProgressTime && DayDurationMinutes > 0.0f)
    {
        float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
        CurrentTimeNormalized += TimeIncrement;
        
        // Wrap around at end of day
        if (CurrentTimeNormalized >= 1.0f)
        {
            CurrentTimeNormalized -= 1.0f;
        }
        
        UpdateTimeOfDayFromNormalizedTime();
    }
    
    // Handle lighting transitions
    if (bIsTransitioning)
    {
        TransitionProgress += DeltaTime / TransitionDuration;
        
        if (TransitionProgress >= 1.0f)
        {
            // Transition complete
            TransitionProgress = 1.0f;
            bIsTransitioning = false;
        }
        
        // Interpolate between start and target settings
        FEnvArt_LightingSettings CurrentSettings = InterpolateLightingSettings(
            TransitionStartSettings, 
            TransitionTargetSettings, 
            TransitionProgress
        );
        
        ApplyLightingSettings(CurrentSettings);
    }
    else
    {
        // Normal lighting update based on current time
        UpdateLightingForCurrentTime();
    }
}

void AEnvArt_AtmosphericLightingManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    // Update normalized time to match
    switch (NewTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            CurrentTimeNormalized = 0.2f;
            break;
        case EEnvArt_TimeOfDay::Morning:
            CurrentTimeNormalized = 0.3f;
            break;
        case EEnvArt_TimeOfDay::Midday:
            CurrentTimeNormalized = 0.5f;
            break;
        case EEnvArt_TimeOfDay::Afternoon:
            CurrentTimeNormalized = 0.7f;
            break;
        case EEnvArt_TimeOfDay::Dusk:
            CurrentTimeNormalized = 0.8f;
            break;
        case EEnvArt_TimeOfDay::Night:
            CurrentTimeNormalized = 0.0f;
            break;
    }
    
    UpdateLightingForCurrentTime();
}

void AEnvArt_AtmosphericLightingManager::SetWeatherState(EEnvArt_WeatherState NewWeatherState)
{
    CurrentWeatherState = NewWeatherState;
    UpdateLightingForCurrentTime();
}

void AEnvArt_AtmosphericLightingManager::SetTimeNormalized(float NormalizedTime)
{
    CurrentTimeNormalized = FMath::Clamp(NormalizedTime, 0.0f, 1.0f);
    UpdateTimeOfDayFromNormalizedTime();
    UpdateLightingForCurrentTime();
}

void AEnvArt_AtmosphericLightingManager::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    // Apply sun light settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        SunComponent->SetIntensity(Settings.SunIntensity);
        SunComponent->SetLightColor(Settings.SunColor);
        SunLight->SetActorRotation(Settings.SunRotation);
    }
    
    // Apply sky light settings
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
        SkyComponent->SetIntensity(Settings.SkyIntensity);
        SkyComponent->SetLightColor(Settings.SkyColor);
    }
    
    // Apply fog settings
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
        FogComponent->SetFogDensity(Settings.FogDensity);
        FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
        FogComponent->SetVolumetricFog(Settings.bVolumetricFog);
        FogComponent->SetVolumetricFogScatteringDistribution(Settings.VolumetricFogScatteringDistribution);
    }
}

void AEnvArt_AtmosphericLightingManager::TransitionToTimeOfDay(EEnvArt_TimeOfDay TargetTimeOfDay, float TransitionDurationSeconds)
{
    if (bIsTransitioning)
    {
        return; // Already transitioning
    }
    
    TransitionStartSettings = GetCurrentLightingSettings();
    TransitionTargetSettings = GetLightingSettingsForTimeOfDay(TargetTimeOfDay);
    TransitionDuration = FMath::Max(TransitionDurationSeconds, 0.1f);
    TransitionProgress = 0.0f;
    bIsTransitioning = true;
    
    CurrentTimeOfDay = TargetTimeOfDay;
}

FEnvArt_LightingSettings AEnvArt_AtmosphericLightingManager::GetCurrentLightingSettings() const
{
    FEnvArt_LightingSettings CurrentSettings;
    
    // Read current settings from actors
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunLight->GetLightComponent();
        CurrentSettings.SunIntensity = SunComponent->Intensity;
        CurrentSettings.SunColor = SunComponent->GetLightColor();
        CurrentSettings.SunRotation = SunLight->GetActorRotation();
    }
    
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLight->GetLightComponent();
        CurrentSettings.SkyIntensity = SkyComponent->Intensity;
        CurrentSettings.SkyColor = SkyComponent->GetLightColor();
    }
    
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
        CurrentSettings.FogDensity = FogComponent->FogDensity;
        CurrentSettings.FogHeightFalloff = FogComponent->FogHeightFalloff;
        CurrentSettings.FogInscatteringColor = FogComponent->FogInscatteringColor;
        CurrentSettings.bVolumetricFog = FogComponent->VolumetricFog;
        CurrentSettings.VolumetricFogScatteringDistribution = FogComponent->VolumetricFogScatteringDistribution;
    }
    
    return CurrentSettings;
}

void AEnvArt_AtmosphericLightingManager::FindAndCacheLightingActors()
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

void AEnvArt_AtmosphericLightingManager::CreateDefaultLightingSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create directional light if none exists
    if (!SunLight)
    {
        SunLight = World->SpawnActor<ADirectionalLight>(
            ADirectionalLight::StaticClass(),
            FVector(0, 0, 1000),
            FRotator(-30, 45, 0)
        );
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("Sun_DirectionalLight"));
        }
    }
    
    // Create sky light if none exists
    if (!SkyLight)
    {
        SkyLight = World->SpawnActor<ASkyLight>(
            ASkyLight::StaticClass(),
            FVector(0, 0, 1500),
            FRotator::ZeroRotator
        );
        if (SkyLight)
        {
            SkyLight->SetActorLabel(TEXT("Sky_SkyLight"));
        }
    }
    
    // Create height fog if none exists
    if (!HeightFog)
    {
        HeightFog = World->SpawnActor<AExponentialHeightFog>(
            AExponentialHeightFog::StaticClass(),
            FVector(0, 0, 500),
            FRotator::ZeroRotator
        );
        if (HeightFog)
        {
            HeightFog->SetActorLabel(TEXT("Atmospheric_HeightFog"));
        }
    }
}

void AEnvArt_AtmosphericLightingManager::UpdateTimeOfDayFromNormalizedTime()
{
    EEnvArt_TimeOfDay NewTimeOfDay;
    
    if (CurrentTimeNormalized < 0.15f || CurrentTimeNormalized >= 0.9f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Night;
    }
    else if (CurrentTimeNormalized < 0.25f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Dawn;
    }
    else if (CurrentTimeNormalized < 0.4f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Morning;
    }
    else if (CurrentTimeNormalized < 0.6f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Midday;
    }
    else if (CurrentTimeNormalized < 0.75f)
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    }
    else
    {
        NewTimeOfDay = EEnvArt_TimeOfDay::Dusk;
    }
    
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
    }
}

void AEnvArt_AtmosphericLightingManager::UpdateLightingForCurrentTime()
{
    if (bIsTransitioning)
    {
        return; // Don't override transition
    }
    
    FEnvArt_LightingSettings TargetSettings = GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
    
    // Apply weather modifications
    switch (CurrentWeatherState)
    {
        case EEnvArt_WeatherState::Overcast:
            TargetSettings.SunIntensity *= 0.5f;
            TargetSettings.SkyIntensity *= 0.7f;
            TargetSettings.FogDensity *= 1.5f;
            break;
        case EEnvArt_WeatherState::Foggy:
            TargetSettings.SunIntensity *= 0.3f;
            TargetSettings.SkyIntensity *= 0.5f;
            TargetSettings.FogDensity *= 3.0f;
            break;
        case EEnvArt_WeatherState::Stormy:
            TargetSettings.SunIntensity *= 0.2f;
            TargetSettings.SkyIntensity *= 0.4f;
            TargetSettings.SunColor = FLinearColor(0.6f, 0.6f, 0.8f, 1.0f);
            break;
        case EEnvArt_WeatherState::Volcanic:
            TargetSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
            TargetSettings.FogInscatteringColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
            TargetSettings.FogDensity *= 2.0f;
            break;
        default:
            break;
    }
    
    ApplyLightingSettings(TargetSettings);
}

FEnvArt_LightingSettings AEnvArt_AtmosphericLightingManager::InterpolateLightingSettings(
    const FEnvArt_LightingSettings& A, 
    const FEnvArt_LightingSettings& B, 
    float Alpha) const
{
    FEnvArt_LightingSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.SkyColor = FMath::Lerp(A.SkyColor, B.SkyColor, Alpha);
    
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.VolumetricFogScatteringDistribution = FMath::Lerp(A.VolumetricFogScatteringDistribution, B.VolumetricFogScatteringDistribution, Alpha);
    Result.bVolumetricFog = Alpha > 0.5f ? B.bVolumetricFog : A.bVolumetricFog;
    
    return Result;
}

FEnvArt_LightingSettings AEnvArt_AtmosphericLightingManager::GetLightingSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const
{
    switch (TimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            return DawnSettings;
        case EEnvArt_TimeOfDay::Morning:
            return MorningSettings;
        case EEnvArt_TimeOfDay::Midday:
            return MiddaySettings;
        case EEnvArt_TimeOfDay::Afternoon:
            return AfternoonSettings;
        case EEnvArt_TimeOfDay::Dusk:
            return DuskSettings;
        case EEnvArt_TimeOfDay::Night:
            return NightSettings;
        default:
            return MorningSettings;
    }
}

void AEnvArt_AtmosphericLightingManager::InitializeDefaultLightingPresets()
{
    // Dawn - soft orange light
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunRotation = FRotator(-10.0f, 75.0f, 0.0f);
    DawnSettings.SkyIntensity = 0.8f;
    DawnSettings.SkyColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    DawnSettings.FogDensity = 0.03f;
    DawnSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    
    // Morning - warm golden light
    MorningSettings.SunIntensity = 3.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);
    MorningSettings.SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
    MorningSettings.SkyIntensity = 1.0f;
    MorningSettings.SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MorningSettings.FogDensity = 0.02f;
    MorningSettings.FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    
    // Midday - bright white light
    MiddaySettings.SunIntensity = 5.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    MiddaySettings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    MiddaySettings.SkyIntensity = 1.2f;
    MiddaySettings.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    // Afternoon - warm light
    AfternoonSettings.SunIntensity = 4.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    AfternoonSettings.SunRotation = FRotator(-30.0f, -30.0f, 0.0f);
    AfternoonSettings.SkyIntensity = 1.0f;
    AfternoonSettings.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogInscatteringColor = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);
    
    // Dusk - deep orange/red light
    DuskSettings.SunIntensity = 2.5f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskSettings.SunRotation = FRotator(-5.0f, -75.0f, 0.0f);
    DuskSettings.SkyIntensity = 0.7f;
    DuskSettings.SkyColor = FLinearColor(0.8f, 0.5f, 0.3f, 1.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogInscatteringColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    
    // Night - minimal blue light
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SunRotation = FRotator(30.0f, 180.0f, 0.0f);
    NightSettings.SkyIntensity = 0.3f;
    NightSettings.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.FogDensity = 0.025f;
    NightSettings.FogInscatteringColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
}
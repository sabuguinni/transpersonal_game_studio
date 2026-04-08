#include "LightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALightingManager::ALightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize default time settings
    TimeSettings.CurrentTime = 12.0f; // Noon
    TimeSettings.TimeSpeed = 1.0f;
    TimeSettings.bAutoProgress = true;
    
    // Initialize default states
    CurrentWeather = EWeatherState::Clear;
    CurrentEmotionalTone = EEmotionalTone::Peaceful;
    
    // Initialize transition settings
    TransitionDuration = 5.0f;
    bSmoothTransitions = true;
    bIsTransitioning = false;
    TransitionTimer = 0.0f;
}

void ALightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    SetupDefaultConfigurations();
    
    // Apply initial configuration
    ETimeOfDay InitialTimeOfDay = GetCurrentTimeOfDay();
    if (TimeSettings.TimeConfigurations.Contains(InitialTimeOfDay))
    {
        ApplyLightingConfiguration(TimeSettings.TimeConfigurations[InitialTimeOfDay], true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("LightingManager initialized - Time: %.2f, Weather: %d, Tone: %d"), 
           TimeSettings.CurrentTime, (int32)CurrentWeather, (int32)CurrentEmotionalTone);
}

void ALightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (TimeSettings.bAutoProgress)
    {
        UpdateTimeOfDay(DeltaTime);
    }
    
    if (bIsTransitioning)
    {
        UpdateLightingTransition(DeltaTime);
    }
}

void ALightingManager::InitializeLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("LightingManager: No valid world found"));
        return;
    }
    
    // Find or create directional light (sun)
    SunLight = Cast<ADirectionalLight>(UGameplayStatics::GetActorOfClass(World, ADirectionalLight::StaticClass()));
    if (!SunLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SunLight");
        SunLight = World->SpawnActor<ADirectionalLight>(SpawnParams);
        if (SunLight && SunLight->GetLightComponent())
        {
            SunLight->GetLightComponent()->SetAtmosphereSunLight(true);
            SunLight->GetLightComponent()->SetCastShadows(true);
            SunLight->GetLightComponent()->SetCastVolumetricShadow(true);
        }
    }
    
    // Find or create sky light
    SkyLight = Cast<ASkyLight>(UGameplayStatics::GetActorOfClass(World, ASkyLight::StaticClass()));
    if (!SkyLight)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyLight");
        SkyLight = World->SpawnActor<ASkyLight>(SpawnParams);
        if (SkyLight && SkyLight->GetLightComponent())
        {
            SkyLight->GetLightComponent()->SetSourceType(SLS_CapturedScene);
            SkyLight->GetLightComponent()->SetRealTimeCapture(true);
        }
    }
    
    // Find or create sky atmosphere
    SkyAtmosphere = Cast<ASkyAtmosphere>(UGameplayStatics::GetActorOfClass(World, ASkyAtmosphere::StaticClass()));
    if (!SkyAtmosphere)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("SkyAtmosphere");
        SkyAtmosphere = World->SpawnActor<ASkyAtmosphere>(SpawnParams);
    }
    
    // Find or create volumetric clouds
    VolumetricClouds = Cast<AVolumetricCloud>(UGameplayStatics::GetActorOfClass(World, AVolumetricCloud::StaticClass()));
    if (!VolumetricClouds)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("VolumetricClouds");
        VolumetricClouds = World->SpawnActor<AVolumetricCloud>(SpawnParams);
    }
    
    // Find or create height fog
    HeightFog = Cast<AExponentialHeightFog>(UGameplayStatics::GetActorOfClass(World, AExponentialHeightFog::StaticClass()));
    if (!HeightFog)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("HeightFog");
        HeightFog = World->SpawnActor<AExponentialHeightFog>(SpawnParams);
    }
    
    // Find or create post process volume
    PostProcessVolume = Cast<APostProcessVolume>(UGameplayStatics::GetActorOfClass(World, APostProcessVolume::StaticClass()));
    if (!PostProcessVolume)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("GlobalPostProcess");
        PostProcessVolume = World->SpawnActor<APostProcessVolume>(SpawnParams);
        if (PostProcessVolume)
        {
            PostProcessVolume->bUnbound = true;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("LightingManager: Initialized all lighting components"));
}

void ALightingManager::UpdateTimeOfDay(float DeltaTime)
{
    ETimeOfDay OldTimeOfDay = GetCurrentTimeOfDay();
    
    // Update time
    TimeSettings.CurrentTime += DeltaTime * TimeSettings.TimeSpeed / 3600.0f; // Convert seconds to hours
    
    // Wrap around 24 hours
    if (TimeSettings.CurrentTime >= 24.0f)
    {
        TimeSettings.CurrentTime -= 24.0f;
    }
    else if (TimeSettings.CurrentTime < 0.0f)
    {
        TimeSettings.CurrentTime += 24.0f;
    }
    
    ETimeOfDay NewTimeOfDay = GetCurrentTimeOfDay();
    
    // Check if we've transitioned to a new time of day
    if (OldTimeOfDay != NewTimeOfDay)
    {
        OnTimeOfDayChanged.Broadcast(OldTimeOfDay, NewTimeOfDay);
        
        // Apply new lighting configuration if available
        if (TimeSettings.TimeConfigurations.Contains(NewTimeOfDay))
        {
            ApplyLightingConfiguration(TimeSettings.TimeConfigurations[NewTimeOfDay], false);
        }
    }
}

void ALightingManager::UpdateLightingTransition(float DeltaTime)
{
    if (!bIsTransitioning)
        return;
    
    TransitionTimer += DeltaTime;
    float Alpha = FMath::Clamp(TransitionTimer / TransitionDuration, 0.0f, 1.0f);
    
    // Apply smooth curve to transition
    Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
    
    // Blend configurations
    CurrentConfiguration = BlendConfigurations(SourceConfiguration, TargetConfiguration, Alpha);
    
    // Apply blended configuration
    ApplyConfigurationToComponents(CurrentConfiguration);
    
    // Check if transition is complete
    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;
        TransitionTimer = 0.0f;
        CurrentConfiguration = TargetConfiguration;
        
        OnLightingUpdated.Broadcast(CurrentConfiguration);
    }
}

void ALightingManager::SetTimeOfDay(ETimeOfDay NewTimeOfDay, bool bImmediate)
{
    ETimeOfDay OldTimeOfDay = GetCurrentTimeOfDay();
    
    // Convert time of day to time value
    switch (NewTimeOfDay)
    {
        case ETimeOfDay::Dawn:      TimeSettings.CurrentTime = 6.0f; break;
        case ETimeOfDay::Morning:   TimeSettings.CurrentTime = 9.0f; break;
        case ETimeOfDay::Noon:      TimeSettings.CurrentTime = 12.0f; break;
        case ETimeOfDay::Afternoon: TimeSettings.CurrentTime = 15.0f; break;
        case ETimeOfDay::Dusk:      TimeSettings.CurrentTime = 18.0f; break;
        case ETimeOfDay::Night:     TimeSettings.CurrentTime = 21.0f; break;
        case ETimeOfDay::DeepNight: TimeSettings.CurrentTime = 0.0f; break;
    }
    
    if (OldTimeOfDay != NewTimeOfDay)
    {
        OnTimeOfDayChanged.Broadcast(OldTimeOfDay, NewTimeOfDay);
        
        if (TimeSettings.TimeConfigurations.Contains(NewTimeOfDay))
        {
            ApplyLightingConfiguration(TimeSettings.TimeConfigurations[NewTimeOfDay], bImmediate);
        }
    }
}

void ALightingManager::SetWeather(EWeatherState NewWeather, bool bImmediate)
{
    if (CurrentWeather != NewWeather)
    {
        EWeatherState OldWeather = CurrentWeather;
        CurrentWeather = NewWeather;
        
        OnWeatherChanged.Broadcast(OldWeather, NewWeather);
        
        if (WeatherConfigurations.Contains(NewWeather))
        {
            ApplyLightingConfiguration(WeatherConfigurations[NewWeather], bImmediate);
        }
    }
}

void ALightingManager::SetEmotionalTone(EEmotionalTone NewTone, bool bImmediate)
{
    if (CurrentEmotionalTone != NewTone)
    {
        CurrentEmotionalTone = NewTone;
        
        if (EmotionalConfigurations.Contains(NewTone))
        {
            ApplyLightingConfiguration(EmotionalConfigurations[NewTone], bImmediate);
        }
    }
}

void ALightingManager::SetCurrentTime(float NewTime)
{
    ETimeOfDay OldTimeOfDay = GetCurrentTimeOfDay();
    TimeSettings.CurrentTime = FMath::Fmod(NewTime, 24.0f);
    if (TimeSettings.CurrentTime < 0.0f)
    {
        TimeSettings.CurrentTime += 24.0f;
    }
    
    ETimeOfDay NewTimeOfDay = GetCurrentTimeOfDay();
    if (OldTimeOfDay != NewTimeOfDay)
    {
        OnTimeOfDayChanged.Broadcast(OldTimeOfDay, NewTimeOfDay);
    }
}

void ALightingManager::SetTimeSpeed(float NewSpeed)
{
    TimeSettings.TimeSpeed = FMath::Max(0.0f, NewSpeed);
}

void ALightingManager::ApplyLightingConfiguration(const FLightingConfiguration& Configuration, bool bImmediate)
{
    if (bImmediate || !bSmoothTransitions)
    {
        CurrentConfiguration = Configuration;
        ApplyConfigurationToComponents(Configuration);
        OnLightingUpdated.Broadcast(Configuration);
    }
    else
    {
        // Start transition
        SourceConfiguration = CurrentConfiguration;
        TargetConfiguration = Configuration;
        bIsTransitioning = true;
        TransitionTimer = 0.0f;
    }
}

ETimeOfDay ALightingManager::GetCurrentTimeOfDay() const
{
    return TimeToTimeOfDay(TimeSettings.CurrentTime);
}

void ALightingManager::ForceUpdateLighting()
{
    ApplyConfigurationToComponents(CurrentConfiguration);
    OnLightingUpdated.Broadcast(CurrentConfiguration);
}

void ALightingManager::SaveCurrentConfigurationAsPreset(const FString& PresetName)
{
    // Implementation for saving presets would go here
    // This could save to a data table or JSON file
    UE_LOG(LogTemp, Log, TEXT("Saving lighting preset: %s"), *PresetName);
}

bool ALightingManager::LoadPreset(const FString& PresetName)
{
    // Implementation for loading presets would go here
    UE_LOG(LogTemp, Log, TEXT("Loading lighting preset: %s"), *PresetName);
    return false;
}

void ALightingManager::ApplyConfigurationToComponents(const FLightingConfiguration& Configuration)
{
    UpdateSunLight(Configuration);
    UpdateSkyLight(Configuration);
    UpdateSkyAtmosphere(Configuration);
    UpdateVolumetricClouds(Configuration);
    UpdateHeightFog(Configuration);
    UpdatePostProcess(Configuration);
}

void ALightingManager::UpdateSunLight(const FLightingConfiguration& Configuration)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Configuration.SunColor);
        LightComp->SetIntensity(Configuration.SunIntensity);
        SunLight->SetActorRotation(Configuration.SunRotation);
    }
}

void ALightingManager::UpdateSkyLight(const FLightingConfiguration& Configuration)
{
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* LightComp = SkyLight->GetLightComponent();
        LightComp->SetIntensity(Configuration.SkyIntensity);
        LightComp->SetLightColor(Configuration.SkyColor);
        LightComp->RecaptureSky();
    }
}

void ALightingManager::UpdateSkyAtmosphere(const FLightingConfiguration& Configuration)
{
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* AtmosphereComp = SkyAtmosphere->GetComponent();
        if (AtmosphereComp)
        {
            // Update atmosphere properties based on configuration
            // This would involve setting various atmosphere parameters
        }
    }
}

void ALightingManager::UpdateVolumetricClouds(const FLightingConfiguration& Configuration)
{
    if (VolumetricClouds)
    {
        UVolumetricCloudComponent* CloudComp = VolumetricClouds->GetVolumetricCloudComponent();
        if (CloudComp)
        {
            // Update cloud properties
            // CloudComp->SetLayerBottomAltitude(Configuration.CloudCoverage);
        }
    }
}

void ALightingManager::UpdateHeightFog(const FLightingConfiguration& Configuration)
{
    if (HeightFog && HeightFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
        FogComp->SetFogInscatteringColor(Configuration.FogColor);
        FogComp->SetFogDensity(Configuration.FogDensity);
        FogComp->SetFogHeightFalloff(Configuration.FogHeightFalloff);
    }
}

void ALightingManager::UpdatePostProcess(const FLightingConfiguration& Configuration)
{
    if (PostProcessVolume)
    {
        FPostProcessSettings& Settings = PostProcessVolume->Settings;
        Settings.bOverride_AutoExposureBias = true;
        Settings.AutoExposureBias = Configuration.Exposure;
        
        // Apply color grading
        Settings.bOverride_ColorContrast = true;
        Settings.ColorContrast = FVector4(Configuration.Contrast, Configuration.Contrast, Configuration.Contrast, 1.0f);
        
        Settings.bOverride_ColorSaturation = true;
        Settings.ColorSaturation = FVector4(Configuration.Saturation, Configuration.Saturation, Configuration.Saturation, 1.0f);
        
        Settings.bOverride_ColorGamma = true;
        Settings.ColorGamma = FVector4(Configuration.ColorGrading.R, Configuration.ColorGrading.G, Configuration.ColorGrading.B, 1.0f);
    }
}

FLightingConfiguration ALightingManager::BlendConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha)
{
    FLightingConfiguration Result;
    
    Result.SunColor = BlendColors(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = BlendFloats(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = BlendRotators(A.SunRotation, B.SunRotation, Alpha);
    
    Result.SkyColor = BlendColors(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = BlendFloats(A.SkyIntensity, B.SkyIntensity, Alpha);
    
    Result.AtmosphereThickness = BlendFloats(A.AtmosphereThickness, B.AtmosphereThickness, Alpha);
    
    Result.FogColor = BlendColors(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = BlendFloats(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = BlendFloats(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    
    Result.CloudCoverage = BlendFloats(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.CloudOpacity = BlendFloats(A.CloudOpacity, B.CloudOpacity, Alpha);
    
    Result.Exposure = BlendFloats(A.Exposure, B.Exposure, Alpha);
    Result.Contrast = BlendFloats(A.Contrast, B.Contrast, Alpha);
    Result.Saturation = BlendFloats(A.Saturation, B.Saturation, Alpha);
    Result.ColorGrading = BlendColors(A.ColorGrading, B.ColorGrading, Alpha);
    
    return Result;
}

ETimeOfDay ALightingManager::TimeToTimeOfDay(float Time) const
{
    if (Time >= 5.0f && Time < 8.0f) return ETimeOfDay::Dawn;
    if (Time >= 8.0f && Time < 11.0f) return ETimeOfDay::Morning;
    if (Time >= 11.0f && Time < 14.0f) return ETimeOfDay::Noon;
    if (Time >= 14.0f && Time < 17.0f) return ETimeOfDay::Afternoon;
    if (Time >= 17.0f && Time < 20.0f) return ETimeOfDay::Dusk;
    if (Time >= 20.0f && Time < 23.0f) return ETimeOfDay::Night;
    return ETimeOfDay::DeepNight;
}

void ALightingManager::SetupDefaultConfigurations()
{
    // Setup default time of day configurations
    FLightingConfiguration DawnConfig;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
    DawnConfig.SunIntensity = 2.0f;
    DawnConfig.SunRotation = FRotator(-10.0f, 90.0f, 0.0f);
    DawnConfig.SkyColor = FLinearColor(0.8f, 0.6f, 0.9f);
    DawnConfig.FogColor = FLinearColor(0.9f, 0.7f, 0.8f);
    TimeSettings.TimeConfigurations.Add(ETimeOfDay::Dawn, DawnConfig);
    
    FLightingConfiguration NoonConfig;
    NoonConfig.SunColor = FLinearColor::White;
    NoonConfig.SunIntensity = 5.0f;
    NoonConfig.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    NoonConfig.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f);
    TimeSettings.TimeConfigurations.Add(ETimeOfDay::Noon, NoonConfig);
    
    FLightingConfiguration DuskConfig;
    DuskConfig.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
    DuskConfig.SunIntensity = 1.5f;
    DuskConfig.SunRotation = FRotator(-5.0f, 270.0f, 0.0f);
    DuskConfig.SkyColor = FLinearColor(0.8f, 0.4f, 0.6f);
    TimeSettings.TimeConfigurations.Add(ETimeOfDay::Dusk, DuskConfig);
    
    FLightingConfiguration NightConfig;
    NightConfig.SunColor = FLinearColor(0.2f, 0.2f, 0.4f);
    NightConfig.SunIntensity = 0.1f;
    NightConfig.SkyColor = FLinearColor(0.1f, 0.1f, 0.3f);
    NightConfig.SkyIntensity = 0.3f;
    TimeSettings.TimeConfigurations.Add(ETimeOfDay::Night, NightConfig);
    
    // Setup weather configurations
    FLightingConfiguration StormConfig;
    StormConfig.SkyColor = FLinearColor(0.3f, 0.3f, 0.3f);
    StormConfig.FogDensity = 0.1f;
    StormConfig.CloudCoverage = 0.9f;
    WeatherConfigurations.Add(EWeatherState::Storm, StormConfig);
    
    // Setup emotional tone configurations
    FLightingConfiguration DangerousConfig;
    DangerousConfig.SunColor = FLinearColor(1.0f, 0.3f, 0.3f);
    DangerousConfig.Contrast = 1.3f;
    DangerousConfig.Saturation = 0.8f;
    EmotionalConfigurations.Add(EEmotionalTone::Dangerous, DangerousConfig);
}

FLinearColor ALightingManager::BlendColors(const FLinearColor& A, const FLinearColor& B, float Alpha)
{
    return FLinearColor::LerpUsingHSV(A, B, Alpha);
}

float ALightingManager::BlendFloats(float A, float B, float Alpha)
{
    return FMath::Lerp(A, B, Alpha);
}

FRotator ALightingManager::BlendRotators(const FRotator& A, const FRotator& B, float Alpha)
{
    return FMath::Lerp(A, B, Alpha);
}
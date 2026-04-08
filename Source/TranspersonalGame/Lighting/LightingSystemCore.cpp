#include "LightingSystemCore.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ULightingSystemCore::ULightingSystemCore()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for smooth transitions
    
    SetupDefaultConfigurations();
}

void ULightingSystemCore::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingComponents();
    UpdateLightingConfiguration();
}

void ULightingSystemCore::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update dynamic time progression
    if (bDynamicTimeProgression)
    {
        CurrentTimeInHours += (DeltaTime * TimeProgressionScale) / 3600.0f; // Convert seconds to hours
        
        // Wrap around 24-hour cycle
        if (CurrentTimeInHours >= 24.0f)
        {
            CurrentTimeInHours -= 24.0f;
        }
        
        UpdateTimeOfDay();
    }

    // Update lighting transitions
    UpdateTransition(DeltaTime);
    
    // Apply current lighting configuration
    UpdateLightingConfiguration();
}

void ULightingSystemCore::SetTimeOfDay(float TimeInHours)
{
    CurrentTimeInHours = FMath::Fmod(TimeInHours, 24.0f);
    if (CurrentTimeInHours < 0.0f)
    {
        CurrentTimeInHours += 24.0f;
    }
    
    UpdateTimeOfDay();
    UpdateLightingConfiguration();
}

void ULightingSystemCore::UpdateTimeOfDay()
{
    ETimeOfDay NewTimeOfDay;
    
    if (CurrentTimeInHours >= 5.0f && CurrentTimeInHours < 7.0f)
    {
        NewTimeOfDay = ETimeOfDay::Dawn;
    }
    else if (CurrentTimeInHours >= 7.0f && CurrentTimeInHours < 11.0f)
    {
        NewTimeOfDay = ETimeOfDay::Morning;
    }
    else if (CurrentTimeInHours >= 11.0f && CurrentTimeInHours < 15.0f)
    {
        NewTimeOfDay = ETimeOfDay::Midday;
    }
    else if (CurrentTimeInHours >= 15.0f && CurrentTimeInHours < 18.0f)
    {
        NewTimeOfDay = ETimeOfDay::Afternoon;
    }
    else if (CurrentTimeInHours >= 18.0f && CurrentTimeInHours < 20.0f)
    {
        NewTimeOfDay = ETimeOfDay::Dusk;
    }
    else
    {
        NewTimeOfDay = ETimeOfDay::Night;
    }
    
    CurrentTimeOfDay = NewTimeOfDay;
}

void ULightingSystemCore::SetWeatherState(EWeatherState NewWeatherState, float TransitionTime)
{
    if (NewWeatherState != CurrentWeatherState)
    {
        // Start transition
        CurrentTransition.StartConfig = BlendConfigurations(
            TimeOfDayConfigurations[CurrentTimeOfDay],
            WeatherConfigurations[CurrentWeatherState],
            0.5f
        );
        
        CurrentWeatherState = NewWeatherState;
        
        CurrentTransition.TargetConfig = BlendConfigurations(
            TimeOfDayConfigurations[CurrentTimeOfDay],
            WeatherConfigurations[CurrentWeatherState],
            0.5f
        );
        
        CurrentTransition.TransitionTime = TransitionTime;
        CurrentTransition.CurrentTime = 0.0f;
        CurrentTransition.bActive = true;
    }
}

void ULightingSystemCore::SetThreatLevel(EThreatLevel NewThreatLevel, float TransitionTime)
{
    if (NewThreatLevel != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreatLevel;
        
        // Threat level changes are immediate but can have smooth transitions
        CurrentTransition.TransitionTime = TransitionTime;
        CurrentTransition.CurrentTime = 0.0f;
        CurrentTransition.bActive = true;
    }
}

void ULightingSystemCore::EnableDynamicTimeProgression(bool bEnable, float TimeScale)
{
    bDynamicTimeProgression = bEnable;
    TimeProgressionScale = TimeScale;
}

void ULightingSystemCore::UpdateLightingConfiguration()
{
    if (!SunLight || !SkyLight || !SkyAtmosphere || !VolumetricClouds || !HeightFog)
    {
        return;
    }

    // Get base configuration from time of day
    FLightingConfiguration BaseConfig = TimeOfDayConfigurations[CurrentTimeOfDay];
    
    // Blend with weather configuration
    FLightingConfiguration WeatherBlended = BlendConfigurations(
        BaseConfig,
        WeatherConfigurations[CurrentWeatherState],
        0.6f // Weather has moderate influence
    );
    
    // Blend with threat level configuration
    FLightingConfiguration FinalConfig = BlendConfigurations(
        WeatherBlended,
        ThreatLevelConfigurations[CurrentThreatLevel],
        0.4f // Threat level has strong influence on mood
    );
    
    // Apply transition if active
    if (CurrentTransition.bActive)
    {
        float Alpha = CurrentTransition.CurrentTime / CurrentTransition.TransitionTime;
        FinalConfig = BlendConfigurations(CurrentTransition.StartConfig, CurrentTransition.TargetConfig, Alpha);
    }
    
    ApplyLightingConfiguration(FinalConfig);
}

void ULightingSystemCore::ApplyLightingConfiguration(const FLightingConfiguration& Config, float BlendWeight)
{
    if (!SunLight || !SkyLight || !HeightFog) return;

    // === SUN LIGHT CONFIGURATION ===
    UDirectionalLightComponent* SunComponent = SunLight->GetComponent();
    if (SunComponent)
    {
        SunComponent->SetLightColor(Config.SunColor);
        SunComponent->SetIntensity(Config.SunIntensity);
        
        // Calculate sun rotation based on time and angle
        float SunRotationPitch = -90.0f + Config.SunAngle; // -90 is straight down, 0 is horizontal
        FRotator SunRotation = FRotator(SunRotationPitch, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        // Enable atmosphere sun light
        SunComponent->SetAtmosphereSunLight(true);
        SunComponent->SetAtmosphereSunLightIndex(0);
    }

    // === SKY LIGHT CONFIGURATION ===
    USkyLightComponent* SkyComponent = SkyLight->GetComponent();
    if (SkyComponent)
    {
        SkyComponent->SetIntensity(Config.SkyIntensity);
        SkyComponent->SetLightColor(Config.SkyColor);
        
        // Enable real-time capture for dynamic time of day
        SkyComponent->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
        SkyComponent->SetRealTimeCapture(true);
    }

    // === HEIGHT FOG CONFIGURATION ===
    UExponentialHeightFogComponent* FogComponent = HeightFog->GetComponent();
    if (FogComponent)
    {
        FogComponent->SetFogInscatteringColor(Config.FogColor);
        FogComponent->SetFogDensity(Config.FogDensity * Config.ThreatMultiplier);
        FogComponent->SetFogHeightFalloff(Config.FogHeightFalloff);
        
        // Adjust fog based on threat level
        float ThreatFogMultiplier = 1.0f;
        switch (CurrentThreatLevel)
        {
            case EThreatLevel::Safe:
                ThreatFogMultiplier = 0.7f;
                break;
            case EThreatLevel::Cautious:
                ThreatFogMultiplier = 1.0f;
                break;
            case EThreatLevel::Dangerous:
                ThreatFogMultiplier = 1.3f;
                break;
            case EThreatLevel::Predator:
                ThreatFogMultiplier = 1.8f;
                break;
        }
        
        FogComponent->SetFogDensity(Config.FogDensity * ThreatFogMultiplier);
    }

    // === VOLUMETRIC CLOUDS CONFIGURATION ===
    if (VolumetricClouds)
    {
        UVolumetricCloudComponent* CloudComponent = VolumetricClouds->GetComponent();
        if (CloudComponent)
        {
            // Cloud coverage varies with weather
            float CloudCoverage = Config.CloudCoverage;
            switch (CurrentWeatherState)
            {
                case EWeatherState::Clear:
                    CloudCoverage *= 0.2f;
                    break;
                case EWeatherState::PartlyCloudy:
                    CloudCoverage *= 0.6f;
                    break;
                case EWeatherState::Overcast:
                    CloudCoverage *= 1.2f;
                    break;
                case EWeatherState::LightRain:
                case EWeatherState::HeavyRain:
                case EWeatherState::Storm:
                    CloudCoverage *= 1.5f;
                    break;
            }
            
            // Apply cloud settings through material parameters if available
            // This would require a material instance with exposed parameters
        }
    }
}

void ULightingSystemCore::InitializeLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find lighting components in the world
    if (!SunLight)
    {
        TArray<AActor*> DirectionalLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
        
        for (AActor* Actor : DirectionalLights)
        {
            ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor);
            if (DirLight && DirLight->GetComponent()->GetAtmosphereSunLight())
            {
                SunLight = DirLight;
                break;
            }
        }
    }

    if (!SkyLight)
    {
        TArray<AActor*> SkyLights;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
        if (SkyLights.Num() > 0)
        {
            SkyLight = Cast<ASkyLight>(SkyLights[0]);
        }
    }

    if (!SkyAtmosphere)
    {
        TArray<AActor*> SkyAtmospheres;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyAtmospheres);
        if (SkyAtmospheres.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(SkyAtmospheres[0]);
        }
    }

    if (!VolumetricClouds)
    {
        TArray<AActor*> Clouds;
        UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), Clouds);
        if (Clouds.Num() > 0)
        {
            VolumetricClouds = Cast<AVolumetricCloud>(Clouds[0]);
        }
    }

    if (!HeightFog)
    {
        TArray<AActor*> Fogs;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), Fogs);
        if (Fogs.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(Fogs[0]);
        }
    }
}

void ULightingSystemCore::SetupDefaultConfigurations()
{
    // === TIME OF DAY CONFIGURATIONS ===
    
    // Dawn (5:00-7:00) - Soft, warm, mysterious
    FLightingConfiguration DawnConfig;
    DawnConfig.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f); // Warm orange
    DawnConfig.SunIntensity = 3.0f;
    DawnConfig.SunAngle = 15.0f; // Low angle
    DawnConfig.SkyColor = FLinearColor(0.8f, 0.4f, 0.6f, 1.0f); // Pink sky
    DawnConfig.SkyIntensity = 0.5f;
    DawnConfig.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    DawnConfig.FogDensity = 0.03f;
    DawnConfig.CloudCoverage = 0.3f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Dawn, DawnConfig);

    // Morning (7:00-11:00) - Clear, hopeful
    FLightingConfiguration MorningConfig;
    MorningConfig.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningConfig.SunIntensity = 8.0f;
    MorningConfig.SunAngle = 35.0f;
    MorningConfig.SkyColor = FLinearColor(0.4f, 0.7f, 1.0f, 1.0f);
    MorningConfig.SkyIntensity = 1.0f;
    MorningConfig.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningConfig.FogDensity = 0.015f;
    MorningConfig.CloudCoverage = 0.4f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Morning, MorningConfig);

    // Midday (11:00-15:00) - Bright, harsh, exposed
    FLightingConfiguration MiddayConfig;
    MiddayConfig.SunColor = FLinearColor(1.0f, 1.0f, 0.9f, 1.0f);
    MiddayConfig.SunIntensity = 12.0f;
    MiddayConfig.SunAngle = 60.0f; // High angle
    MiddayConfig.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    MiddayConfig.SkyIntensity = 1.2f;
    MiddayConfig.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    MiddayConfig.FogDensity = 0.01f;
    MiddayConfig.CloudCoverage = 0.5f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Midday, MiddayConfig);

    // Afternoon (15:00-18:00) - Warm, golden hour approaching
    FLightingConfiguration AfternoonConfig;
    AfternoonConfig.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonConfig.SunIntensity = 9.0f;
    AfternoonConfig.SunAngle = 40.0f;
    AfternoonConfig.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    AfternoonConfig.SkyIntensity = 1.0f;
    AfternoonConfig.FogColor = FLinearColor(0.8f, 0.8f, 0.7f, 1.0f);
    AfternoonConfig.FogDensity = 0.02f;
    AfternoonConfig.CloudCoverage = 0.6f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Afternoon, AfternoonConfig);

    // Dusk (18:00-20:00) - Golden hour, dramatic, beautiful but ominous
    FLightingConfiguration DuskConfig;
    DuskConfig.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f); // Deep orange
    DuskConfig.SunIntensity = 4.0f;
    DuskConfig.SunAngle = 10.0f; // Very low
    DuskConfig.SkyColor = FLinearColor(1.0f, 0.3f, 0.4f, 1.0f); // Red sky
    DuskConfig.SkyIntensity = 0.7f;
    DuskConfig.FogColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskConfig.FogDensity = 0.025f;
    DuskConfig.CloudCoverage = 0.4f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Dusk, DuskConfig);

    // Night (20:00-5:00) - Dark, mysterious, dangerous
    FLightingConfiguration NightConfig;
    NightConfig.SunColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f); // Moonlight
    NightConfig.SunIntensity = 0.5f;
    NightConfig.SunAngle = -30.0f; // Below horizon
    NightConfig.SkyColor = FLinearColor(0.05f, 0.1f, 0.2f, 1.0f);
    NightConfig.SkyIntensity = 0.2f;
    NightConfig.FogColor = FLinearColor(0.1f, 0.15f, 0.3f, 1.0f);
    NightConfig.FogDensity = 0.04f;
    NightConfig.CloudCoverage = 0.7f;
    TimeOfDayConfigurations.Add(ETimeOfDay::Night, NightConfig);

    // === WEATHER STATE CONFIGURATIONS ===
    
    // Clear Sky - Enhances base lighting
    FLightingConfiguration ClearConfig;
    ClearConfig.SunIntensity = 1.2f; // Multiplier
    ClearConfig.SkyIntensity = 1.1f;
    ClearConfig.FogDensity = 0.8f; // Multiplier - less fog
    ClearConfig.CloudCoverage = 0.2f;
    WeatherConfigurations.Add(EWeatherState::Clear, ClearConfig);

    // Overcast - Diffused, muted lighting
    FLightingConfiguration OvercastConfig;
    OvercastConfig.SunIntensity = 0.6f;
    OvercastConfig.SkyIntensity = 1.3f; // More sky light
    OvercastConfig.FogDensity = 1.2f;
    OvercastConfig.CloudCoverage = 0.9f;
    WeatherConfigurations.Add(EWeatherState::Overcast, OvercastConfig);

    // Storm - Dark, dramatic
    FLightingConfiguration StormConfig;
    StormConfig.SunIntensity = 0.3f;
    StormConfig.SkyIntensity = 0.7f;
    StormConfig.FogDensity = 1.5f;
    StormConfig.CloudCoverage = 1.0f;
    WeatherConfigurations.Add(EWeatherState::Storm, StormConfig);

    // === THREAT LEVEL CONFIGURATIONS ===
    
    // Safe - Bright, warm
    FLightingConfiguration SafeConfig;
    SafeConfig.WarmthFactor = 1.2f;
    SafeConfig.ThreatMultiplier = 0.8f;
    ThreatLevelConfigurations.Add(EThreatLevel::Safe, SafeConfig);

    // Predator - Cold, dark, high contrast
    FLightingConfiguration PredatorConfig;
    PredatorConfig.SunIntensity = 0.7f;
    PredatorConfig.SkyIntensity = 0.6f;
    PredatorConfig.FogDensity = 1.4f;
    PredatorConfig.WarmthFactor = 0.6f;
    PredatorConfig.ThreatMultiplier = 1.5f;
    ThreatLevelConfigurations.Add(EThreatLevel::Predator, PredatorConfig);
}

FLightingConfiguration ULightingSystemCore::BlendConfigurations(const FLightingConfiguration& A, const FLightingConfiguration& B, float Alpha)
{
    FLightingConfiguration Result;
    
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.SkyColor = FMath::Lerp(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogColor = FMath::Lerp(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.CloudCoverage = FMath::Lerp(A.CloudCoverage, B.CloudCoverage, Alpha);
    Result.CloudOpacity = FMath::Lerp(A.CloudOpacity, B.CloudOpacity, Alpha);
    Result.ThreatMultiplier = FMath::Lerp(A.ThreatMultiplier, B.ThreatMultiplier, Alpha);
    Result.WarmthFactor = FMath::Lerp(A.WarmthFactor, B.WarmthFactor, Alpha);
    
    return Result;
}

void ULightingSystemCore::UpdateTransition(float DeltaTime)
{
    if (CurrentTransition.bActive)
    {
        CurrentTransition.CurrentTime += DeltaTime;
        
        if (CurrentTransition.CurrentTime >= CurrentTransition.TransitionTime)
        {
            CurrentTransition.bActive = false;
            CurrentTransition.CurrentTime = 0.0f;
        }
    }
}
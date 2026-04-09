// Copyright Transpersonal Game Studio. All Rights Reserved.
// Dynamic Atmosphere System - Real-time atmospheric lighting for Jurassic environments
// Agent #08 - Lighting & Atmosphere Agent
// VALIDATION_AND_FIX_001

#include "DynamicAtmosphereSystem.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogDynamicAtmosphere, Log, All);

ADynamicAtmosphereSystem::ADynamicAtmosphereSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f / 30.0f; // 30 FPS for atmosphere updates
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    // Create sky atmosphere component
    SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
    SkyAtmosphereComponent->SetupAttachment(RootComponent);
    
    // Create volumetric clouds component
    VolumetricCloudsComponent = CreateDefaultSubobject<UVolumetricCloudComponent>(TEXT("VolumetricClouds"));
    VolumetricCloudsComponent->SetupAttachment(RootComponent);
    
    // Initialize default values
    TimeOfDay = 12.0f; // Noon
    TimeSpeed = 1.0f;
    WeatherIntensity = 0.0f;
    AtmosphericDensity = 1.0f;
    bEnableWeatherSystem = true;
    bEnableDynamicClouds = true;
    bEnableAtmosphericScattering = true;
    
    // Initialize sun parameters
    SunElevation = 45.0f;
    SunAzimuth = 180.0f;
    SunIntensity = 10.0f;
    SunTemperature = 6500.0f;
    
    // Initialize atmosphere parameters
    RayleighScatteringScale = 1.0f;
    MieScatteringScale = 1.0f;
    OzoneAbsorptionScale = 1.0f;
    
    // Initialize cloud parameters
    CloudCoverage = 0.5f;
    CloudDensity = 1.0f;
    CloudSpeed = FVector2D(10.0f, 5.0f);
    CloudAltitude = 5000.0f;
    
    // Initialize fog parameters
    FogDensity = 0.02f;
    FogHeight = 1000.0f;
    FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    
    // Initialize weather transition
    WeatherTransitionSpeed = 1.0f;
    TargetWeatherType = EWeatherType::Clear;
    CurrentWeatherType = EWeatherType::Clear;
}

void ADynamicAtmosphereSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogDynamicAtmosphere, Log, TEXT("Dynamic Atmosphere System initialized"));
    
    // Find lighting components in the world
    FindLightingComponents();
    
    // Initialize atmosphere settings
    InitializeAtmosphereSettings();
    
    // Set initial time of day
    UpdateTimeOfDay();
}

void ADynamicAtmosphereSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time progression
    if (bEnableTimeProgression)
    {
        TimeOfDay += DeltaTime * TimeSpeed * (24.0f / 3600.0f); // Convert to hours
        if (TimeOfDay >= 24.0f)
        {
            TimeOfDay -= 24.0f;
        }
    }
    
    // Update atmospheric systems
    UpdateTimeOfDay();
    UpdateWeatherSystem(DeltaTime);
    UpdateCloudSystem(DeltaTime);
    UpdateFogSystem(DeltaTime);
    UpdateAtmosphericScattering(DeltaTime);
    
    // Apply emotional lighting modifications
    ApplyEmotionalLighting(DeltaTime);
}

void ADynamicAtmosphereSystem::FindLightingComponents()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
        if (SunLight)
        {
            UE_LOG(LogDynamicAtmosphere, Log, TEXT("Found sun light: %s"), *SunLight->GetName());
        }
    }
    
    // Find sky light
    TArray<AActor*> SkyLights;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
    
    if (SkyLights.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(SkyLights[0]);
        if (SkyLight)
        {
            UE_LOG(LogDynamicAtmosphere, Log, TEXT("Found sky light: %s"), *SkyLight->GetName());
        }
    }
    
    // Find exponential height fog
    TArray<AActor*> HeightFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), HeightFogs);
    
    if (HeightFogs.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(HeightFogs[0]);
        if (HeightFog)
        {
            UE_LOG(LogDynamicAtmosphere, Log, TEXT("Found height fog: %s"), *HeightFog->GetName());
        }
    }
    
    // Find post process volume
    TArray<AActor*> PostProcessVolumes;
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), PostProcessVolumes);
    
    if (PostProcessVolumes.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessVolumes[0]);
        if (PostProcessVolume)
        {
            UE_LOG(LogDynamicAtmosphere, Log, TEXT("Found post process volume: %s"), *PostProcessVolume->GetName());
        }
    }
}

void ADynamicAtmosphereSystem::InitializeAtmosphereSettings()
{
    if (SkyAtmosphereComponent)
    {
        // Set Earth-like atmosphere parameters
        SkyAtmosphereComponent->SetRayleighScatteringScale(RayleighScatteringScale);
        SkyAtmosphereComponent->SetMieScatteringScale(MieScatteringScale);
        SkyAtmosphereComponent->SetAbsorptionScale(OzoneAbsorptionScale);
        
        UE_LOG(LogDynamicAtmosphere, Log, TEXT("Sky atmosphere component initialized"));
    }
    
    if (VolumetricCloudsComponent)
    {
        // Initialize cloud parameters
        VolumetricCloudsComponent->SetLayerBottomAltitude(CloudAltitude);
        
        UE_LOG(LogDynamicAtmosphere, Log, TEXT("Volumetric clouds component initialized"));
    }
}

void ADynamicAtmosphereSystem::UpdateTimeOfDay()
{
    // Calculate sun position based on time of day
    float SunAngle = (TimeOfDay - 6.0f) * 15.0f; // 6 AM = 0°, 6 PM = 180°
    SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    
    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        // Set sun rotation
        FRotator SunRotation = FRotator(-SunElevation, SunAzimuth, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        // Adjust intensity based on elevation
        float IntensityMultiplier = FMath::Max(0.0f, FMath::Sin(FMath::DegreesToRadians(SunElevation + 10.0f)));
        LightComp->SetIntensity(SunIntensity * IntensityMultiplier);
        
        // Adjust color temperature
        float Temperature = FMath::Lerp(2000.0f, SunTemperature, IntensityMultiplier);
        LightComp->SetTemperature(Temperature);
        
        // Enable/disable sun based on elevation
        LightComp->SetVisibility(SunElevation > -10.0f);
    }
    
    // Update sky light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        
        // Adjust sky light intensity based on time of day
        float SkyIntensity = FMath::Max(0.1f, FMath::Sin(FMath::DegreesToRadians(SunElevation + 20.0f)));
        SkyComp->SetIntensity(SkyIntensity);
        
        // Recapture sky light if real-time capture is enabled
        if (SkyComp->bRealTimeCapture)
        {
            SkyComp->RecaptureSky();
        }
    }
}

void ADynamicAtmosphereSystem::UpdateWeatherSystem(float DeltaTime)
{
    if (!bEnableWeatherSystem) return;
    
    // Transition between weather types
    if (CurrentWeatherType != TargetWeatherType)
    {
        float TransitionRate = WeatherTransitionSpeed * DeltaTime;
        
        // Update weather intensity based on target
        float TargetIntensity = GetWeatherIntensity(TargetWeatherType);
        WeatherIntensity = FMath::FInterpTo(WeatherIntensity, TargetIntensity, DeltaTime, TransitionRate);
        
        // Check if transition is complete
        if (FMath::Abs(WeatherIntensity - TargetIntensity) < 0.01f)
        {
            CurrentWeatherType = TargetWeatherType;
            WeatherIntensity = TargetIntensity;
        }
    }
    
    // Apply weather effects
    ApplyWeatherEffects();
}

void ADynamicAtmosphereSystem::UpdateCloudSystem(float DeltaTime)
{
    if (!bEnableDynamicClouds || !VolumetricCloudsComponent) return;
    
    // Animate cloud movement
    CloudOffset += CloudSpeed * DeltaTime;
    
    // Apply cloud parameters
    // Note: UE5 VolumetricCloudComponent properties may vary by version
    // This is a simplified implementation
}

void ADynamicAtmosphereSystem::UpdateFogSystem(float DeltaTime)
{
    if (!HeightFog || !HeightFog->GetComponent()) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    
    // Update fog based on weather and time of day
    float TimeBasedDensity = FMath::Lerp(FogDensity * 0.5f, FogDensity, 
        FMath::Max(0.0f, 1.0f - FMath::Abs(SunElevation) / 90.0f));
    
    float WeatherBasedDensity = TimeBasedDensity * (1.0f + WeatherIntensity * 2.0f);
    
    FogComp->SetFogDensity(WeatherBasedDensity);
    FogComp->SetFogHeightFalloff(1.0f / FogHeight);
    FogComp->SetFogInscatteringColor(FogColor);
}

void ADynamicAtmosphereSystem::UpdateAtmosphericScattering(float DeltaTime)
{
    if (!bEnableAtmosphericScattering || !SkyAtmosphereComponent) return;
    
    // Adjust scattering based on atmospheric density and weather
    float DensityMultiplier = AtmosphericDensity * (1.0f + WeatherIntensity * 0.5f);
    
    SkyAtmosphereComponent->SetRayleighScatteringScale(RayleighScatteringScale * DensityMultiplier);
    SkyAtmosphereComponent->SetMieScatteringScale(MieScatteringScale * DensityMultiplier);
}

void ADynamicAtmosphereSystem::ApplyEmotionalLighting(float DeltaTime)
{
    // Apply emotional lighting modifications based on current emotional state
    // This integrates with the EmotionalLightingController
    
    if (!PostProcessVolume) return;
    
    // Get current emotional state (this would be set by game systems)
    float TensionLevel = GetCurrentTensionLevel();
    
    // Modify post-process settings based on tension
    FPostProcessSettings& PPSettings = PostProcessVolume->Settings;
    
    // Increase contrast and reduce saturation for tension
    PPSettings.ColorSaturation = FVector4(
        FMath::Lerp(1.0f, 0.7f, TensionLevel),
        FMath::Lerp(1.0f, 0.7f, TensionLevel),
        FMath::Lerp(1.0f, 0.7f, TensionLevel),
        1.0f
    );
    
    PPSettings.ColorContrast = FVector4(
        FMath::Lerp(1.0f, 1.3f, TensionLevel),
        FMath::Lerp(1.0f, 1.3f, TensionLevel),
        FMath::Lerp(1.0f, 1.3f, TensionLevel),
        1.0f
    );
    
    // Adjust fog for claustrophobic effect
    if (HeightFog && HeightFog->GetComponent())
    {
        float TensionFogMultiplier = 1.0f + TensionLevel * 0.5f;
        HeightFog->GetComponent()->SetFogDensity(FogDensity * TensionFogMultiplier);
    }
}

float ADynamicAtmosphereSystem::GetWeatherIntensity(EWeatherType WeatherType) const
{
    switch (WeatherType)
    {
        case EWeatherType::Clear: return 0.0f;
        case EWeatherType::PartlyCloudy: return 0.3f;
        case EWeatherType::Overcast: return 0.6f;
        case EWeatherType::LightRain: return 0.8f;
        case EWeatherType::HeavyRain: return 1.0f;
        case EWeatherType::Storm: return 1.5f;
        case EWeatherType::Fog: return 0.4f;
        default: return 0.0f;
    }
}

void ADynamicAtmosphereSystem::ApplyWeatherEffects()
{
    // Apply weather-specific effects to lighting and atmosphere
    switch (CurrentWeatherType)
    {
        case EWeatherType::Clear:
            // Bright, clear conditions
            if (SunLight && SunLight->GetLightComponent())
            {
                SunLight->GetLightComponent()->SetIntensity(SunIntensity);
            }
            break;
            
        case EWeatherType::Overcast:
            // Reduced sun intensity, increased sky light
            if (SunLight && SunLight->GetLightComponent())
            {
                SunLight->GetLightComponent()->SetIntensity(SunIntensity * 0.3f);
            }
            if (SkyLight && SkyLight->GetLightComponent())
            {
                SkyLight->GetLightComponent()->SetIntensity(1.5f);
            }
            break;
            
        case EWeatherType::Storm:
            // Dramatic lighting with flickering effects
            if (SunLight && SunLight->GetLightComponent())
            {
                float FlickerIntensity = SunIntensity * 0.1f * (1.0f + FMath::Sin(GetWorld()->GetTimeSeconds() * 10.0f) * 0.5f);
                SunLight->GetLightComponent()->SetIntensity(FlickerIntensity);
            }
            break;
            
        default:
            break;
    }
}

float ADynamicAtmosphereSystem::GetCurrentTensionLevel() const
{
    // This would integrate with the game's tension system
    // For now, return a placeholder value
    return 0.3f; // Base tension level for Jurassic environment
}

void ADynamicAtmosphereSystem::SetTimeOfDay(float NewTimeOfDay)
{
    TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateTimeOfDay();
}

void ADynamicAtmosphereSystem::SetWeatherType(EWeatherType NewWeatherType, float TransitionTime)
{
    TargetWeatherType = NewWeatherType;
    WeatherTransitionSpeed = 1.0f / FMath::Max(0.1f, TransitionTime);
}

void ADynamicAtmosphereSystem::SetAtmosphericDensity(float NewDensity)
{
    AtmosphericDensity = FMath::Clamp(NewDensity, 0.1f, 3.0f);
    UpdateAtmosphericScattering(0.0f);
}

void ADynamicAtmosphereSystem::SetCloudParameters(float Coverage, float Density, FVector2D Speed)
{
    CloudCoverage = FMath::Clamp(Coverage, 0.0f, 1.0f);
    CloudDensity = FMath::Clamp(Density, 0.0f, 2.0f);
    CloudSpeed = Speed;
}

void ADynamicAtmosphereSystem::SetFogParameters(float Density, float Height, FLinearColor Color)
{
    FogDensity = FMath::Max(0.0f, Density);
    FogHeight = FMath::Max(100.0f, Height);
    FogColor = Color;
    
    UpdateFogSystem(0.0f);
}
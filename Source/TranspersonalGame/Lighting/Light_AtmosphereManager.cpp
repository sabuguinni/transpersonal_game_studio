#include "Light_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    TimeOfDaySpeed = 1.0f;
    CurrentTimeOfDay = 12.0f; // Start at noon
    CurrentWeatherState = ELight_WeatherState::Clear;
    WeatherTransitionSpeed = 0.5f;
    
    // Initialize atmospheric settings for Cretaceous period
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    AtmosphericSettings.SunTemperature = 5500.0f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.StartDistance = 5000.0f;
    AtmosphericSettings.bVolumetricFog = true;
    
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    HeightFog = nullptr;
    PostProcessVolume = nullptr;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmosphericComponents();
    ApplyAtmosphericSettings(AtmosphericSettings);
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time of day
    CurrentTimeOfDay += TimeOfDaySpeed * DeltaTime / 3600.0f; // Convert to hours
    if (CurrentTimeOfDay >= 24.0f)
    {
        CurrentTimeOfDay = 0.0f;
    }
    
    UpdateSunPosition();
    UpdateWeatherEffects();
    
    // Handle weather transitions
    if (bIsTransitioning)
    {
        WeatherTransitionTimer += DeltaTime;
        // Weather transition logic would go here
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentTimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 24.0f);
    UpdateSunPosition();
}

void ALight_AtmosphereManager::SetWeatherState(ELight_WeatherState NewWeatherState)
{
    if (CurrentWeatherState != NewWeatherState)
    {
        TransitionToWeather(NewWeatherState, 5.0f);
    }
}

void ALight_AtmosphereManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    AtmosphericSettings = Settings;
    
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetTemperature(Settings.SunTemperature);
    }
    
    UpdateFogSettings();
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void ALight_AtmosphereManager::InitializeAtmosphericComponents()
{
    FindAtmosphericActors();
    ConfigureLumenSettings();
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunLight) return;
    
    // Calculate sun angle based on time of day (0-24 hours)
    float SunAngle = (CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    float SunPitch = FMath::Sin(FMath::DegreesToRadians(SunAngle * 0.5f)) * 90.0f;
    float SunYaw = SunAngle;
    
    FRotator SunRotation(SunPitch, SunYaw, 0.0f);
    SunLight->SetActorRotation(SunRotation);
    
    // Adjust intensity based on sun height
    if (SunLight->GetLightComponent())
    {
        float IntensityMultiplier = FMath::Clamp(FMath::Sin(FMath::DegreesToRadians(SunPitch)), 0.1f, 1.0f);
        float AdjustedIntensity = AtmosphericSettings.SunIntensity * IntensityMultiplier;
        SunLight->GetLightComponent()->SetIntensity(AdjustedIntensity);
    }
}

void ALight_AtmosphereManager::UpdateWeatherEffects()
{
    if (!HeightFog) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (!FogComp) return;
    
    float TargetDensity = AtmosphericSettings.FogDensity;
    
    // Adjust fog based on weather
    switch (CurrentWeatherState)
    {
        case ELight_WeatherState::Clear:
            TargetDensity *= 0.5f;
            break;
        case ELight_WeatherState::Overcast:
            TargetDensity *= 1.5f;
            break;
        case ELight_WeatherState::Foggy:
            TargetDensity *= 3.0f;
            break;
        case ELight_WeatherState::Storm:
            TargetDensity *= 2.0f;
            break;
    }
    
    // Smooth transition to target density
    float CurrentDensity = FogComp->GetFogDensity();
    float NewDensity = FMath::FInterpTo(CurrentDensity, TargetDensity, GetWorld()->GetDeltaSeconds(), 2.0f);
    FogComp->SetFogDensity(NewDensity);
}

void ALight_AtmosphereManager::TransitionToWeather(ELight_WeatherState TargetWeather, float TransitionDuration)
{
    TargetWeatherState = TargetWeather;
    WeatherTransitionTimer = 0.0f;
    bIsTransitioning = true;
    
    // Immediate transition for now - could be enhanced with gradual transitions
    CurrentWeatherState = TargetWeather;
    bIsTransitioning = false;
}

void ALight_AtmosphereManager::FindAtmosphericActors()
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
    
    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
    }
}

void ALight_AtmosphereManager::ConfigureLumenSettings()
{
    if (PostProcessVolume && PostProcessVolume->Settings.bOverride_DynamicGlobalIlluminationMethod)
    {
        // Configure Lumen for optimal Cretaceous lighting
        PostProcessVolume->Settings.bOverride_DynamicGlobalIlluminationMethod = true;
        PostProcessVolume->Settings.bOverride_ReflectionMethod = true;
        PostProcessVolume->Settings.bOverride_LumenSceneLightingQuality = true;
        PostProcessVolume->Settings.LumenSceneLightingQuality = 1.0f; // High quality
    }
}

void ALight_AtmosphereManager::UpdateFogSettings()
{
    if (!HeightFog) return;
    
    UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent();
    if (!FogComp) return;
    
    FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
    FogComp->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
    FogComp->SetStartDistance(AtmosphericSettings.StartDistance);
    FogComp->SetVolumetricFog(AtmosphericSettings.bVolumetricFog);
}
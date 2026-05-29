#include "World_BiomeWeatherManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AWorld_BiomeWeatherManager::AWorld_BiomeWeatherManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default weather state
    CurrentWeather.BiomeType = EWorld_BiomeType::Savana;
    CurrentWeather.Temperature = 25.0f;
    CurrentWeather.Humidity = 0.5f;
    CurrentWeather.WindStrength = 0.3f;
    CurrentWeather.RainIntensity = 0.0f;
    CurrentWeather.FogDensity = 0.1f;
    CurrentWeather.SkyTint = FLinearColor(0.5f, 0.8f, 1.0f, 1.0f);

    WeatherTransitionSpeed = 1.0f;
    bEnableDynamicWeather = true;
    WeatherUpdateInterval = 30.0f;
    WeatherTimer = 0.0f;
    bIsTransitioning = false;

    SunLight = nullptr;
    WorldFog = nullptr;
}

void AWorld_BiomeWeatherManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeWeatherPresets();
    FindWorldLightingActors();
    
    // Set initial weather for savana biome
    SetWeatherForBiome(EWorld_BiomeType::Savana);
}

void AWorld_BiomeWeatherManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    WeatherTimer += DeltaTime;

    if (bIsTransitioning)
    {
        float TransitionAlpha = FMath::Clamp(WeatherTimer / WeatherTransitionSpeed, 0.0f, 1.0f);
        InterpolateWeatherState(TransitionAlpha);

        if (TransitionAlpha >= 1.0f)
        {
            bIsTransitioning = false;
            CurrentWeather = TargetWeather;
            WeatherTimer = 0.0f;
        }
    }
    else if (bEnableDynamicWeather && WeatherTimer >= WeatherUpdateInterval)
    {
        // Randomly adjust weather parameters slightly
        CurrentWeather.Temperature += FMath::RandRange(-2.0f, 2.0f);
        CurrentWeather.Humidity = FMath::Clamp(CurrentWeather.Humidity + FMath::RandRange(-0.1f, 0.1f), 0.0f, 1.0f);
        CurrentWeather.WindStrength = FMath::Clamp(CurrentWeather.WindStrength + FMath::RandRange(-0.1f, 0.1f), 0.0f, 1.0f);
        
        UpdateEnvironmentalEffects();
        WeatherTimer = 0.0f;
    }
}

void AWorld_BiomeWeatherManager::SetWeatherForBiome(EWorld_BiomeType BiomeType)
{
    FWorld_WeatherState NewWeather = GetBiomeDefaultWeather(BiomeType);
    TransitionToWeather(NewWeather);
}

void AWorld_BiomeWeatherManager::TransitionToWeather(const FWorld_WeatherState& NewTargetWeather)
{
    TargetWeather = NewTargetWeather;
    bIsTransitioning = true;
    WeatherTimer = 0.0f;
}

void AWorld_BiomeWeatherManager::UpdateEnvironmentalEffects()
{
    UpdateLightingSettings();
    UpdateFogSettings();
}

void AWorld_BiomeWeatherManager::StartRainEffect(float Intensity)
{
    CurrentWeather.RainIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    CurrentWeather.Humidity = FMath::Min(CurrentWeather.Humidity + 0.3f, 1.0f);
    CurrentWeather.FogDensity = FMath::Min(CurrentWeather.FogDensity + 0.2f, 1.0f);
    
    UpdateEnvironmentalEffects();
}

void AWorld_BiomeWeatherManager::StopRainEffect()
{
    CurrentWeather.RainIntensity = 0.0f;
    UpdateEnvironmentalEffects();
}

void AWorld_BiomeWeatherManager::UpdateFogSettings()
{
    if (WorldFog && WorldFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = WorldFog->GetComponent();
        
        float FogDensityValue = CurrentWeather.FogDensity * 0.02f;
        FogComponent->SetFogDensity(FogDensityValue);
        
        // Adjust fog color based on biome and weather
        FLinearColor FogColor = CurrentWeather.SkyTint;
        FogColor.A = 1.0f;
        FogComponent->SetFogInscatteringColor(FogColor);
    }
}

void AWorld_BiomeWeatherManager::UpdateLightingSettings()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        
        // Adjust light intensity based on weather conditions
        float BaseIntensity = 3.0f;
        float WeatherModifier = 1.0f - (CurrentWeather.RainIntensity * 0.4f) - (CurrentWeather.FogDensity * 0.3f);
        float FinalIntensity = BaseIntensity * FMath::Clamp(WeatherModifier, 0.3f, 1.0f);
        
        LightComponent->SetIntensity(FinalIntensity);
        
        // Adjust light color based on biome
        FLinearColor LightColor = CurrentWeather.SkyTint;
        LightColor.A = 1.0f;
        LightComponent->SetLightColor(LightColor);
    }
}

void AWorld_BiomeWeatherManager::InitializeWeatherPresets()
{
    BiomeWeatherPresets.Empty();

    // Savana weather
    FWorld_WeatherState SavanaWeather;
    SavanaWeather.BiomeType = EWorld_BiomeType::Savana;
    SavanaWeather.Temperature = 28.0f;
    SavanaWeather.Humidity = 0.4f;
    SavanaWeather.WindStrength = 0.6f;
    SavanaWeather.RainIntensity = 0.1f;
    SavanaWeather.FogDensity = 0.05f;
    SavanaWeather.SkyTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    BiomeWeatherPresets.Add(SavanaWeather);

    // Forest weather
    FWorld_WeatherState ForestWeather;
    ForestWeather.BiomeType = EWorld_BiomeType::Forest;
    ForestWeather.Temperature = 22.0f;
    ForestWeather.Humidity = 0.8f;
    ForestWeather.WindStrength = 0.3f;
    ForestWeather.RainIntensity = 0.3f;
    ForestWeather.FogDensity = 0.2f;
    ForestWeather.SkyTint = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);
    BiomeWeatherPresets.Add(ForestWeather);

    // Desert weather
    FWorld_WeatherState DesertWeather;
    DesertWeather.BiomeType = EWorld_BiomeType::Desert;
    DesertWeather.Temperature = 35.0f;
    DesertWeather.Humidity = 0.2f;
    DesertWeather.WindStrength = 0.8f;
    DesertWeather.RainIntensity = 0.0f;
    DesertWeather.FogDensity = 0.0f;
    DesertWeather.SkyTint = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    BiomeWeatherPresets.Add(DesertWeather);

    // Mountain weather
    FWorld_WeatherState MountainWeather;
    MountainWeather.BiomeType = EWorld_BiomeType::Mountain;
    MountainWeather.Temperature = 15.0f;
    MountainWeather.Humidity = 0.6f;
    MountainWeather.WindStrength = 0.9f;
    MountainWeather.RainIntensity = 0.2f;
    MountainWeather.FogDensity = 0.3f;
    MountainWeather.SkyTint = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    BiomeWeatherPresets.Add(MountainWeather);
}

void AWorld_BiomeWeatherManager::FindWorldLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }

    // Find exponential height fog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    if (FoundFogs.Num() > 0)
    {
        WorldFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }
}

void AWorld_BiomeWeatherManager::InterpolateWeatherState(float Alpha)
{
    CurrentWeather.Temperature = FMath::Lerp(CurrentWeather.Temperature, TargetWeather.Temperature, Alpha);
    CurrentWeather.Humidity = FMath::Lerp(CurrentWeather.Humidity, TargetWeather.Humidity, Alpha);
    CurrentWeather.WindStrength = FMath::Lerp(CurrentWeather.WindStrength, TargetWeather.WindStrength, Alpha);
    CurrentWeather.RainIntensity = FMath::Lerp(CurrentWeather.RainIntensity, TargetWeather.RainIntensity, Alpha);
    CurrentWeather.FogDensity = FMath::Lerp(CurrentWeather.FogDensity, TargetWeather.FogDensity, Alpha);
    CurrentWeather.SkyTint = FMath::Lerp(CurrentWeather.SkyTint, TargetWeather.SkyTint, Alpha);

    UpdateEnvironmentalEffects();
}

FWorld_WeatherState AWorld_BiomeWeatherManager::GetBiomeDefaultWeather(EWorld_BiomeType BiomeType)
{
    for (const FWorld_WeatherState& Preset : BiomeWeatherPresets)
    {
        if (Preset.BiomeType == BiomeType)
        {
            return Preset;
        }
    }

    // Return default savana weather if biome not found
    return BiomeWeatherPresets.Num() > 0 ? BiomeWeatherPresets[0] : FWorld_WeatherState();
}
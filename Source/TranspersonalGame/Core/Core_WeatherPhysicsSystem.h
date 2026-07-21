#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Eng_BiomeArchitecture.h"
#include "Core_WeatherPhysicsSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_WeatherType : uint8
{
    Clear UMETA(DisplayName = "Clear Sky"),
    Cloudy UMETA(DisplayName = "Cloudy"),
    LightRain UMETA(DisplayName = "Light Rain"),
    HeavyRain UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm UMETA(DisplayName = "Thunderstorm"),
    Fog UMETA(DisplayName = "Fog"),
    Sandstorm UMETA(DisplayName = "Sandstorm"),
    VolcanicAsh UMETA(DisplayName = "Volcanic Ash")
};

UENUM(BlueprintType)
enum class ECore_WindIntensity : uint8
{
    Calm UMETA(DisplayName = "Calm"),
    Light UMETA(DisplayName = "Light Breeze"),
    Moderate UMETA(DisplayName = "Moderate Wind"),
    Strong UMETA(DisplayName = "Strong Wind"),
    Gale UMETA(DisplayName = "Gale Force")
};

USTRUCT(BlueprintType)
struct FCore_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ECore_WeatherType WeatherType = ECore_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f; // 5 minutes default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float TransitionTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    ECore_WindIntensity WindIntensity = ECore_WindIntensity::Light;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Temperature = 25.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Humidity = 60.0f; // Percentage

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Visibility = 10000.0f; // Meters

    FCore_WeatherSettings()
    {
        WeatherType = ECore_WeatherType::Clear;
        Intensity = 0.5f;
        Duration = 300.0f;
        TransitionTime = 30.0f;
        WindIntensity = ECore_WindIntensity::Light;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        Temperature = 25.0f;
        Humidity = 60.0f;
        Visibility = 10000.0f;
    }
};

USTRUCT(BlueprintType)
struct FCore_WeatherEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float FogDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float CloudCoverage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float SunIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    float LightningFrequency = 0.0f;

    FCore_WeatherEffects()
    {
        FogDensity = 0.0f;
        FogColor = FLinearColor::White;
        CloudCoverage = 0.0f;
        SunIntensity = 1.0f;
        SunColor = FLinearColor::White;
        RainIntensity = 0.0f;
        WindStrength = 0.0f;
        LightningFrequency = 0.0f;
    }
};

/**
 * Core Weather Physics System - Realistic weather simulation for prehistoric world
 * Handles atmospheric effects, precipitation, wind physics, and environmental impact
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_WeatherPhysicsSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_WeatherPhysicsSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Weather control functions
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetWeather(const FCore_WeatherSettings& NewWeatherSettings);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void TransitionToWeather(const FCore_WeatherSettings& TargetWeather, float TransitionDuration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetBiomeSpecificWeather(EBiomeType BiomeType, const FCore_WeatherSettings& WeatherSettings);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    FCore_WeatherSettings GetCurrentWeather() const { return CurrentWeatherSettings; }

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void UpdateFogSettings(float Density, const FLinearColor& Color, float Height = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void UpdateSunLighting(float Intensity, const FLinearColor& Color, const FRotator& Direction);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void CreateRainEffect(float Intensity, const FVector& WindDirection);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void CreateThunderstormEffect(float LightningFrequency, float RainIntensity);

    // Wind physics
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetGlobalWind(const FVector& Direction, float Strength);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    FVector GetWindAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void ApplyWindToActor(AActor* Actor, float WindSensitivity = 1.0f);

    // Weather simulation
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void StartWeatherSimulation();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void StopWeatherSimulation();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void SetWeatherCycle(const TArray<FCore_WeatherSettings>& WeatherCycle, float CycleDuration = 1200.0f);

    // Biome integration
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void UpdateWeatherForBiomes();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    FCore_WeatherSettings GenerateWeatherForBiome(EBiomeType BiomeType) const;

    // Testing and validation
    UFUNCTION(BlueprintCallable, Category = "Weather Physics", CallInEditor)
    void CreateWeatherTestScenario();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics", CallInEditor)
    void ValidateWeatherSystems();

protected:
    // Current weather state
    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    FCore_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    FCore_WeatherSettings TargetWeatherSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    FCore_WeatherEffects CurrentEffects;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather State")
    float TransitionDuration = 30.0f;

    // Weather simulation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Simulation")
    bool bWeatherSimulationActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Simulation")
    TArray<FCore_WeatherSettings> WeatherCycle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Simulation")
    float CycleDuration = 1200.0f; // 20 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Simulation")
    int32 CurrentCycleIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Simulation")
    float CycleTimer = 0.0f;

    // Biome-specific weather settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TMap<EBiomeType, FCore_WeatherSettings> BiomeWeatherSettings;

    // Environmental actors
    UPROPERTY()
    TWeakObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY()
    TWeakObjectPtr<AExponentialHeightFog> FogActor;

    UPROPERTY()
    TWeakObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    // Weather effects
    UPROPERTY()
    TArray<TWeakObjectPtr<UNiagaraComponent>> RainEffects;

    UPROPERTY()
    TArray<TWeakObjectPtr<UAudioComponent>> WeatherSounds;

    // Wind system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    FVector GlobalWindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    float GlobalWindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Physics")
    TArray<TWeakObjectPtr<AActor>> WindAffectedActors;

private:
    // Internal update functions
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateWeatherCycle(float DeltaTime);
    void ApplyWeatherEffects();
    void UpdateEnvironmentalActors();
    void UpdateWindPhysics(float DeltaTime);

    // Effect creation helpers
    void CreateWeatherEffects(ECore_WeatherType WeatherType);
    void CleanupWeatherEffects();
    UNiagaraComponent* CreateRainParticles(float Intensity);
    UAudioComponent* CreateWeatherAudio(ECore_WeatherType WeatherType);

    // Biome integration
    void InitializeBiomeWeatherSettings();
    FCore_WeatherSettings GetDefaultWeatherForBiome(EBiomeType BiomeType) const;

    // Performance optimization
    void OptimizeWeatherEffects();
    bool ShouldUpdateWeatherForDistance(const FVector& Location) const;

    // Biome architecture reference
    UPROPERTY()
    TWeakObjectPtr<UEng_BiomeArchitecture> BiomeArchitecture;

    // Performance monitoring
    UPROPERTY()
    float LastWeatherUpdateTime;

    UPROPERTY()
    int32 ActiveWeatherEffects;
};
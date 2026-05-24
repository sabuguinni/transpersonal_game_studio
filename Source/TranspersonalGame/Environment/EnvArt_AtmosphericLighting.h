#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "EnvArt_AtmosphericLighting.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EEnvArt_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Foggy       UMETA(DisplayName = "Foggy"),
    Stormy      UMETA(DisplayName = "Stormy")
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-25.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric")
    float VolumetricScatteringIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FEnvArt_LightingSettings()
    {
        // Default values already set above
    }
};

USTRUCT(BlueprintType)
struct FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHeight = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float RayleighScattering = 0.0331f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float MieScattering = 0.003996f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudCoverage = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clouds")
    float CloudDensity = 0.8f;

    FEnvArt_AtmosphericSettings()
    {
        // Default values already set above
    }
};

/**
 * Component responsible for managing atmospheric lighting effects in prehistoric environments.
 * Handles dynamic time of day, weather systems, and atmospheric rendering for immersive survival gameplay.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_AtmosphericLighting : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_AtmosphericLighting();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === LIGHTING CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetWeatherState(EEnvArt_WeatherState NewWeatherState);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateLightingSettings(const FEnvArt_LightingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void UpdateAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    // === DYNAMIC LIGHTING ===

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void StartDynamicTimeOfDay(float DayDurationMinutes = 20.0f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void StopDynamicTimeOfDay();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Lighting")
    void SetTimeOfDayProgress(float Progress); // 0.0 = dawn, 1.0 = full cycle

    // === WEATHER EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void TransitionToWeather(EEnvArt_WeatherState TargetWeather, float TransitionDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateStormEffect();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void CreateFogEffect(float Intensity = 1.0f);

    // === ATMOSPHERIC PARTICLES ===

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SpawnDustParticles(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void SpawnPollenParticles(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void CreateGodRayEffect(FVector LightDirection);

    // === GETTERS ===

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    EEnvArt_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    EEnvArt_WeatherState GetCurrentWeatherState() const { return CurrentWeatherState; }

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    float GetTimeOfDayProgress() const { return TimeOfDayProgress; }

protected:
    // === CORE PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lighting Settings")
    FEnvArt_LightingSettings LightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Time of Day")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weather")
    EEnvArt_WeatherState CurrentWeatherState = EEnvArt_WeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Lighting")
    bool bDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Lighting")
    float DayDuration = 1200.0f; // 20 minutes in seconds

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Lighting")
    float TimeOfDayProgress = 0.5f; // 0.0 = dawn, 1.0 = full cycle

    // === ACTOR REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting Actors")
    AVolumetricCloud* VolumetricClouds;

    // === WEATHER TRANSITION ===

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    bool bWeatherTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    float WeatherTransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    float WeatherTransitionDuration = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Transition")
    EEnvArt_WeatherState TargetWeatherState = EEnvArt_WeatherState::Clear;

private:
    // === INTERNAL METHODS ===

    void FindLightingActors();
    void ApplyTimeOfDayLighting();
    void ApplyWeatherEffects();
    void UpdateDynamicTimeOfDay(float DeltaTime);
    void UpdateWeatherTransition(float DeltaTime);
    void InterpolateLightingSettings(const FEnvArt_LightingSettings& From, const FEnvArt_LightingSettings& To, float Alpha);
    FEnvArt_LightingSettings GetTimeOfDayLightingSettings(EEnvArt_TimeOfDay TimeOfDay);
    FEnvArt_LightingSettings GetWeatherLightingSettings(EEnvArt_WeatherState WeatherState);
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_DynamicWeatherController.generated.h"

UENUM(BlueprintType)
enum class EWorld_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Skies"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Overcast    UMETA(DisplayName = "Overcast"),
    LightRain   UMETA(DisplayName = "Light Rain"),
    HeavyRain   UMETA(DisplayName = "Heavy Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"),
    Fog         UMETA(DisplayName = "Dense Fog"),
    Mist        UMETA(DisplayName = "Morning Mist")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WeatherParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EWorld_WeatherType WeatherType = EWorld_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float WindStrength = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    FLinearColor SkyTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "15.0", ClampMax = "35.0"))
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.3", ClampMax = "0.9"))
    float Humidity = 0.6f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_DynamicWeatherController : public AActor
{
    GENERATED_BODY()

public:
    AWorld_DynamicWeatherController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather System Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Current Weather State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FWorld_WeatherParameters CurrentWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FWorld_WeatherParameters TargetWeather;

    // Weather Transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float WeatherTransitionTime = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    float CurrentTransitionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather System")
    bool bIsTransitioning = false;

    // Weather Cycle Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle")
    bool bEnableWeatherCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Cycle", meta = (ClampMin = "300.0", ClampMax = "3600.0"))
    float WeatherCycleDuration = 900.0f; // 15 minutes

    UPROPERTY(BlueprintReadOnly, Category = "Weather Cycle")
    float WeatherCycleTimer = 0.0f;

    // Biome-Specific Weather Patterns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TArray<EWorld_WeatherType> ForestWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TArray<EWorld_WeatherType> PlainsWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TArray<EWorld_WeatherType> MountainWeatherTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Weather")
    TArray<EWorld_WeatherType> CoastalWeatherTypes;

    // Environmental References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    class ASkyLight* SkyLightActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    class AExponentialHeightFog* FogActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    class UMaterialParameterCollection* WeatherParameterCollection;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetWeather(EWorld_WeatherType NewWeatherType, bool bInstantChange = false);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetCustomWeatherParameters(const FWorld_WeatherParameters& NewParameters, bool bInstantChange = false);

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void TriggerRandomWeather();

    UFUNCTION(BlueprintCallable, Category = "Weather Control")
    void SetBiomeSpecificWeather(EBiomeType BiomeType);

    // Weather Query Functions
    UFUNCTION(BlueprintPure, Category = "Weather Info")
    EWorld_WeatherType GetCurrentWeatherType() const { return CurrentWeather.WeatherType; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    FWorld_WeatherParameters GetCurrentWeatherParameters() const { return CurrentWeather; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    float GetRainIntensity() const { return CurrentWeather.RainIntensity; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    float GetWindStrength() const { return CurrentWeather.WindStrength; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    float GetTemperature() const { return CurrentWeather.Temperature; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    bool IsRaining() const { return CurrentWeather.RainIntensity > 0.1f; }

    UFUNCTION(BlueprintPure, Category = "Weather Info")
    bool IsStormy() const { return CurrentWeather.WeatherType == EWorld_WeatherType::Storm; }

    // Weather Cycle Control
    UFUNCTION(BlueprintCallable, Category = "Weather Cycle")
    void StartWeatherCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather Cycle")
    void StopWeatherCycle();

    UFUNCTION(BlueprintCallable, Category = "Weather Cycle")
    void ResetWeatherCycle();

protected:
    // Internal Weather Functions
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateWeatherCycle(float DeltaTime);
    void ApplyWeatherToEnvironment();
    void UpdateLighting();
    void UpdateFog();
    void UpdateMaterialParameters();
    EWorld_WeatherType SelectRandomWeatherForBiome(EBiomeType BiomeType);
    FWorld_WeatherParameters GetWeatherParametersForType(EWorld_WeatherType WeatherType);
    void InitializeWeatherArrays();
    void FindEnvironmentActors();

    // Interpolation Helpers
    float LerpWeatherFloat(float Current, float Target, float Alpha);
    FLinearColor LerpWeatherColor(const FLinearColor& Current, const FLinearColor& Target, float Alpha);
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Perf_WeatherPerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Cloudy      UMETA(DisplayName = "Cloudy"),
    Rainy       UMETA(DisplayName = "Rainy"),
    Stormy      UMETA(DisplayName = "Stormy"),
    Foggy       UMETA(DisplayName = "Foggy")
};

UENUM(BlueprintType)
enum class EPerf_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FPerf_WeatherPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxShadowDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowCascades = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FogDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AtmosphereUpdateRate = 0.1f;

    FPerf_WeatherPerformanceSettings()
    {
        MaxShadowDistance = 8000.0f;
        ShadowCascades = 3;
        FogDensityMultiplier = 1.0f;
        bEnableVolumetricFog = true;
        AtmosphereUpdateRate = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_WeatherPerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_WeatherPerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    FPerf_WeatherPerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather State")
    EPerf_WeatherState CurrentWeatherState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EPerf_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    float DayDurationMinutes = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 1.0f;

    // References to lighting actors
    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Lighting")
    ASkyLight* SkyLight;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

private:
    float TimeAccumulator;
    float PerformanceTimer;
    float CurrentTimeOfDayFloat; // 0.0 = midnight, 0.5 = noon

public:
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeForWeatherState(EPerf_WeatherState WeatherState);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeForTimeOfDay(EPerf_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void UpdatePerformanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetDayNightCycleSpeed(float SpeedMultiplier);

    UFUNCTION(BlueprintPure, Category = "Weather Performance")
    float GetCurrentPerformanceScore() const;

    UFUNCTION(BlueprintPure, Category = "Weather Performance")
    bool IsPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void FindLightingActors();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void ApplyLightingOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void MonitorFrameRate();

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Performance")
    void OnPerformanceThresholdReached(float CurrentFPS, float TargetFPS);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Performance")
    void OnWeatherStateChanged(EPerf_WeatherState NewWeatherState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Weather Performance")
    void OnTimeOfDayChanged(EPerf_TimeOfDay NewTimeOfDay);
};
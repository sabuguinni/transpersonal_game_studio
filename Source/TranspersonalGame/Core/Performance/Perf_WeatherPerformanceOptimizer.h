#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_WeatherPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Potato      UMETA(DisplayName = "Potato Mode")
};

USTRUCT(BlueprintType)
struct FPerf_WeatherPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float RainParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float FogDensity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float CloudComplexity;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float LightningEffectCost;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherTransitionCost;

    FPerf_WeatherPerformanceMetrics()
    {
        RainParticleCount = 0.0f;
        FogDensity = 0.0f;
        CloudComplexity = 0.0f;
        LightningEffectCost = 0.0f;
        WeatherTransitionCost = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_WeatherOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain Optimization")
    int32 MaxRainParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rain Optimization")
    float RainParticleLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Optimization")
    float MaxFogDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog Optimization")
    float FogUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Optimization")
    int32 MaxCloudLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cloud Optimization")
    float CloudLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Optimization")
    float LightningCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning Optimization")
    int32 MaxSimultaneousLightning;

    FPerf_WeatherOptimizationSettings()
    {
        MaxRainParticles = 5000;
        RainParticleLifetime = 3.0f;
        MaxFogDistance = 10000.0f;
        FogUpdateFrequency = 0.1f;
        MaxCloudLayers = 3;
        CloudLODDistance = 5000.0f;
        LightningCullingDistance = 8000.0f;
        MaxSimultaneousLightning = 2;
    }
};

/**
 * Performance optimizer specifically for weather systems in the prehistoric world.
 * Manages rain, fog, clouds, lightning and weather transitions to maintain 60fps.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_WeatherPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WeatherPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    EPerf_WeatherOptimizationLevel OptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    FPerf_WeatherOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    FPerf_WeatherPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float WeatherPerformanceBudget;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherSystems();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetOptimizationLevel(EPerf_WeatherOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeRainSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeFogSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeCloudSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeLightningSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherTransitions();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    FPerf_WeatherPerformanceMetrics GetWeatherPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    bool IsWeatherPerformanceWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void ApplyDynamicWeatherOptimization(float CurrentFrameTime);

private:
    void UpdatePerformanceMetrics();
    void ApplyOptimizationSettings();
    void CullDistantWeatherEffects();
    void AdjustParticleSystemLOD();
    void OptimizeWeatherShaders();

    float LastOptimizationTime;
    float OptimizationUpdateInterval;
    bool bIsOptimizationActive;
};
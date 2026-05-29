#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Perf_WeatherPhysicsPerformanceIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherPhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float RainDropletSimulationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float WindForceCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float WeatherCollisionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float AtmosphericPressureTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    int32 ActiveWeatherParticles;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float WeatherMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float WeatherGPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Physics Performance")
    float WeatherCPUTime;

    FPerf_WeatherPhysicsMetrics()
        : RainDropletSimulationTime(0.0f)
        , WindForceCalculationTime(0.0f)
        , WeatherCollisionTime(0.0f)
        , AtmosphericPressureTime(0.0f)
        , ActiveWeatherParticles(0)
        , WeatherMemoryUsage(0.0f)
        , WeatherGPUTime(0.0f)
        , WeatherCPUTime(0.0f)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_WeatherOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_WeatherPhysicsPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WeatherPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Physics Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    FPerf_WeatherPhysicsMetrics GetWeatherPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeWeatherPhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetWeatherFrameImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void EnableWeatherPhysicsLOD(bool bEnable);

    // Rain System Performance
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeRainDropletSimulation();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetRainParticleLimit(int32 MaxParticles);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetRainSimulationCost() const;

    // Wind System Performance
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeWindForceCalculations();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetWindCalculationFrequency(float Frequency);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetWindCalculationCost() const;

    // Weather Collision Performance
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeWeatherCollisions();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetWeatherCollisionComplexity(int32 Complexity);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetWeatherCollisionCost() const;

    // Atmospheric Physics Performance
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeAtmosphericPhysics();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetAtmosphericUpdateRate(float UpdateRate);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetAtmosphericPhysicsCost() const;

    // Weather Memory Management
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void OptimizeWeatherMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    float GetWeatherMemoryFootprint() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void ClearWeatherMemoryCache();

    // Performance Thresholds
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void SetWeatherPerformanceThresholds(float CPUThreshold, float GPUThreshold, float MemoryThreshold);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    bool IsWeatherPerformanceWithinThresholds() const;

    // Debug and Profiling
    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance", CallInEditor)
    void StartWeatherPhysicsProfiler();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance", CallInEditor)
    void StopWeatherPhysicsProfiler();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics Performance")
    void LogWeatherPhysicsPerformance() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    EPerf_WeatherOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    float WeatherCPUThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    float WeatherGPUThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    float WeatherMemoryThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    int32 MaxRainParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    float WindCalculationFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    int32 WeatherCollisionComplexity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    float AtmosphericUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    bool bWeatherPhysicsLODEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Physics Performance")
    bool bProfilerActive;

private:
    mutable FPerf_WeatherPhysicsMetrics CachedMetrics;
    float LastMetricsUpdateTime;
    
    void UpdateWeatherPhysicsMetrics() const;
    void ApplyOptimizationLevel();
    void MonitorWeatherPhysicsPerformance();
    void HandleWeatherPhysicsThresholdViolation();
};
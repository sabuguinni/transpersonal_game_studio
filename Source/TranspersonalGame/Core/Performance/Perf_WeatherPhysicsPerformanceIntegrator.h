#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Perf_WeatherPhysicsPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherSystemFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float RainDropletCount;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WindCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float AtmosphereUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float LightningEffectTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherPhysicsMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherGPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherCPUTime;

    FPerf_WeatherPerformanceMetrics()
    {
        WeatherSystemFrameTime = 0.0f;
        RainDropletCount = 0.0f;
        WindCalculationTime = 0.0f;
        AtmosphereUpdateTime = 0.0f;
        LightningEffectTime = 0.0f;
        WeatherPhysicsMemoryUsage = 0.0f;
        WeatherGPUTime = 0.0f;
        WeatherCPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    int32 MaxRainDroplets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WindUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float AtmosphereUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    bool bEnableLightningEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WeatherLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WeatherCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    bool bEnableWeatherPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WeatherPhysicsQuality;

    FPerf_WeatherOptimizationSettings()
    {
        MaxRainDroplets = 1000;
        WindUpdateFrequency = 30.0f;
        AtmosphereUpdateRate = 60.0f;
        bEnableLightningEffects = true;
        WeatherLODDistance = 5000.0f;
        WeatherCullingDistance = 10000.0f;
        bEnableWeatherPhysics = true;
        WeatherPhysicsQuality = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_WeatherPhysicsPerformanceIntegrator : public AActor
{
    GENERATED_BODY()

public:
    APerf_WeatherPhysicsPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void StartWeatherPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void StopWeatherPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    FPerf_WeatherPerformanceMetrics GetWeatherPerformanceMetrics() const;

    // Optimization Control
    UFUNCTION(BlueprintCallable, Category = "Weather Optimization")
    void SetWeatherOptimizationLevel(EPerf_WeatherOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Weather Optimization")
    void ApplyWeatherOptimizationSettings(const FPerf_WeatherOptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Weather Optimization")
    void OptimizeWeatherForFrameRate(float TargetFrameRate);

    // Weather Physics Integration
    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void IntegrateWithWeatherPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void UpdateWeatherPhysicsPerformance(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Weather Physics")
    void OptimizeWeatherPhysicsCalculations();

    // Adaptive Quality
    UFUNCTION(BlueprintCallable, Category = "Weather Adaptive")
    void EnableAdaptiveWeatherQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Weather Adaptive")
    void UpdateAdaptiveWeatherQuality(float CurrentFrameRate);

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather Debug")
    void RunWeatherPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather Debug")
    void LogWeatherPerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather Debug")
    void ResetWeatherPerformanceCounters();

protected:
    // Core Properties
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_WeatherOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings", meta = (AllowPrivateAccess = "true"))
    FPerf_WeatherOptimizationSettings OptimizationSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Metrics", meta = (AllowPrivateAccess = "true"))
    FPerf_WeatherPerformanceMetrics CurrentMetrics;

    // Performance Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    float WeatherFrameTimeAccumulator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    int32 WeatherFrameCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    float LastWeatherUpdateTime;

    // Adaptive Quality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Adaptive", meta = (AllowPrivateAccess = "true"))
    bool bAdaptiveQualityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Adaptive", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Adaptive", meta = (AllowPrivateAccess = "true"))
    float FrameRateTolerance;

    // Integration References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather Integration", meta = (AllowPrivateAccess = "true"))
    class ACore_WeatherPhysicsSystem* WeatherPhysicsSystem;

private:
    // Internal Methods
    void UpdateWeatherPerformanceMetrics(float DeltaTime);
    void ApplyOptimizationLevel(EPerf_WeatherOptimizationLevel Level);
    void CheckWeatherPhysicsIntegration();
    void OptimizeWeatherBasedOnPerformance();
    void UpdateWeatherQualitySettings();

    // Performance Counters
    double WeatherStartTime;
    double WeatherEndTime;
    int32 TotalWeatherFrames;
    float AverageWeatherFrameTime;
};
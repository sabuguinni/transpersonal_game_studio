#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Perf_WeatherPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherOptimizationLevel : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct FPerf_WeatherMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float ParticleSystemFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    int32 ActiveParticleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherUpdateFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float VolumetricFogFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WindSimulationFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    int32 WeatherEffectLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float TotalWeatherFrameTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FPerf_WeatherOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    EPerf_WeatherOptimizationLevel OptimizationLevel = EPerf_WeatherOptimizationLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float MaxParticleSystemFrameTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    int32 MaxActiveParticles = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WeatherLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    bool bEnableVolumetricFogOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    bool bEnableWindSimulationOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Optimization")
    float WeatherUpdateFrequency = 30.0f;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_WeatherPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WeatherPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherSystems();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void UpdateWeatherMetrics();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    FPerf_WeatherMetrics GetWeatherMetrics() const { return WeatherMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetOptimizationLevel(EPerf_WeatherOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeParticleSystems();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWindSimulation();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Weather Performance")
    void RunWeatherPerformanceTest();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    FPerf_WeatherOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    FPerf_WeatherMetrics WeatherMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float LastOptimizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    bool bIsOptimizationActive = false;

private:
    void UpdateParticleSystemMetrics();
    void UpdateVolumetricFogMetrics();
    void UpdateWindSimulationMetrics();
    void ApplyOptimizationSettings();
    void LogWeatherPerformanceData();
};
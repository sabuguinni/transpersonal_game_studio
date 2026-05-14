#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Perf_WeatherPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FPerf_WeatherPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float ParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameImpact;

    FPerf_WeatherPerformanceMetrics()
    {
        ParticleCount = 0.0f;
        RenderTime = 0.0f;
        GPUMemoryUsage = 0.0f;
        FrameImpact = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_WeatherPerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_WeatherPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Weather performance optimization
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetWeatherQuality(EPerf_WeatherQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    FPerf_WeatherPerformanceMetrics GetWeatherPerformanceMetrics() const;

    // Rain optimization
    UFUNCTION(BlueprintCallable, Category = "Rain Performance")
    void OptimizeRainEffects(float TargetFPS = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "Rain Performance")
    void SetRainParticleCount(int32 ParticleCount);

    // Wind optimization
    UFUNCTION(BlueprintCallable, Category = "Wind Performance")
    void OptimizeWindEffects();

    UFUNCTION(BlueprintCallable, Category = "Wind Performance")
    void SetWindQuality(EPerf_WeatherQuality Quality);

    // Fog optimization
    UFUNCTION(BlueprintCallable, Category = "Fog Performance")
    void OptimizeFogEffects();

    UFUNCTION(BlueprintCallable, Category = "Fog Performance")
    void SetFogDensity(float Density);

    // Lightning optimization
    UFUNCTION(BlueprintCallable, Category = "Lightning Performance")
    void OptimizeLightningEffects();

    UFUNCTION(BlueprintCallable, Category = "Lightning Performance")
    void SetLightningQuality(EPerf_WeatherQuality Quality);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    EPerf_WeatherQuality CurrentWeatherQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float MaxParticleCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float MaxRenderTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_WeatherPerformanceMetrics CurrentMetrics;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float LowQualityParticleMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MediumQualityParticleMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float HighQualityParticleMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float UltraQualityParticleMultiplier;

private:
    void UpdatePerformanceMetrics();
    void ApplyQualitySettings();
    void OptimizeParticleSystems();
    void OptimizeMaterialParameters();
    void MonitorFrameRate();

    float LastFrameTime;
    float AverageFrameTime;
    int32 FrameCounter;
};
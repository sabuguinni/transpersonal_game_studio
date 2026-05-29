#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Perf_WeatherPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherComplexity : uint8
{
    Minimal     UMETA(DisplayName = "Minimal"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct FPerf_WeatherPerformanceSettings
{
    GENERATED_BODY()

    // Particle system settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    int32 MaxRainParticles = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    int32 MaxSnowParticles = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particles")
    int32 MaxFogParticles = 200;

    // Audio settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float WeatherAudioDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float WeatherAudioVolume = 1.0f;

    // Lighting settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableDynamicWeatherLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightingUpdateRate = 0.1f;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    FPerf_WeatherPerformanceSettings()
    {
        MaxRainParticles = 1000;
        MaxSnowParticles = 500;
        MaxFogParticles = 200;
        WeatherAudioDistance = 2000.0f;
        WeatherAudioVolume = 1.0f;
        bEnableDynamicWeatherLighting = true;
        LightingUpdateRate = 0.1f;
        MinFrameRate = 30.0f;
        TargetFrameRate = 60.0f;
    }
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
    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherSystems();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetWeatherComplexity(EPerf_WeatherComplexity Complexity);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void AdjustParticleCount(int32 NewParticleCount);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherAudio();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherLighting();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    bool IsPerformanceAcceptable() const;

    // Weather system monitoring
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void MonitorWeatherPerformance();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void LogWeatherPerformanceMetrics();

    // Dynamic optimization
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void EnableDynamicOptimization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void UpdatePerformanceSettings();

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_WeatherPerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_WeatherComplexity CurrentComplexity;

    // Dynamic optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Optimization")
    bool bDynamicOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Optimization")
    float PerformanceCheckInterval;

    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance Tracking")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Tracking")
    float AverageFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Tracking")
    int32 ActiveParticleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Tracking")
    float WeatherRenderTime;

private:
    // Internal optimization functions
    void OptimizeRainSystem();
    void OptimizeSnowSystem();
    void OptimizeFogSystem();
    void OptimizeWindSystem();
    void AdjustLODSettings();
    void UpdateParticleSettings();
    void UpdateAudioSettings();
    void UpdateLightingSettings();

    // Performance monitoring
    void CalculateFrameRate(float DeltaTime);
    void CheckPerformanceThresholds();
    void ApplyPerformanceAdjustments();

    // Internal state
    float FrameTimeAccumulator;
    int32 FrameCount;
    float LastPerformanceCheck;
    TArray<float> FrameTimeHistory;
    
    // Weather system references
    UPROPERTY()
    TArray<UParticleSystemComponent*> WeatherParticleSystems;
    
    UPROPERTY()
    TArray<UAudioComponent*> WeatherAudioComponents;
    
    UPROPERTY()
    UMaterialParameterCollection* WeatherMaterialCollection;
};
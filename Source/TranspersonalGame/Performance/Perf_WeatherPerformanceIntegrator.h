#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_WeatherPerformanceIntegrator.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherPerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float RainParticleCount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float FogDensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float LightningEffectCount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WindSimulationComplexity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance")
    float WeatherRenderCost = 0.0f;

    FPerf_WeatherPerformanceMetrics()
    {
        RainParticleCount = 0.0f;
        FogDensity = 0.0f;
        LightningEffectCount = 0.0f;
        WindSimulationComplexity = 0.0f;
        WeatherRenderCost = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_WeatherPerformanceIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WeatherPerformanceIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Weather Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_WeatherPerformanceLevel PerformanceLevel = EPerf_WeatherPerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    float MaxWeatherRenderBudget = 5.0f; // milliseconds

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_WeatherPerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance", meta = (AllowPrivateAccess = "true"))
    float PerformanceCheckInterval = 1.0f;

    // Weather Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Integration", meta = (AllowPrivateAccess = "true"))
    bool bEnableWeatherPhysicsIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Integration", meta = (AllowPrivateAccess = "true"))
    bool bEnableTerrainWeatherInteraction = true;

public:
    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherPerformance();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetPerformanceLevel(EPerf_WeatherPerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    FPerf_WeatherPerformanceMetrics GetWeatherPerformanceMetrics() const;

    // Weather System Integration
    UFUNCTION(BlueprintCallable, Category = "Weather Integration")
    void IntegrateWithPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Integration")
    void IntegrateWithTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Integration")
    void UpdateWeatherEffectLOD(float DistanceToPlayer);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    bool IsWeatherPerformanceOptimal() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    float GetWeatherRenderCost() const;

private:
    float TimeSinceLastPerformanceCheck = 0.0f;
    float CurrentFrameTime = 0.0f;

    void UpdatePerformanceMetrics();
    void AdjustWeatherQuality();
    void OptimizeParticleEffects();
    void OptimizeLightingEffects();
    void OptimizeVolumetricFog();
};
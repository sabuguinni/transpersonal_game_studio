#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Perf_FluidPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_FluidQuality : uint8
{
    Ultra UMETA(DisplayName = "Ultra"),
    High UMETA(DisplayName = "High"),
    Medium UMETA(DisplayName = "Medium"),
    Low UMETA(DisplayName = "Low"),
    Disabled UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPerf_FluidSimulationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    int32 MaxFluidParticles = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    float FluidUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    float ParticleLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    bool bEnableFluidCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    float FluidViscosity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Simulation")
    float FluidDensity = 1000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_WaterRenderingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    bool bEnableWaterReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    bool bEnableWaterRefractions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    int32 WaterReflectionResolution = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    float WaterReflectionDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    bool bEnableWaterCaustics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Rendering")
    float WaterTessellationLevel = 1.0f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_FluidPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_FluidPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_FluidQuality FluidQuality = EPerf_FluidQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_FluidSimulationSettings SimulationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_WaterRenderingSettings RenderingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float FluidSimulationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    int32 ActiveFluidParticles = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float FluidMemoryUsageMB = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    int32 VisibleWaterBodies = 0;

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void OptimizeFluidForTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void SetFluidQuality(EPerf_FluidQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void ApplySimulationSettings(const FPerf_FluidSimulationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void ApplyRenderingSettings(const FPerf_WaterRenderingSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void UpdateFluidPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    float GetFluidRenderCost() const;

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void OptimizeWaterMaterials();

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void EnableDynamicFluidLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void CullDistantWaterBodies(float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Fluid Performance")
    void ReduceFluidParticleCount(float ReductionFactor);

private:
    UPROPERTY()
    TArray<AActor*> CachedWaterActors;

    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 0.5f;
    bool bDynamicLODEnabled = true;
    float CurrentParticleReduction = 1.0f;

    void FindAllWaterActors();
    void ApplyQualitySettings(EPerf_FluidQuality Quality);
    void UpdateFluidLODBasedOnPerformance();
    void MonitorFluidMemoryUsage();
    void OptimizeFluidSimulationRate();
    void UpdateWaterReflectionQuality();
};
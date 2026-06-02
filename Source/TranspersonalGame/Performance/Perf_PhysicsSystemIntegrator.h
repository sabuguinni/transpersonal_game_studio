#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_PhysicsSystemIntegrator.generated.h"

// Forward declarations
class UPerf_PhysicsOptimizer;
class UPerf_PhysicsPerformanceMonitor;

UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct FPerf_PhysicsPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float CurrentFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CurrentPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float AveragePhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float PeakPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CulledPhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly)
    EPerf_PhysicsOptimizationLevel OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;

    UPROPERTY(BlueprintReadOnly)
    float PerformanceScore = 1.0f;
};

/**
 * Physics System Integrator - Coordinates physics optimization across all systems
 * Integrates with Core Physics systems to maintain 60fps performance target
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsSystemIntegrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsSystemIntegrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFrameTime = 16.67f; // 60 FPS in milliseconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float PhysicsTimeThreshold = 5.0f; // Max physics time per frame in ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxPhysicsObjects = 500;

    // Optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableComplexityReduction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnablePerformanceLogging = true;

    // Distance thresholds for LOD
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
    float LODDistance1 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distance Culling")
    float LODDistance2 = 2500.0f;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AveragePhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float PeakPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 CulledPhysicsObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    EPerf_PhysicsOptimizationLevel OptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;

private:
    // Component references
    UPROPERTY()
    UPerf_PhysicsOptimizer* PhysicsOptimizer = nullptr;

    UPROPERTY()
    UPerf_PhysicsPerformanceMonitor* PhysicsMonitor = nullptr;

    // Performance history
    TArray<float> PhysicsTimeHistory;
    TArray<float> FrameTimeHistory;
    int32 HistoryIndex = 0;

    // Timing
    double LastUpdateTime = 0.0;

    // Internal methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void OptimizePhysicsPerformance();
    void ApplyDistanceCulling();
    void ApplyComplexityReduction();
    void ApplyAdaptiveOptimization();
    void UpdateOptimizationLevel();
    void ReducePhysicsComplexity(float ComplexityFactor);
    float CalculatePerformanceScore() const;
    void CountPhysicsObjects();
    void LogPerformanceData();

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetPerformanceTargets(float NewTargetFrameTime, float NewPhysicsThreshold, int32 NewMaxObjects);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    FPerf_PhysicsPerformanceData GetPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    EPerf_PhysicsOptimizationLevel GetOptimizationLevel() const { return OptimizationLevel; }

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    float GetCurrentPerformanceScore() const { return CalculatePerformanceScore(); }
};
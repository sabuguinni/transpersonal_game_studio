#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "SharedTypes.h"
#include "Perf_SurvivalPhysicsOptimizer.generated.h"

// Forward declarations
class UCore_SurvivalPhysicsComponent;

/**
 * Performance optimization states for survival physics
 */
UENUM(BlueprintType)
enum class EPerf_PhysicsOptimizationLevel : uint8
{
    None        UMETA(DisplayName = "No Optimization"),
    Low         UMETA(DisplayName = "Low Optimization"),
    Medium      UMETA(DisplayName = "Medium Optimization"),
    High        UMETA(DisplayName = "High Optimization"),
    Extreme     UMETA(DisplayName = "Extreme Optimization")
};

/**
 * Performance metrics for survival physics calculations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SurvivalPhysicsMetrics
{
    GENERATED_BODY()

    // Timing metrics (in milliseconds)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PeakFrameTime;

    // Component counts
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveSurvivalComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 OptimizedComponents;

    // Performance flags
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceCritical;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bRequiresOptimization;

    FPerf_SurvivalPhysicsMetrics()
    {
        CalculationTime = 0.0f;
        AverageFrameTime = 0.0f;
        PeakFrameTime = 0.0f;
        ActiveSurvivalComponents = 0;
        OptimizedComponents = 0;
        bIsPerformanceCritical = false;
        bRequiresOptimization = false;
    }
};

/**
 * Optimization settings for survival physics performance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    // Performance thresholds (in milliseconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float WarningFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float OptimalFrameTime;

    // Optimization parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float DistanceBasedOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODOptimizationFactor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveComponents;

    FPerf_OptimizationSettings()
    {
        CriticalFrameTime = 33.33f;  // 30fps threshold
        WarningFrameTime = 20.0f;    // 50fps threshold
        OptimalFrameTime = 16.67f;   // 60fps threshold
        DistanceBasedOptimization = 5000.0f;
        LODOptimizationFactor = 0.5f;
        MaxActiveComponents = 100;
    }
};

/**
 * Performance optimizer for survival physics calculations
 * Monitors and optimizes Core_SurvivalPhysicsComponent performance
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_SurvivalPhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_SurvivalPhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_SurvivalPhysicsMetrics GetCurrentMetrics() const;

    // Optimization control
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeSurvivalPhysics();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ResetOptimizations();

    // Component management
    UFUNCTION(BlueprintCallable, Category = "Management")
    void RegisterSurvivalComponent(UCore_SurvivalPhysicsComponent* Component);

    UFUNCTION(BlueprintCallable, Category = "Management")
    void UnregisterSurvivalComponent(UCore_SurvivalPhysicsComponent* Component);

    // Performance analysis
    UFUNCTION(BlueprintCallable, Category = "Analysis")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Analysis")
    float GetAverageCalculationTime() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Testing")
    void RunPerformanceTest();

protected:
    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_SurvivalPhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EPerf_PhysicsOptimizationLevel CurrentOptimizationLevel;

    // Monitoring state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MonitoringStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    int32 FrameCount;

    // Component tracking
    UPROPERTY()
    TArray<UCore_SurvivalPhysicsComponent*> RegisteredComponents;

    UPROPERTY()
    TArray<UCore_SurvivalPhysicsComponent*> OptimizedComponents;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyDistanceBasedOptimization();
    void ApplyLODOptimization();
    void ApplyComponentLimiting();
    
    // Performance calculation helpers
    float CalculateFrameTime() const;
    void UpdateComponentCounts();
    bool ShouldOptimizeComponent(UCore_SurvivalPhysicsComponent* Component) const;
    
    // Timing tracking
    double LastFrameTime;
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameHistory = 60;
};
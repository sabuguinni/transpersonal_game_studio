#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"
#include "Perf_VehiclePhysicsOptimizer.generated.h"

// Forward declarations
class UCore_VehiclePhysics;

/**
 * Performance metrics for vehicle physics optimization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float VehiclePhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveVehicleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPerformanceTargetMet = true;

    FPerf_VehiclePerformanceMetrics()
    {
        AverageFrameTime = 16.67f; // 60 FPS target
        VehiclePhysicsTime = 0.0f;
        ActiveVehicleCount = 0;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        bPerformanceTargetMet = true;
    }
};

/**
 * Vehicle physics optimization settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehicleOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float MaxVehicleDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxActiveVehicles = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceThreshold = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    FPerf_VehicleOptimizationSettings()
    {
        MaxVehicleDistance = 5000.0f;
        MaxActiveVehicles = 10;
        PhysicsUpdateRate = 60.0f;
        LODDistanceThreshold = 2000.0f;
        bEnableDistanceCulling = true;
        bEnableLODOptimization = true;
    }
};

/**
 * Vehicle Physics Performance Optimizer
 * Optimizes vehicle physics performance for 60fps target on PC, 30fps on console
 * Handles LOD, culling, and dynamic quality scaling for vehicle systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_VehiclePhysicsOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_VehiclePhysicsOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_VehiclePerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const { return CurrentMetrics.bPerformanceTargetMet; }

    // Vehicle optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationSettings(const FPerf_VehicleOptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void EnableVehicleLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetMaxActiveVehicles(int32 MaxCount);

    // Distance-based optimization
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void UpdateVehicleDistances();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void CullDistantVehicles();

    // Quality scaling
    UFUNCTION(BlueprintCallable, Category = "Quality")
    void ScaleVehicleQuality(float QualityScale);

    UFUNCTION(BlueprintCallable, Category = "Quality")
    void SetVehiclePhysicsUpdateRate(float UpdateRate);

    // Debug and testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogCurrentPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ResetOptimizations();

protected:
    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_VehiclePerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_VehicleOptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bMonitoringActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MonitoringDuration = 0.0f;

    // Vehicle tracking
    UPROPERTY(BlueprintReadOnly, Category = "Vehicles")
    TArray<AActor*> TrackedVehicles;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicles")
    TArray<float> VehicleDistances;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxVehiclePhysicsTime = 5.0f; // 5ms budget

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targets")
    float MaxMemoryUsageMB = 100.0f;

private:
    // Internal optimization methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void OptimizeVehiclesByDistance();
    void OptimizeVehiclesByPerformance();
    void UpdateVehicleLOD();
    void ManageVehiclePhysicsRate();
    
    // Utility methods
    float CalculateDistanceToPlayer(AActor* Vehicle) const;
    void SetVehicleOptimizationLevel(AActor* Vehicle, int32 Level);
    bool ShouldCullVehicle(AActor* Vehicle) const;
    
    // Performance tracking
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;
    float LastOptimizationTime = 0.0f;
    
    // Console variables for runtime adjustment
    static TAutoConsoleVariable<float> CVarVehicleMaxDistance;
    static TAutoConsoleVariable<int32> CVarMaxActiveVehicles;
    static TAutoConsoleVariable<float> CVarVehiclePhysicsRate;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_VehiclePhysicsPerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float PhysicsUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float SuspensionCalculationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float TirePhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float CollisionDetectionTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    int32 ActiveVehicleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    int32 PhysicsBodyCount;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    float CPUUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Metrics")
    bool bIsPerformanceOptimal;

    FPerf_VehiclePhysicsMetrics()
    {
        AverageFrameTime = 0.0f;
        PhysicsUpdateTime = 0.0f;
        SuspensionCalculationTime = 0.0f;
        TirePhysicsTime = 0.0f;
        CollisionDetectionTime = 0.0f;
        ActiveVehicleCount = 0;
        PhysicsBodyCount = 0;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
        bIsPerformanceOptimal = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehicleOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    float MaxPhysicsUpdateRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    float LODDistanceThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    int32 MaxActiveVehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    bool bEnableAdaptiveLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    bool bEnablePhysicsCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    bool bEnableAsyncPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Optimization")
    float TargetFrameRate;

    FPerf_VehicleOptimizationSettings()
    {
        MaxPhysicsUpdateRate = 60.0f;
        LODDistanceThreshold = 2000.0f;
        MaxActiveVehicles = 10;
        bEnableAdaptiveLOD = true;
        bEnablePhysicsCulling = true;
        CullingDistance = 5000.0f;
        bEnableAsyncPhysics = true;
        TargetFrameRate = 60.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_VehicleOptimizationLevel : uint8
{
    Ultra      UMETA(DisplayName = "Ultra"),
    High       UMETA(DisplayName = "High"),
    Medium     UMETA(DisplayName = "Medium"),
    Low        UMETA(DisplayName = "Low"),
    Minimal    UMETA(DisplayName = "Minimal")
};

/**
 * Advanced Vehicle Physics Performance Optimizer
 * Monitors and optimizes vehicle physics performance in real-time
 * Provides adaptive LOD, culling, and quality scaling for vehicle systems
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_VehiclePhysicsPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_VehiclePhysicsPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE PERFORMANCE MONITORING ===
    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    FPerf_VehiclePhysicsMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Vehicle Performance")
    void UpdatePerformanceMetrics();

    // === OPTIMIZATION CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Vehicle Optimization")
    void SetOptimizationLevel(EPerf_VehicleOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Optimization")
    void ApplyOptimizationSettings(const FPerf_VehicleOptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Optimization")
    void OptimizeVehiclePhysics();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Optimization")
    void ResetOptimizations();

    // === ADAPTIVE SYSTEMS ===
    UFUNCTION(BlueprintCallable, Category = "Vehicle Adaptive")
    void UpdateAdaptiveLOD();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Adaptive")
    void UpdatePhysicsCulling();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Adaptive")
    void AdjustPhysicsUpdateRate(float TargetFrameTime);

    // === PROFILING AND ANALYSIS ===
    UFUNCTION(BlueprintCallable, Category = "Vehicle Profiling")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Profiling")
    void SavePerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Profiling")
    TArray<FString> GetPerformanceReport() const;

protected:
    // === CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_VehicleOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_VehicleOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableRealTimeOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    float MetricsUpdateInterval;

    // === PERFORMANCE METRICS ===
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_VehiclePhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_VehiclePhysicsMetrics> MetricsHistory;

    // === RUNTIME STATE ===
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfiling;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Performance", meta = (AllowPrivateAccess = "true"))
    int32 FrameCounter;

    // === TIMERS ===
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle OptimizationTimer;

    // === INTERNAL METHODS ===
    void UpdateVehicleMetrics();
    void CalculatePhysicsPerformance();
    void ApplyLODOptimizations();
    void ApplyCullingOptimizations();
    void ApplyQualityOptimizations();
    void LogPerformanceData();
    
    // === UTILITY METHODS ===
    TArray<AActor*> GetAllVehicleActors() const;
    float CalculateAverageFrameTime() const;
    float GetPhysicsUpdateTime() const;
    float GetMemoryUsage() const;
    float GetCPUUsage() const;
    bool IsPerformanceOptimal() const;
};
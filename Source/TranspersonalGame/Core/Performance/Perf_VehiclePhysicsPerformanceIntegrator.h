#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Perf_VehiclePhysicsPerformanceIntegrator.generated.h"

// Vehicle Physics Performance Metrics Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_VehiclePhysicsMetrics
{
    GENERATED_BODY()

    // Core Performance Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehiclePhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehicleSimulationCost;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    int32 ActiveVehicleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehiclePhysicsMemoryUsage;

    // Advanced Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehicleCollisionCost;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehicleWheelPhysicsCost;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehicleEngineSimulationCost;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehicleSuspensionCost;

    // Quality Scaling Metrics
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    float VehiclePhysicsQualityScale;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    bool bVehiclePhysicsOptimizationActive;

    FPerf_VehiclePhysicsMetrics()
    {
        VehiclePhysicsFrameTime = 0.0f;
        VehicleSimulationCost = 0.0f;
        ActiveVehicleCount = 0;
        VehiclePhysicsMemoryUsage = 0.0f;
        VehicleCollisionCost = 0.0f;
        VehicleWheelPhysicsCost = 0.0f;
        VehicleEngineSimulationCost = 0.0f;
        VehicleSuspensionCost = 0.0f;
        VehiclePhysicsQualityScale = 1.0f;
        bVehiclePhysicsOptimizationActive = false;
    }
};

// Vehicle Physics Optimization Level
UENUM(BlueprintType)
enum class EPerf_VehiclePhysicsOptimizationLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

/**
 * Vehicle Physics Performance Integrator
 * Monitors and optimizes vehicle physics performance in real-time
 * Integrates with Core Systems Programmer's Vehicle Physics System
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_VehiclePhysicsPerformanceIntegrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_VehiclePhysicsPerformanceIntegrator();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void StartVehiclePhysicsMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void StopVehiclePhysicsMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    FPerf_VehiclePhysicsMetrics GetVehiclePhysicsMetrics() const;

    // Optimization Control
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void SetVehiclePhysicsOptimizationLevel(EPerf_VehiclePhysicsOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void OptimizeVehiclePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void EnableAdaptiveVehiclePhysicsOptimization(bool bEnable);

    // Integration with Vehicle Physics System
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void RegisterVehiclePhysicsSystem(class ACore_VehiclePhysicsSystem* VehicleSystem);

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void UnregisterVehiclePhysicsSystem(class ACore_VehiclePhysicsSystem* VehicleSystem);

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void AnalyzeVehiclePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    bool IsVehiclePhysicsPerformanceOptimal() const;

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Vehicle Physics Performance")
    void RunVehiclePhysicsPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "Vehicle Physics Performance")
    void LogVehiclePhysicsPerformanceReport();

protected:
    // Core Properties
    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    FPerf_VehiclePhysicsMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    EPerf_VehiclePhysicsOptimizationLevel CurrentOptimizationLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    bool bIsMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Vehicle Physics Performance")
    bool bAdaptiveOptimizationEnabled;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics Performance", meta = (AllowPrivateAccess = "true"))
    float TargetVehiclePhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics Performance", meta = (AllowPrivateAccess = "true"))
    float MaxVehicleSimulationCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Physics Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveVehicles;

    // Registered Vehicle Systems
    UPROPERTY()
    TArray<class ACore_VehiclePhysicsSystem*> RegisteredVehicleSystems;

    // Timer Handles
    FTimerHandle MonitoringTimerHandle;
    FTimerHandle OptimizationTimerHandle;

private:
    // Internal Methods
    void UpdateVehiclePhysicsMetrics();
    void ApplyVehiclePhysicsOptimizations();
    void CheckAdaptiveOptimization();
    void OptimizeVehicleCollisionSettings();
    void OptimizeVehicleWheelPhysics();
    void OptimizeVehicleEngineSimulation();
    void OptimizeVehicleSuspensionSettings();
    
    // Performance Tracking
    float LastFrameTime;
    float AccumulatedFrameTime;
    int32 FrameCount;
    
    // Optimization State
    bool bOptimizationInProgress;
    float LastOptimizationTime;
};
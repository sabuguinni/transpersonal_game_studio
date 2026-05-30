#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_IntegratedPerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_IntegratedPerformanceData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsObjectCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTickTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsOptimizationActive = false;

    FPerf_IntegratedPerformanceData()
    {
        CurrentFPS = 60.0f;
        TargetFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        ActiveActorCount = 0;
        PhysicsObjectCount = 0;
        PhysicsTickTime = 0.0f;
        RenderThreadTime = 0.0f;
        bIsOptimizationActive = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFPSThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryThresholdMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsTickTime = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMemoryOptimization = true;

    FPerf_OptimizationSettings()
    {
        MinFPSThreshold = 30.0f;
        MaxMemoryThresholdMB = 4096.0f;
        MaxPhysicsObjects = 1000;
        MaxPhysicsTickTime = 16.0f;
        bEnableAutomaticOptimization = true;
        bEnableLODOptimization = true;
        bEnablePhysicsOptimization = true;
        bEnableMemoryOptimization = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_IntegratedPerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_IntegratedPerformanceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_IntegratedPerformanceData GetCurrentPerformanceData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceOptimizationPass();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceCounters();

    // Optimization control functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePhysicsOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableMemoryOptimization(bool bEnable);

    // Performance analysis functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActivePhysicsObjectCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    // Diagnostic functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceDiagnostic();

protected:
    // Core performance data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_IntegratedPerformanceData PerformanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_OptimizationSettings OptimizationSettings;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringStartTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastOptimizationTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TArray<float> FPSSamples;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxFPSSamples = 100;

    // Optimization state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bLODOptimizationEnabled = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsOptimizationEnabled = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bMemoryOptimizationEnabled = true;

private:
    // Internal performance monitoring
    void UpdatePerformanceMetrics(float DeltaTime);
    void CheckOptimizationThresholds();
    void ApplyOptimizations();
    void OptimizeLOD();
    void OptimizePhysics();
    void OptimizeMemory();
    
    // Performance calculation helpers
    float CalculateCurrentFPS(float DeltaTime);
    float CalculateMemoryUsage();
    int32 CountActivePhysicsObjects();
    float GetPhysicsTickTime();
    float GetRenderThreadTime();
};
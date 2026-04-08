#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Components/ActorComponent.h"
#include "PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameTime;

    UPROPERTY(BlueprintReadOnly)
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly)
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly)
    float ConsciousnessSystemTime;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConsciousnessEntities;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsObjects;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    float GPUMemoryUsageMB;

    FPerformanceMetrics()
    {
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        PhysicsTime = 0.0f;
        ConsciousnessSystemTime = 0.0f;
        ActiveConsciousnessEntities = 0;
        PhysicsObjects = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceAlert, const FString&, AlertMessage);

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceSnapshot(const FString& Label);

    // Consciousness System Specific Profiling
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void BeginConsciousnessProfile(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EndConsciousnessProfile(const FString& SystemName);

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(float TargetFPS, float MaxPhysicsTime, float MaxConsciousnessTime);

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnPerformanceAlert OnPerformanceAlert;

protected:
    // Performance tracking
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bIsProfilingEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float ProfilingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bAutoOptimizationEnabled;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxAllowedPhysicsTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MaxAllowedConsciousnessTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Targets")
    float MemoryWarningThresholdMB;

private:
    // Internal tracking
    FPerformanceMetrics CurrentMetrics;
    TMap<FString, double> ConsciousnessProfileTimes;
    TMap<FString, double> ConsciousnessProfileStartTimes;
    
    float LastProfilingUpdate;
    TArray<FPerformanceMetrics> MetricsHistory;
    
    // Performance optimization
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();
    void OptimizePhysicsSettings();
    void OptimizeConsciousnessSystem();
    void OptimizeRenderingSettings();
    
    // Logging and reporting
    void WritePerformanceLog();
    FString FormatMetricsForLog(const FPerformanceMetrics& Metrics) const;
};
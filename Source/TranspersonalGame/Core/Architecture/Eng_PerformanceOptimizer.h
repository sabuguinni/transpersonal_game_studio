#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float TargetFPS;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableAutomaticLOD;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableCulling;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    bool bEnableOcclusion;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float CullingDistance;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    int32 MaxDrawCalls;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    int32 MaxTriangles;

    UPROPERTY(BlueprintReadWrite, Category = "Optimization")
    float MemoryLimitMB;

    FEng_OptimizationSettings()
    {
        TargetFPS = 60.0f;
        bEnableAutomaticLOD = true;
        bEnableCulling = true;
        bEnableOcclusion = true;
        CullingDistance = 5000.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 1000000;
        MemoryLimitMB = 4096.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PerformanceOptimizer : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    bool IsPerformanceTargetMet() const;

    // Optimization Settings
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void SetOptimizationSettings(const FEng_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    FEng_OptimizationSettings GetOptimizationSettings() const;

    // Automatic Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void EnableAutomaticOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeForTarget(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void ApplyLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void ApplyCullingOptimization();

    // Performance Alerts
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void SetPerformanceThreshold(float MinFPS, float MaxFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    bool IsPerformanceBelowThreshold() const;

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimizer")
    float GetMemoryUsageMB() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    FEng_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    bool bAutomaticOptimizationEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    float PerformanceThresholdFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    float PerformanceThresholdFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Optimizer")
    TArray<float> FPSHistory;

private:
    void CollectPerformanceData();
    void ApplyAutomaticOptimizations();
    void UpdateFPSHistory(float CurrentFPS);
    float CalculateAverageFPS() const;
};
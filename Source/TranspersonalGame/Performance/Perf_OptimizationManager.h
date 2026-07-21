#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Perf_OptimizationManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Low,
    Medium,
    High,
    Ultra
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly)
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly)
    int32 PhysicsActorCount;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        PhysicsActorCount = 0;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_OptimizationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_OptimizationLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(int32 TargetFPS);

protected:
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    EPerf_OptimizationLevel CurrentOptimizationLevel;

    UPROPERTY()
    int32 TargetFrameRate;

    UPROPERTY()
    float MonitoringInterval;

    UPROPERTY()
    FPerf_PerformanceMetrics CachedMetrics;

    FTimerHandle MonitoringTimerHandle;

    void UpdatePerformanceMetrics();
    void ApplyOptimizationSettings();
    void OptimizeForTargetFPS();
};
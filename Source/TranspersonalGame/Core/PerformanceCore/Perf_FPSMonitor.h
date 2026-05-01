#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_FPSMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Acceptable  UMETA(DisplayName = "Acceptable (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::Good;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        GameThreadTime = 16.67f;
        RenderThreadTime = 16.67f;
        GPUTime = 16.67f;
        DrawCalls = 0;
        Triangles = 0;
        MemoryUsageMB = 0.0f;
        PerformanceLevel = EPerf_PerformanceLevel::Good;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_FPSMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_FPSMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MonitorMesh;

    // Performance monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float UpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 SampleCount = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableDetailedProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float WarningFPSThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float CriticalFPSThreshold = 15.0f;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_PerformanceMetrics CurrentMetrics;

    // Performance history
    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> GameThreadHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<float> RenderThreadHistory;

private:
    float TimeSinceLastUpdate = 0.0f;
    float TotalFPS = 0.0f;
    int32 FPSSampleCount = 0;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const { return CurrentMetrics.AverageFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const { return CurrentMetrics.PerformanceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetUpdateInterval(float NewInterval) { UpdateInterval = FMath::Max(0.01f, NewInterval); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDetailedProfiling(bool bEnable) { bEnableDetailedProfiling = bEnable; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    TArray<float> GetFPSHistory() const { return FPSHistory; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogCurrentMetrics();

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeForTarget60FPS();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void OptimizeForTarget30FPS();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ApplyLODOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Performance Optimization")
    void ReduceDrawCalls();

private:
    void UpdatePerformanceMetrics();
    void CalculatePerformanceLevel();
    void UpdateFPSHistory(float NewFPS);
    void CheckPerformanceWarnings();
    void GatherDetailedMetrics();
};
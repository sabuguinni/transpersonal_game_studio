#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_PerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    FPerf_FrameData()
    {
        FrameTime = 0.0f;
        DeltaTime = 0.0f;
        ActorCount = 0;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceMonitor : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const { return CurrentFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const { return CurrentPerformanceLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const { return CurrentFrameData; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceStats();

    // Performance optimization triggers
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformanceLevel(EPerf_PerformanceLevel TargetLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceLOD() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldCullDistantObjects() const;

protected:
    // Core performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FPerf_FrameData CurrentFrameData;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float UltraPerformanceThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float HighPerformanceThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MediumPerformanceThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float LowPerformanceThreshold = 20.0f;

    // Monitoring settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    float MonitoringInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    int32 FrameHistorySize = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monitoring")
    bool bLogPerformanceData = false;

private:
    // Internal tracking
    TArray<float> FrameTimeHistory;
    float MonitoringTimer;
    bool bIsMonitoring;
    int32 FrameCounter;
    float TotalFrameTime;

    // Performance calculation methods
    void UpdatePerformanceMetrics(float DeltaTime);
    void CalculateAverageFrameTime();
    void DeterminePerformanceLevel();
    void UpdateFrameData();
    void LogPerformanceData() const;

    // Optimization methods
    void ApplyPerformanceOptimizations();
    void OptimizeLODSystem();
    void OptimizeCulling();
    void OptimizePhysics();
};
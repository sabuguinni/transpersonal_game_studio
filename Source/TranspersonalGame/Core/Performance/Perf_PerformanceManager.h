#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

UENUM(BlueprintType)
enum class EPerf_PlatformType : uint8
{
    PC_HighEnd      UMETA(DisplayName = "PC High-End"),
    PC_MidRange     UMETA(DisplayName = "PC Mid-Range"),
    PC_LowEnd       UMETA(DisplayName = "PC Low-End"),
    Console_Next    UMETA(DisplayName = "Next-Gen Console"),
    Console_Current UMETA(DisplayName = "Current-Gen Console"),
    Mobile          UMETA(DisplayName = "Mobile")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        ActorCount = 0;
        DrawCalls = 0;
        MemoryUsageMB = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PlatformTargets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActors = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryMB = 8192.0f;

    FPerf_PlatformTargets()
    {
        TargetFPS = 60.0f;
        MaxFrameTime = 16.67f;
        MaxActors = 10000;
        MaxDrawCalls = 2000;
        MaxMemoryMB = 8192.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

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
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPlatformType(EPerf_PlatformType NewPlatform);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    // Auto-optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AutoOptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingSettings();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PlatformType CurrentPlatform = EPerf_PlatformType::PC_HighEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PlatformTargets PlatformTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDetailedProfiling = false;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyPlatformSettings();
    void ApplyPerformanceLevelSettings();

    // Optimization methods
    void OptimizeActorsInRadius(const FVector& Center, float Radius);
    void CullDistantActors();
    void AdjustLODDistances();

    // Timing
    float LastMonitorTime = 0.0f;
    TArray<float> FrameTimeHistory;
    static const int32 MaxFrameHistorySize = 60;

    // Performance state
    bool bIsMonitoring = false;
    bool bPerformanceWarning = false;
    float LastOptimizationTime = 0.0f;
    static const float OptimizationCooldown = 5.0f;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_PerformanceProfiler.generated.h"

// Performance metrics structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUTime;

    FPerf_PerformanceMetrics()
        : CurrentFPS(0.0f)
        , AverageFrameTime(0.0f)
        , MemoryUsageMB(0.0f)
        , ActorCount(0)
        , DrawCalls(0)
        , GPUTime(0.0f)
        , CPUTime(0.0f)
    {}
};

// Performance profiling levels
UENUM(BlueprintType)
enum class EPerf_ProfilingLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Basic       UMETA(DisplayName = "Basic"),
    Detailed    UMETA(DisplayName = "Detailed"),
    Advanced    UMETA(DisplayName = "Advanced"),
    Debug       UMETA(DisplayName = "Debug")
};

/**
 * Performance Profiler Component
 * Monitors game performance in real-time and provides optimization recommendations
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetProfilingLevel(EPerf_ProfilingLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorLOD(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantActors(float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS_PC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float TargetFPS_Console;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Targets")
    int32 MaxActorCount;

protected:
    // Profiling state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsProfiling;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_ProfilingLevel CurrentProfilingLevel;

    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    // Frame time history for averaging
    TArray<float> FrameTimeHistory;

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 FrameHistorySize;

    // Profiling timer
    float ProfilingTimer;
    float ProfilingInterval;

private:
    void UpdateMetrics(float DeltaTime);
    void CollectBasicMetrics();
    void CollectDetailedMetrics();
    void CollectAdvancedMetrics();
    float CalculateAverageFrameTime() const;
    void OptimizePerformance();
};
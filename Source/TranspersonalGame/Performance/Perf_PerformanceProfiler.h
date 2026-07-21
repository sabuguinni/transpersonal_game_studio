#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "SharedTypes.h"
#include "Perf_PerformanceProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsed;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUMemoryUsed;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TickingActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    FPerf_PerformanceMetrics()
    {
        FrameTime = 0.0f;
        DeltaTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        GPUMemoryUsed = 0.0f;
        CPUMemoryUsed = 0.0f;
        ActiveActors = 0;
        TickingActors = 0;
        PhysicsTime = 0.0f;
        RenderTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WarningFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGPUMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCPUMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors;

    FPerf_PerformanceThresholds()
    {
        TargetFrameRate = 60.0f;
        CriticalFrameTime = 33.33f;
        WarningFrameTime = 20.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 500000;
        MaxGPUMemory = 4096.0f;
        MaxCPUMemory = 8192.0f;
        MaxActiveActors = 8000;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableProfiling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ProfilingInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLogToFile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDisplayOnScreen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceThresholds PerformanceThresholds;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceCritical() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(const FPerf_PerformanceThresholds& NewThresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceData();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void DisplayPerformanceOnScreen();

private:
    float LastProfilingTime;
    TArray<FPerf_PerformanceMetrics> MetricsHistory;
    FString LogFilePath;

    void UpdateMetrics();
    void CalculatePerformanceLevel();
    void WriteToLogFile(const FPerf_PerformanceMetrics& Metrics);
    void CheckPerformanceThresholds();
    float GetAverageFrameTime() const;
    int32 GetMemoryUsage() const;
};
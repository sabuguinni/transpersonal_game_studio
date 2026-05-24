#pragma once

#include "CoreMinimal.h"
#include "Engine/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "Misc/DateTime.h"
#include "../SharedTypes.h"
#include "PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceTarget;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFrameTime = 16.67f;
        GPUFrameTime = 8.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        ActiveActorCount = 0;
        bIsPerformanceTarget = true;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance"),
    Critical    UMETA(DisplayName = "Critical Performance")
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

    // Performance monitoring properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    EPerf_PerformanceLevel TargetPerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Data")
    TArray<FPerf_PerformanceMetrics> PerformanceHistory;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxMemoryUsageMB;

private:
    float TimeSinceLastUpdate;
    FDateTime LastUpdateTime;

public:
    // Performance monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceTest();

protected:
    void UpdatePerformanceMetrics();
    void AnalyzePerformanceBottlenecks();
    void ApplyAutoOptimizations();
    void LogPerformanceWarning(const FString& Warning);
};
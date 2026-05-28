#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_PerformanceMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    High        UMETA(DisplayName = "High Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    Low         UMETA(DisplayName = "Low Performance"),
    Critical    UMETA(DisplayName = "Critical Performance")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceLevel PerformanceLevel;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        VisibleActors = 0;
        PerformanceLevel = EPerf_PerformanceLevel::High;
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
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CriticalFPSThreshold;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeScene();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceStats(bool bEnable);

private:
    float TimeSinceLastUpdate;
    TArray<float> FPSHistory;
    int32 MaxFPSHistorySize;

    void CalculateAverageFPS();
    void UpdateLODBasedOnPerformance();
    void LogPerformanceWarnings();
};
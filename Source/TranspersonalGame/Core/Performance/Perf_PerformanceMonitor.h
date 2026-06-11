#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Perf_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsPerformanceGood = true;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        FrameTime = 16.67f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        bIsPerformanceGood = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    int32 MaxTriangles = 500000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
    float MaxMemoryMB = 4096.0f;

    FPerf_PerformanceBudget()
    {
        TargetFPS_PC = 60.0f;
        TargetFPS_Console = 30.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 500000;
        MaxMemoryMB = 4096.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* IndicatorMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceBudget PerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVisualIndicator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bLogPerformanceData = true;

private:
    float TimeSinceLastUpdate = 0.0f;
    float AccumulatedFrameTime = 0.0f;
    int32 FrameCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceWithinBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(const FPerf_PerformanceBudget& NewBudget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateVisualIndicator();

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnPerformanceBudgetExceeded();

    UFUNCTION(BlueprintImplementableEvent, Category = "Performance")
    void OnPerformanceRestored();
};
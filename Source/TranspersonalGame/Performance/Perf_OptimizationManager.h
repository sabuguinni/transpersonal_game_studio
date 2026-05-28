#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_OptimizationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsActors = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    FPerf_OptimizationSettings()
    {
        TargetFrameRate = 60.0f;
        MinFrameRate = 30.0f;
        MaxPhysicsActors = 500;
        ViewDistanceScale = 1.0f;
        ShadowResolution = 2048;
        bEnableAutomaticOptimization = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SystemMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMemoryUsageMB = 0.0f;

    FPerf_SystemMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        ActivePhysicsActors = 0;
        TotalActorCount = 0;
        MemoryUsageMB = 0.0f;
        GPUMemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_OptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_OptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_SystemMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MetricsUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDebugDisplay = true;

private:
    float LastMetricsUpdate = 0.0f;
    TArray<float> FrameRateHistory;
    int32 MaxHistorySize = 60;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutomaticOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_SystemMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaultSettings();

protected:
    void CalculateFrameRateMetrics();
    void CountPhysicsActors();
    void UpdateMemoryMetrics();
    void ApplyAutomaticOptimizations();
    void DisplayDebugInfo();
};
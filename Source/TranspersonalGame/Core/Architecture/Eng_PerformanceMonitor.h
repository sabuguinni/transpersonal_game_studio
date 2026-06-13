#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float FrameRate;

    UPROPERTY(BlueprintReadOnly)
    float FrameTime;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly)
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly)
    float CPUTime;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime;

    FEng_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        DrawCalls = 0;
        ActorCount = 0;
        MemoryUsageMB = 0.0f;
        CPUTime = 0.0f;
        GPUTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override { return true; }
    virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UEng_PerformanceMonitor, STATGROUP_Tickables); }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceReport();

private:
    UPROPERTY()
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY()
    bool bIsProfilingActive;

    UPROPERTY()
    float ProfilingInterval;

    UPROPERTY()
    float TimeSinceLastUpdate;

    void UpdateMetrics();
    void CheckPerformanceThresholds();
};
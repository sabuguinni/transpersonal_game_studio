#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "QA_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    FQA_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UQA_PerformanceMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    void LogPerformanceReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxAcceptableMemoryMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    bool bIsMonitoring;

private:
    void UpdateMetrics();
    void CalculateAverageFPS();

    TArray<float> FPSSamples;
    FTimerHandle MonitoringTimerHandle;
    int32 SampleCount;
    float TotalFPS;
};
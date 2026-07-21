#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUUsagePercent;

    FEng_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        ActiveActorCount = 0;
        ActiveDinosaurCount = 0;
        MemoryUsageMB = 0.0f;
        CPUUsagePercent = 0.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastMonitorTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FEng_PerformanceMetrics GetCurrentMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void TriggerPerformanceOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS(int32 SampleCount = 60);
};
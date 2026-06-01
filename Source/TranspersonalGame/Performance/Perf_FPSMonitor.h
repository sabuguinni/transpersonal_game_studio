#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_FPSMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FPSData
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
    float FrameTime;

    FPerf_FPSData()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FPSMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FPSMonitor();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FPSData GetCurrentFPSData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

protected:
    void UpdateFPSData();

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FPSData CurrentData;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FPSSamples;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxSamples;

    FTimerHandle MonitoringTimer;
    float LastFrameTime;
    float AccumulatedTime;
    int32 FrameCount;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_FrameRateMonitor.generated.h"

USTRUCT(BlueprintType)
struct FPerf_FrameRateData
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

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalFrameCount;

    FPerf_FrameRateData()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        TotalFrameCount = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_FrameRateMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateMonitor();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameRateData GetCurrentFrameRateData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsMonitoring() const { return bIsMonitoring; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float InTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformingWell() const;

protected:
    void UpdateFrameRateData();

private:
    UPROPERTY()
    bool bIsMonitoring;

    UPROPERTY()
    float TargetFPS;

    UPROPERTY()
    FPerf_FrameRateData FrameRateData;

    UPROPERTY()
    TArray<float> RecentFrameTimes;

    UPROPERTY()
    float TotalFrameTime;

    FTimerHandle MonitoringTimerHandle;
    
    static const int32 MaxRecentFrames = 60;
};
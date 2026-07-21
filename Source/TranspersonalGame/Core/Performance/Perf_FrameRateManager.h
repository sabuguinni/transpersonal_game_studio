#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Perf_FrameRateManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameStats
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
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    FPerf_FrameStats()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 8.0f;
        GPUTime = 12.0f;
    }
};

UENUM(BlueprintType)
enum class EPerf_PerformanceTarget : uint8
{
    PC_High = 0     UMETA(DisplayName = "PC High (60 FPS)"),
    PC_Medium = 1   UMETA(DisplayName = "PC Medium (45 FPS)"),
    Console_High = 2 UMETA(DisplayName = "Console High (30 FPS)"),
    Console_Low = 3  UMETA(DisplayName = "Console Low (24 FPS)")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameStats GetCurrentFrameStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerf_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetFrameStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTarget TargetPerformance;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMonitoringActive;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FrameTimeHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 FrameCounter;

private:
    void UpdateFrameStats();
    void CheckPerformanceThresholds();
    void AdjustQualitySettings();
    float CalculateAverageFPS() const;

    FTimerHandle MonitoringTimerHandle;
    double LastFrameTime;
    double MonitoringStartTime;
};
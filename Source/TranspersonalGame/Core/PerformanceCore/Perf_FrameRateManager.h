#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Perf_FrameRateManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceTarget : uint8
{
    High_60FPS      UMETA(DisplayName = "High Performance (60 FPS)"),
    Medium_45FPS    UMETA(DisplayName = "Medium Performance (45 FPS)"),
    Console_30FPS   UMETA(DisplayName = "Console Performance (30 FPS)"),
    Low_24FPS       UMETA(DisplayName = "Low Performance (24 FPS)")
};

USTRUCT(BlueprintType)
struct FPerf_FrameMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    FPerf_FrameMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnPerformanceThresholdCrossed, EPerf_PerformanceTarget, NewTarget);

/**
 * Frame Rate Manager - Monitors and manages game performance
 * Automatically adjusts quality settings to maintain target FPS
 * Provides performance metrics for other systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetPerformance(EPerf_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceTarget GetCurrentTarget() const { return CurrentTarget; }

    // Quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustQualityForTarget();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceQualityLevel(int32 QualityLevel);

    // Performance thresholds
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetPerformanceScore() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnPerformanceThresholdCrossed OnPerformanceThresholdCrossed;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceHUD(bool bEnable);

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTarget CurrentTarget = EPerf_PerformanceTarget::High_60FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMonitoringActive = false;

    // Performance history
    UPROPERTY()
    TArray<float> FPSHistory;

    UPROPERTY()
    int32 MaxHistorySize = 300; // 5 seconds at 60fps

    // Quality settings
    UPROPERTY()
    int32 CurrentQualityLevel = 3; // Epic by default

    UPROPERTY()
    bool bAutoAdjustQuality = true;

    // Timing
    UPROPERTY()
    float MonitoringInterval = 0.1f; // Update every 100ms

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    // Performance thresholds
    float GetTargetFPS() const;
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void AdjustQualityDown();
    void AdjustQualityUp();
    
    // Console commands
    void RegisterConsoleCommands();
    void UnregisterConsoleCommands();

private:
    FTimerHandle MonitoringTimerHandle;
    
    // Console command delegates
    FAutoConsoleCommand* StartMonitoringCommand;
    FAutoConsoleCommand* StopMonitoringCommand;
    FAutoConsoleCommand* ReportCommand;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Perf_FrameRateController.generated.h"

UENUM(BlueprintType)
enum class EPerf_FrameRateTarget : uint8
{
    Auto        UMETA(DisplayName = "Auto Detect"),
    FPS_30      UMETA(DisplayName = "30 FPS"),
    FPS_60      UMETA(DisplayName = "60 FPS"),
    FPS_120     UMETA(DisplayName = "120 FPS"),
    Unlimited   UMETA(DisplayName = "Unlimited")
};

USTRUCT(BlueprintType)
struct FPerf_FrameRateSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    EPerf_FrameRateTarget TargetFrameRate = EPerf_FrameRateTarget::Auto;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate", meta = (ClampMin = "10.0", ClampMax = "240.0"))
    float CustomFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bVSyncEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bAdaptiveFrameRate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float FrameRateTolerance = 5.0f;

    FPerf_FrameRateSettings()
    {
        TargetFrameRate = EPerf_FrameRateTarget::Auto;
        CustomFrameRate = 60.0f;
        bVSyncEnabled = true;
        bAdaptiveFrameRate = true;
        FrameRateTolerance = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_FrameRateMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FrameDropCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CPUTime = 0.0f;

    FPerf_FrameRateMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        FrameDropCount = 0;
        GPUTime = 0.0f;
        CPUTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateController : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateController();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Frame rate control
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(EPerf_FrameRateTarget TargetRate);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCustomFrameRate(float CustomRate);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVSync(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveFrameRate(bool bEnable);

    // Metrics
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameRateMetrics GetFrameRateMetrics() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyFrameRateSettings(const FPerf_FrameRateSettings& Settings);

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameRateSettings GetFrameRateSettings() const;

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPlatform();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsFrameRateStable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_FrameRateSettings FrameRateSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_FrameRateMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MetricsUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MetricsHistorySize = 60;

private:
    void UpdateMetrics();
    void ApplyFrameRateLimit();
    void DetectOptimalFrameRate();
    void HandleFrameRateChange();

    FTimerHandle MetricsTimerHandle;
    TArray<float> FrameTimeHistory;
    float LastFrameTime;
    float MetricsAccumulator;
    int32 MetricsFrameCount;
    bool bInitialized;
};
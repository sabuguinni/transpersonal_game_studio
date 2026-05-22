#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Perf_FrameRateManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra     UMETA(DisplayName = "Ultra (60+ FPS)"),
    High      UMETA(DisplayName = "High (45-60 FPS)"),
    Medium    UMETA(DisplayName = "Medium (30-45 FPS)"),
    Low       UMETA(DisplayName = "Low (20-30 FPS)"),
    Critical  UMETA(DisplayName = "Critical (<20 FPS)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameRateData
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
    EPerf_PerformanceLevel PerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    FPerf_FrameRateData()
        : CurrentFPS(60.0f)
        , AverageFPS(60.0f)
        , MinFPS(60.0f)
        , MaxFPS(60.0f)
        , PerformanceLevel(EPerf_PerformanceLevel::Ultra)
        , FrameTime(16.67f)
        , GameThreadTime(8.0f)
        , RenderThreadTime(8.0f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Frame rate monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartFrameRateMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopFrameRateMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameRateData GetCurrentFrameRateData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const;

    // Dynamic quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDynamicQualityAdjustment(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceQualityLevel(EPerf_PerformanceLevel Level);

    // Performance thresholds
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(float UltraFPS, float HighFPS, float MediumFPS, float LowFPS);

    // Console commands
    UFUNCTION(CallInEditor, Category = "Performance")
    void ShowPerformanceStats();

    UFUNCTION(CallInEditor, Category = "Performance")
    void ResetFrameRateStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bDynamicQualityAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float UltraThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float HighThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float MediumThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Thresholds")
    float LowThreshold;

private:
    FPerf_FrameRateData CurrentFrameData;
    TArray<float> FrameRateHistory;
    FTimerHandle MonitoringTimerHandle;
    
    void UpdateFrameRateData();
    void CalculateAverageFrameRate();
    EPerf_PerformanceLevel DeterminePerformanceLevel(float FPS) const;
    void AdjustQualityBasedOnPerformance();
    void LogPerformanceData() const;
};
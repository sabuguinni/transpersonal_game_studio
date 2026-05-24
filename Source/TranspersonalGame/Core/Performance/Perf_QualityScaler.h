#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Perf_QualityScaler.generated.h"

UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Auto        UMETA(DisplayName = "Auto")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_QualitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    EPerf_QualityLevel OverallQuality = EPerf_QualityLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 ViewDistanceQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 ShadowQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 TextureQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 EffectsQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 FoliageQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 PostProcessQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bUseDynamicResolution = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float ResolutionScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bUseTemporalUpsampling = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float FrameTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float GPUTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    bool bIsPerformanceGood = true;
};

/**
 * Quality Scaler for dynamic performance optimization
 * Automatically adjusts graphics settings based on performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_QualityScaler : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_QualityScaler();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(EPerf_QualityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityLevel GetCurrentQualityLevel() const { return CurrentSettings.OverallQuality; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualitySettings(const FPerf_QualitySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_QualitySettings GetCurrentQualitySettings() const { return CurrentSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetPerformanceMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoQualityScaling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsAutoQualityEnabled() const { return bAutoQualityEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFrameRate() const { return TargetFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetDynamicResolution(bool bEnable, float MinScale = 0.5f, float MaxScale = 1.0f);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void TestQualityScaling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceQualityUpdate();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_QualitySettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Scaling")
    bool bAutoQualityEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Scaling")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Scaling")
    float FrameRateThreshold = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Auto Scaling")
    float QualityAdjustmentDelay = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Resolution")
    bool bUseDynamicResolution = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Resolution")
    float MinResolutionScale = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Resolution")
    float MaxResolutionScale = 1.0f;

private:
    void UpdatePerformanceMetrics();
    void CheckAutoQualityScaling();
    void AdjustQualityUp();
    void AdjustQualityDown();
    void ApplyQualityPreset(EPerf_QualityLevel Level);
    void UpdateDynamicResolution();
    
    FTimerHandle MetricsUpdateTimer;
    FTimerHandle QualityCheckTimer;
    
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameTimeHistory = 120; // 2 seconds at 60fps
    
    float LastQualityAdjustmentTime = 0.0f;
    bool bPendingQualityIncrease = false;
    bool bPendingQualityDecrease = false;
};

#include "Perf_QualityScaler.generated.h"
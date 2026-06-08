#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Perf_FrameRateController.generated.h"

UENUM(BlueprintType)
enum class EPerf_FrameRateTarget : uint8
{
    FPS_30 UMETA(DisplayName = "30 FPS"),
    FPS_60 UMETA(DisplayName = "60 FPS"),
    FPS_120 UMETA(DisplayName = "120 FPS"),
    FPS_Unlimited UMETA(DisplayName = "Unlimited")
};

UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Low UMETA(DisplayName = "Low Quality"),
    Medium UMETA(DisplayName = "Medium Quality"),
    High UMETA(DisplayName = "High Quality"),
    Ultra UMETA(DisplayName = "Ultra Quality"),
    Auto UMETA(DisplayName = "Auto Adjust")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameRateSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    EPerf_FrameRateTarget TargetFrameRate = EPerf_FrameRateTarget::FPS_60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bEnableVSync = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bEnableFrameSmoothing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float MinDesiredFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float MaxDesiredFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    EPerf_QualityLevel QualityLevel = EPerf_QualityLevel::Auto;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bAutoAdjustQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 0.9f; // 90% of target frame time
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_FrameRateController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_FrameRateController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Frame rate control functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(EPerf_FrameRateTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityLevel(EPerf_QualityLevel NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoQualityAdjustment(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    // Quality adjustment functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustQualityForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaultSettings();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_FrameRateSettings FrameRateSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring;

private:
    // Internal performance tracking
    TArray<float> FrameTimeHistory;
    float PerformanceCheckTimer;
    float QualityAdjustmentCooldown;
    int32 ConsecutivePoorFrames;
    
    static const int32 MaxFrameHistorySize = 60; // 1 second at 60fps
    static const float PerformanceCheckInterval = 1.0f;
    static const float QualityAdjustmentCooldownTime = 5.0f;
    static const int32 PoorFrameThreshold = 10;

    // Internal helper functions
    void UpdateFrameRateMetrics(float DeltaTime);
    void CheckPerformanceAndAdjust();
    void ApplyFrameRateSettings();
    void ApplyQualitySettings(EPerf_QualityLevel Quality);
    float CalculateAverageFrameRate() const;
    bool ShouldAdjustQuality() const;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Perf_FrameRateController.generated.h"

UENUM(BlueprintType)
enum class EPerf_FrameRateTarget : uint8
{
    FPS_30      UMETA(DisplayName = "30 FPS"),
    FPS_60      UMETA(DisplayName = "60 FPS"),
    FPS_120     UMETA(DisplayName = "120 FPS"),
    FPS_VSYNC   UMETA(DisplayName = "VSync"),
    FPS_UNLIM   UMETA(DisplayName = "Unlimited")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameRateSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    EPerf_FrameRateTarget TargetFrameRate = EPerf_FrameRateTarget::FPS_60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bUseVSync = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bUseFrameRateSmoothing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float FrameRateSmoothingFactor = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    bool bAdaptiveFrameRate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float MinFrameRateThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate")
    float MaxFrameRateThreshold = 65.0f;
};

/**
 * Frame Rate Controller for maintaining stable performance
 * Manages target frame rates and adaptive quality settings
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateController : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateController();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(EPerf_FrameRateTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableVSync(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetFrameRateSmoothing(bool bEnable, float SmoothingFactor = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveFrameRate(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsFrameRateStable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyFrameRateSettings(const FPerf_FrameRateSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameRateSettings GetCurrentSettings() const { return CurrentSettings; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void TestFrameRateControl();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_FrameRateSettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float FrameTimeVariance;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    bool bIsFrameRateStable;

private:
    void UpdateFrameRateStats();
    void ApplyTargetFrameRate();
    void CheckAdaptiveFrameRate();
    
    TArray<float> FrameTimeHistory;
    static constexpr int32 MaxFrameTimeHistory = 60;
    
    FTimerHandle FrameRateUpdateTimer;
    float LastFrameTime;
    int32 FrameCounter;
};

#include "Perf_FrameRateController.generated.h"
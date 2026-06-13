#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_FrameRateManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceTarget : uint8
{
    PC_60FPS UMETA(DisplayName = "PC 60 FPS"),
    Console_30FPS UMETA(DisplayName = "Console 30 FPS"),
    Mobile_30FPS UMETA(DisplayName = "Mobile 30 FPS"),
    Adaptive UMETA(DisplayName = "Adaptive")
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
    bool bIsTargetMet = false;

    FPerf_FrameMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        bIsTargetMet = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_FrameRateManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_FrameRateManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World Subsystem interface
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTarget(EPerf_PerformanceTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartFrameTracking();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopFrameTracking();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFrameRate() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_PerformanceTarget CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_FrameMetrics FrameMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsTracking;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameRate;

private:
    void UpdateFrameMetrics();
    void CheckPerformanceTarget();
    void AdjustQualitySettings();
    
    FTimerHandle MetricsUpdateTimer;
    TArray<float> FrameTimeHistory;
    int32 FrameCounter;
    float TotalFrameTime;
    double LastUpdateTime;
};

#include "Perf_FrameRateManager.generated.h"
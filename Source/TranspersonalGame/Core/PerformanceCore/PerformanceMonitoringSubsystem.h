#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/Platform.h"
#include "../SharedTypes.h"
#include "PerformanceMonitoringSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, EPerf_PerformanceLevel, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFrameRateChanged, float, CurrentFPS, float, TargetFPS);

/**
 * Real-time performance monitoring subsystem for the Transpersonal Game
 * Tracks FPS, memory usage, GPU performance and automatically adjusts quality settings
 * Ensures 60fps on high-end PC and 30fps on console platforms
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceMonitoringSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceMonitoringSubsystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameRate() const { return CurrentFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const { return AverageFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const { return CurrentMemoryUsageMB; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Quality adjustment
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float InTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAutoQualityAdjustment(bool bEnable);

    // Performance zones
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterPerformanceZone(const FString& ZoneName, const FVector& Location, float Radius, EPerf_PerformanceLevel RequiredLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterPerformanceZone(const FString& ZoneName);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnFrameRateChanged OnFrameRateChanged;

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float TargetFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentGPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    // Settings
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bAutoQualityAdjustment;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float MonitoringInterval;

    // Performance zones
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    TMap<FString, FPerf_PerformanceZone> PerformanceZones;

private:
    // Internal monitoring
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void AdjustQualitySettings();
    void UpdatePerformanceZones();

    // Quality adjustment methods
    void ApplyPerformanceLevel(EPerf_PerformanceLevel Level);
    void SetLODBias(float Bias);
    void SetShadowQuality(int32 Quality);
    void SetTextureQuality(int32 Quality);
    void SetPostProcessQuality(int32 Quality);

    // Frame rate tracking
    TArray<float> FrameRateHistory;
    float FrameRateAccumulator;
    int32 FrameRateSamples;
    static constexpr int32 MaxFrameRateHistory = 60;

    // Timing
    FTimerHandle MonitoringTimerHandle;
    double LastUpdateTime;

    // Performance thresholds
    static constexpr float HighPerformanceThreshold = 55.0f;
    static constexpr float MediumPerformanceThreshold = 45.0f;
    static constexpr float LowPerformanceThreshold = 25.0f;
};
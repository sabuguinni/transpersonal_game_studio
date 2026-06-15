#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Perf_FrameRateMonitor.generated.h"

USTRUCT(BlueprintType)
struct FPerf_FrameData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    FPerf_FrameData()
    {
        FrameRate = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceAlert, float, CurrentFPS);

UCLASS()
class TRANSPERSONALGAME_API UPerf_FrameRateMonitor : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameData GetCurrentFrameData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const { return AverageFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMinimumFrameRate() const { return MinFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMaximumFrameRate() const { return MaxFrameRate; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceGood() const { return GetAverageFrameRate() >= TargetFrameRate * 0.9f; }

    // Performance Targets
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFrameRate(float NewTargetFPS) { TargetFrameRate = NewTargetFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetTargetFrameRate() const { return TargetFrameRate; }

    // Performance Alerts
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceAlert OnPerformanceAlert;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceAlerts(bool bEnable) { bPerformanceAlertsEnabled = bEnable; }

    // Statistics
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetStatistics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetSampleCount() const { return FrameSamples.Num(); }

protected:
    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceAlertThreshold = 0.8f; // Alert when FPS drops below 80% of target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceAlertsEnabled = true;

    // Sample Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSamples = 300; // 5 seconds at 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SampleInterval = 0.016f; // Sample every frame at 60fps

    // Internal Data
    UPROPERTY()
    TArray<float> FrameSamples;

    UPROPERTY()
    float AverageFrameRate = 0.0f;

    UPROPERTY()
    float MinFrameRate = 0.0f;

    UPROPERTY()
    float MaxFrameRate = 0.0f;

    UPROPERTY()
    float LastSampleTime = 0.0f;

    UPROPERTY()
    bool bInitialized = false;

    // Tick Function
    FTickerDelegate TickDelegate;
    FTSTicker::FDelegateHandle TickDelegateHandle;

    bool Tick(float DeltaTime);
    void UpdateFrameStatistics();
    void CheckPerformanceAlerts();
    void AddFrameSample(float FrameRate);
    float CalculateCurrentFrameRate() const;
};
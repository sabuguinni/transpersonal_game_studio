#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perf_FPSMonitor.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent (60+ FPS)"),
    Good        UMETA(DisplayName = "Good (45-60 FPS)"),
    Fair        UMETA(DisplayName = "Fair (30-45 FPS)"),
    Poor        UMETA(DisplayName = "Poor (15-30 FPS)"),
    Critical    UMETA(DisplayName = "Critical (<15 FPS)")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceMetrics
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
    EPerf_PerformanceLevel PerformanceLevel = EPerf_PerformanceLevel::Good;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount = 0;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        PerformanceLevel = EPerf_PerformanceLevel::Good;
        PhysicsActorCount = 0;
        VisibleActorCount = 0;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_FPSMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_FPSMonitor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // FPS Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const { return CurrentMetrics.CurrentFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_PerformanceLevel GetPerformanceLevel() const { return CurrentMetrics.PerformanceLevel; }

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetMetrics();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Performance")
    void StartMonitoring();

    UFUNCTION(CallInEditor, Category = "Performance")
    void StopMonitoring();

    UFUNCTION(CallInEditor, Category = "Performance")
    void LogCurrentMetrics();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float MonitoringInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 SampleCount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bLogPerformanceWarnings = true;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bIsMonitoring = false;

private:
    // Internal tracking
    TArray<float> FPSSamples;
    float LastUpdateTime = 0.0f;
    FTimerHandle MonitoringTimer;
    
    // Performance tracking
    int32 CurrentSampleIndex = 0;
    float TotalFPSSum = 0.0f;
    
    // Methods
    void UpdateFPSMetrics();
    void CalculatePerformanceLevel();
    void CountActors();
    void TriggerOptimization();
    
    UFUNCTION()
    void MonitoringTick();
};
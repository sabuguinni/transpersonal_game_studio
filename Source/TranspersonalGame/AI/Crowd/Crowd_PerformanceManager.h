#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "SharedTypes.h"
#include "Crowd_PerformanceManager.generated.h"

class UCrowd_MassEntitySubsystem;
class UCrowd_LODSystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveEntities = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleEntities = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CrowdUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LODLevel0Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LODLevel1Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LODLevel2Count = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CulledCount = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AdaptiveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    float TargetFrameTime = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    float MaxFrameTime = 33.33f; // 30 FPS minimum

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    int32 MaxActiveEntities = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    int32 MinActiveEntities = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    float LODDistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Performance")
    float UpdateFrequencyMultiplier = 1.0f;
};

/**
 * Performance management system for crowd simulation
 * Monitors frame rates and automatically adjusts crowd density and LOD settings
 * to maintain target performance levels
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_PerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    FCrowd_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetAdaptiveSettings(const FCrowd_AdaptiveSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    FCrowd_AdaptiveSettings GetAdaptiveSettings() const { return AdaptiveSettings; }

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetPerformanceLevel(ECrowd_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    ECrowd_PerformanceLevel GetCurrentPerformanceLevel() const { return CurrentPerformanceLevel; }

    // Entity management
    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void ReduceCrowdDensity(float Percentage);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void IncreaseCrowdDensity(float Percentage);

    UFUNCTION(BlueprintCallable, Category = "Crowd Performance")
    void SetMaxEntityCount(int32 MaxCount);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FCrowd_AdaptiveSettings AdaptiveSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FCrowd_PerformanceMetrics CurrentMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    ECrowd_PerformanceLevel CurrentPerformanceLevel = ECrowd_PerformanceLevel::Medium;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    bool bAdaptivePerformanceEnabled = true;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    float LastOptimizationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance State")
    float OptimizationInterval = 2.0f;

private:
    // Frame time tracking
    TArray<float> FrameTimeHistory;
    int32 FrameTimeHistorySize = 60;
    int32 FrameTimeIndex = 0;

    // Performance thresholds
    float GoodPerformanceThreshold = 14.0f; // ms
    float PoorPerformanceThreshold = 20.0f; // ms

    // Subsystem references
    UPROPERTY()
    UCrowd_MassEntitySubsystem* MassEntitySubsystem = nullptr;

    UPROPERTY()
    UCrowd_LODSystem* LODSystem = nullptr;

    // Internal methods
    void CalculateAverageFrameTime();
    void CheckPerformanceThresholds();
    void ApplyPerformanceOptimizations();
    void UpdateLODSettings();
    void UpdateEntityCounts();
    
    float GetAverageFrameTime() const;
    bool IsPerformancePoor() const;
    bool IsPerformanceGood() const;
};
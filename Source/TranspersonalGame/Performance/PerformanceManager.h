#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

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

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisiblePrimitives = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceThresholdExceeded, const FPerformanceMetrics&, Metrics);

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float InTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceThresholds(float MaxFrameTime, float MaxGameThreadTime, float MaxRenderThreadTime);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODBias(int32 LODBias);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetViewDistanceScale(float Scale);

    // Quality Settings
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderingQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnable);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    // Event dispatchers
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceThresholdExceeded OnPerformanceThresholdExceeded;

protected:
    // Performance tracking
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFrameTimeThreshold = 16.67f; // 60 FPS

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxGameThreadTimeThreshold = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxRenderThreadTimeThreshold = 10.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerformanceMetrics CurrentMetrics;

    // Timer handles
    FTimerHandle PerformanceUpdateTimer;
    FTimerHandle MemoryOptimizationTimer;

    // Internal methods
    void UpdatePerformanceMetrics();
    void CheckPerformanceThresholds();
    void ApplyPerformanceOptimizations();
    void UpdateLODSettings();
    void ManageRenderingFeatures();

private:
    // Performance history for trend analysis
    TArray<FPerformanceMetrics> MetricsHistory;
    static constexpr int32 MaxHistorySize = 300; // 5 minutes at 60 FPS

    // Optimization flags
    bool bPerformanceModeEnabled = false;
    bool bAutoLODEnabled = true;
    bool bDynamicResolutionEnabled = false;

    // Quality backup for restoration
    int32 OriginalQualityLevel = 3;
    float OriginalViewDistanceScale = 1.0f;
    int32 OriginalLODBias = 0;
};
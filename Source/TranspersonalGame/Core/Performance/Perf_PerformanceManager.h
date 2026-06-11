#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"), 
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

UENUM(BlueprintType)
enum class EPerf_OptimizationTarget : uint8
{
    FPS_30      UMETA(DisplayName = "30 FPS Target"),
    FPS_60      UMETA(DisplayName = "60 FPS Target"),
    FPS_120     UMETA(DisplayName = "120 FPS Target"),
    Adaptive    UMETA(DisplayName = "Adaptive Target")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TextureMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsTargetFPSMet = false;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFrameTime = 0.0f;
        GPUFrameTime = 0.0f;
        DrawCalls = 0;
        TriangleCount = 0;
        MemoryUsageMB = 0.0f;
        TextureMemoryMB = 0.0f;
        VisibleActors = 0;
        bIsTargetFPSMet = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationTarget TargetFPS = EPerf_OptimizationTarget::FPS_60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel QualityLevel = EPerf_PerformanceLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTextureStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TextureStreamingPoolSizeMB = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAdaptiveQuality = true;

    FPerf_OptimizationSettings()
    {
        TargetFPS = EPerf_OptimizationTarget::FPS_60;
        QualityLevel = EPerf_PerformanceLevel::Medium;
        bEnableAutomaticLOD = true;
        bEnableOcclusionCulling = true;
        bEnableTextureStreaming = true;
        TextureStreamingPoolSizeMB = 2048;
        ViewDistanceScale = 1.0f;
        ShadowQuality = 3;
        bEnableAdaptiveQuality = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentPerformanceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsTargetFPSMet() const;

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings(const FPerf_OptimizationSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(EPerf_OptimizationTarget Target);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentHardware();

    // Adaptive quality
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAdaptiveQuality();

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceUpdateLODs();

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTextureStreamingPoolSize(int32 SizeMB);

    // Culling optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOcclusionCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetViewDistanceScale(float Scale);

    // Debug and profiling
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void ShowPerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void HidePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void DumpPerformanceReport();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings CurrentSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CachedMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsMonitoringEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AdaptiveQualityThreshold = 0.9f;

private:
    FTimerHandle MonitoringTimerHandle;
    TArray<float> FPSHistory;
    int32 MaxHistorySize = 60;

    void UpdatePerformanceMetrics();
    void ApplyConsoleCommands();
    void OptimizeActorsInLevel();
    float CalculateAverageFPS() const;
    void AdjustQualityBasedOnPerformance();
};
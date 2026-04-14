#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

UENUM(BlueprintType)
enum class EPerf_BottleneckType : uint8
{
    None        UMETA(DisplayName = "None"),
    CPU         UMETA(DisplayName = "CPU"),
    GPU         UMETA(DisplayName = "GPU"),
    Memory      UMETA(DisplayName = "Memory"),
    Network     UMETA(DisplayName = "Network"),
    Storage     UMETA(DisplayName = "Storage")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
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
    float UsedMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AvailableMemoryMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_BottleneckType PrimaryBottleneck = EPerf_BottleneckType::None;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 0.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        UsedMemoryMB = 0.0f;
        AvailableMemoryMB = 0.0f;
        ActiveActors = 0;
        VisibleActors = 0;
        PrimaryBottleneck = EPerf_BottleneckType::None;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel TargetPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles = 2000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCullingOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableTextureStreaming = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableGarbageCollectionOptimization = true;

    FPerf_OptimizationSettings()
    {
        TargetPerformanceLevel = EPerf_PerformanceLevel::High;
        TargetFPS = 60.0f;
        MaxDrawCalls = 2000;
        MaxTriangles = 2000000;
        MaxMemoryUsageMB = 4096.0f;
        bEnableAutoOptimization = true;
        bEnableLODOptimization = true;
        bEnableCullingOptimization = true;
        bEnableTextureStreaming = true;
        bEnableGarbageCollectionOptimization = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    // Optimization controls
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_OptimizationSettings GetOptimizationSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceLevel(EPerf_PerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForTarget(float TargetFPS);

    // Bottleneck detection
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BottleneckType DetectBottleneck();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AutoOptimizeBottleneck();

    // LOD and culling optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCullingSettings();

    // Memory optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    // Rendering optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderingSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetScreenPercentage(float Percentage);

    // Performance reporting
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceBenchmark();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsMonitoring = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MonitoringInterval = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxHistorySize = 300;

private:
    FTimerHandle MonitoringTimerHandle;
    
    void UpdateFPSHistory(float NewFPS);
    void CalculateAverageFPS();
    void ApplyUltraSettings();
    void ApplyHighSettings();
    void ApplyMediumSettings();
    void ApplyLowSettings();
    void ApplyPotatoSettings();
    void LogPerformanceWarning(const FString& Warning);
};
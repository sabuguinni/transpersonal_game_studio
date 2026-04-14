#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "../SharedTypes.h"
#include "Perf_PerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Ultra = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Potato = 4
};

UENUM(BlueprintType)
enum class EPerf_PerformanceCategory : uint8
{
    Rendering = 0,
    Lighting = 1,
    Shadows = 2,
    PostProcess = 3,
    Particles = 4,
    Audio = 5,
    Physics = 6,
    AI = 7,
    Streaming = 8
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

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 60.0f;
        AverageFPS = 60.0f;
        MinFPS = 60.0f;
        MaxFPS = 60.0f;
        FrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 8.0f;
        GPUTime = 10.0f;
        DrawCalls = 1000;
        Triangles = 100000;
        UsedMemoryMB = 2048.0f;
        AvailableMemoryMB = 6144.0f;
    }
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel OverallQuality = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 ScreenPercentage = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    int32 ViewDistance = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 ShadowQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 ShadowDistance = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    int32 PostProcessQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    int32 EffectsQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    int32 TextureQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVSync = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFPS = 120;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicResolution = true;

    FPerf_PerformanceSettings()
    {
        OverallQuality = EPerf_PerformanceLevel::High;
        ScreenPercentage = 100;
        ViewDistance = 3;
        ShadowQuality = 3;
        ShadowDistance = 5000;
        PostProcessQuality = 3;
        EffectsQuality = 3;
        TextureQuality = 3;
        bEnableVSync = false;
        MaxFPS = 120;
        bEnableDynamicResolution = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_PerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_PerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceMonitoring = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MonitoringUpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinAcceptableFPS = 30.0f;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CPUThresholdMs = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float GPUThresholdMs = 16.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MemoryThresholdMB = 6144.0f;

    // Performance Functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentHardware();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMode(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetPerformanceStats();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceBenchmark();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void OptimizeCurrentScene();

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetGlobalLODBias(float LODBias);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void OptimizeLODSettings();

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeMemoryUsage();

    // Rendering Optimization
    UFUNCTION(BlueprintCallable, Category = "Rendering")
    void SetRenderingQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Rendering")
    void SetScreenPercentage(int32 Percentage);

private:
    float LastUpdateTime;
    TArray<float> FPSHistory;
    int32 MaxFPSHistorySize = 60;

    void UpdateFPSHistory(float CurrentFPS);
    void CheckPerformanceThresholds();
    void AutoOptimizeSettings();
    void ApplyQualityPreset(EPerf_PerformanceLevel Level);
};
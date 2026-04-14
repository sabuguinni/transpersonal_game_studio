#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "../SharedTypes.h"
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
enum class EPerf_OptimizationMode : uint8
{
    Disabled    UMETA(DisplayName = "Disabled"),
    Conservative UMETA(DisplayName = "Conservative"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Extreme     UMETA(DisplayName = "Extreme")
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
        FrameTime = 16.67f;
        GameThreadTime = 8.0f;
        RenderThreadTime = 12.0f;
        GPUTime = 14.0f;
        DrawCalls = 1000;
        Triangles = 500000;
        UsedMemoryMB = 2048.0f;
        AvailableMemoryMB = 6144.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableOcclusionCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Culling")
    bool bEnableFrustumCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxShadowCascades = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    int32 TextureQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    int32 EffectsQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    int32 PostProcessQuality = 3;

    FPerf_OptimizationSettings()
    {
        LODBias = 0.0f;
        MaxLODDistance = 5000.0f;
        CullDistance = 10000.0f;
        bEnableOcclusionCulling = true;
        bEnableFrustumCulling = true;
        ShadowDistance = 3000.0f;
        MaxShadowCascades = 4;
        TextureQuality = 3;
        EffectsQuality = 3;
        PostProcessQuality = 3;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel TargetPerformanceLevel = EPerf_PerformanceLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationMode OptimizationMode = EPerf_OptimizationMode::Conservative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinimumFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationCheckInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FPerf_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_OptimizationSettings OptimizationSettings;

private:
    float LastOptimizationCheck = 0.0f;
    TArray<float> FPSHistory;
    int32 MaxFPSHistorySize = 60;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyOptimizationSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationMode(EPerf_OptimizationMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceOptimization();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaultSettings();

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_OptimizationSettings GetOptimizationSettings() const { return OptimizationSettings; }

private:
    void PerformAutoOptimization();
    void ApplyLODOptimizations();
    void ApplyCullingOptimizations();
    void ApplyRenderingOptimizations();
    void ApplyMemoryOptimizations();
    void CollectPerformanceData();
};
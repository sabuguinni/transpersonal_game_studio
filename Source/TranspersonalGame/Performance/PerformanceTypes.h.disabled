#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "PerformanceTypes.generated.h"

/**
 * Performance budget categories for different systems
 */
UENUM(BlueprintType)
enum class EPerf_BudgetCategory : uint8
{
    Rendering       UMETA(DisplayName = "Rendering"),
    Physics         UMETA(DisplayName = "Physics"),
    Audio           UMETA(DisplayName = "Audio"),
    AI              UMETA(DisplayName = "AI"),
    Animation       UMETA(DisplayName = "Animation"),
    Networking      UMETA(DisplayName = "Networking"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Memory          UMETA(DisplayName = "Memory")
};

/**
 * Performance quality levels for adaptive scaling
 */
UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Ultra           UMETA(DisplayName = "Ultra"),
    High            UMETA(DisplayName = "High"),
    Medium          UMETA(DisplayName = "Medium"),
    Low             UMETA(DisplayName = "Low"),
    Potato          UMETA(DisplayName = "Potato")
};

/**
 * Performance monitoring frequency
 */
UENUM(BlueprintType)
enum class EPerf_MonitoringFrequency : uint8
{
    EveryFrame      UMETA(DisplayName = "Every Frame"),
    Every10Frames   UMETA(DisplayName = "Every 10 Frames"),
    Every60Frames   UMETA(DisplayName = "Every 60 Frames"),
    OnDemand        UMETA(DisplayName = "On Demand")
};

/**
 * Performance budget allocation for a specific category
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_BudgetAllocation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_BudgetCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float PercentageOfFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.0"))
    float MaxMilliseconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnforceHardLimit;

    FPerf_BudgetAllocation()
    {
        Category = EPerf_BudgetCategory::Rendering;
        PercentageOfFrame = 10.0f;
        MaxMilliseconds = 2.0f;
        bEnforceHardLimit = false;
    }
};

/**
 * Performance metrics snapshot
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MetricsSnapshot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 Triangles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Timestamp;

    FPerf_MetricsSnapshot()
    {
        FrameTime = 0.0f;
        FPS = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        GPUTime = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        Triangles = 0;
        Timestamp = 0.0f;
    }
};

/**
 * Performance optimization settings
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityLevel CurrentQualityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQualityEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableResolutionScaling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinResolutionScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxResolutionScale;

    FPerf_OptimizationSettings()
    {
        CurrentQualityLevel = EPerf_QualityLevel::High;
        bAdaptiveQualityEnabled = true;
        TargetFPS = 60.0f;
        MinAcceptableFPS = 30.0f;
        bEnableLODScaling = true;
        bEnableResolutionScaling = true;
        MinResolutionScale = 0.5f;
        MaxResolutionScale = 1.0f;
    }
};
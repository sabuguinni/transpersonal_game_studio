#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceOptimizer.generated.h"

// Perf_QualityLevel — PC High / Console Medium / Low fallback
UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Ultra   UMETA(DisplayName = "Ultra")
};

// Perf_FrameStats — runtime frame budget tracking
USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DrawCallCount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TriangleCount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bBudgetExceeded = false;
};

/**
 * APerf_PerformanceOptimizer
 * Runtime performance manager — monitors frame budget and applies
 * dynamic quality scaling to maintain 60fps PC / 30fps console targets.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Optimizer"))
class TRANSPERSONALGAME_API APerf_PerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerf_PerformanceOptimizer();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Target frame rates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFPS_Console = 30.0f;

    // Current quality level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityLevel CurrentQualityLevel = EPerf_QualityLevel::High;

    // Frame budget in milliseconds (16.67ms = 60fps)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float FrameBudgetMs = 16.67f;

    // Dynamic resolution scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DRS")
    bool bEnableDynamicResolution = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DRS")
    float MinResolutionScale = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DRS")
    float MaxResolutionScale = 1.0f;

    // LOD bias for distant objects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LODDistanceScale = 1.0f;

    // Current frame stats
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    FPerf_FrameStats CurrentFrameStats;

    // Apply quality preset
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_QualityLevel Level);

    // Force LOD update on all static meshes in level
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceUpdateLODs();

    // Get current FPS estimate
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetCurrentFPS() const;

    // Check if frame budget is exceeded
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsFrameBudgetExceeded() const;

private:
    float AccumulatedTime = 0.0f;
    float AverageFPS = 60.0f;
    int32 FrameCount = 0;

    void UpdateFrameStats(float DeltaTime);
    void ApplyDynamicResolutionScaling();
    void ExecuteConsoleCommand(const FString& Command);
};

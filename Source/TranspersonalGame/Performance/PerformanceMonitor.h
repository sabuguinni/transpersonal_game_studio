// PerformanceMonitor.h — Transpersonal Game Studio
// Agent #4 Performance Optimizer — Cycle 003
// Runtime performance monitoring component — tracks frame time, draw calls, memory

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Performance/PerformanceConfig.h"
#include "PerformanceMonitor.generated.h"

// Forward declarations
class UStaticMeshComponent;
class USkeletalMeshComponent;

// ============================================================
// STRUCTS (global scope — RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bBelowTargetFPS = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleSkeletalMeshes = 0;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0Distance = PERF_DINO_LOD0_DISTANCE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1Distance = PERF_DINO_LOD1_DISTANCE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2Distance = PERF_DINO_LOD2_DISTANCE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float MaxDrawDistance = PERF_DINO_LOD3_DISTANCE;
};

// ============================================================
// ENUMS (global scope — RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_Platform : uint8
{
    PC_HighEnd      UMETA(DisplayName = "PC High-End (60fps)"),
    PC_MidRange     UMETA(DisplayName = "PC Mid-Range (45fps)"),
    Console         UMETA(DisplayName = "Console (30fps)"),
    Mobile          UMETA(DisplayName = "Mobile (30fps low)")
};

UENUM(BlueprintType)
enum class EPerf_FrameBudgetStatus : uint8
{
    Healthy         UMETA(DisplayName = "Healthy (>55fps)"),
    Warning         UMETA(DisplayName = "Warning (45-55fps)"),
    Critical        UMETA(DisplayName = "Critical (<45fps)"),
    Unacceptable    UMETA(DisplayName = "Unacceptable (<30fps)")
};

// ============================================================
// PERFORMANCE MONITOR COMPONENT
// ============================================================

UCLASS(ClassGroup = "TranspersonalGame|Performance", meta = (BlueprintSpawnableComponent), DisplayName = "Performance Monitor")
class TRANSPERSONALGAME_API UPerformanceMonitor : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceMonitor();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --------------------------------------------------------
    // PLATFORM SETTINGS
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Platform")
    EPerf_Platform TargetPlatform = EPerf_Platform::PC_HighEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Platform")
    float TargetFPS = PERF_TARGET_FPS_PC;

    // --------------------------------------------------------
    // FRAME STATS (read-only, updated every tick)
    // --------------------------------------------------------

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    FPerf_FrameStats CurrentFrameStats;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats",
              meta = (AllowPrivateAccess = "true"))
    EPerf_FrameBudgetStatus BudgetStatus = EPerf_FrameBudgetStatus::Healthy;

    // --------------------------------------------------------
    // LOD SETTINGS
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings DinoLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings FoliageLODSettings;

    // --------------------------------------------------------
    // ADAPTIVE QUALITY
    // --------------------------------------------------------

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Adaptive")
    bool bEnableAdaptiveQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Adaptive")
    float AdaptiveQualityCheckInterval = 5.0f;

    // --------------------------------------------------------
    // FUNCTIONS
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameStats GetCurrentFrameStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_FrameBudgetStatus GetBudgetStatus() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPlatformScalabilitySettings(EPerf_Platform Platform);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, const FPerf_LODSettings& LODSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODToAllDinosaurs();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceAudit();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetFrameTimeBudgetMs() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinFrameBudget() const;

private:
    float TimeSinceLastAdaptiveCheck = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;

    void UpdateFrameStats(float DeltaTime);
    void CheckAdaptiveQuality();
    EPerf_FrameBudgetStatus CalculateBudgetStatus(float FPS) const;
};

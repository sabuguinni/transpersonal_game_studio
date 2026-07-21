// PerformanceOptimizer.h
// Transpersonal Game Studio — Agent #04 Performance Optimizer
// Guarantees 60fps PC / 30fps console across all world zones.
// Pairs with PerformanceOptimizer.cpp

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceOptimizer.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — Perf_ prefix to prevent collisions with other agents' types
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Low        UMETA(DisplayName = "Low — 30fps Console"),
    Medium     UMETA(DisplayName = "Medium — 60fps Mid PC"),
    High       UMETA(DisplayName = "High — 60fps High PC"),
    Ultra      UMETA(DisplayName = "Ultra — 60fps+ High-End PC"),
};

UENUM(BlueprintType)
enum class EPerf_BudgetZone : uint8
{
    OpenWorld  UMETA(DisplayName = "Open World — Full LOD chain"),
    Dense      UMETA(DisplayName = "Dense Vegetation — Foliage budget active"),
    Combat     UMETA(DisplayName = "Combat — AI tick priority"),
    Interior   UMETA(DisplayName = "Interior — Reduced draw distance"),
    Cinematic  UMETA(DisplayName = "Cinematic — Uncapped quality"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds (16.67ms = 60fps, 33.33ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f;

    /** GPU budget in ms — typically 70% of frame time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 11.67f;

    /** CPU budget in ms — typically 30% of frame time */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMs = 5.0f;

    /** Maximum number of dynamic lights in scene */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 8;

    /** Maximum number of active AI agents ticking per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveAIAgents = 50;

    /** Texture streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TextureStreamingPoolMB = 2048;
};

USTRUCT(BlueprintType)
struct FPerf_LODBudget
{
    GENERATED_BODY()

    /** Species or actor category label */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    FString CategoryLabel;

    /** Max draw distance in Unreal Units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxDrawDistanceUU = 6000.0f;

    /** LOD0 to LOD1 transition distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0to1Distance = 1500.0f;

    /** LOD1 to LOD2 transition distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1to2Distance = 3500.0f;

    /** Whether this category uses Nanite (if mesh supports it) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseNanite = false;
};

USTRUCT(BlueprintType)
struct FPerf_ProfileSample
{
    GENERATED_BODY()

    /** Frame time in ms at sample point */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float FrameTimeMs = 0.0f;

    /** GPU time in ms */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    float GPUTimeMs = 0.0f;

    /** Actor count at sample time */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    int32 ActorCount = 0;

    /** Budget zone at sample time */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    EPerf_BudgetZone Zone = EPerf_BudgetZone::OpenWorld;

    /** Whether frame budget was exceeded */
    UPROPERTY(BlueprintReadOnly, Category = "Profiling")
    bool bBudgetExceeded = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// UPerf_PerformanceOptimizer — ActorComponent
// Attach to GameMode or a persistent manager actor.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerf_PerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceOptimizer();

    // ── Lifecycle ──────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ── Configuration ─────────────────────────────────────────────────────

    /** Active quality preset — drives all console var settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_QualityPreset QualityPreset = EPerf_QualityPreset::High;

    /** Current budget zone — updated by world manager */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_BudgetZone CurrentZone = EPerf_BudgetZone::OpenWorld;

    /** Frame budget for current platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_FrameBudget FrameBudget;

    /** LOD budgets per dinosaur species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    TArray<FPerf_LODBudget> DinosaurLODBudgets;

    /** How often (in seconds) to run the performance audit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config", meta = (ClampMin = "0.5", ClampMax = "10.0"))
    float AuditIntervalSeconds = 2.0f;

    /** Enable adaptive quality — auto-adjusts settings when FPS drops */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bEnableAdaptiveQuality = true;

    // ── Runtime State ──────────────────────────────────────────────────────

    /** Last recorded frame time in ms */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime", meta = (AllowPrivateAccess = "true"))
    float LastFrameTimeMs = 0.0f;

    /** Whether the current frame is over budget */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime", meta = (AllowPrivateAccess = "true"))
    bool bIsOverBudget = false;

    /** Number of consecutive over-budget frames */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime", meta = (AllowPrivateAccess = "true"))
    int32 ConsecutiveOverBudgetFrames = 0;

    /** Recent profile samples (ring buffer, last 60 samples) */
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    TArray<FPerf_ProfileSample> ProfileSamples;

    // ── Public API ─────────────────────────────────────────────────────────

    /** Apply quality preset — sets all console vars for the chosen preset */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_QualityPreset Preset);

    /** Set the current budget zone (called by world manager on zone transition) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetBudgetZone(EPerf_BudgetZone NewZone);

    /** Get the LOD budget for a given category label (e.g., "TRex", "Raptor") */
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    FPerf_LODBudget GetLODBudgetForCategory(const FString& CategoryLabel) const;

    /** Force an immediate performance audit — returns true if within budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool RunPerformanceAudit();

    /** Get average frame time from recent samples */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetAverageFrameTimeMs() const;

    /** Get estimated FPS from recent samples */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetEstimatedFPS() const;

    /** Apply draw distance to all actors matching the given label fragment */
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ApplyDrawDistanceToActorsByLabel(const FString& LabelFragment, float MaxDrawDistance);

    /** Initialize default LOD budgets for all dinosaur species */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|LOD")
    void InitializeDefaultDinosaurLODBudgets();

private:
    /** Accumulated time since last audit */
    float TimeSinceLastAudit = 0.0f;

    /** Apply console vars for Low preset */
    void ApplyLowPreset();

    /** Apply console vars for Medium preset */
    void ApplyMediumPreset();

    /** Apply console vars for High preset */
    void ApplyHighPreset();

    /** Apply console vars for Ultra preset */
    void ApplyUltraPreset();

    /** Adaptive quality step-down when over budget */
    void StepDownQuality();

    /** Adaptive quality step-up when consistently under budget */
    void StepUpQuality();

    /** Execute a console command */
    void ExecConsoleCommand(const FString& Command);

    /** Add a profile sample to the ring buffer */
    void AddProfileSample(float FrameMs, float GPUMs, int32 ActorCount, bool bOverBudget);
};

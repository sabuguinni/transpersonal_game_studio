#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PerformanceConfig.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EPerf_QualityPreset — target platform quality tier
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Console    UMETA(DisplayName = "Console (30fps)"),
    PCMid      UMETA(DisplayName = "PC Mid (45fps)"),
    PCHigh     UMETA(DisplayName = "PC High (60fps)"),
    PCUltra    UMETA(DisplayName = "PC Ultra (60fps+)"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_FrameBudget — per-frame ms budget breakdown
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Total frame budget in milliseconds (33.3 = 30fps, 16.6 = 60fps) */
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float TotalBudgetMs = 16.6f;

    /** GPU render budget (ms) */
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float GPURenderMs = 10.0f;

    /** CPU game thread budget (ms) */
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float CPUGameThreadMs = 4.0f;

    /** AI / behavior tree budget (ms) */
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float AIBudgetMs = 1.5f;

    /** Physics simulation budget (ms) */
    UPROPERTY(BlueprintReadWrite, Category = "Performance")
    float PhysicsBudgetMs = 1.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_LODSettings — LOD thresholds per quality preset
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    /** Skeletal mesh LOD bias (0 = full quality, 1 = one LOD down) */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 SkeletalLODBias = 0;

    /** Static mesh LOD distance scale multiplier */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float StaticLODDistanceScale = 1.0f;

    /** Foliage density scale (0.0–1.0) */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float FoliageDensityScale = 1.0f;

    /** Shadow cascade count (1–4) */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 ShadowCascades = 3;

    /** Shadow distance scale multiplier */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    float ShadowDistanceScale = 1.0f;

    /** Max anisotropy level (1, 2, 4, 8, 16) */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 MaxAnisotropy = 8;

    /** Texture streaming pool size in MB */
    UPROPERTY(BlueprintReadWrite, Category = "LOD")
    int32 TextureStreamingPoolMB = 2048;
};

// ─────────────────────────────────────────────────────────────────────────────
// UPerformanceConfig — WorldSubsystem that applies and tracks perf settings
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerformanceConfig : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Quality Preset ────────────────────────────────────────────────────────

    /** Apply a full quality preset — sets all LOD/shadow/foliage CVars */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_QualityPreset Preset);

    /** Get the currently active quality preset */
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_QualityPreset GetCurrentPreset() const { return CurrentPreset; }

    // ── Frame Budget ──────────────────────────────────────────────────────────

    /** Get the frame budget for the current preset */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const { return CurrentBudget; }

    /** Get the LOD settings for the current preset */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const { return CurrentLOD; }

    // ── Runtime Queries ───────────────────────────────────────────────────────

    /** Returns true if the last measured frame time exceeded the budget */
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsOverBudget() const { return bOverBudget; }

    /** Returns the last measured frame time in ms */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetLastFrameTimeMs() const { return LastFrameTimeMs; }

    /** Force-apply all performance console commands for the current preset */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ForceApplyConsoleCommands();

private:
    /** Build LOD settings for a given preset */
    FPerf_LODSettings BuildLODSettings(EPerf_QualityPreset Preset) const;

    /** Build frame budget for a given preset */
    FPerf_FrameBudget BuildFrameBudget(EPerf_QualityPreset Preset) const;

    /** Execute a console command in the current world */
    void ExecCmd(const FString& Cmd) const;

    UPROPERTY()
    EPerf_QualityPreset CurrentPreset = EPerf_QualityPreset::PCHigh;

    UPROPERTY()
    FPerf_FrameBudget CurrentBudget;

    UPROPERTY()
    FPerf_LODSettings CurrentLOD;

    bool bOverBudget = false;
    float LastFrameTimeMs = 0.0f;
};

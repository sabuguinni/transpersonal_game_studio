// PerformanceOptimizer.h
// Performance Optimizer Agent #04 — Transpersonal Game Studio
// Runtime performance manager: LOD, culling, scalability, frame budget
// Cycle: PROD_CYCLE_AUTO_20260622_008

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "PerformanceOptimizer.generated.h"

// ============================================================
// ENUMS (global scope — UHT requirement)
// ============================================================

/** Target platform for scalability preset selection */
UENUM(BlueprintType)
enum class EPerf_Platform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End (60fps)"),
    PC_Medium   UMETA(DisplayName = "PC Medium (60fps)"),
    PC_Low      UMETA(DisplayName = "PC Low (30fps)"),
    Console     UMETA(DisplayName = "Console (30fps)"),
    Mobile      UMETA(DisplayName = "Mobile (30fps)")
};

/** Current performance health status */
UENUM(BlueprintType)
enum class EPerf_HealthStatus : uint8
{
    Optimal     UMETA(DisplayName = "Optimal (>55fps)"),
    Acceptable  UMETA(DisplayName = "Acceptable (45-55fps)"),
    Degraded    UMETA(DisplayName = "Degraded (30-45fps)"),
    Critical    UMETA(DisplayName = "Critical (<30fps)")
};

// ============================================================
// STRUCTS (global scope — UHT requirement)
// ============================================================

/** Frame budget allocation per system */
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Total frame budget in milliseconds (16.67ms = 60fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TotalBudgetMs = 16.67f;

    /** CPU game thread budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CpuGameThreadMs = 6.0f;

    /** CPU render thread budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CpuRenderThreadMs = 5.0f;

    /** GPU budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GpuMs = 12.0f;

    /** Dinosaur AI tick budget (ms) — shared across all dino pawns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DinoAiBudgetMs = 2.0f;

    /** Physics simulation budget (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsBudgetMs = 2.5f;

    /** Max simultaneous dinosaur pawns before LOD culling kicks in */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousDinos = 10;

    /** Max static mesh actors before streaming LOD enforced */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxStaticMeshActors = 500;
};

/** Scalability preset for a given platform */
USTRUCT(BlueprintType)
struct FPerf_ScalabilityPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_Platform Platform = EPerf_Platform::PC_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ResolutionQuality = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TextureQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 EffectsQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ViewDistanceQuality = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowMaxResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TextureStreamingPoolMB = 512.0f;
};

// ============================================================
// MAIN CLASS
// ============================================================

/**
 * APerformanceOptimizer
 *
 * Placed once in the level (or spawned by GameMode).
 * Monitors frame budget, applies scalability presets,
 * enforces LOD/culling settings, and logs performance warnings.
 *
 * Usage:
 *   - Place in MinPlayableMap
 *   - Set TargetPlatform in Details panel
 *   - Call ApplyScalabilityPreset() from BP or GameMode
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Optimizer"))
class TRANSPERSONALGAME_API APerformanceOptimizer : public AActor
{
    GENERATED_BODY()

public:
    APerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --------------------------------------------------------
    // Configuration
    // --------------------------------------------------------

    /** Target platform — determines which scalability preset is applied on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_Platform TargetPlatform = EPerf_Platform::PC_High;

    /** Frame budget for this platform */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_FrameBudget FrameBudget;

    /** Whether to auto-apply scalability preset on BeginPlay */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bAutoApplyOnBeginPlay = true;

    /** Whether to log performance warnings to output log */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bEnablePerfWarnings = true;

    /** Interval in seconds between performance checks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float PerfCheckIntervalSeconds = 5.0f;

    // --------------------------------------------------------
    // Runtime State
    // --------------------------------------------------------

    /** Current performance health */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|State",
        meta = (AllowPrivateAccess = "true"))
    EPerf_HealthStatus CurrentHealth = EPerf_HealthStatus::Optimal;

    /** Smoothed frame time (ms) over last 60 frames */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|State")
    float SmoothedFrameTimeMs = 0.0f;

    /** Number of consecutive degraded frames before auto-downscale */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance|State")
    int32 DegradedFrameCount = 0;

    // --------------------------------------------------------
    // Public API
    // --------------------------------------------------------

    /** Apply scalability preset for the configured TargetPlatform */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyScalabilityPreset();

    /** Apply a specific preset struct */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPreset(const FPerf_ScalabilityPreset& Preset);

    /** Get preset struct for a given platform */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_ScalabilityPreset GetPresetForPlatform(EPerf_Platform Platform) const;

    /** Force LOD bias on all static mesh actors in the current level */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void EnforceLODOnAllMeshes();

    /** Enable/disable Lumen GI and reflections */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLumenEnabled(bool bEnabled);

    /** Enable/disable Nanite */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetNaniteEnabled(bool bEnabled);

    /** Get current health status */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EPerf_HealthStatus GetHealthStatus() const { return CurrentHealth; }

    /** Get smoothed FPS */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    float GetSmoothedFPS() const;

private:
    float TimeSinceLastPerfCheck = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameTimesamples = 0;

    void UpdatePerformanceHealth(float DeltaTime);
    void ExecuteConsoleCommand(const FString& Command);
    void LogPerfWarning(const FString& Message) const;
};

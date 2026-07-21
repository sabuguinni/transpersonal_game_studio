// PerformanceSubsystem.h
// Agent #04 — Performance Optimizer | Cycle PROD_CYCLE_AUTO_20260701_004
// UWorld subsystem that enforces 60fps PC / 30fps console budget
// Manages LOD bias, cull distances, shadow quality, and tick batching

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "PerformanceSubsystem.generated.h"

// ============================================================
// Enums — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_60fps        UMETA(DisplayName = "PC 60fps"),
    PC_Ultra        UMETA(DisplayName = "PC Ultra"),
    Custom          UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EPerf_BudgetZone : uint8
{
    Safe            UMETA(DisplayName = "Safe (>55fps)"),
    Warning         UMETA(DisplayName = "Warning (40-55fps)"),
    Critical        UMETA(DisplayName = "Critical (<40fps)"),
    Throttled       UMETA(DisplayName = "Throttled (emergency)")
};

// ============================================================
// Structs — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target milliseconds per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameMs = 16.67f; // 60fps

    // Current measured frame time (ms)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameMs = 0.0f;

    // Draw calls this frame
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    // GPU time (ms)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMs = 0.0f;

    // CPU game thread time (ms)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUGameMs = 0.0f;

    // Current budget zone
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_BudgetZone BudgetZone = EPerf_BudgetZone::Safe;
};

USTRUCT(BlueprintType)
struct FPerf_LODConfig
{
    GENERATED_BODY()

    // Static mesh LOD distance multiplier (lower = more aggressive LOD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float StaticMeshLODDistanceScale = 0.8f;

    // Skeletal mesh LOD bias (higher = lower quality sooner)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0", ClampMax = "3"))
    int32 SkeletalMeshLODBias = 1;

    // Max cull distance for foliage (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 15000.0f;

    // Max cull distance for props (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float PropCullDistance = 8000.0f;

    // Max cull distance for dinosaur pawns (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurCullDistance = 25000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowConfig
{
    GENERATED_BODY()

    // Shadow map max resolution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows", meta = (ClampMin = "256", ClampMax = "4096"))
    int32 MaxShadowResolution = 1024;

    // CSM cascade count
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows", meta = (ClampMin = "1", ClampMax = "4"))
    int32 CSMMaxCascades = 3;

    // Shadow distance scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float DistanceScale = 0.6f;

    // Radius threshold for shadow casting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows", meta = (ClampMin = "0.001", ClampMax = "0.1"))
    float RadiusThreshold = 0.03f;
};

// ============================================================
// Main Subsystem Class
// ============================================================

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerformanceSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceSubsystem();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // ============================================================
    // Public API — Blueprint callable
    // ============================================================

    /** Apply a quality preset (sets all CVars atomically) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_QualityPreset Preset);

    /** Get current frame budget status */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const;

    /** Get current budget zone */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EPerf_BudgetZone GetBudgetZone() const;

    /** Apply LOD configuration */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODConfig(const FPerf_LODConfig& Config);

    /** Apply shadow configuration */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyShadowConfig(const FPerf_ShadowConfig& Config);

    /** Enable/disable adaptive quality (auto-scales settings based on FPS) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAdaptiveQuality(bool bEnable);

    /** Force a full performance audit — logs all heavy actors */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceAudit();

    /** Get recommended max dinosaur count for current hardware */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    int32 GetRecommendedDinosaurCount() const;

    // ============================================================
    // Config Properties
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_QualityPreset ActivePreset = EPerf_QualityPreset::PC_60fps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_LODConfig LODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_ShadowConfig ShadowConfig;

    /** Target FPS — subsystem will throttle quality to maintain this */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config", meta = (ClampMin = "20", ClampMax = "144"))
    float TargetFPS = 60.0f;

    /** Enable adaptive quality scaling */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bAdaptiveQuality = true;

    /** Survival tick batch interval (seconds) — batches all SurvivalComponent ticks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Survival", meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float SurvivalTickBatchInterval = 0.1f;

    /** Max dinosaurs to tick AI per frame (spread over multiple frames) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI", meta = (ClampMin = "1", ClampMax = "20"))
    int32 MaxDinosaurAITicksPerFrame = 5;

private:
    // Current frame budget data
    FPerf_FrameBudget CurrentBudget;

    // Adaptive quality state
    float AdaptiveQualityTimer = 0.0f;
    int32 FramesBelow40fps = 0;
    int32 FramesAbove55fps = 0;

    // Apply console variable
    void SetCVar(const FString& CVarName, float Value);
    void SetCVar(const FString& CVarName, int32 Value);

    // Evaluate current budget zone from frame time
    EPerf_BudgetZone EvaluateBudgetZone(float FrameMs) const;

    // Apply preset-specific CVars
    void ApplyConsole30fps();
    void ApplyPC60fps();
    void ApplyPCUltra();
};

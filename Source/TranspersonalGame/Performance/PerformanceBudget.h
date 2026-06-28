// PerformanceBudget.h
// Agent #4 — Performance Optimizer
// Transpersonal Game Studio — Prehistoric Survival Game
// Defines hard performance budgets enforced at runtime for 60fps PC / 30fps console

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudget.generated.h"

// ============================================================
// ENUMS — prefixed Perf_ to avoid collision with other agents
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_Medium_60fps UMETA(DisplayName = "PC Medium 60fps"),
    PC_High_60fps   UMETA(DisplayName = "PC High 60fps"),
    PC_Ultra_60fps  UMETA(DisplayName = "PC Ultra 60fps"),
};

UENUM(BlueprintType)
enum class EPerf_BottleneckType : uint8
{
    None        UMETA(DisplayName = "No Bottleneck"),
    CPU_Bound   UMETA(DisplayName = "CPU Bound"),
    GPU_Bound   UMETA(DisplayName = "GPU Bound"),
    Draw_Bound  UMETA(DisplayName = "Draw Call Bound"),
    Memory_Bound UMETA(DisplayName = "Memory Bound"),
};

// ============================================================
// STRUCTS — performance budget definitions per quality preset
// ============================================================

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFrameTimeMs = 16.667f; // 60fps default

    // Max draw calls per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDrawCalls = 500;

    // Max shadow-casting dynamic lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxShadowCastingLights = 4;

    // Max simultaneous NavMesh agents (dinosaurs + NPCs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxNavAgents = 200;

    // Max physics substeps per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxPhysicsSubsteps = 4;

    // Texture streaming pool in MB
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 TextureStreamingPoolMB = 2048;

    // World partition streaming radius in Unreal units
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float StreamingRadius = 25600.0f;

    // GC purge interval in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GCPurgeIntervalSeconds = 60.0f;

    // Enable Lumen GI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    bool bEnableLumen = true;

    // Enable Nanite
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    bool bEnableNanite = true;

    // Enable URO (Update Rate Optimization for distant animations)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    bool bEnableURO = true;

    FPerf_FrameBudget() = default;
};

USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    // Current frame time in ms
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    float CurrentFrameTimeMs = 0.0f;

    // Current draw call count (approximated)
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 CurrentDrawCalls = 0;

    // Detected bottleneck type
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    EPerf_BottleneckType Bottleneck = EPerf_BottleneckType::None;

    // Whether we are within budget
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    bool bWithinBudget = true;

    // Frames since last budget violation
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 FramesSinceViolation = 0;

    FPerf_FrameStats() = default;
};

// ============================================================
// UCLASS — PerformanceBudgetManager
// Singleton-style UObject that holds budget config and
// monitors runtime performance. Attach to GameInstance.
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Performance")
class TRANSPERSONALGAME_API UPerformanceBudgetManager : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceBudgetManager();

    // ---- Budget Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_QualityPreset ActivePreset = EPerf_QualityPreset::PC_High_60fps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_FrameBudget CurrentBudget;

    // ---- Runtime Stats ----

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    FPerf_FrameStats CurrentStats;

    // ---- Budget Presets ----

    // Returns the budget definition for a given quality preset
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetBudgetForPreset(EPerf_QualityPreset Preset) const;

    // Apply a quality preset — sets console variables and updates CurrentBudget
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPreset(EPerf_QualityPreset Preset);

    // ---- Runtime Monitoring ----

    // Called every frame by the owning GameInstance or GameMode
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void Tick(float DeltaTime);

    // Returns true if the current frame is within budget
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsWithinBudget() const;

    // Returns the detected bottleneck type
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    EPerf_BottleneckType GetBottleneck() const;

    // ---- Survival-Specific Helpers ----

    // Call when a dinosaur herd spawns — checks if NavAgent budget allows it
    UFUNCTION(BlueprintCallable, Category = "Performance|Survival")
    bool CanSpawnNavAgent(int32 RequestedCount) const;

    // Call when a new biome loads — adjusts streaming radius based on biome complexity
    UFUNCTION(BlueprintCallable, Category = "Performance|Survival")
    void OnBiomeChanged(FName BiomeName, float BiomeComplexityFactor);

    // Disable shadows on a PointLight if over shadow budget
    UFUNCTION(BlueprintCallable, Category = "Performance|Survival")
    void EnforceShadowBudget(class APointLight* Light);

private:
    // Track active NavAgent count
    int32 ActiveNavAgentCount = 0;

    // Accumulate frame time samples for rolling average
    float FrameTimeSampleAccum = 0.0f;
    int32 FrameTimeSampleCount = 0;
    static constexpr int32 FrameTimeSampleWindow = 30;

    // Apply console variable settings for a budget
    void ApplyConsoleVars(const FPerf_FrameBudget& Budget);
};

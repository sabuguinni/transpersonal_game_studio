// PerformanceBudgetManager.h
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260622_011
// Runtime performance budget enforcement: 60fps PC / 30fps console
// Monitors frame time, draw calls, memory, and applies dynamic scalability

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "PerformanceBudgetManager.generated.h"

// --- Scalability tier enum (global scope, Perf_ prefix) ---
UENUM(BlueprintType)
enum class EPerf_ScalabilityTier : uint8
{
    Low      UMETA(DisplayName = "Low (Console 30fps)"),
    Medium   UMETA(DisplayName = "Medium (PC 30fps)"),
    High     UMETA(DisplayName = "High (PC 60fps)"),
    Epic     UMETA(DisplayName = "Epic (PC 60fps Ultra)")
};

// --- Per-frame budget snapshot ---
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS = 16.67f;   // Target: 16.67ms @ 60fps

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bBudgetExceeded = false;
};

// --- LOD distance config per scalability tier ---
USTRUCT(BlueprintType)
struct FPerf_LODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float StaticMeshLODScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SkeletalMeshLODScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageLODScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageDensityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ShadowDistanceScale = 1.0f;
};

/**
 * APerf_BudgetManager
 * Placed once in the level. Monitors frame budget and applies
 * dynamic scalability adjustments to maintain target FPS.
 * 
 * Budget targets:
 *   PC High-end:  60fps = 16.67ms total (Game 6ms, Render 6ms, GPU 10ms)
 *   Console:      30fps = 33.33ms total (Game 10ms, Render 10ms, GPU 20ms)
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Budget Manager"))
class TRANSPERSONALGAME_API APerf_BudgetManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_BudgetManager();

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float TargetFrameTimeMS = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float GameThreadBudgetMS = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float RenderThreadBudgetMS = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float GPUBudgetMS = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Target")
    float MemoryBudgetMB = 2048.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Scalability")
    EPerf_ScalabilityTier CurrentTier = EPerf_ScalabilityTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Scalability")
    bool bEnableDynamicScalability = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Scalability")
    float ScalabilityCheckIntervalSeconds = 2.0f;

    // --- LOD configs per tier ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig LODConfig_Low;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig LODConfig_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig LODConfig_High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig LODConfig_Epic;

    // --- Runtime state (read-only) ---
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    FPerf_FrameBudget CurrentBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    int32 BudgetViolationCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Runtime")
    bool bIsUnderBudget = true;

    // --- Blueprint-callable API ---

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetScalabilityTier(EPerf_ScalabilityTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODConfig(const FPerf_LODConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetCurrentFrameBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceScalabilityCheck();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const { return AverageFPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsUnderBudget() const { return bIsUnderBudget; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_ScalabilityTier GetCurrentTier() const { return CurrentTier; }

    // --- AActor overrides ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

#if WITH_EDITOR
    UFUNCTION(CallInEditor, Category = "Performance")
    void PrintBudgetReport();
#endif

private:
    float ScalabilityCheckTimer = 0.0f;
    TArray<float> FPSSamples;
    static const int32 FPS_SAMPLE_COUNT = 60;

    void UpdateFPSSamples(float DeltaTime);
    void CheckBudgetAndAdjust();
    void ApplyTierConfig(EPerf_ScalabilityTier Tier);
    void ExecuteConsoleCommand(const FString& Command);
    FPerf_LODConfig GetLODConfigForTier(EPerf_ScalabilityTier Tier) const;
    void InitDefaultLODConfigs();
};

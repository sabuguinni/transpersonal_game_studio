// DynamicPerformanceManager.h
// Performance Optimizer — Agent #04
// PROD_CYCLE_AUTO_20260620_005
// World subsystem for dynamic runtime performance scaling.
// Monitors FPS every 2s and adjusts quality CVars to maintain target framerate.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "DynamicPerformanceManager.generated.h"

// Quality tiers — maps to CVar presets applied at runtime
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low     = 0  UMETA(DisplayName = "Low"),
    Medium  = 1  UMETA(DisplayName = "Medium"),
    High    = 2  UMETA(DisplayName = "High"),
    Ultra   = 3  UMETA(DisplayName = "Ultra"),
};

// Frame budget breakdown in milliseconds
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TotalBudgetMs = 16.67f;   // 60fps default

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 9.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 5.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMs = 10.8f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AIBudgetMs = 1.67f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsBudgetMs = 1.33f;
};

// Delegate broadcast when quality tier changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnQualityTierChanged, EPerf_QualityTier, NewTier);

/**
 * UDynamicPerformanceManager
 * World subsystem that monitors runtime FPS and dynamically adjusts
 * rendering CVars to maintain target framerate (60fps PC / 30fps console).
 *
 * Usage:
 *   auto* Mgr = GetWorld()->GetSubsystem<UDynamicPerformanceManager>();
 *   Mgr->RecordFrameTime(DeltaSeconds);  // call from GameMode Tick
 *   FPerf_FrameBudget Budget = Mgr->GetFrameBudget();
 */
UCLASS(BlueprintType, meta = (DisplayName = "Dynamic Performance Manager"))
class TRANSPERSONALGAME_API UDynamicPerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UDynamicPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // --- Public API ---

    /** Record a frame delta for FPS averaging. Call from GameMode or Character Tick. */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RecordFrameTime(float DeltaSeconds);

    /** Force a specific quality tier immediately (bypasses FPS evaluation). */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceQualityTier(EPerf_QualityTier Tier);

    /** Returns current smoothed FPS. */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCurrentFPS() const;

    /** Returns active quality tier. */
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_QualityTier GetCurrentQualityTier() const;

    /** Returns frame budget breakdown for current target platform. */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const;

    /** Returns target FPS for current platform (60 PC / 30 console). */
    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetTargetFPS() const;

    /** Fired whenever quality tier changes. Bind in Blueprint or C++ to react. */
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnQualityTierChanged OnQualityTierChanged;

private:
    void EvaluatePerformance();
    void ApplyQualityTier(EPerf_QualityTier NewTier);
    void ApplyCVar(IConsoleManager& CM, const TCHAR* Name, const TCHAR* Value);

    UPROPERTY()
    float TargetFPS_PC;

    UPROPERTY()
    float TargetFPS_Console;

    UPROPERTY()
    float CurrentFPS;

    UPROPERTY()
    bool bScalingActive;

    UPROPERTY()
    float ScaleCheckInterval;

    UPROPERTY()
    EPerf_QualityTier CurrentQualityTier;

    float FrameTimeAccumulator;
    int32 FrameSampleCount;

    FTimerHandle ScaleCheckHandle;
};

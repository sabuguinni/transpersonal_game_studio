// PerformanceBudget.h
// Performance Optimizer — Agent #04
// Defines frame budget constants and CVar profiles for 60fps PC / 30fps console.
// All types use Perf_ prefix per RULE 2.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudget.generated.h"

// ---------------------------------------------------------------------------
// Enums — global scope (RULE 1)
// ---------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Console     UMETA(DisplayName = "Console (30fps)"),
    PCMedium    UMETA(DisplayName = "PC Medium (60fps)"),
    PCHigh      UMETA(DisplayName = "PC High (60fps)"),
    PCUltra     UMETA(DisplayName = "PC Ultra (60fps+)"),
};

UENUM(BlueprintType)
enum class EPerf_BudgetZone : uint8
{
    Safe        UMETA(DisplayName = "Safe (<80% budget)"),
    Warning     UMETA(DisplayName = "Warning (80-95% budget)"),
    Critical    UMETA(DisplayName = "Critical (>95% budget)"),
};

// ---------------------------------------------------------------------------
// Structs — global scope (RULE 1)
// ---------------------------------------------------------------------------

/** Per-frame budget breakdown in milliseconds */
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Total frame budget in ms (16.67ms = 60fps, 33.33ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TotalBudgetMs = 16.67f;

    /** GPU render thread budget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderBudgetMs = 10.0f;

    /** CPU game thread budget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadBudgetMs = 4.0f;

    /** CPU render thread budget */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadBudgetMs = 3.0f;

    /** Remaining headroom */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float HeadroomMs = 0.0f;
};

/** CVar profile snapshot — applied at runtime per quality preset */
USTRUCT(BlueprintType)
struct FPerf_CVarProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 ShadowMaxResolution = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float ShadowRadiusThreshold = 0.03f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 ShadowCSMMaxCascades = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float LumenMaxTraceDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float LumenReflectionsMaxRoughness = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float FoliageLODDistanceScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float StaticMeshLODDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    int32 StreamingPoolSizeMB = 1024;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Nanite")
    float NaniteMaxPixelsPerEdge = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AA")
    float TSRHistoryScreenPercentage = 200.0f;
};

// ---------------------------------------------------------------------------
// UPerformanceBudgetManager — manages frame budget and applies CVar profiles
// ---------------------------------------------------------------------------

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Performance Budget Manager"))
class TRANSPERSONALGAME_API UPerformanceBudgetManager : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceBudgetManager();

    /** Apply a CVar profile to the engine — call on quality preset change */
    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor)
    void ApplyCVarProfile(EPerf_QualityPreset Preset);

    /** Get the frame budget for the current preset */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget(EPerf_QualityPreset Preset) const;

    /** Evaluate current budget zone based on measured frame time */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_BudgetZone EvaluateBudgetZone(float MeasuredFrameTimeMs, EPerf_QualityPreset Preset) const;

    /** Get the CVar profile for a given preset */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_CVarProfile GetCVarProfile(EPerf_QualityPreset Preset) const;

    /** Currently active preset */
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_QualityPreset ActivePreset = EPerf_QualityPreset::PCMedium;

    /** Maximum actor count before LOD bias is increased */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxActorCountBeforeLODBias = 500;

    /** Maximum dynamic light count */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxDynamicLights = 8;

    /** Maximum simultaneous Niagara particle systems */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxNiagaraSystems = 32;

private:
    /** Build preset profiles */
    FPerf_CVarProfile BuildConsoleProfile() const;
    FPerf_CVarProfile BuildPCMediumProfile() const;
    FPerf_CVarProfile BuildPCHighProfile() const;
    FPerf_CVarProfile BuildPCUltraProfile() const;

    /** Apply a single CVar */
    void ApplyCVar(const FString& Name, float Value) const;
    void ApplyCVar(const FString& Name, int32 Value) const;
};

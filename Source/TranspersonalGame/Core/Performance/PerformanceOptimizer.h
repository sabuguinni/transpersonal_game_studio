// PerformanceOptimizer.h — Agent #04 Performance Optimizer
// Guarantees 60fps PC / 30fps console via dynamic LOD, cull distance, and tick throttling
// Prefix: Perf_ for all types

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "PerformanceOptimizer.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (Console Budget)"),
    Potato      UMETA(DisplayName = "Potato (Minimum Spec)")
};

UENUM(BlueprintType)
enum class EPerf_ActorCategory : uint8
{
    DinosaurPawn    UMETA(DisplayName = "Dinosaur Pawn"),
    Vegetation      UMETA(DisplayName = "Vegetation / Foliage"),
    Rock            UMETA(DisplayName = "Rock / Terrain Prop"),
    Structure       UMETA(DisplayName = "Structure / Building"),
    Particle        UMETA(DisplayName = "Particle / VFX"),
    Light           UMETA(DisplayName = "Dynamic Light"),
    Generic         UMETA(DisplayName = "Generic Actor")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_CullDistanceEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_ActorCategory Category = EPerf_ActorCategory::Generic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceScale = 1.0f;
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps

    // Game thread budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadBudgetMs = 6.0f;

    // Render thread budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadBudgetMs = 8.0f;

    // GPU budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 14.0f;

    // Max dynamic lights in scene
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 20;

    // Max simultaneous particle systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxParticleSystems = 50;

    // Max visible dinosaur pawns
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleDinosaurs = 15;
};

USTRUCT(BlueprintType)
struct FPerf_TickProfile
{
    GENERATED_BODY()

    // Class name being profiled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FName ClassName;

    // Recommended tick interval (0 = every frame, 0.1 = 10Hz, 2.0 = 0.5Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RecommendedTickInterval = 0.0f;

    // Average tick cost in microseconds (measured)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AvgTickCostUs = 0.0f;

    // Whether this component should use async tick
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseAsyncTick = false;
};

USTRUCT(BlueprintType)
struct FPerf_FrameSample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DeltaTimeMs = 0.0f;
};

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Configuration ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    FPerf_FrameBudget FrameBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    TArray<FPerf_CullDistanceEntry> CullDistanceTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    TArray<FPerf_TickProfile> TickProfiles;

    // How often to run the full optimization pass (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float OptimizationPassInterval = 5.0f;

    // Enable dynamic quality scaling based on FPS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bEnableDynamicQuality = true;

    // FPS threshold below which we drop quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config", meta = (ClampMin = "20.0", ClampMax = "60.0"))
    float FPSDropThreshold = 45.0f;

    // FPS threshold above which we raise quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config", meta = (ClampMin = "30.0", ClampMax = "120.0"))
    float FPSRaiseThreshold = 58.0f;

    // ─── Runtime State ───────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    FPerf_FrameSample CurrentFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    float AverageFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    int32 FrameSampleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|State")
    bool bIsUnderBudget = true;

    // ─── Functions ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RunOptimizationPass();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyCullDistances();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ThrottleSurvivalTick(float DesiredHz = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ThrottleCharacterTick(float DesiredHz = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameSample GetCurrentFrameStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsFrameUnderBudget() const { return bIsUnderBudget; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetTargetFPS() const;

    UFUNCTION(CallInEditor, Category = "Performance")
    void PrintPerformanceReport();

private:
    float TimeSinceLastOptPass = 0.0f;
    float FPSAccumulator = 0.0f;
    int32 FPSFrameCount = 0;
    float FPSAverageWindow = 3.0f; // Average over 3 seconds

    void SampleFrameTime(float DeltaTime);
    void CheckBudgetViolations();
    void AutoAdjustQuality();
    void ApplyQualityTierCVars(EPerf_QualityTier Tier);
    void InitializeDefaultCullDistances();
    void InitializeDefaultTickProfiles();
};

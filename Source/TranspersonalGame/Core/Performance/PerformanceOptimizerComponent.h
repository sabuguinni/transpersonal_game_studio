// PerformanceOptimizerComponent.h
// Performance Optimizer — Agent #4
// Monitors and enforces frame budget across all game systems.
// Target: 60fps PC / 30fps console.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceOptimizerComponent.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid)"),
    Medium      UMETA(DisplayName = "Medium (PC Low / Console)"),
    Low         UMETA(DisplayName = "Low (Console Perf Mode)"),
    Minimum     UMETA(DisplayName = "Minimum (Emergency)")
};

UENUM(BlueprintType)
enum class EPerf_FrameBudgetZone : uint8
{
    Green       UMETA(DisplayName = "Green (>55fps)"),
    Yellow      UMETA(DisplayName = "Yellow (40-55fps)"),
    Orange      UMETA(DisplayName = "Orange (25-40fps)"),
    Red         UMETA(DisplayName = "Red (<25fps — critical)")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_FrameBudgetZone BudgetZone = EPerf_FrameBudgetZone::Green;
};

USTRUCT(BlueprintType)
struct FPerf_TickBudget
{
    GENERATED_BODY()

    /** System name for logging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FName SystemName;

    /** Recommended tick interval in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TickIntervalSeconds = 0.1f;

    /** Max allowed time per tick in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxTickTimeMS = 0.5f;

    /** Whether this system can be throttled under load */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCanThrottle = true;
};

// ─── Delegates ───────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPerf_OnQualityTierChanged,
    EPerf_QualityTier, OldTier, EPerf_QualityTier, NewTier);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnFrameBudgetViolation,
    EPerf_FrameBudgetZone, Zone);

// ─── Component ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent),
    DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerformanceOptimizerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizerComponent();

    // ── Lifecycle ──
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // ── Quality Tier ──
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_QualityTier GetCurrentQualityTier() const { return CurrentQualityTier; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AutoDetectQualityTier();

    // ── Frame Stats ──
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameStats GetCurrentFrameStats() const { return CurrentFrameStats; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_FrameBudgetZone GetBudgetZone() const { return CurrentFrameStats.BudgetZone; }

    // ── Tick Budget ──
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterSystemTickBudget(FName SystemName, float TickIntervalSeconds, float MaxTickTimeMS, bool bCanThrottle);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetRecommendedTickInterval(FName SystemName) const;

    // ── Cull Distance ──
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyCullDistanceToActor(AActor* Target, float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyCullDistanceToAllActorsInRadius(float Radius, float CullDistance);

    // ── Console Commands ──
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyPerformancePreset();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void PrintFrameBudgetReport();

    // ── Delegates ──
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnQualityTierChanged OnQualityTierChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FPerf_OnFrameBudgetViolation OnFrameBudgetViolation;

    // ── Config ──
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    EPerf_QualityTier DefaultQualityTier = EPerf_QualityTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float TargetFPS_PC = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float TargetFPS_Console = 30.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    bool bAutoAdjustQuality = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Config")
    float QualityAdjustCooldownSeconds = 5.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budgets")
    TArray<FPerf_TickBudget> SystemTickBudgets;

private:
    UPROPERTY()
    EPerf_QualityTier CurrentQualityTier = EPerf_QualityTier::High;

    UPROPERTY()
    FPerf_FrameStats CurrentFrameStats;

    // Rolling average for FPS
    TArray<float> FPSHistory;
    static constexpr int32 FPS_HISTORY_SIZE = 60;

    float TimeSinceLastQualityAdjust = 0.f;
    float MonitorTickAccumulator = 0.f;
    static constexpr float MONITOR_TICK_INTERVAL = 0.5f; // check every 500ms

    void UpdateFrameStats(float DeltaTime);
    void CheckAndAdjustQuality();
    void ApplyQualitySettings(EPerf_QualityTier Tier);
    EPerf_FrameBudgetZone ClassifyFPS(float FPS) const;

    // Console command helpers
    void ExecConsoleCommand(const FString& Command) const;
};

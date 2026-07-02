// PerformanceSubsystem.h
// Agent #04 — Performance Optimizer
// UWorld subsystem managing LOD budgets, draw call limits, and scalability for 60fps PC / 30fps console.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/EngineTypes.h"
#include "PerformanceSubsystem.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_Medium_60fps UMETA(DisplayName = "PC Medium 60fps"),
    PC_High_60fps   UMETA(DisplayName = "PC High 60fps"),
    PC_Ultra_60fps  UMETA(DisplayName = "PC Ultra 60fps"),
};

UENUM(BlueprintType)
enum class EPerf_BudgetStatus : uint8
{
    Healthy  UMETA(DisplayName = "Healthy"),
    Warning  UMETA(DisplayName = "Warning"),
    Critical UMETA(DisplayName = "Critical"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.67f;   // 60fps default

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CPUBudgetMs = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 800;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDynamicLights = 16;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActors = 500;
};

USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float WorstFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentDynamicLightCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_BudgetStatus BudgetStatus = EPerf_BudgetStatus::Healthy;
};

// ─── Subsystem ────────────────────────────────────────────────────────────────

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

    // ── Quality Tier ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetQualityTier(EPerf_QualityTier NewTier);

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_QualityTier GetQualityTier() const { return CurrentTier; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetCurrentBudget() const { return CurrentBudget; }

    // ── Frame Stats ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameStats GetFrameStats() const { return FrameStats; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_BudgetStatus GetBudgetStatus() const { return FrameStats.BudgetStatus; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsOverBudget() const { return FrameStats.BudgetStatus == EPerf_BudgetStatus::Critical; }

    // ── LOD Management ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetGlobalLODDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetFoliageLODDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ApplyLODSettingsForTier(EPerf_QualityTier Tier);

    // ── Cull Distance ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetMaxDrawDistanceForSmallActors(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void SetMaxDrawDistanceForLargeActors(float Distance);

    // ── Shadow Management ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance|Shadows")
    void SetShadowMaxResolution(int32 Resolution);

    UFUNCTION(BlueprintCallable, Category = "Performance|Shadows")
    void SetShadowDistanceScale(float Scale);

    // ── Diagnostics ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance|Diagnostics")
    void RunPerformanceAudit();

    UFUNCTION(BlueprintCallable, Category = "Performance|Diagnostics")
    FString GetPerformanceReport() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Diagnostics")
    void LogPerformanceSummary();

    // ── Delegates ─────────────────────────────────────────────────────────────

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBudgetStatusChanged, EPerf_BudgetStatus, NewStatus);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnBudgetStatusChanged OnBudgetStatusChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualityTierChanged, EPerf_QualityTier, NewTier);
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnQualityTierChanged OnQualityTierChanged;

private:
    UPROPERTY()
    EPerf_QualityTier CurrentTier = EPerf_QualityTier::PC_High_60fps;

    UPROPERTY()
    FPerf_FrameBudget CurrentBudget;

    UPROPERTY()
    FPerf_FrameStats FrameStats;

    // Frame time ring buffer for averaging
    TArray<float> FrameTimeHistory;
    static constexpr int32 FrameHistorySize = 60;
    int32 FrameHistoryIndex = 0;

    FTimerHandle AuditTimerHandle;

    void ApplyConsoleVarsForTier(EPerf_QualityTier Tier);
    void UpdateFrameStats(float DeltaTime);
    void CheckBudgetStatus();
    void SchedulePeriodicAudit();

    // Budget presets per tier
    FPerf_FrameBudget GetBudgetForTier(EPerf_QualityTier Tier) const;
};

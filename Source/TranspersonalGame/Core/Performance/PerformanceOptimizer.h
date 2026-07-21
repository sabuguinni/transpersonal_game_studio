// PerformanceOptimizer.h
// Agent #4 — Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260628_007
// Monitors and enforces 60fps PC / 30fps console frame budget.
// Manages LOD distances, culling, streaming, and dynamic quality scaling.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "PerformanceOptimizer.generated.h"

// Forward declarations
class UDinosaurBase;
class USurvivalComponent;

// Performance tier — determines quality/performance tradeoff
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (Console Min)"),
    Potato      UMETA(DisplayName = "Potato (Minimum Spec)")
};

// Frame budget status
UENUM(BlueprintType)
enum class EPerf_FrameBudgetStatus : uint8
{
    Green       UMETA(DisplayName = "Green (>50fps)"),
    Yellow      UMETA(DisplayName = "Yellow (30-50fps)"),
    Red         UMETA(DisplayName = "Red (<30fps — Critical)")
};

// Performance snapshot — captured every N seconds
USTRUCT(BlueprintType)
struct FPerf_FrameSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTimeMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCallCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActorCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTimeMS = 0.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_FrameBudgetStatus BudgetStatus = EPerf_FrameBudgetStatus::Green;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CaptureTimestamp = 0.f;
};

// LOD configuration per actor category
USTRUCT(BlueprintType)
struct FPerf_LODConfig
{
    GENERATED_BODY()

    // Distance at which LOD0 transitions to LOD1
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1500.f;

    // Distance at which LOD1 transitions to LOD2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 3000.f;

    // Distance at which actor is culled entirely
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 8000.f;

    // Disable AI/physics beyond this distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float AIDisableDistance = 5000.f;

    // Disable tick beyond this distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float TickDisableDistance = 6000.f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === QUALITY TIER ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityTier CurrentQualityTier = EPerf_QualityTier::High;

    UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
    void SetQualityTier(EPerf_QualityTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance|Quality")
    void AutoDetectQualityTier();

    // === FRAME BUDGET ===

    // Target FPS — 60 for PC, 30 for console
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFPS = 60.f;

    // Minimum acceptable FPS before quality scaling kicks in
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MinAcceptableFPS = 45.f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Budget")
    FPerf_FrameSnapshot LastSnapshot;

    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    FPerf_FrameSnapshot CaptureFrameSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Performance|Budget")
    EPerf_FrameBudgetStatus GetCurrentBudgetStatus() const;

    // === LOD MANAGEMENT ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig DinosaurLODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig FoliageLODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig PropLODConfig;

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ApplyLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetDinosaurAIDistanceCull(float Distance);

    // === DYNAMIC QUALITY SCALING ===

    // Enable automatic quality reduction when FPS drops below MinAcceptableFPS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    bool bEnableDynamicQualityScaling = true;

    // How often to sample FPS for dynamic scaling (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DynamicScaling")
    float SamplingInterval = 3.f;

    UFUNCTION(BlueprintCallable, Category = "Performance|DynamicScaling")
    void EvaluateAndScaleQuality();

    // === SURVIVAL TIMER OPTIMIZATION ===

    // Optimal timer interval for SurvivalComponent (seconds)
    // 5s = 0.2 calls/sec vs 60fps tick = 300x reduction in survival stat overhead
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Survival")
    float OptimalSurvivalTimerInterval = 5.f;

    // Maximum number of active SurvivalComponents before staggering timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Survival")
    int32 MaxSimultaneousSurvivalTimers = 50;

    UFUNCTION(BlueprintCallable, Category = "Performance|Survival")
    float GetOptimalSurvivalInterval(int32 ActiveActorCount) const;

    // === CONSOLE COMMANDS ===

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Debug")
    void ApplyPCHighSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Debug")
    void ApplyConsoleSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance|Debug")
    void PrintPerformanceBudget();

    // === DELEGATES ===

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQualityTierChanged, EPerf_QualityTier, NewTier);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBudgetStatusChanged, EPerf_FrameBudgetStatus, NewStatus);

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnQualityTierChanged OnQualityTierChanged;

    UPROPERTY(BlueprintAssignable, Category = "Performance|Events")
    FOnBudgetStatusChanged OnBudgetStatusChanged;

private:
    // Internal sampling state
    float TimeSinceLastSample = 0.f;
    float FPSAccumulator = 0.f;
    int32 FPSSampleCount = 0;
    float MinFPSThisInterval = 9999.f;
    EPerf_FrameBudgetStatus LastBudgetStatus = EPerf_FrameBudgetStatus::Green;

    // Apply console variables for a given quality tier
    void ApplyConsoleVariables(EPerf_QualityTier Tier);

    // Stagger survival timers to avoid frame spikes
    void StaggerSurvivalTimers();
};

// PerformanceOptimizer.h
// Prehistoric survival game — Performance Optimizer Component
// Manages LOD, tick rates, shadow quality, and frame budget enforcement
// Agent #4 — Performance Optimizer | Cycle 009

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PerformanceOptimizer.generated.h"

// Performance tier for platform-specific settings
UENUM(BlueprintType)
enum class EPerf_PerformanceTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium      UMETA(DisplayName = "Medium (PC Low / Console)"),
    Low         UMETA(DisplayName = "Low (Minimum Spec)")
};

// Frame budget allocation per system (milliseconds)
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float RenderingBudgetMs = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AIBudgetMs = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float PhysicsBudgetMs = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AudioBudgetMs = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MiscBudgetMs = 1.0f;

    float GetTotalBudgetMs() const { return RenderingBudgetMs + AIBudgetMs + PhysicsBudgetMs + AudioBudgetMs + MiscBudgetMs; }
};

// LOD distance thresholds for dinosaur actors
USTRUCT(BlueprintType)
struct FPerf_DinoLODSettings
{
    GENERATED_BODY()

    // Distance at which dino switches to 20Hz tick (reduced AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float ReducedTickDistance = 3000.0f;

    // Distance at which dino switches to 5Hz tick (minimal AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float MinimalTickDistance = 8000.0f;

    // Distance at which dino is fully culled from AI updates
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float CullDistance = 15000.0f;

    // Full tick rate (close range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float FullTickHz = 60.0f;

    // Reduced tick rate (mid range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float ReducedTickHz = 20.0f;

    // Minimal tick rate (far range)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float MinimalTickHz = 5.0f;
};

// Per-frame performance snapshot
USTRUCT(BlueprintType)
struct FPerf_FrameSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    float FrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 ActiveDinoCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 FullTickDinoCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 ReducedTickDinoCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    int32 CulledDinoCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    bool bBudgetExceeded = false;
};

/**
 * UPerf_PerformanceOptimizer
 * 
 * Actor component that manages runtime performance optimization for the
 * prehistoric survival game. Handles:
 * - Distance-based dinosaur tick rate scaling (60Hz → 20Hz → 5Hz → culled)
 * - Dynamic shadow quality adjustment based on frame time
 * - Frame budget monitoring and alerts
 * - Platform-specific quality tier enforcement
 * 
 * Attach to GameMode or a persistent manager actor.
 */
UCLASS(ClassGroup = (Performance), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerf_PerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceOptimizer();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ============================================================
    // CONFIGURATION
    // ============================================================

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceTier PerformanceTier = EPerf_PerformanceTier::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_DinoLODSettings DinoLODSettings;

    // Target FPS (60 for PC, 30 for console)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "20", ClampMax = "120"))
    float TargetFPS = 60.0f;

    // How often to run the full optimization pass (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float OptimizationInterval = 0.5f;

    // Enable dynamic shadow quality scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDynamicShadowQuality = true;

    // Enable distance-based dino tick scaling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDinoTickScaling = true;

    // ============================================================
    // RUNTIME STATS (read-only)
    // ============================================================

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats", meta = (AllowPrivateAccess = "true"))
    FPerf_FrameSnapshot LastSnapshot;

    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats", meta = (AllowPrivateAccess = "true"))
    float AverageFrameTimeMs = 0.0f;

    // ============================================================
    // BLUEPRINT-CALLABLE FUNCTIONS
    // ============================================================

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTier(EPerf_PerformanceTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceOptimizationPass();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_FrameSnapshot GetCurrentSnapshot() const { return LastSnapshot; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const { return LastSnapshot.FPS; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceHealthy() const { return !LastSnapshot.bBudgetExceeded; }

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void LogPerformanceReport();

private:
    // Internal optimization state
    float TimeSinceLastOptimization = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    TWeakObjectPtr<AActor> CachedPlayerActor;

    // Internal methods
    void RunOptimizationPass();
    void UpdateDinoTickRates();
    void UpdateShadowQuality(float CurrentFrameTimeMs);
    void ApplyTierSettings(EPerf_PerformanceTier Tier);
    void CachePlayerActor();
    float GetTargetFrameTimeMs() const { return 1000.0f / TargetFPS; }
};

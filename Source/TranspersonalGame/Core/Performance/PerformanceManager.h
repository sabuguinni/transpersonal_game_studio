#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PerformanceManager.generated.h"

// ============================================================
// EPerf_QualityTier — PC/Console quality presets
// ============================================================
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low         UMETA(DisplayName = "Low (Console 30fps)"),
    Medium      UMETA(DisplayName = "Medium (PC 60fps)"),
    High        UMETA(DisplayName = "High (PC High-end 60fps)"),
    Ultra       UMETA(DisplayName = "Ultra (PC 4K 60fps)")
};

// ============================================================
// FPerf_TickBudget — per-system tick interval config
// ============================================================
USTRUCT(BlueprintType)
struct FPerf_TickBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DinosaurTickInterval = 0.05f;   // 20Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SurvivalTickInterval = 0.1f;    // 10Hz

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CrowdNearTickInterval = 0.1f;   // 10Hz within 500m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CrowdMidTickInterval = 0.5f;    // 2Hz within 2000m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CrowdFarTickInterval = 0.0f;    // Dormant beyond 2000m

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float NearDistanceThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FarDistanceThreshold = 2000.0f;
};

// ============================================================
// FPerf_FrameStats — runtime frame budget tracking
// ============================================================
USTRUCT(BlueprintType)
struct FPerf_FrameStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameTimeMs = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveDinoCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveCrowdCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DormantCrowdCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;
};

// ============================================================
// APerformanceManager — world actor that enforces frame budget
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerformanceManager : public AActor
{
    GENERATED_BODY()

public:
    APerformanceManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Quality Tier ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    // ---- Tick Budgets ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick")
    FPerf_TickBudget TickBudget;

    // ---- Target FPS ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Targets")
    float TargetFPS_Console = 30.0f;

    // ---- Frame Stats (read-only) ----
    UPROPERTY(BlueprintReadOnly, Category = "Performance|Stats")
    FPerf_FrameStats FrameStats;

    // ---- LOD Distance Scales ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD",
              meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float StaticMeshLODScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD",
              meta = (ClampMin = "0.5", ClampMax = "3.0"))
    float SkeletalMeshLODScale = 1.0f;

    // ---- Shadow Budget ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows",
              meta = (ClampMin = "512", ClampMax = "4096"))
    int32 MaxShadowResolution = 2048;

    // ---- Streaming Pool ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming",
              meta = (ClampMin = "256", ClampMax = "4096"))
    int32 TextureStreamingPoolMB = 1024;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier NewTier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTargetFPS(float NewTargetFPS);

    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameStats GetFrameStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateTickIntervalsForAllActors();

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsFrameBudgetHealthy() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyConsoleCommands();

private:
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    float TimeSinceLastTickUpdate = 0.0f;
    static constexpr float TickUpdateInterval = 2.0f; // Re-evaluate tick intervals every 2s

    void UpdateFrameStats(float DeltaTime);
    void ApplyQualityPreset_Low();
    void ApplyQualityPreset_Medium();
    void ApplyQualityPreset_High();
    void ApplyQualityPreset_Ultra();
    float GetDistanceToPlayer(AActor* Actor) const;
};

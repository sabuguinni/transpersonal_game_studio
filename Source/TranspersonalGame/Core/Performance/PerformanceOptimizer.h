// PerformanceOptimizer.h
// Agent #04 — Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260626_001
// Guarantees 60fps PC / 30fps console via dynamic LOD, tick budgeting, and Lumen tuning.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "PerformanceOptimizer.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra       UMETA(DisplayName = "Ultra (PC High-End)"),
    High        UMETA(DisplayName = "High (PC Mid)"),
    Medium      UMETA(DisplayName = "Medium (Console)"),
    Low         UMETA(DisplayName = "Low (Console Min)")
};

UENUM(BlueprintType)
enum class EPerf_TickGroup : uint8
{
    Critical    UMETA(DisplayName = "Critical (every frame)"),
    Standard    UMETA(DisplayName = "Standard (0.1s)"),
    Background  UMETA(DisplayName = "Background (1.0s)"),
    Disabled    UMETA(DisplayName = "Disabled")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameTimeBudgetMs = 16.67f;  // 1000 / 60

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadBudgetMs = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadBudgetMs = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicResolution = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DynamicResolutionMinScale = 0.7f;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurLOD0Distance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurLOD1Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinosaurLOD2Distance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageLODDistanceScale = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 SkeletalMeshLODBias = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 StaticMeshLODBias = 0;
};

USTRUCT(BlueprintType)
struct FPerf_SurvivalTickConfig
{
    GENERATED_BODY()

    // Tick interval when player is idle (far from danger)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Tick")
    float IdleTickInterval = 1.0f;

    // Tick interval during active gameplay (moving, near dinos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Tick")
    float ActiveTickInterval = 0.25f;

    // Tick interval during combat (high precision needed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Tick")
    float CombatTickInterval = 0.1f;

    // Distance threshold from nearest dinosaur to switch to combat tick
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival|Tick")
    float CombatProximityRadius = 1500.0f;
};

USTRUCT(BlueprintType)
struct FPerf_LumenConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    int32 ScreenProbeTraceBudget = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float MaxTraceDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    float ReflectionsMaxRoughness = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bAllowReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lumen")
    bool bAllowGlobalIllumination = true;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 MaxShadowResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 CSMMaxCascades = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bUseFastSkyLUT = true;
};

// ─── UPerformanceOptimizer ────────────────────────────────────────────────────

UCLASS(ClassGroup = "TranspersonalGame", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceOptimizer();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ─── Quality Tier ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Quality")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier Tier);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyConsolePreset();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPCHighEndPreset();

    // ─── Frame Budget ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsFrameBudgetExceeded() const;

    // ─── LOD ──────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings();

    // ─── Survival Tick ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Survival")
    FPerf_SurvivalTickConfig SurvivalTickConfig;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSurvivalTickInterval(bool bInCombat, bool bIsMoving) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateSurvivalTickRate(class USurvivalComponent* SurvivalComp, bool bInCombat, bool bIsMoving);

    // ─── Lumen ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    FPerf_LumenConfig LumenConfig;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLumenSettings();

    // ─── Shadows ──────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowConfig ShadowConfig;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyShadowSettings();

    // ─── Static Actor Tick ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 DisableTickOnStaticActors();

    // ─── Memory ───────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 TextureStreamingPoolSizeMB = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Memory")
    int32 MaxTempMemoryAllowedMB = 256;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyMemorySettings();

    // ─── Full Apply ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyAllSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunPerformanceAudit();

private:
    float AccumulatedDeltaTime = 0.0f;
    float AuditInterval = 5.0f;
    float LastFPS = 0.0f;

    void ExecuteConsoleCommand(const FString& Command);
};

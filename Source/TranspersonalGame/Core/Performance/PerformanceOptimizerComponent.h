#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PerformanceOptimizerComponent.generated.h"

// Forward declarations
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Ultra    UMETA(DisplayName = "Ultra (PC High-End)"),
    High     UMETA(DisplayName = "High (PC Mid-Range)"),
    Medium   UMETA(DisplayName = "Medium (Console)"),
    Low      UMETA(DisplayName = "Low (Console Min)"),
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target FPS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    // Max ms per frame (1000/TargetFPS)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameBudgetMS = 16.67f;

    // Current measured FPS (rolling average)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    // Current frame time in ms
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameMS = 0.0f;

    // Is the game meeting its frame budget?
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bMeetingBudget = true;
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    // Global LOD distance scale multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float LODDistanceScale = 1.0f;

    // Foliage-specific LOD scale
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float FoliageLODScale = 1.5f;

    // Max draw distance for small props (< 1m scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float SmallPropCullDistance = 5000.0f;

    // Max draw distance for medium objects (1-5m scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumObjectCullDistance = 20000.0f;

    // Max draw distance for large objects (dinosaurs, structures)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LargeObjectCullDistance = 50000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowSettings
{
    GENERATED_BODY()

    // Max CSM cascades (3 = good balance for open world)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows", meta = (ClampMin = "1", ClampMax = "4"))
    int32 MaxCSMCascades = 3;

    // Shadow map resolution (1024, 2048, 4096)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 ShadowMapResolution = 2048;

    // Distance at which dynamic shadows switch to static
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float DynamicShadowDistance = 10000.0f;
};

/**
 * UPerf_PerformanceOptimizerComponent
 *
 * Attaches to the GameMode or a manager actor.
 * Monitors frame time, applies adaptive quality settings,
 * and enforces LOD/culling budgets for 60fps PC / 30fps console targets.
 */
UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Performance Optimizer")
class TRANSPERSONALGAME_API UPerf_PerformanceOptimizerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceOptimizerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PUBLIC API ===

    /** Apply quality tier settings immediately */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTier(EPerf_QualityTier Tier);

    /** Force LOD settings update on all StaticMeshComponents in the world */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RefreshLODSettings();

    /** Get current frame budget status */
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const { return CurrentFrameBudget; }

    /** Get current quality tier */
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_QualityTier GetCurrentQualityTier() const { return CurrentQualityTier; }

    /** Manually trigger adaptive quality check */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void RunAdaptiveQualityCheck();

    // === SETTINGS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget TargetFrameBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowSettings ShadowSettings;

    /** Starting quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier InitialQualityTier = EPerf_QualityTier::High;

    /** Enable adaptive quality (auto-downgrade when FPS drops) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bAdaptiveQuality = true;

    /** How many seconds of low FPS before downgrading quality */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float AdaptiveQualityGracePeriod = 3.0f;

    /** FPS rolling average window (frames) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "120"))
    int32 FPSAveragingWindow = 60;

private:
    // Current state
    EPerf_QualityTier CurrentQualityTier = EPerf_QualityTier::High;
    FPerf_FrameBudget CurrentFrameBudget;

    // FPS rolling average
    TArray<float> FPSSamples;
    float AccumulatedLowFPSTime = 0.0f;

    // Internal helpers
    void UpdateFPSAverage(float DeltaTime);
    void CheckAdaptiveQuality(float DeltaTime);
    void ApplyConsoleCVars(EPerf_QualityTier Tier);
    void ApplyShadowCVars();
    void ApplyLODCVars();

    // Console variable helpers
    static void SetCVar(const FString& CVarName, float Value);
    static void SetCVar(const FString& CVarName, int32 Value);
};

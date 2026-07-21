#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

/**
 * Performance tier targets for the prehistoric survival game.
 * Agent #04 — Performance Optimizer
 * Target: 60fps PC high-end / 30fps console
 */
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low     UMETA(DisplayName = "Low (Console 30fps)"),
    Medium  UMETA(DisplayName = "Medium (PC 30fps)"),
    High    UMETA(DisplayName = "High (PC 60fps)"),
    Ultra   UMETA(DisplayName = "Ultra (PC 120fps)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Target frame time in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTimeMs = 16.6f;  // 60fps default

    /** Game thread budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GameThreadBudgetMs = 8.0f;

    /** Render thread budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float RenderThreadBudgetMs = 6.0f;

    /** GPU budget in milliseconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GPUBudgetMs = 14.0f;

    /** Max active dinosaur AI actors before LOD downgrade */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveDinoAI = 20;

    /** Max static mesh actors before culling aggressively */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxStaticMeshActors = 500;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    /** LOD screen size thresholds [LOD0, LOD1, LOD2, LOD3] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> ScreenSizeThresholds = { 1.0f, 0.3f, 0.1f, 0.03f };

    /** Max cull distance for large actors (TRex, Brachiosaurus) in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LargeDinoCullDistance = 8000.0f;

    /** Max cull distance for medium actors (Raptors) in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDinoCullDistance = 5000.0f;

    /** Max cull distance for foliage/props in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageCullDistance = 3000.0f;

    /** AI tick interval for distant dinosaurs (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoAITickInterval = 0.1f;  // 10Hz

    /** AI tick interval for nearby dinosaurs (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DinoAINearTickInterval = 0.033f;  // 30Hz
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_SurvivalTickConfig
{
    GENERATED_BODY()

    /** Timer interval for survival stat drain/recovery (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SurvivalDrainInterval = 0.5f;  // 2Hz — amortized ~0.01ms/frame

    /** Timer interval for biome temperature query (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float BiomeTempQueryInterval = 5.0f;  // 0.2Hz

    /** Stamina drain per second while sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float SprintStaminaDrainPerSecond = 10.0f;

    /** Stamina recovery per second while not sprinting */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float StaminaRecoveryPerSecond = 5.0f;

    /** Minimum stamina to allow sprint activation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MinStaminaToSprint = 5.0f;
};

/**
 * UPerf_PerformanceConfig — Data asset holding all performance settings.
 * Loaded at game startup by the Performance subsystem.
 * Agent #04 — Performance Optimizer
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    /** Current quality tier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    /** Frame budget settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_FrameBudget FrameBudget;

    /** LOD and cull distance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings LODSettings;

    /** Survival component tick configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FPerf_SurvivalTickConfig SurvivalTickConfig;

    /** Apply console variable settings for the current quality tier */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityTierCVars();

    /** Get recommended cull distance for a dinosaur by approximate size */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDinoCullDistance(float DinoSizeCm) const;

    /** Check if current frame budget is exceeded and return severity (0=OK, 1=warn, 2=critical) */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 CheckFrameBudget(float GameThreadMs, float RenderThreadMs, float GPUMs) const;
};

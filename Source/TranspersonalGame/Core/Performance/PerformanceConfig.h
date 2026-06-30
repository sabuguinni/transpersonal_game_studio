// PerformanceConfig.h
// Agent #04 — Performance Optimizer
// Frame budget constants and performance thresholds for Transpersonal Game Studio
// Target: 60fps PC (RTX 3070+) / 30fps Console (PS5/XSX)

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ============================================================
// ENUMS — Perf_ prefix to avoid conflicts
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
    Low        UMETA(DisplayName = "Low (30fps Console)"),
    Medium     UMETA(DisplayName = "Medium (45fps Mid-PC)"),
    High       UMETA(DisplayName = "High (60fps High-PC)"),
    Ultra      UMETA(DisplayName = "Ultra (60fps+ RTX 4080+)")
};

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD0 Full Detail"),
    LOD1_Medium     UMETA(DisplayName = "LOD1 Medium"),
    LOD2_Low        UMETA(DisplayName = "LOD2 Low"),
    LOD3_Impostor   UMETA(DisplayName = "LOD3 Impostor/Billboard"),
    Culled          UMETA(DisplayName = "Culled (not rendered)")
};

// ============================================================
// STRUCTS — Perf_ prefix
// ============================================================

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Total frame time budget in milliseconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TotalFrameMs = 16.67f;  // 60fps target

    // GPU budget (ms) — typically 70% of frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float GPUBudgetMs = 11.67f;

    // CPU budget (ms) — typically 30% of frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float CPUBudgetMs = 5.0f;

    // Shadow rendering budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float ShadowBudgetMs = 2.5f;

    // Lumen GI budget (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float LumenBudgetMs = 3.0f;

    // Max draw calls per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDrawCalls = 1500;

    // Max triangles per frame (millions)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxTrianglesMillion = 8.0f;
};

USTRUCT(BlueprintType)
struct FPerf_LODDistances
{
    GENERATED_BODY()

    // Distance at which LOD1 kicks in (units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1Distance = 1500.0f;

    // Distance at which LOD2 kicks in (units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2Distance = 3500.0f;

    // Distance at which LOD3/impostor kicks in (units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD3Distance = 6000.0f;

    // Max draw distance — beyond this, actor is culled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float MaxDrawDistance = 8000.0f;

    // Dinosaur-specific max draw distance (larger = more visible)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float DinosaurMaxDrawDistance = 10000.0f;

    // Foliage max draw distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float FoliageMaxDrawDistance = 5000.0f;

    // Rock/prop max draw distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float PropMaxDrawDistance = 4000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_ShadowSettings
{
    GENERATED_BODY()

    // Max shadow map resolution
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 MaxShadowResolution = 1024;

    // Number of CSM cascades (3 = balanced, 4 = high quality)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    int32 CSMCascades = 3;

    // Shadow distance scale (0.7 = 30% shorter shadows = faster)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float ShadowDistanceScale = 0.7f;

    // Radius threshold for shadow casters (larger = fewer shadow casters)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    float RadiusThreshold = 0.03f;

    // Enable shadow caching
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    bool bCacheShadows = true;
};

USTRUCT(BlueprintType)
struct FPerf_LumenSettings
{
    GENERATED_BODY()

    // Use hardware raytracing for Lumen (false = software, faster on older GPUs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bHardwareRayTracing = false;

    // Max trace distance for GI (shorter = faster)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float MaxGITraceDistance = 5000.0f;

    // Max roughness for reflections (lower = fewer reflections traced)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    float MaxReflectionRoughness = 0.4f;

    // Enable Lumen reflections
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bLumenReflections = true;

    // Enable Lumen GI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    bool bLumenGI = true;
};

// ============================================================
// UCLASS — Performance Configuration Asset
// ============================================================

UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Performance")
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // Current quality tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

    // Frame budget settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // LOD distance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODDistances LODDistances;

    // Shadow settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows")
    FPerf_ShadowSettings ShadowSettings;

    // Lumen settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lumen")
    FPerf_LumenSettings LumenSettings;

    // Max simultaneous dynamic lights
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Lights")
    int32 MaxDynamicLights = 8;

    // Max simultaneous particle systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|VFX")
    int32 MaxParticleSystems = 32;

    // Max simultaneous dinosaur AI agents (beyond this, use simplified AI)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    int32 MaxFullAIDinosaurs = 12;

    // Distance at which dinosaurs switch from full AI to simplified AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|AI")
    float DinosaurAISimplifyDistance = 4000.0f;

    // Apply all settings to the engine via console commands
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyToEngine();

    // Get frame budget for current quality tier
    UFUNCTION(BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudgetForTier(EPerf_QualityTier Tier) const;

    // Check if we're within frame budget (returns true if OK)
    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsWithinFrameBudget(float CurrentFrameMs) const;

    // Get recommended LOD level for a given distance
    UFUNCTION(BlueprintPure, Category = "Performance")
    EPerf_LODLevel GetLODLevelForDistance(float Distance, bool bIsDinosaur = false) const;
};

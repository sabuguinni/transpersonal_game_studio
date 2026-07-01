// PerformanceConfig.h — Transpersonal Game Studio
// Agent #4 Performance Optimizer — PROD_CYCLE_AUTO_20260701_003
// Central performance constants for 60fps PC / 30fps console targets.
// All systems read from this file — change here, affects everywhere.

#pragma once

#include "CoreMinimal.h"
#include "PerformanceConfig.generated.h"

// ============================================================
// ENUMS — global scope, Perf_ prefix (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Low         UMETA(DisplayName = "Low (30fps Console)"),
    Medium      UMETA(DisplayName = "Medium (60fps Mid PC)"),
    High        UMETA(DisplayName = "High (60fps High-End PC)"),
    Ultra       UMETA(DisplayName = "Ultra (120fps+)"),
};

UENUM(BlueprintType)
enum class EPerf_LODTier : uint8
{
    LOD0_Full       UMETA(DisplayName = "LOD0 Full Detail"),
    LOD1_Medium     UMETA(DisplayName = "LOD1 Medium"),
    LOD2_Low        UMETA(DisplayName = "LOD2 Low"),
    LOD3_Impostor   UMETA(DisplayName = "LOD3 Impostor/Billboard"),
    Culled          UMETA(DisplayName = "Culled (invisible)"),
};

// ============================================================
// STRUCTS — global scope
// ============================================================

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODConfig
{
    GENERATED_BODY()

    // Distance at which LOD0 transitions to LOD1 (units = cm in UE5)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD0_Distance = 1500.0f;

    // Distance at which LOD1 transitions to LOD2
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD1_Distance = 3000.0f;

    // Distance at which LOD2 transitions to LOD3/impostor
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LOD2_Distance = 6000.0f;

    // Distance beyond which actor is fully culled (invisible)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float CullDistance = 8000.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in ms (16.67ms = 60fps, 33.33ms = 30fps)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TargetFrameTimeMs = 16.67f;

    // Max ms allowed for game thread (CPU)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxGameThreadMs = 6.0f;

    // Max ms allowed for render thread
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxRenderThreadMs = 8.0f;

    // Max ms allowed for GPU
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MaxGPUMs = 14.0f;

    // Max dynamic shadow-casting lights per frame
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDynamicShadowLights = 4;

    // Max draw calls per frame (rough target)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    int32 MaxDrawCalls = 1500;
};

// ============================================================
// MAIN CONFIG CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable, DefaultToInstanced, EditInlineNew,
       meta = (DisplayName = "Performance Config"))
class TRANSPERSONALGAME_API UPerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceConfig();

    // --------------------------------------------------------
    // Quality preset (drives all other settings)
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityPreset QualityPreset = EPerf_QualityPreset::High;

    // --------------------------------------------------------
    // Frame budget targets
    // --------------------------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // --------------------------------------------------------
    // LOD configs per actor category
    // --------------------------------------------------------

    // Foliage (trees, bushes, ferns) — aggressive culling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig FoliageLOD;

    // Dinosaur actors — visible at distance for atmosphere
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig DinosaurLOD;

    // Props (rocks, debris, structures)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig PropLOD;

    // Character (player + NPCs)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODConfig CharacterLOD;

    // --------------------------------------------------------
    // Tick intervals (seconds) — reduce tick frequency for perf
    // --------------------------------------------------------

    // SurvivalComponent tick interval (0.1 = 10/sec, imperceptible to player)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick",
              meta = (ClampMin = "0.05", ClampMax = "1.0"))
    float SurvivalComponentTickInterval = 0.1f;

    // DinosaurBase patrol tick interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick",
              meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float DinosaurPatrolTickInterval = 0.25f;

    // FoliageManager update interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick",
              meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float FoliageUpdateInterval = 1.0f;

    // BiomeManager update interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Tick",
              meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float BiomeUpdateInterval = 2.0f;

    // --------------------------------------------------------
    // Shadow settings
    // --------------------------------------------------------

    // Max shadow map resolution (1024 = good quality/perf balance)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows",
              meta = (ClampMin = "256", ClampMax = "4096"))
    int32 MaxShadowResolution = 1024;

    // Shadow radius threshold — skip shadows smaller than this
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Shadows",
              meta = (ClampMin = "0.0", ClampMax = "0.1"))
    float ShadowRadiusThreshold = 0.03f;

    // --------------------------------------------------------
    // Streaming settings
    // --------------------------------------------------------

    // Texture streaming pool size in MB
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming",
              meta = (ClampMin = "256", ClampMax = "4096"))
    int32 TextureStreamingPoolMB = 1024;

    // --------------------------------------------------------
    // Blueprint callable utilities
    // --------------------------------------------------------

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPreset(EPerf_QualityPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetFoliageCullDistance() const { return FoliageLOD.CullDistance; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDinosaurCullDistance() const { return DinosaurLOD.CullDistance; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinFrameBudget(float GameThreadMs, float RenderThreadMs, float GPUMs) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetFrameBudgetReport(float GameThreadMs, float RenderThreadMs, float GPUMs) const;

    // Static accessor — returns project default config
    UFUNCTION(BlueprintCallable, Category = "Performance", meta = (WorldContext = "WorldContextObject"))
    static UPerformanceConfig* GetDefault(UObject* WorldContextObject);
};

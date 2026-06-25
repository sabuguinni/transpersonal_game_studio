#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceConfig.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// EPerf_QualityPreset — Target platform quality tier
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EPerf_QualityPreset : uint8
{
    Console_30fps   UMETA(DisplayName = "Console 30fps"),
    PC_60fps        UMETA(DisplayName = "PC 60fps"),
    PC_Ultra        UMETA(DisplayName = "PC Ultra"),
};

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_FrameBudget — Per-system ms allocations within a 16.7ms frame
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    /** Total frame budget in milliseconds (16.7ms = 60fps, 33.3ms = 30fps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float TotalBudgetMs = 16.7f;

    /** GPU time allocated to shadow rendering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float ShadowBudgetMs = 3.0f;

    /** GPU time allocated to Lumen GI + reflections */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float LumenBudgetMs = 4.0f;

    /** GPU time allocated to skeletal mesh rendering (dinosaurs + character) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float SkeletalMeshBudgetMs = 2.5f;

    /** GPU time allocated to foliage + static mesh rendering */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float FoliageBudgetMs = 3.0f;

    /** CPU time allocated to AI tick (dinosaur behavior trees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float AIBudgetMs = 2.0f;

    /** Remaining budget for post-process, UI, audio, misc */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    float MiscBudgetMs = 2.2f;
};

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_LODSettings — LOD thresholds for dinosaurs and vegetation
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    /** Distance at which skeletal meshes switch to LOD1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float SkeletalLOD1Distance = 1500.0f;

    /** Distance at which skeletal meshes switch to LOD2 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float SkeletalLOD2Distance = 4000.0f;

    /** Distance at which skeletal meshes are culled entirely */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float SkeletalCullDistance = 8000.0f;

    /** Distance at which foliage static meshes switch to LOD1 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float FoliageLOD1Distance = 2000.0f;

    /** Distance at which foliage is culled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float FoliageCullDistance = 6000.0f;

    /** Global LOD distance scale multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float LODDistanceScale = 1.5f;
};

// ─────────────────────────────────────────────────────────────────────────────
// UPerf_PerformanceConfig — Runtime performance configuration object
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Performance")
class TRANSPERSONALGAME_API UPerf_PerformanceConfig : public UObject
{
    GENERATED_BODY()

public:
    UPerf_PerformanceConfig();

    // ── Quality Preset ────────────────────────────────────────────────────────

    /** Active quality preset — drives all sub-settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_QualityPreset QualityPreset = EPerf_QualityPreset::PC_60fps;

    // ── Frame Budget ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
    FPerf_FrameBudget FrameBudget;

    // ── LOD Settings ─────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    // ── Streaming ────────────────────────────────────────────────────────────

    /** Texture streaming pool size in MB */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Streaming")
    int32 StreamingPoolSizeMB = 2048;

    /** Max number of skeletal mesh actors allowed simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxSkeletalMeshActors = 20;

    /** Max number of static mesh actors allowed simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxStaticMeshActors = 200;

    /** Max number of active dinosaur AI agents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Limits")
    int32 MaxActiveDinoAI = 12;

    // ── Methods ───────────────────────────────────────────────────────────────

    /** Apply this config's console variable settings to the running engine */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
    void ApplyToEngine();

    /** Switch to Console 30fps preset and apply */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetConsolePreset();

    /** Switch to PC 60fps preset and apply */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPC60Preset();

    /** Switch to PC Ultra preset and apply */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetUltraPreset();

    /** Returns true if current actor counts are within budget */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsWithinBudget(int32 SkeletalCount, int32 StaticCount) const;

    /** Returns the frame budget struct for the active preset */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const { return FrameBudget; }

    /** Returns the LOD settings for the active preset */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

private:
    void ApplyConsolePlatformSettings();
    void ApplyPC60Settings();
    void ApplyUltraSettings();
};

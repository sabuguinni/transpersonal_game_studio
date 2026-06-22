#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PerformanceBudget.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_FrameBudget — per-frame time allocation targets (milliseconds)
// PC High-End: 60fps = 16.67ms total frame budget
// Console:     30fps = 33.33ms total frame budget
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameBudget
{
	GENERATED_BODY()

	// Total frame budget in ms
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	float TotalBudgetMs = 16.67f;

	// CPU game thread budget (AI, physics, gameplay logic)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	float GameThreadMs = 6.0f;

	// CPU render thread budget (draw calls, state changes)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	float RenderThreadMs = 7.0f;

	// GPU budget (shading, shadows, post-process)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	float GpuMs = 14.0f;

	// Max active dynamic lights (shadow casting)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	int32 MaxDynamicLights = 4;

	// Max simultaneous dino AI ticks per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	int32 MaxDinoAITicksPerFrame = 8;

	// Max draw calls per frame
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	int32 MaxDrawCalls = 1500;

	// Texture streaming pool size (MB)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|Budget")
	int32 TextureStreamingPoolMB = 1024;
};

// ─────────────────────────────────────────────────────────────────────────────
// EPerf_QualityTier — hardware quality tier
// ─────────────────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EPerf_QualityTier : uint8
{
	Low        UMETA(DisplayName = "Low (Console/Potato)"),
	Medium     UMETA(DisplayName = "Medium (Mid-range PC)"),
	High       UMETA(DisplayName = "High (High-end PC)"),
	Ultra      UMETA(DisplayName = "Ultra (Enthusiast PC)")
};

// ─────────────────────────────────────────────────────────────────────────────
// FPerf_DinoTickConfig — per-dino LOD tick rate configuration
// Far dinos tick at 5Hz; near dinos tick at 30Hz; combat dinos tick at 60Hz
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_DinoTickConfig
{
	GENERATED_BODY()

	// Distance (cm) beyond which dino ticks at FarTickInterval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float FarDistanceCm = 5000.0f;

	// Distance (cm) within which dino ticks at NearTickInterval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float NearDistanceCm = 2000.0f;

	// Tick interval when far from player (5Hz = 0.2s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float FarTickIntervalSec = 0.2f;

	// Tick interval when mid-range (10Hz = 0.1s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float MidTickIntervalSec = 0.1f;

	// Tick interval when near player or in combat (30Hz = 0.033s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float NearTickIntervalSec = 0.033f;

	// Tick interval when in active combat (60Hz = 0.016s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|DinoLOD")
	float CombatTickIntervalSec = 0.016f;
};

// ─────────────────────────────────────────────────────────────────────────────
// UPerf_BudgetManager — runtime performance budget enforcer
// Attach to GameState or use as subsystem to monitor frame budget
// ─────────────────────────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_BudgetManager : public UObject
{
	GENERATED_BODY()

public:
	UPerf_BudgetManager();

	// Current quality tier (set on startup based on hardware)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	EPerf_QualityTier QualityTier = EPerf_QualityTier::High;

	// Frame budget for current tier
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FPerf_FrameBudget FrameBudget;

	// Dino LOD tick configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	FPerf_DinoTickConfig DinoTickConfig;

	// Returns the correct tick interval for a dino at given distance from player
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetDinoTickInterval(float DistanceFromPlayerCm, bool bInCombat) const;

	// Apply quality tier presets (sets FrameBudget values)
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void ApplyQualityTier(EPerf_QualityTier NewTier);

	// Returns true if we are over budget (game thread > GameThreadMs)
	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool IsOverBudget() const;

	// Log current budget status to output log
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Performance")
	void LogBudgetStatus() const;

private:
	// Cached last frame time (ms)
	float LastFrameTimeMs = 0.0f;
};

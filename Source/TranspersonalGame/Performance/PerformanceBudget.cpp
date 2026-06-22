// PerformanceBudget.cpp
// Performance Optimizer — Agent #04
// Transpersonal Game Studio
//
// Runtime performance budget manager.
// Enforces 60fps PC / 30fps console targets.
// Controls dino LOD tick rates and frame budget monitoring.

#include "PerformanceBudget.h"
#include "Engine/Engine.h"
#include "HAL/PlatformTime.h"

// ─────────────────────────────────────────────────────────────────────────────
// UPerf_BudgetManager — Constructor
// ─────────────────────────────────────────────────────────────────────────────
UPerf_BudgetManager::UPerf_BudgetManager()
{
	// Default to High quality (PC high-end target: 60fps = 16.67ms)
	QualityTier = EPerf_QualityTier::High;

	// Default frame budget: 60fps PC
	FrameBudget.TotalBudgetMs       = 16.67f;
	FrameBudget.GameThreadMs        = 6.0f;
	FrameBudget.RenderThreadMs      = 7.0f;
	FrameBudget.GpuMs               = 14.0f;
	FrameBudget.MaxDynamicLights    = 4;
	FrameBudget.MaxDinoAITicksPerFrame = 8;
	FrameBudget.MaxDrawCalls        = 1500;
	FrameBudget.TextureStreamingPoolMB = 1024;

	// Default dino LOD tick config
	DinoTickConfig.FarDistanceCm    = 5000.0f;
	DinoTickConfig.NearDistanceCm   = 2000.0f;
	DinoTickConfig.FarTickIntervalSec    = 0.2f;   // 5Hz  — far dinos
	DinoTickConfig.MidTickIntervalSec    = 0.1f;   // 10Hz — mid-range
	DinoTickConfig.NearTickIntervalSec   = 0.033f; // 30Hz — near player
	DinoTickConfig.CombatTickIntervalSec = 0.016f; // 60Hz — active combat
}

// ─────────────────────────────────────────────────────────────────────────────
// GetDinoTickInterval
// Returns the correct tick interval (seconds) for a dinosaur based on:
//   - Distance from player
//   - Whether the dino is in active combat
//
// This is the core of the dino LOD system:
//   Combat   → 60Hz (0.016s) — frame-perfect combat response
//   Near     → 30Hz (0.033s) — smooth animation/AI near player
//   Mid      → 10Hz (0.100s) — background behaviour
//   Far      →  5Hz (0.200s) — minimal CPU cost for distant dinos
// ─────────────────────────────────────────────────────────────────────────────
float UPerf_BudgetManager::GetDinoTickInterval(float DistanceFromPlayerCm, bool bInCombat) const
{
	if (bInCombat)
	{
		return DinoTickConfig.CombatTickIntervalSec;
	}

	if (DistanceFromPlayerCm <= DinoTickConfig.NearDistanceCm)
	{
		return DinoTickConfig.NearTickIntervalSec;
	}

	if (DistanceFromPlayerCm <= DinoTickConfig.FarDistanceCm)
	{
		return DinoTickConfig.MidTickIntervalSec;
	}

	return DinoTickConfig.FarTickIntervalSec;
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyQualityTier
// Sets frame budget values appropriate for the given hardware tier.
// Called once on startup after hardware detection.
// ─────────────────────────────────────────────────────────────────────────────
void UPerf_BudgetManager::ApplyQualityTier(EPerf_QualityTier NewTier)
{
	QualityTier = NewTier;

	switch (NewTier)
	{
	case EPerf_QualityTier::Low:
		// Console / low-end: 30fps target = 33.33ms
		FrameBudget.TotalBudgetMs          = 33.33f;
		FrameBudget.GameThreadMs           = 10.0f;
		FrameBudget.RenderThreadMs         = 12.0f;
		FrameBudget.GpuMs                  = 28.0f;
		FrameBudget.MaxDynamicLights       = 1;
		FrameBudget.MaxDinoAITicksPerFrame = 4;
		FrameBudget.MaxDrawCalls           = 800;
		FrameBudget.TextureStreamingPoolMB = 512;
		// Increase tick intervals for low-end
		DinoTickConfig.FarTickIntervalSec    = 0.5f;   // 2Hz
		DinoTickConfig.MidTickIntervalSec    = 0.2f;   // 5Hz
		DinoTickConfig.NearTickIntervalSec   = 0.066f; // 15Hz
		DinoTickConfig.CombatTickIntervalSec = 0.033f; // 30Hz
		break;

	case EPerf_QualityTier::Medium:
		// Mid-range PC: 60fps target
		FrameBudget.TotalBudgetMs          = 16.67f;
		FrameBudget.GameThreadMs           = 5.0f;
		FrameBudget.RenderThreadMs         = 6.0f;
		FrameBudget.GpuMs                  = 13.0f;
		FrameBudget.MaxDynamicLights       = 2;
		FrameBudget.MaxDinoAITicksPerFrame = 6;
		FrameBudget.MaxDrawCalls           = 1200;
		FrameBudget.TextureStreamingPoolMB = 768;
		DinoTickConfig.FarTickIntervalSec    = 0.2f;
		DinoTickConfig.MidTickIntervalSec    = 0.1f;
		DinoTickConfig.NearTickIntervalSec   = 0.033f;
		DinoTickConfig.CombatTickIntervalSec = 0.016f;
		break;

	case EPerf_QualityTier::High:
		// High-end PC: 60fps with headroom
		FrameBudget.TotalBudgetMs          = 16.67f;
		FrameBudget.GameThreadMs           = 6.0f;
		FrameBudget.RenderThreadMs         = 7.0f;
		FrameBudget.GpuMs                  = 14.0f;
		FrameBudget.MaxDynamicLights       = 4;
		FrameBudget.MaxDinoAITicksPerFrame = 8;
		FrameBudget.MaxDrawCalls           = 1500;
		FrameBudget.TextureStreamingPoolMB = 1024;
		DinoTickConfig.FarTickIntervalSec    = 0.2f;
		DinoTickConfig.MidTickIntervalSec    = 0.1f;
		DinoTickConfig.NearTickIntervalSec   = 0.033f;
		DinoTickConfig.CombatTickIntervalSec = 0.016f;
		break;

	case EPerf_QualityTier::Ultra:
		// Enthusiast PC: 120fps target = 8.33ms
		FrameBudget.TotalBudgetMs          = 8.33f;
		FrameBudget.GameThreadMs           = 3.0f;
		FrameBudget.RenderThreadMs         = 3.5f;
		FrameBudget.GpuMs                  = 7.0f;
		FrameBudget.MaxDynamicLights       = 8;
		FrameBudget.MaxDinoAITicksPerFrame = 16;
		FrameBudget.MaxDrawCalls           = 2500;
		FrameBudget.TextureStreamingPoolMB = 2048;
		DinoTickConfig.FarTickIntervalSec    = 0.1f;   // 10Hz
		DinoTickConfig.MidTickIntervalSec    = 0.033f; // 30Hz
		DinoTickConfig.NearTickIntervalSec   = 0.016f; // 60Hz
		DinoTickConfig.CombatTickIntervalSec = 0.008f; // 120Hz
		break;

	default:
		break;
	}

	UE_LOG(LogTemp, Log, TEXT("[PerformanceBudget] Quality tier applied: %s | Budget: %.2fms total, %d draw calls, %d MB textures"),
		*UEnum::GetValueAsString(NewTier),
		FrameBudget.TotalBudgetMs,
		FrameBudget.MaxDrawCalls,
		FrameBudget.TextureStreamingPoolMB);
}

// ─────────────────────────────────────────────────────────────────────────────
// IsOverBudget
// Returns true if the last frame exceeded the game thread budget.
// Used by DinosaurBase to decide whether to defer non-critical AI work.
// ─────────────────────────────────────────────────────────────────────────────
bool UPerf_BudgetManager::IsOverBudget() const
{
	if (GEngine)
	{
		// FApp::GetDeltaTime() gives last frame time in seconds
		const float LastFrameSec = FApp::GetDeltaTime();
		const float LastFrameMs  = LastFrameSec * 1000.0f;
		return LastFrameMs > FrameBudget.TotalBudgetMs;
	}
	return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// LogBudgetStatus
// Prints current budget config to output log.
// Callable from editor via CallInEditor button.
// ─────────────────────────────────────────────────────────────────────────────
void UPerf_BudgetManager::LogBudgetStatus() const
{
	UE_LOG(LogTemp, Log, TEXT("═══════════════════════════════════════════════════"));
	UE_LOG(LogTemp, Log, TEXT("[PerformanceBudget] STATUS REPORT"));
	UE_LOG(LogTemp, Log, TEXT("  Quality Tier:      %s"), *UEnum::GetValueAsString(QualityTier));
	UE_LOG(LogTemp, Log, TEXT("  Total Budget:      %.2f ms"), FrameBudget.TotalBudgetMs);
	UE_LOG(LogTemp, Log, TEXT("  Game Thread:       %.2f ms"), FrameBudget.GameThreadMs);
	UE_LOG(LogTemp, Log, TEXT("  Render Thread:     %.2f ms"), FrameBudget.RenderThreadMs);
	UE_LOG(LogTemp, Log, TEXT("  GPU Budget:        %.2f ms"), FrameBudget.GpuMs);
	UE_LOG(LogTemp, Log, TEXT("  Max Dyn Lights:    %d"),      FrameBudget.MaxDynamicLights);
	UE_LOG(LogTemp, Log, TEXT("  Max Draw Calls:    %d"),      FrameBudget.MaxDrawCalls);
	UE_LOG(LogTemp, Log, TEXT("  Texture Pool:      %d MB"),   FrameBudget.TextureStreamingPoolMB);
	UE_LOG(LogTemp, Log, TEXT("  Dino Far Tick:     %.3f s (%.0f Hz)"), DinoTickConfig.FarTickIntervalSec,    1.0f / DinoTickConfig.FarTickIntervalSec);
	UE_LOG(LogTemp, Log, TEXT("  Dino Mid Tick:     %.3f s (%.0f Hz)"), DinoTickConfig.MidTickIntervalSec,    1.0f / DinoTickConfig.MidTickIntervalSec);
	UE_LOG(LogTemp, Log, TEXT("  Dino Near Tick:    %.3f s (%.0f Hz)"), DinoTickConfig.NearTickIntervalSec,   1.0f / DinoTickConfig.NearTickIntervalSec);
	UE_LOG(LogTemp, Log, TEXT("  Dino Combat Tick:  %.3f s (%.0f Hz)"), DinoTickConfig.CombatTickIntervalSec, 1.0f / DinoTickConfig.CombatTickIntervalSec);
	UE_LOG(LogTemp, Log, TEXT("  Over Budget:       %s"), IsOverBudget() ? TEXT("YES ⚠") : TEXT("NO ✓"));
	UE_LOG(LogTemp, Log, TEXT("═══════════════════════════════════════════════════"));
}

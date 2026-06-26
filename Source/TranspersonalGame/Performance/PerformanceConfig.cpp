// PerformanceConfig.cpp — Agent #4 Performance Optimizer
// Cycle: PROD_CYCLE_AUTO_20260626_009
// Implements: UPerf_PerformanceConfig — LOD thresholds, dino tick budgets, frame budget

#include "PerformanceConfig.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

// ============================================================
// Constructor — initialize default budgets
// ============================================================

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // Default frame budget: 60fps PC
    FrameBudget.TargetFrameTimeMs = 16.67f;
    FrameBudget.AIBudgetMs        = 3.0f;
    FrameBudget.RenderBudgetMs    = 10.0f;
    FrameBudget.PhysicsBudgetMs   = 2.5f;
    FrameBudget.GameLogicBudgetMs = 1.17f;

    // Default LOD thresholds
    LODThresholds.LOD0_To_LOD1    = 2000.0f;
    LODThresholds.LOD1_To_LOD2    = 4000.0f;
    LODThresholds.LOD2_To_LOD3    = 7000.0f;
    LODThresholds.CullDistance    = 12000.0f;
    LODThresholds.DinoCullDistance = 15000.0f;

    // Dino tick budgets — 4 distance zones
    // Zone 0: Near (0-3000u) — full AI, 0.1s tick
    DinoTickBudgets.Add(FPerf_DinoTickBudget(3000.0f,  0.1f,  true,  true));
    // Zone 1: Mid (3000-5000u) — full AI, 0.5s tick
    DinoTickBudgets.Add(FPerf_DinoTickBudget(5000.0f,  0.5f,  true,  true));
    // Zone 2: Far (5000-8000u) — simplified AI, 2.0s tick, no perception
    DinoTickBudgets.Add(FPerf_DinoTickBudget(8000.0f,  2.0f,  false, false));
    // Zone 3: VeryFar (8000u+) — frozen AI, 5.0s tick
    DinoTickBudgets.Add(FPerf_DinoTickBudget(TNumericLimits<float>::Max(), 5.0f, false, false));
}

// ============================================================
// ApplyToWorld — push config to running world via console cmds
// ============================================================

void UPerf_PerformanceConfig::ApplyToWorld(UWorld* World)
{
    if (!World)
    {
        return;
    }

    // LOD distance scale
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.StaticMeshLODDistanceScale 1.0")));
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("foliage.LODDistanceScale 1.5")));

    // Shadow resolution
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.Shadow.MaxCSMResolution %d"), ShadowMapResolution));

    // Lumen GI
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.Lumen.GlobalIllumination.Allow %d"), bEnableLumenGI ? 1 : 0));
    UKismetSystemLibrary::ExecuteConsoleCommand(World,
        FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"), bEnableLumenReflections ? 1 : 0));

    // Occlusion culling
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.HZBOcclusion 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.OcclusionCullParallelUpdate 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.EarlyZPass 3"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.EarlyZPassMovable 1"));

    // AI crowd cap
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("ai.crowd.MaxAgents 50"));

    // Anisotropy
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.MaxAnisotropy 8"));

    // FastSkyLUT
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));

    UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Applied quality tier %d to world %s"),
        (int32)QualityTier, *World->GetName());
}

// ============================================================
// GetDinoTickInterval — returns tick interval for given distance
// ============================================================

float UPerf_PerformanceConfig::GetDinoTickInterval(float DistanceFromPlayer) const
{
    for (const FPerf_DinoTickBudget& Budget : DinoTickBudgets)
    {
        if (DistanceFromPlayer <= Budget.MaxDistanceUnits)
        {
            return Budget.TickIntervalSeconds;
        }
    }
    // Beyond all zones — use slowest interval
    return 5.0f;
}

// ============================================================
// GetLODZone — returns LOD zone enum for given distance
// ============================================================

EPerf_LODZone UPerf_PerformanceConfig::GetLODZone(float DistanceFromPlayer) const
{
    if (DistanceFromPlayer > LODThresholds.CullDistance)
    {
        return EPerf_LODZone::Culled;
    }
    if (DistanceFromPlayer > LODThresholds.LOD2_To_LOD3)
    {
        return EPerf_LODZone::VeryFar;
    }
    if (DistanceFromPlayer > LODThresholds.LOD1_To_LOD2)
    {
        return EPerf_LODZone::Far;
    }
    if (DistanceFromPlayer > LODThresholds.LOD0_To_LOD1)
    {
        return EPerf_LODZone::Mid;
    }
    return EPerf_LODZone::Near;
}

// ============================================================
// Get — singleton accessor via CDO
// ============================================================

UPerf_PerformanceConfig* UPerf_PerformanceConfig::Get(UObject* WorldContextObject)
{
    return GetMutableDefault<UPerf_PerformanceConfig>();
}

// PerformanceBudget.cpp
// Performance Optimizer Agent #04 — Transpersonal Game Studio
// Implementation of UPerf_BudgetSettings runtime methods.

#include "PerformanceBudget.h"

// ---------------------------------------------------------------------------
// UPerf_BudgetSettings
// ---------------------------------------------------------------------------

UPerf_BudgetSettings::UPerf_BudgetSettings()
{
    // Default values already set via UPROPERTY initialisers in header.
    // Constructor is intentionally minimal — CDO construction must be crash-free.
}

EPerf_TickPriority UPerf_BudgetSettings::GetTickPriorityForDistance(float DistanceCm) const
{
    if (DistanceCm <= TickConfig.FullRateDistance)
    {
        return EPerf_TickPriority::High;
    }
    else if (DistanceCm <= TickConfig.HalfRateDistance)
    {
        return EPerf_TickPriority::Medium;
    }
    else if (DistanceCm <= TickConfig.SleepDistance)
    {
        return EPerf_TickPriority::Low;
    }
    return EPerf_TickPriority::Sleeping;
}

bool UPerf_BudgetSettings::IsWithinActorBudget(int32 CurrentDinoCount, int32 CurrentCrowdCount) const
{
    const bool bDinoOk  = CurrentDinoCount  <= MemoryLimits.MaxFullAIDinos;
    const bool bCrowdOk = CurrentCrowdCount <= MemoryLimits.MaxCrowdAgents;
    return bDinoOk && bCrowdOk;
}

void UPerf_BudgetSettings::ApplyPlatformScaling(EPerf_PlatformTier Tier)
{
    PlatformTier = Tier;

    switch (Tier)
    {
    case EPerf_PlatformTier::HighEndPC:
        // Full quality — no scaling
        LODThresholds.LOD0ToLOD1  = 1500.0f;
        LODThresholds.LOD1ToLOD2  = 4000.0f;
        LODThresholds.LOD2ToLOD3  = 8000.0f;
        LODThresholds.CullDistance = 15000.0f;
        MemoryLimits.MaxFullAIDinos   = 12;
        MemoryLimits.MaxCrowdAgents   = 200;
        MemoryLimits.MaxNiagaraSystems = 16;
        FrameBudget.RenderBudgetMs    = 8.0f;
        FrameBudget.AIBudgetMs        = 2.0f;
        break;

    case EPerf_PlatformTier::MidPC:
        // Moderate scaling — ~80% of high-end
        LODThresholds.LOD0ToLOD1  = 1200.0f;
        LODThresholds.LOD1ToLOD2  = 3200.0f;
        LODThresholds.LOD2ToLOD3  = 6400.0f;
        LODThresholds.CullDistance = 12000.0f;
        MemoryLimits.MaxFullAIDinos   = 8;
        MemoryLimits.MaxCrowdAgents   = 120;
        MemoryLimits.MaxNiagaraSystems = 10;
        FrameBudget.RenderBudgetMs    = 10.0f;  // 30fps = 33.3ms budget
        FrameBudget.AIBudgetMs        = 2.5f;
        break;

    case EPerf_PlatformTier::Console:
        // Console 30fps target — 33.3ms total budget
        LODThresholds.LOD0ToLOD1  = 1000.0f;
        LODThresholds.LOD1ToLOD2  = 2500.0f;
        LODThresholds.LOD2ToLOD3  = 5000.0f;
        LODThresholds.CullDistance = 10000.0f;
        MemoryLimits.MaxFullAIDinos   = 6;
        MemoryLimits.MaxCrowdAgents   = 80;
        MemoryLimits.MaxNiagaraSystems = 8;
        FrameBudget.RenderBudgetMs    = 16.0f;
        FrameBudget.AIBudgetMs        = 3.0f;
        break;

    case EPerf_PlatformTier::LowEndPC:
        // Minimum spec — aggressive culling
        LODThresholds.LOD0ToLOD1  = 800.0f;
        LODThresholds.LOD1ToLOD2  = 2000.0f;
        LODThresholds.LOD2ToLOD3  = 4000.0f;
        LODThresholds.CullDistance = 8000.0f;
        MemoryLimits.MaxFullAIDinos   = 4;
        MemoryLimits.MaxCrowdAgents   = 40;
        MemoryLimits.MaxNiagaraSystems = 4;
        FrameBudget.RenderBudgetMs    = 20.0f;
        FrameBudget.AIBudgetMs        = 4.0f;
        break;

    default:
        break;
    }
}

// PerformanceBudget.cpp
// Performance Optimizer — Agent #04
// Implementation of UPerf_BudgetLibrary — per-platform budget queries and validation.

#include "PerformanceBudget.h"

// ─────────────────────────────────────────────────────────────────────────────
// GetFrameTargets
// ─────────────────────────────────────────────────────────────────────────────

FPerf_FrameTargets UPerf_BudgetLibrary::GetFrameTargets(EPerf_Platform Platform)
{
    FPerf_FrameTargets Targets;
    switch (Platform)
    {
    case EPerf_Platform::PC_High:
        Targets.FrameTimeMS        = 16.67f;  // 60 fps
        Targets.GPUBudgetMS        = 10.0f;
        Targets.CPUGameThreadMS    = 4.0f;
        Targets.CPURenderThreadMS  = 6.0f;
        break;

    case EPerf_Platform::PC_Mid:
        Targets.FrameTimeMS        = 22.22f;  // 45 fps
        Targets.GPUBudgetMS        = 14.0f;
        Targets.CPUGameThreadMS    = 5.0f;
        Targets.CPURenderThreadMS  = 8.0f;
        break;

    case EPerf_Platform::Console:
        Targets.FrameTimeMS        = 33.33f;  // 30 fps
        Targets.GPUBudgetMS        = 22.0f;
        Targets.CPUGameThreadMS    = 6.0f;
        Targets.CPURenderThreadMS  = 10.0f;
        break;

    case EPerf_Platform::Mobile:
        Targets.FrameTimeMS        = 33.33f;  // 30 fps
        Targets.GPUBudgetMS        = 25.0f;
        Targets.CPUGameThreadMS    = 8.0f;
        Targets.CPURenderThreadMS  = 12.0f;
        break;

    default:
        break;
    }
    return Targets;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetDrawCallBudget
// ─────────────────────────────────────────────────────────────────────────────

FPerf_DrawCallBudget UPerf_BudgetLibrary::GetDrawCallBudget(EPerf_Platform Platform)
{
    FPerf_DrawCallBudget Budget;
    switch (Platform)
    {
    case EPerf_Platform::PC_High:
        Budget.MaxStaticMeshActors   = 500;
        Budget.MaxSkeletalMeshActors = 50;
        Budget.MaxDynamicLights      = 20;
        Budget.MaxParticleSystems    = 30;
        Budget.MaxTotalActors        = 300;
        Budget.MaxDinoAIAgents       = 12;
        break;

    case EPerf_Platform::PC_Mid:
        Budget.MaxStaticMeshActors   = 300;
        Budget.MaxSkeletalMeshActors = 30;
        Budget.MaxDynamicLights      = 12;
        Budget.MaxParticleSystems    = 20;
        Budget.MaxTotalActors        = 200;
        Budget.MaxDinoAIAgents       = 8;
        break;

    case EPerf_Platform::Console:
        Budget.MaxStaticMeshActors   = 200;
        Budget.MaxSkeletalMeshActors = 20;
        Budget.MaxDynamicLights      = 8;
        Budget.MaxParticleSystems    = 15;
        Budget.MaxTotalActors        = 150;
        Budget.MaxDinoAIAgents       = 6;
        break;

    case EPerf_Platform::Mobile:
        Budget.MaxStaticMeshActors   = 100;
        Budget.MaxSkeletalMeshActors = 10;
        Budget.MaxDynamicLights      = 4;
        Budget.MaxParticleSystems    = 8;
        Budget.MaxTotalActors        = 80;
        Budget.MaxDinoAIAgents       = 3;
        break;

    default:
        break;
    }
    return Budget;
}

// ─────────────────────────────────────────────────────────────────────────────
// GetMemoryBudget
// ─────────────────────────────────────────────────────────────────────────────

FPerf_MemoryBudget UPerf_BudgetLibrary::GetMemoryBudget(EPerf_Platform Platform)
{
    FPerf_MemoryBudget Mem;
    switch (Platform)
    {
    case EPerf_Platform::PC_High:
        Mem.TextureStreamingPoolMB = 2048;
        Mem.MeshMemoryMB           = 1024;
        Mem.AudioMemoryMB          = 256;
        Mem.TotalGPUMemoryMB       = 8192;
        break;

    case EPerf_Platform::PC_Mid:
        Mem.TextureStreamingPoolMB = 1024;
        Mem.MeshMemoryMB           = 512;
        Mem.AudioMemoryMB          = 128;
        Mem.TotalGPUMemoryMB       = 4096;
        break;

    case EPerf_Platform::Console:
        Mem.TextureStreamingPoolMB = 512;
        Mem.MeshMemoryMB           = 256;
        Mem.AudioMemoryMB          = 64;
        Mem.TotalGPUMemoryMB       = 2048;
        break;

    case EPerf_Platform::Mobile:
        Mem.TextureStreamingPoolMB = 256;
        Mem.MeshMemoryMB           = 128;
        Mem.AudioMemoryMB          = 32;
        Mem.TotalGPUMemoryMB       = 1024;
        break;

    default:
        break;
    }
    return Mem;
}

// ─────────────────────────────────────────────────────────────────────────────
// CheckActorBudget
// ─────────────────────────────────────────────────────────────────────────────

EPerf_BudgetStatus UPerf_BudgetLibrary::CheckActorBudget(
    EPerf_Platform Platform,
    int32 CurrentStaticMeshCount,
    int32 CurrentSkeletalMeshCount,
    int32 CurrentDynamicLightCount,
    int32 AdditionalStatic,
    int32 AdditionalSkeletal,
    int32 AdditionalLights)
{
    const FPerf_DrawCallBudget Budget = GetDrawCallBudget(Platform);

    const int32 FutureStatic    = CurrentStaticMeshCount   + AdditionalStatic;
    const int32 FutureSkeletal  = CurrentSkeletalMeshCount + AdditionalSkeletal;
    const int32 FutureLights    = CurrentDynamicLightCount + AdditionalLights;

    // Hard violation — over cap
    if (FutureStatic   > Budget.MaxStaticMeshActors   ||
        FutureSkeletal > Budget.MaxSkeletalMeshActors  ||
        FutureLights   > Budget.MaxDynamicLights)
    {
        return EPerf_BudgetStatus::Violated;
    }

    // Critical — over 90% of cap
    const float WarnThreshold = 0.9f;
    if (FutureStatic   > FMath::FloorToInt(Budget.MaxStaticMeshActors   * WarnThreshold) ||
        FutureSkeletal > FMath::FloorToInt(Budget.MaxSkeletalMeshActors  * WarnThreshold) ||
        FutureLights   > FMath::FloorToInt(Budget.MaxDynamicLights       * WarnThreshold))
    {
        return EPerf_BudgetStatus::Critical;
    }

    // Warning — over 75% of cap
    const float CritThreshold = 0.75f;
    if (FutureStatic   > FMath::FloorToInt(Budget.MaxStaticMeshActors   * CritThreshold) ||
        FutureSkeletal > FMath::FloorToInt(Budget.MaxSkeletalMeshActors  * CritThreshold) ||
        FutureLights   > FMath::FloorToInt(Budget.MaxDynamicLights       * CritThreshold))
    {
        return EPerf_BudgetStatus::Warning;
    }

    return EPerf_BudgetStatus::OK;
}

// ─────────────────────────────────────────────────────────────────────────────
// IsWithinHardLimits
// ─────────────────────────────────────────────────────────────────────────────

bool UPerf_BudgetLibrary::IsWithinHardLimits(EPerf_Platform Platform, const FPerf_RuntimeSnapshot& Snapshot)
{
    const FPerf_DrawCallBudget Budget = GetDrawCallBudget(Platform);

    return (Snapshot.ActiveStaticMeshes   <= Budget.MaxStaticMeshActors   &&
            Snapshot.ActiveSkeletalMeshes <= Budget.MaxSkeletalMeshActors  &&
            Snapshot.ActiveDynamicLights  <= Budget.MaxDynamicLights       &&
            Snapshot.ActiveParticleSystems<= Budget.MaxParticleSystems);
}

// ─────────────────────────────────────────────────────────────────────────────
// GetRecommendedLODBias
// ─────────────────────────────────────────────────────────────────────────────

int32 UPerf_BudgetLibrary::GetRecommendedLODBias(EPerf_Platform Platform)
{
    switch (Platform)
    {
    case EPerf_Platform::PC_High:    return 0;
    case EPerf_Platform::PC_Mid:     return 1;
    case EPerf_Platform::Console:    return 1;
    case EPerf_Platform::Mobile:     return 2;
    default:                         return 0;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// GetRecommendedShadowResolution
// ─────────────────────────────────────────────────────────────────────────────

int32 UPerf_BudgetLibrary::GetRecommendedShadowResolution(EPerf_Platform Platform)
{
    switch (Platform)
    {
    case EPerf_Platform::PC_High:    return 2048;
    case EPerf_Platform::PC_Mid:     return 1024;
    case EPerf_Platform::Console:    return 1024;
    case EPerf_Platform::Mobile:     return 512;
    default:                         return 1024;
    }
}

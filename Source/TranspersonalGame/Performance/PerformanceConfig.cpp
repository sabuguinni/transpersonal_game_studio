// PerformanceConfig.cpp — Performance Optimizer Agent #4
// Implementation of frame budget and scalability configuration
#include "PerformanceConfig.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // Default to PC 60fps tier
    ActiveTier = EPerf_ScalabilityTier::PC_60fps;

    // PC 60fps frame budget defaults
    FrameBudget.TargetFrameTimeMs = 16.6f;
    FrameBudget.ShadowBudgetMs    = 3.0f;
    FrameBudget.LumenBudgetMs     = 4.0f;
    FrameBudget.MaxDrawCalls      = 2000;
    FrameBudget.MaxDynamicActors  = 500;
    FrameBudget.MaxDynamicLights  = 20;
    FrameBudget.MaxNiagaraSystems = 50;
    FrameBudget.TexturePoolMB     = 2048;

    // Default LOD settings
    LODSettings.StaticMeshLODScale      = 1.0f;
    LODSettings.SkeletalMeshLODBias     = 0;
    LODSettings.FoliageCullDistance     = 15000.0f;
    LODSettings.DinoAITickRateNear      = 30.0f;
    LODSettings.DinoAITickRateMid       = 10.0f;
    LODSettings.DinoAITickRateFar       = 2.0f;
    LODSettings.DynamicShadowDistance   = 5000.0f;
}

FPerf_FrameBudget UPerf_PerformanceConfig::GetBudgetForTier(EPerf_ScalabilityTier Tier) const
{
    FPerf_FrameBudget Budget;

    switch (Tier)
    {
    case EPerf_ScalabilityTier::Console_30fps:
        Budget.TargetFrameTimeMs = 33.3f;
        Budget.ShadowBudgetMs    = 5.0f;
        Budget.LumenBudgetMs     = 8.0f;
        Budget.MaxDrawCalls      = 1000;
        Budget.MaxDynamicActors  = 200;
        Budget.MaxDynamicLights  = 8;
        Budget.MaxNiagaraSystems = 20;
        Budget.TexturePoolMB     = 1024;
        break;

    case EPerf_ScalabilityTier::PC_60fps:
        Budget.TargetFrameTimeMs = 16.6f;
        Budget.ShadowBudgetMs    = 3.0f;
        Budget.LumenBudgetMs     = 4.0f;
        Budget.MaxDrawCalls      = 2000;
        Budget.MaxDynamicActors  = 500;
        Budget.MaxDynamicLights  = 20;
        Budget.MaxNiagaraSystems = 50;
        Budget.TexturePoolMB     = 2048;
        break;

    case EPerf_ScalabilityTier::PC_Ultra:
        Budget.TargetFrameTimeMs = 11.1f; // 90fps
        Budget.ShadowBudgetMs    = 2.0f;
        Budget.LumenBudgetMs     = 3.0f;
        Budget.MaxDrawCalls      = 4000;
        Budget.MaxDynamicActors  = 1000;
        Budget.MaxDynamicLights  = 40;
        Budget.MaxNiagaraSystems = 100;
        Budget.TexturePoolMB     = 4096;
        break;

    default:
        // Fallback to PC 60fps
        Budget.TargetFrameTimeMs = 16.6f;
        Budget.MaxDrawCalls      = 2000;
        break;
    }

    return Budget;
}

void UPerf_PerformanceConfig::ApplyScalabilitySettings(EPerf_ScalabilityTier Tier)
{
    // Get a world context — use GWorld as fallback
    UWorld* World = GWorld;
    if (!World)
    {
        return;
    }

    switch (Tier)
    {
    case EPerf_ScalabilityTier::Console_30fps:
        // Console quality settings — all groups at 2 (High)
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ResolutionQuality 100"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ViewDistanceQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.AntiAliasingQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadowQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.GlobalIlluminationQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ReflectionQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.PostProcessQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.TextureQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.EffectsQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.FoliageQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadingQuality 2"));
        // Lumen reduced for console
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.MaxTraceDistance 4000"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.MaxResolution 1024"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 2"));
        break;

    case EPerf_ScalabilityTier::PC_60fps:
        // PC Epic quality with performance tweaks
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ResolutionQuality 100"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ViewDistanceQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.AntiAliasingQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadowQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.GlobalIlluminationQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ReflectionQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.PostProcessQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.TextureQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.EffectsQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.FoliageQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadingQuality 3"));
        // Lumen balanced
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.MaxTraceDistance 8000"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.MaxResolution 2048"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.AllowOcclusionQueries 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.HZBOcclusion 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.MeshDrawCommands.DynamicInstancing 1"));
        break;

    case EPerf_ScalabilityTier::PC_Ultra:
        // Maximum quality — no compromises
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ResolutionQuality 100"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ViewDistanceQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.AntiAliasingQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadowQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.GlobalIlluminationQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ReflectionQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.PostProcessQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.TextureQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.EffectsQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.FoliageQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("sg.ShadingQuality 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.MaxTraceDistance 16000"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.MaxResolution 4096"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 4"));
        break;

    default:
        break;
    }

    // Update active tier and budget
    ActiveTier = Tier;
    FrameBudget = GetBudgetForTier(Tier);
}

EPerf_BudgetWarning UPerf_PerformanceConfig::CheckSceneBudget(
    int32 ActorCount,
    int32 DynamicLightCount,
    int32 DrawCallCount) const
{
    // Check in priority order — most critical first
    if (DynamicLightCount > FrameBudget.MaxDynamicLights)
    {
        return EPerf_BudgetWarning::DynamicLights;
    }

    if (DrawCallCount > FrameBudget.MaxDrawCalls)
    {
        return EPerf_BudgetWarning::DrawCalls;
    }

    if (ActorCount > FrameBudget.MaxDynamicActors)
    {
        return EPerf_BudgetWarning::ActorCount;
    }

    return EPerf_BudgetWarning::None;
}

float UPerf_PerformanceConfig::GetLumenTraceDistance(EPerf_ScalabilityTier Tier) const
{
    switch (Tier)
    {
    case EPerf_ScalabilityTier::Console_30fps: return 4000.0f;
    case EPerf_ScalabilityTier::PC_60fps:      return 8000.0f;
    case EPerf_ScalabilityTier::PC_Ultra:      return 16000.0f;
    default:                                    return 8000.0f;
    }
}

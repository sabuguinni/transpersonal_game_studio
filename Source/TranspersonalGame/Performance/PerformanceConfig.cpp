#include "PerformanceConfig.h"
#include "HAL/IConsoleManager.h"

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // Default LOD settings for dinosaurs
    DinoLODSettings.LOD0Distance  = 1500.0f;
    DinoLODSettings.LOD1Distance  = 3000.0f;
    DinoLODSettings.LOD2Distance  = 6000.0f;
    DinoLODSettings.CullDistance  = 8000.0f;

    // Default LOD settings for foliage (trees/plants can be culled sooner)
    FoliageLODSettings.LOD0Distance = 2000.0f;
    FoliageLODSettings.LOD1Distance = 5000.0f;
    FoliageLODSettings.LOD2Distance = 9000.0f;
    FoliageLODSettings.CullDistance = 12000.0f;

    // Default tick budget
    TickBudget.HeroTickInterval     = 0.016f;
    TickBudget.MidRangeTickInterval = 0.033f;
    TickBudget.DistantTickInterval  = 0.1f;
    TickBudget.AITickPauseDistance  = 6000.0f;

    // Default frame budget (High quality tier — PC mid-range)
    FrameBudget.TargetFrameTimeMs   = 16.6f;
    FrameBudget.DinoGPUBudgetMs     = 4.0f;
    FrameBudget.FoliageGPUBudgetMs  = 3.0f;
    FrameBudget.LightingGPUBudgetMs = 5.0f;
    FrameBudget.MaxSkeletalMeshActors = 20;
    FrameBudget.MaxStaticMeshActors   = 150;
}

void UPerf_PerformanceConfig::ApplyQualityTierCVars()
{
    IConsoleManager& CM = IConsoleManager::Get();

    switch (QualityTier)
    {
    case EPerf_QualityTier::Ultra:
        // Ultra: full quality, hardware ray tracing, max shadow res
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
            CV->Set(2048);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")))
            CV->Set(4);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing")))
            CV->Set(1);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.Scene.SurfaceCacheResolution")))
            CV->Set(1.0f);
        FrameBudget.TargetFrameTimeMs = 16.6f;
        break;

    case EPerf_QualityTier::High:
        // High: screen-space Lumen, 2 CSM cascades, 1024 shadow res
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
            CV->Set(1024);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")))
            CV->Set(2);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing")))
            CV->Set(0);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.Scene.SurfaceCacheResolution")))
            CV->Set(0.75f);
        FrameBudget.TargetFrameTimeMs = 16.6f;
        break;

    case EPerf_QualityTier::Medium:
        // Medium: console target — 30fps, reduced Lumen, 512 shadow res
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
            CV->Set(512);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")))
            CV->Set(2);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.HardwareRayTracing")))
            CV->Set(0);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.Scene.SurfaceCacheResolution")))
            CV->Set(0.5f);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")))
            CV->Set(1);
        FrameBudget.TargetFrameTimeMs = 33.3f;
        FrameBudget.MaxSkeletalMeshActors = 12;
        FrameBudget.MaxStaticMeshActors   = 100;
        break;

    case EPerf_QualityTier::Low:
        // Low: minimal quality, no Lumen, basic shadows
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.MaxResolution")))
            CV->Set(256);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades")))
            CV->Set(1);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.DiffuseIndirect.Allow")))
            CV->Set(0);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Lumen.Reflections.Allow")))
            CV->Set(0);
        if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.SkeletalMeshLODBias")))
            CV->Set(2);
        FrameBudget.TargetFrameTimeMs = 33.3f;
        FrameBudget.MaxSkeletalMeshActors = 8;
        FrameBudget.MaxStaticMeshActors   = 60;
        break;
    }

    // Common CVars across all tiers
    if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.AllowOcclusionQueries")))
        CV->Set(1);
    if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.TemporalAA.Upsampling")))
        CV->Set(1);
    if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.SkyAtmosphere.FastSkyLUT")))
        CV->Set(1);
    if (IConsoleVariable* CV = CM.FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge")))
        CV->Set(1.0f);
}

float UPerf_PerformanceConfig::GetTickIntervalForDistance(float DistanceFromPlayer) const
{
    if (DistanceFromPlayer < 1500.0f)
    {
        return TickBudget.HeroTickInterval;
    }
    else if (DistanceFromPlayer < 3000.0f)
    {
        return TickBudget.MidRangeTickInterval;
    }
    else if (DistanceFromPlayer < TickBudget.AITickPauseDistance)
    {
        return TickBudget.DistantTickInterval;
    }
    else
    {
        // Beyond pause distance — return a very large interval (effectively paused)
        return 10.0f;
    }
}

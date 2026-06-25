// PerformanceConfig.cpp
// Performance Optimizer — Agent #4
// Targets: 60fps high-end PC / 30fps console
// Prehistoric survival game — NO spiritual content

#include "PerformanceConfig.h"
#include "Kismet/KismetSystemLibrary.h"

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // ── PC High-End Budget (target 60fps) ─────────────────────────────────────
    PCHighEndBudget.TargetFrameTimeMs  = 16.67f;
    PCHighEndBudget.GPUBudgetMs        = 11.67f;
    PCHighEndBudget.CPUGameThreadMs    = 8.0f;
    PCHighEndBudget.CPURenderThreadMs  = 10.0f;
    PCHighEndBudget.MaxDrawCalls       = 2000;
    PCHighEndBudget.MaxTrianglesMillion = 8.0f;

    // ── Console Budget (target 30fps) ─────────────────────────────────────────
    ConsoleBudget.TargetFrameTimeMs  = 33.33f;
    ConsoleBudget.GPUBudgetMs        = 23.33f;
    ConsoleBudget.CPUGameThreadMs    = 16.0f;
    ConsoleBudget.CPURenderThreadMs  = 20.0f;
    ConsoleBudget.MaxDrawCalls       = 1200;
    ConsoleBudget.MaxTrianglesMillion = 5.0f;

    // ── Large Dino LOD (T-Rex scale ~3.0, Brachiosaurus scale ~2.5) ──────────
    LargeDinoLOD.LOD0Distance  = 2000.0f;
    LargeDinoLOD.LOD1Distance  = 4000.0f;
    LargeDinoLOD.LOD2Distance  = 8000.0f;
    LargeDinoLOD.CullDistance  = 18000.0f;
    LargeDinoLOD.LODBias       = 0;
    LargeDinoLOD.bUseScreenSizeLOD = true;

    // ── Medium Dino LOD (Raptor scale ~1.5, Triceratops scale ~2.0) ──────────
    MediumDinoLOD.LOD0Distance  = 1500.0f;
    MediumDinoLOD.LOD1Distance  = 3000.0f;
    MediumDinoLOD.LOD2Distance  = 6000.0f;
    MediumDinoLOD.CullDistance  = 12000.0f;
    MediumDinoLOD.LODBias       = 0;
    MediumDinoLOD.bUseScreenSizeLOD = true;

    // ── Foliage LOD ───────────────────────────────────────────────────────────
    FoliageLOD.LOD0Distance  = 800.0f;
    FoliageLOD.LOD1Distance  = 2000.0f;
    FoliageLOD.LOD2Distance  = 4000.0f;
    FoliageLOD.CullDistance  = 8000.0f;
    FoliageLOD.LODBias       = 1;   // Foliage uses lower LOD sooner
    FoliageLOD.bUseScreenSizeLOD = true;

    // ── Dinosaur Visibility Budget ────────────────────────────────────────────
    DinosaurBudget.MaxLargeDinosVisible      = 4;
    DinosaurBudget.MaxMediumDinosVisible     = 8;
    DinosaurBudget.MaxSmallDinosVisible      = 16;
    DinosaurBudget.AnimTickHalfRateDistance  = 4000.0f;
    DinosaurBudget.AnimTickPauseDistance     = 8000.0f;
}

FPerf_FrameBudget UPerf_PerformanceConfig::GetActiveBudget() const
{
    switch (ActiveQualityTier)
    {
        case EPerf_QualityTier::Low:
        case EPerf_QualityTier::Medium:
            return ConsoleBudget;

        case EPerf_QualityTier::High:
        case EPerf_QualityTier::Epic:
        case EPerf_QualityTier::Cinematic:
        default:
            return PCHighEndBudget;
    }
}

bool UPerf_PerformanceConfig::IsWithinDinosaurBudget(int32 LargeCount, int32 MediumCount, int32 SmallCount) const
{
    return (LargeCount  <= DinosaurBudget.MaxLargeDinosVisible)
        && (MediumCount <= DinosaurBudget.MaxMediumDinosVisible)
        && (SmallCount  <= DinosaurBudget.MaxSmallDinosVisible);
}

void UPerf_PerformanceConfig::ApplyQualityTierToConsole()
{
    // Build a world context object for ExecuteConsoleCommand
    // In editor context, pass nullptr — the command is applied globally
    UWorld* World = nullptr;

    // Find any valid world
    for (const FWorldContext& Ctx : GEngine->GetWorldContexts())
    {
        if (Ctx.World())
        {
            World = Ctx.World();
            break;
        }
    }

    if (!GEngine || !World)
    {
        return;
    }

    auto Exec = [&](const FString& Cmd)
    {
        GEngine->Exec(World, *Cmd);
    };

    switch (ActiveQualityTier)
    {
        case EPerf_QualityTier::Low:
            Exec(TEXT("sg.ResolutionQuality 50"));
            Exec(TEXT("sg.ViewDistanceQuality 0"));
            Exec(TEXT("sg.AntiAliasingQuality 0"));
            Exec(TEXT("sg.ShadowQuality 0"));
            Exec(TEXT("sg.GlobalIlluminationQuality 0"));
            Exec(TEXT("sg.ReflectionQuality 0"));
            Exec(TEXT("sg.PostProcessQuality 0"));
            Exec(TEXT("sg.TextureQuality 0"));
            Exec(TEXT("sg.EffectsQuality 0"));
            Exec(TEXT("sg.FoliageQuality 0"));
            Exec(TEXT("sg.ShadingQuality 0"));
            Exec(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
            Exec(TEXT("r.Lumen.HardwareRayTracing 0"));
            break;

        case EPerf_QualityTier::Medium:
            Exec(TEXT("sg.ResolutionQuality 75"));
            Exec(TEXT("sg.ViewDistanceQuality 1"));
            Exec(TEXT("sg.AntiAliasingQuality 1"));
            Exec(TEXT("sg.ShadowQuality 1"));
            Exec(TEXT("sg.GlobalIlluminationQuality 1"));
            Exec(TEXT("sg.ReflectionQuality 1"));
            Exec(TEXT("sg.PostProcessQuality 1"));
            Exec(TEXT("sg.TextureQuality 1"));
            Exec(TEXT("sg.EffectsQuality 1"));
            Exec(TEXT("sg.FoliageQuality 1"));
            Exec(TEXT("sg.ShadingQuality 1"));
            Exec(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
            Exec(TEXT("r.Lumen.HardwareRayTracing 0"));
            break;

        case EPerf_QualityTier::High:
            Exec(TEXT("sg.ResolutionQuality 90"));
            Exec(TEXT("sg.ViewDistanceQuality 2"));
            Exec(TEXT("sg.AntiAliasingQuality 2"));
            Exec(TEXT("sg.ShadowQuality 2"));
            Exec(TEXT("sg.GlobalIlluminationQuality 2"));
            Exec(TEXT("sg.ReflectionQuality 2"));
            Exec(TEXT("sg.PostProcessQuality 2"));
            Exec(TEXT("sg.TextureQuality 2"));
            Exec(TEXT("sg.EffectsQuality 2"));
            Exec(TEXT("sg.FoliageQuality 2"));
            Exec(TEXT("sg.ShadingQuality 2"));
            Exec(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
            Exec(TEXT("r.Lumen.HardwareRayTracing 0"));
            break;

        case EPerf_QualityTier::Epic:
        case EPerf_QualityTier::Cinematic:
        default:
            Exec(TEXT("sg.ResolutionQuality 100"));
            Exec(TEXT("sg.ViewDistanceQuality 3"));
            Exec(TEXT("sg.AntiAliasingQuality 3"));
            Exec(TEXT("sg.ShadowQuality 3"));
            Exec(TEXT("sg.GlobalIlluminationQuality 3"));
            Exec(TEXT("sg.ReflectionQuality 3"));
            Exec(TEXT("sg.PostProcessQuality 3"));
            Exec(TEXT("sg.TextureQuality 3"));
            Exec(TEXT("sg.EffectsQuality 3"));
            Exec(TEXT("sg.FoliageQuality 3"));
            Exec(TEXT("sg.ShadingQuality 3"));
            Exec(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
            Exec(TEXT("r.Lumen.HardwareRayTracing 0"));
            break;
    }
}

FPerf_LODConfig UPerf_PerformanceConfig::GetLODConfigForScale(float DinoScale) const
{
    // Large: scale >= 2.5 (T-Rex at 3.0, Brachiosaurus at 2.5)
    if (DinoScale >= 2.5f)
    {
        return LargeDinoLOD;
    }
    // Medium: scale 1.0 - 2.5 (Raptor at 1.5, Triceratops at 2.0)
    return MediumDinoLOD;
}

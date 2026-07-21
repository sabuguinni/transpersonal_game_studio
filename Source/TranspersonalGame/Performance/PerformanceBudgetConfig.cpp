// PerformanceBudgetConfig.cpp
// Performance Optimizer #04 — Implementation
// Cycle: PROD_CYCLE_AUTO_20260626_002

#include "PerformanceBudgetConfig.h"
#include "Engine/Engine.h"

UPerf_PerformanceBudgetConfig::UPerf_PerformanceBudgetConfig()
{
    // PC High-End budget (60fps = 16.67ms)
    PCHighBudget.TotalBudgetMs        = 16.67f;
    PCHighBudget.LumenGIBudgetMs      = 4.0f;
    PCHighBudget.ShadowBudgetMs       = 2.5f;
    PCHighBudget.SkeletalMeshBudgetMs = 3.0f;
    PCHighBudget.StaticMeshBudgetMs   = 2.0f;
    PCHighBudget.ParticlesBudgetMs    = 1.0f;
    PCHighBudget.GameThreadBudgetMs   = 4.0f;
    // Headroom: 16.67 - (4+2.5+3+2+1+4) = 0.17ms — tight but achievable with Nanite

    // Console budget (30fps = 33.33ms)
    ConsoleBudget.TotalBudgetMs        = 33.33f;
    ConsoleBudget.LumenGIBudgetMs      = 8.0f;
    ConsoleBudget.ShadowBudgetMs       = 4.0f;
    ConsoleBudget.SkeletalMeshBudgetMs = 5.0f;
    ConsoleBudget.StaticMeshBudgetMs   = 4.0f;
    ConsoleBudget.ParticlesBudgetMs    = 2.0f;
    ConsoleBudget.GameThreadBudgetMs   = 6.0f;
    // Headroom: 33.33 - (8+4+5+4+2+6) = 4.33ms — comfortable

    // LOD thresholds — tuned for prehistoric open world
    LODThresholds.DinoLOD1Distance         = 1500.0f;  // 15m — start simplifying
    LODThresholds.DinoLOD2Distance         = 4000.0f;  // 40m — low poly
    LODThresholds.DinoCullDistance         = 8000.0f;  // 80m — cull entirely
    LODThresholds.FoliageCullDistance      = 5000.0f;  // 50m — foliage cull
    LODThresholds.NPCURODistance           = 2000.0f;  // 20m — URO threshold
    LODThresholds.MaxAnimatedSkeletalMeshes = 12;       // max 12 animated dinos simultaneously

    // Shadow budget
    ShadowBudget.MaxCSMCascades        = 3;
    ShadowBudget.MaxShadowResolution   = 2048;
    ShadowBudget.MaxDynamicShadowLights = 4;
    ShadowBudget.ShadowCullDistance    = 6000.0f;
}

FPerf_FrameBudget UPerf_PerformanceBudgetConfig::GetActiveBudget() const
{
    switch (TargetPlatform)
    {
    case EPerf_TargetPlatform::Console:
    case EPerf_TargetPlatform::Mobile:
        return ConsoleBudget;
    default:
        return PCHighBudget;
    }
}

EPerf_BudgetStatus UPerf_PerformanceBudgetConfig::EvaluateBudgetStatus(float MeasuredFrameMs) const
{
    const FPerf_FrameBudget& Budget = GetActiveBudget();
    const float Headroom = Budget.TotalBudgetMs - MeasuredFrameMs;

    if (Headroom > 4.0f)
        return EPerf_BudgetStatus::Comfortable;
    else if (Headroom > 1.0f)
        return EPerf_BudgetStatus::Tight;
    else
        return EPerf_BudgetStatus::OverBudget;
}

FString UPerf_PerformanceBudgetConfig::GetPerformanceReport() const
{
    const FPerf_FrameBudget& Budget = GetActiveBudget();
    return FString::Printf(
        TEXT("=== PERF BUDGET REPORT ===\n"
             "Platform: %s\n"
             "Total Budget: %.2fms (%.0f fps)\n"
             "  Lumen GI:      %.2fms\n"
             "  Shadows:       %.2fms\n"
             "  Skeletal Mesh: %.2fms (max %d animated)\n"
             "  Static Mesh:   %.2fms (Nanite)\n"
             "  Particles:     %.2fms (max %d systems)\n"
             "  Game Thread:   %.2fms\n"
             "  Headroom:      %.2fms\n"
             "CAP Limits: %d dinos, %d foliage, %d lights\n"
             "Streaming Pool: %dMB"),
        TargetPlatform == EPerf_TargetPlatform::Console ? TEXT("Console") : TEXT("PC High"),
        Budget.TotalBudgetMs,
        1000.0f / Budget.TotalBudgetMs,
        Budget.LumenGIBudgetMs,
        Budget.ShadowBudgetMs,
        Budget.SkeletalMeshBudgetMs,
        LODThresholds.MaxAnimatedSkeletalMeshes,
        Budget.StaticMeshBudgetMs,
        Budget.ParticlesBudgetMs,
        MaxParticleSystemsActive,
        Budget.GameThreadBudgetMs,
        Budget.GetHeadroomMs(),
        MaxDinosInScene,
        MaxFoliageActors,
        MaxDynamicLights,
        TextureStreamingPoolMB
    );
}

void UPerf_PerformanceBudgetConfig::ApplyConsoleCommandsForPlatform()
{
    if (!GEngine) return;

    // Core rendering
    GEngine->Exec(nullptr, TEXT("r.Lumen.Reflections.Allow 1"));
    GEngine->Exec(nullptr, TEXT("r.Lumen.GlobalIllumination.Allow 1"));
    GEngine->Exec(nullptr, TEXT("r.Nanite.Enabled 1"));

    // Sky atmosphere — FastSkyLUT saves ~1ms
    GEngine->Exec(nullptr, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    GEngine->Exec(nullptr, TEXT("r.SkyAtmosphere.AerialPerspectiveLUT.FastApplyOnOpaque 1"));

    // Shadow budget
    GEngine->Exec(nullptr, *FString::Printf(TEXT("r.Shadow.MaxResolution %d"), ShadowBudget.MaxShadowResolution));
    GEngine->Exec(nullptr, *FString::Printf(TEXT("r.Shadow.CSM.MaxCascades %d"), ShadowBudget.MaxCSMCascades));

    // LOD
    GEngine->Exec(nullptr, TEXT("r.StaticMeshLODDistanceScale 1.0"));
    GEngine->Exec(nullptr, TEXT("r.SkeletalMeshLODBias 0"));

    // Foliage culling
    GEngine->Exec(nullptr, TEXT("foliage.MinimumScreenSize 0.0002"));
    GEngine->Exec(nullptr, TEXT("foliage.LODDistanceScale 1.0"));

    // URO for distant NPCs/dinos
    GEngine->Exec(nullptr, TEXT("a.URO.Enable 1"));
    GEngine->Exec(nullptr, TEXT("a.URO.ForceAnimRate 0"));

    // Streaming
    GEngine->Exec(nullptr, *FString::Printf(TEXT("r.Streaming.PoolSize %d"), TextureStreamingPoolMB));

    UE_LOG(LogTemp, Log, TEXT("[PerformanceBudgetConfig] Console commands applied for platform: %s"),
        TargetPlatform == EPerf_TargetPlatform::Console ? TEXT("Console") : TEXT("PC High"));
}

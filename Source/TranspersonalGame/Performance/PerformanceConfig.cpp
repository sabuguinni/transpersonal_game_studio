// PerformanceConfig.cpp
// Performance Optimizer — Agent #04
// WorldSubsystem that applies quality presets and tracks frame budget.
// Targets: 60fps PC High / 30fps Console.

#include "PerformanceConfig.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// USubsystem interface
// ─────────────────────────────────────────────────────────────────────────────

void UPerformanceConfig::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Default to PC High (60fps) on startup
    ApplyQualityPreset(EPerf_QualityPreset::PCHigh);

    UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Initialized — preset PCHigh applied."));
}

void UPerformanceConfig::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Deinitialized."));
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyQualityPreset
// ─────────────────────────────────────────────────────────────────────────────

void UPerformanceConfig::ApplyQualityPreset(EPerf_QualityPreset Preset)
{
    CurrentPreset = Preset;
    CurrentLOD    = BuildLODSettings(Preset);
    CurrentBudget = BuildFrameBudget(Preset);

    ForceApplyConsoleCommands();

    UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Preset applied: %d"), static_cast<int32>(Preset));
}

// ─────────────────────────────────────────────────────────────────────────────
// ForceApplyConsoleCommands — apply all CVars for the current preset
// ─────────────────────────────────────────────────────────────────────────────

void UPerformanceConfig::ForceApplyConsoleCommands()
{
    const FPerf_LODSettings& L = CurrentLOD;

    // Skeletal mesh LOD
    ExecCmd(FString::Printf(TEXT("r.SkeletalMeshLODBias %d"), L.SkeletalLODBias));

    // Static mesh LOD distance
    ExecCmd(FString::Printf(TEXT("r.StaticMesh.LODDistanceScale %.2f"), L.StaticLODDistanceScale));

    // Foliage density
    ExecCmd(FString::Printf(TEXT("r.Foliage.DensityScale %.2f"), L.FoliageDensityScale));

    // Shadow cascades
    ExecCmd(FString::Printf(TEXT("r.Shadow.CSM.MaxCascades %d"), L.ShadowCascades));

    // Shadow distance scale
    ExecCmd(FString::Printf(TEXT("r.Shadow.DistanceScale %.2f"), L.ShadowDistanceScale));

    // Anisotropy
    ExecCmd(FString::Printf(TEXT("r.MaxAnisotropy %d"), L.MaxAnisotropy));

    // Texture streaming pool
    ExecCmd(FString::Printf(TEXT("r.Streaming.PoolSize %d"), L.TextureStreamingPoolMB));

    // Always-on performance settings (preset-independent)
    ExecCmd(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    ExecCmd(TEXT("r.HZBOcclusion 1"));
    ExecCmd(TEXT("r.OcclusionCullingEnabled 1"));
    ExecCmd(TEXT("r.EyeAdaptation.MethodOverride 1"));
    ExecCmd(TEXT("r.DynamicGlobalIlluminationMethod 1"));   // Lumen GI
    ExecCmd(TEXT("r.ReflectionMethod 1"));                  // Lumen reflections
    ExecCmd(TEXT("r.Nanite.MaxPixelsPerEdge 1.0"));

    UE_LOG(LogTemp, Log, TEXT("[PerformanceConfig] Console commands applied for preset %d."),
           static_cast<int32>(CurrentPreset));
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildLODSettings — preset-specific LOD parameters
// ─────────────────────────────────────────────────────────────────────────────

FPerf_LODSettings UPerformanceConfig::BuildLODSettings(EPerf_QualityPreset Preset) const
{
    FPerf_LODSettings S;

    switch (Preset)
    {
    case EPerf_QualityPreset::Console:
        S.SkeletalLODBias          = 1;
        S.StaticLODDistanceScale   = 0.7f;
        S.FoliageDensityScale      = 0.5f;
        S.ShadowCascades           = 2;
        S.ShadowDistanceScale      = 0.5f;
        S.MaxAnisotropy            = 4;
        S.TextureStreamingPoolMB   = 512;
        break;

    case EPerf_QualityPreset::PCMid:
        S.SkeletalLODBias          = 0;
        S.StaticLODDistanceScale   = 0.85f;
        S.FoliageDensityScale      = 0.7f;
        S.ShadowCascades           = 3;
        S.ShadowDistanceScale      = 0.7f;
        S.MaxAnisotropy            = 8;
        S.TextureStreamingPoolMB   = 1024;
        break;

    case EPerf_QualityPreset::PCHigh:
        S.SkeletalLODBias          = 0;
        S.StaticLODDistanceScale   = 1.0f;
        S.FoliageDensityScale      = 0.85f;
        S.ShadowCascades           = 3;
        S.ShadowDistanceScale      = 0.85f;
        S.MaxAnisotropy            = 8;
        S.TextureStreamingPoolMB   = 2048;
        break;

    case EPerf_QualityPreset::PCUltra:
        S.SkeletalLODBias          = 0;
        S.StaticLODDistanceScale   = 1.2f;
        S.FoliageDensityScale      = 1.0f;
        S.ShadowCascades           = 4;
        S.ShadowDistanceScale      = 1.0f;
        S.MaxAnisotropy            = 16;
        S.TextureStreamingPoolMB   = 4096;
        break;
    }

    return S;
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildFrameBudget — ms budget per preset
// ─────────────────────────────────────────────────────────────────────────────

FPerf_FrameBudget UPerformanceConfig::BuildFrameBudget(EPerf_QualityPreset Preset) const
{
    FPerf_FrameBudget B;

    switch (Preset)
    {
    case EPerf_QualityPreset::Console:
        B.TotalBudgetMs   = 33.3f;
        B.GPURenderMs     = 22.0f;
        B.CPUGameThreadMs = 7.0f;
        B.AIBudgetMs      = 3.0f;
        B.PhysicsBudgetMs = 1.3f;
        break;

    case EPerf_QualityPreset::PCMid:
        B.TotalBudgetMs   = 22.2f;  // ~45fps
        B.GPURenderMs     = 14.0f;
        B.CPUGameThreadMs = 5.0f;
        B.AIBudgetMs      = 2.0f;
        B.PhysicsBudgetMs = 1.2f;
        break;

    case EPerf_QualityPreset::PCHigh:
        B.TotalBudgetMs   = 16.6f;  // 60fps
        B.GPURenderMs     = 10.0f;
        B.CPUGameThreadMs = 4.0f;
        B.AIBudgetMs      = 1.5f;
        B.PhysicsBudgetMs = 1.1f;
        break;

    case EPerf_QualityPreset::PCUltra:
        B.TotalBudgetMs   = 16.6f;  // still targeting 60fps
        B.GPURenderMs     = 11.0f;
        B.CPUGameThreadMs = 3.5f;
        B.AIBudgetMs      = 1.0f;
        B.PhysicsBudgetMs = 1.1f;
        break;
    }

    return B;
}

// ─────────────────────────────────────────────────────────────────────────────
// ExecCmd — helper to execute a console command in the current world
// ─────────────────────────────────────────────────────────────────────────────

void UPerformanceConfig::ExecCmd(const FString& Cmd) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceConfig] ExecCmd — no world: %s"), *Cmd);
        return;
    }

    if (GEngine)
    {
        GEngine->Exec(World, *Cmd);
    }
}

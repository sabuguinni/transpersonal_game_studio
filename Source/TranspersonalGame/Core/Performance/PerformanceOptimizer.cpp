// PerformanceOptimizer.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260622_009
// Runtime performance manager: scalability tiers, LOD bias, Lumen budget, BiomeTick throttle

#include "PerformanceOptimizer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

UPerformanceOptimizer::UPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Check every 1s — not per-frame

    TargetFPS_PC = 60.0f;
    TargetFPS_Console = 30.0f;
    CurrentTier = EPerf_ScalabilityTier::High;
    BiomeTickInterval = 5.0f;
    SprintStaminaDrainRate = 15.0f;
    SprintStaminaRecoveryRate = 8.0f;
    MaxDrawCallBudget_PC = 2000;
    MaxDrawCallBudget_Console = 800;
    LumenMaxTraceDistance = 20000.0f;
    ShadowMaxCSMResolution = 2048;
    StreamingPoolSizeMB = 2048;
    bAdaptiveScalabilityEnabled = true;
    FrameTimeAccumulator = 0.0f;
    FrameSampleCount = 0;
    AverageFPS = 60.0f;
}

void UPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    ApplyTier(CurrentTier);
}

void UPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bAdaptiveScalabilityEnabled) return;

    // Rolling average FPS over 60 samples (~1 min at 1s tick)
    FrameTimeAccumulator += DeltaTime;
    FrameSampleCount++;
    if (FrameSampleCount >= 10)
    {
        AverageFPS = (float)FrameSampleCount / FrameTimeAccumulator;
        FrameTimeAccumulator = 0.0f;
        FrameSampleCount = 0;
        AdaptScalability(AverageFPS);
    }
}

void UPerformanceOptimizer::ApplyTier(EPerf_ScalabilityTier Tier)
{
    CurrentTier = Tier;
    UWorld* World = GetWorld();
    if (!World) return;

    switch (Tier)
    {
        case EPerf_ScalabilityTier::Ultra:
            ExecConsoleCmd(World, TEXT("sg.ResolutionQuality 100"));
            ExecConsoleCmd(World, TEXT("sg.ViewDistanceQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.AntiAliasingQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.ShadowQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.GlobalIlluminationQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.ReflectionQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.PostProcessQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.TextureQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.EffectsQuality 4"));
            ExecConsoleCmd(World, TEXT("sg.FoliageQuality 4"));
            ExecConsoleCmd(World, TEXT("r.Lumen.MaxTraceDistance 30000"));
            ExecConsoleCmd(World, TEXT("r.Shadow.MaxCSMResolution 4096"));
            break;

        case EPerf_ScalabilityTier::High:
            ExecConsoleCmd(World, TEXT("sg.ResolutionQuality 100"));
            ExecConsoleCmd(World, TEXT("sg.ViewDistanceQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.AntiAliasingQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.ShadowQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.GlobalIlluminationQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.ReflectionQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.PostProcessQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.TextureQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.EffectsQuality 3"));
            ExecConsoleCmd(World, TEXT("sg.FoliageQuality 3"));
            ExecConsoleCmd(World, TEXT("r.Lumen.MaxTraceDistance 20000"));
            ExecConsoleCmd(World, TEXT("r.Shadow.MaxCSMResolution 2048"));
            break;

        case EPerf_ScalabilityTier::Medium:
            ExecConsoleCmd(World, TEXT("sg.ResolutionQuality 85"));
            ExecConsoleCmd(World, TEXT("sg.ViewDistanceQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.AntiAliasingQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.ShadowQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.GlobalIlluminationQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.ReflectionQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.PostProcessQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.TextureQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.EffectsQuality 2"));
            ExecConsoleCmd(World, TEXT("sg.FoliageQuality 2"));
            ExecConsoleCmd(World, TEXT("r.Lumen.MaxTraceDistance 10000"));
            ExecConsoleCmd(World, TEXT("r.Shadow.MaxCSMResolution 1024"));
            break;

        case EPerf_ScalabilityTier::Low:
            ExecConsoleCmd(World, TEXT("sg.ResolutionQuality 70"));
            ExecConsoleCmd(World, TEXT("sg.ViewDistanceQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.AntiAliasingQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.ShadowQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.GlobalIlluminationQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.ReflectionQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.PostProcessQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.TextureQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.EffectsQuality 1"));
            ExecConsoleCmd(World, TEXT("sg.FoliageQuality 1"));
            ExecConsoleCmd(World, TEXT("r.Lumen.MaxTraceDistance 5000"));
            ExecConsoleCmd(World, TEXT("r.Shadow.MaxCSMResolution 512"));
            break;
    }

    // Always-on settings regardless of tier
    ExecConsoleCmd(World, TEXT("r.HZBOcclusion 1"));
    ExecConsoleCmd(World, TEXT("r.AllowOcclusionQueries 1"));
    ExecConsoleCmd(World, TEXT("r.TextureStreaming 1"));
    ExecConsoleCmd(World, TEXT("r.TemporalAA.Upsampling 1"));
    ExecConsoleCmd(World, TEXT("r.Nanite.MaxPixelsPerEdge 1.0"));
    ExecConsoleCmd(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    ExecConsoleCmd(World, TEXT("r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1"));
}

void UPerformanceOptimizer::AdaptScalability(float CurrentAverageFPS)
{
    // Adaptive tier stepping — only step down/up one tier at a time
    const float TargetFPS = TargetFPS_PC;

    if (CurrentAverageFPS < TargetFPS * 0.75f) // Below 75% of target — step down
    {
        int32 TierInt = (int32)CurrentTier;
        if (TierInt > 0)
        {
            ApplyTier((EPerf_ScalabilityTier)(TierInt - 1));
            UE_LOG(LogTemp, Warning, TEXT("PerformanceOptimizer: FPS=%.1f below target %.1f — stepped down to tier %d"), CurrentAverageFPS, TargetFPS, TierInt - 1);
        }
    }
    else if (CurrentAverageFPS > TargetFPS * 1.2f) // 20% headroom — step up
    {
        int32 TierInt = (int32)CurrentTier;
        if (TierInt < (int32)EPerf_ScalabilityTier::Ultra)
        {
            ApplyTier((EPerf_ScalabilityTier)(TierInt + 1));
            UE_LOG(LogTemp, Log, TEXT("PerformanceOptimizer: FPS=%.1f above target %.1f — stepped up to tier %d"), CurrentAverageFPS, TargetFPS, TierInt + 1);
        }
    }
}

float UPerformanceOptimizer::GetBiomeTickInterval() const
{
    return BiomeTickInterval;
}

float UPerformanceOptimizer::GetSprintDrainRate() const
{
    return SprintStaminaDrainRate;
}

float UPerformanceOptimizer::GetSprintRecoveryRate() const
{
    return SprintStaminaRecoveryRate;
}

EPerf_ScalabilityTier UPerformanceOptimizer::GetCurrentTier() const
{
    return CurrentTier;
}

float UPerformanceOptimizer::GetAverageFPS() const
{
    return AverageFPS;
}

void UPerformanceOptimizer::ExecConsoleCmd(UWorld* World, const FString& Cmd)
{
    if (World)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, Cmd);
    }
}

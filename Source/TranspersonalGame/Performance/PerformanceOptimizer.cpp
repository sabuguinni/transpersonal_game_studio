// PerformanceOptimizer.cpp
// Runtime performance manager — 60fps PC / 30fps console targets
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260622_003

#include "PerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

APerf_PerformanceOptimizer::APerf_PerformanceOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Check every 0.5s to reduce overhead
}

void APerf_PerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial quality preset on game start
    ApplyQualityPreset(CurrentQualityLevel);
}

void APerf_PerformanceOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFrameStats(DeltaTime);

    if (bEnableDynamicResolution)
    {
        ApplyDynamicResolutionScaling();
    }
}

void APerf_PerformanceOptimizer::UpdateFrameStats(float DeltaTime)
{
    CurrentFrameStats.FrameTimeMs = DeltaTime * 1000.0f;

    // Rolling average FPS
    AccumulatedTime += DeltaTime;
    FrameCount++;
    if (AccumulatedTime >= 1.0f)
    {
        AverageFPS = static_cast<float>(FrameCount) / AccumulatedTime;
        AccumulatedTime = 0.0f;
        FrameCount = 0;
    }

    CurrentFrameStats.bBudgetExceeded = (CurrentFrameStats.FrameTimeMs > FrameBudgetMs);
}

void APerf_PerformanceOptimizer::ApplyDynamicResolutionScaling()
{
    if (!IsFrameBudgetExceeded())
    {
        return;
    }

    // Reduce resolution scale when over budget
    float CurrentScale = FMath::Clamp(AverageFPS / TargetFPS_PC, MinResolutionScale, MaxResolutionScale);
    FString Cmd = FString::Printf(TEXT("r.ScreenPercentage %d"), FMath::RoundToInt(CurrentScale * 100.0f));
    ExecuteConsoleCommand(Cmd);
}

void APerf_PerformanceOptimizer::ApplyQualityPreset(EPerf_QualityLevel Level)
{
    CurrentQualityLevel = Level;

    switch (Level)
    {
    case EPerf_QualityLevel::Low:
        // Console / low-end PC — 30fps target
        FrameBudgetMs = 33.33f;
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 1"));
        ExecuteConsoleCommand(TEXT("r.TextureQuality 1"));
        ExecuteConsoleCommand(TEXT("r.PostProcessAAQuality 2"));
        ExecuteConsoleCommand(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 4"));
        ExecuteConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 2.0"));
        ExecuteConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 0.75"));
        ExecuteConsoleCommand(TEXT("r.Shadow.CSM.MaxCascades 2"));
        ExecuteConsoleCommand(TEXT("r.VolumetricFog.GridSizeZ 32"));
        break;

    case EPerf_QualityLevel::Medium:
        // Mid-range PC — 60fps target
        FrameBudgetMs = 16.67f;
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 2"));
        ExecuteConsoleCommand(TEXT("r.TextureQuality 2"));
        ExecuteConsoleCommand(TEXT("r.PostProcessAAQuality 4"));
        ExecuteConsoleCommand(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 2"));
        ExecuteConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 1.0"));
        ExecuteConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.0"));
        ExecuteConsoleCommand(TEXT("r.Shadow.CSM.MaxCascades 3"));
        ExecuteConsoleCommand(TEXT("r.VolumetricFog.GridSizeZ 64"));
        break;

    case EPerf_QualityLevel::High:
        // High-end PC — 60fps with full quality
        FrameBudgetMs = 16.67f;
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 4"));
        ExecuteConsoleCommand(TEXT("r.TextureQuality 3"));
        ExecuteConsoleCommand(TEXT("r.PostProcessAAQuality 6"));
        ExecuteConsoleCommand(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 2"));
        ExecuteConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 1.0"));
        ExecuteConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.0"));
        ExecuteConsoleCommand(TEXT("r.Shadow.CSM.MaxCascades 4"));
        ExecuteConsoleCommand(TEXT("r.VolumetricFog.GridSizeZ 64"));
        break;

    case EPerf_QualityLevel::Ultra:
        // Enthusiast PC — uncapped, maximum fidelity
        FrameBudgetMs = 11.11f; // ~90fps target
        ExecuteConsoleCommand(TEXT("r.ShadowQuality 5"));
        ExecuteConsoleCommand(TEXT("r.TextureQuality 4"));
        ExecuteConsoleCommand(TEXT("r.PostProcessAAQuality 6"));
        ExecuteConsoleCommand(TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 1"));
        ExecuteConsoleCommand(TEXT("r.Nanite.MaxPixelsPerEdge 0.5"));
        ExecuteConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.25"));
        ExecuteConsoleCommand(TEXT("r.Shadow.CSM.MaxCascades 4"));
        ExecuteConsoleCommand(TEXT("r.VolumetricFog.GridSizeZ 128"));
        break;
    }
}

void APerf_PerformanceOptimizer::ForceUpdateLODs()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    int32 UpdatedCount = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor)
        {
            continue;
        }

        TArray<UStaticMeshComponent*> MeshComps;
        Actor->GetComponents<UStaticMeshComponent>(MeshComps);
        for (UStaticMeshComponent* Comp : MeshComps)
        {
            if (Comp)
            {
                Comp->MarkRenderStateDirty();
                UpdatedCount++;
            }
        }
    }
}

float APerf_PerformanceOptimizer::GetCurrentFPS() const
{
    return AverageFPS;
}

bool APerf_PerformanceOptimizer::IsFrameBudgetExceeded() const
{
    return CurrentFrameStats.bBudgetExceeded;
}

void APerf_PerformanceOptimizer::ExecuteConsoleCommand(const FString& Command)
{
    UWorld* World = GetWorld();
    if (World)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, Command);
    }
}

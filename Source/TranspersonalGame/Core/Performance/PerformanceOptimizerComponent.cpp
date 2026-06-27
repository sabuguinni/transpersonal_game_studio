// PerformanceOptimizerComponent.cpp
// Agent #4 — Performance Optimizer
// Targets: 60fps PC high-end / 30fps console
// Strategy: rolling frame-time average -> tier downscale/upscale -> console commands

#include "PerformanceOptimizerComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"

UPerf_PerformanceOptimizerComponent::UPerf_PerformanceOptimizerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Every frame — we need accurate timing

    // Pre-allocate frame history
    FrameTimeHistory.SetNumZeroed(FrameHistorySize);
    FrameHistoryIndex = 0;
}

void UPerf_PerformanceOptimizerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set initial budget based on target FPS
    CurrentBudget.TargetFrameTimeMs = 1000.0f / FMath::Max(TargetFPS, 1.0f);
    CurrentScreenPercentage = MaxScreenPercentage;

    // Apply initial quality tier
    ApplyQualityTier(CurrentTier);

    UE_LOG(LogTemp, Log, TEXT("[PerfOptimizer] Initialized. Target: %.1ffps (%.2fms budget). Tier: High"),
        TargetFPS, CurrentBudget.TargetFrameTimeMs);
}

void UPerf_PerformanceOptimizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateFrameBudget(DeltaTime);

    // Quality evaluation every 0.5 seconds to avoid thrashing
    QualityCheckAccumulator += DeltaTime;
    if (QualityCheckAccumulator >= QualityCheckInterval)
    {
        QualityCheckAccumulator = 0.0f;
        EvaluateQualityScaling();
    }
}

void UPerf_PerformanceOptimizerComponent::UpdateFrameBudget(float DeltaTime)
{
    const float FrameMs = DeltaTime * 1000.0f;

    // Store in rolling history
    FrameTimeHistory[FrameHistoryIndex] = FrameMs;
    FrameHistoryIndex = (FrameHistoryIndex + 1) % FrameHistorySize;

    CurrentBudget.LastFrameTimeMs = FrameMs;
    CurrentBudget.bOverBudget = (FrameMs > CurrentBudget.TargetFrameTimeMs * 1.1f); // 10% headroom

    if (CurrentBudget.bOverBudget)
    {
        CurrentBudget.ConsecutiveOverBudgetFrames++;
        const float OverrunMs = FrameMs - CurrentBudget.TargetFrameTimeMs;
        if (CurrentBudget.ConsecutiveOverBudgetFrames == DownscaleThresholdFrames)
        {
            OnFrameBudgetExceeded(OverrunMs);
        }
        UnderBudgetFrameCount = 0;
    }
    else
    {
        CurrentBudget.ConsecutiveOverBudgetFrames = 0;
        UnderBudgetFrameCount++;
    }
}

void UPerf_PerformanceOptimizerComponent::EvaluateQualityScaling()
{
    const float AvgMs = GetAverageFrameTimeMs();
    const float BudgetMs = CurrentBudget.TargetFrameTimeMs;

    // --- Downscale logic ---
    if (CurrentBudget.ConsecutiveOverBudgetFrames >= DownscaleThresholdFrames)
    {
        const EPerf_QualityTier OldTier = CurrentTier;
        EPerf_QualityTier NewTier = CurrentTier;

        if (AvgMs > BudgetMs * 2.0f && CurrentTier != EPerf_QualityTier::Minimal)
        {
            NewTier = EPerf_QualityTier::Minimal;
        }
        else if (AvgMs > BudgetMs * 1.5f && CurrentTier > EPerf_QualityTier::Low)
        {
            NewTier = static_cast<EPerf_QualityTier>(static_cast<uint8>(CurrentTier) + 1);
        }
        else if (AvgMs > BudgetMs * 1.1f && CurrentTier > EPerf_QualityTier::Medium)
        {
            NewTier = static_cast<EPerf_QualityTier>(static_cast<uint8>(CurrentTier) + 1);
        }

        if (NewTier != OldTier)
        {
            SetQualityTier(NewTier);
        }
    }
    // --- Upscale logic (only after sustained good performance) ---
    else if (UnderBudgetFrameCount >= UpscaleThresholdFrames && AvgMs < BudgetMs * 0.8f)
    {
        if (CurrentTier > EPerf_QualityTier::Ultra)
        {
            const EPerf_QualityTier OldTier = CurrentTier;
            const EPerf_QualityTier NewTier = static_cast<EPerf_QualityTier>(static_cast<uint8>(CurrentTier) - 1);
            SetQualityTier(NewTier);
        }
        UnderBudgetFrameCount = 0;
    }
}

void UPerf_PerformanceOptimizerComponent::SetQualityTier(EPerf_QualityTier NewTier)
{
    if (NewTier == CurrentTier) return;

    const EPerf_QualityTier OldTier = CurrentTier;
    CurrentTier = NewTier;

    ApplyQualityTier(NewTier);
    OnQualityTierChanged(NewTier, OldTier);

    UE_LOG(LogTemp, Warning, TEXT("[PerfOptimizer] Quality tier changed: %d -> %d (avg: %.2fms, budget: %.2fms)"),
        static_cast<int32>(OldTier), static_cast<int32>(NewTier),
        GetAverageFrameTimeMs(), CurrentBudget.TargetFrameTimeMs);
}

void UPerf_PerformanceOptimizerComponent::ApplyQualityTier(EPerf_QualityTier Tier)
{
    switch (Tier)
    {
    case EPerf_QualityTier::Ultra:
        ApplyScreenPercentage(100.0f);
        ExecConsoleCommand(TEXT("r.Shadow.MaxResolution 2048"));
        ExecConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.01"));
        ExecConsoleCommand(TEXT("foliage.LODDistanceScale 1.5"));
        ExecConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.5"));
        ExecConsoleCommand(TEXT("r.MaxAnisotropy 16"));
        break;

    case EPerf_QualityTier::High:
        ApplyScreenPercentage(100.0f);
        ExecConsoleCommand(TEXT("r.Shadow.MaxResolution 1024"));
        ExecConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.02"));
        ExecConsoleCommand(TEXT("foliage.LODDistanceScale 1.0"));
        ExecConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 1.0"));
        ExecConsoleCommand(TEXT("r.MaxAnisotropy 8"));
        break;

    case EPerf_QualityTier::Medium:
        ApplyScreenPercentage(85.0f);
        ExecConsoleCommand(TEXT("r.Shadow.MaxResolution 512"));
        ExecConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.04"));
        ExecConsoleCommand(TEXT("foliage.LODDistanceScale 0.75"));
        ExecConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 0.75"));
        ExecConsoleCommand(TEXT("r.MaxAnisotropy 4"));
        break;

    case EPerf_QualityTier::Low:
        ApplyScreenPercentage(70.0f);
        ExecConsoleCommand(TEXT("r.Shadow.MaxResolution 256"));
        ExecConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.08"));
        ExecConsoleCommand(TEXT("foliage.LODDistanceScale 0.5"));
        ExecConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 0.5"));
        ExecConsoleCommand(TEXT("r.MaxAnisotropy 2"));
        ExecConsoleCommand(TEXT("r.Lumen.Reflections.Allow 0"));
        break;

    case EPerf_QualityTier::Minimal:
        ApplyScreenPercentage(60.0f);
        ExecConsoleCommand(TEXT("r.Shadow.MaxResolution 128"));
        ExecConsoleCommand(TEXT("r.Shadow.RadiusThreshold 0.15"));
        ExecConsoleCommand(TEXT("foliage.LODDistanceScale 0.3"));
        ExecConsoleCommand(TEXT("r.StaticMeshLODDistanceScale 0.3"));
        ExecConsoleCommand(TEXT("r.MaxAnisotropy 1"));
        ExecConsoleCommand(TEXT("r.Lumen.Reflections.Allow 0"));
        ExecConsoleCommand(TEXT("r.DynamicGlobalIlluminationMethod 0"));
        break;
    }
}

void UPerf_PerformanceOptimizerComponent::ApplyScreenPercentage(float Percentage)
{
    if (!bDynamicResolutionEnabled) return;

    const float ClampedPct = FMath::Clamp(Percentage, MinScreenPercentage, MaxScreenPercentage);
    CurrentScreenPercentage = ClampedPct;
    ExecConsoleCommand(FString::Printf(TEXT("r.ScreenPercentage %.0f"), ClampedPct));
}

float UPerf_PerformanceOptimizerComponent::GetAverageFrameTimeMs() const
{
    if (FrameTimeHistory.Num() == 0) return 0.0f;

    float Sum = 0.0f;
    for (float T : FrameTimeHistory)
    {
        Sum += T;
    }
    return Sum / static_cast<float>(FrameTimeHistory.Num());
}

void UPerf_PerformanceOptimizerComponent::ExecConsoleCommand(const FString& Command) const
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PC->ConsoleCommand(Command);
        }
    }
}

void UPerf_PerformanceOptimizerComponent::LogPerformanceStats() const
{
    const float AvgMs = GetAverageFrameTimeMs();
    const float AvgFPS = AvgMs > 0.0f ? 1000.0f / AvgMs : 0.0f;

    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE OPTIMIZER STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("  Target FPS: %.1f (%.2fms budget)"), TargetFPS, CurrentBudget.TargetFrameTimeMs);
    UE_LOG(LogTemp, Log, TEXT("  Current FPS: %.1f (%.2fms avg)"), AvgFPS, AvgMs);
    UE_LOG(LogTemp, Log, TEXT("  Quality Tier: %d"), static_cast<int32>(CurrentTier));
    UE_LOG(LogTemp, Log, TEXT("  Screen Percentage: %.0f%%"), CurrentScreenPercentage);
    UE_LOG(LogTemp, Log, TEXT("  Over Budget: %s (%d consecutive frames)"),
        CurrentBudget.bOverBudget ? TEXT("YES") : TEXT("NO"),
        CurrentBudget.ConsecutiveOverBudgetFrames);
    UE_LOG(LogTemp, Log, TEXT("==================================="));

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
            FString::Printf(TEXT("[PerfOpt] FPS: %.1f | Tier: %d | Screen: %.0f%%"),
                AvgFPS, static_cast<int32>(CurrentTier), CurrentScreenPercentage));
    }
}

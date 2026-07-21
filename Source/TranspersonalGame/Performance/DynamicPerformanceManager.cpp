// DynamicPerformanceManager.cpp
// Performance Optimizer — Agent #04
// PROD_CYCLE_AUTO_20260620_005
// Dynamic runtime performance scaling for 60fps PC / 30fps console targets.

#include "DynamicPerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UDynamicPerformanceManager::UDynamicPerformanceManager()
{
    TargetFPS_PC = 60.0f;
    TargetFPS_Console = 30.0f;
    CurrentFPS = 60.0f;
    bScalingActive = false;
    ScaleCheckInterval = 2.0f;
    CurrentQualityTier = EPerf_QualityTier::High;
    FrameTimeAccumulator = 0.0f;
    FrameSampleCount = 0;
}

void UDynamicPerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bScalingActive = true;
    GetWorld()->GetTimerManager().SetTimer(
        ScaleCheckHandle,
        this,
        &UDynamicPerformanceManager::EvaluatePerformance,
        ScaleCheckInterval,
        true
    );
    UE_LOG(LogTemp, Log, TEXT("[PerfMgr] Initialized — target PC=%.0ffps, Console=%.0ffps"), TargetFPS_PC, TargetFPS_Console);
}

void UDynamicPerformanceManager::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(ScaleCheckHandle);
    }
    Super::Deinitialize();
}

float UDynamicPerformanceManager::GetTargetFPS() const
{
#if PLATFORM_DESKTOP
    return TargetFPS_PC;
#else
    return TargetFPS_Console;
#endif
}

void UDynamicPerformanceManager::RecordFrameTime(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f) return;
    FrameTimeAccumulator += DeltaSeconds;
    FrameSampleCount++;
}

void UDynamicPerformanceManager::EvaluatePerformance()
{
    if (FrameSampleCount == 0) return;

    const float AvgFrameTime = FrameTimeAccumulator / static_cast<float>(FrameSampleCount);
    CurrentFPS = (AvgFrameTime > 0.0f) ? (1.0f / AvgFrameTime) : 60.0f;

    FrameTimeAccumulator = 0.0f;
    FrameSampleCount = 0;

    const float Target = GetTargetFPS();
    const float Ratio = CurrentFPS / Target;

    EPerf_QualityTier NewTier = CurrentQualityTier;

    if (Ratio < 0.75f)
    {
        // Severely below target — drop two tiers
        NewTier = static_cast<EPerf_QualityTier>(FMath::Max(0, static_cast<int32>(CurrentQualityTier) - 2));
    }
    else if (Ratio < 0.90f)
    {
        // Below target — drop one tier
        NewTier = static_cast<EPerf_QualityTier>(FMath::Max(0, static_cast<int32>(CurrentQualityTier) - 1));
    }
    else if (Ratio > 1.15f && CurrentFPS > Target + 10.0f)
    {
        // Well above target — raise one tier (headroom available)
        NewTier = static_cast<EPerf_QualityTier>(FMath::Min(3, static_cast<int32>(CurrentQualityTier) + 1));
    }

    if (NewTier != CurrentQualityTier)
    {
        ApplyQualityTier(NewTier);
    }

    UE_LOG(LogTemp, Verbose, TEXT("[PerfMgr] FPS=%.1f Target=%.0f Tier=%d"), CurrentFPS, Target, static_cast<int32>(CurrentQualityTier));
}

void UDynamicPerformanceManager::ApplyQualityTier(EPerf_QualityTier NewTier)
{
    CurrentQualityTier = NewTier;
    IConsoleManager& CM = IConsoleManager::Get();

    switch (NewTier)
    {
    case EPerf_QualityTier::Ultra:
        ApplyCVar(CM, TEXT("r.Shadow.MaxResolution"), TEXT("4096"));
        ApplyCVar(CM, TEXT("r.Shadow.CSM.MaxCascades"), TEXT("4"));
        ApplyCVar(CM, TEXT("r.Lumen.MaxTraceDistance"), TEXT("12000"));
        ApplyCVar(CM, TEXT("r.StaticMeshLODDistanceScale"), TEXT("1.5"));
        ApplyCVar(CM, TEXT("fx.Niagara.MaxGPUParticlesSpawnPerFrame"), TEXT("8192"));
        break;

    case EPerf_QualityTier::High:
        ApplyCVar(CM, TEXT("r.Shadow.MaxResolution"), TEXT("2048"));
        ApplyCVar(CM, TEXT("r.Shadow.CSM.MaxCascades"), TEXT("3"));
        ApplyCVar(CM, TEXT("r.Lumen.MaxTraceDistance"), TEXT("8000"));
        ApplyCVar(CM, TEXT("r.StaticMeshLODDistanceScale"), TEXT("1.0"));
        ApplyCVar(CM, TEXT("fx.Niagara.MaxGPUParticlesSpawnPerFrame"), TEXT("4096"));
        break;

    case EPerf_QualityTier::Medium:
        ApplyCVar(CM, TEXT("r.Shadow.MaxResolution"), TEXT("1024"));
        ApplyCVar(CM, TEXT("r.Shadow.CSM.MaxCascades"), TEXT("2"));
        ApplyCVar(CM, TEXT("r.Lumen.MaxTraceDistance"), TEXT("4000"));
        ApplyCVar(CM, TEXT("r.StaticMeshLODDistanceScale"), TEXT("0.75"));
        ApplyCVar(CM, TEXT("fx.Niagara.MaxGPUParticlesSpawnPerFrame"), TEXT("2048"));
        break;

    case EPerf_QualityTier::Low:
        ApplyCVar(CM, TEXT("r.Shadow.MaxResolution"), TEXT("512"));
        ApplyCVar(CM, TEXT("r.Shadow.CSM.MaxCascades"), TEXT("1"));
        ApplyCVar(CM, TEXT("r.Lumen.MaxTraceDistance"), TEXT("2000"));
        ApplyCVar(CM, TEXT("r.StaticMeshLODDistanceScale"), TEXT("0.5"));
        ApplyCVar(CM, TEXT("fx.Niagara.MaxGPUParticlesSpawnPerFrame"), TEXT("512"));
        break;
    }

    OnQualityTierChanged.Broadcast(NewTier);
    UE_LOG(LogTemp, Log, TEXT("[PerfMgr] Quality tier changed to %d (FPS=%.1f)"), static_cast<int32>(NewTier), CurrentFPS);
}

void UDynamicPerformanceManager::ApplyCVar(IConsoleManager& CM, const TCHAR* Name, const TCHAR* Value)
{
    IConsoleVariable* CVar = CM.FindConsoleVariable(Name);
    if (CVar)
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

void UDynamicPerformanceManager::ForceQualityTier(EPerf_QualityTier Tier)
{
    ApplyQualityTier(Tier);
}

float UDynamicPerformanceManager::GetCurrentFPS() const
{
    return CurrentFPS;
}

EPerf_QualityTier UDynamicPerformanceManager::GetCurrentQualityTier() const
{
    return CurrentQualityTier;
}

FPerf_FrameBudget UDynamicPerformanceManager::GetFrameBudget() const
{
    FPerf_FrameBudget Budget;
    const float TotalMs = 1000.0f / GetTargetFPS();
    Budget.TotalBudgetMs = TotalMs;
    Budget.RenderThreadMs = TotalMs * 0.55f;   // 55% render
    Budget.GameThreadMs   = TotalMs * 0.30f;   // 30% game logic
    Budget.GPUMs          = TotalMs * 0.65f;   // 65% GPU (overlaps with render)
    Budget.AIBudgetMs     = TotalMs * 0.10f;   // 10% AI ticks
    Budget.PhysicsBudgetMs = TotalMs * 0.08f;  // 8% physics
    return Budget;
}

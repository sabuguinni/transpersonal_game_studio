// PerformanceBudgetManager.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260620_003

#include "PerformanceBudgetManager.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"

APerformanceBudgetManager::APerformanceBudgetManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Check every 100ms — not every frame

    TotalFrameBudgetMs = 1000.0f / TargetFPS;
    FPSHistory.Reserve(FPS_HISTORY_SIZE);

    InitializeDefaultBudgets();
    InitializeQualityPresets();
}

void APerformanceBudgetManager::BeginPlay()
{
    Super::BeginPlay();
    ApplyQualityTier(CurrentQualityTier);
    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Initialized — Target: %.0ffps (%.2fms budget), Tier: %d"),
        TargetFPS, TotalFrameBudgetMs, (int32)CurrentQualityTier);
}

void APerformanceBudgetManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFPSSmoothing(DeltaTime);

    if (bEnableDynamicScaling)
    {
        TimeSinceLastScaleUp += DeltaTime;
        CheckDynamicScaling();
    }
}

// ─── Initialization ──────────────────────────────────────────────────────────

void APerformanceBudgetManager::InitializeDefaultBudgets()
{
    // 16.67ms total at 60fps — allocate per system
    // Rendering gets the lion's share; AI and physics are secondary
    auto AddBudget = [&](EPerf_BudgetSystem Sys, float Ms)
    {
        FPerf_SystemBudget B;
        B.System = Sys;
        B.BudgetMs = Ms;
        SystemBudgets.Add(B);
    };

    AddBudget(EPerf_BudgetSystem::Rendering,  8.0f);  // GPU-bound — 48% of frame
    AddBudget(EPerf_BudgetSystem::Physics,    2.5f);  // Collision, ragdoll
    AddBudget(EPerf_BudgetSystem::AI,         2.0f);  // BT ticks, perception
    AddBudget(EPerf_BudgetSystem::Animation,  1.5f);  // Anim graph, IK
    AddBudget(EPerf_BudgetSystem::Audio,      1.0f);  // MetaSounds
    AddBudget(EPerf_BudgetSystem::Streaming,  1.0f);  // World partition I/O
    // ~0.67ms headroom for GC, misc
}

void APerformanceBudgetManager::InitializeQualityPresets()
{
    auto AddPreset = [&](EPerf_QualityTier Tier, float ShadowDist, int32 ShadowRes,
        float ScreenPct, bool LumenGI, bool LumenRefl, int32 MaxDinoLOD, int32 MaxDinos)
    {
        FPerf_QualitySettings S;
        S.Tier = Tier;
        S.ShadowDistanceScale = ShadowDist;
        S.MaxShadowResolution = ShadowRes;
        S.ScreenPercentage = ScreenPct;
        S.bLumenGI = LumenGI;
        S.bLumenReflections = LumenRefl;
        S.MaxDinoLOD = MaxDinoLOD;
        S.MaxActiveDinosaurs = MaxDinos;
        QualityPresets.Add(S);
    };

    //                    Tier                ShadDist ShadRes  Screen  LumenGI LumenR  LOD  Dinos
    AddPreset(EPerf_QualityTier::Ultra,       1.2f,    4096,    100.0f, true,   true,   0,   30);
    AddPreset(EPerf_QualityTier::High,        1.0f,    2048,    100.0f, true,   true,   0,   20);
    AddPreset(EPerf_QualityTier::Medium,      0.8f,    1024,    85.0f,  true,   false,  1,   15);
    AddPreset(EPerf_QualityTier::Low,         0.6f,    512,     75.0f,  false,  false,  2,   10);
    AddPreset(EPerf_QualityTier::Potato,      0.4f,    256,     60.0f,  false,  false,  3,   5);
}

// ─── Public API ──────────────────────────────────────────────────────────────

void APerformanceBudgetManager::ApplyQualityTier(EPerf_QualityTier NewTier)
{
    FPerf_QualitySettings Settings = GetQualitySettings(NewTier);
    CurrentQualityTier = NewTier;
    ApplyCVarsForTier(Settings);

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Quality tier changed to %d — Screen:%.0f%% ShadowRes:%d LumenGI:%d MaxDinos:%d"),
        (int32)NewTier, Settings.ScreenPercentage, Settings.MaxShadowResolution,
        Settings.bLumenGI ? 1 : 0, Settings.MaxActiveDinosaurs);
}

FPerf_QualitySettings APerformanceBudgetManager::GetQualitySettings(EPerf_QualityTier Tier) const
{
    for (const FPerf_QualitySettings& S : QualityPresets)
    {
        if (S.Tier == Tier)
        {
            return S;
        }
    }
    // Fallback: return High preset defaults
    FPerf_QualitySettings Default;
    Default.Tier = EPerf_QualityTier::High;
    return Default;
}

void APerformanceBudgetManager::RunPerformanceAudit()
{
    UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] === PERFORMANCE AUDIT ==="));
    UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] SmoothedFPS: %.1f | FrameMs: %.2f | Budget: %.2fms"),
        SmoothedFPS, CurrentFrameMs, TotalFrameBudgetMs);
    UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] Quality Tier: %d | OverBudgetFrames: %d"),
        (int32)CurrentQualityTier, OverBudgetFrameCount);

    for (const FPerf_SystemBudget& B : SystemBudgets)
    {
        float Ratio = B.BudgetMs > 0.0f ? B.LastMeasuredMs / B.BudgetMs : 0.0f;
        UE_LOG(LogTemp, Warning, TEXT("[PerfBudget]   System %d: %.2fms / %.2fms (%.0f%%) %s"),
            (int32)B.System, B.LastMeasuredMs, B.BudgetMs, Ratio * 100.0f,
            B.bOverBudget ? TEXT("OVER_BUDGET") : TEXT("OK"));
    }
    UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] === END AUDIT ==="));
}

float APerformanceBudgetManager::GetBudgetUsageRatio(EPerf_BudgetSystem System) const
{
    for (const FPerf_SystemBudget& B : SystemBudgets)
    {
        if (B.System == System)
        {
            return B.BudgetMs > 0.0f ? B.LastMeasuredMs / B.BudgetMs : 0.0f;
        }
    }
    return 0.0f;
}

void APerformanceBudgetManager::ReportSystemTime(EPerf_BudgetSystem System, float MeasuredMs)
{
    for (FPerf_SystemBudget& B : SystemBudgets)
    {
        if (B.System == System)
        {
            B.LastMeasuredMs = MeasuredMs;
            B.bOverBudget = MeasuredMs > B.BudgetMs;
            if (B.bOverBudget)
            {
                UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] System %d over budget: %.2fms > %.2fms"),
                    (int32)System, MeasuredMs, B.BudgetMs);
            }
            return;
        }
    }
}

// ─── Private ─────────────────────────────────────────────────────────────────

void APerformanceBudgetManager::UpdateFPSSmoothing(float DeltaTime)
{
    if (DeltaTime <= 0.0f) return;

    CurrentFrameMs = DeltaTime * 1000.0f;
    float InstantFPS = 1.0f / DeltaTime;

    FPSHistory.Add(InstantFPS);
    if (FPSHistory.Num() > FPS_HISTORY_SIZE)
    {
        FPSHistory.RemoveAt(0);
    }

    float Sum = 0.0f;
    for (float F : FPSHistory) Sum += F;
    SmoothedFPS = FPSHistory.Num() > 0 ? Sum / FPSHistory.Num() : InstantFPS;

    if (CurrentFrameMs > TotalFrameBudgetMs)
    {
        OverBudgetFrameCount++;
    }
    else
    {
        OverBudgetFrameCount = FMath::Max(0, OverBudgetFrameCount - 1);
    }
}

void APerformanceBudgetManager::CheckDynamicScaling()
{
    // Scale down: 10 consecutive over-budget frames at current tier
    if (OverBudgetFrameCount >= 10 && SmoothedFPS < ScaleDownThresholdFPS)
    {
        int32 TierInt = (int32)CurrentQualityTier;
        if (TierInt < (int32)EPerf_QualityTier::Potato)
        {
            EPerf_QualityTier NewTier = (EPerf_QualityTier)(TierInt + 1);
            UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] DynamicScale DOWN: %.1ffps < %.1f threshold"),
                SmoothedFPS, ScaleDownThresholdFPS);
            ApplyQualityTier(NewTier);
            OverBudgetFrameCount = 0;
            TimeSinceLastScaleUp = 0.0f;
        }
    }
    // Scale up: sustained high FPS with hysteresis
    else if (SmoothedFPS > ScaleUpThresholdFPS && TimeSinceLastScaleUp > ScaleUpHysteresisSeconds)
    {
        int32 TierInt = (int32)CurrentQualityTier;
        if (TierInt > (int32)EPerf_QualityTier::Ultra)
        {
            EPerf_QualityTier NewTier = (EPerf_QualityTier)(TierInt - 1);
            UE_LOG(LogTemp, Log, TEXT("[PerfBudget] DynamicScale UP: %.1ffps > %.1f threshold (hysteresis %.1fs)"),
                SmoothedFPS, ScaleUpThresholdFPS, TimeSinceLastScaleUp);
            ApplyQualityTier(NewTier);
            TimeSinceLastScaleUp = 0.0f;
        }
    }
}

void APerformanceBudgetManager::ApplyCVarsForTier(const FPerf_QualitySettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World) return;

    auto Exec = [&](const FString& Cmd)
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, Cmd);
    };

    Exec(FString::Printf(TEXT("r.Shadow.MaxResolution %d"), Settings.MaxShadowResolution));
    Exec(FString::Printf(TEXT("r.Shadow.DistanceScale %.2f"), Settings.ShadowDistanceScale));
    Exec(FString::Printf(TEXT("r.ScreenPercentage %.0f"), Settings.ScreenPercentage));
    Exec(FString::Printf(TEXT("r.Lumen.GlobalIllumination.Allow %d"), Settings.bLumenGI ? 1 : 0));
    Exec(FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"), Settings.bLumenReflections ? 1 : 0));
    Exec(FString::Printf(TEXT("r.SkeletalMeshLODBias %d"), Settings.MaxDinoLOD));
}

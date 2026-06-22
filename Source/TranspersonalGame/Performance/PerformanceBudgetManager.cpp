// PerformanceBudgetManager.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260622_011
// Full implementation of runtime performance budget manager

#include "PerformanceBudgetManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

APerf_BudgetManager::APerf_BudgetManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Every frame for accurate FPS sampling

    InitDefaultLODConfigs();
}

void APerf_BudgetManager::InitDefaultLODConfigs()
{
    // Low — Console 30fps target
    LODConfig_Low.StaticMeshLODScale    = 2.0f;
    LODConfig_Low.SkeletalMeshLODScale  = 2.0f;
    LODConfig_Low.FoliageLODScale       = 2.0f;
    LODConfig_Low.FoliageDensityScale   = 0.3f;
    LODConfig_Low.MaxShadowResolution   = 512;
    LODConfig_Low.ShadowDistanceScale   = 0.5f;

    // Medium — PC 30fps
    LODConfig_Medium.StaticMeshLODScale    = 1.5f;
    LODConfig_Medium.SkeletalMeshLODScale  = 1.5f;
    LODConfig_Medium.FoliageLODScale       = 1.5f;
    LODConfig_Medium.FoliageDensityScale   = 0.6f;
    LODConfig_Medium.MaxShadowResolution   = 1024;
    LODConfig_Medium.ShadowDistanceScale   = 0.75f;

    // High — PC 60fps (default)
    LODConfig_High.StaticMeshLODScale    = 1.0f;
    LODConfig_High.SkeletalMeshLODScale  = 1.0f;
    LODConfig_High.FoliageLODScale       = 1.0f;
    LODConfig_High.FoliageDensityScale   = 1.0f;
    LODConfig_High.MaxShadowResolution   = 2048;
    LODConfig_High.ShadowDistanceScale   = 1.0f;

    // Epic — PC 60fps Ultra
    LODConfig_Epic.StaticMeshLODScale    = 0.75f;
    LODConfig_Epic.SkeletalMeshLODScale  = 0.75f;
    LODConfig_Epic.FoliageLODScale       = 0.75f;
    LODConfig_Epic.FoliageDensityScale   = 1.0f;
    LODConfig_Epic.MaxShadowResolution   = 4096;
    LODConfig_Epic.ShadowDistanceScale   = 1.5f;
}

void APerf_BudgetManager::BeginPlay()
{
    Super::BeginPlay();

    FPSSamples.Reserve(FPS_SAMPLE_COUNT);

    // Apply initial tier
    ApplyTierConfig(CurrentTier);

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Initialized — Target: %.0ffps (%.2fms), Tier: %d"),
        TargetFPS, TargetFrameTimeMS, (int32)CurrentTier);
}

void APerf_BudgetManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFPSSamples(DeltaTime);

    if (bEnableDynamicScalability)
    {
        ScalabilityCheckTimer += DeltaTime;
        if (ScalabilityCheckTimer >= ScalabilityCheckIntervalSeconds)
        {
            ScalabilityCheckTimer = 0.0f;
            CheckBudgetAndAdjust();
        }
    }
}

void APerf_BudgetManager::UpdateFPSSamples(float DeltaTime)
{
    if (DeltaTime <= 0.0f) return;

    float CurrentFPS = 1.0f / DeltaTime;

    if (FPSSamples.Num() >= FPS_SAMPLE_COUNT)
    {
        FPSSamples.RemoveAt(0);
    }
    FPSSamples.Add(CurrentFPS);

    // Rolling average
    if (FPSSamples.Num() > 0)
    {
        float Sum = 0.0f;
        for (float S : FPSSamples) Sum += S;
        AverageFPS = Sum / FPSSamples.Num();
    }

    // Update budget snapshot
    float FrameMS = DeltaTime * 1000.0f;
    CurrentBudget.FrameTimeMS = FrameMS;
    CurrentBudget.bBudgetExceeded = (FrameMS > TargetFrameTimeMS * 1.1f); // 10% tolerance

    if (CurrentBudget.bBudgetExceeded)
    {
        BudgetViolationCount++;
        bIsUnderBudget = false;
    }
    else
    {
        bIsUnderBudget = true;
    }
}

void APerf_BudgetManager::CheckBudgetAndAdjust()
{
    if (AverageFPS <= 0.0f) return;

    float TargetWithMargin = TargetFPS * 0.9f; // 10% margin before downgrade

    if (AverageFPS < TargetWithMargin)
    {
        // Under budget — downgrade tier
        int32 TierInt = (int32)CurrentTier;
        if (TierInt > 0)
        {
            EPerf_ScalabilityTier NewTier = (EPerf_ScalabilityTier)(TierInt - 1);
            UE_LOG(LogTemp, Warning, TEXT("[PerfBudget] FPS %.1f < target %.1f — downgrading tier %d -> %d"),
                AverageFPS, TargetWithMargin, TierInt, TierInt - 1);
            SetScalabilityTier(NewTier);
        }
    }
    else if (AverageFPS > TargetFPS * 1.2f) // 20% headroom before upgrade
    {
        // Well above budget — upgrade tier if possible
        int32 TierInt = (int32)CurrentTier;
        if (TierInt < (int32)EPerf_ScalabilityTier::Epic)
        {
            EPerf_ScalabilityTier NewTier = (EPerf_ScalabilityTier)(TierInt + 1);
            UE_LOG(LogTemp, Log, TEXT("[PerfBudget] FPS %.1f > headroom %.1f — upgrading tier %d -> %d"),
                AverageFPS, TargetFPS * 1.2f, TierInt, TierInt + 1);
            SetScalabilityTier(NewTier);
        }
    }
}

void APerf_BudgetManager::SetScalabilityTier(EPerf_ScalabilityTier NewTier)
{
    CurrentTier = NewTier;
    ApplyTierConfig(NewTier);
}

void APerf_BudgetManager::ApplyTierConfig(EPerf_ScalabilityTier Tier)
{
    FPerf_LODConfig Config = GetLODConfigForTier(Tier);
    ApplyLODConfig(Config);

    // Apply scalability group settings
    int32 QualityLevel = (int32)Tier;
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.ViewDistanceQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.ShadowQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.TextureQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.EffectsQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.FoliageQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.PostProcessQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.GlobalIlluminationQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.ReflectionQuality %d"), QualityLevel));
    ExecuteConsoleCommand(FString::Printf(TEXT("sg.ShadingQuality %d"), QualityLevel));
}

void APerf_BudgetManager::ApplyLODConfig(const FPerf_LODConfig& Config)
{
    ExecuteConsoleCommand(FString::Printf(TEXT("r.StaticMeshLODDistanceScale %.2f"), Config.StaticMeshLODScale));
    ExecuteConsoleCommand(FString::Printf(TEXT("foliage.LODDistanceScale %.2f"), Config.FoliageLODScale));
    ExecuteConsoleCommand(FString::Printf(TEXT("foliage.DensityScale %.2f"), Config.FoliageDensityScale));
    ExecuteConsoleCommand(FString::Printf(TEXT("grass.DensityScale %.2f"), Config.FoliageDensityScale));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Shadow.MaxCSMResolution %d"), Config.MaxShadowResolution));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.Shadow.DistanceScale %.2f"), Config.ShadowDistanceScale));
}

void APerf_BudgetManager::ExecuteConsoleCommand(const FString& Command)
{
    if (UWorld* World = GetWorld())
    {
        UKismetSystemLibrary::ExecuteConsoleCommand(World, Command, nullptr);
    }
}

FPerf_LODConfig APerf_BudgetManager::GetLODConfigForTier(EPerf_ScalabilityTier Tier) const
{
    switch (Tier)
    {
        case EPerf_ScalabilityTier::Low:    return LODConfig_Low;
        case EPerf_ScalabilityTier::Medium: return LODConfig_Medium;
        case EPerf_ScalabilityTier::High:   return LODConfig_High;
        case EPerf_ScalabilityTier::Epic:   return LODConfig_Epic;
        default:                            return LODConfig_High;
    }
}

FPerf_FrameBudget APerf_BudgetManager::GetCurrentFrameBudget() const
{
    return CurrentBudget;
}

void APerf_BudgetManager::ForceScalabilityCheck()
{
    CheckBudgetAndAdjust();
}

void APerf_BudgetManager::SetTargetFPS(float NewTargetFPS)
{
    TargetFPS = FMath::Clamp(NewTargetFPS, 15.0f, 144.0f);
    TargetFrameTimeMS = 1000.0f / TargetFPS;

    // Update game thread / render thread budgets proportionally
    GameThreadBudgetMS  = TargetFrameTimeMS * 0.36f; // 36% of frame
    RenderThreadBudgetMS = TargetFrameTimeMS * 0.36f; // 36% of frame
    GPUBudgetMS          = TargetFrameTimeMS * 0.60f; // 60% of frame

    ExecuteConsoleCommand(FString::Printf(TEXT("t.MaxFPS %.0f"), TargetFPS));

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Target FPS set to %.0f (%.2fms frame budget)"),
        TargetFPS, TargetFrameTimeMS);
}

#if WITH_EDITOR
void APerf_BudgetManager::PrintBudgetReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE BUDGET REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Target FPS: %.0f | Average FPS: %.1f | Under Budget: %s"),
        TargetFPS, AverageFPS, bIsUnderBudget ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Current Tier: %d | Budget Violations: %d"),
        (int32)CurrentTier, BudgetViolationCount);
    UE_LOG(LogTemp, Log, TEXT("Last Frame: %.2fms (budget: %.2fms)"),
        CurrentBudget.FrameTimeMS, TargetFrameTimeMS);
    UE_LOG(LogTemp, Log, TEXT("================================="));
}
#endif

// PerformanceOptimizerComponent.cpp
// Agent #4 — Performance Optimizer
// Transpersonal Game Studio
// Target: 60fps PC High-End / 30fps Console
// Strategy: Adaptive quality tiers, LOD enforcement, shadow budget management

#include "PerformanceOptimizerComponent.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

// ============================================================
// Constructor
// ============================================================

UPerf_PerformanceOptimizerComponent::UPerf_PerformanceOptimizerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms — not every frame

    // Default frame budget: 60fps PC target
    TargetFrameBudget.TargetFPS = 60.0f;
    TargetFrameBudget.FrameBudgetMS = 1000.0f / 60.0f; // 16.67ms

    // Pre-size FPS sample buffer
    FPSSamples.Reserve(120);
}

// ============================================================
// BeginPlay
// ============================================================

void UPerf_PerformanceOptimizerComponent::BeginPlay()
{
    Super::BeginPlay();

    // Apply initial quality tier
    ApplyQualityTier(InitialQualityTier);

    // Apply shadow and LOD CVars
    ApplyShadowCVars();
    ApplyLODCVars();

    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Initialized. Tier=%d, TargetFPS=%.0f, Budget=%.2fms"),
        (int32)InitialQualityTier, TargetFrameBudget.TargetFPS, TargetFrameBudget.FrameBudgetMS);
}

// ============================================================
// TickComponent
// ============================================================

void UPerf_PerformanceOptimizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateFPSAverage(DeltaTime);

    if (bAdaptiveQuality)
    {
        CheckAdaptiveQuality(DeltaTime);
    }
}

// ============================================================
// UpdateFPSAverage — rolling window average
// ============================================================

void UPerf_PerformanceOptimizerComponent::UpdateFPSAverage(float DeltaTime)
{
    if (DeltaTime <= 0.0f) return;

    float InstantFPS = 1.0f / DeltaTime;
    FPSSamples.Add(InstantFPS);

    // Keep rolling window
    while (FPSSamples.Num() > FPSAveragingWindow)
    {
        FPSSamples.RemoveAt(0);
    }

    // Compute average
    float Sum = 0.0f;
    for (float Sample : FPSSamples)
    {
        Sum += Sample;
    }

    CurrentFrameBudget.CurrentFPS = (FPSSamples.Num() > 0) ? (Sum / FPSSamples.Num()) : 0.0f;
    CurrentFrameBudget.CurrentFrameMS = (CurrentFrameBudget.CurrentFPS > 0.0f) ? (1000.0f / CurrentFrameBudget.CurrentFPS) : 0.0f;
    CurrentFrameBudget.bMeetingBudget = (CurrentFrameBudget.CurrentFPS >= TargetFrameBudget.TargetFPS * 0.9f); // 10% tolerance
}

// ============================================================
// CheckAdaptiveQuality — downgrade if FPS is consistently low
// ============================================================

void UPerf_PerformanceOptimizerComponent::CheckAdaptiveQuality(float DeltaTime)
{
    if (FPSSamples.Num() < FPSAveragingWindow / 2)
    {
        return; // Not enough samples yet
    }

    bool bLowFPS = !CurrentFrameBudget.bMeetingBudget;

    if (bLowFPS)
    {
        AccumulatedLowFPSTime += DeltaTime;

        if (AccumulatedLowFPSTime >= AdaptiveQualityGracePeriod)
        {
            // Downgrade quality tier
            int32 CurrentTierInt = (int32)CurrentQualityTier;
            if (CurrentTierInt < (int32)EPerf_QualityTier::Low)
            {
                EPerf_QualityTier NewTier = (EPerf_QualityTier)(CurrentTierInt + 1);
                UE_LOG(LogTemp, Warning, TEXT("[PerformanceOptimizer] FPS=%.1f below target %.1f — downgrading quality tier %d -> %d"),
                    CurrentFrameBudget.CurrentFPS, TargetFrameBudget.TargetFPS, CurrentTierInt, (int32)NewTier);
                ApplyQualityTier(NewTier);
            }
            AccumulatedLowFPSTime = 0.0f;
        }
    }
    else
    {
        // FPS is good — reset accumulator
        AccumulatedLowFPSTime = FMath::Max(0.0f, AccumulatedLowFPSTime - DeltaTime * 2.0f);
    }
}

// ============================================================
// ApplyQualityTier — apply all CVars for a given tier
// ============================================================

void UPerf_PerformanceOptimizerComponent::ApplyQualityTier(EPerf_QualityTier Tier)
{
    CurrentQualityTier = Tier;
    ApplyConsoleCVars(Tier);

    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Quality tier applied: %d"), (int32)Tier);
}

void UPerf_PerformanceOptimizerComponent::ApplyConsoleCVars(EPerf_QualityTier Tier)
{
    UWorld* World = GetWorld();
    if (!World) return;

    switch (Tier)
    {
    case EPerf_QualityTier::Ultra:
        SetCVar(TEXT("r.ScreenPercentage"), 100);
        SetCVar(TEXT("r.Shadow.MaxResolution"), 4096);
        SetCVar(TEXT("r.Shadow.CSM.MaxCascades"), 4);
        SetCVar(TEXT("r.Lumen.Reflections.Allow"), 1);
        SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 0.5f);
        SetCVar(TEXT("foliage.LODDistanceScale"), 2.0f);
        SetCVar(TEXT("r.LODDistanceFactor"), 1.5f);
        SetCVar(TEXT("r.Streaming.PoolSize"), 4096);
        break;

    case EPerf_QualityTier::High:
        SetCVar(TEXT("r.ScreenPercentage"), 100);
        SetCVar(TEXT("r.Shadow.MaxResolution"), 2048);
        SetCVar(TEXT("r.Shadow.CSM.MaxCascades"), 3);
        SetCVar(TEXT("r.Lumen.Reflections.Allow"), 1);
        SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 1.0f);
        SetCVar(TEXT("foliage.LODDistanceScale"), 1.5f);
        SetCVar(TEXT("r.LODDistanceFactor"), 1.0f);
        SetCVar(TEXT("r.Streaming.PoolSize"), 2048);
        break;

    case EPerf_QualityTier::Medium:
        SetCVar(TEXT("r.ScreenPercentage"), 85);
        SetCVar(TEXT("r.Shadow.MaxResolution"), 1024);
        SetCVar(TEXT("r.Shadow.CSM.MaxCascades"), 2);
        SetCVar(TEXT("r.Lumen.Reflections.Allow"), 0);
        SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 2.0f);
        SetCVar(TEXT("foliage.LODDistanceScale"), 1.0f);
        SetCVar(TEXT("r.LODDistanceFactor"), 0.75f);
        SetCVar(TEXT("r.Streaming.PoolSize"), 1024);
        break;

    case EPerf_QualityTier::Low:
        SetCVar(TEXT("r.ScreenPercentage"), 70);
        SetCVar(TEXT("r.Shadow.MaxResolution"), 512);
        SetCVar(TEXT("r.Shadow.CSM.MaxCascades"), 1);
        SetCVar(TEXT("r.Lumen.Reflections.Allow"), 0);
        SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"), 4.0f);
        SetCVar(TEXT("foliage.LODDistanceScale"), 0.5f);
        SetCVar(TEXT("r.LODDistanceFactor"), 0.5f);
        SetCVar(TEXT("r.Streaming.PoolSize"), 512);
        break;
    }

    // Always-on performance settings regardless of tier
    SetCVar(TEXT("r.HZBOcclusion"), 1);
    SetCVar(TEXT("r.SkyAtmosphere.FastSkyLUT"), 1);
    SetCVar(TEXT("r.DistanceFieldAO"), 1);
    SetCVar(TEXT("r.VirtualTextureMemoryBudget"), 1024);
}

// ============================================================
// ApplyShadowCVars — apply shadow settings from struct
// ============================================================

void UPerf_PerformanceOptimizerComponent::ApplyShadowCVars()
{
    SetCVar(TEXT("r.Shadow.CSM.MaxCascades"), ShadowSettings.MaxCSMCascades);
    SetCVar(TEXT("r.Shadow.MaxResolution"), ShadowSettings.ShadowMapResolution);
}

// ============================================================
// ApplyLODCVars — apply LOD settings from struct
// ============================================================

void UPerf_PerformanceOptimizerComponent::ApplyLODCVars()
{
    SetCVar(TEXT("r.LODDistanceFactor"), LODSettings.LODDistanceScale);
    SetCVar(TEXT("foliage.LODDistanceScale"), LODSettings.FoliageLODScale);
}

// ============================================================
// RefreshLODSettings — update cull distances on all SMCs
// ============================================================

void UPerf_PerformanceOptimizerComponent::RefreshLODSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 Updated = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        TArray<UStaticMeshComponent*> SMCs;
        Actor->GetComponents<UStaticMeshComponent>(SMCs);

        for (UStaticMeshComponent* SMC : SMCs)
        {
            if (!SMC) continue;

            // Determine cull distance based on actor scale
            FVector Scale = Actor->GetActorScale3D();
            float AvgScale = (Scale.X + Scale.Y + Scale.Z) / 3.0f;

            float CullDist;
            if (AvgScale < 1.0f)
            {
                CullDist = LODSettings.SmallPropCullDistance;
            }
            else if (AvgScale < 5.0f)
            {
                CullDist = LODSettings.MediumObjectCullDistance;
            }
            else
            {
                CullDist = LODSettings.LargeObjectCullDistance;
            }

            SMC->SetCullDistance(CullDist);
            Updated++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] RefreshLODSettings: updated %d StaticMeshComponents"), Updated);
}

// ============================================================
// RunAdaptiveQualityCheck — callable from editor
// ============================================================

void UPerf_PerformanceOptimizerComponent::RunAdaptiveQualityCheck()
{
    UE_LOG(LogTemp, Log, TEXT("[PerformanceOptimizer] Manual quality check — FPS=%.1f, Budget=%.2fms, MeetingBudget=%s, Tier=%d"),
        CurrentFrameBudget.CurrentFPS,
        CurrentFrameBudget.CurrentFrameMS,
        CurrentFrameBudget.bMeetingBudget ? TEXT("YES") : TEXT("NO"),
        (int32)CurrentQualityTier);

    // Re-apply current tier CVars
    ApplyConsoleCVars(CurrentQualityTier);
    ApplyShadowCVars();
    ApplyLODCVars();
    RefreshLODSettings();
}

// ============================================================
// SetCVar helpers
// ============================================================

void UPerf_PerformanceOptimizerComponent::SetCVar(const FString& CVarName, float Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

void UPerf_PerformanceOptimizerComponent::SetCVar(const FString& CVarName, int32 Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

// PerformanceBudget.cpp
// Performance Optimizer — Agent #04
// Full implementation of UPerformanceBudgetManager.

#include "PerformanceBudget.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

UPerformanceBudgetManager::UPerformanceBudgetManager()
{
    ActivePreset = EPerf_QualityPreset::PCMedium;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void UPerformanceBudgetManager::ApplyCVarProfile(EPerf_QualityPreset Preset)
{
    ActivePreset = Preset;
    FPerf_CVarProfile Profile = GetCVarProfile(Preset);

    ApplyCVar(TEXT("r.Shadow.MaxResolution"),                  Profile.ShadowMaxResolution);
    ApplyCVar(TEXT("r.Shadow.RadiusThreshold"),                Profile.ShadowRadiusThreshold);
    ApplyCVar(TEXT("r.Shadow.CSM.MaxCascades"),                Profile.ShadowCSMMaxCascades);
    ApplyCVar(TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), Profile.LumenMaxTraceDistance);
    ApplyCVar(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"), Profile.LumenReflectionsMaxRoughness);
    ApplyCVar(TEXT("foliage.LODDistanceScale"),                Profile.FoliageLODDistanceScale);
    ApplyCVar(TEXT("r.StaticMeshLODDistanceScale"),            Profile.StaticMeshLODDistanceScale);
    ApplyCVar(TEXT("r.Streaming.PoolSize"),                    Profile.StreamingPoolSizeMB);
    ApplyCVar(TEXT("r.Nanite.MaxPixelsPerEdge"),               Profile.NaniteMaxPixelsPerEdge);
    ApplyCVar(TEXT("r.TSR.History.ScreenPercentage"),          Profile.TSRHistoryScreenPercentage);

    // Sky atmosphere — always fast
    ApplyCVar(TEXT("r.SkyAtmosphere.FastSkyLUT"), 1);
    ApplyCVar(TEXT("r.SkyAtmosphere.AerialPerspectiveLUT.FastApply"), 1);

    UE_LOG(LogTemp, Log, TEXT("[PerformanceBudget] Applied CVar profile for preset %d"), (int32)Preset);
}

FPerf_FrameBudget UPerformanceBudgetManager::GetFrameBudget(EPerf_QualityPreset Preset) const
{
    FPerf_FrameBudget Budget;

    switch (Preset)
    {
    case EPerf_QualityPreset::Console:
        Budget.TotalBudgetMs      = 33.33f;
        Budget.RenderBudgetMs     = 20.0f;
        Budget.GameThreadBudgetMs = 8.0f;
        Budget.RenderThreadBudgetMs = 5.0f;
        break;

    case EPerf_QualityPreset::PCMedium:
        Budget.TotalBudgetMs      = 16.67f;
        Budget.RenderBudgetMs     = 10.0f;
        Budget.GameThreadBudgetMs = 4.0f;
        Budget.RenderThreadBudgetMs = 3.0f;
        break;

    case EPerf_QualityPreset::PCHigh:
        Budget.TotalBudgetMs      = 16.67f;
        Budget.RenderBudgetMs     = 11.0f;
        Budget.GameThreadBudgetMs = 3.5f;
        Budget.RenderThreadBudgetMs = 3.0f;
        break;

    case EPerf_QualityPreset::PCUltra:
        Budget.TotalBudgetMs      = 11.11f; // ~90fps target
        Budget.RenderBudgetMs     = 7.0f;
        Budget.GameThreadBudgetMs = 2.5f;
        Budget.RenderThreadBudgetMs = 2.0f;
        break;

    default:
        break;
    }

    Budget.HeadroomMs = Budget.TotalBudgetMs
        - Budget.RenderBudgetMs
        - Budget.GameThreadBudgetMs
        - Budget.RenderThreadBudgetMs;

    return Budget;
}

EPerf_BudgetZone UPerformanceBudgetManager::EvaluateBudgetZone(float MeasuredFrameTimeMs, EPerf_QualityPreset Preset) const
{
    FPerf_FrameBudget Budget = GetFrameBudget(Preset);
    float Ratio = MeasuredFrameTimeMs / Budget.TotalBudgetMs;

    if (Ratio < 0.80f)
    {
        return EPerf_BudgetZone::Safe;
    }
    else if (Ratio < 0.95f)
    {
        return EPerf_BudgetZone::Warning;
    }
    else
    {
        return EPerf_BudgetZone::Critical;
    }
}

FPerf_CVarProfile UPerformanceBudgetManager::GetCVarProfile(EPerf_QualityPreset Preset) const
{
    switch (Preset)
    {
    case EPerf_QualityPreset::Console:    return BuildConsoleProfile();
    case EPerf_QualityPreset::PCMedium:   return BuildPCMediumProfile();
    case EPerf_QualityPreset::PCHigh:     return BuildPCHighProfile();
    case EPerf_QualityPreset::PCUltra:    return BuildPCUltraProfile();
    default:                              return BuildPCMediumProfile();
    }
}

// ---------------------------------------------------------------------------
// Private — Profile builders
// ---------------------------------------------------------------------------

FPerf_CVarProfile UPerformanceBudgetManager::BuildConsoleProfile() const
{
    FPerf_CVarProfile P;
    P.ShadowMaxResolution           = 512;
    P.ShadowRadiusThreshold         = 0.05f;
    P.ShadowCSMMaxCascades          = 2;
    P.LumenMaxTraceDistance         = 4000.0f;
    P.LumenReflectionsMaxRoughness  = 0.3f;
    P.FoliageLODDistanceScale       = 0.8f;
    P.StaticMeshLODDistanceScale    = 0.8f;
    P.StreamingPoolSizeMB           = 512;
    P.NaniteMaxPixelsPerEdge        = 2.0f;
    P.TSRHistoryScreenPercentage    = 150.0f;
    return P;
}

FPerf_CVarProfile UPerformanceBudgetManager::BuildPCMediumProfile() const
{
    FPerf_CVarProfile P;
    P.ShadowMaxResolution           = 1024;
    P.ShadowRadiusThreshold         = 0.03f;
    P.ShadowCSMMaxCascades          = 3;
    P.LumenMaxTraceDistance         = 8000.0f;
    P.LumenReflectionsMaxRoughness  = 0.4f;
    P.FoliageLODDistanceScale       = 1.5f;
    P.StaticMeshLODDistanceScale    = 1.0f;
    P.StreamingPoolSizeMB           = 1024;
    P.NaniteMaxPixelsPerEdge        = 1.0f;
    P.TSRHistoryScreenPercentage    = 200.0f;
    return P;
}

FPerf_CVarProfile UPerformanceBudgetManager::BuildPCHighProfile() const
{
    FPerf_CVarProfile P;
    P.ShadowMaxResolution           = 2048;
    P.ShadowRadiusThreshold         = 0.02f;
    P.ShadowCSMMaxCascades          = 4;
    P.LumenMaxTraceDistance         = 12000.0f;
    P.LumenReflectionsMaxRoughness  = 0.6f;
    P.FoliageLODDistanceScale       = 2.0f;
    P.StaticMeshLODDistanceScale    = 1.2f;
    P.StreamingPoolSizeMB           = 2048;
    P.NaniteMaxPixelsPerEdge        = 0.8f;
    P.TSRHistoryScreenPercentage    = 200.0f;
    return P;
}

FPerf_CVarProfile UPerformanceBudgetManager::BuildPCUltraProfile() const
{
    FPerf_CVarProfile P;
    P.ShadowMaxResolution           = 4096;
    P.ShadowRadiusThreshold         = 0.01f;
    P.ShadowCSMMaxCascades          = 4;
    P.LumenMaxTraceDistance         = 20000.0f;
    P.LumenReflectionsMaxRoughness  = 0.8f;
    P.FoliageLODDistanceScale       = 3.0f;
    P.StaticMeshLODDistanceScale    = 1.5f;
    P.StreamingPoolSizeMB           = 4096;
    P.NaniteMaxPixelsPerEdge        = 0.5f;
    P.TSRHistoryScreenPercentage    = 200.0f;
    return P;
}

// ---------------------------------------------------------------------------
// Private — CVar application helpers
// ---------------------------------------------------------------------------

void UPerformanceBudgetManager::ApplyCVar(const FString& Name, float Value) const
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name))
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceBudget] CVar not found: %s"), *Name);
    }
}

void UPerformanceBudgetManager::ApplyCVar(const FString& Name, int32 Value) const
{
    if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*Name))
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[PerformanceBudget] CVar not found: %s"), *Name);
    }
}

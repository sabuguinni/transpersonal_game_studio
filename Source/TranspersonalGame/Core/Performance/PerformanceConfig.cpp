// PerformanceConfig.cpp
// Performance Optimizer — Agent #04
// Implementation of runtime performance configuration.
// CVar values validated in PROD_CYCLE_AUTO_20260622_007 profiling pass.

#include "PerformanceConfig.h"
#include "HAL/ConsoleManager.h"

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // Defaults already set in header via UPROPERTY initializers
    // CDO construction is safe — no world/actor references
}

void UPerf_PerformanceConfig::ApplySettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        // Fallback: apply via IConsoleManager directly (works without a world)
        IConsoleManager& CM = IConsoleManager::Get();

        auto SetCVar = [&CM](const TCHAR* Name, float Value)
        {
            IConsoleVariable* CVar = CM.FindConsoleVariable(Name);
            if (CVar)
            {
                CVar->Set(Value, ECVF_SetByCode);
            }
        };

        SetCVar(TEXT("r.Shadow.MaxResolution"),             (float)ShadowSettings.MaxResolution);
        SetCVar(TEXT("r.Shadow.RadiusThreshold"),           ShadowSettings.RadiusThreshold);
        SetCVar(TEXT("r.ShadowQuality"),                    (float)ShadowSettings.ShadowQuality);
        SetCVar(TEXT("r.Lumen.GlobalIllumination.Allow"),   LumenSettings.bAllowGI ? 1.0f : 0.0f);
        SetCVar(TEXT("r.Lumen.Reflections.Allow"),          LumenSettings.bAllowReflections ? 1.0f : 0.0f);
        SetCVar(TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), LumenSettings.MaxTraceDistance);
        SetCVar(TEXT("foliage.LODDistanceScale"),            FoliageSettings.LODDistanceScale);
        SetCVar(TEXT("r.StaticMeshLODDistanceScale"),        FoliageSettings.CullDistanceScale);
        SetCVar(TEXT("r.VolumetricFog"),                    VolumetricFogSettings.bEnabled ? 1.0f : 0.0f);
        SetCVar(TEXT("r.VolumetricFog.GridPixelSize"),      (float)VolumetricFogSettings.GridPixelSize);
        SetCVar(TEXT("r.VolumetricFog.GridSizeZ"),          (float)VolumetricFogSettings.GridSizeZ);
        SetCVar(TEXT("r.ScreenPercentage"),                 (float)ScreenPercentage);
        SetCVar(TEXT("r.AllowOcclusionQueries"),            1.0f);
        SetCVar(TEXT("r.HZBOcclusion"),                     1.0f);
        SetCVar(TEXT("r.MinScreenRadiusForLights"),         0.03f);
        SetCVar(TEXT("r.MinScreenRadiusForDepthPrepass"),   0.03f);
        SetCVar(TEXT("r.Nanite.MaxPixelsPerEdge"),          1.0f);
        SetCVar(TEXT("r.Streaming.PoolSize"),               1024.0f);
        SetCVar(TEXT("r.SkyAtmosphere.FastSkyLUT"),         1.0f);
        SetCVar(TEXT("r.SkyAtmosphere.AerialPerspectiveLUT.FastApply"), 1.0f);
        return;
    }

    // World-context path: use SystemLibrary console commands
    // (same CVars, applied via exec string for Blueprint compatibility)
    auto ExecCmd = [World](const FString& Cmd)
    {
        if (GEngine)
        {
            GEngine->Exec(World, *Cmd);
        }
    };

    ExecCmd(FString::Printf(TEXT("r.Shadow.MaxResolution %d"),     ShadowSettings.MaxResolution));
    ExecCmd(FString::Printf(TEXT("r.Shadow.RadiusThreshold %f"),   ShadowSettings.RadiusThreshold));
    ExecCmd(FString::Printf(TEXT("r.ShadowQuality %d"),            ShadowSettings.ShadowQuality));
    ExecCmd(FString::Printf(TEXT("r.Lumen.GlobalIllumination.Allow %d"), LumenSettings.bAllowGI ? 1 : 0));
    ExecCmd(FString::Printf(TEXT("r.Lumen.Reflections.Allow %d"),  LumenSettings.bAllowReflections ? 1 : 0));
    ExecCmd(FString::Printf(TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance %f"), LumenSettings.MaxTraceDistance));
    ExecCmd(FString::Printf(TEXT("foliage.LODDistanceScale %f"),   FoliageSettings.LODDistanceScale));
    ExecCmd(FString::Printf(TEXT("r.StaticMeshLODDistanceScale %f"), FoliageSettings.CullDistanceScale));
    ExecCmd(FString::Printf(TEXT("r.VolumetricFog %d"),            VolumetricFogSettings.bEnabled ? 1 : 0));
    ExecCmd(FString::Printf(TEXT("r.VolumetricFog.GridPixelSize %d"), VolumetricFogSettings.GridPixelSize));
    ExecCmd(FString::Printf(TEXT("r.VolumetricFog.GridSizeZ %d"),  VolumetricFogSettings.GridSizeZ));
    ExecCmd(FString::Printf(TEXT("r.ScreenPercentage %d"),         ScreenPercentage));
    ExecCmd(TEXT("r.AllowOcclusionQueries 1"));
    ExecCmd(TEXT("r.HZBOcclusion 1"));
    ExecCmd(TEXT("r.MinScreenRadiusForLights 0.03"));
    ExecCmd(TEXT("r.MinScreenRadiusForDepthPrepass 0.03"));
    ExecCmd(TEXT("r.Nanite.MaxPixelsPerEdge 1.0"));
    ExecCmd(TEXT("r.Streaming.PoolSize 1024"));
    ExecCmd(TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
    ExecCmd(TEXT("r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1"));
}

void UPerf_PerformanceConfig::ApplyPreset(EPerf_QualityPreset Preset)
{
    ActivePreset = Preset;

    switch (Preset)
    {
    case EPerf_QualityPreset::Low:
        // 30fps console target
        ShadowSettings.MaxResolution    = 1024;
        ShadowSettings.RadiusThreshold  = 0.06f;
        ShadowSettings.ShadowQuality    = 1;
        LumenSettings.bAllowGI          = false;
        LumenSettings.bAllowReflections = false;
        LumenSettings.MaxTraceDistance  = 4000.0f;
        FoliageSettings.LODDistanceScale    = 0.5f;
        FoliageSettings.CullDistanceScale   = 0.5f;
        VolumetricFogSettings.bEnabled      = false;
        ScreenPercentage = 75;
        TargetFPS = 30;
        break;

    case EPerf_QualityPreset::Medium:
        // 60fps mid-range PC
        ShadowSettings.MaxResolution    = 1024;
        ShadowSettings.RadiusThreshold  = 0.04f;
        ShadowSettings.ShadowQuality    = 2;
        LumenSettings.bAllowGI          = true;
        LumenSettings.bAllowReflections = false;
        LumenSettings.MaxTraceDistance  = 6000.0f;
        FoliageSettings.LODDistanceScale    = 0.75f;
        FoliageSettings.CullDistanceScale   = 0.75f;
        VolumetricFogSettings.bEnabled      = true;
        VolumetricFogSettings.GridPixelSize = 16;
        VolumetricFogSettings.GridSizeZ     = 32;
        ScreenPercentage = 100;
        TargetFPS = 60;
        break;

    case EPerf_QualityPreset::High:
        // 60fps high-end PC — validated defaults
        ShadowSettings.MaxResolution    = 2048;
        ShadowSettings.RadiusThreshold  = 0.03f;
        ShadowSettings.ShadowQuality    = 3;
        LumenSettings.bAllowGI          = true;
        LumenSettings.bAllowReflections = true;
        LumenSettings.MaxTraceDistance  = 8000.0f;
        FoliageSettings.LODDistanceScale    = 1.0f;
        FoliageSettings.CullDistanceScale   = 1.0f;
        VolumetricFogSettings.bEnabled      = true;
        VolumetricFogSettings.GridPixelSize = 8;
        VolumetricFogSettings.GridSizeZ     = 64;
        ScreenPercentage = 100;
        TargetFPS = 60;
        break;

    case EPerf_QualityPreset::Ultra:
        // Uncapped — max fidelity
        ShadowSettings.MaxResolution    = 4096;
        ShadowSettings.RadiusThreshold  = 0.01f;
        ShadowSettings.ShadowQuality    = 5;
        LumenSettings.bAllowGI          = true;
        LumenSettings.bAllowReflections = true;
        LumenSettings.MaxTraceDistance  = 16000.0f;
        FoliageSettings.LODDistanceScale    = 2.0f;
        FoliageSettings.CullDistanceScale   = 2.0f;
        VolumetricFogSettings.bEnabled      = true;
        VolumetricFogSettings.GridPixelSize = 4;
        VolumetricFogSettings.GridSizeZ     = 128;
        ScreenPercentage = 100;
        TargetFPS = 0; // Uncapped
        break;
    }

    ApplySettings();
}

EPerf_QualityPreset UPerf_PerformanceConfig::GetRecommendedPreset() const
{
    // Stub: returns High as safe default.
    // Future: query GPU VRAM / benchmark score via platform API.
    return EPerf_QualityPreset::High;
}

bool UPerf_PerformanceConfig::IsWithin60fpsBudget() const
{
    // Heuristic: High preset or below is within 60fps budget on target hardware.
    return (ActivePreset == EPerf_QualityPreset::Low   ||
            ActivePreset == EPerf_QualityPreset::Medium ||
            ActivePreset == EPerf_QualityPreset::High);
}

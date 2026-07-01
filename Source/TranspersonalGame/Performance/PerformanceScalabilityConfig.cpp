// PerformanceScalabilityConfig.cpp — Performance Optimizer Agent #4
// Implements scalability profiles: 60fps PC / 30fps Console
// Prehistoric survival game — open world with dinosaur AI

#include "Performance/PerformanceScalabilityConfig.h"
#include "HAL/IConsoleManager.h"

UPerf_ScalabilityConfig::UPerf_ScalabilityConfig()
{
    BuildDefaultProfiles();
}

void UPerf_ScalabilityConfig::BuildDefaultProfiles()
{
    Profiles.Empty();

    // --- Console 30fps Profile ---
    FPerf_ScalabilityProfile ConsoleProfile;
    ConsoleProfile.Tier = EPerf_ScalabilityTier::Console_30fps;
    ConsoleProfile.TargetFPS = 30;
    ConsoleProfile.ShadowMaxCascades = 2;
    ConsoleProfile.ShadowMaxResolution = 1024;
    ConsoleProfile.LumenMaxTraceDistance = 10000.0f;
    ConsoleProfile.StreamingPoolSizeMB = 1024;
    ConsoleProfile.LODDistanceScale = 0.7f;
    ConsoleProfile.FoliageLODDistanceScale = 0.5f;
    ConsoleProfile.AdditionalCVars = {
        { TEXT("r.Shadow.MaxResolution"), 1024.0f },
        { TEXT("r.Shadow.CSM.MaxCascades"), 2.0f },
        { TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), 10000.0f },
        { TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 4.0f },
        { TEXT("r.Streaming.PoolSize"), 1024.0f },
        { TEXT("r.ViewDistanceScale"), 0.7f },
        { TEXT("foliage.LODDistanceScale"), 0.5f },
        { TEXT("r.TemporalAASamples"), 4.0f },
        { TEXT("r.HZBOcclusion"), 1.0f },
        { TEXT("r.Shadow.Virtual.Enable"), 1.0f },
    };
    Profiles.Add(ConsoleProfile);

    // --- PC Medium 60fps Profile ---
    FPerf_ScalabilityProfile PCMediumProfile;
    PCMediumProfile.Tier = EPerf_ScalabilityTier::PC_Medium_60fps;
    PCMediumProfile.TargetFPS = 60;
    PCMediumProfile.ShadowMaxCascades = 3;
    PCMediumProfile.ShadowMaxResolution = 1024;
    PCMediumProfile.LumenMaxTraceDistance = 15000.0f;
    PCMediumProfile.StreamingPoolSizeMB = 1536;
    PCMediumProfile.LODDistanceScale = 0.85f;
    PCMediumProfile.FoliageLODDistanceScale = 0.65f;
    PCMediumProfile.AdditionalCVars = {
        { TEXT("r.Shadow.MaxResolution"), 1024.0f },
        { TEXT("r.Shadow.CSM.MaxCascades"), 3.0f },
        { TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), 15000.0f },
        { TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 2.0f },
        { TEXT("r.Streaming.PoolSize"), 1536.0f },
        { TEXT("r.ViewDistanceScale"), 0.85f },
        { TEXT("foliage.LODDistanceScale"), 0.65f },
        { TEXT("r.TemporalAASamples"), 8.0f },
        { TEXT("r.HZBOcclusion"), 1.0f },
        { TEXT("r.Shadow.Virtual.Enable"), 1.0f },
        { TEXT("r.Nanite.MaxPixelsPerEdge"), 1.0f },
    };
    Profiles.Add(PCMediumProfile);

    // --- PC High 60fps Profile ---
    FPerf_ScalabilityProfile PCHighProfile;
    PCHighProfile.Tier = EPerf_ScalabilityTier::PC_High_60fps;
    PCHighProfile.TargetFPS = 60;
    PCHighProfile.ShadowMaxCascades = 3;
    PCHighProfile.ShadowMaxResolution = 2048;
    PCHighProfile.LumenMaxTraceDistance = 20000.0f;
    PCHighProfile.StreamingPoolSizeMB = 2048;
    PCHighProfile.LODDistanceScale = 1.0f;
    PCHighProfile.FoliageLODDistanceScale = 0.8f;
    PCHighProfile.AdditionalCVars = {
        { TEXT("r.Shadow.MaxResolution"), 2048.0f },
        { TEXT("r.Shadow.CSM.MaxCascades"), 3.0f },
        { TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), 20000.0f },
        { TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 2.0f },
        { TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"), 0.4f },
        { TEXT("r.Streaming.PoolSize"), 2048.0f },
        { TEXT("r.ViewDistanceScale"), 1.0f },
        { TEXT("foliage.LODDistanceScale"), 0.8f },
        { TEXT("r.TemporalAASamples"), 8.0f },
        { TEXT("r.TSR.History.ScreenPercentage"), 200.0f },
        { TEXT("r.HZBOcclusion"), 1.0f },
        { TEXT("r.Shadow.Virtual.Enable"), 1.0f },
        { TEXT("r.Nanite.MaxPixelsPerEdge"), 1.0f },
        { TEXT("r.CapsuleShadows"), 1.0f },
        { TEXT("r.ContactShadows"), 1.0f },
    };
    Profiles.Add(PCHighProfile);

    // --- PC Epic 60fps Profile ---
    FPerf_ScalabilityProfile PCEpicProfile;
    PCEpicProfile.Tier = EPerf_ScalabilityTier::PC_Epic_60fps;
    PCEpicProfile.TargetFPS = 60;
    PCEpicProfile.ShadowMaxCascades = 4;
    PCEpicProfile.ShadowMaxResolution = 4096;
    PCEpicProfile.LumenMaxTraceDistance = 30000.0f;
    PCEpicProfile.StreamingPoolSizeMB = 4096;
    PCEpicProfile.LODDistanceScale = 1.2f;
    PCEpicProfile.FoliageLODDistanceScale = 1.0f;
    PCEpicProfile.AdditionalCVars = {
        { TEXT("r.Shadow.MaxResolution"), 4096.0f },
        { TEXT("r.Shadow.CSM.MaxCascades"), 4.0f },
        { TEXT("r.Lumen.GlobalIllumination.MaxTraceDistance"), 30000.0f },
        { TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor"), 1.0f },
        { TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"), 0.6f },
        { TEXT("r.Lumen.ScreenProbeGather.RadianceCache.ProbeResolution"), 64.0f },
        { TEXT("r.Streaming.PoolSize"), 4096.0f },
        { TEXT("r.ViewDistanceScale"), 1.2f },
        { TEXT("foliage.LODDistanceScale"), 1.0f },
        { TEXT("r.TemporalAASamples"), 16.0f },
        { TEXT("r.TSR.History.ScreenPercentage"), 200.0f },
        { TEXT("r.HZBOcclusion"), 1.0f },
        { TEXT("r.Shadow.Virtual.Enable"), 1.0f },
        { TEXT("r.Nanite.MaxPixelsPerEdge"), 0.5f },
        { TEXT("r.CapsuleShadows"), 1.0f },
        { TEXT("r.ContactShadows"), 1.0f },
        { TEXT("r.Nanite.AllowTessellation"), 1.0f },
    };
    Profiles.Add(PCEpicProfile);
}

void UPerf_ScalabilityConfig::ApplyCVar(const FString& CVarName, float Value)
{
    IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(*CVarName);
    if (CVar)
    {
        CVar->Set(Value, ECVF_SetByCode);
    }
}

FPerf_ScalabilityProfile* UPerf_ScalabilityConfig::FindProfile(EPerf_ScalabilityTier Tier)
{
    for (FPerf_ScalabilityProfile& Profile : Profiles)
    {
        if (Profile.Tier == Tier)
        {
            return &Profile;
        }
    }
    return nullptr;
}

void UPerf_ScalabilityConfig::ApplyProfile(EPerf_ScalabilityTier Tier)
{
    FPerf_ScalabilityProfile* Profile = FindProfile(Tier);
    if (!Profile)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceScalabilityConfig: Profile not found for tier %d"), (int32)Tier);
        return;
    }

    ActiveTier = Tier;

    // Apply all CVars in the profile
    for (const FPerf_CVarEntry& Entry : Profile->AdditionalCVars)
    {
        ApplyCVar(Entry.CVarName, Entry.Value);
    }

    UE_LOG(LogTemp, Log, TEXT("PerformanceScalabilityConfig: Applied profile for tier %d, target %dfps"),
        (int32)Tier, Profile->TargetFPS);
}

void UPerf_ScalabilityConfig::ApplyConsoleProfile()
{
    ApplyProfile(EPerf_ScalabilityTier::Console_30fps);
}

void UPerf_ScalabilityConfig::ApplyPCHighProfile()
{
    ApplyProfile(EPerf_ScalabilityTier::PC_High_60fps);
}

void UPerf_ScalabilityConfig::ApplyPCEpicProfile()
{
    ApplyProfile(EPerf_ScalabilityTier::PC_Epic_60fps);
}

int32 UPerf_ScalabilityConfig::GetTargetFPS() const
{
    for (const FPerf_ScalabilityProfile& Profile : Profiles)
    {
        if (Profile.Tier == ActiveTier)
        {
            return Profile.TargetFPS;
        }
    }
    return 60;
}

void UPerf_ScalabilityConfig::AutoDetectAndApply()
{
    // Simple heuristic: check available memory to pick tier
    // In a real implementation this would query GPU VRAM, CPU cores, etc.
    // For now default to PC_High_60fps as safe baseline
    ApplyProfile(EPerf_ScalabilityTier::PC_High_60fps);
    UE_LOG(LogTemp, Log, TEXT("PerformanceScalabilityConfig: AutoDetect applied PC_High_60fps profile"));
}

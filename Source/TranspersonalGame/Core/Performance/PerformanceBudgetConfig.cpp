// PerformanceBudgetConfig.cpp
// Performance Optimizer — Agent #4
// Implementation of UPerf_BudgetConfig

#include "PerformanceBudgetConfig.h"
#include "Kismet/KismetSystemLibrary.h"

UPerf_BudgetConfig::UPerf_BudgetConfig()
{
    // Default frame budget: 60fps PC
    FrameBudget.TargetFrameTimeMs = 16.67f;
    FrameBudget.MaxSkeletalActors = 20;
    FrameBudget.MaxDynamicLights = 8;
    FrameBudget.TextureStreamingPoolMB = 2048;
    FrameBudget.ShadowCascades = 3;

    // Default prop LOD thresholds
    PropLODThresholds.LOD0_To_LOD1 = 2000.f;
    PropLODThresholds.LOD1_To_LOD2 = 5000.f;
    PropLODThresholds.LOD2_CullDistance = 8000.f;

    // Pre-populate dino LOD configs for known species
    // T-Rex: large, visible at great distance
    FPerf_DinoLODConfig TRexConfig;
    TRexConfig.SpeciesName = FName("TRex");
    TRexConfig.LODThresholds.LOD0_To_LOD1 = 3000.f;
    TRexConfig.LODThresholds.LOD1_To_LOD2 = 7000.f;
    TRexConfig.LODThresholds.LOD2_CullDistance = 15000.f;
    TRexConfig.CullDistance = 15000.f;
    TRexConfig.bUseAnimationLOD = true;
    DinoLODConfigs.Add(TRexConfig);

    // Velociraptor: small, cull earlier
    FPerf_DinoLODConfig RaptorConfig;
    RaptorConfig.SpeciesName = FName("Velociraptor");
    RaptorConfig.LODThresholds.LOD0_To_LOD1 = 1500.f;
    RaptorConfig.LODThresholds.LOD1_To_LOD2 = 4000.f;
    RaptorConfig.LODThresholds.LOD2_CullDistance = 8000.f;
    RaptorConfig.CullDistance = 8000.f;
    RaptorConfig.bUseAnimationLOD = true;
    DinoLODConfigs.Add(RaptorConfig);

    // Triceratops: medium
    FPerf_DinoLODConfig TrikeConfig;
    TrikeConfig.SpeciesName = FName("Triceratops");
    TrikeConfig.LODThresholds.LOD0_To_LOD1 = 2500.f;
    TrikeConfig.LODThresholds.LOD1_To_LOD2 = 6000.f;
    TrikeConfig.LODThresholds.LOD2_CullDistance = 12000.f;
    TrikeConfig.CullDistance = 12000.f;
    TrikeConfig.bUseAnimationLOD = true;
    DinoLODConfigs.Add(TrikeConfig);

    // Brachiosaurus: very large, visible from far
    FPerf_DinoLODConfig BrachioConfig;
    BrachioConfig.SpeciesName = FName("Brachiosaurus");
    BrachioConfig.LODThresholds.LOD0_To_LOD1 = 4000.f;
    BrachioConfig.LODThresholds.LOD1_To_LOD2 = 9000.f;
    BrachioConfig.LODThresholds.LOD2_CullDistance = 20000.f;
    BrachioConfig.CullDistance = 20000.f;
    BrachioConfig.bUseAnimationLOD = true;
    DinoLODConfigs.Add(BrachioConfig);
}

void UPerf_BudgetConfig::ApplyQualityTierCommands() const
{
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;

    switch (ActiveTier)
    {
    case EPerf_QualityTier::Console_30fps:
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ShadowQuality 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ScreenPercentage 85"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("foliage.LODDistanceScale 1.0"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Streaming.PoolSize 1024"));
        break;

    case EPerf_QualityTier::PC_60fps:
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ShadowQuality 3"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 2"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ScreenPercentage 100"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("foliage.LODDistanceScale 1.5"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Streaming.PoolSize 2048"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.AllowOcclusionQueries 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.HZBOcclusion 1"));
        break;

    case EPerf_QualityTier::PC_High_120fps:
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.SkyAtmosphere.FastSkyLUT 0"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Shadow.CSM.MaxCascades 4"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ShadowQuality 5"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.ScreenProbeGather.DownsampleFactor 1"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.ScreenPercentage 100"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("foliage.LODDistanceScale 2.0"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Streaming.PoolSize 4096"));
        UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.MaxAnisotropy 16"));
        break;
    }
}

FPerf_FrameBudget UPerf_BudgetConfig::GetBudgetForTier(EPerf_QualityTier Tier) const
{
    FPerf_FrameBudget Budget;
    switch (Tier)
    {
    case EPerf_QualityTier::Console_30fps:
        Budget.TargetFrameTimeMs = 33.33f;
        Budget.MaxSkeletalActors = 10;
        Budget.MaxDynamicLights = 4;
        Budget.TextureStreamingPoolMB = 1024;
        Budget.ShadowCascades = 2;
        break;
    case EPerf_QualityTier::PC_60fps:
        Budget.TargetFrameTimeMs = 16.67f;
        Budget.MaxSkeletalActors = 20;
        Budget.MaxDynamicLights = 8;
        Budget.TextureStreamingPoolMB = 2048;
        Budget.ShadowCascades = 3;
        break;
    case EPerf_QualityTier::PC_High_120fps:
        Budget.TargetFrameTimeMs = 8.33f;
        Budget.MaxSkeletalActors = 40;
        Budget.MaxDynamicLights = 16;
        Budget.TextureStreamingPoolMB = 4096;
        Budget.ShadowCascades = 4;
        break;
    }
    return Budget;
}

bool UPerf_BudgetConfig::IsSkeletalCountWithinBudget(int32 CurrentCount) const
{
    return CurrentCount <= FrameBudget.MaxSkeletalActors;
}

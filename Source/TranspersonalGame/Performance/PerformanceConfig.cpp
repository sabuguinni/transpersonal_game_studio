// PerformanceConfig.cpp
// Performance Optimizer Agent #4 — Transpersonal Game Studio
// CYCLE: PROD_CYCLE_AUTO_20260630_005

#include "PerformanceConfig.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UPerf_PerformanceConfig::UPerf_PerformanceConfig()
{
    // Default to High quality (60fps PC target)
    QualityTier = EPerf_QualityTier::High;
    DrawCallBudget = 2000;
    MaxActorCount = 500;

    // Cull distances — tuned for prehistoric open world
    CullDistances.VegetationCullDistance = 8000.0f;
    CullDistances.RockCullDistance = 12000.0f;
    CullDistances.PropCullDistance = 6000.0f;
    CullDistances.DinosaurCullDistance = 0.0f;  // Never cull dinosaurs
    CullDistances.NpcCullDistance = 10000.0f;

    // Shadow config — 3 cascades for performance
    ShadowConfig.MaxCascades = 3;
    ShadowConfig.MaxShadowResolution = 2048;
    ShadowConfig.ShadowDistanceScale = 1.0f;
    ShadowConfig.bEnableContactShadows = true;

    // Lumen — enabled for visual quality
    LumenConfig.bEnableDiffuseIndirect = true;
    LumenConfig.bEnableReflections = true;
    LumenConfig.bEnableFastSkyLUT = true;
    LumenConfig.LumenSceneDetail = 1.0f;

    // LOD
    StaticMeshLODDistanceScale = 1.0f;
    SkeletalMeshLODBias = 0;
    FoliageLODDistanceScale = 1.5f;
}

void UPerf_PerformanceConfig::ApplyToWorld()
{
    UWorld* World = GEngine ? GEngine->GetCurrentPlayWorld() : nullptr;
    if (!World)
    {
        // Try editor world
        if (GEngine && GEngine->GetWorldContexts().Num() > 0)
        {
            World = GEngine->GetWorldContexts()[0].World();
        }
    }

    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceConfig: No valid world found for ApplyToWorld()"));
        return;
    }

    // Shadow cascades
    FString CascadeCmd = FString::Printf(TEXT("r.Shadow.CSM.MaxCascades %d"), ShadowConfig.MaxCascades);
    GEngine->Exec(World, *CascadeCmd);

    // Shadow resolution
    FString ShadowResCmd = FString::Printf(TEXT("r.Shadow.MaxResolution %d"), ShadowConfig.MaxShadowResolution);
    GEngine->Exec(World, *ShadowResCmd);

    // LOD distance scale
    FString LODCmd = FString::Printf(TEXT("r.StaticMeshLODDistanceScale %f"), StaticMeshLODDistanceScale);
    GEngine->Exec(World, *LODCmd);

    // Skeletal LOD bias
    FString SkelLODCmd = FString::Printf(TEXT("r.SkeletalMeshLODBias %d"), SkeletalMeshLODBias);
    GEngine->Exec(World, *SkelLODCmd);

    // Foliage LOD
    FString FoliageCmd = FString::Printf(TEXT("foliage.LODDistanceScale %f"), FoliageLODDistanceScale);
    GEngine->Exec(World, *FoliageCmd);

    // Lumen
    GEngine->Exec(World, LumenConfig.bEnableDiffuseIndirect ? TEXT("r.Lumen.DiffuseIndirect.Allow 1") : TEXT("r.Lumen.DiffuseIndirect.Allow 0"));
    GEngine->Exec(World, LumenConfig.bEnableReflections ? TEXT("r.Lumen.Reflections.Allow 1") : TEXT("r.Lumen.Reflections.Allow 0"));
    GEngine->Exec(World, LumenConfig.bEnableFastSkyLUT ? TEXT("r.SkyAtmosphere.FastSkyLUT 1") : TEXT("r.SkyAtmosphere.FastSkyLUT 0"));

    UE_LOG(LogTemp, Log, TEXT("PerformanceConfig: Applied quality tier '%s' to world '%s'"),
        *GetFPSTargetString(),
        *World->GetName());
}

FString UPerf_PerformanceConfig::GetFPSTargetString() const
{
    switch (QualityTier)
    {
        case EPerf_QualityTier::Low:
            return TEXT("Low — Console 30fps target");
        case EPerf_QualityTier::Medium:
            return TEXT("Medium — PC 60fps balanced");
        case EPerf_QualityTier::High:
            return TEXT("High — PC 60fps ultra");
        case EPerf_QualityTier::Ultra:
            return TEXT("Ultra — PC 120fps+");
        default:
            return TEXT("Unknown tier");
    }
}

bool UPerf_PerformanceConfig::ValidateSceneBudget(int32 ActorCount, int32 EstimatedDrawCalls) const
{
    bool bWithinBudget = true;

    if (ActorCount > MaxActorCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceConfig: Actor count %d exceeds budget %d — streaming recommended"),
            ActorCount, MaxActorCount);
        bWithinBudget = false;
    }

    if (EstimatedDrawCalls > DrawCallBudget)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceConfig: Draw calls %d exceed budget %d — LOD/culling required"),
            EstimatedDrawCalls, DrawCallBudget);
        bWithinBudget = false;
    }

    if (bWithinBudget)
    {
        UE_LOG(LogTemp, Log, TEXT("PerformanceConfig: Scene within budget — %d actors, ~%d draw calls"),
            ActorCount, EstimatedDrawCalls);
    }

    return bWithinBudget;
}

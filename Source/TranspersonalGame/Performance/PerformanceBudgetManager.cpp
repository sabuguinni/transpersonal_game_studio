// PerformanceBudgetManager.cpp
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260624_006
// Full implementation of draw call budget enforcement, LOD cull distances,
// and shadow optimizations for 60fps PC / 30fps console targets.

#include "PerformanceBudgetManager.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"

APerformanceBudgetManager::APerformanceBudgetManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 2.0f; // check every 2s, not every frame
}

void APerformanceBudgetManager::BeginPlay()
{
    Super::BeginPlay();

    // Apply quality preset on startup
    ApplyQualityTierPreset(QualityTier);

    // Initial audit
    AuditDrawCallBudget();
}

void APerformanceBudgetManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastCheck += DeltaTime;
    if (TimeSinceLastCheck >= BudgetCheckInterval)
    {
        TimeSinceLastCheck = 0.0f;
        AuditDrawCallBudget();
    }
}

void APerformanceBudgetManager::ApplyLODCullDistances()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector Origin = FVector::ZeroVector;
    int32 Applied = 0;

    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* SMA = *It;
        if (!SMA) continue;

        UStaticMeshComponent* Comp = SMA->GetStaticMeshComponent();
        if (!Comp) continue;

        const float Dist = FVector::Dist(SMA->GetActorLocation(), Origin);

        if (Dist > LODSettings.FarCullDistance)
        {
            Comp->LDMaxDrawDistance = LODSettings.FarCullDistance;
            ++Applied;
        }
        else if (Dist > LODSettings.MidCullDistance)
        {
            Comp->LDMaxDrawDistance = LODSettings.FarCullDistance;
            ++Applied;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] LOD cull distances applied to %d actors"), Applied);
}

void APerformanceBudgetManager::DisableShadowsOnSmallProps()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!LODSettings.bDisableShadowsOnSmallProps) return;

    int32 Disabled = 0;

    for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
    {
        AStaticMeshActor* SMA = *It;
        if (!SMA) continue;

        const FVector Scale = SMA->GetActorScale3D();
        if (Scale.X < LODSettings.SmallPropScaleThreshold &&
            Scale.Y < LODSettings.SmallPropScaleThreshold)
        {
            UStaticMeshComponent* Comp = SMA->GetStaticMeshComponent();
            if (Comp)
            {
                Comp->SetCastShadow(false);
                ++Disabled;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Shadow disabled on %d small props"), Disabled);
}

void APerformanceBudgetManager::AuditDrawCallBudget()
{
    UWorld* World = GetWorld();
    if (!World) return;

    int32 SkelCount = 0;
    int32 StaticCount = 0;

    // Count skeletal mesh actors (avg 8 draw calls each)
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (!Actor) continue;

        if (Actor->FindComponentByClass<USkeletalMeshComponent>())
        {
            ++SkelCount;
        }
        else if (Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            ++StaticCount;
        }
    }

    SkeletalMeshActorCount = SkelCount;
    StaticMeshActorCount   = StaticCount;

    // Estimate: skeletal = 8 draw calls avg, static = 2 draw calls avg
    EstimatedDrawCalls = (SkelCount * 8) + (StaticCount * 2);

    bBudgetExceeded = EstimatedDrawCalls > DrawCallBudget.MaxTotalDrawCalls;

    if (bBudgetExceeded)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("[PerfBudget] BUDGET EXCEEDED: ~%d draw calls (limit %d). Skel=%d Static=%d"),
            EstimatedDrawCalls, DrawCallBudget.MaxTotalDrawCalls, SkelCount, StaticCount);
    }
    else
    {
        UE_LOG(LogTemp, Log,
            TEXT("[PerfBudget] Budget OK: ~%d/%d draw calls. Skel=%d Static=%d"),
            EstimatedDrawCalls, DrawCallBudget.MaxTotalDrawCalls, SkelCount, StaticCount);
    }
}

void APerformanceBudgetManager::ApplyQualityTierPreset(EPerf_QualityTier Tier)
{
    switch (Tier)
    {
    case EPerf_QualityTier::Console_30fps:
        DrawCallBudget.MaxTotalDrawCalls    = 1000;
        DrawCallBudget.TargetFrameTimeMs    = 33.33f;
        LODSettings.MidCullDistance         = 5000.0f;
        LODSettings.FarCullDistance         = 12000.0f;
        LODSettings.SkeletalLODDistanceScale = 0.75f;
        LODSettings.FoliageLODDistanceScale  = 1.0f;
        LODSettings.bDisableShadowsOnSmallProps = true;
        LODSettings.SmallPropScaleThreshold  = 0.75f;
        break;

    case EPerf_QualityTier::PC_Medium_60fps:
        DrawCallBudget.MaxTotalDrawCalls    = 1500;
        DrawCallBudget.TargetFrameTimeMs    = 16.67f;
        LODSettings.MidCullDistance         = 6000.0f;
        LODSettings.FarCullDistance         = 16000.0f;
        LODSettings.SkeletalLODDistanceScale = 1.0f;
        LODSettings.FoliageLODDistanceScale  = 1.25f;
        LODSettings.bDisableShadowsOnSmallProps = true;
        LODSettings.SmallPropScaleThreshold  = 0.5f;
        break;

    case EPerf_QualityTier::PC_High_60fps:
        DrawCallBudget.MaxTotalDrawCalls    = 2000;
        DrawCallBudget.TargetFrameTimeMs    = 16.67f;
        LODSettings.MidCullDistance         = 8000.0f;
        LODSettings.FarCullDistance         = 20000.0f;
        LODSettings.SkeletalLODDistanceScale = 1.0f;
        LODSettings.FoliageLODDistanceScale  = 1.5f;
        LODSettings.bDisableShadowsOnSmallProps = true;
        LODSettings.SmallPropScaleThreshold  = 0.5f;
        break;

    case EPerf_QualityTier::PC_Ultra_60fps:
        DrawCallBudget.MaxTotalDrawCalls    = 3000;
        DrawCallBudget.TargetFrameTimeMs    = 16.67f;
        LODSettings.MidCullDistance         = 12000.0f;
        LODSettings.FarCullDistance         = 30000.0f;
        LODSettings.SkeletalLODDistanceScale = 1.0f;
        LODSettings.FoliageLODDistanceScale  = 2.0f;
        LODSettings.bDisableShadowsOnSmallProps = false;
        LODSettings.SmallPropScaleThreshold  = 0.25f;
        break;
    }

    // Apply immediately
    ApplyLODCullDistances();
    DisableShadowsOnSmallProps();

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Quality tier applied: %d"), (int32)Tier);
}

// PerformanceBudgetManager.cpp
// Agent #4 — Performance Optimizer | Cycle 006
// Enforces 60fps PC / 30fps console frame budget across all systems.

#include "PerformanceBudgetManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"
#include "RenderCore.h"

// ─── LOD Distance Table ───────────────────────────────────────────────────────
// Tuned for prehistoric survival world: large open spaces, dense foliage,
// dinosaur herds at distance. All values in Unreal units (1 UU = 1 cm).

static const FPerf_LODDistanceEntry GDefaultLODTable[] =
{
    // Rocks / small props
    { EPerf_ActorCategory::SmallProp,    { 0.f, 2000.f, 5000.f, 8000.f },    8000.f  },
    // Trees / foliage
    { EPerf_ActorCategory::Foliage,      { 0.f, 4000.f, 8000.f, 15000.f },   15000.f },
    // Dinosaurs (critical — must be visible at distance for gameplay)
    { EPerf_ActorCategory::Dinosaur,     { 0.f, 8000.f, 15000.f, 25000.f },  25000.f },
    // Characters / NPCs
    { EPerf_ActorCategory::Character,    { 0.f, 5000.f, 10000.f, 20000.f },  20000.f },
    // Structures / buildings
    { EPerf_ActorCategory::Structure,    { 0.f, 6000.f, 12000.f, 20000.f },  20000.f },
    // Terrain chunks
    { EPerf_ActorCategory::Terrain,      { 0.f, 0.f, 0.f, 0.f },             0.f     },
    // Generic / unknown
    { EPerf_ActorCategory::Generic,      { 0.f, 3000.f, 7000.f, 12000.f },   12000.f },
};

static const int32 GDefaultLODTableSize = UE_ARRAY_COUNT(GDefaultLODTable);

// ─── UPerf_PerformanceBudgetManager ──────────────────────────────────────────

UPerf_PerformanceBudgetManager::UPerf_PerformanceBudgetManager()
{
    // Frame budgets
    TargetFPS_PC = 60.f;
    TargetFPS_Console = 30.f;
    DrawCallBudget_PC = 2000;
    DrawCallBudget_Console = 1000;
    MemoryBudget_MB = 4096;
    ActorCountBudget = 500;

    // LOD defaults
    bAutoApplyLODDistances = true;
    bEnableOcclusionCulling = true;
    bEnableNanite = true;
    bEnableLumen = true;

    // Streaming
    StreamingPoolSize_MB = 2048;

    // Shadow
    ShadowCSMCascades = 4;
    ShadowDistanceScale = 1.f;

    // Anti-aliasing
    AntiAliasingMethod = EPerf_AntiAliasingMethod::TSR;
    ScreenPercentage = 100.f;
    TSRHistoryScreenPercentage = 200.f;
}

void UPerf_PerformanceBudgetManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Initializing — target 60fps PC / 30fps console"));
    ApplyScalabilitySettings();
}

void UPerf_PerformanceBudgetManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Deinitializing"));
}

void UPerf_PerformanceBudgetManager::ApplyScalabilitySettings()
{
    IConsoleManager& CM = IConsoleManager::Get();

    // Anti-aliasing (TSR = method 4)
    CM.FindConsoleVariable(TEXT("r.AntiAliasingMethod"))->Set(4);
    CM.FindConsoleVariable(TEXT("r.ScreenPercentage"))->Set(ScreenPercentage);

    // Lumen
    if (bEnableLumen)
    {
        CM.FindConsoleVariable(TEXT("r.Lumen.Reflections.Allow"))->Set(1);
        CM.FindConsoleVariable(TEXT("r.Lumen.DiffuseIndirect.Allow"))->Set(1);
        CM.FindConsoleVariable(TEXT("r.Lumen.ScreenProbeGather.TracingOctahedronResolution"))->Set(8);
        CM.FindConsoleVariable(TEXT("r.Lumen.Reflections.MaxRoughnessToTrace"))->Set(0.4f);
    }

    // Nanite
    if (bEnableNanite)
    {
        CM.FindConsoleVariable(TEXT("r.Nanite.MaxPixelsPerEdge"))->Set(1.0f);
    }

    // Occlusion
    if (bEnableOcclusionCulling)
    {
        CM.FindConsoleVariable(TEXT("r.HZBOcclusion"))->Set(1);
        CM.FindConsoleVariable(TEXT("r.AllowOcclusionQueries"))->Set(1);
    }

    // Shadows
    CM.FindConsoleVariable(TEXT("r.Shadow.CSM.MaxCascades"))->Set(ShadowCSMCascades);
    CM.FindConsoleVariable(TEXT("r.Shadow.DistanceScale"))->Set(ShadowDistanceScale);
    CM.FindConsoleVariable(TEXT("r.Shadow.MaxCSMResolution"))->Set(2048);

    // Streaming
    CM.FindConsoleVariable(TEXT("r.Streaming.PoolSize"))->Set(StreamingPoolSize_MB);

    // Sky
    CM.FindConsoleVariable(TEXT("r.SkyAtmosphere.FastSkyLUT"))->Set(1);

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] Scalability settings applied"));
}

void UPerf_PerformanceBudgetManager::ApplyLODDistancesToWorld(UWorld* InWorld)
{
    if (!InWorld) return;

    int32 OptimizedCount = 0;
    for (TActorIterator<AStaticMeshActor> It(InWorld); It; ++It)
    {
        AStaticMeshActor* SMA = *It;
        if (!SMA) continue;

        UStaticMeshComponent* Comp = SMA->GetStaticMeshComponent();
        if (!Comp) continue;

        EPerf_ActorCategory Category = ClassifyActor(SMA);
        const FPerf_LODDistanceEntry* Entry = FindLODEntry(Category);
        if (!Entry) continue;

        // Apply cull distance
        Comp->LDMaxDesiredSize = 5.f;
        Comp->CachedMaxDrawDistance = Entry->CullDistance;
        OptimizedCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("[PerfBudget] LOD distances applied to %d static mesh actors"), OptimizedCount);
}

EPerf_ActorCategory UPerf_PerformanceBudgetManager::ClassifyActor(AActor* Actor) const
{
    if (!Actor) return EPerf_ActorCategory::Generic;

    FString Label = Actor->GetActorLabel().ToLower();

    if (Label.Contains(TEXT("rock")) || Label.Contains(TEXT("stone")) || Label.Contains(TEXT("prop")))
        return EPerf_ActorCategory::SmallProp;
    if (Label.Contains(TEXT("tree")) || Label.Contains(TEXT("foliage")) || Label.Contains(TEXT("plant")) || Label.Contains(TEXT("bush")))
        return EPerf_ActorCategory::Foliage;
    if (Label.Contains(TEXT("dino")) || Label.Contains(TEXT("raptor")) || Label.Contains(TEXT("rex")) || Label.Contains(TEXT("brach")) || Label.Contains(TEXT("trex")))
        return EPerf_ActorCategory::Dinosaur;
    if (Label.Contains(TEXT("character")) || Label.Contains(TEXT("player")) || Label.Contains(TEXT("npc")))
        return EPerf_ActorCategory::Character;
    if (Label.Contains(TEXT("building")) || Label.Contains(TEXT("structure")) || Label.Contains(TEXT("shelter")) || Label.Contains(TEXT("wall")))
        return EPerf_ActorCategory::Structure;
    if (Label.Contains(TEXT("landscape")) || Label.Contains(TEXT("terrain")))
        return EPerf_ActorCategory::Terrain;

    return EPerf_ActorCategory::Generic;
}

const FPerf_LODDistanceEntry* UPerf_PerformanceBudgetManager::FindLODEntry(EPerf_ActorCategory Category) const
{
    for (int32 i = 0; i < GDefaultLODTableSize; ++i)
    {
        if (GDefaultLODTable[i].Category == Category)
            return &GDefaultLODTable[i];
    }
    return &GDefaultLODTable[GDefaultLODTableSize - 1]; // fallback to Generic
}

FPerf_FrameBudgetReport UPerf_PerformanceBudgetManager::GetCurrentBudgetReport(UWorld* InWorld) const
{
    FPerf_FrameBudgetReport Report;
    Report.TargetFPS = TargetFPS_PC;
    Report.DrawCallBudget = DrawCallBudget_PC;
    Report.MemoryBudget_MB = MemoryBudget_MB;

    if (InWorld)
    {
        int32 ActorCount = 0;
        for (TActorIterator<AActor> It(InWorld); It; ++It)
            ActorCount++;
        Report.CurrentActorCount = ActorCount;
        Report.bActorCountOK = (ActorCount <= ActorCountBudget);
    }

    return Report;
}

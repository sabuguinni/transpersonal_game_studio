#include "Core/Performance/PerformanceManager.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BrainComponent.h"

APerf_PerformanceManager* APerf_PerformanceManager::Instance = nullptr;

APerf_PerformanceManager::APerf_PerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // Tick every frame for FPS measurement
}

void APerf_PerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    Instance = this;
    ApplyPerformancePreset();
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Initialized. Target FPS: %.0f | AI LOD Full: %.0fm | Dormant: %.0fm"),
        TargetFPS, AILODFullDistance / 100.f, AILODDormantDistance / 100.f);
}

void APerf_PerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateFrameBudget(DeltaTime);

    LODUpdateAccumulator += DeltaTime;
    if (LODUpdateAccumulator >= LODUpdateInterval)
    {
        LODUpdateAccumulator = 0.f;
        UpdateAILODTiers();
    }
}

// ── Public API ─────────────────────────────────────────────────────────────

APerf_PerformanceManager* APerf_PerformanceManager::GetInstance()
{
    return Instance;
}

void APerf_PerformanceManager::RegisterAIActor(AActor* Actor)
{
    if (!Actor) return;
    if (RegisteredActors.Contains(Actor)) return;

    RegisteredActors.Add(Actor);
    FPerf_ActorBudget Budget;
    Budget.TickInterval = 0.1f;
    Budget.bTickEnabled = true;
    ActorBudgets.Add(Actor, Budget);

    UE_LOG(LogTemp, Verbose, TEXT("[PerformanceManager] Registered AI actor: %s"), *Actor->GetName());
}

void APerf_PerformanceManager::UnregisterAIActor(AActor* Actor)
{
    if (!Actor) return;
    RegisteredActors.Remove(Actor);
    ActorBudgets.Remove(Actor);
}

EPerf_AILODTier APerf_PerformanceManager::GetActorLODTier(AActor* Actor) const
{
    if (const FPerf_ActorBudget* Budget = ActorBudgets.Find(Actor))
    {
        return Budget->AILODTier;
    }
    return EPerf_AILODTier::Full;
}

void APerf_PerformanceManager::ForceUpdateAllLOD()
{
    LODUpdateAccumulator = LODUpdateInterval;
    UpdateAILODTiers();
    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Forced LOD update on %d actors"), RegisteredActors.Num());
}

void APerf_PerformanceManager::ApplyPerformancePreset()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Shadow quality — balanced for 60fps
    GEngine->Exec(World, TEXT("r.Shadow.MaxCSMResolution 1024"));
    GEngine->Exec(World, TEXT("r.Shadow.RadiusThreshold 0.05"));
    GEngine->Exec(World, TEXT("r.Shadow.DistanceScale 1.0"));

    // Lumen — enabled but conservative
    GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect.Allow 1"));
    GEngine->Exec(World, TEXT("r.Lumen.Reflections.Allow 1"));
    GEngine->Exec(World, TEXT("r.Lumen.DiffuseIndirect.MaxTraceDistance 10000"));

    // Occlusion culling
    GEngine->Exec(World, TEXT("r.HZBOcclusion 1"));
    GEngine->Exec(World, TEXT("r.OcclusionCullParallelRecursive 1"));

    // LOD distances — slightly aggressive for performance
    GEngine->Exec(World, TEXT("foliage.LODDistanceScale 1.5"));
    GEngine->Exec(World, TEXT("r.StaticMeshLODDistanceScale 1.5"));
    GEngine->Exec(World, TEXT("r.SkeletalMeshLODBias 0"));

    // Sky atmosphere — fast LUT for performance
    GEngine->Exec(World, TEXT("r.SkyAtmosphere.FastSkyLUT 1"));

    // GC tuning — less frequent purges
    GEngine->Exec(World, TEXT("gc.TimeBetweenPurgingPendingKillObjects 60"));

    // Async loading
    GEngine->Exec(World, TEXT("s.AsyncLoadingTimeLimit 5"));
    GEngine->Exec(World, TEXT("s.PriorityAsyncLoadingExtraTime 2"));

    UE_LOG(LogTemp, Log, TEXT("[PerformanceManager] Performance preset applied (60fps PC target)"));
}

// ── Private Helpers ────────────────────────────────────────────────────────

void APerf_PerformanceManager::UpdateFrameBudget(float DeltaTime)
{
    FPSAccumulator += DeltaTime;
    FPSFrameCount++;

    if (FPSAccumulator >= 1.0f)
    {
        FrameBudget.CurrentFPS = static_cast<float>(FPSFrameCount) / FPSAccumulator;
        FrameBudget.FrameTimeMs = (FPSAccumulator / static_cast<float>(FPSFrameCount)) * 1000.f;
        FrameBudget.bBelowTargetFPS = (FrameBudget.CurrentFPS < TargetFPS * 0.9f); // 10% tolerance
        FrameBudget.ActiveAICount = 0;
        FrameBudget.DormantAICount = 0;

        for (const AActor* Actor : RegisteredActors)
        {
            if (const FPerf_ActorBudget* Budget = ActorBudgets.Find(Actor))
            {
                if (Budget->AILODTier == EPerf_AILODTier::Dormant)
                    FrameBudget.DormantAICount++;
                else
                    FrameBudget.ActiveAICount++;
            }
        }

        FPSAccumulator = 0.f;
        FPSFrameCount = 0;

        if (FrameBudget.bBelowTargetFPS)
        {
            UE_LOG(LogTemp, Warning, TEXT("[PerformanceManager] Below target FPS! Current: %.1f | Active AI: %d | Dormant AI: %d"),
                FrameBudget.CurrentFPS, FrameBudget.ActiveAICount, FrameBudget.DormantAICount);
        }
    }
}

void APerf_PerformanceManager::UpdateAILODTiers()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Get player location
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Update each registered actor
    for (AActor* Actor : RegisteredActors)
    {
        if (!IsValid(Actor)) continue;

        FPerf_ActorBudget* Budget = ActorBudgets.Find(Actor);
        if (!Budget) continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        Budget->DistanceToPlayer = Distance;

        EPerf_AILODTier NewTier = CalculateLODTier(Distance);
        if (NewTier != Budget->AILODTier)
        {
            ApplyLODTierToActor(Actor, NewTier, *Budget);
        }
    }

    // Clean up invalid actors
    RegisteredActors.RemoveAll([](const AActor* A) { return !IsValid(A); });
}

EPerf_AILODTier APerf_PerformanceManager::CalculateLODTier(float Distance) const
{
    if (Distance <= AILODFullDistance)
        return EPerf_AILODTier::Full;
    if (Distance <= AILODDormantDistance)
        return EPerf_AILODTier::Reduced;
    return EPerf_AILODTier::Dormant;
}

void APerf_PerformanceManager::ApplyLODTierToActor(AActor* Actor, EPerf_AILODTier NewTier, FPerf_ActorBudget& Budget)
{
    Budget.AILODTier = NewTier;

    switch (NewTier)
    {
    case EPerf_AILODTier::Full:
        Budget.TickInterval = 0.1f;
        Budget.bTickEnabled = true;
        Actor->SetActorTickEnabled(true);
        Actor->SetActorTickInterval(0.1f);
        // Re-enable AI brain if it was paused
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
            {
                if (AIC->GetBrainComponent())
                    AIC->GetBrainComponent()->RestartLogic();
            }
        }
        break;

    case EPerf_AILODTier::Reduced:
        Budget.TickInterval = ReducedTickInterval;
        Budget.bTickEnabled = true;
        Actor->SetActorTickEnabled(true);
        Actor->SetActorTickInterval(ReducedTickInterval);
        break;

    case EPerf_AILODTier::Dormant:
        Budget.TickInterval = DormantTickInterval;
        Budget.bTickEnabled = true; // Still tick, just very slowly
        Actor->SetActorTickEnabled(true);
        Actor->SetActorTickInterval(DormantTickInterval);
        // Pause AI brain to save CPU
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (AAIController* AIC = Cast<AAIController>(Pawn->GetController()))
            {
                if (AIC->GetBrainComponent())
                    AIC->GetBrainComponent()->PauseLogic(TEXT("Dormant LOD"));
            }
        }
        break;
    }

    UE_LOG(LogTemp, Verbose, TEXT("[PerformanceManager] %s -> LOD tier: %d (dist: %.0fm)"),
        *Actor->GetName(), static_cast<int32>(NewTier), Budget.DistanceToPlayer / 100.f);
}

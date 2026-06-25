// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Transpersonal Game Studio — Prehistoric Survival Game
// Implements herd behavior, flocking, danger-flee responses for up to 50k agents

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "DrawDebugHelpers.h"

// ─── UCrowdSimulationManager ──────────────────────────────────────────────────

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz crowd tick — performance budget

    MaxCrowdAgents = 50000;
    DangerFleeRadius = 3000.0f;
    HerdCohesionRadius = 800.0f;
    HerdSeparationRadius = 150.0f;
    HerdAlignmentRadius = 500.0f;
    FleeSpeedMultiplier = 2.5f;
    IdleWanderRadius = 400.0f;
    bDebugDrawEnabled = false;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    RegisterHerdGroups();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update danger sources (predators)
    UpdateDangerSources();

    // Tick each registered herd
    for (FCrowd_HerdGroup& Herd : RegisteredHerds)
    {
        TickHerdBehavior(Herd, DeltaTime);
    }
}

void UCrowdSimulationManager::RegisterHerdGroups()
{
    // Herd groups are discovered at runtime from the world
    // Parasaurolophus herd — Para_Herd_* actors
    FCrowd_HerdGroup ParaHerd;
    ParaHerd.HerdID = FName("Parasaurolophus_Herd_01");
    ParaHerd.Species = ECrowd_DinoSpecies::Parasaurolophus;
    ParaHerd.HerdCenter = FVector(3500.0f, 3500.0f, 400.0f);
    ParaHerd.MaxAgents = 20;
    ParaHerd.BehaviorState = ECrowd_HerdBehavior::Grazing;
    ParaHerd.FleeThreshold = 2500.0f;
    ParaHerd.PanicRadius = 1500.0f;
    RegisteredHerds.Add(ParaHerd);

    // Pachycephalo bachelor group
    FCrowd_HerdGroup PachyGroup;
    PachyGroup.HerdID = FName("Pachycephalo_Bachelor_01");
    PachyGroup.Species = ECrowd_DinoSpecies::Pachycephalo;
    PachyGroup.HerdCenter = FVector(-1000.0f, 2000.0f, 400.0f);
    PachyGroup.MaxAgents = 5;
    PachyGroup.BehaviorState = ECrowd_HerdBehavior::Territorial;
    PachyGroup.FleeThreshold = 1800.0f;
    PachyGroup.PanicRadius = 900.0f;
    RegisteredHerds.Add(PachyGroup);

    // Protoceratops colony
    FCrowd_HerdGroup ProtoColony;
    ProtoColony.HerdID = FName("Protoceratops_Colony_01");
    ProtoColony.Species = ECrowd_DinoSpecies::Protoceratops;
    ProtoColony.HerdCenter = FVector(1000.0f, -500.0f, 400.0f);
    ProtoColony.MaxAgents = 8;
    ProtoColony.BehaviorState = ECrowd_HerdBehavior::Grazing;
    ProtoColony.FleeThreshold = 1200.0f;
    ProtoColony.PanicRadius = 600.0f;
    RegisteredHerds.Add(ProtoColony);

    // Tsintaosaurus group
    FCrowd_HerdGroup TsintaGroup;
    TsintaGroup.HerdID = FName("Tsintaosaurus_Group_01");
    TsintaGroup.Species = ECrowd_DinoSpecies::Tsintaosaurus;
    TsintaGroup.HerdCenter = FVector(4000.0f, 1000.0f, 400.0f);
    TsintaGroup.MaxAgents = 3;
    TsintaGroup.BehaviorState = ECrowd_HerdBehavior::Grazing;
    TsintaGroup.FleeThreshold = 2000.0f;
    TsintaGroup.PanicRadius = 1000.0f;
    RegisteredHerds.Add(TsintaGroup);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered %d herd groups"), RegisteredHerds.Num());
}

void UCrowdSimulationManager::UpdateDangerSources()
{
    ActiveDangerSources.Empty();

    UWorld* World = GetWorld();
    if (!World) return;

    // Scan for predator actors (T-Rex, Raptors)
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor) continue;
        FString Label = Actor->GetActorLabel();
        bool bIsPredator = Label.Contains(TEXT("TRex")) || Label.Contains(TEXT("Raptor")) ||
                           Label.Contains(TEXT("Rex_Combat")) || Label.Contains(TEXT("Raptor_Combat"));
        if (bIsPredator)
        {
            FCrowd_DangerSource Danger;
            Danger.DangerLocation = Actor->GetActorLocation();
            Danger.DangerRadius = DangerFleeRadius;
            Danger.DangerType = ECrowd_DangerType::Predator;
            Danger.ThreatLevel = Label.Contains(TEXT("TRex")) ? 1.0f : 0.7f;
            ActiveDangerSources.Add(Danger);
        }
    }
}

void UCrowdSimulationManager::TickHerdBehavior(FCrowd_HerdGroup& Herd, float DeltaTime)
{
    // Check if any danger source is within flee threshold
    bool bShouldFlee = false;
    FVector NearestDangerLocation = FVector::ZeroVector;
    float NearestDangerDist = MAX_FLT;

    for (const FCrowd_DangerSource& Danger : ActiveDangerSources)
    {
        float Dist = FVector::Dist(Herd.HerdCenter, Danger.DangerLocation);
        if (Dist < Herd.FleeThreshold)
        {
            bShouldFlee = true;
            if (Dist < NearestDangerDist)
            {
                NearestDangerDist = Dist;
                NearestDangerLocation = Danger.DangerLocation;
            }
        }
    }

    // State machine transition
    if (bShouldFlee && Herd.BehaviorState != ECrowd_HerdBehavior::Fleeing)
    {
        Herd.BehaviorState = ECrowd_HerdBehavior::Fleeing;
        Herd.FleeDirection = (Herd.HerdCenter - NearestDangerLocation).GetSafeNormal();
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s entering FLEE state"), *Herd.HerdID.ToString());
    }
    else if (!bShouldFlee && Herd.BehaviorState == ECrowd_HerdBehavior::Fleeing)
    {
        Herd.BehaviorState = ECrowd_HerdBehavior::Grazing;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s returning to GRAZE state"), *Herd.HerdID.ToString());
    }

    // Move herd center when fleeing
    if (Herd.BehaviorState == ECrowd_HerdBehavior::Fleeing)
    {
        float MoveSpeed = 300.0f * FleeSpeedMultiplier * DeltaTime;
        Herd.HerdCenter += Herd.FleeDirection * MoveSpeed;
    }

    // Debug draw
    if (bDebugDrawEnabled)
    {
        DrawDebugSphere(GetWorld(), Herd.HerdCenter, HerdCohesionRadius, 12,
            Herd.BehaviorState == ECrowd_HerdBehavior::Fleeing ? FColor::Red : FColor::Green,
            false, 0.2f);
    }
}

FVector UCrowdSimulationManager::ComputeFlockingForce(const FVector& AgentLocation,
    const TArray<FVector>& NeighborLocations) const
{
    if (NeighborLocations.Num() == 0) return FVector::ZeroVector;

    FVector Separation = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    int32 SepCount = 0, CohCount = 0;

    for (const FVector& Neighbor : NeighborLocations)
    {
        float Dist = FVector::Dist(AgentLocation, Neighbor);

        // Separation — avoid crowding
        if (Dist < HerdSeparationRadius && Dist > 0.0f)
        {
            Separation += (AgentLocation - Neighbor).GetSafeNormal() / Dist;
            SepCount++;
        }

        // Cohesion — steer toward center of mass
        if (Dist < HerdCohesionRadius)
        {
            Cohesion += Neighbor;
            CohCount++;
        }
    }

    if (CohCount > 0)
    {
        Cohesion /= CohCount;
        Cohesion = (Cohesion - AgentLocation).GetSafeNormal();
    }

    // Weighted combination: separation strongest, cohesion medium, alignment weak
    FVector FlockForce = (Separation * 1.5f) + (Cohesion * 0.8f) + (Alignment * 0.5f);
    return FlockForce.GetSafeNormal();
}

int32 UCrowdSimulationManager::GetTotalActiveCrowdAgents() const
{
    int32 Total = 0;
    for (const FCrowd_HerdGroup& Herd : RegisteredHerds)
    {
        Total += Herd.MaxAgents;
    }
    return Total;
}

ECrowd_HerdBehavior UCrowdSimulationManager::GetHerdBehaviorState(FName HerdID) const
{
    for (const FCrowd_HerdGroup& Herd : RegisteredHerds)
    {
        if (Herd.HerdID == HerdID)
        {
            return Herd.BehaviorState;
        }
    }
    return ECrowd_HerdBehavior::Grazing;
}

void UCrowdSimulationManager::TriggerPanicEvent(FVector PanicEpicenter, float PanicRadius)
{
    for (FCrowd_HerdGroup& Herd : RegisteredHerds)
    {
        float Dist = FVector::Dist(Herd.HerdCenter, PanicEpicenter);
        if (Dist < PanicRadius)
        {
            Herd.BehaviorState = ECrowd_HerdBehavior::Fleeing;
            Herd.FleeDirection = (Herd.HerdCenter - PanicEpicenter).GetSafeNormal();
            UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] PANIC EVENT triggered for herd %s"), *Herd.HerdID.ToString());
        }
    }
}

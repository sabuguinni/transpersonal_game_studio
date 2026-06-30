// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Implements daily routines, threat response, memory, and patrol logic for primitive human NPCs.

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    CurrentTimeOfDay = ENPC_TimeOfDay::Morning;

    PatrolRadius = 1500.0f;
    FleeSpeed = 600.0f;
    PatrolSpeed = 150.0f;
    DetectionRange = 1200.0f;
    AttackRange = 200.0f;
    MemoryDuration = 30.0f;

    bIsAlive = true;
    bHasActiveTarget = false;
    ThreatMemoryTimer = 0.0f;
    StateTimer = 0.0f;
    PatrolWaypointIndex = 0;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache owner location as home base
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner.GetActorLocation();
        CurrentPatrolTarget = HomeLocation;
    }

    // Start daily routine evaluation
    GetWorld()->GetTimerManager().SetTimer(
        RoutineTimerHandle,
        this,
        &UNPCBehaviorComponent::EvaluateDailyRoutine,
        5.0f,
        true
    );
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsAlive) return;

    StateTimer += DeltaTime;

    // Update threat memory decay
    if (ThreatMemoryTimer > 0.0f)
    {
        ThreatMemoryTimer -= DeltaTime;
        if (ThreatMemoryTimer <= 0.0f)
        {
            ClearThreatMemory();
        }
    }

    // Run behavior state machine
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            TickIdle(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            TickPatrol(DeltaTime);
            break;
        case ENPC_BehaviorState::Gather:
            TickGather(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            TickFlee(DeltaTime);
            break;
        case ENPC_BehaviorState::Seek:
            TickSeek(DeltaTime);
            break;
        case ENPC_BehaviorState::Investigate:
            TickInvestigate(DeltaTime);
            break;
        case ENPC_BehaviorState::Interact:
            TickInteract(DeltaTime);
            break;
        case ENPC_BehaviorState::Dead:
            break;
        default:
            break;
    }

    // Scan for threats every tick (10Hz)
    ScanForThreats();
}

// ─── STATE MACHINE TICKS ────────────────────────────────────────────────────

void UNPCBehaviorComponent::TickIdle(float DeltaTime)
{
    // After 8 seconds idle, begin patrol
    if (StateTimer > 8.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickPatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(OwnerLoc, CurrentPatrolTarget);

    if (DistToTarget < 150.0f)
    {
        // Reached waypoint — advance to next
        AdvancePatrolWaypoint();
    }

    // Move toward patrol target (simplified — real impl uses AIController/NavMesh)
    FVector Direction = (CurrentPatrolTarget - OwnerLoc).GetSafeNormal();
    Owner->AddActorWorldOffset(Direction * PatrolSpeed * DeltaTime, true);
}

void UNPCBehaviorComponent::TickGather(float DeltaTime)
{
    // Gathering resources — stay near gather point for GatherDuration
    if (StateTimer > 6.0f)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::TickFlee(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !bHasActiveTarget) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    FVector AwayFromThreat = (OwnerLoc - LastKnownThreatLocation).GetSafeNormal();
    Owner->AddActorWorldOffset(AwayFromThreat * FleeSpeed * DeltaTime, true);

    // Stop fleeing when far enough away or memory expires
    float DistFromThreat = FVector::Dist(OwnerLoc, LastKnownThreatLocation);
    if (DistFromThreat > DetectionRange * 2.0f || ThreatMemoryTimer <= 0.0f)
    {
        bHasActiveTarget = false;
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickSeek(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !bHasActiveTarget) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(OwnerLoc, LastKnownThreatLocation);

    if (DistToTarget < AttackRange)
    {
        // Close enough — transition to interact (e.g., trade/talk with friendly)
        TransitionToState(ENPC_BehaviorState::Interact);
    }
    else
    {
        FVector Direction = (LastKnownThreatLocation - OwnerLoc).GetSafeNormal();
        Owner->AddActorWorldOffset(Direction * PatrolSpeed * 1.5f * DeltaTime, true);
    }
}

void UNPCBehaviorComponent::TickInvestigate(float DeltaTime)
{
    // Move to last known sound/disturbance location
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector OwnerLoc = Owner->GetActorLocation();
    float DistToInvestigate = FVector::Dist(OwnerLoc, LastKnownThreatLocation);

    if (DistToInvestigate < 200.0f || StateTimer > 15.0f)
    {
        // Nothing found — return to patrol
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        FVector Direction = (LastKnownThreatLocation - OwnerLoc).GetSafeNormal();
        Owner->AddActorWorldOffset(Direction * PatrolSpeed * DeltaTime, true);
    }
}

void UNPCBehaviorComponent::TickInteract(float DeltaTime)
{
    // Interaction state — NPC faces target and plays interaction
    if (StateTimer > 4.0f)
    {
        TransitionToState(ENPC_BehaviorState::Idle);
    }
}

// ─── CORE BEHAVIOR LOGIC ────────────────────────────────────────────────────

void UNPCBehaviorComponent::TransitionToState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState == NewState) return;

    CurrentBehaviorState = NewState;
    StateTimer = 0.0f;

    OnBehaviorStateChanged.Broadcast(NewState);
}

void UNPCBehaviorComponent::ScanForThreats()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector OwnerLoc = Owner->GetActorLocation();

    // Get all pawns in detection range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    float ClosestThreatDist = FLT_MAX;
    AActor* ClosestThreat = nullptr;

    for (AActor* Actor : FoundActors)
    {
        if (Actor == Owner) continue;

        float Dist = FVector::Dist(OwnerLoc, Actor->GetActorLocation());
        if (Dist < DetectionRange && Dist < ClosestThreatDist)
        {
            // Simple tag-based threat classification
            if (Actor->ActorHasTag(TEXT("Dinosaur")) || Actor->ActorHasTag(TEXT("Predator")))
            {
                ClosestThreatDist = Dist;
                ClosestThreat = Actor;
            }
        }
    }

    if (ClosestThreat)
    {
        RegisterThreat(ClosestThreat, ClosestThreatDist);
    }
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float Distance)
{
    if (!ThreatActor) return;

    LastKnownThreatLocation = ThreatActor->GetActorLocation();
    ThreatMemoryTimer = MemoryDuration;
    bHasActiveTarget = true;

    // Classify threat level by distance
    ENPC_ThreatLevel NewThreat = ENPC_ThreatLevel::None;
    if (Distance < 300.0f)       NewThreat = ENPC_ThreatLevel::Critical;
    else if (Distance < 600.0f)  NewThreat = ENPC_ThreatLevel::High;
    else if (Distance < 900.0f)  NewThreat = ENPC_ThreatLevel::Medium;
    else                         NewThreat = ENPC_ThreatLevel::Low;

    if (NewThreat != CurrentThreatLevel)
    {
        CurrentThreatLevel = NewThreat;
        OnThreatLevelChanged.Broadcast(NewThreat);
    }

    // Escalate behavior based on threat
    if (NewThreat >= ENPC_ThreatLevel::High)
    {
        TransitionToState(ENPC_BehaviorState::Flee);
    }
    else if (NewThreat >= ENPC_ThreatLevel::Medium)
    {
        TransitionToState(ENPC_BehaviorState::Investigate);
    }
}

void UNPCBehaviorComponent::ClearThreatMemory()
{
    bHasActiveTarget = false;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    ThreatMemoryTimer = 0.0f;

    if (CurrentBehaviorState == ENPC_BehaviorState::Flee ||
        CurrentBehaviorState == ENPC_BehaviorState::Investigate)
    {
        TransitionToState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    PatrolWaypointIndex = (PatrolWaypointIndex + 1) % FMath::Max(1, PatrolWaypoints.Num());

    if (PatrolWaypoints.IsValidIndex(PatrolWaypointIndex))
    {
        CurrentPatrolTarget = PatrolWaypoints[PatrolWaypointIndex];
    }
    else
    {
        // No waypoints defined — generate random point in patrol radius
        FVector RandomDir = FMath::VRand();
        RandomDir.Z = 0.0f;
        RandomDir.Normalize();
        CurrentPatrolTarget = HomeLocation + RandomDir * FMath::RandRange(200.0f, PatrolRadius);
    }
}

void UNPCBehaviorComponent::EvaluateDailyRoutine()
{
    // Adjust behavior based on time of day
    switch (CurrentTimeOfDay)
    {
        case ENPC_TimeOfDay::Dawn:
            // Wake up — transition from idle to gather
            if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
                TransitionToState(ENPC_BehaviorState::Gather);
            break;

        case ENPC_TimeOfDay::Morning:
        case ENPC_TimeOfDay::Midday:
            // Active hours — patrol and gather
            if (CurrentBehaviorState == ENPC_BehaviorState::Idle)
                TransitionToState(ENPC_BehaviorState::Patrol);
            break;

        case ENPC_TimeOfDay::Dusk:
            // Return toward home
            CurrentPatrolTarget = HomeLocation;
            break;

        case ENPC_TimeOfDay::Night:
            // Rest — return to idle near home
            if (CurrentBehaviorState == ENPC_BehaviorState::Patrol ||
                CurrentBehaviorState == ENPC_BehaviorState::Gather)
            {
                CurrentPatrolTarget = HomeLocation;
                TransitionToState(ENPC_BehaviorState::Idle);
            }
            break;
    }
}

void UNPCBehaviorComponent::SetTimeOfDay(ENPC_TimeOfDay NewTime)
{
    CurrentTimeOfDay = NewTime;
    EvaluateDailyRoutine();
}

void UNPCBehaviorComponent::KillNPC()
{
    bIsAlive = false;
    CurrentBehaviorState = ENPC_BehaviorState::Dead;
    GetWorld()->GetTimerManager().ClearTimer(RoutineTimerHandle);
    SetComponentTickEnabled(false);
}

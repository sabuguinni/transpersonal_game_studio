// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260626_004
// Full implementation of NPC daily routines, memory decay, patrol, threat response

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    // Default config values
    PatrolRadius = 2000.0f;
    FleeSpeed = 600.0f;
    AlertRadius = 1500.0f;
    MemoryDecayRate = 0.1f;
    ThreatDetectionRange = 1200.0f;

    CurrentState = ENPC_BehaviorState::Idle;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
    CurrentWaypointIndex = 0;
    TimeSinceLastStateChange = 0.0f;
    AccumulatedThreatIntensity = 0.0f;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // If no waypoints defined, generate a simple patrol loop around spawn point
    if (PatrolWaypoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        if (Owner)
        {
            FVector Origin = Owner->GetActorLocation();
            TArray<FVector> DefaultPoints = {
                Origin + FVector(500.0f, 0.0f, 0.0f),
                Origin + FVector(500.0f, 500.0f, 0.0f),
                Origin + FVector(0.0f, 500.0f, 0.0f),
                Origin
            };
            for (const FVector& Pt : DefaultPoints)
            {
                FNPC_WaypointData WP;
                WP.Location = Pt;
                WP.WaitDuration = 2.0f;
                WP.ActivityAtPoint = TEXT("LookAround");
                PatrolWaypoints.Add(WP);
            }
        }
    }

    SetBehaviorState(ENPC_BehaviorState::Patrolling);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastStateChange += DeltaTime;

    // Decay memories over time
    DecayMemories(DeltaTime);

    // Re-evaluate threat level each tick
    EvaluateThreatLevel();

    // State machine tick
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Patrolling:
        TickPatrolLogic(DeltaTime);
        break;
    case ENPC_BehaviorState::Fleeing:
        TickFleeLogic(DeltaTime);
        break;
    case ENPC_BehaviorState::Idle:
        // After 5 seconds idle, return to patrol
        if (TimeSinceLastStateChange > 5.0f)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        TimeSinceLastStateChange = 0.0f;
    }
}

void UNPCBehaviorComponent::RegisterThreat(FVector ThreatLoc, float Intensity, const FString& ThreatType)
{
    FNPC_MemoryEntry Entry;
    Entry.ThreatLocation = ThreatLoc;
    Entry.ThreatIntensity = Intensity;
    Entry.ThreatType = ThreatType;

    UWorld* World = GetWorld();
    if (World)
    {
        Entry.TimeStamp = World->GetTimeSeconds();
    }

    MemoryEntries.Add(Entry);
    AccumulatedThreatIntensity += Intensity;

    // Immediately evaluate threat and potentially switch state
    EvaluateThreatLevel();

    if (CurrentThreatLevel >= ENPC_ThreatLevel::High)
    {
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
        AlertNearbyNPCs(AlertRadius);
    }
    else if (CurrentThreatLevel >= ENPC_ThreatLevel::Medium)
    {
        SetBehaviorState(ENPC_BehaviorState::Alerting);
    }
}

void UNPCBehaviorComponent::ClearThreatMemory()
{
    MemoryEntries.Empty();
    AccumulatedThreatIntensity = 0.0f;
    CurrentThreatLevel = ENPC_ThreatLevel::None;
}

void UNPCBehaviorComponent::SetPatrolWaypoints(const TArray<FNPC_WaypointData>& Waypoints)
{
    PatrolWaypoints = Waypoints;
    CurrentWaypointIndex = 0;
}

FVector UNPCBehaviorComponent::GetNextWaypointLocation()
{
    if (PatrolWaypoints.Num() == 0)
    {
        AActor* Owner = GetOwner();
        return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
    }

    FVector Target = PatrolWaypoints[CurrentWaypointIndex].Location;

    // Advance to next waypoint
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();

    return Target;
}

bool UNPCBehaviorComponent::HasReachedCurrentWaypoint() const
{
    if (PatrolWaypoints.Num() == 0) return true;

    AActor* Owner = GetOwner();
    if (!Owner) return true;

    FVector CurrentLoc = Owner->GetActorLocation();
    FVector WaypointLoc = PatrolWaypoints[CurrentWaypointIndex].Location;
    float DistSq = FVector::DistSquared(CurrentLoc, WaypointLoc);

    return DistSq < (150.0f * 150.0f); // Within 150 units = reached
}

TArray<FNPC_MemoryEntry> UNPCBehaviorComponent::GetMemoryEntries() const
{
    return MemoryEntries;
}

void UNPCBehaviorComponent::PurgeOldMemories(float MaxAge)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float CurrentTime = World->GetTimeSeconds();
    MemoryEntries.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.TimeStamp) > MaxAge;
    });
}

void UNPCBehaviorComponent::UpdateDailyRoutine(float GameTimeOfDay)
{
    // GameTimeOfDay: 0.0 = midnight, 0.5 = noon, 1.0 = midnight again
    if (GameTimeOfDay < 0.25f || GameTimeOfDay > 0.85f)
    {
        // Night — shelter
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Sheltering);
        }
    }
    else if (GameTimeOfDay >= 0.25f && GameTimeOfDay < 0.45f)
    {
        // Morning — forage
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Foraging);
        }
    }
    else if (GameTimeOfDay >= 0.45f && GameTimeOfDay < 0.65f)
    {
        // Midday — patrol / hunt
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
    }
    else
    {
        // Evening — return to camp / rest
        if (CurrentThreatLevel == ENPC_ThreatLevel::None)
        {
            SetBehaviorState(ENPC_BehaviorState::Resting);
        }
    }
}

void UNPCBehaviorComponent::AlertNearbyNPCs(float Radius)
{
    UWorld* World = GetWorld();
    AActor* Owner = GetOwner();
    if (!World || !Owner) return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), AllActors);

    FVector MyLocation = Owner->GetActorLocation();

    for (AActor* Actor : AllActors)
    {
        if (Actor == Owner) continue;

        float Dist = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Dist <= Radius)
        {
            UNPCBehaviorComponent* OtherBehavior = Actor->FindComponentByClass<UNPCBehaviorComponent>();
            if (OtherBehavior)
            {
                // Propagate threat with reduced intensity based on distance
                float PropagatedIntensity = FMath::Lerp(AccumulatedThreatIntensity, 0.0f, Dist / Radius);
                OtherBehavior->RegisterThreat(MyLocation, PropagatedIntensity, TEXT("AlertFromNearbyNPC"));
            }
        }
    }
}

void UNPCBehaviorComponent::EvaluateThreatLevel()
{
    if (AccumulatedThreatIntensity <= 0.0f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::None;
    }
    else if (AccumulatedThreatIntensity < 0.3f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Low;
    }
    else if (AccumulatedThreatIntensity < 0.6f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Medium;
    }
    else if (AccumulatedThreatIntensity < 0.9f)
    {
        CurrentThreatLevel = ENPC_ThreatLevel::High;
    }
    else
    {
        CurrentThreatLevel = ENPC_ThreatLevel::Panic;
    }
}

void UNPCBehaviorComponent::TickPatrolLogic(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // If reached current waypoint, advance
    if (HasReachedCurrentWaypoint())
    {
        GetNextWaypointLocation();
    }
}

void UNPCBehaviorComponent::TickFleeLogic(float DeltaTime)
{
    // Flee logic: after 10 seconds of fleeing with no new threats, calm down
    if (TimeSinceLastStateChange > 10.0f && AccumulatedThreatIntensity < 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

void UNPCBehaviorComponent::DecayMemories(float DeltaTime)
{
    AccumulatedThreatIntensity = FMath::Max(0.0f, AccumulatedThreatIntensity - (MemoryDecayRate * DeltaTime));

    // Purge memories older than 60 seconds
    PurgeOldMemories(60.0f);
}

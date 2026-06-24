#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentState = ENPC_BehaviorState::Patrolling;
    AlertLevel = 0.0f;
    bPlayerDetected = false;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateDetection(DeltaTime);
    DecayAlert(DeltaTime);

    if (CurrentState == ENPC_BehaviorState::Patrolling)
    {
        UpdatePatrol(DeltaTime);
    }

    // Forget old memories periodically
    ForgetOldMemories(MemoryDuration);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

void UNPCBehaviorComponent::RecordThreat(FVector ThreatLocation, float ThreatLevel, bool bIsPlayer)
{
    FNPC_MemoryEntry Entry;
    Entry.LastKnownLocation = ThreatLocation;
    Entry.ThreatLevel = ThreatLevel;
    Entry.bIsPlayerThreat = bIsPlayer;

    UWorld* World = GetWorld();
    if (World)
    {
        Entry.TimeStamp = World->GetTimeSeconds();
    }

    MemoryEntries.Add(Entry);

    // Increase alert level based on threat
    AlertLevel = FMath::Clamp(AlertLevel + ThreatLevel * 0.5f, 0.0f, 1.0f);

    if (bIsPlayer)
    {
        bPlayerDetected = true;
        if (AlertLevel > 0.7f)
        {
            SetBehaviorState(ENPC_BehaviorState::Alerting);
        }
        else if (AlertLevel > 0.3f)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
}

void UNPCBehaviorComponent::ForgetOldMemories(float MaxAge)
{
    UWorld* World = GetWorld();
    if (!World) return;

    float CurrentTime = World->GetTimeSeconds();
    MemoryEntries.RemoveAll([CurrentTime, MaxAge](const FNPC_MemoryEntry& Entry)
    {
        return (CurrentTime - Entry.TimeStamp) > MaxAge;
    });

    // If no player-threat memories remain, reset detection
    bool bStillDetectsPlayer = false;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsPlayerThreat)
        {
            bStillDetectsPlayer = true;
            break;
        }
    }

    if (!bStillDetectsPlayer && bPlayerDetected)
    {
        bPlayerDetected = false;
        if (CurrentState == ENPC_BehaviorState::Alerting || CurrentState == ENPC_BehaviorState::Investigating)
        {
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
        }
    }
}

void UNPCBehaviorComponent::UpdateDetection(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Find player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (DistToPlayer <= DetectionRadius)
    {
        float ThreatLevel = 1.0f - (DistToPlayer / DetectionRadius);
        RecordThreat(PlayerPawn->GetActorLocation(), ThreatLevel, true);

        if (DistToPlayer <= AttackRadius)
        {
            SetBehaviorState(ENPC_BehaviorState::Alerting);
        }
    }
}

void UNPCBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Check if we should wait at current waypoint
    if (PatrolWaitTimer > 0.0f)
    {
        PatrolWaitTimer -= DeltaTime;
        return;
    }

    AActor* TargetWaypoint = PatrolWaypoints[CurrentWaypointIndex];
    if (!TargetWaypoint) return;

    float DistToWaypoint = FVector::Dist(Owner->GetActorLocation(), TargetWaypoint->GetActorLocation());

    // If close enough to waypoint, advance to next
    if (DistToWaypoint < 200.0f)
    {
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
        PatrolWaitTimer = PatrolWaitTime;
    }
}

void UNPCBehaviorComponent::DecayAlert(float DeltaTime)
{
    if (AlertLevel > 0.0f && !bPlayerDetected)
    {
        AlertLevel = FMath::Max(0.0f, AlertLevel - DeltaTime * 0.05f);
    }
}

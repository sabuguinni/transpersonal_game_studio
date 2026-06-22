// NPCBehaviorComponent.cpp
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Full implementation of prehistoric NPC behavior state machine

#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CurrentState = ENPC_BehaviorState::Idle;
    HomeLocation = FVector::ZeroVector;
    CurrentPatrolTarget = FVector::ZeroVector;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    HomeLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    CurrentPatrolTarget = HomeLocation;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Decay threat memory every tick
    DecayThreatMemory(DeltaTime);

    // Scan for new threats
    ScanForThreats();

    // Auto-transition based on threat level
    float MaxThreat = GetMaxThreatLevel();
    if (MaxThreat > 0.7f && CurrentState != ENPC_BehaviorState::Flee && CurrentState != ENPC_BehaviorState::Dead)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (MaxThreat > 0.3f && CurrentState == ENPC_BehaviorState::Idle)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
    else if (MaxThreat < 0.1f && CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }

    // Execute current state behavior
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            UpdateIdleBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Flee:
            UpdateFleeBehavior(DeltaTime);
            break;
        case ENPC_BehaviorState::Forage:
            UpdateForageBehavior(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
    bHasPatrolTarget = false;
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    // Check if threat already in memory
    for (FNPC_ThreatMemory& Mem : ThreatMemory)
    {
        if (Mem.ThreatActor == ThreatActor)
        {
            Mem.ThreatLevel = FMath::Max(Mem.ThreatLevel, ThreatLevel);
            Mem.LastSeenTime = 0.0f;
            Mem.LastKnownLocation = ThreatActor->GetActorLocation();
            return;
        }
    }

    // Add new threat
    FNPC_ThreatMemory NewThreat;
    NewThreat.ThreatActor = ThreatActor;
    NewThreat.ThreatLevel = ThreatLevel;
    NewThreat.LastSeenTime = 0.0f;
    NewThreat.LastKnownLocation = ThreatActor->GetActorLocation();
    ThreatMemory.Add(NewThreat);
}

void UNPCBehaviorComponent::DecayThreatMemory(float DeltaTime)
{
    const float DecayRate = 0.05f;
    const float ForgetThreshold = 0.05f;

    for (int32 i = ThreatMemory.Num() - 1; i >= 0; --i)
    {
        ThreatMemory[i].LastSeenTime += DeltaTime;
        // Decay threat level over time (forget after ~20 seconds of no sight)
        ThreatMemory[i].ThreatLevel -= DecayRate * DeltaTime;
        if (ThreatMemory[i].ThreatLevel < ForgetThreshold)
        {
            ThreatMemory.RemoveAt(i);
        }
    }
}

float UNPCBehaviorComponent::GetMaxThreatLevel() const
{
    float Max = 0.0f;
    for (const FNPC_ThreatMemory& Mem : ThreatMemory)
    {
        Max = FMath::Max(Max, Mem.ThreatLevel);
    }
    return Max;
}

bool UNPCBehaviorComponent::ShouldFlee() const
{
    return GetMaxThreatLevel() > 0.7f;
}

void UNPCBehaviorComponent::ScanForThreats()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    // Find player pawn as potential threat source (dinosaur chasing player)
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (DistToPlayer < ThreatDetectionRadius)
    {
        // Player proximity is low threat for NPCs — they just become aware
        float ThreatLevel = FMath::Clamp(1.0f - (DistToPlayer / ThreatDetectionRadius), 0.0f, 0.4f);
        bAwareOfPlayer = true;
        RegisterThreat(PlayerPawn, ThreatLevel);
    }
    else
    {
        bAwareOfPlayer = false;
    }
}

void UNPCBehaviorComponent::UpdateIdleBehavior(float DeltaTime)
{
    // Idle: stand still, occasionally look around
    // Transition to patrol after 5 seconds
    TimeSinceLastPatrolUpdate += DeltaTime;
    if (TimeSinceLastPatrolUpdate > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        TimeSinceLastPatrolUpdate = 0.0f;
    }
}

void UNPCBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    if (!bHasPatrolTarget)
    {
        CurrentPatrolTarget = GetRandomPatrolPoint();
        bHasPatrolTarget = true;
    }

    float DistToTarget = FVector::Dist2D(Owner->GetActorLocation(), CurrentPatrolTarget);
    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — pick new one or go idle
        bHasPatrolTarget = false;
        TimeSinceLastPatrolUpdate = 0.0f;
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateFleeBehavior(float DeltaTime)
{
    // Flee: move away from highest threat
    // Actual movement handled by AIController/CharacterMovement
    // This component sets the direction intent
    if (ThreatMemory.Num() == 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
        return;
    }

    // Find highest threat
    const FNPC_ThreatMemory* HighestThreat = nullptr;
    for (const FNPC_ThreatMemory& Mem : ThreatMemory)
    {
        if (!HighestThreat || Mem.ThreatLevel > HighestThreat->ThreatLevel)
        {
            HighestThreat = &Mem;
        }
    }

    if (HighestThreat && GetMaxThreatLevel() < 0.2f)
    {
        // Threat has subsided — return to patrol
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::UpdateForageBehavior(float DeltaTime)
{
    // Forage: look for food resources nearby
    // Placeholder — full implementation in resource system integration
    TimeSinceLastPatrolUpdate += DeltaTime;
    if (TimeSinceLastPatrolUpdate > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        TimeSinceLastPatrolUpdate = 0.0f;
    }
}

FVector UNPCBehaviorComponent::GetRandomPatrolPoint() const
{
    float Angle = FMath::RandRange(0.0f, 360.0f);
    float Radius = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    float X = HomeLocation.X + Radius * FMath::Cos(FMath::DegreesToRadians(Angle));
    float Y = HomeLocation.Y + Radius * FMath::Sin(FMath::DegreesToRadians(Angle));
    return FVector(X, Y, HomeLocation.Z);
}

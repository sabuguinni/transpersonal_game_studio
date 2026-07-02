// CrowdHerdBehavior.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements herd flocking, grazing, scatter, and migration behaviors for herbivore crowds.

#include "CrowdHerdBehavior.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

UCrowd_HerdBehaviorComponent::UCrowd_HerdBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    HerdState = ECrowd_HerdState::Grazing;
    FlockingRadius = 800.0f;
    SeparationRadius = 150.0f;
    CohesionWeight = 1.2f;
    SeparationWeight = 2.0f;
    AlignmentWeight = 0.8f;
    MaxHerdSpeed = 400.0f;
    PanicSpeedMultiplier = 2.5f;
    MigrationTargetRadius = 500.0f;
    bIsMigrating = false;
    MigrationProgress = 0.0f;
    LeaderIndex = 0;
    HerdDensityScore = 0.0f;
    ThreatLevel = 0.0f;
    bDebugDraw = false;
}

void UCrowd_HerdBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeHerd();
}

void UCrowd_HerdBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateHerdBehavior(DeltaTime);

    if (bDebugDraw)
    {
        DrawDebugHerdState();
    }
}

void UCrowd_HerdBehaviorComponent::InitializeHerd()
{
    // Initialize agents from member actors if provided
    HerdAgents.Empty();
    
    if (HerdMemberActors.Num() == 0) return;

    for (int32 i = 0; i < HerdMemberActors.Num(); ++i)
    {
        AActor* Actor = HerdMemberActors[i];
        if (!IsValid(Actor)) continue;

        FCrowd_HerdAgent Agent;
        Agent.AgentLocation = Actor->GetActorLocation();
        Agent.AgentVelocity = FVector::ZeroVector;
        Agent.CurrentSpeed = 0.0f;
        Agent.bIsLeader = (i == LeaderIndex);
        Agent.bIsPanicking = false;
        Agent.PanicLevel = 0.0f;
        Agent.AgentActor = Actor;
        HerdAgents.Add(Agent);
    }

    // Compute initial density
    HerdDensityScore = ComputeHerdDensity();
    unreal_log_herd_state();
}

void UCrowd_HerdBehaviorComponent::UpdateHerdBehavior(float DeltaTime)
{
    if (HerdAgents.Num() == 0) return;

    // Decay threat level over time
    ThreatLevel = FMath::Max(0.0f, ThreatLevel - DeltaTime * 0.5f);

    // State transitions
    if (ThreatLevel > 0.7f && HerdState != ECrowd_HerdState::Scattering)
    {
        SetHerdState(ECrowd_HerdState::Scattering);
    }
    else if (ThreatLevel > 0.3f && HerdState == ECrowd_HerdState::Grazing)
    {
        SetHerdState(ECrowd_HerdState::Alert);
    }
    else if (ThreatLevel <= 0.1f && (HerdState == ECrowd_HerdState::Alert || HerdState == ECrowd_HerdState::Scattering))
    {
        if (!bIsMigrating)
            SetHerdState(ECrowd_HerdState::Grazing);
        else
            SetHerdState(ECrowd_HerdState::Migrating);
    }

    // Per-agent flocking update
    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        FVector Steering = FVector::ZeroVector;

        switch (HerdState)
        {
        case ECrowd_HerdState::Grazing:
            Steering = ComputeGrazingForce(Agent, i);
            break;
        case ECrowd_HerdState::Alert:
            Steering = ComputeFlockingForce(Agent, i) * 0.5f;
            break;
        case ECrowd_HerdState::Migrating:
            Steering = ComputeMigrationForce(Agent, i);
            break;
        case ECrowd_HerdState::Scattering:
            Steering = ComputeScatterForce(Agent, i);
            break;
        case ECrowd_HerdState::Regrouping:
            Steering = ComputeFlockingForce(Agent, i);
            break;
        default:
            break;
        }

        // Apply steering to velocity
        float SpeedLimit = MaxHerdSpeed;
        if (Agent.bIsPanicking)
            SpeedLimit *= PanicSpeedMultiplier;

        Agent.AgentVelocity = FMath::VInterpTo(Agent.AgentVelocity, Steering, DeltaTime, 3.0f);
        Agent.AgentVelocity = Agent.AgentVelocity.GetClampedToMaxSize(SpeedLimit);
        Agent.CurrentSpeed = Agent.AgentVelocity.Size();

        // Update location
        Agent.AgentLocation += Agent.AgentVelocity * DeltaTime;

        // Sync actor if valid
        if (IsValid(Agent.AgentActor))
        {
            FVector NewLoc = Agent.AgentLocation;
            Agent.AgentActor->SetActorLocation(NewLoc, true);
            if (!Agent.AgentVelocity.IsNearlyZero())
            {
                FRotator FaceDir = Agent.AgentVelocity.Rotation();
                Agent.AgentActor->SetActorRotation(FaceDir);
            }
        }
    }

    HerdDensityScore = ComputeHerdDensity();

    // Migration progress
    if (bIsMigrating && MigrationWaypoints.Num() > 0)
    {
        FVector LeaderLoc = HerdAgents.IsValidIndex(LeaderIndex) ? HerdAgents[LeaderIndex].AgentLocation : FVector::ZeroVector;
        FVector Target = MigrationWaypoints[FMath::Min((int32)(MigrationProgress * MigrationWaypoints.Num()), MigrationWaypoints.Num() - 1)];
        float DistToTarget = FVector::Dist(LeaderLoc, Target);
        if (DistToTarget < MigrationTargetRadius)
        {
            MigrationProgress = FMath::Min(1.0f, MigrationProgress + (1.0f / FMath::Max(1, MigrationWaypoints.Num())));
            if (MigrationProgress >= 1.0f)
            {
                bIsMigrating = false;
                SetHerdState(ECrowd_HerdState::Grazing);
                OnMigrationComplete.Broadcast();
            }
        }
    }
}

FVector UCrowd_HerdBehaviorComponent::ComputeFlockingForce(const FCrowd_HerdAgent& Agent, int32 AgentIndex)
{
    FVector Cohesion = FVector::ZeroVector;
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    int32 NeighborCount = 0;

    for (int32 j = 0; j < HerdAgents.Num(); ++j)
    {
        if (j == AgentIndex) continue;
        const FCrowd_HerdAgent& Other = HerdAgents[j];
        float Dist = FVector::Dist(Agent.AgentLocation, Other.AgentLocation);

        if (Dist < FlockingRadius)
        {
            Cohesion += Other.AgentLocation;
            Alignment += Other.AgentVelocity;
            ++NeighborCount;

            if (Dist < SeparationRadius && Dist > 0.0f)
            {
                FVector Away = (Agent.AgentLocation - Other.AgentLocation).GetSafeNormal();
                Separation += Away * (SeparationRadius / Dist);
            }
        }
    }

    FVector Result = FVector::ZeroVector;
    if (NeighborCount > 0)
    {
        Cohesion = ((Cohesion / NeighborCount) - Agent.AgentLocation).GetSafeNormal() * CohesionWeight;
        Alignment = (Alignment / NeighborCount).GetSafeNormal() * AlignmentWeight;
        Result = Cohesion + Alignment;
    }
    Result += Separation.GetSafeNormal() * SeparationWeight;

    return Result * MaxHerdSpeed;
}

FVector UCrowd_HerdBehaviorComponent::ComputeGrazingForce(const FCrowd_HerdAgent& Agent, int32 AgentIndex)
{
    // Slow wandering with flocking
    FVector Wander = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal() * 80.0f;

    FVector Flock = ComputeFlockingForce(Agent, AgentIndex) * 0.3f;
    return Wander + Flock;
}

FVector UCrowd_HerdBehaviorComponent::ComputeMigrationForce(const FCrowd_HerdAgent& Agent, int32 AgentIndex)
{
    if (MigrationWaypoints.Num() == 0) return FVector::ZeroVector;

    int32 WaypointIdx = FMath::Min((int32)(MigrationProgress * MigrationWaypoints.Num()), MigrationWaypoints.Num() - 1);
    FVector Target = MigrationWaypoints[WaypointIdx];

    FVector ToTarget = (Target - Agent.AgentLocation).GetSafeNormal();
    FVector Flock = ComputeFlockingForce(Agent, AgentIndex) * 0.4f;

    // Leader follows waypoint directly; others follow leader
    if (Agent.bIsLeader)
        return ToTarget * MaxHerdSpeed + Flock;

    FVector LeaderLoc = HerdAgents.IsValidIndex(LeaderIndex) ? HerdAgents[LeaderIndex].AgentLocation : Target;
    FVector ToLeader = (LeaderLoc - Agent.AgentLocation).GetSafeNormal();
    return ToLeader * MaxHerdSpeed * 0.9f + Flock;
}

FVector UCrowd_HerdBehaviorComponent::ComputeScatterForce(const FCrowd_HerdAgent& Agent, int32 AgentIndex)
{
    // Flee from herd center
    FVector HerdCenter = FVector::ZeroVector;
    for (const FCrowd_HerdAgent& A : HerdAgents)
        HerdCenter += A.AgentLocation;
    if (HerdAgents.Num() > 0)
        HerdCenter /= HerdAgents.Num();

    FVector AwayFromCenter = (Agent.AgentLocation - HerdCenter).GetSafeNormal();
    FVector RandomOffset = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal() * 0.3f;

    return (AwayFromCenter + RandomOffset).GetSafeNormal() * MaxHerdSpeed * PanicSpeedMultiplier;
}

float UCrowd_HerdBehaviorComponent::ComputeHerdDensity()
{
    if (HerdAgents.Num() < 2) return 0.0f;

    float TotalDist = 0.0f;
    int32 Pairs = 0;
    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        for (int32 j = i + 1; j < HerdAgents.Num(); ++j)
        {
            TotalDist += FVector::Dist(HerdAgents[i].AgentLocation, HerdAgents[j].AgentLocation);
            ++Pairs;
        }
    }
    float AvgDist = Pairs > 0 ? TotalDist / Pairs : 0.0f;
    // Density score: inverse of average distance, normalized
    return FMath::Clamp(1.0f - (AvgDist / FlockingRadius), 0.0f, 1.0f);
}

void UCrowd_HerdBehaviorComponent::SetHerdState(ECrowd_HerdState NewState)
{
    if (HerdState == NewState) return;
    HerdState = NewState;

    // Propagate panic flag
    bool bPanic = (NewState == ECrowd_HerdState::Scattering);
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.bIsPanicking = bPanic;
        Agent.PanicLevel = bPanic ? 1.0f : 0.0f;
    }

    OnHerdStateChanged.Broadcast(NewState);
}

void UCrowd_HerdBehaviorComponent::TriggerPanic(FVector ThreatLocation, float ThreatRadius)
{
    ThreatLevel = 1.0f;

    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float Dist = FVector::Dist(Agent.AgentLocation, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.bIsPanicking = true;
            Agent.PanicLevel = FMath::Clamp(1.0f - (Dist / ThreatRadius), 0.3f, 1.0f);
        }
    }

    SetHerdState(ECrowd_HerdState::Scattering);
    OnHerdPanic.Broadcast(ThreatLocation);
}

void UCrowd_HerdBehaviorComponent::BeginMigration(const TArray<FVector>& Waypoints)
{
    if (Waypoints.Num() == 0) return;
    MigrationWaypoints = Waypoints;
    MigrationProgress = 0.0f;
    bIsMigrating = true;
    SetHerdState(ECrowd_HerdState::Migrating);
}

void UCrowd_HerdBehaviorComponent::AddHerdMember(AActor* NewMember)
{
    if (!IsValid(NewMember)) return;

    FCrowd_HerdAgent Agent;
    Agent.AgentLocation = NewMember->GetActorLocation();
    Agent.AgentVelocity = FVector::ZeroVector;
    Agent.CurrentSpeed = 0.0f;
    Agent.bIsLeader = false;
    Agent.bIsPanicking = false;
    Agent.PanicLevel = 0.0f;
    Agent.AgentActor = NewMember;
    HerdAgents.Add(Agent);
    HerdMemberActors.Add(NewMember);
}

void UCrowd_HerdBehaviorComponent::RemoveHerdMember(AActor* Member)
{
    if (!IsValid(Member)) return;
    HerdMemberActors.Remove(Member);
    HerdAgents.RemoveAll([Member](const FCrowd_HerdAgent& A) { return A.AgentActor == Member; });
}

void UCrowd_HerdBehaviorComponent::DrawDebugHerdState()
{
#if ENABLE_DRAW_DEBUG
    UWorld* World = GetWorld();
    if (!World) return;

    FColor StateColor = FColor::Green;
    switch (HerdState)
    {
    case ECrowd_HerdState::Alert:      StateColor = FColor::Yellow; break;
    case ECrowd_HerdState::Migrating:  StateColor = FColor::Cyan;   break;
    case ECrowd_HerdState::Scattering: StateColor = FColor::Red;    break;
    case ECrowd_HerdState::Regrouping: StateColor = FColor::Orange; break;
    default: break;
    }

    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        DrawDebugSphere(World, Agent.AgentLocation, 60.0f, 8, StateColor, false, 0.15f);
        if (!Agent.AgentVelocity.IsNearlyZero())
        {
            DrawDebugLine(World, Agent.AgentLocation, Agent.AgentLocation + Agent.AgentVelocity * 0.3f, StateColor, false, 0.15f, 0, 3.0f);
        }
    }
#endif
}

void UCrowd_HerdBehaviorComponent::unreal_log_herd_state()
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdHerd] State: %d | Agents: %d | Density: %.2f | Threat: %.2f"),
        (int32)HerdState, HerdAgents.Num(), HerdDensityScore, ThreatLevel);
}

#include "StampedeController.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// AStampedeController — Implementation
// Agent #13 Crowd & Traffic Simulation
// ============================================================

AStampedeController::AStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // We manage our own tick rate
}

void AStampedeController::BeginPlay()
{
    Super::BeginPlay();
    CurrentPhase = ECrowd_StampedePhase::Inactive;
    AgentsSurvivedCount = 0;
}

void AStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Throttle to 20Hz
    TickAccumulator += DeltaTime;
    if (TickAccumulator < TickInterval)
    {
        return;
    }
    float SimDelta = TickAccumulator;
    TickAccumulator = 0.0f;

    if (CurrentPhase == ECrowd_StampedePhase::Inactive ||
        CurrentPhase == ECrowd_StampedePhase::Settled)
    {
        return;
    }

    PhaseElapsedTime += SimDelta;
    UpdatePhase(SimDelta);
    UpdateAgentFlocking(SimDelta);
    CheckPlayerTrampling();
}

// ── Public API ────────────────────────────────────────────────

void AStampedeController::TriggerStampede(FVector TriggerLocation,
                                           ECrowd_StampedeCause Cause,
                                           FVector FleeDirection)
{
    if (CurrentPhase != ECrowd_StampedePhase::Inactive &&
        CurrentPhase != ECrowd_StampedePhase::Settled)
    {
        return; // Already running
    }

    ActiveEvent.TriggerLocation = TriggerLocation;
    ActiveEvent.Cause = Cause;
    ActiveEvent.FleeDirection = FleeDirection.GetSafeNormal();
    ActiveEvent.EventTimestamp = GetWorld()->GetTimeSeconds();
    ActiveEvent.AgentsAffected = Agents.Num();

    TransitionToPhase(ECrowd_StampedePhase::Triggered);
    OnStampedeStarted.Broadcast(TriggerLocation, Cause);

    UE_LOG(LogTemp, Log, TEXT("StampedeController: Stampede triggered! Cause=%d, Agents=%d"),
           (int32)Cause, Agents.Num());
}

void AStampedeController::AddAgent(FVector SpawnLocation, bool bIsLeader)
{
    if (Agents.Num() >= MaxAgents)
    {
        return;
    }

    FCrowd_StampedeAgent NewAgent;
    NewAgent.Location = SpawnLocation;
    NewAgent.Velocity = FVector::ZeroVector;
    NewAgent.Speed = 0.0f;
    NewAgent.MaxSpeed = bIsLeader ? 1400.0f : 1200.0f;
    NewAgent.bIsLeader = bIsLeader;
    NewAgent.AgentIndex = Agents.Num();
    NewAgent.bIsAlive = true;

    Agents.Add(NewAgent);
}

void AStampedeController::ClearAllAgents()
{
    Agents.Empty();
    TransitionToPhase(ECrowd_StampedePhase::Inactive);
}

bool AStampedeController::IsStampedeActive() const
{
    return CurrentPhase != ECrowd_StampedePhase::Inactive &&
           CurrentPhase != ECrowd_StampedePhase::Settled;
}

int32 AStampedeController::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        if (Agent.bIsAlive) Count++;
    }
    return Count;
}

void AStampedeController::ForceSettle()
{
    TransitionToPhase(ECrowd_StampedePhase::Settled);
    int32 Survived = GetActiveAgentCount();
    OnStampedeEnded.Broadcast(Survived);
}

// ── Phase Management ──────────────────────────────────────────

void AStampedeController::TransitionToPhase(ECrowd_StampedePhase NewPhase)
{
    CurrentPhase = NewPhase;
    PhaseElapsedTime = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("StampedeController: Phase -> %d"), (int32)NewPhase);
}

void AStampedeController::UpdatePhase(float DeltaTime)
{
    switch (CurrentPhase)
    {
    case ECrowd_StampedePhase::Triggered:
        // Brief panic moment — 1 second before accelerating
        if (PhaseElapsedTime >= 1.0f)
        {
            TransitionToPhase(ECrowd_StampedePhase::Accelerating);
        }
        break;

    case ECrowd_StampedePhase::Accelerating:
        // Ramp up speed over 3 seconds
        if (PhaseElapsedTime >= 3.0f)
        {
            TransitionToPhase(ECrowd_StampedePhase::FullRun);
        }
        break;

    case ECrowd_StampedePhase::FullRun:
        // Full stampede — check if agents have dispersed far enough
        {
            float AvgDistFromTrigger = 0.0f;
            int32 AliveCount = 0;
            for (const FCrowd_StampedeAgent& Agent : Agents)
            {
                if (Agent.bIsAlive)
                {
                    AvgDistFromTrigger += FVector::Dist(Agent.Location,
                                                        ActiveEvent.TriggerLocation);
                    AliveCount++;
                }
            }
            if (AliveCount > 0)
            {
                AvgDistFromTrigger /= AliveCount;
            }

            // Disperse after 20 seconds OR agents are 8000 units away
            if (PhaseElapsedTime >= 20.0f || AvgDistFromTrigger >= 8000.0f)
            {
                TransitionToPhase(ECrowd_StampedePhase::Dispersing);
            }
        }
        break;

    case ECrowd_StampedePhase::Dispersing:
        // Slow down and spread out
        if (PhaseElapsedTime >= SettleTime)
        {
            int32 Survived = GetActiveAgentCount();
            AgentsSurvivedCount = Survived;
            TransitionToPhase(ECrowd_StampedePhase::Settled);
            OnStampedeEnded.Broadcast(Survived);
        }
        break;

    default:
        break;
    }
}

// ── Flocking Simulation ───────────────────────────────────────

void AStampedeController::UpdateAgentFlocking(float DeltaTime)
{
    // Compute speed multiplier based on phase
    float SpeedMult = 0.0f;
    switch (CurrentPhase)
    {
    case ECrowd_StampedePhase::Triggered:     SpeedMult = 0.3f; break;
    case ECrowd_StampedePhase::Accelerating:  SpeedMult = FMath::Lerp(0.3f, 1.0f,
                                                  PhaseElapsedTime / 3.0f); break;
    case ECrowd_StampedePhase::FullRun:       SpeedMult = 1.0f; break;
    case ECrowd_StampedePhase::Dispersing:    SpeedMult = FMath::Lerp(1.0f, 0.1f,
                                                  PhaseElapsedTime / SettleTime); break;
    default: SpeedMult = 0.0f; break;
    }

    for (FCrowd_StampedeAgent& Agent : Agents)
    {
        if (!Agent.bIsAlive) continue;

        // Flocking forces
        FVector Flocking = ComputeFlockingForce(Agent);

        // Flee direction bias (strongest force)
        FVector FleeForce = ActiveEvent.FleeDirection * StampedeAcceleration * 2.0f;

        // Combine forces
        FVector TotalForce = FleeForce + Flocking;
        FVector Acceleration = TotalForce.GetClampedToMaxSize(StampedeAcceleration);

        // Integrate velocity
        Agent.Velocity += Acceleration * DeltaTime;
        float TargetSpeed = Agent.MaxSpeed * SpeedMult;
        Agent.Velocity = Agent.Velocity.GetClampedToMaxSize(TargetSpeed);
        Agent.Speed = Agent.Velocity.Size();

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector AStampedeController::ComputeFlockingForce(const FCrowd_StampedeAgent& Agent) const
{
    FVector Separation = ComputeSeparation(Agent) * 3.0f;
    FVector Cohesion   = ComputeCohesion(Agent)   * 1.0f;
    FVector Alignment  = ComputeAlignment(Agent)  * 1.5f;
    return Separation + Cohesion + Alignment;
}

FVector AStampedeController::ComputeSeparation(const FCrowd_StampedeAgent& Agent) const
{
    FVector Force = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentIndex == Agent.AgentIndex || !Other.bIsAlive) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < AgentSeparationRadius && Dist > 1.0f)
        {
            FVector Away = (Agent.Location - Other.Location).GetSafeNormal();
            Force += Away * (AgentSeparationRadius / Dist);
            Count++;
        }
    }

    return Count > 0 ? Force / Count : FVector::ZeroVector;
}

FVector AStampedeController::ComputeCohesion(const FCrowd_StampedeAgent& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentIndex == Agent.AgentIndex || !Other.bIsAlive) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < CohesionRadius)
        {
            CenterOfMass += Other.Location;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;

    CenterOfMass /= Count;
    return (CenterOfMass - Agent.Location).GetSafeNormal() * 200.0f;
}

FVector AStampedeController::ComputeAlignment(const FCrowd_StampedeAgent& Agent) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (const FCrowd_StampedeAgent& Other : Agents)
    {
        if (Other.AgentIndex == Agent.AgentIndex || !Other.bIsAlive) continue;

        float Dist = FVector::Dist(Agent.Location, Other.Location);
        if (Dist < AlignmentRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;

    AvgVelocity /= Count;
    return (AvgVelocity - Agent.Velocity).GetSafeNormal() * 150.0f;
}

// ── Trampling ─────────────────────────────────────────────────

void AStampedeController::CheckPlayerTrampling()
{
    if (CurrentPhase != ECrowd_StampedePhase::FullRun &&
        CurrentPhase != ECrowd_StampedePhase::Accelerating)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (const FCrowd_StampedeAgent& Agent : Agents)
    {
        if (!Agent.bIsAlive) continue;

        float Dist = FVector::Dist(Agent.Location, PlayerLoc);
        if (Dist < TramplingRadius)
        {
            // Apply trampling damage
            UGameplayStatics::ApplyDamage(
                PlayerPawn,
                TramplingDamage,
                nullptr,
                this,
                nullptr
            );

            OnAgentTrampled.Broadcast(Agent.AgentIndex, Agent.Location);

            UE_LOG(LogTemp, Warning,
                   TEXT("StampedeController: Player TRAMPLED by agent %d! Damage=%.1f"),
                   Agent.AgentIndex, TramplingDamage);

            // Only one trample per tick to avoid stacking
            break;
        }
    }
}

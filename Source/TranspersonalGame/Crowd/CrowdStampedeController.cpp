// CrowdStampedeController.cpp — Agent #13 Crowd & Traffic Simulation
// Stampede trigger system: mass panic events for herbivore herds fleeing predators
// Transpersonal Game Studio — Prehistoric Survival Game

#include "Crowd/CrowdStampedeController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ACrowdStampedeController::ACrowdStampedeController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;  // 20Hz tick — sufficient for crowd sim

    MaxHerdSize = 40;
    HerdSpawnRadius = 3000.0f;
    HerdSpecies = ECrowd_AgentType::DinosaurHerbivore;
    StampedeSpeed = 1200.0f;
    CalmSpeed = 200.0f;
    PlayerTrampleDamage = 35.0f;
    PanicPropagationRadius = 600.0f;
    CurrentHerdState = ECrowd_StampedeState::Calm;
    ActiveAgentCount = 0;
    AveragePanicLevel = 0.0f;
    StampedeTimer = 0.0f;
    StampedeDuration = 0.0f;
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ACrowdStampedeController::BeginPlay()
{
    Super::BeginPlay();

    // Auto-spawn herd at actor location on play
    if (MaxHerdSize > 0)
    {
        SpawnHerd(GetActorLocation(), MaxHerdSize);
    }
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ACrowdStampedeController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HerdAgents.Num() == 0) return;

    // Countdown stampede timer
    if (CurrentHerdState == ECrowd_StampedeState::Stampeding ||
        CurrentHerdState == ECrowd_StampedeState::Panic)
    {
        StampedeTimer += DeltaTime;
        if (StampedeTimer >= StampedeDuration)
        {
            CalmHerd();
        }
    }

    UpdateAgentPositions(DeltaTime);
    UpdateHerdState();

    // Compute average panic
    float TotalPanic = 0.0f;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        TotalPanic += Agent.PanicLevel;
    }
    AveragePanicLevel = HerdAgents.Num() > 0 ? TotalPanic / HerdAgents.Num() : 0.0f;
    ActiveAgentCount = HerdAgents.Num();

    // Check player trample
    UWorld* World = GetWorld();
    if (World && CurrentHerdState == ECrowd_StampedeState::Stampeding)
    {
        ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
        if (PlayerChar)
        {
            FVector PlayerLoc = PlayerChar->GetActorLocation();
            if (IsPlayerInStampedePath(PlayerLoc, 350.0f))
            {
                // Find closest stampeding agent
                for (const FCrowd_HerdAgent& Agent : HerdAgents)
                {
                    if (Agent.State == ECrowd_StampedeState::Stampeding)
                    {
                        float Dist = FVector::Dist(Agent.Location, PlayerLoc);
                        if (Dist < 350.0f)
                        {
                            OnPlayerTrampled(PlayerTrampleDamage * Agent.PanicLevel, Agent.Velocity);
                            break;
                        }
                    }
                }
            }
        }
    }
}

// ─── SpawnHerd ────────────────────────────────────────────────────────────────

void ACrowdStampedeController::SpawnHerd(FVector CenterLocation, int32 Count)
{
    HerdAgents.Empty();
    Count = FMath::Clamp(Count, 1, MaxHerdSize);

    FRandomStream Rand(42);

    for (int32 i = 0; i < Count; ++i)
    {
        FCrowd_HerdAgent Agent;
        Agent.AgentID = i;

        // Scatter agents in a circle around center
        float Angle = (float)i / (float)Count * 2.0f * PI;
        float Radius = Rand.FRandRange(200.0f, HerdSpawnRadius);
        Agent.Location = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        Agent.Velocity = FVector::ZeroVector;
        Agent.State = ECrowd_StampedeState::Calm;
        Agent.PanicLevel = 0.0f;
        Agent.Mass = Rand.FRandRange(300.0f, 800.0f);
        Agent.bIsLeader = (i == 0);  // First agent is herd leader

        HerdAgents.Add(Agent);
    }

    CurrentHerdState = ECrowd_StampedeState::Calm;
    ActiveAgentCount = HerdAgents.Num();

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Spawned herd of %d agents at %s"),
        Count, *CenterLocation.ToString());
}

// ─── TriggerStampede ─────────────────────────────────────────────────────────

void ACrowdStampedeController::TriggerStampede(const FCrowd_StampedeEvent& Event)
{
    if (HerdAgents.Num() == 0) return;

    CurrentFleeDirection = ComputeFleeDirection(Event.TriggerLocation);
    StampedeDuration = Event.Duration;
    StampedeTimer = 0.0f;

    // Agents within trigger radius immediately panic
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float Dist = FVector::Dist(Agent.Location, Event.TriggerLocation);
        if (Dist <= Event.TriggerRadius)
        {
            float DistanceFactor = 1.0f - (Dist / Event.TriggerRadius);
            Agent.PanicLevel = FMath::Clamp(Event.PanicIntensity * DistanceFactor, 0.0f, 1.0f);
            Agent.State = Agent.PanicLevel > 0.5f
                ? ECrowd_StampedeState::Stampeding
                : ECrowd_StampedeState::Panic;
        }
    }

    // Leader always panics first
    if (HerdAgents.Num() > 0)
    {
        HerdAgents[0].PanicLevel = 1.0f;
        HerdAgents[0].State = ECrowd_StampedeState::Stampeding;
    }

    CurrentHerdState = ECrowd_StampedeState::Stampeding;

    OnStampedeStarted(CurrentFleeDirection, HerdAgents.Num());

    UE_LOG(LogTemp, Warning, TEXT("CrowdStampedeController: STAMPEDE triggered! Trigger=%s, Direction=%s, %d agents"),
        *UEnum::GetValueAsString(Event.TriggerType),
        *CurrentFleeDirection.ToString(),
        HerdAgents.Num());
}

// ─── CalmHerd ────────────────────────────────────────────────────────────────

void ACrowdStampedeController::CalmHerd()
{
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - 0.1f);
        if (Agent.PanicLevel <= 0.1f)
        {
            Agent.State = ECrowd_StampedeState::Calm;
            Agent.Velocity = FVector::ZeroVector;
        }
        else
        {
            Agent.State = ECrowd_StampedeState::Dispersing;
        }
    }

    CurrentHerdState = ECrowd_StampedeState::Calm;
    CurrentFleeDirection = FVector::ZeroVector;
    StampedeTimer = 0.0f;

    OnStampedeEnded();

    UE_LOG(LogTemp, Log, TEXT("CrowdStampedeController: Herd calmed."));
}

// ─── GetPlayerDangerLevel ────────────────────────────────────────────────────

float ACrowdStampedeController::GetPlayerDangerLevel(FVector PlayerLocation) const
{
    if (CurrentHerdState != ECrowd_StampedeState::Stampeding) return 0.0f;

    float MaxDanger = 0.0f;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.State != ECrowd_StampedeState::Stampeding) continue;

        float Dist = FVector::Dist(Agent.Location, PlayerLocation);
        float Danger = FMath::Clamp(1.0f - (Dist / 2000.0f), 0.0f, 1.0f) * Agent.PanicLevel;
        MaxDanger = FMath::Max(MaxDanger, Danger);
    }
    return MaxDanger;
}

// ─── IsPlayerInStampedePath ──────────────────────────────────────────────────

bool ACrowdStampedeController::IsPlayerInStampedePath(FVector PlayerLocation, float CheckRadius) const
{
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.State != ECrowd_StampedeState::Stampeding) continue;
        if (FVector::Dist(Agent.Location, PlayerLocation) < CheckRadius)
        {
            return true;
        }
    }
    return false;
}

// ─── GetNearbyAgents ─────────────────────────────────────────────────────────

TArray<FCrowd_HerdAgent> ACrowdStampedeController::GetNearbyAgents(FVector Location, float Radius) const
{
    TArray<FCrowd_HerdAgent> Result;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (FVector::Dist(Agent.Location, Location) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

// ─── Blueprint Native Events ──────────────────────────────────────────────────

void ACrowdStampedeController::OnStampedeStarted_Implementation(FVector FleeDirection, int32 AgentCount)
{
    UE_LOG(LogTemp, Warning, TEXT("OnStampedeStarted: %d agents fleeing toward %s"),
        AgentCount, *FleeDirection.ToString());
}

void ACrowdStampedeController::OnStampedeEnded_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("OnStampedeEnded: Herd has calmed."));
}

void ACrowdStampedeController::OnPlayerTrampled_Implementation(float DamageDealt, FVector ImpactVelocity)
{
    UE_LOG(LogTemp, Warning, TEXT("OnPlayerTrampled: Damage=%.1f, Velocity=%s"),
        DamageDealt, *ImpactVelocity.ToString());
}

// ─── Private Helpers ──────────────────────────────────────────────────────────

void ACrowdStampedeController::UpdateAgentPositions(float DeltaTime)
{
    for (int32 i = 0; i < HerdAgents.Num(); ++i)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        if (Agent.State == ECrowd_StampedeState::Calm) continue;

        // Flocking forces (Reynolds boids)
        FVector Separation = ComputeSeparationForce(i) * 1.5f;
        FVector Alignment  = ComputeAlignmentForce(i) * 1.0f;
        FVector Cohesion   = ComputeCohesionForce(i) * 0.8f;

        // Flee direction weighted by panic
        FVector FleeForce = CurrentFleeDirection * StampedeSpeed * Agent.PanicLevel;

        // Combine forces
        FVector DesiredVelocity = FleeForce + Separation + Alignment + Cohesion;

        // Clamp to max speed
        float MaxSpeed = FMath::Lerp(CalmSpeed, StampedeSpeed, Agent.PanicLevel);
        if (DesiredVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
        {
            DesiredVelocity = DesiredVelocity.GetSafeNormal() * MaxSpeed;
        }

        // Smooth velocity
        Agent.Velocity = FMath::VInterpTo(Agent.Velocity, DesiredVelocity, DeltaTime, 3.0f);
        Agent.Location += Agent.Velocity * DeltaTime;

        // Propagate panic to nearby calm agents
        if (Agent.PanicLevel > 0.5f)
        {
            PropagatePanic(i);
        }
    }
}

void ACrowdStampedeController::PropagatePanic(int32 AgentIndex)
{
    const FCrowd_HerdAgent& Source = HerdAgents[AgentIndex];

    for (FCrowd_HerdAgent& Other : HerdAgents)
    {
        if (Other.AgentID == Source.AgentID) continue;
        if (Other.PanicLevel >= Source.PanicLevel) continue;

        float Dist = FVector::Dist(Source.Location, Other.Location);
        if (Dist < PanicPropagationRadius)
        {
            float PropagationStrength = (1.0f - Dist / PanicPropagationRadius) * 0.3f;
            Other.PanicLevel = FMath::Min(1.0f, Other.PanicLevel + PropagationStrength);

            if (Other.PanicLevel > 0.3f && Other.State == ECrowd_StampedeState::Calm)
            {
                Other.State = ECrowd_StampedeState::Alert;
            }
            if (Other.PanicLevel > 0.6f)
            {
                Other.State = ECrowd_StampedeState::Stampeding;
            }
        }
    }
}

void ACrowdStampedeController::UpdateHerdState()
{
    int32 CalmCount = 0, PanicCount = 0, StampedeCount = 0;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        switch (Agent.State)
        {
            case ECrowd_StampedeState::Calm:       CalmCount++;     break;
            case ECrowd_StampedeState::Stampeding: StampedeCount++; break;
            default:                               PanicCount++;    break;
        }
    }

    int32 Total = HerdAgents.Num();
    if (Total == 0) return;

    if (StampedeCount > Total / 2)
        CurrentHerdState = ECrowd_StampedeState::Stampeding;
    else if (PanicCount + StampedeCount > Total / 3)
        CurrentHerdState = ECrowd_StampedeState::Panic;
    else if (CalmCount == Total)
        CurrentHerdState = ECrowd_StampedeState::Calm;
}

FVector ACrowdStampedeController::ComputeFleeDirection(FVector ThreatLocation) const
{
    FVector HerdCenter = FVector::ZeroVector;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        HerdCenter += Agent.Location;
    }
    if (HerdAgents.Num() > 0) HerdCenter /= HerdAgents.Num();

    FVector FleeDir = (HerdCenter - ThreatLocation).GetSafeNormal();
    return FleeDir;
}

FVector ACrowdStampedeController::ComputeSeparationForce(int32 AgentIndex) const
{
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];
    FVector Force = FVector::ZeroVector;
    float MinSeparation = 200.0f;

    for (const FCrowd_HerdAgent& Other : HerdAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        FVector Delta = Agent.Location - Other.Location;
        float Dist = Delta.Size();
        if (Dist < MinSeparation && Dist > 0.0f)
        {
            Force += Delta.GetSafeNormal() * (MinSeparation - Dist);
        }
    }
    return Force;
}

FVector ACrowdStampedeController::ComputeAlignmentForce(int32 AgentIndex) const
{
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    float NeighborRadius = 800.0f;

    for (const FCrowd_HerdAgent& Other : HerdAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (FVector::Dist(Agent.Location, Other.Location) < NeighborRadius)
        {
            AvgVelocity += Other.Velocity;
            Count++;
        }
    }

    if (Count > 0) AvgVelocity /= Count;
    return AvgVelocity;
}

FVector ACrowdStampedeController::ComputeCohesionForce(int32 AgentIndex) const
{
    const FCrowd_HerdAgent& Agent = HerdAgents[AgentIndex];
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    float NeighborRadius = 1200.0f;

    for (const FCrowd_HerdAgent& Other : HerdAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (FVector::Dist(Agent.Location, Other.Location) < NeighborRadius)
        {
            Center += Other.Location;
            Count++;
        }
    }

    if (Count > 0)
    {
        Center /= Count;
        return (Center - Agent.Location).GetSafeNormal() * 100.0f;
    }
    return FVector::ZeroVector;
}

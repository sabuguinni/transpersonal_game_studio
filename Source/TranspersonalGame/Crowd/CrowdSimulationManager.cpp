#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

// ============================================================
// ACrowdSimulationManager — Implementation
// Agent #13 Crowd & Traffic Simulation
// Boids-based flocking for prehistoric herds, packs, tribes
// ============================================================

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz simulation

    MaxAgents = 200;
    SimulationTickRate = 0.05f;
    bEnableFlocking = true;
    bEnableStampede = true;
    LODDistanceFar = 5000.0f;
    LODDistanceMid = 2500.0f;
    NextAgentID = 0;
    CurrentLODLevel = 0;
    SimAccumulator = 0.0f;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: BeginPlay — %d agents ready"), Agents.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SimAccumulator += DeltaTime;
    if (SimAccumulator >= SimulationTickRate)
    {
        SimAccumulator = 0.0f;
        UpdateAgentStates(SimulationTickRate);
    }
}

// ---- Spawn Herd ----

void ACrowdSimulationManager::SpawnHerd(ECrowd_AgentType AgentType, FVector CenterLocation, int32 Count)
{
    if (!GetWorld()) return;

    int32 ToSpawn = FMath::Min(Count, MaxAgents - Agents.Num());
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawning herd of %d agents (type %d)"), ToSpawn, (int32)AgentType);

    for (int32 i = 0; i < ToSpawn; i++)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = NextAgentID++;
        NewAgent.AgentType = AgentType;

        // Scatter agents in a circle around center
        float Angle = (float)i / (float)ToSpawn * 2.0f * PI;
        float Radius = FMath::RandRange(100.0f, HerdConfig.CohesionRadius * 0.5f);
        NewAgent.Location = CenterLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );

        // Initial state by type
        switch (AgentType)
        {
        case ECrowd_AgentType::DinosaurHerbivore:
            NewAgent.State = ECrowd_AgentState::Grazing;
            NewAgent.Speed = FMath::RandRange(150.0f, 300.0f);
            break;
        case ECrowd_AgentType::DinosaurCarnivore:
        case ECrowd_AgentType::DinosaurPack:
            NewAgent.State = ECrowd_AgentState::Wandering;
            NewAgent.Speed = FMath::RandRange(400.0f, 700.0f);
            break;
        case ECrowd_AgentType::HumanPrimitive:
            NewAgent.State = ECrowd_AgentState::Idle;
            NewAgent.Speed = FMath::RandRange(100.0f, 200.0f);
            break;
        case ECrowd_AgentType::MigrationHerd:
            NewAgent.State = ECrowd_AgentState::Wandering;
            NewAgent.Speed = FMath::RandRange(200.0f, 350.0f);
            break;
        default:
            NewAgent.Speed = 200.0f;
            break;
        }

        NewAgent.Velocity = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal() * NewAgent.Speed;
        NewAgent.FearLevel = 0.0f;

        Agents.Add(NewAgent);
    }
}

// ---- Stampede Trigger ----

void ACrowdSimulationManager::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    if (!bEnableStampede) return;

    int32 AffectedCount = 0;
    for (FCrowd_AgentData& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            float FearBoost = FMath::Clamp(1.0f - (Dist / ThreatRadius), 0.0f, 1.0f);
            Agent.FearLevel = FMath::Clamp(Agent.FearLevel + FearBoost, 0.0f, 1.0f);

            if (Agent.FearLevel >= HerdConfig.StampedeThreshold)
            {
                Agent.State = ECrowd_AgentState::Stampeding;
                // Flee direction = away from threat
                FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
                Agent.Velocity = FleeDir * Agent.Speed * 2.5f; // Stampede = 2.5x normal speed
                AffectedCount++;
            }
            else if (Agent.FearLevel >= HerdConfig.FleeThreshold)
            {
                Agent.State = ECrowd_AgentState::Fleeing;
                FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
                Agent.Velocity = FleeDir * Agent.Speed * 1.5f;
                AffectedCount++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Stampede triggered — %d agents affected"), AffectedCount);
}

// ---- Flee Trigger ----

void ACrowdSimulationManager::TriggerFlee(FVector ThreatLocation, float FearAmount)
{
    for (FCrowd_AgentData& Agent : Agents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        float FalloffFear = FearAmount * FMath::Clamp(1.0f - (Dist / 3000.0f), 0.0f, 1.0f);
        Agent.FearLevel = FMath::Clamp(Agent.FearLevel + FalloffFear, 0.0f, 1.0f);

        if (Agent.FearLevel >= HerdConfig.FleeThreshold && Agent.State != ECrowd_AgentState::Stampeding)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * Agent.Speed * 1.5f;
        }
    }
}

// ---- Update Agent States ----

void ACrowdSimulationManager::UpdateAgentStates(float DeltaTime)
{
    for (int32 i = 0; i < Agents.Num(); i++)
    {
        TickAgent(Agents[i], DeltaTime);
    }
}

void ACrowdSimulationManager::TickAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Fear decay
    Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - DeltaTime * 0.1f);

    // State transitions
    if (Agent.State == ECrowd_AgentState::Stampeding || Agent.State == ECrowd_AgentState::Fleeing)
    {
        if (Agent.FearLevel < HerdConfig.FleeThreshold * 0.5f)
        {
            Agent.State = ECrowd_AgentState::Wandering;
        }
    }

    // Apply flocking forces
    if (bEnableFlocking && (Agent.State == ECrowd_AgentState::Grazing || Agent.State == ECrowd_AgentState::Wandering))
    {
        int32 Idx = Agents.IndexOfByKey(Agent);
        if (Idx != INDEX_NONE)
        {
            FVector Cohesion = ComputeCohesion(Idx) * 0.3f;
            FVector Separation = ComputeSeparation(Idx) * 1.5f;
            FVector Alignment = ComputeAlignment(Idx) * 0.5f;

            Agent.Velocity += (Cohesion + Separation + Alignment) * DeltaTime;
            Agent.Velocity = ClampVelocity(Agent.Velocity, Agent.Speed);
        }
    }

    // Move agent
    Agent.Location += Agent.Velocity * DeltaTime;

    // Keep on ground (Z = 100 placeholder)
    Agent.Location.Z = 100.0f;
}

// ---- Boids: Cohesion ----

FVector ACrowdSimulationManager::ComputeCohesion(int32 AgentIndex) const
{
    if (!Agents.IsValidIndex(AgentIndex)) return FVector::ZeroVector;

    const FCrowd_AgentData& Self = Agents[AgentIndex];
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 i = 0; i < Agents.Num(); i++)
    {
        if (i == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[i].Location);
        if (Dist < HerdConfig.CohesionRadius)
        {
            Center += Agents[i].Location;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    Center /= (float)Count;
    return (Center - Self.Location).GetSafeNormal() * 100.0f;
}

// ---- Boids: Separation ----

FVector ACrowdSimulationManager::ComputeSeparation(int32 AgentIndex) const
{
    if (!Agents.IsValidIndex(AgentIndex)) return FVector::ZeroVector;

    const FCrowd_AgentData& Self = Agents[AgentIndex];
    FVector Repulsion = FVector::ZeroVector;

    for (int32 i = 0; i < Agents.Num(); i++)
    {
        if (i == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[i].Location);
        if (Dist < HerdConfig.SeparationRadius && Dist > 0.0f)
        {
            FVector Away = (Self.Location - Agents[i].Location).GetSafeNormal();
            Repulsion += Away * (HerdConfig.SeparationRadius - Dist);
        }
    }

    return Repulsion;
}

// ---- Boids: Alignment ----

FVector ACrowdSimulationManager::ComputeAlignment(int32 AgentIndex) const
{
    if (!Agents.IsValidIndex(AgentIndex)) return FVector::ZeroVector;

    const FCrowd_AgentData& Self = Agents[AgentIndex];
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;

    for (int32 i = 0; i < Agents.Num(); i++)
    {
        if (i == AgentIndex) continue;
        float Dist = FVector::Dist(Self.Location, Agents[i].Location);
        if (Dist < HerdConfig.AlignmentRadius)
        {
            AvgVelocity += Agents[i].Velocity;
            Count++;
        }
    }

    if (Count == 0) return FVector::ZeroVector;
    AvgVelocity /= (float)Count;
    return (AvgVelocity - Self.Velocity).GetSafeNormal() * 50.0f;
}

// ---- LOD ----

void ACrowdSimulationManager::SetLODLevel(int32 Level)
{
    CurrentLODLevel = FMath::Clamp(Level, 0, 2);
    switch (CurrentLODLevel)
    {
    case 0: // Full simulation
        SimulationTickRate = 0.05f;
        bEnableFlocking = true;
        break;
    case 1: // Mid LOD — reduce tick rate
        SimulationTickRate = 0.1f;
        bEnableFlocking = true;
        break;
    case 2: // Far LOD — minimal simulation
        SimulationTickRate = 0.25f;
        bEnableFlocking = false;
        break;
    }
    PrimaryActorTick.TickInterval = SimulationTickRate;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: LOD level set to %d (tick %.2fs)"), CurrentLODLevel, SimulationTickRate);
}

// ---- Utility ----

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return Agents.Num();
}

void ACrowdSimulationManager::ClearAllAgents()
{
    Agents.Empty();
    NextAgentID = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: All agents cleared"));
}

FVector ACrowdSimulationManager::ClampVelocity(const FVector& Vel, float MaxSpeed) const
{
    float Len = Vel.Size();
    if (Len > MaxSpeed)
    {
        return Vel.GetSafeNormal() * MaxSpeed;
    }
    return Vel;
}

void ACrowdSimulationManager::SpawnAgentMesh(const FCrowd_AgentData& Agent)
{
    // Placeholder — in full implementation, spawn instanced static mesh
    // or use Mass Entity for large-scale simulation
    UE_LOG(LogTemp, Verbose, TEXT("CrowdSimulationManager: SpawnAgentMesh for ID %d at %s"),
        Agent.AgentID, *Agent.Location.ToString());
}

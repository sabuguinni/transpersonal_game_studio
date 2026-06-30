#include "CrowdSimulationManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = AgentUpdateInterval;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Spawn default herds on begin play
    SpawnHerd(ECrowd_HerdSpecies::Gallimimus, 30, GetActorLocation() + FVector(1000.0f, 0.0f, 0.0f));
    SpawnHerd(ECrowd_HerdSpecies::Parasaurolophus, 15, GetActorLocation() + FVector(-1500.0f, 800.0f, 0.0f));
    SpawnHerd(ECrowd_HerdSpecies::Triceratops, 10, GetActorLocation() + FVector(500.0f, -2000.0f, 0.0f));

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: BeginPlay — %d herds spawned, %d total agents"), Herds.Num(), Agents.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeSinceLastUpdate += DeltaTime;
    if (TimeSinceLastUpdate >= AgentUpdateInterval)
    {
        UpdateAgents(TimeSinceLastUpdate);
        UpdateHerdCenters();
        TimeSinceLastUpdate = 0.0f;
    }
}

void ACrowdSimulationManager::SpawnHerd(ECrowd_HerdSpecies Species, int32 Count, FVector SpawnCenter)
{
    if (Agents.Num() + Count > MaxAgents)
    {
        Count = MaxAgents - Agents.Num();
    }
    if (Count <= 0) return;

    FCrowd_HerdGroup NewHerd;
    NewHerd.HerdID = NextHerdID++;
    NewHerd.Species = Species;
    NewHerd.CenterOfMass = SpawnCenter;

    for (int32 i = 0; i < Count; i++)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = NextAgentID++;
        NewAgent.Species = Species;
        NewAgent.State = ECrowd_AgentState::Wandering;

        // Scatter agents around spawn center
        float Angle = FMath::RandRange(0.0f, 360.0f);
        float Radius = FMath::RandRange(50.0f, 400.0f);
        NewAgent.Location = SpawnCenter + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.0f
        );

        NewAgent.Velocity = FVector(
            FMath::RandRange(-WanderSpeed, WanderSpeed),
            FMath::RandRange(-WanderSpeed, WanderSpeed),
            0.0f
        ).GetClampedToMaxSize(WanderSpeed);

        NewAgent.FlockRadius = FMath::RandRange(200.0f, 400.0f);
        NewAgent.FearLevel = 0.0f;
        NewAgent.bIsLeader = (i == 0); // First agent is leader

        if (i == 0)
        {
            NewHerd.LeaderAgentID = NewAgent.AgentID;
        }

        NewHerd.AgentIDs.Add(NewAgent.AgentID);
        Agents.Add(NewAgent);
    }

    Herds.Add(NewHerd);
    ActiveAgentCount = Agents.Num();

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned herd %d — species %d, %d agents at (%.0f, %.0f, %.0f)"),
        NewHerd.HerdID, (int32)Species, Count, SpawnCenter.X, SpawnCenter.Y, SpawnCenter.Z);
}

void ACrowdSimulationManager::TriggerStampede(int32 HerdID, FVector ThreatLocation)
{
    for (FCrowd_HerdGroup& Herd : Herds)
    {
        if (Herd.HerdID == HerdID)
        {
            Herd.bIsStampeding = true;
            // Stampede away from threat
            Herd.StampedeDirection = (Herd.CenterOfMass - ThreatLocation).GetSafeNormal();

            // Update all agents in this herd
            for (FCrowd_AgentData& Agent : Agents)
            {
                if (Herd.AgentIDs.Contains(Agent.AgentID))
                {
                    Agent.State = ECrowd_AgentState::Stampeding;
                    Agent.FearLevel = 1.0f;
                    Agent.Velocity = Herd.StampedeDirection * StampedeSpeed;
                }
            }

            UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: STAMPEDE triggered on herd %d! Direction: (%.1f, %.1f)"),
                HerdID, Herd.StampedeDirection.X, Herd.StampedeDirection.Y);
            break;
        }
    }
}

void ACrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    int32 AffectedAgents = 0;
    for (FCrowd_AgentData& Agent : Agents)
    {
        float Distance = FVector::Dist(Agent.Location, ThreatLocation);
        if (Distance <= ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            float FearFactor = 1.0f - (Distance / ThreatRadius);
            Agent.FearLevel = FMath::Max(Agent.FearLevel, FearFactor);

            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * FleeSpeed * FearFactor;
            AffectedAgents++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: FleeResponse — %d agents fleeing from (%.0f, %.0f)"),
        AffectedAgents, ThreatLocation.X, ThreatLocation.Y);
}

void ACrowdSimulationManager::UpdateAgentLOD(FCrowd_AgentData& Agent, float DistanceToPlayer)
{
    // LOD 0 (close) — full simulation
    // LOD 1 (medium) — reduced update rate
    // LOD 2 (far) — position only, no flocking
    // LOD 3 (very far) — culled from simulation

    if (DistanceToPlayer > LODDistanceFar)
    {
        // Cull agent from active simulation — just maintain position
        Agent.Velocity = FVector::ZeroVector;
    }
    else if (DistanceToPlayer > LODDistanceMedium)
    {
        // Simplified movement — follow herd center only
        Agent.FlockRadius = 0.0f; // Disable flocking
    }
    else if (DistanceToPlayer > LODDistanceClose)
    {
        // Reduced flocking precision
        Agent.FlockRadius = FMath::Min(Agent.FlockRadius, 200.0f);
    }
    // else: full simulation at close range
}

FVector ACrowdSimulationManager::ComputeFlockingVelocity(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors)
{
    if (Neighbors.Num() == 0)
    {
        return Agent.Velocity;
    }

    FVector Separation = GetSeparationForce(Agent, Neighbors) * SeparationWeight;
    FVector Cohesion = GetCohesionForce(Agent, Neighbors) * CohesionWeight;
    FVector Alignment = GetAlignmentForce(Agent, Neighbors) * AlignmentWeight;

    FVector NewVelocity = Agent.Velocity + Separation + Cohesion + Alignment;

    // Clamp to appropriate speed based on state
    float MaxSpeed = WanderSpeed;
    if (Agent.State == ECrowd_AgentState::Fleeing) MaxSpeed = FleeSpeed;
    else if (Agent.State == ECrowd_AgentState::Stampeding) MaxSpeed = StampedeSpeed;

    return NewVelocity.GetClampedToMaxSize(MaxSpeed);
}

TArray<FCrowd_AgentData> ACrowdSimulationManager::GetNeighbors(const FCrowd_AgentData& Agent, float Radius)
{
    TArray<FCrowd_AgentData> Neighbors;
    for (const FCrowd_AgentData& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (Other.Species != Agent.Species) continue; // Only flock with same species
        if (FVector::Dist(Agent.Location, Other.Location) <= Radius)
        {
            Neighbors.Add(Other);
        }
    }
    return Neighbors;
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

void ACrowdSimulationManager::ClearAllAgents()
{
    Agents.Empty();
    Herds.Empty();
    ActiveAgentCount = 0;
    NextAgentID = 0;
    NextHerdID = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: All agents cleared"));
}

void ACrowdSimulationManager::UpdateAgents(float DeltaTime)
{
    APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
    FVector PlayerLocation = FVector::ZeroVector;
    if (PC && PC->GetPawn())
    {
        PlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    for (FCrowd_AgentData& Agent : Agents)
    {
        float DistToPlayer = FVector::Dist(Agent.Location, PlayerLocation);
        UpdateAgentLOD(Agent, DistToPlayer);

        // Skip full simulation for far agents
        if (DistToPlayer > LODDistanceFar) continue;

        // Gradually reduce fear
        if (Agent.FearLevel > 0.0f)
        {
            Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - DeltaTime * 0.1f);
            if (Agent.FearLevel < 0.1f && Agent.State == ECrowd_AgentState::Fleeing)
            {
                Agent.State = ECrowd_AgentState::Wandering;
            }
        }

        // Compute flocking for close/medium agents
        if (DistToPlayer <= LODDistanceMedium && Agent.FlockRadius > 0.0f)
        {
            TArray<FCrowd_AgentData> Neighbors = GetNeighbors(Agent, Agent.FlockRadius);
            Agent.Velocity = ComputeFlockingVelocity(Agent, Neighbors);
        }

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

void ACrowdSimulationManager::UpdateHerdCenters()
{
    for (FCrowd_HerdGroup& Herd : Herds)
    {
        if (Herd.AgentIDs.Num() == 0) continue;

        FVector Sum = FVector::ZeroVector;
        int32 Count = 0;

        for (const FCrowd_AgentData& Agent : Agents)
        {
            if (Herd.AgentIDs.Contains(Agent.AgentID))
            {
                Sum += Agent.Location;
                Count++;
            }
        }

        if (Count > 0)
        {
            Herd.CenterOfMass = Sum / Count;
        }

        // Check if stampede should end
        if (Herd.bIsStampeding)
        {
            bool bAnyStampeding = false;
            for (const FCrowd_AgentData& Agent : Agents)
            {
                if (Herd.AgentIDs.Contains(Agent.AgentID) && Agent.State == ECrowd_AgentState::Stampeding)
                {
                    bAnyStampeding = true;
                    break;
                }
            }
            if (!bAnyStampeding)
            {
                Herd.bIsStampeding = false;
                Herd.StampedeDirection = FVector::ZeroVector;
            }
        }
    }
}

FVector ACrowdSimulationManager::GetSeparationForce(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors) const
{
    FVector Force = FVector::ZeroVector;
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        FVector Diff = Agent.Location - Neighbor.Location;
        float Dist = Diff.Size();
        if (Dist > 0.0f && Dist < 150.0f)
        {
            Force += Diff.GetSafeNormal() / Dist;
        }
    }
    return Force;
}

FVector ACrowdSimulationManager::GetCohesionForce(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors) const
{
    FVector Center = FVector::ZeroVector;
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        Center += Neighbor.Location;
    }
    Center /= Neighbors.Num();
    return (Center - Agent.Location).GetSafeNormal();
}

FVector ACrowdSimulationManager::GetAlignmentForce(const FCrowd_AgentData& Agent, const TArray<FCrowd_AgentData>& Neighbors) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    for (const FCrowd_AgentData& Neighbor : Neighbors)
    {
        AvgVelocity += Neighbor.Velocity;
    }
    AvgVelocity /= Neighbors.Num();
    return (AvgVelocity - Agent.Velocity).GetSafeNormal();
}

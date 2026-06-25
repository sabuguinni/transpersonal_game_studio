#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = TickIntervalSeconds;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    // Auto-initialize with default population on play
    InitializeCrowd(80, 20, 10);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AccumulatedTime += DeltaTime;
    StepSimulation(DeltaTime);
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void ACrowdSimulationManager::InitializeCrowd(int32 NumHerbivores, int32 NumCarnivores, int32 NumHumans)
{
    Agents.Empty();
    HerdGroups.Empty();
    NextAgentID = 0;
    NextGroupID = 0;

    const int32 Total = FMath::Min(NumHerbivores + NumCarnivores + NumHumans, MaxAgents);

    // Create herd groups
    auto MakeGroup = [&](ECrowd_AgentType Type) -> int32
    {
        FCrowd_HerdGroup G;
        G.GroupID = NextGroupID++;
        G.GroupType = Type;
        G.CentroidLocation = FVector(
            FMath::RandRange(-SimulationRadius * 0.5f, SimulationRadius * 0.5f),
            FMath::RandRange(-SimulationRadius * 0.5f, SimulationRadius * 0.5f),
            0.f
        );
        G.MigrationTarget = G.CentroidLocation;
        HerdGroups.Add(G);
        return G.GroupID;
    };

    int32 HerbGroup = MakeGroup(ECrowd_AgentType::HerbivorePrey);
    int32 CarnGroup = MakeGroup(ECrowd_AgentType::CarnivoreHunter);
    int32 HumanGroup = MakeGroup(ECrowd_AgentType::HumanTribe);

    auto SpawnAgents = [&](int32 Count, ECrowd_AgentType Type, int32 GroupID)
    {
        FCrowd_HerdGroup* Group = HerdGroups.FindByPredicate([GroupID](const FCrowd_HerdGroup& G){ return G.GroupID == GroupID; });
        FVector Base = Group ? Group->CentroidLocation : FVector::ZeroVector;

        for (int32 i = 0; i < Count && Agents.Num() < MaxAgents; ++i)
        {
            FCrowd_AgentData A;
            A.AgentID = NextAgentID++;
            A.AgentType = Type;
            A.HerdGroupID = GroupID;
            A.State = ECrowd_AgentState::Foraging;
            A.Health = 100.f;
            A.Location = Base + FVector(
                FMath::RandRange(-500.f, 500.f),
                FMath::RandRange(-500.f, 500.f),
                0.f
            );
            A.Velocity = FVector(
                FMath::RandRange(-50.f, 50.f),
                FMath::RandRange(-50.f, 50.f),
                0.f
            );
            A.FleeRadius = (Type == ECrowd_AgentType::CarnivoreHunter) ? 2000.f : 1500.f;
            Agents.Add(A);

            if (Group)
            {
                Group->MemberAgentIDs.Add(A.AgentID);
            }
        }
    };

    SpawnAgents(NumHerbivores, ECrowd_AgentType::HerbivorePrey, HerbGroup);
    SpawnAgents(NumCarnivores, ECrowd_AgentType::CarnivoreHunter, CarnGroup);
    SpawnAgents(NumHumans, ECrowd_AgentType::HumanTribe, HumanGroup);

    ActiveAgentCount = Agents.Num();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized %d agents in %d herd groups"), ActiveAgentCount, HerdGroups.Num());
}

void ACrowdSimulationManager::TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius, ECrowd_AgentType AffectedType)
{
    for (FCrowd_AgentData& Agent : Agents)
    {
        if (Agent.AgentType != AffectedType) continue;
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < ThreatRadius)
        {
            Agent.State = ECrowd_AgentState::Fleeing;
            // Flee direction = away from threat
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * FleeSpeed;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] FleeEvent triggered at %s radius=%.0f"), *ThreatLocation.ToString(), ThreatRadius);
}

void ACrowdSimulationManager::StartMigration(int32 GroupID, FVector Destination)
{
    for (FCrowd_HerdGroup& Group : HerdGroups)
    {
        if (Group.GroupID == GroupID)
        {
            Group.MigrationTarget = Destination;
            Group.bIsMigrating = true;
            break;
        }
    }
    for (FCrowd_AgentData& Agent : Agents)
    {
        if (Agent.HerdGroupID == GroupID)
        {
            Agent.State = ECrowd_AgentState::Migrating;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Migration started for group %d -> %s"), GroupID, *Destination.ToString());
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FCrowd_AgentData> ACrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    const float RadiusSq = Radius * Radius;
    for (const FCrowd_AgentData& A : Agents)
    {
        if (FVector::DistSquared(A.Location, Center) <= RadiusSq)
        {
            Result.Add(A);
        }
    }
    return Result;
}

void ACrowdSimulationManager::SetAgentState(int32 AgentID, ECrowd_AgentState NewState)
{
    for (FCrowd_AgentData& A : Agents)
    {
        if (A.AgentID == AgentID)
        {
            A.State = NewState;
            return;
        }
    }
}

FCrowd_AgentData ACrowdSimulationManager::GetAgentData(int32 AgentID) const
{
    for (const FCrowd_AgentData& A : Agents)
    {
        if (A.AgentID == AgentID) return A;
    }
    return FCrowd_AgentData();
}

// ---------------------------------------------------------------------------
// Private simulation step
// ---------------------------------------------------------------------------

void ACrowdSimulationManager::StepSimulation(float DeltaTime)
{
    UpdateHerdCentroids();

    for (FCrowd_AgentData& Agent : Agents)
    {
        UpdateAgent(Agent, DeltaTime);
    }
}

void ACrowdSimulationManager::UpdateAgent(FCrowd_AgentData& Agent, float DeltaTime)
{
    FVector Steering = FVector::ZeroVector;

    switch (Agent.State)
    {
    case ECrowd_AgentState::Foraging:
    {
        // Flocking behaviour: separation + cohesion + alignment
        Steering = ComputeFlockingForce(Agent);
        // Wander: small random nudge
        Steering += FVector(FMath::RandRange(-30.f, 30.f), FMath::RandRange(-30.f, 30.f), 0.f);
        Agent.Velocity = (Agent.Velocity + Steering * DeltaTime).GetClampedToMaxSize(ForageSpeed);
        break;
    }
    case ECrowd_AgentState::Fleeing:
    {
        // Keep fleeing direction, decay after distance
        Agent.Velocity = Agent.Velocity.GetClampedToMaxSize(FleeSpeed);
        // Transition back to foraging after a while
        if (Agent.Velocity.SizeSquared() < 100.f)
        {
            Agent.State = ECrowd_AgentState::Foraging;
        }
        break;
    }
    case ECrowd_AgentState::Migrating:
    {
        FCrowd_HerdGroup* Group = HerdGroups.FindByPredicate(
            [&](const FCrowd_HerdGroup& G){ return G.GroupID == Agent.HerdGroupID; });
        if (Group)
        {
            FVector ToTarget = (Group->MigrationTarget - Agent.Location).GetSafeNormal();
            Agent.Velocity = ToTarget * MigrationSpeed;
            // Arrived?
            if (FVector::Dist(Agent.Location, Group->MigrationTarget) < 200.f)
            {
                Agent.State = ECrowd_AgentState::Foraging;
                Group->bIsMigrating = false;
            }
        }
        break;
    }
    case ECrowd_AgentState::Resting:
        Agent.Velocity = FVector::ZeroVector;
        break;
    case ECrowd_AgentState::Idle:
    default:
        Agent.Velocity *= 0.9f; // Dampen
        break;
    }

    // Integrate position
    Agent.Location += Agent.Velocity * DeltaTime;

    // Boundary wrap within simulation radius
    const float R = SimulationRadius;
    if (Agent.Location.X > R)  Agent.Location.X -= 2.f * R;
    if (Agent.Location.X < -R) Agent.Location.X += 2.f * R;
    if (Agent.Location.Y > R)  Agent.Location.Y -= 2.f * R;
    if (Agent.Location.Y < -R) Agent.Location.Y += 2.f * R;
}

FVector ACrowdSimulationManager::ComputeFlockingForce(const FCrowd_AgentData& Agent) const
{
    return ComputeSeparation(Agent) * 1.5f
         + ComputeCohesion(Agent)   * 0.8f
         + ComputeAlignment(Agent)  * 1.0f;
}

FVector ACrowdSimulationManager::ComputeSeparation(const FCrowd_AgentData& Agent) const
{
    FVector Force = FVector::ZeroVector;
    int32 Count = 0;
    const float RadSq = SeparationRadius * SeparationRadius;

    for (const FCrowd_AgentData& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        float DistSq = FVector::DistSquared(Agent.Location, Other.Location);
        if (DistSq < RadSq && DistSq > 0.f)
        {
            FVector Away = (Agent.Location - Other.Location);
            Away /= FMath::Sqrt(DistSq); // Normalize and weight by proximity
            Force += Away;
            ++Count;
        }
    }
    return Count > 0 ? (Force / Count) : FVector::ZeroVector;
}

FVector ACrowdSimulationManager::ComputeCohesion(const FCrowd_AgentData& Agent) const
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    const float RadSq = CohesionRadius * CohesionRadius;

    for (const FCrowd_AgentData& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (Other.HerdGroupID != Agent.HerdGroupID) continue;
        if (FVector::DistSquared(Agent.Location, Other.Location) < RadSq)
        {
            CenterOfMass += Other.Location;
            ++Count;
        }
    }
    if (Count == 0) return FVector::ZeroVector;
    CenterOfMass /= Count;
    return (CenterOfMass - Agent.Location).GetSafeNormal();
}

FVector ACrowdSimulationManager::ComputeAlignment(const FCrowd_AgentData& Agent) const
{
    FVector AvgVelocity = FVector::ZeroVector;
    int32 Count = 0;
    const float RadSq = AlignmentRadius * AlignmentRadius;

    for (const FCrowd_AgentData& Other : Agents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (Other.HerdGroupID != Agent.HerdGroupID) continue;
        if (FVector::DistSquared(Agent.Location, Other.Location) < RadSq)
        {
            AvgVelocity += Other.Velocity;
            ++Count;
        }
    }
    return Count > 0 ? (AvgVelocity / Count).GetSafeNormal() : FVector::ZeroVector;
}

void ACrowdSimulationManager::UpdateHerdCentroids()
{
    for (FCrowd_HerdGroup& Group : HerdGroups)
    {
        if (Group.MemberAgentIDs.Num() == 0) continue;
        FVector Sum = FVector::ZeroVector;
        int32 Found = 0;
        for (int32 ID : Group.MemberAgentIDs)
        {
            const FCrowd_AgentData* A = Agents.FindByPredicate([ID](const FCrowd_AgentData& Ag){ return Ag.AgentID == ID; });
            if (A)
            {
                Sum += A->Location;
                ++Found;
            }
        }
        if (Found > 0)
        {
            Group.CentroidLocation = Sum / Found;
        }
    }
}

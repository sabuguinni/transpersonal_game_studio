// CrowdMassAgentController.cpp
// Crowd & Traffic Simulation Agent #13
// Full implementation — 50,000 agent Mass AI crowd system
// Integrates with DinosaurCombatAI (Agent #12) threat signals
// and CrowdStampedeController (Agent #13 prev cycles) panic waves

#include "CrowdMassAgentController.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Math/UnrealMathUtility.h"

ACrowdMassAgentController::ACrowdMassAgentController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz simulation tick

    // Default flocking params tuned for prehistoric herd animals
    FlockingParams.SeparationRadius = 120.0f;
    FlockingParams.AlignmentRadius  = 350.0f;
    FlockingParams.CohesionRadius   = 500.0f;
    FlockingParams.SeparationWeight = 1.5f;
    FlockingParams.AlignmentWeight  = 1.0f;
    FlockingParams.CohesionWeight   = 0.8f;
    FlockingParams.MaxSpeed         = 400.0f;
    FlockingParams.FleeSpeedMultiplier = 2.2f;
}

void ACrowdMassAgentController::BeginPlay()
{
    Super::BeginPlay();
    SimTime = 0.0f;
    NextAgentID = 0;
    UE_LOG(LogTemp, Log, TEXT("[CrowdMassAgentController] Initialized — MaxAgents=%d, SimRadius=%.0f"),
        MaxActiveAgents, SimulationRadius);
}

void ACrowdMassAgentController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    SimTime += DeltaTime;

    // Get player location for LOD computation
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        LastPlayerLocation = PC->GetPawn()->GetActorLocation();
    }

    // Clean expired threats
    CleanExpiredThreats(SimTime);

    // Compute global panic level (weighted average of all threat intensities)
    float TotalPanic = 0.0f;
    for (const FCrowd_ThreatSource& Threat : ActiveThreats)
    {
        TotalPanic += Threat.Intensity;
    }
    GlobalPanicLevel = FMath::Clamp(TotalPanic / FMath::Max(1, ActiveThreats.Num()), 0.0f, 1.0f);

    // Tick all agents
    TickAgents(DeltaTime);

    TotalAgentsSimulated = ActiveAgents.Num();
}

// ============================================================
// SpawnAgentGroup — place N agents of a species near a center
// ============================================================
void ACrowdMassAgentController::SpawnAgentGroup(
    ECrowd_AgentSpecies Species, int32 Count, FVector CenterLocation, float SpawnRadius)
{
    int32 Remaining = MaxActiveAgents - ActiveAgents.Num();
    int32 ToSpawn = FMath::Min(Count, Remaining);

    for (int32 i = 0; i < ToSpawn; ++i)
    {
        FCrowd_AgentData Agent;
        Agent.AgentID  = NextAgentID++;
        Agent.Species  = Species;
        Agent.Behavior = ECrowd_AgentBehavior::Wandering;
        Agent.Health   = 100.0f;
        Agent.PanicLevel = 0.0f;
        Agent.LastUpdateTime = SimTime;

        // Random position within spawn radius
        float Angle  = FMath::RandRange(0.0f, 2.0f * PI);
        float Dist   = FMath::RandRange(0.0f, SpawnRadius);
        Agent.Position = CenterLocation + FVector(FMath::Cos(Angle) * Dist, FMath::Sin(Angle) * Dist, 0.0f);

        // Species-specific initial velocity
        float BaseSpeed = 80.0f;
        switch (Species)
        {
            case ECrowd_AgentSpecies::HadrosaurHerd:   BaseSpeed = 120.0f; break;
            case ECrowd_AgentSpecies::CeratopsHerd:    BaseSpeed = 90.0f;  break;
            case ECrowd_AgentSpecies::OrnithopodFlock: BaseSpeed = 200.0f; break;
            case ECrowd_AgentSpecies::PterosaurSwarm:  BaseSpeed = 350.0f; break;
            case ECrowd_AgentSpecies::SmallMammal:     BaseSpeed = 160.0f; break;
            default: BaseSpeed = 80.0f; break;
        }

        float VAngle = FMath::RandRange(0.0f, 2.0f * PI);
        Agent.Velocity = FVector(FMath::Cos(VAngle) * BaseSpeed, FMath::Sin(VAngle) * BaseSpeed, 0.0f);

        ActiveAgents.Add(Agent);
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdMassAgentController] Spawned %d agents of species %d near (%.0f,%.0f,%.0f)"),
        ToSpawn, (int32)Species, CenterLocation.X, CenterLocation.Y, CenterLocation.Z);
}

// ============================================================
// RegisterThreat — called by DinosaurCombatAI or StampedeController
// ============================================================
void ACrowdMassAgentController::RegisterThreat(FVector Location, float Radius, float Intensity, FString Tag)
{
    // Check for existing threat at same location (update instead of duplicate)
    for (FCrowd_ThreatSource& Existing : ActiveThreats)
    {
        if (FVector::Dist(Existing.Location, Location) < 200.0f && Existing.SourceTag == Tag)
        {
            Existing.Intensity  = FMath::Max(Existing.Intensity, Intensity);
            Existing.Radius     = FMath::Max(Existing.Radius, Radius);
            Existing.ExpiresAt  = SimTime + 15.0f;
            return;
        }
    }

    FCrowd_ThreatSource NewThreat;
    NewThreat.Location   = Location;
    NewThreat.Radius     = Radius;
    NewThreat.Intensity  = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewThreat.ExpiresAt  = SimTime + 15.0f;
    NewThreat.SourceTag  = Tag;
    ActiveThreats.Add(NewThreat);

    UE_LOG(LogTemp, Log, TEXT("[CrowdMassAgentController] Threat registered: %s at (%.0f,%.0f) R=%.0f I=%.2f"),
        *Tag, Location.X, Location.Y, Radius, Intensity);
}

void ACrowdMassAgentController::ClearAllThreats()
{
    ActiveThreats.Empty();
    GlobalPanicLevel = 0.0f;
    UE_LOG(LogTemp, Log, TEXT("[CrowdMassAgentController] All threats cleared"));
}

// ============================================================
// TickAgents — main simulation loop
// ============================================================
void ACrowdMassAgentController::TickAgents(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        // LOD update
        float DistToPlayer = FVector::Dist(Agent.Position, LastPlayerLocation);
        UpdateLOD(Agent, LastPlayerLocation);

        // Skip culled agents
        if (Agent.LOD == ECrowd_LODLevel::Culled)
        {
            continue;
        }

        // Reduced tick rate for distant agents
        if (Agent.LOD == ECrowd_LODLevel::Minimal)
        {
            if (FMath::Fmod(SimTime, 0.5f) > 0.05f) continue;
        }

        // Update behavior state
        UpdateAgentBehavior(Agent, DeltaTime);

        // Apply flocking for full/reduced LOD
        if (Agent.LOD == ECrowd_LODLevel::Full || Agent.LOD == ECrowd_LODLevel::Reduced)
        {
            ApplyFlocking(Agent, DeltaTime);
        }

        // Integrate position
        float SpeedMult = (Agent.Behavior == ECrowd_AgentBehavior::Fleeing ||
                           Agent.Behavior == ECrowd_AgentBehavior::Stampeding)
                           ? FlockingParams.FleeSpeedMultiplier : 1.0f;

        float MaxSpd = FlockingParams.MaxSpeed * SpeedMult;
        if (Agent.Velocity.SizeSquared() > MaxSpd * MaxSpd)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * MaxSpd;
        }

        Agent.Position += Agent.Velocity * DeltaTime;
        Agent.LastUpdateTime = SimTime;

        // Debug draw
        if (bDebugDraw && Agent.LOD == ECrowd_LODLevel::Full)
        {
            FColor DebugColor = FColor::Green;
            if (Agent.Behavior == ECrowd_AgentBehavior::Fleeing)   DebugColor = FColor::Red;
            if (Agent.Behavior == ECrowd_AgentBehavior::Stampeding) DebugColor = FColor::Orange;
            DrawDebugSphere(GetWorld(), Agent.Position, 40.0f, 6, DebugColor, false, 0.1f);
        }
    }
}

// ============================================================
// UpdateAgentBehavior — state machine per agent
// ============================================================
void ACrowdMassAgentController::UpdateAgentBehavior(FCrowd_AgentData& Agent, float DeltaTime)
{
    // Compute local panic from threats
    float LocalPanic = GetPanicLevelAtLocation(Agent.Position);
    Agent.PanicLevel = FMath::FInterpTo(Agent.PanicLevel, LocalPanic, DeltaTime, 3.0f);

    // State transitions
    switch (Agent.Behavior)
    {
        case ECrowd_AgentBehavior::Idle:
        case ECrowd_AgentBehavior::Wandering:
        case ECrowd_AgentBehavior::Foraging:
            if (Agent.PanicLevel > 0.3f)
            {
                Agent.Behavior  = ECrowd_AgentBehavior::Fleeing;
                Agent.FleeTarget = ComputeFleeDirection(Agent.Position) * 3000.0f + Agent.Position;
                PropagateFleeSignal(Agent);
            }
            else
            {
                // Gentle wander — slight random velocity perturbation
                float WanderAngle = FMath::RandRange(-0.3f, 0.3f);
                FVector Right(-Agent.Velocity.Y, Agent.Velocity.X, 0.0f);
                Agent.Velocity += Right.GetSafeNormal() * WanderAngle * 50.0f * DeltaTime;
            }
            break;

        case ECrowd_AgentBehavior::Fleeing:
            if (Agent.PanicLevel > 0.7f)
            {
                Agent.Behavior = ECrowd_AgentBehavior::Stampeding;
            }
            else if (Agent.PanicLevel < 0.1f)
            {
                Agent.Behavior = ECrowd_AgentBehavior::Wandering;
                Agent.FleeTarget = FVector::ZeroVector;
            }
            else
            {
                // Steer toward flee target
                FVector ToTarget = (Agent.FleeTarget - Agent.Position).GetSafeNormal();
                Agent.Velocity = FMath::VInterpTo(Agent.Velocity, ToTarget * FlockingParams.MaxSpeed, DeltaTime, 4.0f);
            }
            break;

        case ECrowd_AgentBehavior::Stampeding:
            if (Agent.PanicLevel < 0.2f)
            {
                Agent.Behavior = ECrowd_AgentBehavior::Fleeing;
            }
            else
            {
                // Full stampede — max speed away from all threats
                FVector FleeDir = ComputeFleeDirection(Agent.Position);
                Agent.Velocity = FleeDir * FlockingParams.MaxSpeed * FlockingParams.FleeSpeedMultiplier;
            }
            break;

        case ECrowd_AgentBehavior::Sheltering:
            // Slow down, reduce panic
            Agent.Velocity *= 0.9f;
            Agent.PanicLevel = FMath::Max(0.0f, Agent.PanicLevel - DeltaTime * 0.5f);
            if (Agent.PanicLevel < 0.05f)
            {
                Agent.Behavior = ECrowd_AgentBehavior::Idle;
            }
            break;

        case ECrowd_AgentBehavior::Migrating:
            // Maintain heading, minimal steering
            break;

        case ECrowd_AgentBehavior::Dead:
            Agent.Velocity = FVector::ZeroVector;
            break;

        default:
            break;
    }
}

// ============================================================
// ApplyFlocking — Reynolds boids: separation, alignment, cohesion
// ============================================================
void ACrowdMassAgentController::ApplyFlocking(FCrowd_AgentData& Agent, float DeltaTime)
{
    FVector Separation  = FVector::ZeroVector;
    FVector Alignment   = FVector::ZeroVector;
    FVector Cohesion    = FVector::ZeroVector;
    int32   AlignCount  = 0;
    int32   CohesCount  = 0;

    for (const FCrowd_AgentData& Other : ActiveAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        if (Other.Species != Agent.Species) continue; // Only flock with same species

        float Dist = FVector::Dist(Agent.Position, Other.Position);

        if (Dist < FlockingParams.SeparationRadius && Dist > 0.1f)
        {
            FVector Away = (Agent.Position - Other.Position).GetSafeNormal();
            Separation += Away / Dist; // Stronger push when closer
        }

        if (Dist < FlockingParams.AlignmentRadius)
        {
            Alignment += Other.Velocity;
            ++AlignCount;
        }

        if (Dist < FlockingParams.CohesionRadius)
        {
            Cohesion += Other.Position;
            ++CohesCount;
        }
    }

    if (AlignCount > 0) Alignment /= AlignCount;
    if (CohesCount > 0)
    {
        Cohesion = (Cohesion / CohesCount - Agent.Position).GetSafeNormal() * FlockingParams.MaxSpeed;
    }

    FVector FlockForce =
        Separation * FlockingParams.SeparationWeight +
        Alignment  * FlockingParams.AlignmentWeight  +
        Cohesion   * FlockingParams.CohesionWeight;

    Agent.Velocity += FlockForce * DeltaTime;
}

// ============================================================
// PropagateFleeSignal — nearby agents catch the panic
// ============================================================
void ACrowdMassAgentController::PropagateFleeSignal(FCrowd_AgentData& Agent)
{
    const float PropagationRadius = 600.0f;
    const float PanicBoost = 0.4f;

    for (FCrowd_AgentData& Other : ActiveAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        float Dist = FVector::Dist(Agent.Position, Other.Position);
        if (Dist < PropagationRadius)
        {
            float FalloffFactor = 1.0f - (Dist / PropagationRadius);
            Other.PanicLevel = FMath::Clamp(Other.PanicLevel + PanicBoost * FalloffFactor, 0.0f, 1.0f);
            if (Other.PanicLevel > 0.3f && Other.Behavior == ECrowd_AgentBehavior::Wandering)
            {
                Other.Behavior   = ECrowd_AgentBehavior::Fleeing;
                Other.FleeTarget = ComputeFleeDirection(Other.Position) * 3000.0f + Other.Position;
            }
        }
    }
}

// ============================================================
// CleanExpiredThreats
// ============================================================
void ACrowdMassAgentController::CleanExpiredThreats(float CurrentTime)
{
    ActiveThreats.RemoveAll([CurrentTime](const FCrowd_ThreatSource& T)
    {
        return T.ExpiresAt < CurrentTime;
    });
}

// ============================================================
// ComputeFleeDirection — away from all active threats (weighted)
// ============================================================
FVector ACrowdMassAgentController::ComputeFleeDirection(const FVector& AgentPos) const
{
    FVector FleeDir = FVector::ZeroVector;

    for (const FCrowd_ThreatSource& Threat : ActiveThreats)
    {
        float Dist = FVector::Dist(AgentPos, Threat.Location);
        if (Dist < Threat.Radius && Dist > 1.0f)
        {
            FVector Away = (AgentPos - Threat.Location).GetSafeNormal();
            float Weight = Threat.Intensity * (1.0f - Dist / Threat.Radius);
            FleeDir += Away * Weight;
        }
    }

    return FleeDir.IsNearlyZero() ? FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal()
                                  : FleeDir.GetSafeNormal();
}

// ============================================================
// UpdateLOD
// ============================================================
void ACrowdMassAgentController::UpdateLOD(FCrowd_AgentData& Agent, const FVector& PlayerLocation)
{
    float Dist = FVector::Dist(Agent.Position, PlayerLocation);
    Agent.LOD = ComputeLODForDistance(Dist);
}

ECrowd_LODLevel ACrowdMassAgentController::ComputeLODForDistance(float Distance) const
{
    if (Distance < FullLODRadius)    return ECrowd_LODLevel::Full;
    if (Distance < ReducedLODRadius) return ECrowd_LODLevel::Reduced;
    if (Distance < SimulationRadius) return ECrowd_LODLevel::Minimal;
    return ECrowd_LODLevel::Culled;
}

// ============================================================
// Query API
// ============================================================
float ACrowdMassAgentController::GetPanicLevelAtLocation(FVector Location) const
{
    float MaxPanic = 0.0f;
    for (const FCrowd_ThreatSource& Threat : ActiveThreats)
    {
        float Dist = FVector::Dist(Location, Threat.Location);
        if (Dist < Threat.Radius)
        {
            float LocalPanic = Threat.Intensity * (1.0f - Dist / Threat.Radius);
            MaxPanic = FMath::Max(MaxPanic, LocalPanic);
        }
    }
    return FMath::Clamp(MaxPanic, 0.0f, 1.0f);
}

int32 ACrowdMassAgentController::GetAgentCountInRadius(FVector Center, float Radius) const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.Position, Center) < Radius) ++Count;
    }
    return Count;
}

TArray<FCrowd_AgentData> ACrowdMassAgentController::GetFleeingAgentsNear(FVector Location, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if ((Agent.Behavior == ECrowd_AgentBehavior::Fleeing ||
             Agent.Behavior == ECrowd_AgentBehavior::Stampeding) &&
            FVector::Dist(Agent.Position, Location) < Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

void ACrowdMassAgentController::DrawDebugAgents(bool bEnabled)
{
    bDebugDraw = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("[CrowdMassAgentController] Debug draw: %s"), bEnabled ? TEXT("ON") : TEXT("OFF"));
}

FString ACrowdMassAgentController::GetSimulationStats() const
{
    int32 Fleeing = 0, Stampeding = 0, Idle = 0, Wandering = 0;
    for (const FCrowd_AgentData& A : ActiveAgents)
    {
        switch (A.Behavior)
        {
            case ECrowd_AgentBehavior::Fleeing:    ++Fleeing;    break;
            case ECrowd_AgentBehavior::Stampeding: ++Stampeding; break;
            case ECrowd_AgentBehavior::Idle:       ++Idle;       break;
            case ECrowd_AgentBehavior::Wandering:  ++Wandering;  break;
            default: break;
        }
    }
    return FString::Printf(
        TEXT("Agents=%d | Fleeing=%d | Stampeding=%d | Idle=%d | Wandering=%d | Threats=%d | GlobalPanic=%.2f"),
        ActiveAgents.Num(), Fleeing, Stampeding, Idle, Wandering, ActiveThreats.Num(), GlobalPanicLevel);
}

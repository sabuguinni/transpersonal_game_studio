// CrowdMassSpawner.cpp
// Agent #13 — Crowd & Traffic Simulation
// Full implementation of Mass Entity spawner for prehistoric herd simulation

#include "CrowdMassSpawner.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ACrowd_MassSpawner::ACrowd_MassSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    MaxActiveAgents = 200;
    bAutoSpawnOnBeginPlay = true;
    MigrationSpeed = 150.0f;
    bDebugDrawEnabled = false;

    // Default LOD settings
    DefaultLODSettings.FullDetailDistance = 2000.0f;
    DefaultLODSettings.MediumDetailDistance = 5000.0f;
    DefaultLODSettings.LowDetailDistance = 10000.0f;
    DefaultLODSettings.bUseBillboardBeyondLow = true;
}

void ACrowd_MassSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoSpawnOnBeginPlay)
    {
        // Spawn default herds after a short delay to let world initialize
        FTimerHandle SpawnTimer;
        GetWorldTimerManager().SetTimer(SpawnTimer, this, &ACrowd_MassSpawner::SpawnDefaultHerds, 1.0f, false);
    }
}

void ACrowd_MassSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateMigrationPaths(DeltaTime);
    UpdateLODForAllAgents();

    if (bDebugDrawEnabled)
    {
        DrawDebugAgentStates();
    }
}

void ACrowd_MassSpawner::SpawnHerd(const FCrowd_SpawnRequest& Request)
{
    if (!GetWorld()) return;

    int32 SpawnCount = FMath::Min(Request.AgentCount, MaxActiveAgents - ActiveAgents.Num());
    if (SpawnCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdMassSpawner: Max agent limit reached (%d/%d)"), ActiveAgents.Num(), MaxActiveAgents);
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Spawning %d agents with pattern %d"), SpawnCount, (int32)Request.SpawnPattern);

    TArray<FVector> SpawnPositions = GenerateSpawnPositions(Request, SpawnCount);

    for (int32 i = 0; i < SpawnPositions.Num(); i++)
    {
        FCrowd_AgentState NewAgent;
        NewAgent.AgentID = NextAgentID++;
        NewAgent.Location = SpawnPositions[i];
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.CurrentState = ECrowd_AgentState::Grazing;
        NewAgent.LODLevel = 0;
        NewAgent.SpeciesTag = Request.SpeciesTag;
        NewAgent.bIsHerdLeader = (i == 0); // First agent is herd leader

        // Assign migration target for migration patterns
        if (Request.SpawnPattern == ECrowd_SpawnPattern::MigrationLine)
        {
            NewAgent.MigrationTarget = Request.MigrationDestination;
            NewAgent.CurrentState = ECrowd_AgentState::Migrating;
        }

        ActiveAgents.Add(NewAgent);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Total active agents: %d"), ActiveAgents.Num());
}

TArray<FVector> ACrowd_MassSpawner::GenerateSpawnPositions(const FCrowd_SpawnRequest& Request, int32 Count)
{
    TArray<FVector> Positions;
    FVector Center = Request.SpawnCenter;
    float Radius = Request.SpawnRadius;

    switch (Request.SpawnPattern)
    {
        case ECrowd_SpawnPattern::Scattered:
        {
            for (int32 i = 0; i < Count; i++)
            {
                FVector2D RandCircle = FMath::RandPointInCircle(Radius);
                FVector Pos = Center + FVector(RandCircle.X, RandCircle.Y, 0.0f);
                Positions.Add(SnapToGround(Pos));
            }
            break;
        }

        case ECrowd_SpawnPattern::HerdFormation:
        {
            // Tight cluster with leader at front
            Positions.Add(SnapToGround(Center)); // Leader
            for (int32 i = 1; i < Count; i++)
            {
                float Angle = FMath::RandRange(0.0f, 360.0f);
                float Dist = FMath::RandRange(Request.MinSpacing, Radius * 0.5f);
                FVector Offset = FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                                        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist, 0.0f);
                Positions.Add(SnapToGround(Center + Offset));
            }
            break;
        }

        case ECrowd_SpawnPattern::MigrationLine:
        {
            // Line formation moving toward destination
            FVector Direction = (Request.MigrationDestination - Center).GetSafeNormal();
            FVector Perpendicular = FVector(-Direction.Y, Direction.X, 0.0f);
            int32 Columns = FMath::Max(1, Count / 3);
            for (int32 i = 0; i < Count; i++)
            {
                int32 Row = i / Columns;
                int32 Col = i % Columns;
                float ColOffset = (Col - Columns * 0.5f) * Request.MinSpacing;
                FVector Pos = Center - Direction * (Row * Request.MinSpacing) + Perpendicular * ColOffset;
                Positions.Add(SnapToGround(Pos));
            }
            break;
        }

        case ECrowd_SpawnPattern::WaterGathering:
        {
            // Agents clustered around a water source (center)
            for (int32 i = 0; i < Count; i++)
            {
                float Angle = (float)i / Count * 360.0f;
                float Dist = FMath::RandRange(Radius * 0.3f, Radius);
                FVector Pos = Center + FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                                               FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist, 0.0f);
                Positions.Add(SnapToGround(Pos));
            }
            break;
        }

        case ECrowd_SpawnPattern::ForestEdge:
        {
            // Line along forest edge with some depth variation
            for (int32 i = 0; i < Count; i++)
            {
                float T = (float)i / Count;
                float LinePos = (T - 0.5f) * Radius * 2.0f;
                float Depth = FMath::RandRange(0.0f, Radius * 0.3f);
                FVector Pos = Center + FVector(LinePos, Depth, 0.0f);
                Positions.Add(SnapToGround(Pos));
            }
            break;
        }

        default:
        {
            for (int32 i = 0; i < Count; i++)
            {
                FVector2D RandCircle = FMath::RandPointInCircle(Radius);
                Positions.Add(SnapToGround(Center + FVector(RandCircle.X, RandCircle.Y, 0.0f)));
            }
            break;
        }
    }

    return Positions;
}

FVector ACrowd_MassSpawner::SnapToGround(const FVector& WorldLocation)
{
    if (!GetWorld()) return WorldLocation;

    FHitResult Hit;
    FVector Start = WorldLocation + FVector(0.0f, 0.0f, 500.0f);
    FVector End = WorldLocation - FVector(0.0f, 0.0f, 1000.0f);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
        return Hit.Location + FVector(0.0f, 0.0f, 5.0f);
    }

    return WorldLocation;
}

void ACrowd_MassSpawner::UpdateMigrationPaths(float DeltaTime)
{
    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        if (Agent.CurrentState != ECrowd_AgentState::Migrating) continue;

        FVector ToTarget = Agent.MigrationTarget - Agent.Location;
        float DistToTarget = ToTarget.Size();

        if (DistToTarget < 200.0f)
        {
            // Arrived at destination — switch to grazing
            Agent.CurrentState = ECrowd_AgentState::Grazing;
            Agent.Velocity = FVector::ZeroVector;
            continue;
        }

        // Move toward migration target
        FVector MoveDir = ToTarget.GetSafeNormal();

        // Add slight separation from nearby agents
        FVector Separation = ComputeSeparationForce(Agent);
        MoveDir = (MoveDir + Separation * 0.3f).GetSafeNormal();

        Agent.Velocity = MoveDir * MigrationSpeed;
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

FVector ACrowd_MassSpawner::ComputeSeparationForce(const FCrowd_AgentState& Agent)
{
    FVector Separation = FVector::ZeroVector;
    float SeparationRadius = 200.0f;

    for (const FCrowd_AgentState& Other : ActiveAgents)
    {
        if (Other.AgentID == Agent.AgentID) continue;
        FVector Delta = Agent.Location - Other.Location;
        float Dist = Delta.Size();
        if (Dist < SeparationRadius && Dist > 0.1f)
        {
            Separation += Delta.GetSafeNormal() * (1.0f - Dist / SeparationRadius);
        }
    }

    return Separation.GetSafeNormal();
}

void ACrowd_MassSpawner::UpdateLODForAllAgents()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.Location, PlayerLoc);

        int32 NewLOD = 0;
        if (Dist > DefaultLODSettings.LowDetailDistance)
            NewLOD = 3; // Billboard
        else if (Dist > DefaultLODSettings.MediumDetailDistance)
            NewLOD = 2; // Low poly
        else if (Dist > DefaultLODSettings.FullDetailDistance)
            NewLOD = 1; // Medium poly
        else
            NewLOD = 0; // Full detail

        Agent.LODLevel = NewLOD;
    }
}

void ACrowd_MassSpawner::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius, float PanicIntensity)
{
    int32 FleeCount = 0;

    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        float DistToThreat = FVector::Dist(Agent.Location, ThreatLocation);
        if (DistToThreat < ThreatRadius)
        {
            Agent.CurrentState = ECrowd_AgentState::Fleeing;

            // Flee direction — away from threat
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.MigrationTarget = Agent.Location + FleeDir * 5000.0f;
            Agent.Velocity = FleeDir * (MigrationSpeed * 2.5f * PanicIntensity);
            FleeCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Flee triggered — %d agents fleeing from threat at (%.0f, %.0f, %.0f)"),
           FleeCount, ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
}

void ACrowd_MassSpawner::SpawnDefaultHerds()
{
    // Brachiosaurus herd — grazing near water
    FCrowd_SpawnRequest BrachioRequest;
    BrachioRequest.SpawnPattern = ECrowd_SpawnPattern::WaterGathering;
    BrachioRequest.AgentCount = 8;
    BrachioRequest.SpawnCenter = FVector(3000.0f, 2000.0f, 0.0f);
    BrachioRequest.SpawnRadius = 1500.0f;
    BrachioRequest.MinSpacing = 300.0f;
    BrachioRequest.SpeciesTag = FName("Brachiosaurus");
    SpawnHerd(BrachioRequest);

    // Raptor pack — forest edge
    FCrowd_SpawnRequest RaptorRequest;
    RaptorRequest.SpawnPattern = ECrowd_SpawnPattern::ForestEdge;
    RaptorRequest.AgentCount = 6;
    RaptorRequest.SpawnCenter = FVector(-2000.0f, 1500.0f, 0.0f);
    RaptorRequest.SpawnRadius = 800.0f;
    RaptorRequest.MinSpacing = 150.0f;
    RaptorRequest.SpeciesTag = FName("Velociraptor");
    SpawnHerd(RaptorRequest);

    // Triceratops migration line
    FCrowd_SpawnRequest TriceRequest;
    TriceRequest.SpawnPattern = ECrowd_SpawnPattern::MigrationLine;
    TriceRequest.AgentCount = 12;
    TriceRequest.SpawnCenter = FVector(-5000.0f, 0.0f, 0.0f);
    TriceRequest.SpawnRadius = 600.0f;
    TriceRequest.MinSpacing = 200.0f;
    TriceRequest.MigrationDestination = FVector(5000.0f, 0.0f, 0.0f);
    TriceRequest.SpeciesTag = FName("Triceratops");
    SpawnHerd(TriceRequest);

    UE_LOG(LogTemp, Log, TEXT("CrowdMassSpawner: Default herds spawned — %d total agents"), ActiveAgents.Num());
}

void ACrowd_MassSpawner::DrawDebugAgentStates()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FCrowd_AgentState& Agent : ActiveAgents)
    {
        FColor DebugColor = FColor::Green;
        switch (Agent.CurrentState)
        {
            case ECrowd_AgentState::Fleeing:    DebugColor = FColor::Red;    break;
            case ECrowd_AgentState::Migrating:  DebugColor = FColor::Yellow; break;
            case ECrowd_AgentState::Drinking:   DebugColor = FColor::Blue;   break;
            case ECrowd_AgentState::Resting:    DebugColor = FColor::White;  break;
            default: break;
        }

        DrawDebugSphere(World, Agent.Location, 50.0f, 8, DebugColor, false, 0.15f);

        if (Agent.bIsHerdLeader)
        {
            DrawDebugSphere(World, Agent.Location + FVector(0, 0, 80), 20.0f, 8, FColor::Orange, false, 0.15f);
        }
    }
}

int32 ACrowd_MassSpawner::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

TArray<FCrowd_AgentState> ACrowd_MassSpawner::GetAgentsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_AgentState> Result;
    for (const FCrowd_AgentState& Agent : ActiveAgents)
    {
        if (FVector::Dist(Agent.Location, Center) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

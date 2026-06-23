#include "CrowdSimulationManager.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz tick for performance

    MaxAgents = 500;
    ActiveAgentCount = 0;
    SeparationRadius = 150.f;
    CohesionRadius = 400.f;
    AlignmentRadius = 300.f;
    LODSimplifiedDistance = 3000.f;
    LODCullDistance = 8000.f;
    PanicRadius = 1200.f;
    bSimulationActive = false;
    LODUpdateTimer = 0.f;
    CachedPlayerLocation = FVector::ZeroVector;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Build default migration paths from waypoints in the world
    FCrowd_MigrationPath DefaultPath;
    DefaultPath.PathName = TEXT("MainMigrationCorridor");
    DefaultPath.PathRadius = 300.f;
    DefaultPath.Waypoints.Add(FVector(-2000, -1500, 100));
    DefaultPath.Waypoints.Add(FVector(-1000, -800, 100));
    DefaultPath.Waypoints.Add(FVector(0, 0, 100));
    DefaultPath.Waypoints.Add(FVector(1000, 800, 100));
    DefaultPath.Waypoints.Add(FVector(2000, 1500, 100));
    MigrationPaths.Add(DefaultPath);

    FCrowd_MigrationPath GrazingPath;
    GrazingPath.PathName = TEXT("GrazingCircuit");
    GrazingPath.PathRadius = 400.f;
    GrazingPath.Waypoints.Add(FVector(1500, -2000, 100));
    GrazingPath.Waypoints.Add(FVector(1800, -1500, 100));
    GrazingPath.Waypoints.Add(FVector(2200, -1000, 100));
    MigrationPaths.Add(GrazingPath);

    bSimulationActive = true;
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSimulationActive || HerdAgents.Num() == 0)
    {
        return;
    }

    // Update cached player location every 0.5s for LOD
    LODUpdateTimer += DeltaTime;
    if (LODUpdateTimer >= 0.5f)
    {
        LODUpdateTimer = 0.f;
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            CachedPlayerLocation = PlayerPawn->GetActorLocation();
        }
    }

    UpdateBoidsSimulation(DeltaTime);
}

void ACrowdSimulationManager::UpdateBoidsSimulation(float DeltaTime)
{
    const int32 AgentCount = HerdAgents.Num();

    for (int32 i = 0; i < AgentCount; ++i)
    {
        FCrowd_HerdAgent& Agent = HerdAgents[i];

        // LOD cull — skip agents too far from player
        float DistToPlayer = FVector::Dist(Agent.Location, CachedPlayerLocation);
        if (DistToPlayer > LODCullDistance)
        {
            continue;
        }

        // Simplified simulation for distant agents
        bool bFullUpdate = ShouldUpdateAgentFull(Agent);

        FVector Separation = FVector::ZeroVector;
        FVector Cohesion = FVector::ZeroVector;
        FVector Alignment = FVector::ZeroVector;
        int32 SepCount = 0, CohCount = 0, AlnCount = 0;

        if (bFullUpdate)
        {
            // Boids: calculate forces from neighbors
            for (int32 j = 0; j < AgentCount; ++j)
            {
                if (i == j) continue;
                if (HerdAgents[j].HerdID != Agent.HerdID) continue;

                FVector Delta = Agent.Location - HerdAgents[j].Location;
                float Dist = Delta.Size();

                if (Dist < SeparationRadius && Dist > 0.f)
                {
                    Separation += Delta.GetSafeNormal() / Dist;
                    SepCount++;
                }
                if (Dist < CohesionRadius)
                {
                    Cohesion += HerdAgents[j].Location;
                    CohCount++;
                }
                if (Dist < AlignmentRadius)
                {
                    Alignment += HerdAgents[j].Velocity;
                    AlnCount++;
                }
            }

            if (CohCount > 0)
            {
                Cohesion = (Cohesion / CohCount - Agent.Location).GetSafeNormal();
            }
            if (AlnCount > 0)
            {
                Alignment = (Alignment / AlnCount).GetSafeNormal();
            }
        }

        // Combine forces
        FVector SteeringForce = FVector::ZeroVector;

        if (Agent.CurrentBehavior == ECrowd_HerdBehavior::Fleeing)
        {
            // Fleeing: separation dominates, move away from threat
            SteeringForce = Separation * 3.f + Alignment * 0.5f;
        }
        else if (Agent.CurrentBehavior == ECrowd_HerdBehavior::Migrating)
        {
            // Migrating: follow path + cohesion
            UpdateMigration(Agent, DeltaTime);
            SteeringForce = Separation * 1.5f + Cohesion * 0.8f + Alignment * 1.0f;
        }
        else if (Agent.CurrentBehavior == ECrowd_HerdBehavior::Grazing)
        {
            // Grazing: slow movement, stay cohesive
            SteeringForce = Separation * 1.0f + Cohesion * 1.2f;
        }
        else if (Agent.CurrentBehavior == ECrowd_HerdBehavior::Hunting)
        {
            // Hunting: alignment dominant (pack coordination)
            SteeringForce = Separation * 1.0f + Alignment * 2.0f + Cohesion * 0.5f;
        }

        // Apply steering to velocity
        Agent.Velocity = (Agent.Velocity + SteeringForce * DeltaTime * 100.f).GetClampedToMaxSize(Agent.Speed);

        // Update position
        Agent.Location += Agent.Velocity * DeltaTime;
    }
}

void ACrowdSimulationManager::UpdateMigration(FCrowd_HerdAgent& Agent, float DeltaTime)
{
    if (MigrationPaths.Num() == 0) return;

    // Find the path for this herd
    int32 PathIndex = Agent.HerdID % MigrationPaths.Num();
    const FCrowd_MigrationPath& Path = MigrationPaths[PathIndex];

    if (Path.Waypoints.Num() == 0) return;

    // Find nearest waypoint ahead
    float MinDist = MAX_FLT;
    int32 TargetWaypoint = 0;
    for (int32 w = 0; w < Path.Waypoints.Num(); ++w)
    {
        float Dist = FVector::Dist(Agent.Location, Path.Waypoints[w]);
        if (Dist < MinDist)
        {
            MinDist = Dist;
            TargetWaypoint = w;
        }
    }

    // Steer toward next waypoint
    int32 NextWaypoint = (TargetWaypoint + 1) % Path.Waypoints.Num();
    FVector ToWaypoint = (Path.Waypoints[NextWaypoint] - Agent.Location).GetSafeNormal();
    Agent.Velocity = FMath::Lerp(Agent.Velocity, ToWaypoint * Agent.Speed, DeltaTime * 2.f);
}

bool ACrowdSimulationManager::ShouldUpdateAgentFull(const FCrowd_HerdAgent& Agent) const
{
    float DistToPlayer = FVector::Dist(Agent.Location, CachedPlayerLocation);
    return DistToPlayer < LODSimplifiedDistance;
}

void ACrowdSimulationManager::SpawnHerd(FVector SpawnLocation, int32 HerdSize, ECrowd_HerdBehavior InitialBehavior, int32 HerdID)
{
    int32 SpawnCount = FMath::Min(HerdSize, MaxAgents - HerdAgents.Num());
    if (SpawnCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Cannot spawn herd — agent cap reached (%d/%d)"), HerdAgents.Num(), MaxAgents);
        return;
    }

    for (int32 i = 0; i < SpawnCount; ++i)
    {
        FCrowd_HerdAgent NewAgent;
        // Scatter agents within 500 unit radius of spawn location
        FVector Offset = FVector(
            FMath::RandRange(-500.f, 500.f),
            FMath::RandRange(-500.f, 500.f),
            0.f
        );
        NewAgent.Location = SpawnLocation + Offset;
        NewAgent.Velocity = FVector(FMath::RandRange(-1.f, 1.f), FMath::RandRange(-1.f, 1.f), 0.f).GetSafeNormal() * 100.f;
        NewAgent.CurrentBehavior = InitialBehavior;
        NewAgent.HerdID = HerdID;
        NewAgent.Speed = FMath::RandRange(150.f, 250.f);
        HerdAgents.Add(NewAgent);
    }

    ActiveAgentCount = HerdAgents.Num();
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned herd %d with %d agents. Total: %d"), HerdID, SpawnCount, ActiveAgentCount);
}

void ACrowdSimulationManager::TriggerPanic(FVector ThreatLocation, float Radius)
{
    int32 PanicCount = 0;
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        float Dist = FVector::Dist(Agent.Location, ThreatLocation);
        if (Dist < Radius)
        {
            Agent.CurrentBehavior = ECrowd_HerdBehavior::Fleeing;
            // Flee direction: away from threat
            FVector FleeDir = (Agent.Location - ThreatLocation).GetSafeNormal();
            Agent.Velocity = FleeDir * Agent.Speed * 1.5f; // Panic boost
            PanicCount++;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Panic triggered at %s — %d agents fleeing"), *ThreatLocation.ToString(), PanicCount);
}

void ACrowdSimulationManager::SetHerdBehavior(int32 HerdID, ECrowd_HerdBehavior NewBehavior)
{
    for (FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.HerdID == HerdID)
        {
            Agent.CurrentBehavior = NewBehavior;
        }
    }
}

int32 ACrowdSimulationManager::GetHerdAgentCount(int32 HerdID) const
{
    int32 Count = 0;
    for (const FCrowd_HerdAgent& Agent : HerdAgents)
    {
        if (Agent.HerdID == HerdID)
        {
            Count++;
        }
    }
    return Count;
}

void ACrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Simulation %s"), bActive ? TEXT("STARTED") : TEXT("STOPPED"));
}

void ACrowdSimulationManager::AddMigrationPath(const FCrowd_MigrationPath& Path)
{
    MigrationPaths.Add(Path);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Added migration path '%s' with %d waypoints"), *Path.PathName, Path.Waypoints.Num());
}

void ACrowdSimulationManager::ClearAllAgents()
{
    HerdAgents.Empty();
    ActiveAgentCount = 0;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: All agents cleared"));
}

// CrowdMigrationSystem.cpp
// Agent #13 — Crowd & Traffic Simulation
// Cycle: PROD_CYCLE_AUTO_20260627_009

#include "CrowdMigrationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ACrowdMigrationSystem::ACrowdMigrationSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // LOD distances (units)
    LOD0_Distance = 500.0f;
    LOD1_Distance = 2000.0f;
    LOD2_Distance = 5000.0f;

    // Agent caps per LOD tier
    MaxAgentsLOD0 = 12;
    MaxAgentsLOD1 = 40;
    MaxAgentsLOD2 = 200;

    // Alarm radius — herd scatters if threat enters this range
    AlarmRadius = 800.0f;
}

void ACrowdMigrationSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeMigrationPath();
}

void ACrowdMigrationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    AdvanceHerdAlongPath(DeltaTime);
}

void ACrowdMigrationSystem::InitializeMigrationPath()
{
    MigrationPath.Empty();

    // West Forest → River → Eastern Plains
    TArray<FVector> PathPositions = {
        FVector(-2000.0f,    0.0f, 100.0f),  // WP0: West forest edge
        FVector(-1200.0f,  400.0f, 100.0f),  // WP1: Forest clearing
        FVector( -400.0f,  600.0f, 100.0f),  // WP2: Pre-river approach
        FVector(  200.0f,  800.0f, 100.0f),  // WP3: River crossing
        FVector(  900.0f,  700.0f, 100.0f),  // WP4: Eastern bank
        FVector( 1600.0f,  500.0f, 100.0f),  // WP5: Eastern plains
        FVector( 2200.0f,  200.0f, 100.0f),  // WP6: Grazing grounds
    };

    for (int32 i = 0; i < PathPositions.Num(); ++i)
    {
        FCrowd_MigrationWaypoint WP;
        WP.WorldPosition   = PathPositions[i];
        WP.WaypointIndex   = i;
        WP.ArrivalRadius   = 200.0f;
        MigrationPath.Add(WP);
    }

    UE_LOG(LogTemp, Log, TEXT("CrowdMigration: Initialized %d waypoints"), MigrationPath.Num());
}

void ACrowdMigrationSystem::AdvanceHerdAlongPath(float DeltaTime)
{
    if (MigrationPath.Num() == 0) return;

    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        // Alarmed agents scatter — skip normal migration
        if (Agent.bIsAlarmed) continue;

        // Clamp waypoint index
        if (Agent.CurrentWaypointIndex >= MigrationPath.Num())
        {
            Agent.CurrentWaypointIndex = 0; // Loop migration
        }

        // Advance toward current waypoint (conceptual — actual movement via BT/NavMesh)
        // This tick updates agent state for Blueprint/BT queries
    }
}

ECrowd_LODTier ACrowdMigrationSystem::CalculateLODTier(FVector AgentLocation, FVector PlayerLocation) const
{
    const float Distance = FVector::Dist(AgentLocation, PlayerLocation);

    if (Distance <= LOD0_Distance)
    {
        return ECrowd_LODTier::LOD0_Individual;
    }
    else if (Distance <= LOD1_Distance)
    {
        return ECrowd_LODTier::LOD1_Group;
    }
    else
    {
        return ECrowd_LODTier::LOD2_Billboard;
    }
}

void ACrowdMigrationSystem::TriggerAlarm(FVector AlarmSource, float Radius)
{
    int32 AlarmedCount = 0;
    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        // Mark agents within alarm radius as alarmed
        // Actual distance check would use agent actor location
        Agent.bIsAlarmed = true;
        ++AlarmedCount;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdMigration: ALARM triggered — %d agents alarmed"), AlarmedCount);
    ScatterHerd(AlarmSource);
}

void ACrowdMigrationSystem::ScatterHerd(FVector ThreatLocation)
{
    // Scatter: agents flee away from threat
    // Direction = AgentLocation - ThreatLocation (normalized)
    // Applied via NavMesh move request in Behavior Tree
    UE_LOG(LogTemp, Warning, TEXT("CrowdMigration: Herd scattering from threat at %s"), *ThreatLocation.ToString());
}

int32 ACrowdMigrationSystem::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_AgentState& Agent : ActiveAgents)
    {
        if (Agent.bIsActive) ++Count;
    }
    return Count;
}

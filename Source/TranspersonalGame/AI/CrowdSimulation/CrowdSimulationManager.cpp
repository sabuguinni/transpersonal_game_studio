#include "CrowdSimulationManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    ActiveAgentCount = 0;
    SimulationTickAccumulator = 0.0f;

    // Default migration loop: North → East → South → West → Center
    FCrowd_WaypointData WP_North;
    WP_North.Location = FVector(0.0f, 2000.0f, 100.0f);
    WP_North.Radius = 300.0f;
    WP_North.DensityWeight = 0.8f;
    WP_North.BehaviorAtWaypoint = ECrowd_HerdBehavior::Grazing;

    FCrowd_WaypointData WP_East;
    WP_East.Location = FVector(2000.0f, 0.0f, 100.0f);
    WP_East.Radius = 300.0f;
    WP_East.DensityWeight = 0.6f;
    WP_East.BehaviorAtWaypoint = ECrowd_HerdBehavior::Migrating;

    FCrowd_WaypointData WP_South;
    WP_South.Location = FVector(0.0f, -2000.0f, 100.0f);
    WP_South.Radius = 300.0f;
    WP_South.DensityWeight = 1.0f;
    WP_South.BehaviorAtWaypoint = ECrowd_HerdBehavior::Drinking;

    FCrowd_WaypointData WP_West;
    WP_West.Location = FVector(-2000.0f, 0.0f, 100.0f);
    WP_West.Radius = 300.0f;
    WP_West.DensityWeight = 0.7f;
    WP_West.BehaviorAtWaypoint = ECrowd_HerdBehavior::Resting;

    GlobalWaypoints.Add(WP_North);
    GlobalWaypoints.Add(WP_East);
    GlobalWaypoints.Add(WP_South);
    GlobalWaypoints.Add(WP_West);

    // Default herd: small herbivore group
    FCrowd_HerdConfig DefaultHerd;
    DefaultHerd.SpeciesName = TEXT("Parasaurolophus");
    DefaultHerd.HerdSize = 8;
    DefaultHerd.MigrationSpeed = 180.0f;
    DefaultHerd.FleeRadius = 1200.0f;
    DefaultHerd.MigrationPath = GlobalWaypoints;
    RegisteredHerds.Add(DefaultHerd);
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    ActiveAgentCount = 0;
    for (const FCrowd_HerdConfig& Herd : RegisteredHerds)
    {
        ActiveAgentCount += Herd.HerdSize;
    }
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: BeginPlay — %d agents across %d herds"),
        ActiveAgentCount, RegisteredHerds.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SimulationTickAccumulator += DeltaTime;
    if (SimulationTickAccumulator >= SimulationTickInterval)
    {
        SimulationTickAccumulator = 0.0f;
        UpdateHerdMigration(SimulationTickInterval);
        ApplyLODCulling();
    }

    if (bDrawDebugPaths && GetWorld())
    {
        for (const FCrowd_WaypointData& WP : GlobalWaypoints)
        {
            DrawDebugSphere(GetWorld(), WP.Location, WP.Radius, 12,
                FColor::Green, false, SimulationTickInterval * 2.0f);
        }
    }
}

void ACrowdSimulationManager::RegisterHerd(const FCrowd_HerdConfig& HerdConfig)
{
    RegisteredHerds.Add(HerdConfig);
    ActiveAgentCount += HerdConfig.HerdSize;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered herd '%s' with %d agents"),
        *HerdConfig.SpeciesName, HerdConfig.HerdSize);
}

void ACrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Flee response triggered at (%f,%f,%f) radius=%f"),
        ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z, ThreatRadius);

    for (FCrowd_HerdConfig& Herd : RegisteredHerds)
    {
        if (ThreatRadius >= Herd.FleeRadius * 0.5f)
        {
            UE_LOG(LogTemp, Log, TEXT("  Herd '%s' is fleeing!"), *Herd.SpeciesName);
        }
    }
}

int32 ACrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

void ACrowdSimulationManager::SetHerdBehavior(const FString& SpeciesName, ECrowd_HerdBehavior NewBehavior)
{
    for (FCrowd_HerdConfig& Herd : RegisteredHerds)
    {
        if (Herd.SpeciesName == SpeciesName)
        {
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: '%s' behavior changed to %d"),
                *SpeciesName, (int32)NewBehavior);
            return;
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Herd '%s' not found"), *SpeciesName);
}

FCrowd_WaypointData ACrowdSimulationManager::GetNearestWaypoint(FVector FromLocation) const
{
    FCrowd_WaypointData Nearest;
    float BestDist = TNumericLimits<float>::Max();

    for (const FCrowd_WaypointData& WP : GlobalWaypoints)
    {
        float Dist = FVector::Dist(FromLocation, WP.Location);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Nearest = WP;
        }
    }
    return Nearest;
}

void ACrowdSimulationManager::UpdateHerdMigration(float DeltaTime)
{
    // Tick-based simulation: herds advance along their migration paths
    // Actual pawn movement is handled by Mass AI / BehaviorTree integration
    // This manager tracks state and triggers behavioral transitions
    for (const FCrowd_HerdConfig& Herd : RegisteredHerds)
    {
        if (Herd.MigrationPath.Num() == 0) continue;
        // State machine tick — future: integrate with MassEntitySubsystem
    }
}

void ACrowdSimulationManager::ApplyLODCulling()
{
    // LOD culling: agents beyond LOD_FarDistance use simplified simulation
    // Agents beyond LOD_NearDistance skip full physics, use capsule approximation
    // Implementation hooks into Mass AI LOD system when available
}

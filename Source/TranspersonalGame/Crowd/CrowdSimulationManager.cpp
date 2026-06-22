#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz — performance-safe

    MaxTotalAgents = 50000;
    bEnableMigration = true;
    bEnablePlayerAvoidance = true;
    PlayerAvoidanceRadius = 800.0f;

    ActiveAgentCount = 0;
    CulledAgentCount = 0;
    CurrentSimulationLoad = 0.0f;
    TickAccumulator = 0.0f;
    MigrationTimer = 0.0f;

    // Default LOD chain
    LODConfig.FullSimDistance = 2000.0f;
    LODConfig.ReducedTickDistance = 5000.0f;
    LODConfig.MinimalPresenceDistance = 10000.0f;
    LODConfig.CullDistance = 15000.0f;
    LODConfig.FullSimTickRate = 0.1f;
    LODConfig.ReducedTickRate = 0.5f;

    // Default migration corridors
    {
        FCrowd_MigrationCorridor CorridorA;
        CorridorA.CorridorID = TEXT("HerdCorridor_A");
        CorridorA.AgentType = ECrowd_AgentType::Herbivore_Herd;
        CorridorA.Waypoints = {
            FVector(-2000, -1500, 50),
            FVector(-1000, -1200, 50),
            FVector(0,      -800, 50),
            FVector(1000,   -600, 50),
            FVector(2000,   -400, 50)
        };
        CorridorA.AgentSpeedMultiplier = 1.0f;
        CorridorA.MaxAgentsInCorridor = 80;
        CorridorA.bIsActive = true;
        MigrationCorridors.Add(CorridorA);

        FCrowd_MigrationCorridor CorridorB;
        CorridorB.CorridorID = TEXT("HerdCorridor_B");
        CorridorB.AgentType = ECrowd_AgentType::Herbivore_Herd;
        CorridorB.Waypoints = {
            FVector(-1500, 1000, 50),
            FVector(-500,   800, 50),
            FVector(500,    600, 50),
            FVector(1500,   400, 50)
        };
        CorridorB.AgentSpeedMultiplier = 0.8f;
        CorridorB.MaxAgentsInCorridor = 60;
        CorridorB.bIsActive = true;
        MigrationCorridors.Add(CorridorB);
    }

    // Default density zones
    {
        // Watering hole — HIGH density
        FCrowd_DensityZoneData WateringHole;
        WateringHole.Center = FVector(300, 400, 0);
        WateringHole.Radius = 400.0f;
        WateringHole.DensityLevel = ECrowd_DensityZone::High;
        WateringHole.MaxConcurrentAgents = 120;
        WateringHole.AgentRespawnInterval = 2.0f;
        DensityZones.Add(WateringHole);

        // Open plains — MEDIUM density
        FCrowd_DensityZoneData Plains;
        Plains.Center = FVector(0, 1800, 0);
        Plains.Radius = 1200.0f;
        Plains.DensityLevel = ECrowd_DensityZone::Medium;
        Plains.MaxConcurrentAgents = 200;
        Plains.AgentRespawnInterval = 5.0f;
        DensityZones.Add(Plains);

        // Forest edge — LOW density
        FCrowd_DensityZoneData Forest;
        Forest.Center = FVector(1200, 1200, 0);
        Forest.Radius = 800.0f;
        Forest.DensityLevel = ECrowd_DensityZone::Low;
        Forest.MaxConcurrentAgents = 30;
        Forest.AgentRespawnInterval = 10.0f;
        DensityZones.Add(Forest);
    }
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCrowdSystem();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickAccumulator += DeltaTime;
    MigrationTimer += DeltaTime;

    // Update LOD every tick (lightweight)
    UpdateLODLevels();

    // Process migration every 2 seconds
    if (MigrationTimer >= 2.0f)
    {
        if (bEnableMigration)
        {
            ProcessMigrationCorridors(DeltaTime);
        }
        MigrationTimer = 0.0f;
    }

    // Player avoidance every tick
    if (bEnablePlayerAvoidance)
    {
        EnforcePlayerAvoidance();
    }

    // Simulation load metric (0-1)
    CurrentSimulationLoad = (MaxTotalAgents > 0)
        ? FMath::Clamp((float)ActiveAgentCount / (float)MaxTotalAgents, 0.0f, 1.0f)
        : 0.0f;
}

void UCrowdSimulationManager::InitializeCrowdSystem()
{
    ActiveAgentCount = 0;
    CulledAgentCount = 0;
    CurrentSimulationLoad = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — MaxAgents:%d Corridors:%d DensityZones:%d"),
        MaxTotalAgents, MigrationCorridors.Num(), DensityZones.Num());
}

void UCrowdSimulationManager::SetMigrationActive(bool bActive)
{
    bEnableMigration = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Migration %s"), bActive ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCrowdSimulationManager::TriggerHerdFlee(FVector ThreatLocation, float ThreatRadius)
{
    if (!GetWorld()) return;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] HERD FLEE triggered at (%.0f, %.0f, %.0f) radius=%.0f"),
        ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z, ThreatRadius);

    // Increase agent speed multiplier for all corridors near threat
    for (FCrowd_MigrationCorridor& Corridor : MigrationCorridors)
    {
        if (!Corridor.Waypoints.IsEmpty())
        {
            float DistToThreat = FVector::Dist(Corridor.Waypoints[0], ThreatLocation);
            if (DistToThreat < ThreatRadius * 2.0f)
            {
                Corridor.AgentSpeedMultiplier = FMath::Clamp(Corridor.AgentSpeedMultiplier * 2.5f, 1.0f, 5.0f);
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Corridor %s flee speed: %.1f"),
                    *Corridor.CorridorID, Corridor.AgentSpeedMultiplier);
            }
        }
    }
}

void UCrowdSimulationManager::TriggerHerdGather(FVector GatherPoint, ECrowd_AgentType AgentType)
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] HERD GATHER at (%.0f, %.0f, %.0f) type=%d"),
        GatherPoint.X, GatherPoint.Y, GatherPoint.Z, (int32)AgentType);

    // Add a temporary high-density zone at the gather point
    FCrowd_DensityZoneData GatherZone;
    GatherZone.Center = GatherPoint;
    GatherZone.Radius = 600.0f;
    GatherZone.DensityLevel = ECrowd_DensityZone::High;
    GatherZone.MaxConcurrentAgents = 150;
    GatherZone.AgentRespawnInterval = 1.0f;
    DensityZones.Add(GatherZone);
}

int32 UCrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    // Returns estimated agent count based on density zones overlapping the query radius
    int32 EstimatedCount = 0;
    for (const FCrowd_DensityZoneData& Zone : DensityZones)
    {
        float Dist = FVector::Dist(Zone.Center, Center);
        if (Dist < Zone.Radius + Radius)
        {
            float Overlap = FMath::Clamp(1.0f - (Dist / (Zone.Radius + Radius)), 0.0f, 1.0f);
            EstimatedCount += FMath::RoundToInt(Zone.MaxConcurrentAgents * Overlap);
        }
    }
    return EstimatedCount;
}

ECrowd_DensityZone UCrowdSimulationManager::GetDensityAtLocation(FVector Location) const
{
    ECrowd_DensityZone Best = ECrowd_DensityZone::Empty;
    float BestDist = MAX_FLT;

    for (const FCrowd_DensityZoneData& Zone : DensityZones)
    {
        float Dist = FVector::Dist(Zone.Center, Location);
        if (Dist < Zone.Radius && Dist < BestDist)
        {
            Best = Zone.DensityLevel;
            BestDist = Dist;
        }
    }
    return Best;
}

ECrowd_LODLevel UCrowdSimulationManager::GetLODForDistance(float DistanceFromPlayer) const
{
    if (DistanceFromPlayer <= LODConfig.FullSimDistance)
        return ECrowd_LODLevel::Full;
    if (DistanceFromPlayer <= LODConfig.ReducedTickDistance)
        return ECrowd_LODLevel::Reduced;
    if (DistanceFromPlayer <= LODConfig.MinimalPresenceDistance)
        return ECrowd_LODLevel::Minimal;
    return ECrowd_LODLevel::Culled;
}

void UCrowdSimulationManager::LogCrowdStats() const
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] === CROWD STATS ==="));
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Active:%d Culled:%d Load:%.1f%%"),
        ActiveAgentCount, CulledAgentCount, CurrentSimulationLoad * 100.0f);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Corridors:%d DensityZones:%d"),
        MigrationCorridors.Num(), DensityZones.Num());
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Migration:%s PlayerAvoidance:%s"),
        bEnableMigration ? TEXT("ON") : TEXT("OFF"),
        bEnablePlayerAvoidance ? TEXT("ON") : TEXT("OFF"));
}

void UCrowdSimulationManager::UpdateLODLevels()
{
    if (!GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Count active vs culled based on density zones
    int32 NewActive = 0;
    int32 NewCulled = 0;

    for (const FCrowd_DensityZoneData& Zone : DensityZones)
    {
        float Dist = FVector::Dist(Zone.Center, PlayerLoc);
        ECrowd_LODLevel LOD = GetLODForDistance(Dist);

        switch (LOD)
        {
        case ECrowd_LODLevel::Full:
            NewActive += Zone.MaxConcurrentAgents;
            break;
        case ECrowd_LODLevel::Reduced:
            NewActive += Zone.MaxConcurrentAgents / 2;
            NewCulled += Zone.MaxConcurrentAgents / 2;
            break;
        case ECrowd_LODLevel::Minimal:
            NewActive += Zone.MaxConcurrentAgents / 5;
            NewCulled += Zone.MaxConcurrentAgents * 4 / 5;
            break;
        case ECrowd_LODLevel::Culled:
            NewCulled += Zone.MaxConcurrentAgents;
            break;
        }
    }

    ActiveAgentCount = FMath::Min(NewActive, MaxTotalAgents);
    CulledAgentCount = NewCulled;
}

void UCrowdSimulationManager::ProcessMigrationCorridors(float DeltaTime)
{
    // Advance migration progress — in full Mass AI integration this drives agent movement
    for (FCrowd_MigrationCorridor& Corridor : MigrationCorridors)
    {
        if (!Corridor.bIsActive || Corridor.Waypoints.IsEmpty()) continue;

        // Decay flee speed back to normal over time
        if (Corridor.AgentSpeedMultiplier > 1.0f)
        {
            Corridor.AgentSpeedMultiplier = FMath::FInterpTo(
                Corridor.AgentSpeedMultiplier, 1.0f, DeltaTime, 0.1f);
        }
    }
}

void UCrowdSimulationManager::EnforcePlayerAvoidance()
{
    if (!GetWorld()) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // Reduce density in zones too close to player (avoidance behaviour)
    for (FCrowd_DensityZoneData& Zone : DensityZones)
    {
        float Dist = FVector::Dist(Zone.Center, PlayerLoc);
        if (Dist < PlayerAvoidanceRadius)
        {
            // Agents scatter — reduce effective count near player
            float ScatterFactor = Dist / PlayerAvoidanceRadius;
            int32 EffectiveMax = FMath::RoundToInt(Zone.MaxConcurrentAgents * ScatterFactor);
            ActiveAgentCount = FMath::Max(0, ActiveAgentCount - (Zone.MaxConcurrentAgents - EffectiveMax));
        }
    }
}

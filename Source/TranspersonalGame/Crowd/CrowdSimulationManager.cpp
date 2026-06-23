// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd simulation: migration corridors, density zones, LOD management

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxActiveAgents = 500;
    LODDistanceClose = 2000.0f;
    LODDistanceMedium = 5000.0f;
    LODDistanceFar = 10000.0f;
    bSimulationActive = false;
    CurrentAgentCount = 0;
    TickInterval = 0.5f;
    TimeSinceLastTick = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    InitializeMigrationCorridors();
    InitializeDensityZones();
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized with %d corridors, %d density zones"),
        MigrationCorridors.Num(), DensityZones.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    ActiveAgents.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::InitializeMigrationCorridors()
{
    // Brachiosaurus herd: North-South migration
    FCrowd_MigrationCorridor BrachioPath;
    BrachioPath.CorridorName = TEXT("Brachio_NorthSouth");
    BrachioPath.SpeciesType = ECrowd_DinoSpecies::Brachiosaurus;
    BrachioPath.Waypoints = {
        FVector(-3000, -5000, 100),
        FVector(-2500, -2500, 120),
        FVector(-2000, 0, 100),
        FVector(-1500, 2500, 110),
        FVector(-1000, 5000, 100)
    };
    BrachioPath.HerdSize = 8;
    BrachioPath.MovementSpeed = 150.0f;
    BrachioPath.bIsCircular = false;
    MigrationCorridors.Add(BrachioPath);

    // Raptor pack: East-West hunting circuit
    FCrowd_MigrationCorridor RaptorPath;
    RaptorPath.CorridorName = TEXT("Raptor_EastWest");
    RaptorPath.SpeciesType = ECrowd_DinoSpecies::Velociraptor;
    RaptorPath.Waypoints = {
        FVector(-4000, -1000, 100),
        FVector(-2000, -500, 100),
        FVector(0, 0, 100),
        FVector(2000, 500, 100),
        FVector(4000, 1000, 100)
    };
    RaptorPath.HerdSize = 5;
    RaptorPath.MovementSpeed = 350.0f;
    RaptorPath.bIsCircular = true;
    MigrationCorridors.Add(RaptorPath);

    // TRex: territorial patrol loop
    FCrowd_MigrationCorridor TRexPath;
    TRexPath.CorridorName = TEXT("TRex_TerritoryPatrol");
    TRexPath.SpeciesType = ECrowd_DinoSpecies::TyrannosaurusRex;
    TRexPath.Waypoints = {
        FVector(1000, 1000, 100),
        FVector(2000, -1000, 100),
        FVector(3000, 1000, 100),
        FVector(2000, 3000, 100)
    };
    TRexPath.HerdSize = 1;
    TRexPath.MovementSpeed = 200.0f;
    TRexPath.bIsCircular = true;
    MigrationCorridors.Add(TRexPath);
}

void UCrowdSimulationManager::InitializeDensityZones()
{
    // Watering hole — high density congregation
    FCrowd_DensityZone WaterZone;
    WaterZone.ZoneName = TEXT("WateringHole_North");
    WaterZone.Center = FVector(-2000, 3000, 50);
    WaterZone.Radius = 800.0f;
    WaterZone.MaxDensity = 25;
    WaterZone.ZoneType = ECrowd_ZoneType::WateringHole;
    WaterZone.bIsActive = true;
    DensityZones.Add(WaterZone);

    // Feeding ground — medium density
    FCrowd_DensityZone FeedZone;
    FeedZone.ZoneName = TEXT("FeedingGround_East");
    FeedZone.Center = FVector(3500, 0, 50);
    FeedZone.Radius = 1200.0f;
    FeedZone.MaxDensity = 15;
    FeedZone.ZoneType = ECrowd_ZoneType::FeedingGround;
    FeedZone.bIsActive = true;
    DensityZones.Add(FeedZone);

    // Open plains — low density, transit zone
    FCrowd_DensityZone PlainsZone;
    PlainsZone.ZoneName = TEXT("OpenPlains_Center");
    PlainsZone.Center = FVector(0, 0, 50);
    PlainsZone.Radius = 2000.0f;
    PlainsZone.MaxDensity = 8;
    PlainsZone.ZoneType = ECrowd_ZoneType::OpenPlains;
    PlainsZone.bIsActive = true;
    DensityZones.Add(PlainsZone);

    // Raptor nest — exclusive predator zone
    FCrowd_DensityZone RaptorNest;
    RaptorNest.ZoneName = TEXT("RaptorNest_West");
    RaptorNest.Center = FVector(-3500, -2000, 50);
    RaptorNest.Radius = 600.0f;
    RaptorNest.MaxDensity = 6;
    RaptorNest.ZoneType = ECrowd_ZoneType::PredatorTerritory;
    RaptorNest.bIsActive = true;
    DensityZones.Add(RaptorNest);

    // TRex territory — exclusive apex predator
    FCrowd_DensityZone TRexTerritory;
    TRexTerritory.ZoneName = TEXT("TRexTerritory_NE");
    TRexTerritory.Center = FVector(2500, 2500, 50);
    TRexTerritory.Radius = 1500.0f;
    TRexTerritory.MaxDensity = 1;
    TRexTerritory.ZoneType = ECrowd_ZoneType::PredatorTerritory;
    TRexTerritory.bIsActive = true;
    DensityZones.Add(TRexTerritory);
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime)
{
    if (!bSimulationActive) return;

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < TickInterval) return;
    TimeSinceLastTick = 0.0f;

    // Update agent LOD based on distance to player
    UpdateAgentLOD();

    // Update migration progress for each corridor
    for (FCrowd_MigrationCorridor& Corridor : MigrationCorridors)
    {
        UpdateCorridorProgress(Corridor, TickInterval);
    }

    CurrentAgentCount = ActiveAgents.Num();
}

void UCrowdSimulationManager::UpdateAgentLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentState& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(Agent.CurrentLocation, PlayerLocation);

        if (Distance < LODDistanceClose)
        {
            Agent.LODLevel = ECrowd_LODLevel::Full;
        }
        else if (Distance < LODDistanceMedium)
        {
            Agent.LODLevel = ECrowd_LODLevel::Medium;
        }
        else if (Distance < LODDistanceFar)
        {
            Agent.LODLevel = ECrowd_LODLevel::Low;
        }
        else
        {
            Agent.LODLevel = ECrowd_LODLevel::Culled;
        }
    }
}

void UCrowdSimulationManager::UpdateCorridorProgress(FCrowd_MigrationCorridor& Corridor, float DeltaTime)
{
    if (Corridor.Waypoints.Num() < 2) return;

    // Simple waypoint progression — advance target waypoint index
    int32 NextWP = (Corridor.CurrentWaypointIndex + 1) % Corridor.Waypoints.Num();

    FVector Current = Corridor.Waypoints[Corridor.CurrentWaypointIndex];
    FVector Target = Corridor.Waypoints[NextWP];
    float DistToTarget = FVector::Dist(Current, Target);

    // Simulate movement progress
    float MoveThisFrame = Corridor.MovementSpeed * DeltaTime;
    if (MoveThisFrame >= DistToTarget)
    {
        // Reached waypoint
        Corridor.CurrentWaypointIndex = NextWP;
        if (!Corridor.bIsCircular && NextWP == Corridor.Waypoints.Num() - 1)
        {
            // End of path — reverse direction or stop
            Corridor.CurrentWaypointIndex = 0;
        }
    }
}

int32 UCrowdSimulationManager::GetAgentCountInZone(const FString& ZoneName) const
{
    for (const FCrowd_DensityZone& Zone : DensityZones)
    {
        if (Zone.ZoneName == ZoneName)
        {
            int32 Count = 0;
            for (const FCrowd_AgentState& Agent : ActiveAgents)
            {
                if (FVector::Dist(Agent.CurrentLocation, Zone.Center) <= Zone.Radius)
                {
                    Count++;
                }
            }
            return Count;
        }
    }
    return 0;
}

bool UCrowdSimulationManager::IsLocationInPredatorTerritory(const FVector& Location) const
{
    for (const FCrowd_DensityZone& Zone : DensityZones)
    {
        if (Zone.ZoneType == ECrowd_ZoneType::PredatorTerritory && Zone.bIsActive)
        {
            if (FVector::Dist(Location, Zone.Center) <= Zone.Radius)
            {
                return true;
            }
        }
    }
    return false;
}

float UCrowdSimulationManager::GetDangerLevelAtLocation(const FVector& Location) const
{
    float MaxDanger = 0.0f;

    for (const FCrowd_DensityZone& Zone : DensityZones)
    {
        if (!Zone.bIsActive) continue;

        float Dist = FVector::Dist(Location, Zone.Center);
        if (Dist <= Zone.Radius)
        {
            float ZoneDanger = 0.0f;
            switch (Zone.ZoneType)
            {
                case ECrowd_ZoneType::PredatorTerritory:
                    ZoneDanger = 1.0f - (Dist / Zone.Radius); // Max danger at center
                    break;
                case ECrowd_ZoneType::WateringHole:
                    ZoneDanger = 0.4f; // Medium danger — predators hunt here
                    break;
                case ECrowd_ZoneType::FeedingGround:
                    ZoneDanger = 0.2f;
                    break;
                default:
                    ZoneDanger = 0.1f;
                    break;
            }
            MaxDanger = FMath::Max(MaxDanger, ZoneDanger);
        }
    }

    return MaxDanger;
}

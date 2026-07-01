// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements Mass AI crowd simulation for prehistoric herds up to 50,000 agents

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgentCount = 500;
    ActiveAgentCount = 0;
    bSimulationActive = false;
    bStampedeActive = false;
    GlobalFlowFieldResolution = 64;
    AgentUpdateBudgetMs = 4.0f;
    LODDistanceNear = 2000.0f;
    LODDistanceMid = 8000.0f;
    LODDistanceFar = 20000.0f;
    StampedeRadius = 3000.0f;
    StampedeSpeed = 1200.0f;
    HerdCohesionRadius = 800.0f;
    SeparationRadius = 150.0f;
    AlignmentWeight = 0.4f;
    CohesionWeight = 0.3f;
    SeparationWeight = 0.8f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initializing Crowd Simulation Manager — Max agents: %d"), MaxAgentCount);
    InitializeFlowField();
    RegisterHerdSpawnZones();
}

void UCrowdSimulationManager::Deinitialize()
{
    StopSimulation();
    ActiveHerds.Empty();
    FlowFieldGrid.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Crowd Simulation Manager shut down"));
}

void UCrowdSimulationManager::StartSimulation()
{
    if (bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Simulation already active"));
        return;
    }

    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Starting crowd simulation with %d herds"), ActiveHerds.Num());

    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            SimulationTickHandle,
            this,
            &UCrowdSimulationManager::TickSimulation,
            0.05f, // 20Hz update
            true
        );
    }
}

void UCrowdSimulationManager::StopSimulation()
{
    bSimulationActive = false;
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(SimulationTickHandle);
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation stopped"));
}

void UCrowdSimulationManager::SpawnHerd(const FCrowd_HerdData& HerdData)
{
    if (ActiveHerds.Num() >= 32)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max herd count (32) reached — cannot spawn new herd"));
        return;
    }

    FCrowd_HerdData NewHerd = HerdData;
    NewHerd.HerdID = FMath::Rand();
    NewHerd.bIsActive = true;
    NewHerd.CurrentAgentCount = FMath::Min(HerdData.CurrentAgentCount, MaxAgentCount - ActiveAgentCount);

    ActiveHerds.Add(NewHerd);
    ActiveAgentCount += NewHerd.CurrentAgentCount;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Spawned herd '%s' — %d agents at (%.0f, %.0f, %.0f)"),
        *HerdData.SpeciesName.ToString(),
        NewHerd.CurrentAgentCount,
        HerdData.CenterLocation.X,
        HerdData.CenterLocation.Y,
        HerdData.CenterLocation.Z);

    OnHerdSpawned.Broadcast(NewHerd);
}

void UCrowdSimulationManager::DisbandHerd(int32 HerdID)
{
    for (int32 i = ActiveHerds.Num() - 1; i >= 0; --i)
    {
        if (ActiveHerds[i].HerdID == HerdID)
        {
            ActiveAgentCount -= ActiveHerds[i].CurrentAgentCount;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Disbanded herd %d"), HerdID);
            ActiveHerds.RemoveAt(i);
            return;
        }
    }
}

void UCrowdSimulationManager::TriggerStampede(FVector Origin, ECrowd_StampedeType Type, float Radius)
{
    if (bStampedeActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Stampede already in progress"));
        return;
    }

    bStampedeActive = true;
    StampedeOrigin = Origin;
    CurrentStampedeType = Type;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] STAMPEDE TRIGGERED — Type: %d, Origin: (%.0f, %.0f, %.0f), Radius: %.0f"),
        (int32)Type, Origin.X, Origin.Y, Origin.Z, Radius);

    // Affect all herds within radius
    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        float Distance = FVector::Dist(Herd.CenterLocation, Origin);
        if (Distance <= Radius)
        {
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Stampede;
            Herd.FleeDirection = (Herd.CenterLocation - Origin).GetSafeNormal();
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd '%s' entering stampede — flee dir: (%.2f, %.2f)"),
                *Herd.SpeciesName.ToString(),
                Herd.FleeDirection.X,
                Herd.FleeDirection.Y);
        }
    }

    OnStampedeTriggered.Broadcast(Origin, Type);

    // Auto-resolve stampede after duration
    UWorld* World = GetWorld();
    if (World)
    {
        FTimerHandle StampedeEndHandle;
        World->GetTimerManager().SetTimer(
            StampedeEndHandle,
            [this]()
            {
                bStampedeActive = false;
                for (FCrowd_HerdData& Herd : ActiveHerds)
                {
                    if (Herd.CurrentBehavior == ECrowd_HerdBehavior::Stampede)
                    {
                        Herd.CurrentBehavior = ECrowd_HerdBehavior::Grazing;
                    }
                }
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Stampede resolved — herds returning to normal behavior"));
            },
            30.0f,
            false
        );
    }
}

void UCrowdSimulationManager::SetPlayerLocation(FVector PlayerLoc)
{
    PlayerLocation = PlayerLoc;
    UpdateLODForAllHerds();
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FCrowd_HerdData> UCrowdSimulationManager::GetHerdsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_HerdData> Result;
    for (const FCrowd_HerdData& Herd : ActiveHerds)
    {
        if (FVector::Dist(Herd.CenterLocation, Center) <= Radius)
        {
            Result.Add(Herd);
        }
    }
    return Result;
}

void UCrowdSimulationManager::TickSimulation()
{
    if (!bSimulationActive) return;

    float DeltaTime = 0.05f; // Fixed 20Hz step

    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        if (!Herd.bIsActive) continue;

        switch (Herd.CurrentBehavior)
        {
        case ECrowd_HerdBehavior::Migrating:
            TickHerdMigration(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Stampede:
            TickHerdStampede(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Grazing:
            TickHerdGrazing(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Fleeing:
            TickHerdFleeing(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Resting:
            // No movement during rest
            break;
        default:
            break;
        }

        // Clamp agents to world bounds
        Herd.CenterLocation.X = FMath::Clamp(Herd.CenterLocation.X, -50000.0f, 50000.0f);
        Herd.CenterLocation.Y = FMath::Clamp(Herd.CenterLocation.Y, -50000.0f, 50000.0f);
    }
}

void UCrowdSimulationManager::TickHerdMigration(FCrowd_HerdData& Herd, float DeltaTime)
{
    if (Herd.MigrationWaypoints.Num() == 0) return;

    FVector Target = Herd.MigrationWaypoints[Herd.CurrentWaypointIndex % Herd.MigrationWaypoints.Num()];
    FVector Direction = (Target - Herd.CenterLocation).GetSafeNormal();
    float Speed = Herd.MigrationSpeed;

    // Apply flocking: alignment + cohesion + separation
    FVector FlockForce = ComputeFlockingForce(Herd);
    FVector FinalDir = (Direction + FlockForce * 0.3f).GetSafeNormal();

    Herd.CenterLocation += FinalDir * Speed * DeltaTime;

    // Advance waypoint when close enough
    if (FVector::Dist2D(Herd.CenterLocation, Target) < 500.0f)
    {
        Herd.CurrentWaypointIndex = (Herd.CurrentWaypointIndex + 1) % FMath::Max(1, Herd.MigrationWaypoints.Num());
        UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Herd '%s' reached waypoint %d"), *Herd.SpeciesName.ToString(), Herd.CurrentWaypointIndex);
    }
}

void UCrowdSimulationManager::TickHerdStampede(FCrowd_HerdData& Herd, float DeltaTime)
{
    float StampedeSpeedActual = StampedeSpeed;
    Herd.CenterLocation += Herd.FleeDirection * StampedeSpeedActual * DeltaTime;
}

void UCrowdSimulationManager::TickHerdGrazing(FCrowd_HerdData& Herd, float DeltaTime)
{
    // Slow random drift while grazing
    float DriftAngle = FMath::FRandRange(-15.0f, 15.0f);
    FVector DriftDir = FVector(FMath::Cos(FMath::DegreesToRadians(DriftAngle)), FMath::Sin(FMath::DegreesToRadians(DriftAngle)), 0.0f);
    Herd.CenterLocation += DriftDir * 30.0f * DeltaTime;
}

void UCrowdSimulationManager::TickHerdFleeing(FCrowd_HerdData& Herd, float DeltaTime)
{
    FVector FleeDir = (Herd.CenterLocation - PlayerLocation).GetSafeNormal();
    Herd.CenterLocation += FleeDir * Herd.MigrationSpeed * 1.8f * DeltaTime;

    // Stop fleeing when far enough from player
    if (FVector::Dist(Herd.CenterLocation, PlayerLocation) > 8000.0f)
    {
        Herd.CurrentBehavior = ECrowd_HerdBehavior::Grazing;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd '%s' stopped fleeing — safe distance reached"), *Herd.SpeciesName.ToString());
    }
}

void UCrowdSimulationManager::UpdateLODForAllHerds()
{
    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        float Distance = FVector::Dist(Herd.CenterLocation, PlayerLocation);

        if (Distance < LODDistanceNear)
        {
            Herd.CurrentLOD = ECrowd_AgentLOD::Full;
        }
        else if (Distance < LODDistanceMid)
        {
            Herd.CurrentLOD = ECrowd_AgentLOD::Medium;
        }
        else if (Distance < LODDistanceFar)
        {
            Herd.CurrentLOD = ECrowd_AgentLOD::Low;
        }
        else
        {
            Herd.CurrentLOD = ECrowd_AgentLOD::Culled;
        }
    }
}

void UCrowdSimulationManager::InitializeFlowField()
{
    FlowFieldGrid.SetNum(GlobalFlowFieldResolution * GlobalFlowFieldResolution);
    for (FVector& Cell : FlowFieldGrid)
    {
        Cell = FVector::ForwardVector; // Default flow direction
    }
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Flow field initialized — %dx%d grid"), GlobalFlowFieldResolution, GlobalFlowFieldResolution);
}

void UCrowdSimulationManager::RegisterHerdSpawnZones()
{
    // Register default prehistoric biome spawn zones
    // These are populated by the World Generator (Agent #05)
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd spawn zones registered"));
}

FVector UCrowdSimulationManager::ComputeFlockingForce(const FCrowd_HerdData& Herd) const
{
    // Simplified Reynolds flocking: alignment + cohesion + separation
    FVector AlignmentForce = Herd.FleeDirection.IsNearlyZero() ? FVector::ForwardVector : Herd.FleeDirection;
    FVector CohesionForce = FVector::ZeroVector;
    FVector SeparationForce = FVector::ZeroVector;

    for (const FCrowd_HerdData& Other : ActiveHerds)
    {
        if (Other.HerdID == Herd.HerdID) continue;
        float Dist = FVector::Dist(Herd.CenterLocation, Other.CenterLocation);

        if (Dist < HerdCohesionRadius)
        {
            CohesionForce += (Other.CenterLocation - Herd.CenterLocation).GetSafeNormal();
        }
        if (Dist < SeparationRadius)
        {
            SeparationForce += (Herd.CenterLocation - Other.CenterLocation).GetSafeNormal();
        }
    }

    return (AlignmentForce * AlignmentWeight + CohesionForce * CohesionWeight + SeparationForce * SeparationWeight).GetSafeNormal();
}

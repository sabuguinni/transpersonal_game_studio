// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — Mass AI crowd simulation for up to 50,000 agents

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Actor.h"

// ============================================================
// UCrowdSimulationManager — UObject subsystem for crowd logic
// ============================================================

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxActiveAgents = 200;
    LODDistanceNear = 1500.0f;
    LODDistanceMid  = 4000.0f;
    LODDistanceFar  = 8000.0f;
    bSimulationActive = false;
    CurrentAgentCount = 0;
    DangerAlertRadius = 800.0f;
    bDangerStateActive = false;
}

void UCrowdSimulationManager::InitializeSimulation(UWorld* InWorld)
{
    if (!InWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] InitializeSimulation called with null world"));
        return;
    }

    WorldRef = InWorld;
    bSimulationActive = true;
    CurrentAgentCount = 0;

    // Register default activity zones
    RegisterActivityZone(ECrowd_ActivityZone::Camp,       FVector(0.0f, 0.0f, 0.0f),    500.0f, 20);
    RegisterActivityZone(ECrowd_ActivityZone::Foraging,   FVector(600.0f, 250.0f, 0.0f), 300.0f, 15);
    RegisterActivityZone(ECrowd_ActivityZone::WaterSource,FVector(-500.0f, 325.0f, 0.0f),200.0f, 10);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation initialized. Zones: %d, MaxAgents: %d"),
        ActivityZones.Num(), MaxActiveAgents);
}

void UCrowdSimulationManager::RegisterActivityZone(
    ECrowd_ActivityZone ZoneType,
    FVector Location,
    float Radius,
    int32 MaxOccupants)
{
    FCrowd_ActivityZoneData Zone;
    Zone.ZoneType     = ZoneType;
    Zone.Location     = Location;
    Zone.Radius       = Radius;
    Zone.MaxOccupants = MaxOccupants;
    Zone.CurrentOccupants = 0;
    Zone.bIsActive    = true;

    ActivityZones.Add(Zone);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Zone registered: Type=%d Loc=(%.0f,%.0f) R=%.0f MaxOcc=%d"),
        (int32)ZoneType, Location.X, Location.Y, Radius, MaxOccupants);
}

void UCrowdSimulationManager::SpawnCrowdAgent(
    ECrowd_AgentRole Role,
    FVector SpawnLocation,
    ECrowd_ActivityZone InitialZone)
{
    if (!bSimulationActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Cannot spawn agent — simulation not active"));
        return;
    }

    if (CurrentAgentCount >= MaxActiveAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Agent cap reached: %d/%d"), CurrentAgentCount, MaxActiveAgents);
        return;
    }

    FCrowd_AgentData Agent;
    Agent.AgentID       = FGuid::NewGuid();
    Agent.Role          = Role;
    Agent.CurrentLocation = SpawnLocation;
    Agent.TargetLocation  = SpawnLocation;
    Agent.CurrentZone   = InitialZone;
    Agent.State         = ECrowd_AgentState::Idle;
    Agent.LODLevel      = ECrowd_LODLevel::Near;
    Agent.bIsAlive      = true;
    Agent.FearLevel     = 0.0f;
    Agent.StaminaLevel  = 1.0f;

    ActiveAgents.Add(Agent);
    CurrentAgentCount++;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Agent spawned: Role=%d Zone=%d Total=%d"),
        (int32)Role, (int32)InitialZone, CurrentAgentCount);
}

void UCrowdSimulationManager::TriggerDangerAlert(FVector DangerSource, float AlertRadius)
{
    bDangerStateActive = true;
    DangerAlertRadius  = AlertRadius;

    int32 AffectedCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.CurrentLocation, DangerSource);
        if (Dist <= AlertRadius)
        {
            Agent.State     = ECrowd_AgentState::Fleeing;
            Agent.FearLevel = FMath::Clamp(1.0f - (Dist / AlertRadius), 0.2f, 1.0f);

            // Flee away from danger source
            FVector FleeDir = (Agent.CurrentLocation - DangerSource).GetSafeNormal();
            Agent.TargetLocation = Agent.CurrentLocation + FleeDir * (AlertRadius * 1.5f);
            AffectedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] DANGER ALERT at (%.0f,%.0f,%.0f) R=%.0f — %d agents fleeing"),
        DangerSource.X, DangerSource.Y, DangerSource.Z, AlertRadius, AffectedCount);
}

void UCrowdSimulationManager::UpdateAgentLOD(FVector PlayerLocation)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Dist = FVector::Dist(Agent.CurrentLocation, PlayerLocation);

        if (Dist <= LODDistanceNear)
        {
            Agent.LODLevel = ECrowd_LODLevel::Near;
        }
        else if (Dist <= LODDistanceMid)
        {
            Agent.LODLevel = ECrowd_LODLevel::Mid;
        }
        else if (Dist <= LODDistanceFar)
        {
            Agent.LODLevel = ECrowd_LODLevel::Far;
        }
        else
        {
            Agent.LODLevel = ECrowd_LODLevel::Culled;
        }
    }
}

void UCrowdSimulationManager::TickSimulation(float DeltaTime)
{
    if (!bSimulationActive) return;

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsAlive) continue;
        if (Agent.LODLevel == ECrowd_LODLevel::Culled) continue;

        // Simple state machine tick
        switch (Agent.State)
        {
            case ECrowd_AgentState::Idle:
                // Occasionally transition to moving
                Agent.StaminaLevel = FMath::Min(1.0f, Agent.StaminaLevel + DeltaTime * 0.1f);
                break;

            case ECrowd_AgentState::Moving:
            {
                FVector Dir = (Agent.TargetLocation - Agent.CurrentLocation).GetSafeNormal();
                float Speed = (Agent.LODLevel == ECrowd_LODLevel::Near) ? 150.0f : 100.0f;
                Agent.CurrentLocation += Dir * Speed * DeltaTime;

                float DistToTarget = FVector::Dist(Agent.CurrentLocation, Agent.TargetLocation);
                if (DistToTarget < 50.0f)
                {
                    Agent.State = ECrowd_AgentState::Idle;
                }
                break;
            }

            case ECrowd_AgentState::Fleeing:
            {
                FVector Dir = (Agent.TargetLocation - Agent.CurrentLocation).GetSafeNormal();
                Agent.CurrentLocation += Dir * 300.0f * DeltaTime;
                Agent.FearLevel = FMath::Max(0.0f, Agent.FearLevel - DeltaTime * 0.05f);

                if (Agent.FearLevel <= 0.0f)
                {
                    Agent.State = ECrowd_AgentState::Idle;
                }
                break;
            }

            case ECrowd_AgentState::Working:
                Agent.StaminaLevel = FMath::Max(0.0f, Agent.StaminaLevel - DeltaTime * 0.02f);
                if (Agent.StaminaLevel <= 0.1f)
                {
                    Agent.State = ECrowd_AgentState::Idle;
                }
                break;

            default:
                break;
        }
    }
}

int32 UCrowdSimulationManager::GetAgentCountByRole(ECrowd_AgentRole Role) const
{
    int32 Count = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.Role == Role && Agent.bIsAlive) Count++;
    }
    return Count;
}

void UCrowdSimulationManager::ShutdownSimulation()
{
    bSimulationActive = false;
    ActiveAgents.Empty();
    ActivityZones.Empty();
    CurrentAgentCount = 0;
    bDangerStateActive = false;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Simulation shut down."));
}

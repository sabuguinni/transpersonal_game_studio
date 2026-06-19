// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric crowd simulation: herds, migration corridors, LOD zones, panic scatter

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

// ============================================================
// UCrowdSimulationManager — UWorldSubsystem implementation
// ============================================================

UCrowdSimulationManager::UCrowdSimulationManager()
{
    bSimulationActive = false;
    GlobalDensityScale = 1.0f;
    MaxSimulatedAgents = 500;
    CurrentAgentCount = 0;
    PanicDecayRate = 0.05f;
    GlobalPanicLevel = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Register default migration corridor waypoints
    // These match the in-editor markers placed by Agent #13
    FCrowd_MigrationWaypoint WP1, WP2, WP3, WP4, WP5;
    WP1.WorldLocation = FVector(2000.f,  3000.f, 100.f);
    WP1.WaypointID    = TEXT("MigWP_North_001");
    WP1.CrowdDensity  = 0.3f;

    WP2.WorldLocation = FVector(1500.f,  1500.f, 80.f);
    WP2.WaypointID    = TEXT("MigWP_Plains_002");
    WP2.CrowdDensity  = 0.5f;

    WP3.WorldLocation = FVector(800.f,   0.f,    60.f);
    WP3.WaypointID    = TEXT("MigWP_Crossing_003");
    WP3.CrowdDensity  = 0.8f;

    WP4.WorldLocation = FVector(200.f,  -1500.f, 50.f);
    WP4.WaypointID    = TEXT("MigWP_River_004");
    WP4.CrowdDensity  = 0.6f;

    WP5.WorldLocation = FVector(-500.f, -3000.f, 40.f);
    WP5.WaypointID    = TEXT("MigWP_South_005");
    WP5.CrowdDensity  = 0.4f;

    MigrationCorridor.Add(WP1);
    MigrationCorridor.Add(WP2);
    MigrationCorridor.Add(WP3);
    MigrationCorridor.Add(WP4);
    MigrationCorridor.Add(WP5);

    // Register LOD zones
    FCrowd_LODZone HighLOD, MedLOD1, MedLOD2, LowLOD1, LowLOD2;

    HighLOD.ZoneCenter      = FVector(0.f,    0.f,    0.f);
    HighLOD.ZoneRadius      = 800.f;
    HighLOD.LODLevel        = ECrowd_LODLevel::High;
    HighLOD.MaxAgentsInZone = 50;

    MedLOD1.ZoneCenter      = FVector(1000.f, 1000.f, 0.f);
    MedLOD1.ZoneRadius      = 1200.f;
    MedLOD1.LODLevel        = ECrowd_LODLevel::Medium;
    MedLOD1.MaxAgentsInZone = 150;

    MedLOD2.ZoneCenter      = FVector(-800.f, 800.f,  0.f);
    MedLOD2.ZoneRadius      = 1200.f;
    MedLOD2.LODLevel        = ECrowd_LODLevel::Medium;
    MedLOD2.MaxAgentsInZone = 150;

    LowLOD1.ZoneCenter      = FVector(2500.f, 2500.f, 0.f);
    LowLOD1.ZoneRadius      = 2000.f;
    LowLOD1.LODLevel        = ECrowd_LODLevel::Low;
    LowLOD1.MaxAgentsInZone = 300;

    LowLOD2.ZoneCenter      = FVector(-2000.f, 1500.f, 0.f);
    LowLOD2.ZoneRadius      = 2000.f;
    LowLOD2.LODLevel        = ECrowd_LODLevel::Low;
    LowLOD2.MaxAgentsInZone = 300;

    LODZones.Add(HighLOD);
    LODZones.Add(MedLOD1);
    LODZones.Add(MedLOD2);
    LODZones.Add(LowLOD1);
    LODZones.Add(LowLOD2);

    // Register grazing zones
    FCrowd_GrazingZone GZ1, GZ2, GZ3;

    GZ1.ZoneCenter    = FVector(1800.f,  2200.f, 80.f);
    GZ1.ZoneRadius    = 500.f;
    GZ1.ZoneID        = TEXT("GrazingZone_Plains_001");
    GZ1.MaxHerdSize   = 12;
    GZ1.bIsActive     = true;

    GZ2.ZoneCenter    = FVector(300.f,  -1200.f, 50.f);
    GZ2.ZoneRadius    = 400.f;
    GZ2.ZoneID        = TEXT("GrazingZone_River_002");
    GZ2.MaxHerdSize   = 8;
    GZ2.bIsActive     = true;

    GZ3.ZoneCenter    = FVector(-1500.f, 500.f,  90.f);
    GZ3.ZoneRadius    = 600.f;
    GZ3.ZoneID        = TEXT("GrazingZone_Forest_003");
    GZ3.MaxHerdSize   = 6;
    GZ3.bIsActive     = true;

    GrazingZones.Add(GZ1);
    GrazingZones.Add(GZ2);
    GrazingZones.Add(GZ3);

    bSimulationActive = true;

    // Start panic decay timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PanicDecayTimerHandle,
            this,
            &UCrowdSimulationManager::TickPanicDecay,
            0.5f,
            true
        );
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — %d waypoints, %d LOD zones, %d grazing zones"),
        MigrationCorridor.Num(), LODZones.Num(), GrazingZones.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PanicDecayTimerHandle);
    }
    bSimulationActive = false;
    Super::Deinitialize();
}

// ============================================================
// Migration Corridor
// ============================================================

FCrowd_MigrationWaypoint UCrowdSimulationManager::GetNextMigrationWaypoint(int32 CurrentIndex) const
{
    if (MigrationCorridor.IsEmpty())
    {
        return FCrowd_MigrationWaypoint();
    }
    int32 NextIndex = (CurrentIndex + 1) % MigrationCorridor.Num();
    return MigrationCorridor[NextIndex];
}

int32 UCrowdSimulationManager::GetMigrationWaypointCount() const
{
    return MigrationCorridor.Num();
}

FVector UCrowdSimulationManager::GetMigrationDirectionAt(int32 WaypointIndex) const
{
    if (MigrationCorridor.Num() < 2) return FVector::ForwardVector;

    int32 Current = FMath::Clamp(WaypointIndex, 0, MigrationCorridor.Num() - 1);
    int32 Next    = (Current + 1) % MigrationCorridor.Num();

    FVector Dir = MigrationCorridor[Next].WorldLocation - MigrationCorridor[Current].WorldLocation;
    Dir.Normalize();
    return Dir;
}

// ============================================================
// LOD System
// ============================================================

ECrowd_LODLevel UCrowdSimulationManager::GetLODLevelForLocation(const FVector& WorldLocation) const
{
    // Find smallest zone that contains this location
    float SmallestRadius = MAX_FLT;
    ECrowd_LODLevel BestLOD = ECrowd_LODLevel::Culled;

    for (const FCrowd_LODZone& Zone : LODZones)
    {
        float Dist = FVector::Dist(WorldLocation, Zone.ZoneCenter);
        if (Dist <= Zone.ZoneRadius && Zone.ZoneRadius < SmallestRadius)
        {
            SmallestRadius = Zone.ZoneRadius;
            BestLOD = Zone.LODLevel;
        }
    }
    return BestLOD;
}

int32 UCrowdSimulationManager::GetMaxAgentsForLOD(ECrowd_LODLevel LODLevel) const
{
    switch (LODLevel)
    {
        case ECrowd_LODLevel::High:   return 50;
        case ECrowd_LODLevel::Medium: return 150;
        case ECrowd_LODLevel::Low:    return 300;
        case ECrowd_LODLevel::Culled: return 0;
        default:                      return 0;
    }
}

// ============================================================
// Panic System
// ============================================================

void UCrowdSimulationManager::TriggerPanicAt(const FVector& EpicenterLocation, float PanicRadius, float PanicIntensity)
{
    // Increase global panic proportional to intensity
    GlobalPanicLevel = FMath::Clamp(GlobalPanicLevel + PanicIntensity * 0.3f, 0.f, 1.f);

    // Record panic event
    FCrowd_PanicEvent Event;
    Event.Epicenter      = EpicenterLocation;
    Event.Radius         = PanicRadius;
    Event.Intensity      = PanicIntensity;
    Event.TimeTriggered  = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    ActivePanicEvents.Add(Event);

    UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] PANIC triggered at (%.0f,%.0f) radius=%.0f intensity=%.2f GlobalPanic=%.2f"),
        EpicenterLocation.X, EpicenterLocation.Y, PanicRadius, PanicIntensity, GlobalPanicLevel);

    // Broadcast panic to all grazing zones in range
    for (FCrowd_GrazingZone& Zone : GrazingZones)
    {
        float DistToZone = FVector::Dist(EpicenterLocation, Zone.ZoneCenter);
        if (DistToZone <= PanicRadius + Zone.ZoneRadius)
        {
            Zone.bIsActive = false; // Herd scatters — zone temporarily inactive
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Zone %s scattered by panic"), *Zone.ZoneID);
        }
    }
}

float UCrowdSimulationManager::GetPanicLevelAt(const FVector& Location) const
{
    float MaxPanic = 0.f;
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

    for (const FCrowd_PanicEvent& Event : ActivePanicEvents)
    {
        float Dist = FVector::Dist(Location, Event.Epicenter);
        if (Dist <= Event.Radius)
        {
            float DistFactor = 1.f - (Dist / Event.Radius);
            float TimeFactor = FMath::Clamp(1.f - (Now - Event.TimeTriggered) * PanicDecayRate, 0.f, 1.f);
            float LocalPanic = Event.Intensity * DistFactor * TimeFactor;
            MaxPanic = FMath::Max(MaxPanic, LocalPanic);
        }
    }
    return MaxPanic;
}

void UCrowdSimulationManager::TickPanicDecay()
{
    // Decay global panic
    GlobalPanicLevel = FMath::Max(0.f, GlobalPanicLevel - PanicDecayRate);

    // Remove expired panic events (older than 60s)
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    ActivePanicEvents.RemoveAll([Now](const FCrowd_PanicEvent& E)
    {
        return (Now - E.TimeTriggered) > 60.f;
    });

    // Re-activate grazing zones when panic subsides
    if (GlobalPanicLevel < 0.1f)
    {
        for (FCrowd_GrazingZone& Zone : GrazingZones)
        {
            Zone.bIsActive = true;
        }
    }
}

// ============================================================
// Grazing System
// ============================================================

const FCrowd_GrazingZone* UCrowdSimulationManager::GetNearestGrazingZone(const FVector& Location) const
{
    const FCrowd_GrazingZone* Best = nullptr;
    float BestDist = MAX_FLT;

    for (const FCrowd_GrazingZone& Zone : GrazingZones)
    {
        if (!Zone.bIsActive) continue;
        float Dist = FVector::Dist(Location, Zone.ZoneCenter);
        if (Dist < BestDist)
        {
            BestDist = Dist;
            Best = &Zone;
        }
    }
    return Best;
}

bool UCrowdSimulationManager::IsLocationInGrazingZone(const FVector& Location) const
{
    for (const FCrowd_GrazingZone& Zone : GrazingZones)
    {
        if (!Zone.bIsActive) continue;
        if (FVector::Dist(Location, Zone.ZoneCenter) <= Zone.ZoneRadius)
        {
            return true;
        }
    }
    return false;
}

// ============================================================
// Agent Registration
// ============================================================

void UCrowdSimulationManager::RegisterAgent(int32 AgentID, const FVector& InitialLocation)
{
    if (CurrentAgentCount >= MaxSimulatedAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Agent cap reached (%d). Cannot register agent %d."),
            MaxSimulatedAgents, AgentID);
        return;
    }

    FCrowd_AgentState State;
    State.AgentID       = AgentID;
    State.Location      = InitialLocation;
    State.Velocity      = FVector::ZeroVector;
    State.CurrentState  = ECrowd_AgentState::Grazing;
    State.LODLevel      = GetLODLevelForLocation(InitialLocation);

    AgentStates.Add(AgentID, State);
    CurrentAgentCount++;
}

void UCrowdSimulationManager::UnregisterAgent(int32 AgentID)
{
    if (AgentStates.Remove(AgentID) > 0)
    {
        CurrentAgentCount = FMath::Max(0, CurrentAgentCount - 1);
    }
}

void UCrowdSimulationManager::UpdateAgentState(int32 AgentID, const FVector& NewLocation, const FVector& NewVelocity)
{
    if (FCrowd_AgentState* State = AgentStates.Find(AgentID))
    {
        State->Location  = NewLocation;
        State->Velocity  = NewVelocity;
        State->LODLevel  = GetLODLevelForLocation(NewLocation);

        // Update behavior state based on panic
        float LocalPanic = GetPanicLevelAt(NewLocation);
        if (LocalPanic > 0.7f)
        {
            State->CurrentState = ECrowd_AgentState::Fleeing;
        }
        else if (LocalPanic > 0.3f)
        {
            State->CurrentState = ECrowd_AgentState::Alert;
        }
        else if (IsLocationInGrazingZone(NewLocation))
        {
            State->CurrentState = ECrowd_AgentState::Grazing;
        }
        else
        {
            State->CurrentState = ECrowd_AgentState::Migrating;
        }
    }
}

int32 UCrowdSimulationManager::GetCurrentAgentCount() const
{
    return CurrentAgentCount;
}

float UCrowdSimulationManager::GetGlobalPanicLevel() const
{
    return GlobalPanicLevel;
}

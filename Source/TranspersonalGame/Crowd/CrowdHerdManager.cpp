// CrowdHerdManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Manages herd groups: formation, migration, LOD, panic propagation
// ============================================================

#include "CrowdHerdManager.h"
#include "CrowdBehaviorTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

// ---------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------
UCrowdHerdManager::UCrowdHerdManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for crowd update

    MaxActiveAgents = 50000;
    LODConfig = FCrowd_LODConfig();
    NextHerdID = 0;
    TotalActiveAgents = 0;
}

// ---------------------------------------------------------------
// BeginPlay
// ---------------------------------------------------------------
void UCrowdHerdManager::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    if (!World) return;

    // Schedule periodic migration path recalculation
    World->GetTimerManager().SetTimer(
        MigrationTimerHandle,
        this,
        &UCrowdHerdManager::UpdateMigrationPaths,
        30.0f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdManager: Initialized. MaxAgents=%d"), MaxActiveAgents);
}

// ---------------------------------------------------------------
// TickComponent
// ---------------------------------------------------------------
void UCrowdHerdManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateLODLevels();
    UpdateStampedeStates(DeltaTime);
    UpdateHerdCentroids();
}

// ---------------------------------------------------------------
// RegisterHerd — add a new herd to the manager
// ---------------------------------------------------------------
int32 UCrowdHerdManager::RegisterHerd(const FString& SpeciesName, int32 AgentCount, FVector SpawnCentre)
{
    FCrowd_HerdDescriptor NewHerd;
    NewHerd.HerdID = NextHerdID++;
    NewHerd.SpeciesName = SpeciesName;
    NewHerd.AgentCount = AgentCount;
    NewHerd.CentroidLocation = SpawnCentre;
    NewHerd.StampedePhase = ECrowd_StampedePhase::Inactive;
    NewHerd.ThreatLevel = ECrowd_ThreatLevel::None;
    NewHerd.bIsGrazing = true;
    NewHerd.MigrationProgress = 0.0f;

    // Default speed by species
    if (SpeciesName.Contains(TEXT("Brachiosaurus")))
        NewHerd.AverageSpeed = 150.0f;
    else if (SpeciesName.Contains(TEXT("Triceratops")))
        NewHerd.AverageSpeed = 220.0f;
    else if (SpeciesName.Contains(TEXT("Parasaurolophus")))
        NewHerd.AverageSpeed = 280.0f;
    else
        NewHerd.AverageSpeed = 200.0f;

    ActiveHerds.Add(NewHerd);
    TotalActiveAgents += AgentCount;

    UE_LOG(LogTemp, Log, TEXT("CrowdHerdManager: Registered herd %d (%s) with %d agents at %s"),
        NewHerd.HerdID, *SpeciesName, AgentCount, *SpawnCentre.ToString());

    return NewHerd.HerdID;
}

// ---------------------------------------------------------------
// TriggerHerdPanic — initiate stampede for a specific herd
// ---------------------------------------------------------------
void UCrowdHerdManager::TriggerHerdPanic(int32 HerdID, FVector ThreatLocation, float PanicIntensity)
{
    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        if (Herd.HerdID != HerdID) continue;

        Herd.StampedePhase = ECrowd_StampedePhase::Triggering;
        Herd.ThreatLevel = PanicIntensity >= 0.8f ? ECrowd_ThreatLevel::Critical
                         : PanicIntensity >= 0.5f ? ECrowd_ThreatLevel::High
                         : PanicIntensity >= 0.3f ? ECrowd_ThreatLevel::Medium
                         : ECrowd_ThreatLevel::Low;
        Herd.bIsGrazing = false;

        // Compute flee direction — away from threat
        FVector FleeDir = (Herd.CentroidLocation - ThreatLocation).GetSafeNormal();

        // Broadcast stampede event
        FCrowd_StampedeEvent Event;
        Event.Origin = ThreatLocation;
        Event.Radius = 3000.0f * PanicIntensity;
        Event.PanicIntensity = PanicIntensity;
        Event.FleeDirection = FleeDir;
        Event.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        Event.TriggerHerdID = HerdID;

        ActiveStampedeEvents.Add(Event);

        UE_LOG(LogTemp, Warning, TEXT("CrowdHerdManager: Herd %d (%s) PANIC triggered! Intensity=%.2f FleeDir=%s"),
            HerdID, *Herd.SpeciesName, PanicIntensity, *FleeDir.ToString());

        // Propagate to nearby herds
        PropagateStampedeToNearbyHerds(Event);
        break;
    }
}

// ---------------------------------------------------------------
// PropagateStampedeToNearbyHerds — chain reaction
// ---------------------------------------------------------------
void UCrowdHerdManager::PropagateStampedeToNearbyHerds(const FCrowd_StampedeEvent& Event)
{
    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        if (Herd.HerdID == Event.TriggerHerdID) continue;
        if (Herd.StampedePhase != ECrowd_StampedePhase::Inactive) continue;

        float Distance = FVector::Dist(Herd.CentroidLocation, Event.Origin);
        if (Distance <= Event.Radius)
        {
            // Attenuate panic by distance
            float AttenuatedPanic = Event.PanicIntensity * (1.0f - Distance / Event.Radius);
            if (AttenuatedPanic >= 0.2f)
            {
                Herd.StampedePhase = ECrowd_StampedePhase::Propagating;
                Herd.ThreatLevel = AttenuatedPanic >= 0.5f ? ECrowd_ThreatLevel::High : ECrowd_ThreatLevel::Medium;
                Herd.bIsGrazing = false;
                UE_LOG(LogTemp, Log, TEXT("CrowdHerdManager: Stampede propagated to herd %d (%s) panic=%.2f"),
                    Herd.HerdID, *Herd.SpeciesName, AttenuatedPanic);
            }
        }
    }
}

// ---------------------------------------------------------------
// UpdateLODLevels — assign LOD tier to each agent based on distance to player
// ---------------------------------------------------------------
void UCrowdHerdManager::UpdateLODLevels()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (FCrowd_AgentData& Agent : AllAgents)
    {
        float Dist = FVector::Dist(Agent.Location, PlayerLoc);

        if (Dist <= LODConfig.FullDetailRadius * 100.0f)
            Agent.LOD = ECrowd_LODLevel::Full;
        else if (Dist <= LODConfig.MediumDetailRadius * 100.0f)
            Agent.LOD = ECrowd_LODLevel::Medium;
        else if (Dist <= LODConfig.LowDetailRadius * 100.0f)
            Agent.LOD = ECrowd_LODLevel::Low;
        else
            Agent.LOD = ECrowd_LODLevel::Ghost;
    }
}

// ---------------------------------------------------------------
// UpdateStampedeStates — advance stampede phase state machine
// ---------------------------------------------------------------
void UCrowdHerdManager::UpdateStampedeStates(float DeltaTime)
{
    float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        switch (Herd.StampedePhase)
        {
        case ECrowd_StampedePhase::Triggering:
            Herd.StampedePhase = ECrowd_StampedePhase::FullPanic;
            break;

        case ECrowd_StampedePhase::Propagating:
            Herd.StampedePhase = ECrowd_StampedePhase::FullPanic;
            break;

        case ECrowd_StampedePhase::FullPanic:
            // After 30s of full panic, begin dissipation
            {
                float PanicDuration = 30.0f;
                // Reduce threat level over time
                if (Herd.ThreatLevel > ECrowd_ThreatLevel::None)
                {
                    uint8 ThreatVal = static_cast<uint8>(Herd.ThreatLevel);
                    if (ThreatVal > 0)
                    {
                        // Gradually reduce (simplified — real impl uses timer)
                        Herd.StampedePhase = ECrowd_StampedePhase::Dissipating;
                    }
                }
            }
            break;

        case ECrowd_StampedePhase::Dissipating:
            Herd.StampedePhase = ECrowd_StampedePhase::Inactive;
            Herd.ThreatLevel = ECrowd_ThreatLevel::None;
            Herd.bIsGrazing = true;
            UE_LOG(LogTemp, Log, TEXT("CrowdHerdManager: Herd %d (%s) stampede dissipated — returning to grazing"),
                Herd.HerdID, *Herd.SpeciesName);
            break;

        default:
            break;
        }
    }

    // Clean up expired stampede events (older than 60s)
    ActiveStampedeEvents.RemoveAll([Now](const FCrowd_StampedeEvent& E) {
        return (Now - E.Timestamp) > 60.0f;
    });
}

// ---------------------------------------------------------------
// UpdateHerdCentroids — recalculate centroid from agent positions
// ---------------------------------------------------------------
void UCrowdHerdManager::UpdateHerdCentroids()
{
    // Group agents by HerdID and compute average position
    TMap<int32, TPair<FVector, int32>> HerdSums;

    for (const FCrowd_AgentData& Agent : AllAgents)
    {
        if (Agent.HerdID < 0) continue;
        auto& Entry = HerdSums.FindOrAdd(Agent.HerdID);
        Entry.Key += Agent.Location;
        Entry.Value++;
    }

    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        if (HerdSums.Contains(Herd.HerdID))
        {
            auto& Entry = HerdSums[Herd.HerdID];
            if (Entry.Value > 0)
                Herd.CentroidLocation = Entry.Key / static_cast<float>(Entry.Value);
        }
    }
}

// ---------------------------------------------------------------
// UpdateMigrationPaths — recalculate waypoints for migrating herds
// ---------------------------------------------------------------
void UCrowdHerdManager::UpdateMigrationPaths()
{
    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        if (Herd.StampedePhase != ECrowd_StampedePhase::Inactive) continue;
        if (!Herd.bIsGrazing) continue;

        // Simple migration: advance along existing waypoints
        if (Herd.WaypointPath.Num() > 1)
        {
            Herd.MigrationProgress = FMath::Clamp(Herd.MigrationProgress + 0.05f, 0.0f, 1.0f);
            if (Herd.MigrationProgress >= 1.0f)
            {
                // Reached end of path — reverse or loop
                Herd.WaypointPath.Add(Herd.WaypointPath[0]);
                Herd.WaypointPath.RemoveAt(0);
                Herd.MigrationProgress = 0.0f;
                UE_LOG(LogTemp, Log, TEXT("CrowdHerdManager: Herd %d completed migration waypoint"), Herd.HerdID);
            }
        }
    }
}

// ---------------------------------------------------------------
// GetHerdByID — safe accessor
// ---------------------------------------------------------------
FCrowd_HerdDescriptor* UCrowdHerdManager::GetHerdByID(int32 HerdID)
{
    for (FCrowd_HerdDescriptor& Herd : ActiveHerds)
    {
        if (Herd.HerdID == HerdID) return &Herd;
    }
    return nullptr;
}

// ---------------------------------------------------------------
// GetTotalActiveAgents
// ---------------------------------------------------------------
int32 UCrowdHerdManager::GetTotalActiveAgents() const
{
    return TotalActiveAgents;
}

// ---------------------------------------------------------------
// GetActiveHerdCount
// ---------------------------------------------------------------
int32 UCrowdHerdManager::GetActiveHerdCount() const
{
    return ActiveHerds.Num();
}

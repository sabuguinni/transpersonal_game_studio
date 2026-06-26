// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric crowd simulation: herd AI, territory zones, LOD management

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UCrowdSimulationManager — UObject lifecycle
// ============================================================

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50000;
    ActiveAgentCount = 0;
    LODUpdateInterval = 0.5f;
    HerdUpdateInterval = 1.0f;
    bSimulationActive = false;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — max agents: %d"), MaxCrowdAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    HerdGroups.Empty();
    TerritoryZones.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Deinitialized"));
}

// ============================================================
// Herd Registration
// ============================================================

void UCrowdSimulationManager::RegisterHerdGroup(const FCrowd_HerdGroup& HerdGroup)
{
    if (HerdGroups.Num() >= MaxCrowdAgents / 10)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max herd groups reached"));
        return;
    }
    HerdGroups.Add(HerdGroup);
    ActiveAgentCount += HerdGroup.Members.Num();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd registered: %s (%d members)"),
        *HerdGroup.HerdID.ToString(), HerdGroup.Members.Num());
}

void UCrowdSimulationManager::UnregisterHerdGroup(const FName& HerdID)
{
    int32 Removed = HerdGroups.RemoveAll([&](const FCrowd_HerdGroup& H) {
        if (H.HerdID == HerdID)
        {
            ActiveAgentCount -= H.Members.Num();
            return true;
        }
        return false;
    });
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd unregistered: %s (removed %d)"), *HerdID.ToString(), Removed);
}

// ============================================================
// Territory Zone Management
// ============================================================

void UCrowdSimulationManager::RegisterTerritoryZone(const FCrowd_TerritoryZone& Zone)
{
    TerritoryZones.Add(Zone);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Territory zone registered: %s at (%.0f, %.0f, %.0f) radius %.0f"),
        *Zone.ZoneID.ToString(),
        Zone.Center.X, Zone.Center.Y, Zone.Center.Z,
        Zone.Radius);
}

ECrowd_TerritoryType UCrowdSimulationManager::GetTerritoryTypeAtLocation(const FVector& Location) const
{
    for (const FCrowd_TerritoryZone& Zone : TerritoryZones)
    {
        float DistSq = FVector::DistSquared(Location, Zone.Center);
        if (DistSq <= Zone.Radius * Zone.Radius)
        {
            return Zone.TerritoryType;
        }
    }
    return ECrowd_TerritoryType::Neutral;
}

// ============================================================
// LOD Management
// ============================================================

void UCrowdSimulationManager::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    for (FCrowd_HerdGroup& Herd : HerdGroups)
    {
        float DistToPlayer = FVector::Dist(PlayerLocation, Herd.CenterLocation);

        ECrowd_LODLevel NewLOD;
        if (DistToPlayer < 1500.0f)
            NewLOD = ECrowd_LODLevel::Full;
        else if (DistToPlayer < 4000.0f)
            NewLOD = ECrowd_LODLevel::Medium;
        else if (DistToPlayer < 8000.0f)
            NewLOD = ECrowd_LODLevel::Low;
        else
            NewLOD = ECrowd_LODLevel::Culled;

        if (NewLOD != Herd.CurrentLOD)
        {
            ApplyLODToHerd(Herd, NewLOD);
            Herd.CurrentLOD = NewLOD;
        }
    }
}

void UCrowdSimulationManager::ApplyLODToHerd(FCrowd_HerdGroup& Herd, ECrowd_LODLevel LODLevel)
{
    for (TWeakObjectPtr<AActor> MemberPtr : Herd.Members)
    {
        if (!MemberPtr.IsValid()) continue;
        AActor* Member = MemberPtr.Get();

        USkeletalMeshComponent* SkelComp = Member->FindComponentByClass<USkeletalMeshComponent>();
        if (!SkelComp) continue;

        switch (LODLevel)
        {
        case ECrowd_LODLevel::Full:
            SkelComp->SetForcedLOD(0);
            Member->SetActorHiddenInGame(false);
            break;
        case ECrowd_LODLevel::Medium:
            SkelComp->SetForcedLOD(1);
            Member->SetActorHiddenInGame(false);
            break;
        case ECrowd_LODLevel::Low:
            SkelComp->SetForcedLOD(2);
            Member->SetActorHiddenInGame(false);
            break;
        case ECrowd_LODLevel::Culled:
            Member->SetActorHiddenInGame(true);
            break;
        }
    }
}

// ============================================================
// Herd Behavior Update
// ============================================================

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    if (!bSimulationActive) return;

    for (FCrowd_HerdGroup& Herd : HerdGroups)
    {
        switch (Herd.BehaviorState)
        {
        case ECrowd_HerdBehavior::Grazing:
            UpdateGrazingBehavior(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Migrating:
            UpdateMigrationBehavior(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Fleeing:
            UpdateFleeingBehavior(Herd, DeltaTime);
            break;
        case ECrowd_HerdBehavior::Hunting:
            UpdateHuntingBehavior(Herd, DeltaTime);
            break;
        default:
            break;
        }

        // Recalculate herd center
        UpdateHerdCenter(Herd);
    }
}

void UCrowdSimulationManager::UpdateGrazingBehavior(FCrowd_HerdGroup& Herd, float DeltaTime)
{
    // Grazing: slow random movement within territory radius
    Herd.StateTimer += DeltaTime;
    if (Herd.StateTimer > 10.0f)
    {
        // Occasionally shift grazing direction
        float AngleRad = FMath::RandRange(0.0f, 2.0f * PI);
        Herd.MoveDirection = FVector(FMath::Cos(AngleRad), FMath::Sin(AngleRad), 0.0f);
        Herd.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::UpdateMigrationBehavior(FCrowd_HerdGroup& Herd, float DeltaTime)
{
    // Migration: move toward destination in column formation
    if (!Herd.MigrationDestination.IsZero())
    {
        FVector ToDestination = (Herd.MigrationDestination - Herd.CenterLocation).GetSafeNormal();
        Herd.MoveDirection = ToDestination;

        float DistToDest = FVector::Dist(Herd.CenterLocation, Herd.MigrationDestination);
        if (DistToDest < 500.0f)
        {
            // Arrived — switch to grazing
            Herd.BehaviorState = ECrowd_HerdBehavior::Grazing;
            Herd.MigrationDestination = FVector::ZeroVector;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s arrived at destination"), *Herd.HerdID.ToString());
        }
    }
}

void UCrowdSimulationManager::UpdateFleeingBehavior(FCrowd_HerdGroup& Herd, float DeltaTime)
{
    // Fleeing: move away from threat at high speed
    Herd.StateTimer += DeltaTime;
    if (Herd.StateTimer > 15.0f)
    {
        // Calmed down — return to grazing
        Herd.BehaviorState = ECrowd_HerdBehavior::Grazing;
        Herd.StateTimer = 0.0f;
    }
}

void UCrowdSimulationManager::UpdateHuntingBehavior(FCrowd_HerdGroup& Herd, float DeltaTime)
{
    // Hunting (predator packs): coordinate flanking movement
    Herd.StateTimer += DeltaTime;
    // Flanking logic: split into sub-groups
    // Simplified: rotate move direction over time to simulate encirclement
    float RotationRate = 45.0f * DeltaTime; // degrees per second
    FRotator Rot(0.0f, RotationRate, 0.0f);
    Herd.MoveDirection = Rot.RotateVector(Herd.MoveDirection);
}

void UCrowdSimulationManager::UpdateHerdCenter(FCrowd_HerdGroup& Herd)
{
    if (Herd.Members.Num() == 0) return;

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;

    for (TWeakObjectPtr<AActor> MemberPtr : Herd.Members)
    {
        if (MemberPtr.IsValid())
        {
            Sum += MemberPtr->GetActorLocation();
            ValidCount++;
        }
    }

    if (ValidCount > 0)
    {
        Herd.CenterLocation = Sum / ValidCount;
    }
}

// ============================================================
// Threat Response
// ============================================================

void UCrowdSimulationManager::TriggerHerdFleeResponse(const FName& HerdID, const FVector& ThreatLocation)
{
    for (FCrowd_HerdGroup& Herd : HerdGroups)
    {
        if (Herd.HerdID == HerdID)
        {
            Herd.BehaviorState = ECrowd_HerdBehavior::Fleeing;
            FVector FleeDir = (Herd.CenterLocation - ThreatLocation).GetSafeNormal();
            Herd.MoveDirection = FleeDir;
            Herd.StateTimer = 0.0f;
            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s FLEEING from threat at (%.0f, %.0f)"),
                *HerdID.ToString(), ThreatLocation.X, ThreatLocation.Y);
            return;
        }
    }
}

void UCrowdSimulationManager::TriggerAreaFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (FCrowd_HerdGroup& Herd : HerdGroups)
    {
        float DistToThreat = FVector::Dist(Herd.CenterLocation, ThreatLocation);
        if (DistToThreat <= ThreatRadius)
        {
            TriggerHerdFleeResponse(Herd.HerdID, ThreatLocation);
        }
    }
}

// ============================================================
// Debug
// ============================================================

void UCrowdSimulationManager::DrawDebugCrowdState(UWorld* World) const
{
    if (!World) return;

    for (const FCrowd_HerdGroup& Herd : HerdGroups)
    {
        FColor DebugColor = FColor::Green;
        switch (Herd.BehaviorState)
        {
        case ECrowd_HerdBehavior::Fleeing:  DebugColor = FColor::Red;    break;
        case ECrowd_HerdBehavior::Hunting:  DebugColor = FColor::Orange; break;
        case ECrowd_HerdBehavior::Migrating:DebugColor = FColor::Yellow; break;
        default: break;
        }

        DrawDebugSphere(World, Herd.CenterLocation, 200.0f, 12, DebugColor, false, 1.0f);
        DrawDebugString(World, Herd.CenterLocation + FVector(0, 0, 300),
            FString::Printf(TEXT("%s [%d]"), *Herd.HerdID.ToString(), Herd.Members.Num()),
            nullptr, DebugColor, 1.0f);
    }

    for (const FCrowd_TerritoryZone& Zone : TerritoryZones)
    {
        DrawDebugCylinder(World, Zone.Center - FVector(0, 0, 100), Zone.Center + FVector(0, 0, 100),
            Zone.Radius, 24, FColor::Cyan, false, 1.0f);
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

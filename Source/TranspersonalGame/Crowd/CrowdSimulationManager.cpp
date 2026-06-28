// CrowdSimulationManager.cpp — Agent #13 Crowd & Traffic Simulation
// Prehistoric crowd simulation: herds, packs, stampedes, migration

#include "CrowdSimulationManager.h"
#include "CrowdBehaviorTypes.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for performance

    MaxAgents = 500;
    StampedeRadius = 2000.0f;
    StampedeSpeedMultiplier = 2.5f;
    LODDistanceFull = 2000.0f;
    LODDistanceMedium = 5000.0f;
    LODDistanceLow = 10000.0f;
    bDebugDraw = false;
    NextGroupID = 0;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    // Start crowd update timer at 5Hz (every 0.2s) for group-level decisions
    GetWorld()->GetTimerManager().SetTimer(
        CrowdUpdateTimer,
        this,
        &UCrowdSimulationManager::UpdateCrowdGroups,
        0.2f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Manager initialized. MaxAgents=%d"), MaxAgents);
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update LOD levels based on player distance
    UpdateLODLevels();
}

int32 UCrowdSimulationManager::RegisterCrowdGroup(const FCrowd_GroupData& GroupData)
{
    FCrowd_GroupData NewGroup = GroupData;
    NewGroup.GroupID = NextGroupID++;
    ActiveGroups.Add(NewGroup.GroupID, NewGroup);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group registered: ID=%d Type=%d AgentCount=%d"),
        NewGroup.GroupID, (int32)NewGroup.GroupType, NewGroup.AgentCount);

    return NewGroup.GroupID;
}

void UCrowdSimulationManager::UnregisterCrowdGroup(int32 GroupID)
{
    if (ActiveGroups.Contains(GroupID))
    {
        ActiveGroups.Remove(GroupID);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d unregistered"), GroupID);
    }
}

void UCrowdSimulationManager::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] STAMPEDE TRIGGERED at %s radius=%.0f"),
        *ThreatLocation.ToString(), ThreatRadius);

    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;
        float DistToThreat = FVector::Dist(Group.GroupCenter, ThreatLocation);

        if (DistToThreat <= ThreatRadius)
        {
            Group.CurrentState = ECrowd_BehaviorState::Stampeding;
            Group.GroupThreatLevel = 1.0f;

            // Calculate flee direction away from threat
            FVector FleeDir = (Group.GroupCenter - ThreatLocation).GetSafeNormal();
            Group.MigrationTarget = Group.GroupCenter + FleeDir * 5000.0f;

            UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d entering stampede, fleeing to %s"),
                Group.GroupID, *Group.MigrationTarget.ToString());
        }
    }

    OnStampedeTriggered.Broadcast(ThreatLocation, ThreatRadius);
}

void UCrowdSimulationManager::SetGroupBehaviorState(int32 GroupID, ECrowd_BehaviorState NewState)
{
    if (FCrowd_GroupData* Group = ActiveGroups.Find(GroupID))
    {
        Group->CurrentState = NewState;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d state -> %d"), GroupID, (int32)NewState);
    }
}

FCrowd_GroupData UCrowdSimulationManager::GetGroupData(int32 GroupID) const
{
    if (const FCrowd_GroupData* Group = ActiveGroups.Find(GroupID))
    {
        return *Group;
    }
    return FCrowd_GroupData();
}

TArray<int32> UCrowdSimulationManager::GetGroupsInRadius(FVector Center, float Radius) const
{
    TArray<int32> Result;
    for (const auto& Pair : ActiveGroups)
    {
        if (FVector::Dist(Pair.Value.GroupCenter, Center) <= Radius)
        {
            Result.Add(Pair.Key);
        }
    }
    return Result;
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Total = 0;
    for (const auto& Pair : ActiveGroups)
    {
        Total += Pair.Value.AgentCount;
    }
    return Total;
}

void UCrowdSimulationManager::UpdateCrowdGroups()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;

        // Decay threat level over time
        if (Group.GroupThreatLevel > 0.0f)
        {
            Group.GroupThreatLevel = FMath::Max(0.0f, Group.GroupThreatLevel - 0.05f);
        }

        // Transition from stampede back to fleeing when threat decays
        if (Group.CurrentState == ECrowd_BehaviorState::Stampeding && Group.GroupThreatLevel < 0.3f)
        {
            Group.CurrentState = ECrowd_BehaviorState::Fleeing;
        }
        else if (Group.CurrentState == ECrowd_BehaviorState::Fleeing && Group.GroupThreatLevel <= 0.0f)
        {
            Group.CurrentState = ECrowd_BehaviorState::Wandering;
        }

        // Debug draw group centers
        if (bDebugDraw)
        {
            FColor DebugColor = FColor::Green;
            if (Group.CurrentState == ECrowd_BehaviorState::Stampeding) DebugColor = FColor::Red;
            else if (Group.CurrentState == ECrowd_BehaviorState::Fleeing) DebugColor = FColor::Orange;
            else if (Group.CurrentState == ECrowd_BehaviorState::Migrating) DebugColor = FColor::Yellow;

            DrawDebugSphere(GetWorld(), Group.GroupCenter, 100.0f, 8, DebugColor, false, 0.25f);
        }
    }
}

void UCrowdSimulationManager::UpdateLODLevels()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;
        float Dist = FVector::Dist(Group.GroupCenter, PlayerLoc);

        // LOD assignment based on distance (stored implicitly via group state)
        // Full LOD: < LODDistanceFull
        // Medium LOD: < LODDistanceMedium
        // Low LOD: < LODDistanceLow
        // Dormant: >= LODDistanceLow
        // (Actual LOD switching handled by individual agent actors)
    }
}

void UCrowdSimulationManager::SpawnHerdAtLocation(FVector Location, ECrowd_AgentType HerdType, int32 Count)
{
    FCrowd_GroupData NewGroup;
    NewGroup.GroupType = HerdType;
    NewGroup.GroupCenter = Location;
    NewGroup.AgentCount = FMath::Clamp(Count, 1, MaxAgents);
    NewGroup.CurrentState = ECrowd_BehaviorState::Grazing;
    NewGroup.GroupThreatLevel = 0.0f;

    int32 ID = RegisterCrowdGroup(NewGroup);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd spawned: ID=%d Type=%d Count=%d at %s"),
        ID, (int32)HerdType, Count, *Location.ToString());
}

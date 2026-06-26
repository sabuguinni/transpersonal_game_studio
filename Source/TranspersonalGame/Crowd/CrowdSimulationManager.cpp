// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival crowd simulation: herds, predator packs, fleeing groups

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Actor.h"

// ─── UCrowdSimulationManager ─────────────────────────────────────────────────

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 500;
    UpdateIntervalSeconds = 0.25f;
    FlockingRadius = 800.0f;
    SeparationRadius = 150.0f;
    bDebugDraw = false;
    CurrentAgentCount = 0;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Subsystem initialized. MaxAgents=%d"), MaxAgents);

    // Start periodic crowd update tick
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &UCrowdSimulationManager::TickCrowdUpdate,
            UpdateIntervalSeconds,
            true
        );
    }
}

void UCrowdSimulationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(UpdateTimerHandle);
    }
    ActiveGroups.Empty();
    Super::Deinitialize();
}

// ─── Group Registration ───────────────────────────────────────────────────────

int32 UCrowdSimulationManager::RegisterCrowdGroup(ECrowd_GroupType GroupType, FVector SpawnCenter, int32 AgentCount)
{
    if (CurrentAgentCount + AgentCount > MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Agent cap reached (%d/%d). Clamping group."), CurrentAgentCount, MaxAgents);
        AgentCount = FMath::Max(0, MaxAgents - CurrentAgentCount);
    }
    if (AgentCount <= 0) return -1;

    FCrowd_GroupData NewGroup;
    NewGroup.GroupID = NextGroupID++;
    NewGroup.GroupType = GroupType;
    NewGroup.CenterLocation = SpawnCenter;
    NewGroup.AgentCount = AgentCount;
    NewGroup.bIsActive = true;
    NewGroup.CurrentState = ECrowd_GroupState::Idle;
    NewGroup.AlertLevel = 0.0f;
    NewGroup.TargetLocation = SpawnCenter;

    // Assign default behavior based on group type
    switch (GroupType)
    {
        case ECrowd_GroupType::HerbivoreHerd:
            NewGroup.WanderRadius = 2000.0f;
            NewGroup.MoveSpeed = 250.0f;
            NewGroup.FleeThreshold = 0.4f;
            break;
        case ECrowd_GroupType::PredatorPack:
            NewGroup.WanderRadius = 3500.0f;
            NewGroup.MoveSpeed = 450.0f;
            NewGroup.FleeThreshold = 0.8f;
            break;
        case ECrowd_GroupType::ScavengerFlock:
            NewGroup.WanderRadius = 1500.0f;
            NewGroup.MoveSpeed = 180.0f;
            NewGroup.FleeThreshold = 0.2f;
            break;
        case ECrowd_GroupType::MigrationHerd:
            NewGroup.WanderRadius = 8000.0f;
            NewGroup.MoveSpeed = 300.0f;
            NewGroup.FleeThreshold = 0.5f;
            break;
        default:
            NewGroup.WanderRadius = 1000.0f;
            NewGroup.MoveSpeed = 200.0f;
            NewGroup.FleeThreshold = 0.3f;
            break;
    }

    ActiveGroups.Add(NewGroup.GroupID, NewGroup);
    CurrentAgentCount += AgentCount;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d registered. Type=%d, Agents=%d, Total=%d/%d"),
        NewGroup.GroupID, (int32)GroupType, AgentCount, CurrentAgentCount, MaxAgents);

    return NewGroup.GroupID;
}

bool UCrowdSimulationManager::UnregisterCrowdGroup(int32 GroupID)
{
    if (FCrowd_GroupData* Group = ActiveGroups.Find(GroupID))
    {
        CurrentAgentCount -= Group->AgentCount;
        ActiveGroups.Remove(GroupID);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d unregistered. Total=%d"), GroupID, CurrentAgentCount);
        return true;
    }
    return false;
}

// ─── Alert & Flee System ──────────────────────────────────────────────────────

void UCrowdSimulationManager::AlertGroupsInRadius(FVector AlertOrigin, float Radius, float AlertIntensity)
{
    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;
        float Dist = FVector::Dist(Group.CenterLocation, AlertOrigin);
        if (Dist <= Radius)
        {
            // Closer = more alert
            float DistanceFactor = 1.0f - (Dist / Radius);
            Group.AlertLevel = FMath::Clamp(Group.AlertLevel + AlertIntensity * DistanceFactor, 0.0f, 1.0f);

            if (Group.AlertLevel >= Group.FleeThreshold)
            {
                TriggerGroupFlee(Group.GroupID, AlertOrigin);
            }
            else if (Group.AlertLevel > 0.2f && Group.CurrentState == ECrowd_GroupState::Idle)
            {
                Group.CurrentState = ECrowd_GroupState::Alert;
            }

            UE_LOG(LogTemp, Verbose, TEXT("[CrowdSim] Group %d alert=%.2f (dist=%.0f)"),
                Group.GroupID, Group.AlertLevel, Dist);
        }
    }
}

void UCrowdSimulationManager::TriggerGroupFlee(int32 GroupID, FVector ThreatLocation)
{
    FCrowd_GroupData* Group = ActiveGroups.Find(GroupID);
    if (!Group || !Group->bIsActive) return;

    // Flee direction = away from threat
    FVector FleeDir = (Group->CenterLocation - ThreatLocation).GetSafeNormal();
    Group->TargetLocation = Group->CenterLocation + FleeDir * Group->WanderRadius;
    Group->CurrentState = ECrowd_GroupState::Fleeing;
    Group->AlertLevel = 1.0f;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Group %d FLEEING from threat at (%.0f,%.0f,%.0f)"),
        GroupID, ThreatLocation.X, ThreatLocation.Y, ThreatLocation.Z);
}

void UCrowdSimulationManager::SetGroupTarget(int32 GroupID, FVector TargetLocation)
{
    if (FCrowd_GroupData* Group = ActiveGroups.Find(GroupID))
    {
        Group->TargetLocation = TargetLocation;
        Group->CurrentState = ECrowd_GroupState::Moving;
    }
}

// ─── Periodic Update ──────────────────────────────────────────────────────────

void UCrowdSimulationManager::TickCrowdUpdate()
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;
        if (!Group.bIsActive) continue;

        UpdateGroupBehavior(Group, UpdateIntervalSeconds);

        if (bDebugDraw)
        {
            DrawDebugSphere(World, Group.CenterLocation, 100.0f, 8,
                Group.CurrentState == ECrowd_GroupState::Fleeing ? FColor::Red :
                Group.CurrentState == ECrowd_GroupState::Alert   ? FColor::Yellow : FColor::Green,
                false, UpdateIntervalSeconds * 1.5f);
        }
    }

    // Decay alert levels over time
    DecayAlertLevels();
}

void UCrowdSimulationManager::UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime)
{
    switch (Group.CurrentState)
    {
        case ECrowd_GroupState::Idle:
        {
            // Random wander trigger (10% chance per tick)
            if (FMath::RandRange(0, 9) == 0)
            {
                FVector WanderOffset = FVector(
                    FMath::RandRange(-Group.WanderRadius * 0.3f, Group.WanderRadius * 0.3f),
                    FMath::RandRange(-Group.WanderRadius * 0.3f, Group.WanderRadius * 0.3f),
                    0.0f
                );
                Group.TargetLocation = Group.CenterLocation + WanderOffset;
                Group.CurrentState = ECrowd_GroupState::Moving;
            }
            break;
        }
        case ECrowd_GroupState::Moving:
        {
            FVector ToTarget = Group.TargetLocation - Group.CenterLocation;
            float DistToTarget = ToTarget.Size();
            if (DistToTarget < 200.0f)
            {
                Group.CurrentState = ECrowd_GroupState::Idle;
            }
            else
            {
                FVector MoveDir = ToTarget.GetSafeNormal();
                Group.CenterLocation += MoveDir * Group.MoveSpeed * DeltaTime;
            }
            break;
        }
        case ECrowd_GroupState::Fleeing:
        {
            FVector ToTarget = Group.TargetLocation - Group.CenterLocation;
            float DistToTarget = ToTarget.Size();
            if (DistToTarget < 300.0f)
            {
                Group.CurrentState = ECrowd_GroupState::Alert;
            }
            else
            {
                FVector MoveDir = ToTarget.GetSafeNormal();
                // Flee at 1.5x normal speed
                Group.CenterLocation += MoveDir * Group.MoveSpeed * 1.5f * DeltaTime;
            }
            break;
        }
        case ECrowd_GroupState::Alert:
        {
            // Slowly decay back to idle if alert level drops
            if (Group.AlertLevel < 0.1f)
            {
                Group.CurrentState = ECrowd_GroupState::Idle;
            }
            break;
        }
        case ECrowd_GroupState::Grazing:
        {
            // Herbivores graze in place — very slow drift
            FVector GrazeOffset = FVector(
                FMath::RandRange(-50.0f, 50.0f),
                FMath::RandRange(-50.0f, 50.0f),
                0.0f
            );
            Group.CenterLocation += GrazeOffset * DeltaTime;
            break;
        }
        default:
            break;
    }
}

void UCrowdSimulationManager::DecayAlertLevels()
{
    for (auto& Pair : ActiveGroups)
    {
        FCrowd_GroupData& Group = Pair.Value;
        // Alert decays at 5% per update tick
        Group.AlertLevel = FMath::Max(0.0f, Group.AlertLevel - 0.05f);
    }
}

// ─── Query API ────────────────────────────────────────────────────────────────

int32 UCrowdSimulationManager::GetActiveGroupCount() const
{
    return ActiveGroups.Num();
}

int32 UCrowdSimulationManager::GetTotalAgentCount() const
{
    return CurrentAgentCount;
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
        if (FVector::Dist(Pair.Value.CenterLocation, Center) <= Radius)
        {
            Result.Add(Pair.Key);
        }
    }
    return Result;
}

ECrowd_GroupState UCrowdSimulationManager::GetGroupState(int32 GroupID) const
{
    if (const FCrowd_GroupData* Group = ActiveGroups.Find(GroupID))
    {
        return Group->CurrentState;
    }
    return ECrowd_GroupState::Idle;
}

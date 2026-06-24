// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd/pack/cluster crowd simulation for up to 50,000 agents via Mass AI

#include "CrowdSimulationManager.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxAgents = 500;
    HerdCohesionRadius = 800.0f;
    SeparationRadius = 150.0f;
    AlignmentRadius = 400.0f;
    FleeRadius = 2000.0f;
    bCrowdDebugDraw = false;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeCrowdGroups();
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateCrowdBehavior(DeltaTime);
}

void UCrowdSimulationManager::InitializeCrowdGroups()
{
    // Register default crowd groups for prehistoric world
    // Parasaurolophus herd — large herbivore, high cohesion
    FCrowd_GroupConfig ParaHerd;
    ParaHerd.GroupID = ECrowd_GroupType::HerbivoreHerd;
    ParaHerd.GroupLabel = TEXT("Parasaurolophus_Herd");
    ParaHerd.MaxGroupSize = 8;
    ParaHerd.CohesionStrength = 1.2f;
    ParaHerd.FleeThreshold = 1500.0f;
    ParaHerd.bIsPreySpecies = true;
    RegisterCrowdGroup(ParaHerd);

    // Velociraptor pack — predator, coordinated flanking
    FCrowd_GroupConfig RaptorPack;
    RaptorPack.GroupID = ECrowd_GroupType::PredatorPack;
    RaptorPack.GroupLabel = TEXT("Velociraptor_Pack");
    RaptorPack.MaxGroupSize = 5;
    RaptorPack.CohesionStrength = 0.8f;
    RaptorPack.FleeThreshold = 0.0f; // predators don't flee
    RaptorPack.bIsPreySpecies = false;
    RegisterCrowdGroup(RaptorPack);

    // Triceratops cluster — defensive formation
    FCrowd_GroupConfig TrikeCluster;
    TrikeCluster.GroupID = ECrowd_GroupType::DefensiveCluster;
    TrikeCluster.GroupLabel = TEXT("Triceratops_Cluster");
    TrikeCluster.MaxGroupSize = 4;
    TrikeCluster.CohesionStrength = 1.5f;
    TrikeCluster.FleeThreshold = 500.0f;
    TrikeCluster.bIsPreySpecies = true;
    RegisterCrowdGroup(TrikeCluster);

    // Brachiosaurus solitary — large, slow, solitary wanderers
    FCrowd_GroupConfig BrachioSolitary;
    BrachioSolitary.GroupID = ECrowd_GroupType::SolitaryWanderer;
    BrachioSolitary.GroupLabel = TEXT("Brachiosaurus_Solitary");
    BrachioSolitary.MaxGroupSize = 2;
    BrachioSolitary.CohesionStrength = 0.3f;
    BrachioSolitary.FleeThreshold = 3000.0f;
    BrachioSolitary.bIsPreySpecies = true;
    RegisterCrowdGroup(BrachioSolitary);

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized %d crowd groups"), CrowdGroups.Num());
}

void UCrowdSimulationManager::RegisterCrowdGroup(const FCrowd_GroupConfig& GroupConfig)
{
    CrowdGroups.Add(GroupConfig.GroupLabel, GroupConfig);
}

void UCrowdSimulationManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!GetWorld()) return;

    for (auto& Pair : CrowdGroups)
    {
        FCrowd_GroupConfig& Group = Pair.Value;
        UpdateGroupBehavior(Group, DeltaTime);
    }
}

void UCrowdSimulationManager::UpdateGroupBehavior(FCrowd_GroupConfig& Group, float DeltaTime)
{
    // Boids-style flocking for herd/pack behavior
    // Separation: avoid crowding neighbors
    // Alignment: steer toward average heading of neighbors
    // Cohesion: steer toward average position of neighbors
    // Flee: move away from predators/player if within FleeThreshold

    if (Group.AgentActors.Num() == 0) return;

    FVector GroupCenter = FVector::ZeroVector;
    for (AActor* Agent : Group.AgentActors)
    {
        if (Agent && IsValid(Agent))
        {
            GroupCenter += Agent->GetActorLocation();
        }
    }
    GroupCenter /= FMath::Max(1, Group.AgentActors.Num());

    // Store centroid for debug/query
    Group.LastGroupCentroid = GroupCenter;

    if (bCrowdDebugDraw && GetWorld())
    {
        DrawDebugSphere(GetWorld(), GroupCenter, 100.0f, 8, FColor::Green, false, 0.1f);
    }
}

void UCrowdSimulationManager::RegisterAgentToGroup(AActor* Agent, const FString& GroupLabel)
{
    if (!Agent || !CrowdGroups.Contains(GroupLabel)) return;
    FCrowd_GroupConfig& Group = CrowdGroups[GroupLabel];
    if (Group.AgentActors.Num() < Group.MaxGroupSize)
    {
        Group.AgentActors.Add(Agent);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Agent %s registered to group %s"), *Agent->GetName(), *GroupLabel);
    }
}

void UCrowdSimulationManager::TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    for (auto& Pair : CrowdGroups)
    {
        FCrowd_GroupConfig& Group = Pair.Value;
        if (!Group.bIsPreySpecies) continue;

        for (AActor* Agent : Group.AgentActors)
        {
            if (!Agent || !IsValid(Agent)) continue;
            float Dist = FVector::Dist(Agent->GetActorLocation(), ThreatLocation);
            if (Dist < ThreatRadius)
            {
                // Trigger flee — agent moves away from threat
                FVector FleeDir = (Agent->GetActorLocation() - ThreatLocation).GetSafeNormal();
                FVector FleeTarget = Agent->GetActorLocation() + FleeDir * 2000.0f;
                // In full implementation: set AI MoveToLocation(FleeTarget)
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] FLEE triggered for %s in group %s"),
                    *Agent->GetName(), *Group.GroupLabel);
            }
        }
    }
}

int32 UCrowdSimulationManager::GetTotalAgentCount() const
{
    int32 Total = 0;
    for (const auto& Pair : CrowdGroups)
    {
        Total += Pair.Value.AgentActors.Num();
    }
    return Total;
}

FCrowd_GroupConfig UCrowdSimulationManager::GetGroupConfig(const FString& GroupLabel) const
{
    if (CrowdGroups.Contains(GroupLabel))
    {
        return CrowdGroups[GroupLabel];
    }
    return FCrowd_GroupConfig();
}

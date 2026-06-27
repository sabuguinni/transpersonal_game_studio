// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: herds, packs, tribes with LOD and panic behavior

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxSimultaneousAgents = 500;
    LODCloseRadius = 500.0f;
    LODMediumRadius = 1500.0f;
    PanicRadius = 300.0f;
    bSimulationActive = false;
    TickInterval = 0.1f;
    TimeSinceLastTick = 0.0f;
}

void UCrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    bSimulationActive = true;
    InitializeHerds();
    unreal::log("CrowdSimulationManager: Initialized with " + FString::FromInt(ActiveAgents.Num()) + " agents");
}

void UCrowdSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bSimulationActive) return;

    TimeSinceLastTick += DeltaTime;
    if (TimeSinceLastTick < TickInterval) return;
    TimeSinceLastTick = 0.0f;

    UpdateAgentLOD();
    UpdateHerdBehavior(TickInterval);
    CheckPanicTriggers();
}

void UCrowdSimulationManager::InitializeHerds()
{
    // Register herd groups from level data
    // Herbivore herd
    FCrowd_HerdData HerbivoreHerd;
    HerbivoreHerd.HerdID = 1;
    HerbivoreHerd.HerdType = ECrowd_AgentType::Herbivore;
    HerbivoreHerd.MaxSize = 20;
    HerbivoreHerd.CurrentSize = 7;
    HerbivoreHerd.MigrationSpeed = 150.0f;
    HerbivoreHerd.bIsMigrating = true;
    HerbivoreHerd.MigrationDirection = FVector(1.0f, 0.0f, 0.0f);
    RegisteredHerds.Add(HerbivoreHerd);

    // Raptor pack
    FCrowd_HerdData RaptorPack;
    RaptorPack.HerdID = 2;
    RaptorPack.HerdType = ECrowd_AgentType::Predator;
    RaptorPack.MaxSize = 8;
    RaptorPack.CurrentSize = 5;
    RaptorPack.MigrationSpeed = 300.0f;
    RaptorPack.bIsMigrating = false;
    RegisteredHerds.Add(RaptorPack);

    // Human tribe
    FCrowd_HerdData HumanTribe;
    HumanTribe.HerdID = 3;
    HumanTribe.HerdType = ECrowd_AgentType::Human;
    HumanTribe.MaxSize = 15;
    HumanTribe.CurrentSize = 8;
    HumanTribe.MigrationSpeed = 100.0f;
    HumanTribe.bIsMigrating = false;
    RegisteredHerds.Add(HumanTribe);
}

void UCrowdSimulationManager::UpdateAgentLOD()
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(PlayerLocation, Agent.WorldLocation);

        ECrowd_LODLevel NewLOD;
        if (Distance <= LODCloseRadius)
        {
            NewLOD = ECrowd_LODLevel::Full;
        }
        else if (Distance <= LODMediumRadius)
        {
            NewLOD = ECrowd_LODLevel::Simplified;
        }
        else
        {
            NewLOD = ECrowd_LODLevel::Billboard;
        }

        if (Agent.CurrentLOD != NewLOD)
        {
            Agent.CurrentLOD = NewLOD;
            OnAgentLODChanged(Agent, NewLOD);
        }
    }
}

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    for (FCrowd_HerdData& Herd : RegisteredHerds)
    {
        if (!Herd.bIsMigrating) continue;

        // Move herd center along migration direction
        Herd.CenterLocation += Herd.MigrationDirection * Herd.MigrationSpeed * DeltaTime;

        // Wrap around map bounds (simple toroidal topology)
        if (Herd.CenterLocation.X > 3000.0f) Herd.CenterLocation.X = -3000.0f;
        if (Herd.CenterLocation.X < -3000.0f) Herd.CenterLocation.X = 3000.0f;
    }
}

void UCrowdSimulationManager::CheckPanicTriggers()
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        float Distance = FVector::Dist(PlayerLocation, Agent.WorldLocation);
        bool bWasPanicking = Agent.bIsPanicking;

        if (Distance < PanicRadius)
        {
            Agent.bIsPanicking = true;
            Agent.PanicSource = PlayerLocation;
            if (!bWasPanicking)
            {
                TriggerHerdPanic(Agent.HerdID, PlayerLocation);
            }
        }
        else if (Distance > PanicRadius * 2.0f)
        {
            Agent.bIsPanicking = false;
        }
    }
}

void UCrowdSimulationManager::TriggerHerdPanic(int32 HerdID, FVector PanicSource)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.HerdID == HerdID)
        {
            Agent.bIsPanicking = true;
            Agent.PanicSource = PanicSource;
            // Calculate flee direction (away from panic source)
            FVector FleeDir = (Agent.WorldLocation - PanicSource).GetSafeNormal();
            Agent.FleeDirection = FleeDir;
        }
    }
}

void UCrowdSimulationManager::OnAgentLODChanged(const FCrowd_AgentData& Agent, ECrowd_LODLevel NewLOD)
{
    // LOD transition logic — in full implementation this would swap meshes
    // For now, log the transition
    FString LODName;
    switch (NewLOD)
    {
        case ECrowd_LODLevel::Full: LODName = TEXT("Full"); break;
        case ECrowd_LODLevel::Simplified: LODName = TEXT("Simplified"); break;
        case ECrowd_LODLevel::Billboard: LODName = TEXT("Billboard"); break;
        default: LODName = TEXT("Unknown"); break;
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgents.Num();
}

int32 UCrowdSimulationManager::GetHerdCount() const
{
    return RegisteredHerds.Num();
}

void UCrowdSimulationManager::SetSimulationActive(bool bActive)
{
    bSimulationActive = bActive;
}

TArray<FCrowd_AgentData> UCrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_AgentData> Result;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (FVector::Dist(Center, Agent.WorldLocation) <= Radius)
        {
            Result.Add(Agent);
        }
    }
    return Result;
}

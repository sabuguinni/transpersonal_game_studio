// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric tribe + dinosaur herd crowd simulation using UE5 Mass AI patterns

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxAgents = 200;
    HerdCohesionRadius = 800.0f;
    FlockSeparationRadius = 150.0f;
    AgentMoveSpeed = 300.0f;
    bSimulationActive = false;
    SimulationTickInterval = 0.5f;
    AccumulatedTime = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    RegisteredAgents.Reserve(MaxAgents);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initialized — max agents: %d"), MaxAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    RegisteredAgents.Empty();
    ActiveHerds.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::Tick(float DeltaTime)
{
    if (!bSimulationActive) return;

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime < SimulationTickInterval) return;
    AccumulatedTime = 0.0f;

    TickHerdBehavior(SimulationTickInterval);
    TickTribeBehavior(SimulationTickInterval);
    TickFleeResponse(SimulationTickInterval);
}

bool UCrowdSimulationManager::IsTickable() const
{
    return bSimulationActive;
}

TStatId UCrowdSimulationManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UCrowdSimulationManager, STATGROUP_Tickables);
}

void UCrowdSimulationManager::RegisterCrowdAgent(AActor* Agent, ECrowd_AgentRole Role)
{
    if (!Agent) return;
    if (RegisteredAgents.Num() >= MaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Max agents reached (%d)"), MaxAgents);
        return;
    }

    FCrowd_AgentData Data;
    Data.AgentActor = Agent;
    Data.Role = Role;
    Data.HomeLocation = Agent->GetActorLocation();
    Data.CurrentState = ECrowd_AgentState::Idle;
    Data.bIsFleeing = false;
    Data.FleeTimer = 0.0f;

    RegisteredAgents.Add(Data);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Registered agent: %s (Role=%d)"), *Agent->GetName(), (int32)Role);
}

void UCrowdSimulationManager::UnregisterCrowdAgent(AActor* Agent)
{
    if (!Agent) return;
    RegisteredAgents.RemoveAll([Agent](const FCrowd_AgentData& D) {
        return D.AgentActor == Agent;
    });
}

void UCrowdSimulationManager::RegisterHerd(const TArray<AActor*>& HerdMembers, ECrowd_DinoSpecies Species)
{
    FCrowd_HerdData Herd;
    Herd.Species = Species;
    Herd.bIsMigrating = false;
    Herd.MigrationProgress = 0.0f;

    for (AActor* Member : HerdMembers)
    {
        if (Member)
        {
            FCrowd_AgentData AgentData;
            AgentData.AgentActor = Member;
            AgentData.Role = ECrowd_AgentRole::DinoHerdMember;
            AgentData.HomeLocation = Member->GetActorLocation();
            AgentData.CurrentState = ECrowd_AgentState::Grazing;
            AgentData.bIsFleeing = false;
            AgentData.FleeTimer = 0.0f;
            Herd.Members.Add(AgentData);
        }
    }

    if (Herd.Members.Num() > 0)
    {
        Herd.HerdCenter = CalculateHerdCenter(Herd);
        ActiveHerds.Add(Herd);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd registered: Species=%d, Members=%d"), (int32)Species, Herd.Members.Num());
    }
}

void UCrowdSimulationManager::TriggerFleeEvent(FVector ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Flee event triggered at %s, radius=%.0f"), *ThreatLocation.ToString(), ThreatRadius);

    // Trigger flee on registered tribe agents
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor) continue;
        float Dist = FVector::Dist(Agent.AgentActor->GetActorLocation(), ThreatLocation);
        if (Dist <= ThreatRadius)
        {
            Agent.bIsFleeing = true;
            Agent.FleeTimer = 8.0f; // flee for 8 seconds
            Agent.CurrentState = ECrowd_AgentState::Fleeing;

            // Calculate flee direction (away from threat)
            FVector FleeDir = (Agent.AgentActor->GetActorLocation() - ThreatLocation).GetSafeNormal();
            Agent.FleeDestination = Agent.AgentActor->GetActorLocation() + FleeDir * 2000.0f;
        }
    }

    // Trigger flee on herds
    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        float HerdDist = FVector::Dist(Herd.HerdCenter, ThreatLocation);
        if (HerdDist <= ThreatRadius * 1.5f) // herds have wider awareness
        {
            for (FCrowd_AgentData& Member : Herd.Members)
            {
                if (!Member.AgentActor) continue;
                Member.bIsFleeing = true;
                Member.FleeTimer = 12.0f;
                Member.CurrentState = ECrowd_AgentState::Fleeing;
                FVector FleeDir = (Member.AgentActor->GetActorLocation() - ThreatLocation).GetSafeNormal();
                Member.FleeDestination = Member.AgentActor->GetActorLocation() + FleeDir * 3000.0f;
            }
        }
    }
}

void UCrowdSimulationManager::StartHerdMigration(int32 HerdIndex, FVector Destination)
{
    if (!ActiveHerds.IsValidIndex(HerdIndex)) return;

    FCrowd_HerdData& Herd = ActiveHerds[HerdIndex];
    Herd.bIsMigrating = true;
    Herd.MigrationDestination = Destination;
    Herd.MigrationProgress = 0.0f;

    for (FCrowd_AgentData& Member : Herd.Members)
    {
        Member.CurrentState = ECrowd_AgentState::Migrating;
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %d started migration to %s"), HerdIndex, *Destination.ToString());
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    int32 Total = RegisteredAgents.Num();
    for (const FCrowd_HerdData& Herd : ActiveHerds)
    {
        Total += Herd.Members.Num();
    }
    return Total;
}

// --- Private Tick Methods ---

void UCrowdSimulationManager::TickHerdBehavior(float DeltaTime)
{
    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        // Update herd center
        Herd.HerdCenter = CalculateHerdCenter(Herd);

        if (Herd.bIsMigrating)
        {
            // Move herd members toward migration destination
            float StepSize = AgentMoveSpeed * DeltaTime;
            Herd.MigrationProgress = FMath::Min(1.0f, Herd.MigrationProgress + DeltaTime * 0.01f);

            for (FCrowd_AgentData& Member : Herd.Members)
            {
                if (!Member.AgentActor) continue;
                if (Member.bIsFleeing) continue;

                // Flocking: move toward herd center + separation from neighbors
                FVector ToCenter = (Herd.HerdCenter - Member.AgentActor->GetActorLocation()).GetSafeNormal();
                FVector ToDestination = (Herd.MigrationDestination - Member.AgentActor->GetActorLocation()).GetSafeNormal();
                FVector MoveDir = (ToCenter * 0.3f + ToDestination * 0.7f).GetSafeNormal();

                FVector NewLoc = Member.AgentActor->GetActorLocation() + MoveDir * StepSize;
                Member.AgentActor->SetActorLocation(NewLoc, true);
            }

            if (Herd.MigrationProgress >= 1.0f)
            {
                Herd.bIsMigrating = false;
                for (FCrowd_AgentData& Member : Herd.Members)
                {
                    Member.CurrentState = ECrowd_AgentState::Grazing;
                    Member.HomeLocation = Member.AgentActor ? Member.AgentActor->GetActorLocation() : Member.HomeLocation;
                }
                UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd migration complete"));
            }
        }
    }
}

void UCrowdSimulationManager::TickTribeBehavior(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor) continue;
        if (Agent.bIsFleeing) continue;

        switch (Agent.CurrentState)
        {
        case ECrowd_AgentState::Idle:
            // Occasionally wander
            if (FMath::RandRange(0, 100) < 5)
            {
                FVector WanderOffset = FVector(
                    FMath::RandRange(-300.0f, 300.0f),
                    FMath::RandRange(-300.0f, 300.0f),
                    0.0f
                );
                Agent.WanderTarget = Agent.HomeLocation + WanderOffset;
                Agent.CurrentState = ECrowd_AgentState::Wandering;
            }
            break;

        case ECrowd_AgentState::Wandering:
        {
            FVector ToTarget = Agent.WanderTarget - Agent.AgentActor->GetActorLocation();
            float Dist = ToTarget.Size();
            if (Dist < 50.0f)
            {
                Agent.CurrentState = ECrowd_AgentState::Idle;
            }
            else
            {
                FVector MoveDir = ToTarget.GetSafeNormal();
                FVector NewLoc = Agent.AgentActor->GetActorLocation() + MoveDir * AgentMoveSpeed * 0.5f * DeltaTime;
                Agent.AgentActor->SetActorLocation(NewLoc, true);
            }
            break;
        }

        default:
            break;
        }
    }
}

void UCrowdSimulationManager::TickFleeResponse(float DeltaTime)
{
    // Process fleeing agents
    for (FCrowd_AgentData& Agent : RegisteredAgents)
    {
        if (!Agent.AgentActor || !Agent.bIsFleeing) continue;

        Agent.FleeTimer -= DeltaTime;
        if (Agent.FleeTimer <= 0.0f)
        {
            Agent.bIsFleeing = false;
            Agent.CurrentState = ECrowd_AgentState::Idle;
            continue;
        }

        FVector ToFlee = Agent.FleeDestination - Agent.AgentActor->GetActorLocation();
        if (ToFlee.Size() > 100.0f)
        {
            FVector MoveDir = ToFlee.GetSafeNormal();
            FVector NewLoc = Agent.AgentActor->GetActorLocation() + MoveDir * AgentMoveSpeed * 2.0f * DeltaTime;
            Agent.AgentActor->SetActorLocation(NewLoc, true);
        }
    }

    // Process fleeing herd members
    for (FCrowd_HerdData& Herd : ActiveHerds)
    {
        for (FCrowd_AgentData& Member : Herd.Members)
        {
            if (!Member.AgentActor || !Member.bIsFleeing) continue;

            Member.FleeTimer -= DeltaTime;
            if (Member.FleeTimer <= 0.0f)
            {
                Member.bIsFleeing = false;
                Member.CurrentState = Herd.bIsMigrating ? ECrowd_AgentState::Migrating : ECrowd_AgentState::Grazing;
                continue;
            }

            FVector ToFlee = Member.FleeDestination - Member.AgentActor->GetActorLocation();
            if (ToFlee.Size() > 100.0f)
            {
                FVector MoveDir = ToFlee.GetSafeNormal();
                FVector NewLoc = Member.AgentActor->GetActorLocation() + MoveDir * AgentMoveSpeed * 3.0f * DeltaTime;
                Member.AgentActor->SetActorLocation(NewLoc, true);
            }
        }
    }
}

FVector UCrowdSimulationManager::CalculateHerdCenter(const FCrowd_HerdData& Herd) const
{
    if (Herd.Members.Num() == 0) return FVector::ZeroVector;

    FVector Sum = FVector::ZeroVector;
    int32 ValidCount = 0;
    for (const FCrowd_AgentData& Member : Herd.Members)
    {
        if (Member.AgentActor)
        {
            Sum += Member.AgentActor->GetActorLocation();
            ValidCount++;
        }
    }
    return ValidCount > 0 ? Sum / (float)ValidCount : FVector::ZeroVector;
}

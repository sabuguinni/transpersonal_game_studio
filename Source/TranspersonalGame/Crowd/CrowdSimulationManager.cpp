// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric herd simulation: Parasaurolophus, Protoceratops, Pachycephalosaurus, Tsintaosaurus

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "Engine/SkeletalMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxCrowdAgents = 50;
    HerdUpdateInterval = 2.0f;
    FlockingRadius = 800.0f;
    SeparationRadius = 200.0f;
    bCrowdSimulationActive = false;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bCrowdSimulationActive = true;
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized. MaxAgents=%d"), MaxCrowdAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bCrowdSimulationActive = false;
    RegisteredHerds.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterHerd(const FCrowd_HerdData& HerdData)
{
    if (RegisteredHerds.Num() >= MaxCrowdAgents / 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Max herd count reached."));
        return;
    }
    RegisteredHerds.Add(HerdData);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered herd '%s' with %d members."),
        *HerdData.HerdName.ToString(), HerdData.MemberCount);
}

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    if (!bCrowdSimulationActive) return;

    for (FCrowd_HerdData& Herd : RegisteredHerds)
    {
        Herd.TimeSinceLastUpdate += DeltaTime;
        if (Herd.TimeSinceLastUpdate < HerdUpdateInterval) continue;
        Herd.TimeSinceLastUpdate = 0.0f;

        // Simple flocking: move herd center toward waypoint
        FVector ToWaypoint = Herd.CurrentWaypoint - Herd.HerdCenter;
        float DistToWaypoint = ToWaypoint.Size();

        if (DistToWaypoint < 300.0f)
        {
            // Pick new waypoint within grazing range
            float Angle = FMath::RandRange(0.0f, 360.0f);
            float Dist = FMath::RandRange(500.0f, 2000.0f);
            Herd.CurrentWaypoint = Herd.HerdCenter +
                FVector(FMath::Cos(FMath::DegreesToRadians(Angle)) * Dist,
                        FMath::Sin(FMath::DegreesToRadians(Angle)) * Dist,
                        0.0f);
        }

        // Update herd center toward waypoint (slow herbivore grazing speed)
        FVector MoveDir = ToWaypoint.GetSafeNormal();
        Herd.HerdCenter += MoveDir * Herd.MoveSpeed * HerdUpdateInterval;
    }
}

int32 UCrowdSimulationManager::GetActiveCrowdCount() const
{
    int32 Total = 0;
    for (const FCrowd_HerdData& Herd : RegisteredHerds)
    {
        Total += Herd.MemberCount;
    }
    return Total;
}

FCrowd_HerdData UCrowdSimulationManager::GetHerdByName(FName HerdName) const
{
    for (const FCrowd_HerdData& Herd : RegisteredHerds)
    {
        if (Herd.HerdName == HerdName)
        {
            return Herd;
        }
    }
    return FCrowd_HerdData();
}

void UCrowdSimulationManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    for (FCrowd_HerdData& Herd : RegisteredHerds)
    {
        float DistToThreat = FVector::Dist(Herd.HerdCenter, ThreatLocation);
        if (DistToThreat < ThreatRadius)
        {
            // Flee: move waypoint directly away from threat
            FVector FleeDir = (Herd.HerdCenter - ThreatLocation).GetSafeNormal();
            Herd.CurrentWaypoint = Herd.HerdCenter + FleeDir * 3000.0f;
            Herd.MoveSpeed = Herd.MoveSpeed * 3.0f; // Panic sprint
            Herd.bIsFleeing = true;
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Herd '%s' FLEEING from threat!"),
                *Herd.HerdName.ToString());
        }
        else if (Herd.bIsFleeing && DistToThreat > ThreatRadius * 2.0f)
        {
            // Calm down once far enough
            Herd.MoveSpeed = Herd.MoveSpeed / 3.0f;
            Herd.bIsFleeing = false;
        }
    }
}

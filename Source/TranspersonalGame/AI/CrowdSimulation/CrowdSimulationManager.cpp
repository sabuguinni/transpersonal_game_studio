// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Implements prehistoric herd and pack crowd simulation using UE5 Mass AI patterns.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "DrawDebugHelpers.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxHerdAgents = 50;
    MaxPackAgents = 12;
    HerdUpdateInterval = 0.5f;
    PackUpdateInterval = 0.25f;
    bSimulationActive = false;
    AgentSeparationRadius = 150.0f;
    HerdCohesionRadius = 800.0f;
    PackHuntingRadius = 1200.0f;
    FleeResponseRadius = 600.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    bSimulationActive = true;
    RegisteredHerds.Empty();
    RegisteredPacks.Empty();
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Initialized. MaxHerd=%d MaxPack=%d"), MaxHerdAgents, MaxPackAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    bSimulationActive = false;
    RegisteredHerds.Empty();
    RegisteredPacks.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::RegisterHerd(const FCrowd_HerdDescriptor& HerdDesc)
{
    if (RegisteredHerds.Num() >= 10)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Herd cap reached (10). Cannot register %s"), *HerdDesc.HerdID.ToString());
        return;
    }
    RegisteredHerds.Add(HerdDesc);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Herd registered — ID=%s Species=%s AgentCount=%d"),
        *HerdDesc.HerdID.ToString(), *HerdDesc.SpeciesName, HerdDesc.AgentCount);
}

void UCrowdSimulationManager::RegisterPack(const FCrowd_PackDescriptor& PackDesc)
{
    if (RegisteredPacks.Num() >= 5)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Pack cap reached (5). Cannot register %s"), *PackDesc.PackID.ToString());
        return;
    }
    RegisteredPacks.Add(PackDesc);
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Pack registered — ID=%s Species=%s AgentCount=%d HuntMode=%d"),
        *PackDesc.PackID.ToString(), *PackDesc.SpeciesName, PackDesc.AgentCount, (int32)PackDesc.HuntingMode);
}

void UCrowdSimulationManager::UpdateHerdBehavior(float DeltaTime)
{
    if (!bSimulationActive) return;

    for (FCrowd_HerdDescriptor& Herd : RegisteredHerds)
    {
        // Advance migration progress
        Herd.MigrationProgress = FMath::Clamp(Herd.MigrationProgress + DeltaTime * 0.01f, 0.0f, 1.0f);

        // State transitions based on threat level
        if (Herd.ThreatLevel > 0.7f && Herd.CurrentState != ECrowd_HerdState::Fleeing)
        {
            Herd.CurrentState = ECrowd_HerdState::Fleeing;
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Herd %s FLEEING (threat=%.2f)"), *Herd.HerdID.ToString(), Herd.ThreatLevel);
        }
        else if (Herd.ThreatLevel < 0.2f && Herd.CurrentState == ECrowd_HerdState::Fleeing)
        {
            Herd.CurrentState = ECrowd_HerdState::Grazing;
            Herd.ThreatLevel = 0.0f;
        }

        // Decay threat over time
        Herd.ThreatLevel = FMath::Max(0.0f, Herd.ThreatLevel - DeltaTime * 0.05f);
    }
}

void UCrowdSimulationManager::UpdatePackBehavior(float DeltaTime)
{
    if (!bSimulationActive) return;

    for (FCrowd_PackDescriptor& Pack : RegisteredPacks)
    {
        // Pack hunting state machine
        switch (Pack.HuntingMode)
        {
            case ECrowd_PackHuntMode::Patrolling:
                // Check if any herd is within hunting range
                for (const FCrowd_HerdDescriptor& Herd : RegisteredHerds)
                {
                    float Dist = FVector::Dist(Pack.PackCenterLocation, Herd.HerdCenterLocation);
                    if (Dist < PackHuntingRadius)
                    {
                        Pack.HuntingMode = ECrowd_PackHuntMode::Stalking;
                        Pack.TargetHerdID = Herd.HerdID;
                        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Pack %s STALKING herd %s"),
                            *Pack.PackID.ToString(), *Herd.HerdID.ToString());
                        break;
                    }
                }
                break;

            case ECrowd_PackHuntMode::Stalking:
                // Transition to coordinated attack if close enough
                for (const FCrowd_HerdDescriptor& Herd : RegisteredHerds)
                {
                    if (Herd.HerdID == Pack.TargetHerdID)
                    {
                        float Dist = FVector::Dist(Pack.PackCenterLocation, Herd.HerdCenterLocation);
                        if (Dist < FleeResponseRadius)
                        {
                            Pack.HuntingMode = ECrowd_PackHuntMode::CoordinatedAttack;
                            // Alert the herd
                            for (FCrowd_HerdDescriptor& MutableHerd : RegisteredHerds)
                            {
                                if (MutableHerd.HerdID == Pack.TargetHerdID)
                                {
                                    MutableHerd.ThreatLevel = 1.0f;
                                    MutableHerd.CurrentState = ECrowd_HerdState::Fleeing;
                                }
                            }
                        }
                        break;
                    }
                }
                break;

            case ECrowd_PackHuntMode::CoordinatedAttack:
                // After attack, return to patrol
                Pack.HuntCooldown -= DeltaTime;
                if (Pack.HuntCooldown <= 0.0f)
                {
                    Pack.HuntingMode = ECrowd_PackHuntMode::Patrolling;
                    Pack.TargetHerdID = FName(NAME_None);
                    Pack.HuntCooldown = 120.0f; // 2 min cooldown
                }
                break;

            default:
                break;
        }
    }
}

void UCrowdSimulationManager::NotifyThreatAtLocation(FVector ThreatLocation, float ThreatRadius, float ThreatIntensity)
{
    for (FCrowd_HerdDescriptor& Herd : RegisteredHerds)
    {
        float Dist = FVector::Dist(ThreatLocation, Herd.HerdCenterLocation);
        if (Dist < ThreatRadius)
        {
            float DistanceFactor = 1.0f - (Dist / ThreatRadius);
            Herd.ThreatLevel = FMath::Max(Herd.ThreatLevel, ThreatIntensity * DistanceFactor);
            UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Herd %s threat updated to %.2f"),
                *Herd.HerdID.ToString(), Herd.ThreatLevel);
        }
    }
}

int32 UCrowdSimulationManager::GetTotalActiveAgents() const
{
    int32 Total = 0;
    for (const FCrowd_HerdDescriptor& Herd : RegisteredHerds)
    {
        Total += Herd.AgentCount;
    }
    for (const FCrowd_PackDescriptor& Pack : RegisteredPacks)
    {
        Total += Pack.AgentCount;
    }
    return Total;
}

TArray<FCrowd_HerdDescriptor> UCrowdSimulationManager::GetHerdsInRadius(FVector Center, float Radius) const
{
    TArray<FCrowd_HerdDescriptor> Result;
    for (const FCrowd_HerdDescriptor& Herd : RegisteredHerds)
    {
        if (FVector::Dist(Center, Herd.HerdCenterLocation) <= Radius)
        {
            Result.Add(Herd);
        }
    }
    return Result;
}

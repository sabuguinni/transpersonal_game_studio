// CrowdSimulationManager.cpp
// Agent #13 — Crowd & Traffic Simulation
// Central manager for all crowd simulation: herds, stampedes, NPC groups, LOD scaling.

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxSimulatedAgents = 500;
    LODDistanceNear = 2000.0f;
    LODDistanceMid = 6000.0f;
    LODDistanceFar = 15000.0f;
    bCrowdSystemActive = true;
    TotalActiveAgents = 0;
    SimulationTickRate = 0.1f;
    AccumulatedTime = 0.0f;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimManager] Initialized — MaxAgents: %d"), MaxSimulatedAgents);
}

void UCrowdSimulationManager::Deinitialize()
{
    ActiveHerds.Empty();
    ActiveStampedes.Empty();
    AgentLODMap.Empty();
    Super::Deinitialize();
}

void UCrowdSimulationManager::Tick(float DeltaTime)
{
    if (!bCrowdSystemActive) return;

    AccumulatedTime += DeltaTime;
    if (AccumulatedTime < SimulationTickRate) return;
    AccumulatedTime = 0.0f;

    UpdateAgentLOD();
    UpdateCrowdDensityMetrics();
}

bool UCrowdSimulationManager::IsTickable() const
{
    return bCrowdSystemActive;
}

TStatId UCrowdSimulationManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UCrowdSimulationManager, STATGROUP_Tickables);
}

void UCrowdSimulationManager::RegisterHerd(UCrowd_HerdBehaviorComponent* HerdComponent)
{
    if (!IsValid(HerdComponent)) return;
    ActiveHerds.AddUnique(HerdComponent);
    TotalActiveAgents = ComputeTotalAgentCount();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimManager] Herd registered — Total herds: %d, Total agents: %d"),
        ActiveHerds.Num(), TotalActiveAgents);
}

void UCrowdSimulationManager::UnregisterHerd(UCrowd_HerdBehaviorComponent* HerdComponent)
{
    ActiveHerds.Remove(HerdComponent);
    TotalActiveAgents = ComputeTotalAgentCount();
}

void UCrowdSimulationManager::TriggerGlobalPanic(FVector EpicenterLocation, float PanicRadius, float ThreatIntensity)
{
    UE_LOG(LogTemp, Warning, TEXT("[CrowdSimManager] GLOBAL PANIC triggered at (%.0f, %.0f, %.0f) radius=%.0f intensity=%.2f"),
        EpicenterLocation.X, EpicenterLocation.Y, EpicenterLocation.Z, PanicRadius, ThreatIntensity);

    for (UCrowd_HerdBehaviorComponent* Herd : ActiveHerds)
    {
        if (!IsValid(Herd)) continue;

        // Check if any agent in this herd is within panic radius
        bool bAffected = false;
        for (const FCrowd_HerdAgent& Agent : Herd->HerdAgents)
        {
            if (FVector::Dist(Agent.AgentLocation, EpicenterLocation) < PanicRadius)
            {
                bAffected = true;
                break;
            }
        }

        if (bAffected)
        {
            Herd->ThreatLevel = FMath::Max(Herd->ThreatLevel, ThreatIntensity);
            Herd->TriggerPanic(EpicenterLocation, PanicRadius);
        }
    }
}

void UCrowdSimulationManager::SetCrowdSystemActive(bool bActive)
{
    bCrowdSystemActive = bActive;
    UE_LOG(LogTemp, Log, TEXT("[CrowdSimManager] Crowd system %s"), bActive ? TEXT("ACTIVATED") : TEXT("DEACTIVATED"));
}

void UCrowdSimulationManager::UpdateAgentLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    for (UCrowd_HerdBehaviorComponent* Herd : ActiveHerds)
    {
        if (!IsValid(Herd)) continue;

        // Compute herd center
        FVector HerdCenter = FVector::ZeroVector;
        for (const FCrowd_HerdAgent& Agent : Herd->HerdAgents)
            HerdCenter += Agent.AgentLocation;
        if (Herd->HerdAgents.Num() > 0)
            HerdCenter /= Herd->HerdAgents.Num();

        float DistToPlayer = FVector::Dist(PlayerLoc, HerdCenter);

        // Adjust tick rate based on LOD distance
        if (DistToPlayer < LODDistanceNear)
        {
            Herd->PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — full detail
        }
        else if (DistToPlayer < LODDistanceMid)
        {
            Herd->PrimaryComponentTick.TickInterval = 0.1f;  // 10Hz — medium
        }
        else if (DistToPlayer < LODDistanceFar)
        {
            Herd->PrimaryComponentTick.TickInterval = 0.25f; // 4Hz — low
        }
        else
        {
            Herd->PrimaryComponentTick.TickInterval = 1.0f;  // 1Hz — culled
        }
    }
}

void UCrowdSimulationManager::UpdateCrowdDensityMetrics()
{
    TotalActiveAgents = ComputeTotalAgentCount();
}

int32 UCrowdSimulationManager::ComputeTotalAgentCount() const
{
    int32 Total = 0;
    for (const UCrowd_HerdBehaviorComponent* Herd : ActiveHerds)
    {
        if (IsValid(Herd))
            Total += Herd->HerdAgents.Num();
    }
    return Total;
}

int32 UCrowdSimulationManager::GetTotalActiveAgents() const
{
    return TotalActiveAgents;
}

int32 UCrowdSimulationManager::GetActiveHerdCount() const
{
    return ActiveHerds.Num();
}

float UCrowdSimulationManager::GetAverageHerdDensity() const
{
    if (ActiveHerds.Num() == 0) return 0.0f;
    float Total = 0.0f;
    int32 Valid = 0;
    for (const UCrowd_HerdBehaviorComponent* Herd : ActiveHerds)
    {
        if (IsValid(Herd))
        {
            Total += Herd->HerdDensityScore;
            ++Valid;
        }
    }
    return Valid > 0 ? Total / Valid : 0.0f;
}

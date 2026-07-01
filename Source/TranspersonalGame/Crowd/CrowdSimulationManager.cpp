// CrowdSimulationManager.cpp
// Crowd & Traffic Simulation Agent #13
// Transpersonal Game Studio — Prehistoric Survival Game
// Manages up to 50,000 simultaneous crowd agents using UE5 Mass AI

#include "CrowdSimulationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UCrowdSimulationManager::UCrowdSimulationManager()
{
    MaxActiveAgents = 50000;
    LODDistanceNear = 2000.0f;
    LODDistanceMid = 8000.0f;
    LODDistanceFar = 20000.0f;
    StampedeRadius = 1500.0f;
    HerdCohesionRadius = 400.0f;
    MigrationCycleHours = 6.0f;
    bStampedeActive = false;
    bMigrationActive = false;
    ActiveAgentCount = 0;
}

void UCrowdSimulationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Initializing Crowd Simulation Manager — max agents: %d"), MaxActiveAgents);
    
    // Register herd species configurations
    RegisterSpeciesConfig(ECrowd_DinoSpecies::Brachiosaurus, 5, 15, 800.0f, 300.0f, false);
    RegisterSpeciesConfig(ECrowd_DinoSpecies::Triceratops, 8, 20, 600.0f, 250.0f, false);
    RegisterSpeciesConfig(ECrowd_DinoSpecies::Raptor, 3, 8, 1200.0f, 150.0f, true);
    RegisterSpeciesConfig(ECrowd_DinoSpecies::Parasaurolophus, 10, 30, 700.0f, 200.0f, false);
    RegisterSpeciesConfig(ECrowd_DinoSpecies::Pterodactyl, 4, 12, 1500.0f, 0.0f, true);

    // Start migration timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MigrationTimerHandle,
            this,
            &UCrowdSimulationManager::UpdateMigrationCycle,
            MigrationCycleHours * 3600.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Manager initialized — %d species registered"), SpeciesConfigs.Num());
}

void UCrowdSimulationManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MigrationTimerHandle);
    }
    ActiveHerds.Empty();
    SpeciesConfigs.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Manager deinitialized"));
}

void UCrowdSimulationManager::RegisterSpeciesConfig(
    ECrowd_DinoSpecies Species,
    int32 MinHerdSize,
    int32 MaxHerdSize,
    float MoveSpeed,
    float PersonalSpace,
    bool bIsPredator)
{
    FCrowd_SpeciesConfig Config;
    Config.Species = Species;
    Config.MinHerdSize = MinHerdSize;
    Config.MaxHerdSize = MaxHerdSize;
    Config.MoveSpeed = MoveSpeed;
    Config.PersonalSpace = PersonalSpace;
    Config.bIsPredator = bIsPredator;
    Config.FleeSpeed = MoveSpeed * 1.8f;
    Config.AttackRange = bIsPredator ? 200.0f : 0.0f;
    
    SpeciesConfigs.Add(Species, Config);
}

void UCrowdSimulationManager::SpawnHerd(
    ECrowd_DinoSpecies Species,
    FVector SpawnCenter,
    int32 AgentCount,
    ECrowd_HerdBehavior InitialBehavior)
{
    if (!SpeciesConfigs.Contains(Species))
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] SpawnHerd: Unknown species %d"), (int32)Species);
        return;
    }

    FCrowd_HerdData NewHerd;
    NewHerd.HerdID = FGuid::NewGuid();
    NewHerd.Species = Species;
    NewHerd.AgentCount = FMath::Clamp(AgentCount, 1, MaxActiveAgents - ActiveAgentCount);
    NewHerd.CurrentBehavior = InitialBehavior;
    NewHerd.CenterLocation = SpawnCenter;
    NewHerd.MigrationTarget = SpawnCenter;
    NewHerd.bIsLeaderAlive = true;
    NewHerd.ThreatLevel = 0.0f;
    NewHerd.LastUpdateTime = 0.0f;

    // Distribute agents around spawn center
    const FCrowd_SpeciesConfig& Config = SpeciesConfigs[Species];
    for (int32 i = 0; i < NewHerd.AgentCount; ++i)
    {
        float Angle = (float)i / (float)NewHerd.AgentCount * 2.0f * PI;
        float Radius = FMath::RandRange(0.0f, Config.PersonalSpace * 3.0f);
        FVector AgentPos = SpawnCenter + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        NewHerd.AgentLocations.Add(AgentPos);
    }

    ActiveHerds.Add(NewHerd.HerdID, NewHerd);
    ActiveAgentCount += NewHerd.AgentCount;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Spawned herd %s — species: %d, agents: %d, behavior: %d"),
        *NewHerd.HerdID.ToString(), (int32)Species, NewHerd.AgentCount, (int32)InitialBehavior);

    OnHerdSpawned.Broadcast(NewHerd.HerdID, Species, NewHerd.AgentCount);
}

void UCrowdSimulationManager::TriggerStampede(
    FVector TriggerLocation,
    ECrowd_StampedeType StampedeType,
    float Radius,
    float Duration)
{
    if (bStampedeActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("[CrowdSim] Stampede already active — ignoring new trigger"));
        return;
    }

    bStampedeActive = true;
    ActiveStampedeLocation = TriggerLocation;
    ActiveStampedeType = StampedeType;

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] STAMPEDE TRIGGERED at %s — type: %d, radius: %.0f, duration: %.1fs"),
        *TriggerLocation.ToString(), (int32)StampedeType, Radius, Duration);

    // Affect all herds within radius
    int32 AffectedHerds = 0;
    for (auto& HerdPair : ActiveHerds)
    {
        FCrowd_HerdData& Herd = HerdPair.Value;
        float DistToTrigger = FVector::Dist(Herd.CenterLocation, TriggerLocation);
        
        if (DistToTrigger <= Radius)
        {
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Stampede;
            Herd.ThreatLevel = 1.0f;
            
            // Set flee direction away from trigger
            FVector FleeDir = (Herd.CenterLocation - TriggerLocation).GetSafeNormal();
            Herd.MigrationTarget = Herd.CenterLocation + FleeDir * 5000.0f;
            
            AffectedHerds++;
            OnHerdBehaviorChanged.Broadcast(HerdPair.Key, ECrowd_HerdBehavior::Stampede);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Stampede affected %d herds"), AffectedHerds);
    OnStampedeTriggered.Broadcast(TriggerLocation, StampedeType, AffectedHerds);

    // Schedule stampede end
    if (UWorld* World = GetWorld())
    {
        FTimerHandle StampedeEndHandle;
        World->GetTimerManager().SetTimer(
            StampedeEndHandle,
            [this]() { EndStampede(); },
            Duration,
            false
        );
    }
}

void UCrowdSimulationManager::EndStampede()
{
    bStampedeActive = false;
    
    // Return herds to normal behavior
    for (auto& HerdPair : ActiveHerds)
    {
        FCrowd_HerdData& Herd = HerdPair.Value;
        if (Herd.CurrentBehavior == ECrowd_HerdBehavior::Stampede)
        {
            Herd.CurrentBehavior = ECrowd_HerdBehavior::Grazing;
            Herd.ThreatLevel = FMath::Max(0.0f, Herd.ThreatLevel - 0.5f);
            OnHerdBehaviorChanged.Broadcast(HerdPair.Key, ECrowd_HerdBehavior::Grazing);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Stampede ended — herds returning to normal behavior"));
}

void UCrowdSimulationManager::UpdateMigrationCycle()
{
    if (!bMigrationActive)
    {
        bMigrationActive = true;
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Migration cycle started"));
    }

    // Update migration targets for all herds
    for (auto& HerdPair : ActiveHerds)
    {
        FCrowd_HerdData& Herd = HerdPair.Value;
        
        // Only migrate herbivores
        if (SpeciesConfigs.Contains(Herd.Species) && !SpeciesConfigs[Herd.Species].bIsPredator)
        {
            if (Herd.CurrentBehavior != ECrowd_HerdBehavior::Stampede)
            {
                Herd.CurrentBehavior = ECrowd_HerdBehavior::Migrating;
                
                // Set new migration target (seasonal movement pattern)
                float MigAngle = FMath::RandRange(0.0f, 2.0f * PI);
                float MigDist = FMath::RandRange(3000.0f, 8000.0f);
                Herd.MigrationTarget = Herd.CenterLocation + FVector(
                    FMath::Cos(MigAngle) * MigDist,
                    FMath::Sin(MigAngle) * MigDist,
                    0.0f
                );
                
                OnHerdBehaviorChanged.Broadcast(HerdPair.Key, ECrowd_HerdBehavior::Migrating);
            }
        }
    }
}

void UCrowdSimulationManager::UpdateLOD(FVector PlayerLocation)
{
    for (auto& HerdPair : ActiveHerds)
    {
        FCrowd_HerdData& Herd = HerdPair.Value;
        float DistToPlayer = FVector::Dist(Herd.CenterLocation, PlayerLocation);
        
        ECrowd_LODLevel NewLOD;
        if (DistToPlayer <= LODDistanceNear)
            NewLOD = ECrowd_LODLevel::Full;
        else if (DistToPlayer <= LODDistanceMid)
            NewLOD = ECrowd_LODLevel::Medium;
        else if (DistToPlayer <= LODDistanceFar)
            NewLOD = ECrowd_LODLevel::Low;
        else
            NewLOD = ECrowd_LODLevel::Culled;
        
        if (Herd.CurrentLOD != NewLOD)
        {
            Herd.CurrentLOD = NewLOD;
            // LOD change would trigger mesh/animation quality update
        }
    }
}

int32 UCrowdSimulationManager::GetActiveAgentCount() const
{
    return ActiveAgentCount;
}

TArray<FGuid> UCrowdSimulationManager::GetHerdsInRadius(FVector Center, float Radius) const
{
    TArray<FGuid> Result;
    for (const auto& HerdPair : ActiveHerds)
    {
        if (FVector::Dist(HerdPair.Value.CenterLocation, Center) <= Radius)
        {
            Result.Add(HerdPair.Key);
        }
    }
    return Result;
}

void UCrowdSimulationManager::SetHerdBehavior(FGuid HerdID, ECrowd_HerdBehavior NewBehavior)
{
    if (ActiveHerds.Contains(HerdID))
    {
        ActiveHerds[HerdID].CurrentBehavior = NewBehavior;
        OnHerdBehaviorChanged.Broadcast(HerdID, NewBehavior);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s behavior set to %d"), *HerdID.ToString(), (int32)NewBehavior);
    }
}

void UCrowdSimulationManager::DisbandHerd(FGuid HerdID)
{
    if (ActiveHerds.Contains(HerdID))
    {
        ActiveAgentCount -= ActiveHerds[HerdID].AgentCount;
        ActiveHerds.Remove(HerdID);
        UE_LOG(LogTemp, Log, TEXT("[CrowdSim] Herd %s disbanded"), *HerdID.ToString());
    }
}

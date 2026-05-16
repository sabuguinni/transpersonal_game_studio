#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassSimulationManager::UCrowd_MassSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    // Initialize default values
    MaxTotalEntities = 50000;
    TickInterval = 0.1f;
    bEnablePerformanceOptimization = true;
    CombatPanicDuration = 30.0f;
    DamageProcessingRadius = 1000.0f;
    CurrentFPS = 60.0f;
    CurrentEntityCount = 0;
    bPerformanceModeActive = false;
    LastTickTime = 0.0f;
    bIsInitialized = false;

    // Setup default crowd archetypes
    FCrowd_EntityArchetype DefaultArchetype;
    DefaultArchetype.ArchetypeName = "PrehistoricTribal";
    DefaultArchetype.MaxEntities = 5000;
    DefaultArchetype.SpawnRadius = 10000.0f;
    DefaultArchetype.SpawnCenter = FVector(0, 0, 100);
    DefaultArchetype.BehaviorType = ECrowdBehaviorType::Wandering;
    DefaultArchetype.MovementSpeed = 350.0f;
    DefaultArchetype.bCanTakeDamage = true;
    DefaultArchetype.MaxHealth = 80.0f;
    CrowdArchetypes.Add(DefaultArchetype);

    FCrowd_EntityArchetype PanicArchetype;
    PanicArchetype.ArchetypeName = "PanickedCivilians";
    PanicArchetype.MaxEntities = 2000;
    PanicArchetype.SpawnRadius = 5000.0f;
    PanicArchetype.SpawnCenter = FVector(5000, 5000, 100);
    PanicArchetype.BehaviorType = ECrowdBehaviorType::Fleeing;
    PanicArchetype.MovementSpeed = 600.0f;
    PanicArchetype.bCanTakeDamage = true;
    PanicArchetype.MaxHealth = 60.0f;
    CrowdArchetypes.Add(PanicArchetype);
}

void UCrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: BeginPlay started"));
    
    // Get Mass Entity subsystems
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (MassEntitySubsystem && MassSpawnerSubsystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Mass subsystems found"));
            InitializeMassSimulation();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Failed to get Mass subsystems"));
        }
    }
}

void UCrowd_MassSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized)
    {
        return;
    }
    
    LastTickTime += DeltaTime;
    
    // Update at specified interval
    if (LastTickTime >= TickInterval)
    {
        UpdateCombatZones(LastTickTime);
        ProcessCrowdDamage(LastTickTime);
        
        if (bEnablePerformanceOptimization)
        {
            OptimizePerformance(LastTickTime);
        }
        
        UpdateEntityLOD(LastTickTime);
        LastTickTime = 0.0f;
    }
}

void UCrowd_MassSimulationManager::InitializeMassSimulation()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: MassEntitySubsystem is null"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Initializing Mass simulation"));
    
    // Clear any existing entities
    DespawnAllEntities();
    
    // Initialize entity manager
    CurrentEntityCount = 0;
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Mass simulation initialized successfully"));
}

void UCrowd_MassSimulationManager::SpawnCrowdArchetype(const FCrowd_EntityArchetype& Archetype)
{
    if (!bIsInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd_MassSimulationManager: Cannot spawn archetype - not initialized"));
        return;
    }
    
    if (CurrentEntityCount + Archetype.MaxEntities > MaxTotalEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Cannot spawn %s - would exceed max entities (%d/%d)"), 
               *Archetype.ArchetypeName, CurrentEntityCount + Archetype.MaxEntities, MaxTotalEntities);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Spawning archetype %s with %d entities"), 
           *Archetype.ArchetypeName, Archetype.MaxEntities);
    
    // Create entities in a circular pattern around spawn center
    for (int32 i = 0; i < Archetype.MaxEntities; ++i)
    {
        float Angle = (2.0f * PI * i) / Archetype.MaxEntities;
        float Distance = FMath::RandRange(100.0f, Archetype.SpawnRadius);
        
        FVector SpawnLocation = Archetype.SpawnCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Create entity (simplified for now - would use proper Mass Entity creation in full implementation)
        CurrentEntityCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Successfully spawned %d entities. Total: %d"), 
           Archetype.MaxEntities, CurrentEntityCount);
}

void UCrowd_MassSimulationManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Despawning all entities"));
    
    // Clear all entities (simplified - would use proper Mass Entity destruction)
    CurrentEntityCount = 0;
    ActiveCombatZones.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: All entities despawned"));
}

int32 UCrowd_MassSimulationManager::GetTotalEntityCount() const
{
    return CurrentEntityCount;
}

void UCrowd_MassSimulationManager::RegisterCombatZone(const FCrowd_CombatZone& CombatZone)
{
    FCrowd_CombatZone NewZone = CombatZone;
    NewZone.bIsActive = true;
    NewZone.CombatStartTime = GetWorld()->GetTimeSeconds();
    
    ActiveCombatZones.Add(NewZone);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Registered combat zone at %s with radius %f"), 
           *CombatZone.ZoneCenter.ToString(), CombatZone.ZoneRadius);
    
    // Trigger immediate panic response
    TriggerCombatPanic(CombatZone.ZoneCenter, CombatZone.PanicRadius);
}

void UCrowd_MassSimulationManager::TriggerCombatPanic(FVector CombatLocation, float PanicRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Triggering combat panic at %s with radius %f"), 
           *CombatLocation.ToString(), PanicRadius);
    
    // Switch entities in panic radius to fleeing behavior
    int32 PanickedEntities = 0;
    
    // Simplified panic calculation - in full implementation would iterate through Mass entities
    float PanicArea = PI * PanicRadius * PanicRadius;
    float EntityDensity = CurrentEntityCount / (PI * 10000.0f * 10000.0f); // Assume 10km radius for total area
    PanickedEntities = FMath::RoundToInt(PanicArea * EntityDensity);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: %d entities entering panic state"), PanickedEntities);
}

void UCrowd_MassSimulationManager::ApplyMassDamage(FVector DamageCenter, float DamageRadius, float DamageAmount)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Applying mass damage at %s - radius: %f, damage: %f"), 
           *DamageCenter.ToString(), DamageRadius, DamageAmount);
    
    int32 AffectedEntities = 0;
    int32 KilledEntities = 0;
    
    // Simplified damage calculation
    float DamageArea = PI * DamageRadius * DamageRadius;
    float EntityDensity = CurrentEntityCount / (PI * 10000.0f * 10000.0f);
    AffectedEntities = FMath::RoundToInt(DamageArea * EntityDensity);
    
    // Assume 30% kill rate for mass damage events
    KilledEntities = FMath::RoundToInt(AffectedEntities * 0.3f);
    CurrentEntityCount = FMath::Max(0, CurrentEntityCount - KilledEntities);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Mass damage affected %d entities, killed %d. Remaining: %d"), 
           AffectedEntities, KilledEntities, CurrentEntityCount);
    
    // Trigger panic in surrounding area
    TriggerCombatPanic(DamageCenter, DamageRadius * 2.0f);
}

void UCrowd_MassSimulationManager::SetGlobalBehaviorMode(ECrowdBehaviorType NewBehaviorType)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Setting global behavior mode to %d"), (int32)NewBehaviorType);
    
    // Update all archetype behaviors
    for (FCrowd_EntityArchetype& Archetype : CrowdArchetypes)
    {
        Archetype.BehaviorType = NewBehaviorType;
    }
}

void UCrowd_MassSimulationManager::UpdateCrowdDensity(FVector Location, float Radius, float DensityMultiplier)
{
    UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Updating crowd density at %s - radius: %f, multiplier: %f"), 
           *Location.ToString(), Radius, DensityMultiplier);
}

void UCrowd_MassSimulationManager::SetLODDistance(float NearLOD, float MidLOD, float FarLOD)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Setting LOD distances - Near: %f, Mid: %f, Far: %f"), 
           NearLOD, MidLOD, FarLOD);
}

void UCrowd_MassSimulationManager::EnablePerformanceMode(bool bEnable)
{
    bPerformanceModeActive = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Performance mode %s"), 
           bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    if (bEnable)
    {
        // Reduce entity count by 50% in performance mode
        int32 TargetCount = MaxTotalEntities / 2;
        if (CurrentEntityCount > TargetCount)
        {
            int32 EntitiesToRemove = CurrentEntityCount - TargetCount;
            CurrentEntityCount = TargetCount;
            UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Performance mode removed %d entities"), EntitiesToRemove);
        }
    }
}

void UCrowd_MassSimulationManager::SpawnTestCrowd()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Spawning test crowd"));
    
    if (CrowdArchetypes.Num() > 0)
    {
        FCrowd_EntityArchetype TestArchetype = CrowdArchetypes[0];
        TestArchetype.MaxEntities = 1000;
        TestArchetype.SpawnCenter = FVector(0, 0, 100);
        TestArchetype.SpawnRadius = 2000.0f;
        
        SpawnCrowdArchetype(TestArchetype);
    }
}

void UCrowd_MassSimulationManager::ClearAllCrowds()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Clearing all crowds"));
    DespawnAllEntities();
}

void UCrowd_MassSimulationManager::DebugPrintCrowdStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== CROWD SIMULATION STATS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Entities: %d / %d"), CurrentEntityCount, MaxTotalEntities);
    UE_LOG(LogTemp, Warning, TEXT("Active Combat Zones: %d"), ActiveCombatZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Mode: %s"), bPerformanceModeActive ? TEXT("ON") : TEXT("OFF"));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Archetype Count: %d"), CrowdArchetypes.Num());
    
    for (int32 i = 0; i < CrowdArchetypes.Num(); ++i)
    {
        const FCrowd_EntityArchetype& Archetype = CrowdArchetypes[i];
        UE_LOG(LogTemp, Warning, TEXT("  Archetype %d: %s (%d entities)"), 
               i, *Archetype.ArchetypeName, Archetype.MaxEntities);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("==============================="));
}

void UCrowd_MassSimulationManager::UpdateCombatZones(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove expired combat zones
    for (int32 i = ActiveCombatZones.Num() - 1; i >= 0; --i)
    {
        FCrowd_CombatZone& Zone = ActiveCombatZones[i];
        
        if (CurrentTime - Zone.CombatStartTime > CombatPanicDuration)
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: Combat zone expired at %s"), 
                   *Zone.ZoneCenter.ToString());
            ActiveCombatZones.RemoveAt(i);
        }
    }
}

void UCrowd_MassSimulationManager::ProcessCrowdDamage(float DeltaTime)
{
    // Process ongoing damage effects in combat zones
    for (FCrowd_CombatZone& Zone : ActiveCombatZones)
    {
        if (Zone.bIsActive && Zone.EntitiesInZone > 0)
        {
            // Apply gradual damage to entities in combat zones
            float ZoneDamage = 5.0f * DeltaTime; // 5 damage per second
            // In full implementation, would iterate through entities in zone
        }
    }
}

void UCrowd_MassSimulationManager::OptimizePerformance(float DeltaTime)
{
    // Monitor performance and adjust entity count
    if (CurrentFPS < 30.0f && !bPerformanceModeActive)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd_MassSimulationManager: Low FPS detected (%.1f), enabling performance mode"), CurrentFPS);
        EnablePerformanceMode(true);
    }
    else if (CurrentFPS > 50.0f && bPerformanceModeActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd_MassSimulationManager: FPS recovered (%.1f), disabling performance mode"), CurrentFPS);
        EnablePerformanceMode(false);
    }
}

bool UCrowd_MassSimulationManager::IsLocationInCombatZone(const FVector& Location) const
{
    for (const FCrowd_CombatZone& Zone : ActiveCombatZones)
    {
        if (Zone.bIsActive)
        {
            float DistanceSquared = FVector::DistSquared(Location, Zone.ZoneCenter);
            if (DistanceSquared <= Zone.ZoneRadius * Zone.ZoneRadius)
            {
                return true;
            }
        }
    }
    return false;
}

void UCrowd_MassSimulationManager::UpdateEntityLOD(float DeltaTime)
{
    // Simplified LOD management - would be more complex in full implementation
    if (CurrentEntityCount > 30000)
    {
        // High entity count - use aggressive LOD
        UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd_MassSimulationManager: Using aggressive LOD for %d entities"), CurrentEntityCount);
    }
    else if (CurrentEntityCount > 10000)
    {
        // Medium entity count - use moderate LOD
        UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd_MassSimulationManager: Using moderate LOD for %d entities"), CurrentEntityCount);
    }
}
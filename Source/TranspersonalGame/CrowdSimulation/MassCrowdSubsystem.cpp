#include "MassCrowdSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "ZoneGraphSubsystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

UMassCrowdSubsystem::UMassCrowdSubsystem()
{
    MaxCrowdAgents = 50000;
    UpdateFrequency = 30.0f;
    CurrentCrowdCount = 0;
    bEmergencyActive = false;
}

void UMassCrowdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Initializing crowd simulation system"));
    
    // Get required subsystems
    EntitySubsystem = GetWorld()->GetSubsystem<UMassEntitySubsystem>();
    SpawnerSubsystem = GetWorld()->GetSubsystem<UMassSpawnerSubsystem>();
    ZoneGraphSubsystem = GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
    
    if (!EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Failed to get MassEntitySubsystem"));
        return;
    }
    
    InitializeMassEntity();
    SetupCrowdProcessors();
    
    // Start density update timer
    GetWorld()->GetTimerManager().SetTimer(
        DensityUpdateTimer,
        this,
        &UMassCrowdSubsystem::UpdateDensityZones,
        1.0f / UpdateFrequency,
        true
    );
}

void UMassCrowdSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Shutting down crowd simulation"));
    
    DespawnAllCrowdAgents();
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(DensityUpdateTimer);
        GetWorld()->GetTimerManager().ClearTimer(CrowdUpdateTimer);
    }
    
    Super::Deinitialize();
}

bool UMassCrowdSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UMassCrowdSubsystem::SpawnCrowdAgents(const TArray<FCrowdAgentData>& AgentData)
{
    if (!EntitySubsystem || !SpawnerSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Missing required subsystems for spawning"));
        return;
    }
    
    if (CurrentCrowdCount + AgentData.Num() > MaxCrowdAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Cannot spawn %d agents, would exceed max limit of %d"), 
               AgentData.Num(), MaxCrowdAgents);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Spawning %d crowd agents"), AgentData.Num());
    
    for (const FCrowdAgentData& Agent : AgentData)
    {
        // Create entity archetype with required fragments
        FMassArchetypeHandle ArchetypeHandle = EntitySubsystem->CreateArchetype(
            TArray<const UScriptStruct*>{
                FTransformFragment::StaticStruct(),
                FMassVelocityFragment::StaticStruct(),
                FMassMoveTargetFragment::StaticStruct(),
                FMassNavigationEdgesFragment::StaticStruct(),
                FMassForceFragment::StaticStruct()
            }
        );
        
        // Spawn entity
        FMassEntityHandle EntityHandle = EntitySubsystem->CreateEntity(ArchetypeHandle);
        
        if (EntityHandle.IsValid())
        {
            // Set transform
            FTransformFragment& TransformFragment = EntitySubsystem->GetFragmentDataChecked<FTransformFragment>(EntityHandle);
            TransformFragment.SetTransform(FTransform(Agent.SpawnRotation, Agent.SpawnLocation));
            
            // Set velocity
            FMassVelocityFragment& VelocityFragment = EntitySubsystem->GetFragmentDataChecked<FMassVelocityFragment>(EntityHandle);
            VelocityFragment.Value = FVector::ZeroVector;
            
            // Set movement target
            FMassMoveTargetFragment& MoveTargetFragment = EntitySubsystem->GetFragmentDataChecked<FMassMoveTargetFragment>(EntityHandle);
            MoveTargetFragment.Center = Agent.SpawnLocation;
            MoveTargetFragment.DesiredSpeed = FMassInt16Real(Agent.MovementSpeed);
            
            CurrentCrowdCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Total crowd count: %d"), CurrentCrowdCount);
}

void UMassCrowdSubsystem::DespawnAllCrowdAgents()
{
    if (!EntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Despawning all crowd agents"));
    
    // Note: In a real implementation, we'd iterate through all crowd entities
    // and destroy them individually. For now, we'll reset the counter.
    CurrentCrowdCount = 0;
    bEmergencyActive = false;
}

void UMassCrowdSubsystem::SetCrowdDensityZone(const FCrowdDensityZone& DensityZone)
{
    DensityZones.Add(DensityZone);
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Added density zone at %s with target density %d"), 
           *DensityZone.ZoneCenter.ToString(), DensityZone.TargetDensity);
}

int32 UMassCrowdSubsystem::GetTotalCrowdCount() const
{
    return CurrentCrowdCount;
}

void UMassCrowdSubsystem::SetGlobalCrowdBehavior(const FString& BehaviorName)
{
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Setting global crowd behavior to: %s"), *BehaviorName);
    
    // In a full implementation, this would update behavior tree or state tree assets
    // for all crowd entities
}

void UMassCrowdSubsystem::TriggerCrowdEmergencyResponse(const FVector& ThreatLocation, float ThreatRadius)
{
    bEmergencyActive = true;
    EmergencyThreatLocation = ThreatLocation;
    EmergencyThreatRadius = ThreatRadius;
    
    UE_LOG(LogTemp, Warning, TEXT("MassCrowdSubsystem: Emergency triggered at %s with radius %f"), 
           *ThreatLocation.ToString(), ThreatRadius);
    
    // In a full implementation, this would:
    // 1. Query all entities within threat radius
    // 2. Switch their behavior to panic/flee mode
    // 3. Calculate evacuation paths away from threat
}

void UMassCrowdSubsystem::SetCrowdEvacuationTarget(const FVector& EvacuationPoint)
{
    EvacuationTarget = EvacuationPoint;
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Set evacuation target to %s"), *EvacuationPoint.ToString());
}

void UMassCrowdSubsystem::SetCrowdLODDistance(float NearDistance, float MidDistance, float FarDistance)
{
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Set LOD distances - Near: %f, Mid: %f, Far: %f"), 
           NearDistance, MidDistance, FarDistance);
    
    // In a full implementation, this would configure Mass LOD system
}

void UMassCrowdSubsystem::EnableCrowdCulling(bool bEnable)
{
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Crowd culling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UMassCrowdSubsystem::InitializeMassEntity()
{
    if (!EntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassCrowdSubsystem: Cannot initialize Mass Entity - EntitySubsystem is null"));
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Initializing Mass Entity framework"));
    
    // Initialize Mass Entity processing pipeline
    // In a full implementation, this would set up:
    // - Entity archetypes for different crowd types
    // - Processing phases and dependencies
    // - LOD configuration
    // - Spatial partitioning
}

void UMassCrowdSubsystem::SetupCrowdProcessors()
{
    UE_LOG(LogTemp, Log, TEXT("MassCrowdSubsystem: Setting up crowd processors"));
    
    // In a full implementation, this would register:
    // - Movement processors
    // - Avoidance processors  
    // - Behavior processors
    // - LOD processors
    // - Visualization processors
}

void UMassCrowdSubsystem::UpdateDensityZones()
{
    // Update density zones and spawn/despawn agents as needed
    for (FCrowdDensityZone& Zone : DensityZones)
    {
        // In a full implementation, this would:
        // 1. Count entities in zone
        // 2. Compare to target density
        // 3. Spawn or despawn entities to reach target
        // 4. Update Zone.CurrentDensity
    }
}
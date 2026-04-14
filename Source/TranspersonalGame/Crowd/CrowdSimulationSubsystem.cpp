#include "CrowdSimulationSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassEntityConfigAsset.h"
#include "MassEntityTemplateRegistry.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UCrowdSimulationSubsystem::UCrowdSimulationSubsystem()
{
    // Initialize default values
    EntityConfig = FCrowd_EntityConfig();
    HighDetailDistance = 1000.0f;
    MediumDetailDistance = 2500.0f;
    LowDetailDistance = 5000.0f;
    ActiveEntityCount = 0;
    bCrowdSystemInitialized = false;
    bDebugVisualizationEnabled = false;
}

void UCrowdSimulationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initializing..."));
    
    // Set up Mass Entity references
    SetupMassEntityReferences();
    
    // Create default spawn zones
    if (SpawnZones.Num() == 0)
    {
        FCrowd_SpawnZone DefaultZone;
        DefaultZone.Center = FVector(0, 0, 100);
        DefaultZone.Extents = FVector(2000, 2000, 200);
        DefaultZone.TargetPopulation = 50;
        DefaultZone.BehaviorType = ECrowd_BehaviorType::Wandering;
        SpawnZones.Add(DefaultZone);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Initialized with %d spawn zones"), SpawnZones.Num());
}

void UCrowdSimulationSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: Deinitializing..."));
    
    // Clean up crowd entities
    DespawnAllCrowdEntities();
    
    // Clear references
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    
    Super::Deinitialize();
}

bool UCrowdSimulationSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UCrowdSimulationSubsystem::InitializeCrowdSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationSubsystem: InitializeCrowdSystem called"));
    
    if (bCrowdSystemInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Crowd system already initialized"));
        return;
    }
    
    // Ensure Mass Entity subsystems are available
    SetupMassEntityReferences();
    
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get MassEntitySubsystem - Mass Entity plugin may not be enabled"));
        return;
    }
    
    // Create crowd archetype and register processors
    CreateCrowdArchetype();
    RegisterCrowdProcessors();
    
    bCrowdSystemInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd system initialized successfully"));
}

void UCrowdSimulationSubsystem::SpawnCrowdEntities(const FCrowd_SpawnZone& SpawnZone)
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnCrowdEntities: Spawning %d entities in zone at %s"), 
           SpawnZone.TargetPopulation, *SpawnZone.Center.ToString());
    
    if (!bCrowdSystemInitialized)
    {
        InitializeCrowdSystem();
    }
    
    // For now, create simple placeholder entities
    // In a full implementation, this would use Mass Entity spawning
    for (int32 i = 0; i < SpawnZone.TargetPopulation; ++i)
    {
        FVector SpawnLocation = SpawnZone.Center + FVector(
            FMath::RandRange(-SpawnZone.Extents.X, SpawnZone.Extents.X),
            FMath::RandRange(-SpawnZone.Extents.Y, SpawnZone.Extents.Y),
            FMath::RandRange(-SpawnZone.Extents.Z, SpawnZone.Extents.Z)
        );
        
        // TODO: Replace with actual Mass Entity spawning
        ActiveEntityCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd entities. Total active: %d"), 
           SpawnZone.TargetPopulation, ActiveEntityCount);
}

void UCrowdSimulationSubsystem::DespawnAllCrowdEntities()
{
    UE_LOG(LogTemp, Warning, TEXT("DespawnAllCrowdEntities: Removing %d entities"), ActiveEntityCount);
    
    // TODO: Implement actual Mass Entity despawning
    ActiveEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("All crowd entities despawned"));
}

void UCrowdSimulationSubsystem::SetCrowdDensity(float DensityMultiplier)
{
    UE_LOG(LogTemp, Warning, TEXT("SetCrowdDensity: Setting density multiplier to %f"), DensityMultiplier);
    
    DensityMultiplier = FMath::Clamp(DensityMultiplier, 0.1f, 5.0f);
    
    // Update spawn zone populations
    for (FCrowd_SpawnZone& Zone : SpawnZones)
    {
        Zone.TargetPopulation = FMath::RoundToInt(Zone.TargetPopulation * DensityMultiplier);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd density updated"));
}

int32 UCrowdSimulationSubsystem::GetActiveCrowdEntityCount() const
{
    return ActiveEntityCount;
}

void UCrowdSimulationSubsystem::UpdateCrowdLOD(const FVector& PlayerLocation)
{
    // Update LOD based on distance from player
    UpdateEntityLOD(PlayerLocation);
}

void UCrowdSimulationSubsystem::SetLODDistances(float HighDetailDistance, float MediumDetailDistance, float LowDetailDistance)
{
    this->HighDetailDistance = HighDetailDistance;
    this->MediumDetailDistance = MediumDetailDistance;
    this->LowDetailDistance = LowDetailDistance;
    
    UE_LOG(LogTemp, Warning, TEXT("LOD distances updated: High=%f, Medium=%f, Low=%f"), 
           HighDetailDistance, MediumDetailDistance, LowDetailDistance);
}

void UCrowdSimulationSubsystem::SetGlobalCrowdBehavior(ECrowd_BehaviorType NewBehavior)
{
    UE_LOG(LogTemp, Warning, TEXT("SetGlobalCrowdBehavior: Setting behavior to %d"), (int32)NewBehavior);
    
    // Update all spawn zones to use new behavior
    for (FCrowd_SpawnZone& Zone : SpawnZones)
    {
        Zone.BehaviorType = NewBehavior;
    }
    
    // TODO: Update existing entities with new behavior
}

void UCrowdSimulationSubsystem::TriggerCrowdEvent(ECrowd_EventType EventType, const FVector& EventLocation, float EventRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("TriggerCrowdEvent: Event %d at %s with radius %f"), 
           (int32)EventType, *EventLocation.ToString(), EventRadius);
    
    HandleCrowdEvent(EventType, EventLocation, EventRadius);
}

void UCrowdSimulationSubsystem::DebugDrawCrowdInfo()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Draw spawn zones
    for (const FCrowd_SpawnZone& Zone : SpawnZones)
    {
        DrawDebugBox(World, Zone.Center, Zone.Extents, FColor::Green, false, 5.0f, 0, 10.0f);
        
        // Draw zone info text
        FString ZoneInfo = FString::Printf(TEXT("Population: %d\nBehavior: %d"), 
                                         Zone.TargetPopulation, (int32)Zone.BehaviorType);
        DrawDebugString(World, Zone.Center + FVector(0, 0, Zone.Extents.Z + 100), 
                       ZoneInfo, nullptr, FColor::White, 5.0f);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Debug info drawn for %d spawn zones"), SpawnZones.Num());
}

void UCrowdSimulationSubsystem::ToggleCrowdDebugVisualization()
{
    bDebugVisualizationEnabled = !bDebugVisualizationEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Crowd debug visualization: %s"), 
           bDebugVisualizationEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UCrowdSimulationSubsystem::SetupMassEntityReferences()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world available for Mass Entity setup"));
        return;
    }
    
    // Get Mass Entity subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassEntitySubsystem not available - Mass Entity plugin may not be enabled"));
    }
    
    // Get Mass Spawner subsystem
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (!MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("MassSpawnerSubsystem not available"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity references setup complete"));
}

void UCrowdSimulationSubsystem::CreateCrowdArchetype()
{
    UE_LOG(LogTemp, Warning, TEXT("CreateCrowdArchetype: Creating crowd entity archetype"));
    
    // TODO: Create Mass Entity archetype with required components
    // This would include: Transform, Movement, LOD, Behavior components
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd archetype creation completed"));
}

void UCrowdSimulationSubsystem::RegisterCrowdProcessors()
{
    UE_LOG(LogTemp, Warning, TEXT("RegisterCrowdProcessors: Registering Mass processors"));
    
    // TODO: Register custom processors for:
    // - Movement/Pathfinding
    // - LOD management
    // - Behavior execution
    // - Avoidance
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd processors registration completed"));
}

void UCrowdSimulationSubsystem::UpdateEntityLOD(const FVector& PlayerLocation)
{
    // TODO: Implement LOD updates based on distance from player
    // High detail: Full animation and AI
    // Medium detail: Reduced animation, simplified AI
    // Low detail: Static poses, no AI updates
}

void UCrowdSimulationSubsystem::HandleCrowdEvent(ECrowd_EventType EventType, const FVector& Location, float Radius)
{
    // TODO: Implement crowd event responses
    switch (EventType)
    {
    case ECrowd_EventType::Panic:
        // Make entities flee from location
        break;
    case ECrowd_EventType::Curiosity:
        // Make entities move toward location
        break;
    case ECrowd_EventType::Celebration:
        // Make entities gather and celebrate
        break;
    default:
        break;
    }
}
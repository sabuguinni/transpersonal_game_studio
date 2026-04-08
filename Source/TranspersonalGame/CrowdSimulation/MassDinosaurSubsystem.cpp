#include "MassDinosaurSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/World.h"
#include "DinosaurFragments.h"
#include "DinosaurProcessors.h"

void UMassDinosaurSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    NextHerdID = 1;
    MaxEntitiesPerFrame = 50000; // Target: 50k simultaneous entities
    
    InitializeMassFramework();
    SetupDinosaurArchetypes();
    RegisterProcessors();
    
    UE_LOG(LogTemp, Warning, TEXT("MassDinosaurSubsystem initialized - Ready for up to %d entities"), MaxEntitiesPerFrame);
}

void UMassDinosaurSubsystem::Deinitialize()
{
    // Clean up all active herds
    for (auto& HerdPair : ActiveHerds)
    {
        if (MassEntitySubsystem && HerdPair.Value.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(HerdPair.Value);
        }
    }
    ActiveHerds.Empty();
    
    Super::Deinitialize();
}

bool UMassDinosaurSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UMassDinosaurSubsystem::InitializeMassFramework()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("MassDinosaurSubsystem: No valid world found"));
        return;
    }
    
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    
    if (!MassEntitySubsystem || !MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("MassDinosaurSubsystem: Failed to get Mass subsystems"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mass framework initialized successfully"));
}

void UMassDinosaurSubsystem::SetupDinosaurArchetypes()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Create base dinosaur archetype with essential fragments
    FMassArchetypeHandle HerbivoreArchetype = MassEntitySubsystem->CreateArchetype({
        FDinosaurSpeciesFragment::StaticStruct(),
        FDinosaurHerdFragment::StaticStruct(),
        FDinosaurBehaviorFragment::StaticStruct(),
        FMassTransformFragment::StaticStruct(),
        FMassVelocityFragment::StaticStruct(),
        FMassForceFragment::StaticStruct(),
        FMassNavigationEdgesFragment::StaticStruct(),
        FMassMoveTargetFragment::StaticStruct(),
        FMassAvoidanceColliderFragment::StaticStruct()
    });
    
    // Create carnivore archetype with hunting behavior
    FMassArchetypeHandle CarnivoreArchetype = MassEntitySubsystem->CreateArchetype({
        FDinosaurSpeciesFragment::StaticStruct(),
        FDinosaurHuntingFragment::StaticStruct(),
        FDinosaurBehaviorFragment::StaticStruct(),
        FMassTransformFragment::StaticStruct(),
        FMassVelocityFragment::StaticStruct(),
        FMassForceFragment::StaticStruct(),
        FMassNavigationEdgesFragment::StaticStruct(),
        FMassMoveTargetFragment::StaticStruct(),
        FMassAvoidanceColliderFragment::StaticStruct()
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur archetypes created - Herbivore and Carnivore"));
}

void UMassDinosaurSubsystem::RegisterProcessors()
{
    if (!MassSimulationSubsystem)
    {
        return;
    }
    
    // Register core dinosaur behavior processors
    // These will be executed in order during each simulation tick
    
    // 1. Herd cohesion and separation
    MassSimulationSubsystem->RegisterProcessor<UDinosaurHerdBehaviorProcessor>();
    
    // 2. Predator-prey interactions
    MassSimulationSubsystem->RegisterProcessor<UDinosaurPredatorPreyProcessor>();
    
    // 3. Migration and seasonal movement
    MassSimulationSubsystem->RegisterProcessor<UDinosaurMigrationProcessor>();
    
    // 4. Environmental response (player proximity, sounds, etc)
    MassSimulationSubsystem->RegisterProcessor<UDinosaurEnvironmentProcessor>();
    
    // 5. Movement and avoidance (using Mass Avoidance system)
    MassSimulationSubsystem->RegisterProcessor<UMassMovingAvoidanceProcessor>();
    
    // 6. LOD management for performance
    MassSimulationSubsystem->RegisterProcessor<UDinosaurLODProcessor>();
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur processors registered successfully"));
}

void UMassDinosaurSubsystem::SpawnHerd(const FVector& Location, int32 HerdSize, TSubclassOf<ADinosaurBase> DinosaurClass)
{
    if (!MassEntitySubsystem || HerdSize <= 0)
    {
        return;
    }
    
    // Clamp herd size to prevent performance issues
    HerdSize = FMath::Clamp(HerdSize, 1, 200);
    
    // Check if we're approaching entity limit
    if (GetActiveEntityCount() + HerdSize > MaxEntitiesPerFrame)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn herd - would exceed entity limit"));
        return;
    }
    
    // Create herd entities in a scattered formation
    TArray<FMassEntityHandle> HerdEntities;
    
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Scatter entities in a circle around the spawn location
        float Angle = (2.0f * PI * i) / HerdSize;
        float Radius = FMath::RandRange(50.0f, 300.0f);
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        FMassEntityHandle Entity = MassEntitySubsystem->CreateEntity(/* Archetype based on DinosaurClass */);
        
        if (Entity.IsValid())
        {
            // Initialize entity fragments
            FMassTransformFragment& Transform = MassEntitySubsystem->GetFragmentDataChecked<FMassTransformFragment>(Entity);
            Transform.SetTransform(FTransform(SpawnLocation));
            
            FDinosaurHerdFragment& HerdData = MassEntitySubsystem->GetFragmentDataChecked<FDinosaurHerdFragment>(Entity);
            HerdData.HerdID = NextHerdID;
            HerdData.HerdSize = HerdSize;
            HerdData.HerdCenter = Location;
            
            FDinosaurBehaviorFragment& Behavior = MassEntitySubsystem->GetFragmentDataChecked<FDinosaurBehaviorFragment>(Entity);
            Behavior.CurrentState = EDinosaurBehaviorState::Grazing;
            Behavior.StateTimer = FMath::RandRange(5.0f, 15.0f);
            
            HerdEntities.Add(Entity);
        }
    }
    
    // Store herd reference (using first entity as representative)
    if (HerdEntities.Num() > 0)
    {
        ActiveHerds.Add(NextHerdID, HerdEntities[0]);
        NextHerdID++;
        
        UE_LOG(LogTemp, Warning, TEXT("Spawned herd of %d dinosaurs at %s"), HerdSize, *Location.ToString());
    }
}

void UMassDinosaurSubsystem::DespawnHerd(int32 HerdID)
{
    if (!MassEntitySubsystem || !ActiveHerds.Contains(HerdID))
    {
        return;
    }
    
    // Find and destroy all entities belonging to this herd
    // This would require iterating through entities with matching HerdID
    // Implementation depends on Mass Entity query system
    
    ActiveHerds.Remove(HerdID);
    UE_LOG(LogTemp, Warning, TEXT("Despawned herd %d"), HerdID);
}

void UMassDinosaurSubsystem::TriggerMigration(const FVector& FromLocation, const FVector& ToLocation, float MigrationRadius)
{
    // Set migration targets for all herds within radius
    // This would be handled by the UDinosaurMigrationProcessor
    
    UE_LOG(LogTemp, Warning, TEXT("Migration triggered from %s to %s (radius: %.1f)"), 
           *FromLocation.ToString(), *ToLocation.ToString(), MigrationRadius);
}

void UMassDinosaurSubsystem::TriggerPredatorAlert(const FVector& ThreatLocation, float AlertRadius)
{
    // Trigger flee behavior for herbivores within radius
    // Trigger hunting behavior for carnivores within radius
    
    UE_LOG(LogTemp, Warning, TEXT("Predator alert at %s (radius: %.1f)"), 
           *ThreatLocation.ToString(), AlertRadius);
}

int32 UMassDinosaurSubsystem::GetActiveEntityCount() const
{
    if (!MassEntitySubsystem)
    {
        return 0;
    }
    
    // This would query the actual entity count from Mass Entity system
    return ActiveHerds.Num() * 50; // Rough estimate for now
}

float UMassDinosaurSubsystem::GetSimulationPerformance() const
{
    // Return performance metrics (frame time, entity count, etc.)
    return LastFrameTime;
}
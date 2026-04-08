#include "DinosaurMassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "MassSimulationLOD.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UDinosaurMassProcessor::UDinosaurMassProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UDinosaurMassProcessor::ConfigureQueries()
{
    // Herbivore behavior query
    HerbivoreQuery.AddRequirement<FDinosaurFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FDinosaurSensoryFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FDinosaurRoutineFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FMassVisualizationLODFragment>(EMassFragmentAccess::ReadOnly);
    HerbivoreQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly);
    
    // Carnivore behavior query
    CarnivoreQuery.AddRequirement<FDinosaurFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FDinosaurSensoryFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FDinosaurTerritoryFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FMassVisualizationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Pack behavior query
    PackQuery.AddRequirement<FDinosaurFragment>(EMassFragmentAccess::ReadWrite);
    PackQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    PackQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    PackQuery.AddRequirement<FDinosaurPackFragment>(EMassFragmentAccess::ReadWrite);
    PackQuery.AddRequirement<FDinosaurSensoryFragment>(EMassFragmentAccess::ReadWrite);
    
    // Territorial behavior query
    TerritorialQuery.AddRequirement<FDinosaurFragment>(EMassFragmentAccess::ReadWrite);
    TerritorialQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    TerritorialQuery.AddRequirement<FDinosaurTerritoryFragment>(EMassFragmentAccess::ReadWrite);
    TerritorialQuery.AddRequirement<FDinosaurSensoryFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Only process if we have active entities within simulation distance
    if (EntityManager.GetNumEntities() == 0)
    {
        return;
    }

    // Get current world time for behavior timing
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Process different behavior types
    ProcessHerbivores(EntityManager, Context);
    ProcessCarnivores(EntityManager, Context);
    ProcessPackBehavior(EntityManager, Context);
    ProcessTerritorialBehavior(EntityManager, Context);
    
    // Handle ecosystem interactions
    HandlePredatorPreyInteractions(EntityManager, Context);
    UpdateTerritorialBoundaries(EntityManager, Context);
    ProcessMigrationPatterns(EntityManager, Context);
}

void UDinosaurMassProcessor::ProcessHerbivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    HerbivoreQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FDinosaurFragment> DinosaurList = Context.GetMutableFragmentView<FDinosaurFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FDinosaurSensoryFragment> SensoryList = Context.GetMutableFragmentView<FDinosaurSensoryFragment>();
        const TArrayView<FDinosaurRoutineFragment> RoutineList = Context.GetMutableFragmentView<FDinosaurRoutineFragment>();
        const TArrayView<FMassVisualizationLODFragment> LODList = Context.GetFragmentView<FMassVisualizationLODFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FDinosaurFragment& Dinosaur = DinosaurList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FDinosaurSensoryFragment& Sensory = SensoryList[EntityIndex];
            FDinosaurRoutineFragment& Routine = RoutineList[EntityIndex];
            const FMassVisualizationLODFragment& LOD = LODList[EntityIndex];

            // Skip if too far from player (LOD optimization)
            if (LOD.LODSignificance > 0.8f)
            {
                continue;
            }

            // Filter for herbivore species
            if (Dinosaur.Species != EDinosaurSpecies::Triceratops && 
                Dinosaur.Species != EDinosaurSpecies::Brontosaurus &&
                Dinosaur.Species != EDinosaurSpecies::Stegosaurus &&
                Dinosaur.Species != EDinosaurSpecies::Parasaurolophus &&
                Dinosaur.Species != EDinosaurSpecies::Gallimimus &&
                Dinosaur.Species != EDinosaurSpecies::Compsognathus)
            {
                continue;
            }

            float DeltaTime = Context.GetDeltaTimeSeconds();
            float CurrentTime = GetWorld()->GetTimeSeconds();
            
            // Update needs over time
            Dinosaur.Hunger += DeltaTime * 2.0f; // Gets hungry over time
            Dinosaur.Thirst += DeltaTime * 1.5f; // Gets thirsty over time
            Dinosaur.Fear = FMath::Max(0.0f, Dinosaur.Fear - DeltaTime * 5.0f); // Fear decays
            
            // Check for predators in sensory range
            bool bPredatorDetected = false;
            FVector FleeDirection = FVector::ZeroVector;
            
            for (const FMassEntityHandle& SensedEntity : Sensory.VisibleEntities)
            {
                // Would need to check if sensed entity is a predator
                // For now, simulate predator detection
                if (FMath::RandRange(0.0f, 1.0f) < 0.05f) // 5% chance per frame
                {
                    bPredatorDetected = true;
                    FleeDirection = (Transform.GetTransform().GetLocation() - 
                                   FVector(FMath::RandRange(-1000, 1000), FMath::RandRange(-1000, 1000), 0)).GetSafeNormal();
                    Dinosaur.Fear = 100.0f;
                    break;
                }
            }

            // State machine logic
            switch (Dinosaur.CurrentState)
            {
                case EDinosaurBehaviorState::Idle:
                {
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (bPredatorDetected)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Fleeing;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.Hunger > 60.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Foraging;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.Thirst > 70.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Drinking;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.StateTimer > FMath::RandRange(5.0f, 15.0f))
                    {
                        // Random state change for natural behavior
                        int32 RandomState = FMath::RandRange(0, 2);
                        switch (RandomState)
                        {
                            case 0: Dinosaur.CurrentState = EDinosaurBehaviorState::Foraging; break;
                            case 1: Dinosaur.CurrentState = EDinosaurBehaviorState::Socializing; break;
                            case 2: Dinosaur.CurrentState = EDinosaurBehaviorState::Resting; break;
                        }
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Foraging:
                {
                    // Move towards feeding area and reduce hunger
                    FVector FeedingDirection = (Routine.PreferredFeedingArea - Transform.GetTransform().GetLocation()).GetSafeNormal();
                    Velocity.Value = FeedingDirection * 150.0f * Dinosaur.Traits.SizeVariation;
                    
                    Dinosaur.Hunger = FMath::Max(0.0f, Dinosaur.Hunger - DeltaTime * 10.0f);
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (bPredatorDetected)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Fleeing;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.Hunger < 20.0f || Dinosaur.StateTimer > 30.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Idle;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Fleeing:
                {
                    // Flee at maximum speed
                    Velocity.Value = FleeDirection * 400.0f * Dinosaur.Traits.SizeVariation;
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (Dinosaur.StateTimer > 10.0f || Dinosaur.Fear < 20.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Idle;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Drinking:
                {
                    // Move towards water source
                    FVector WaterDirection = (Routine.PreferredWaterSource - Transform.GetTransform().GetLocation()).GetSafeNormal();
                    Velocity.Value = WaterDirection * 100.0f * Dinosaur.Traits.SizeVariation;
                    
                    Dinosaur.Thirst = FMath::Max(0.0f, Dinosaur.Thirst - DeltaTime * 15.0f);
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (bPredatorDetected)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Fleeing;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.Thirst < 10.0f || Dinosaur.StateTimer > 20.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Idle;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Socializing:
                {
                    // Simple flocking behavior - move towards other herbivores
                    FVector FlockCenter = FVector::ZeroVector;
                    int32 FlockCount = 0;
                    
                    for (const FMassEntityHandle& NearbyEntity : Sensory.VisibleEntities)
                    {
                        // Would check if it's another herbivore
                        FlockCenter += Transform.GetTransform().GetLocation(); // Simplified
                        FlockCount++;
                    }
                    
                    if (FlockCount > 0)
                    {
                        FlockCenter /= FlockCount;
                        FVector FlockDirection = (FlockCenter - Transform.GetTransform().GetLocation()).GetSafeNormal();
                        Velocity.Value = FlockDirection * 80.0f * Dinosaur.Traits.SocialTendency;
                    }
                    
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (bPredatorDetected)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Fleeing;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.StateTimer > 25.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Idle;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Resting:
                {
                    // Move towards resting spot and stay still
                    FVector RestDirection = (Routine.PreferredRestingSpot - Transform.GetTransform().GetLocation()).GetSafeNormal();
                    float DistanceToRest = FVector::Dist(Transform.GetTransform().GetLocation(), Routine.PreferredRestingSpot);
                    
                    if (DistanceToRest > 100.0f)
                    {
                        Velocity.Value = RestDirection * 50.0f;
                    }
                    else
                    {
                        Velocity.Value = FVector::ZeroVector;
                    }
                    
                    Dinosaur.StateTimer += DeltaTime;
                    
                    if (bPredatorDetected)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Fleeing;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    else if (Dinosaur.StateTimer > 40.0f)
                    {
                        Dinosaur.CurrentState = EDinosaurBehaviorState::Idle;
                        Dinosaur.StateTimer = 0.0f;
                    }
                    break;
                }
            }
        }
    });
}

void UDinosaurMassProcessor::ProcessCarnivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementation for carnivore behavior (hunting, territorial patrol, etc.)
    // Similar structure to herbivores but with predatory behaviors
}

void UDinosaurMassProcessor::ProcessPackBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementation for pack hunting behavior (raptors, etc.)
    // Coordinate between pack members for group hunting
}

void UDinosaurMassProcessor::ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implementation for territorial species (T-Rex, etc.)
    // Patrol territory, chase away intruders
}

void UDinosaurMassProcessor::HandlePredatorPreyInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Cross-species interaction logic
    // Predators hunt prey, prey flees from predators
}

void UDinosaurMassProcessor::UpdateTerritorialBoundaries(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Update and maintain territorial boundaries
    // Handle territory conflicts between species
}

void UDinosaurMassProcessor::ProcessMigrationPatterns(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Seasonal migration behavior
    // Large herds moving between feeding areas
}
#include "MassDinosaurSystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassSimulationLOD.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

//----------------------------------------------------------------------//
// UMassDinosaurMovementProcessor
//----------------------------------------------------------------------//

UMassDinosaurMovementProcessor::UMassDinosaurMovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UMassDinosaurMovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassDinosaurMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TConstArrayView<FMassDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FMassDinosaurSpeciesFragment>();
        const TArrayView<FMassDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FMassDinosaurBehaviorFragment>();
        const TConstArrayView<FMassForceFragment> ForceList = Context.GetFragmentView<FMassForceFragment>();
        const FMassSimulationLODFragment& LODFragment = Context.GetConstChunkFragment<FMassSimulationLODFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FMassDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            const FMassForceFragment& Force = ForceList[EntityIndex];

            // Skip if LOD is too low for detailed simulation
            if (LODFragment.LOD > EMassLOD::Medium)
            {
                continue;
            }

            // Apply forces to velocity
            FVector DesiredVelocity = Force.Value;
            
            // Behavior-specific movement modifications
            switch (Behavior.CurrentState)
            {
                case EDinosaurBehaviorState::Grazing:
                    DesiredVelocity *= 0.3f; // Slow movement while grazing
                    break;
                    
                case EDinosaurBehaviorState::Fleeing:
                    DesiredVelocity *= 2.0f; // Panic speed
                    break;
                    
                case EDinosaurBehaviorState::Hunting:
                    DesiredVelocity *= 1.5f; // Predator speed
                    break;
                    
                case EDinosaurBehaviorState::Resting:
                    DesiredVelocity = FVector::ZeroVector; // No movement
                    break;
                    
                default:
                    break;
            }

            // Apply species-specific speed limits
            const float MaxSpeed = Species.MaxSpeed * Species.Size;
            if (DesiredVelocity.SizeSquared() > MaxSpeed * MaxSpeed)
            {
                DesiredVelocity = DesiredVelocity.GetSafeNormal() * MaxSpeed;
            }

            // Smooth velocity changes for natural movement
            const float AccelerationRate = Species.bIsFlying ? 1000.0f : 500.0f;
            Velocity.Value = FMath::VInterpTo(Velocity.Value, DesiredVelocity, DeltaTime, AccelerationRate);

            // Update behavior timer
            Behavior.StateTimer += DeltaTime;
        }
    });
}

//----------------------------------------------------------------------//
// UMassDinosaurHerdProcessor
//----------------------------------------------------------------------//

UMassDinosaurHerdProcessor::UMassDinosaurHerdProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMassDinosaurHerdProcessor::ConfigureQueries()
{
    HerdQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    HerdQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
    HerdQuery.AddRequirement<FMassDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    HerdQuery.AddRequirement<FMassDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    HerdQuery.AddRequirement<FMassDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
    HerdQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassDinosaurHerdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    HerdQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
        const TConstArrayView<FMassDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FMassDinosaurSpeciesFragment>();
        const TArrayView<FMassDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FMassDinosaurBehaviorFragment>();
        const TArrayView<FMassDinosaurHerdFragment> HerdList = Context.GetMutableFragmentView<FMassDinosaurHerdFragment>();
        const TArrayView<FMassForceFragment> ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
        const FMassSimulationLODFragment& LODFragment = Context.GetConstChunkFragment<FMassSimulationLODFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            const FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FMassDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FMassDinosaurHerdFragment& Herd = HerdList[EntityIndex];
            FMassForceFragment& Force = ForceList[EntityIndex];

            // Skip if LOD is too low or if this is a solitary species
            if (LODFragment.LOD > EMassLOD::High || Behavior.Role == EDinosaurRole::Solitary)
            {
                continue;
            }

            // Only process herbivores for herd behavior
            if (Species.bIsCarnivore)
            {
                continue;
            }

            FVector HerdForce = FVector::ZeroVector;

            // Calculate herd center and update herd data
            if (Behavior.Role == EDinosaurRole::HerdLeader)
            {
                Herd.HerdCenter = Transform.GetTransform().GetLocation();
                // Leader sets migration targets and general herd direction
                if (!Herd.bIsMigrating && FVector::Dist(Transform.GetTransform().GetLocation(), Herd.MigrationTarget) < 1000.0f)
                {
                    // Find new migration target (simplified - would use proper resource finding)
                    Herd.MigrationTarget = Transform.GetTransform().GetLocation() + 
                        FVector(FMath::RandRange(-10000.0f, 10000.0f), FMath::RandRange(-10000.0f, 10000.0f), 0.0f);
                    Herd.bIsMigrating = true;
                }
            }
            else
            {
                // Herd members follow leader and maintain formation
                
                // Cohesion: move towards herd center
                FVector CohesionForce = (Herd.HerdCenter - Transform.GetTransform().GetLocation()).GetSafeNormal();
                float DistanceToCenter = FVector::Dist(Transform.GetTransform().GetLocation(), Herd.HerdCenter);
                if (DistanceToCenter > CohesionRadius)
                {
                    HerdForce += CohesionForce * 300.0f; // Strong pull back to herd
                }

                // Separation: avoid crowding
                // (Simplified - would query nearby entities using spatial hash)
                
                // Alignment: match herd movement
                if (Herd.HerdLeader.IsValid())
                {
                    // Get leader's velocity (simplified access)
                    HerdForce += Velocity.Value.GetSafeNormal() * 100.0f;
                }
            }

            // Apply herd forces
            Force.Value += HerdForce;
        }
    });
}

//----------------------------------------------------------------------//
// UMassDinosaurPredatorProcessor
//----------------------------------------------------------------------//

UMassDinosaurPredatorProcessor::UMassDinosaurPredatorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UMassDinosaurPredatorProcessor::ConfigureQueries()
{
    PredatorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PredatorQuery.AddRequirement<FMassDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    PredatorQuery.AddRequirement<FMassDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddRequirement<FMassDinosaurSensesFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
    PredatorQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);

    PreyQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FMassDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    PreyQuery.AddRequirement<FMassDinosaurBehaviorFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassDinosaurPredatorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    PredatorQuery.ForEachEntityChunk(EntityManager, Context, [this, &EntityManager](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FMassDinosaurSpeciesFragment>();
        const TArrayView<FMassDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FMassDinosaurBehaviorFragment>();
        const TArrayView<FMassDinosaurSensesFragment> SensesList = Context.GetMutableFragmentView<FMassDinosaurSensesFragment>();
        const TArrayView<FMassForceFragment> ForceList = Context.GetMutableFragmentView<FMassForceFragment>();
        const FMassSimulationLODFragment& LODFragment = Context.GetConstChunkFragment<FMassSimulationLODFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            const FMassDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FMassDinosaurSensesFragment& Senses = SensesList[EntityIndex];
            FMassForceFragment& Force = ForceList[EntityIndex];

            // Only process carnivores
            if (!Species.bIsCarnivore || LODFragment.LOD > EMassLOD::Medium)
            {
                continue;
            }

            FVector HuntingForce = FVector::ZeroVector;

            // Hunting state machine
            switch (Behavior.CurrentState)
            {
                case EDinosaurBehaviorState::Patrolling:
                {
                    // Look for prey within hunting radius
                    // (Simplified - would use spatial queries)
                    if (Senses.NearbyPrey.Num() > 0)
                    {
                        Behavior.CurrentState = EDinosaurBehaviorState::Hunting;
                        Behavior.StateTimer = 0.0f;
                    }
                    else
                    {
                        // Random patrol movement
                        if (Behavior.StateTimer > Behavior.StateDuration)
                        {
                            Behavior.TargetLocation = Transform.GetTransform().GetLocation() + 
                                FVector(FMath::RandRange(-3000.0f, 3000.0f), FMath::RandRange(-3000.0f, 3000.0f), 0.0f);
                            Behavior.StateTimer = 0.0f;
                            Behavior.StateDuration = FMath::RandRange(10.0f, 30.0f);
                        }
                        
                        HuntingForce = (Behavior.TargetLocation - Transform.GetTransform().GetLocation()).GetSafeNormal() * 200.0f;
                    }
                    break;
                }
                
                case EDinosaurBehaviorState::Hunting:
                {
                    // Chase nearest prey
                    if (Senses.NearbyPrey.Num() > 0)
                    {
                        // Simplified prey targeting - would use proper entity queries
                        FVector PreyDirection = (Behavior.TargetLocation - Transform.GetTransform().GetLocation()).GetSafeNormal();
                        HuntingForce = PreyDirection * 500.0f;
                        
                        // Check if close enough to attack
                        float DistanceToPrey = FVector::Dist(Transform.GetTransform().GetLocation(), Behavior.TargetLocation);
                        if (DistanceToPrey < AmbushDistance)
                        {
                            // Attack logic would go here
                            Behavior.CurrentState = EDinosaurBehaviorState::Patrolling;
                            Behavior.StateTimer = 0.0f;
                        }
                    }
                    else
                    {
                        // Lost prey, return to patrolling
                        Behavior.CurrentState = EDinosaurBehaviorState::Patrolling;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                }
                
                default:
                    break;
            }

            // Apply hunting forces
            Force.Value += HuntingForce;
        }
    });
}

//----------------------------------------------------------------------//
// UMassDinosaurEcosystemProcessor
//----------------------------------------------------------------------//

UMassDinosaurEcosystemProcessor::UMassDinosaurEcosystemProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UMassDinosaurEcosystemProcessor::ConfigureQueries()
{
    EcosystemQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EcosystemQuery.AddRequirement<FMassDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    EcosystemQuery.AddRequirement<FMassDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EcosystemQuery.AddRequirement<FMassDinosaurMemoryFragment>(EMassFragmentAccess::ReadWrite);
    EcosystemQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UMassDinosaurEcosystemProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EcosystemQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FMassDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FMassDinosaurSpeciesFragment>();
        const TArrayView<FMassDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FMassDinosaurBehaviorFragment>();
        const TArrayView<FMassDinosaurMemoryFragment> MemoryList = Context.GetMutableFragmentView<FMassDinosaurMemoryFragment>();
        const FMassSimulationLODFragment& LODFragment = Context.GetConstChunkFragment<FMassSimulationLODFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            const FMassDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FMassDinosaurMemoryFragment& Memory = MemoryList[EntityIndex];

            // Skip if LOD is too low for ecosystem simulation
            if (LODFragment.LOD > EMassLOD::Low)
            {
                continue;
            }

            // Update basic needs over time
            Behavior.Hunger += DeltaTime * 0.01f; // Hunger increases slowly
            Behavior.Thirst += DeltaTime * 0.02f; // Thirst increases faster
            Behavior.Energy -= DeltaTime * 0.005f; // Energy decreases slowly

            // Clamp values
            Behavior.Hunger = FMath::Clamp(Behavior.Hunger, 0.0f, 1.0f);
            Behavior.Thirst = FMath::Clamp(Behavior.Thirst, 0.0f, 1.0f);
            Behavior.Energy = FMath::Clamp(Behavior.Energy, 0.0f, 1.0f);

            // Basic need-driven behavior changes
            if (Behavior.Thirst > 0.8f && Behavior.CurrentState != EDinosaurBehaviorState::Drinking)
            {
                // Find water source from memory
                if (Memory.KnownWaterSources.Num() > 0)
                {
                    Behavior.TargetLocation = Memory.KnownWaterSources[0];
                    Behavior.CurrentState = EDinosaurBehaviorState::Moving;
                }
            }
            else if (Behavior.Hunger > 0.8f && Behavior.CurrentState != EDinosaurBehaviorState::Grazing)
            {
                // Find food source
                if (!Species.bIsCarnivore)
                {
                    Behavior.CurrentState = EDinosaurBehaviorState::Grazing;
                    Behavior.StateDuration = FMath::RandRange(20.0f, 60.0f);
                }
            }
            else if (Behavior.Energy < 0.3f)
            {
                // Need rest
                Behavior.CurrentState = EDinosaurBehaviorState::Resting;
                Behavior.StateDuration = FMath::RandRange(30.0f, 120.0f);
            }

            // Territory management
            float DistanceFromHome = FVector::Dist(Transform.GetTransform().GetLocation(), Memory.HomeTerritory);
            if (DistanceFromHome > Memory.TerritorialRadius)
            {
                // Too far from home territory, head back
                if (Behavior.CurrentState == EDinosaurBehaviorState::Grazing || 
                    Behavior.CurrentState == EDinosaurBehaviorState::Moving)
                {
                    Behavior.TargetLocation = Memory.HomeTerritory;
                    Behavior.CurrentState = EDinosaurBehaviorState::Moving;
                }
            }
        }
    });
}
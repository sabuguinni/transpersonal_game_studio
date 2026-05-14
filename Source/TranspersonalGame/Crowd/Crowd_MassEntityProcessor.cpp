#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassLODFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"

// UCrowd_MovementProcessor Implementation
UCrowd_MovementProcessor::UCrowd_MovementProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Tasks);
}

void UCrowd_MovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_FlockingFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();
        const TArrayView<FCrowd_FlockingFragment> FlockingList = Context.GetMutableFragmentView<FCrowd_FlockingFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_AgentFragment& Agent = AgentList[EntityIndex];
            FCrowd_FlockingFragment& Flocking = FlockingList[EntityIndex];

            // Calculate flocking forces
            CalculateFlockingForces(Context, Agent, Flocking, Transform);

            // Combine forces
            FVector TotalForce = Flocking.SeparationForce * Flocking.SeparationWeight +
                               Flocking.CohesionForce * Flocking.CohesionWeight +
                               Flocking.AlignmentForce * Flocking.AlignmentWeight;

            // Add goal-seeking behavior
            if (!Agent.TargetLocation.IsZero())
            {
                FVector ToTarget = (Agent.TargetLocation - Transform.GetTransform().GetLocation()).GetSafeNormal();
                TotalForce += ToTarget * 50.0f; // Goal weight
            }

            // Apply forces to velocity
            Velocity.Value += TotalForce * DeltaTime;
            
            // Limit speed
            if (Velocity.Value.Size() > Agent.MovementSpeed)
            {
                Velocity.Value = Velocity.Value.GetSafeNormal() * Agent.MovementSpeed;
            }

            // Update position
            FVector NewLocation = Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime;
            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Update rotation to face movement direction
            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }
        }
    });
}

void UCrowd_MovementProcessor::CalculateFlockingForces(const FMassExecutionContext& Context, 
                                                      FCrowd_AgentFragment& Agent, 
                                                      FCrowd_FlockingFragment& Flocking,
                                                      const FTransformFragment& Transform)
{
    FVector Position = Transform.GetTransform().GetLocation();
    
    Flocking.SeparationForce = CalculateSeparation(Position, Context, NeighborRadius);
    Flocking.CohesionForce = CalculateCohesion(Position, Context, NeighborRadius);
    Flocking.AlignmentForce = CalculateAlignment(FVector::ZeroVector, Context, NeighborRadius);
}

FVector UCrowd_MovementProcessor::CalculateSeparation(const FVector& Position, const FMassExecutionContext& Context, float Radius)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;

    // Simple separation - move away from nearby agents
    // In a full implementation, this would query nearby entities
    return SeparationForce;
}

FVector UCrowd_MovementProcessor::CalculateCohesion(const FVector& Position, const FMassExecutionContext& Context, float Radius)
{
    FVector CohesionForce = FVector::ZeroVector;
    
    // Simple cohesion - move toward average position of nearby agents
    // In a full implementation, this would calculate center of mass
    return CohesionForce;
}

FVector UCrowd_MovementProcessor::CalculateAlignment(const FVector& Velocity, const FMassExecutionContext& Context, float Radius)
{
    FVector AlignmentForce = FVector::ZeroVector;
    
    // Simple alignment - match velocity of nearby agents
    // In a full implementation, this would average nearby velocities
    return AlignmentForce;
}

// UCrowd_LODProcessor Implementation
UCrowd_LODProcessor::UCrowd_LODProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
}

void UCrowd_LODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FMassLODFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_LODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get player location for distance calculations
    UWorld* World = Context.GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassLODFragment> LODList = Context.GetMutableFragmentView<FMassLODFragment>();
        const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetFragmentView<FCrowd_AgentFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassLODFragment& LODFragment = LODList[EntityIndex];
            const FCrowd_AgentFragment& Agent = AgentList[EntityIndex];

            float Distance = FVector::Dist(Transform.GetTransform().GetLocation(), PlayerLocation);
            UpdateLODLevel(LODFragment, Distance);
        }
    });
}

void UCrowd_LODProcessor::UpdateLODLevel(FMassLODFragment& LODFragment, float DistanceToPlayer)
{
    if (DistanceToPlayer <= HighDetailDistance)
    {
        LODFragment.LOD = EMassLOD::High;
    }
    else if (DistanceToPlayer <= MediumDetailDistance)
    {
        LODFragment.LOD = EMassLOD::Medium;
    }
    else if (DistanceToPlayer <= LowDetailDistance)
    {
        LODFragment.LOD = EMassLOD::Low;
    }
    else
    {
        LODFragment.LOD = EMassLOD::Off;
    }
}

// UCrowd_BehaviorProcessor Implementation
UCrowd_BehaviorProcessor::UCrowd_BehaviorProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UCrowd_BehaviorProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddChunkRequirement<FMassSimulationVariableTickChunkFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_BehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_AgentFragment& Agent = AgentList[EntityIndex];

            UpdateAgentBehavior(Agent, Transform, DeltaTime);
            
            if (Agent.bIsInGroup)
            {
                HandleGroupBehavior(Agent, Context);
            }
        }
    });
}

void UCrowd_BehaviorProcessor::UpdateAgentBehavior(FCrowd_AgentFragment& Agent, const FTransformFragment& Transform, float DeltaTime)
{
    FVector Position = Transform.GetTransform().GetLocation();

    // Update stress level based on environment
    Agent.StressLevel = FMath::Max(0.0f, Agent.StressLevel - DeltaTime * 0.1f);

    // Assign new targets based on agent type
    if (Agent.TargetLocation.IsZero() || FVector::Dist(Position, Agent.TargetLocation) < 100.0f)
    {
        switch (Agent.AgentType)
        {
            case 0: // Villager
                Agent.TargetLocation = FindNearestGatheringPoint(Position);
                break;
            case 1: // Hunter
                Agent.TargetLocation = Position + FMath::VRand() * 1000.0f; // Random patrol
                break;
            case 2: // Gatherer
                Agent.TargetLocation = FindNearestGatheringPoint(Position);
                break;
            case 3: // Child
                Agent.TargetLocation = FindNearestShelter(Position);
                break;
        }
    }
}

void UCrowd_BehaviorProcessor::HandleGroupBehavior(FCrowd_AgentFragment& Agent, const FMassExecutionContext& Context)
{
    // Simple group behavior - stay close to group members
    // In a full implementation, this would find other agents with the same GroupID
}

FVector UCrowd_BehaviorProcessor::FindNearestGatheringPoint(const FVector& Position)
{
    // Simple implementation - return a point near water or food sources
    // In a full implementation, this would query the world for resource locations
    return Position + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
}

FVector UCrowd_BehaviorProcessor::FindNearestShelter(const FVector& Position)
{
    // Simple implementation - return a point near caves or settlements
    // In a full implementation, this would query the world for shelter locations
    return Position + FVector(FMath::RandRange(-200.0f, 200.0f), FMath::RandRange(-200.0f, 200.0f), 0.0f);
}
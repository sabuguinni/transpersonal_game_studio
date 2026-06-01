#include "Crowd_MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"

UCrowd_MassProcessor::UCrowd_MassProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassProcessor::ConfigureQueries()
{
    // Configure movement query
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddTagRequirement<FCrowd_MovingTag>(EMassFragmentPresence::All);

    // Configure pathfinding query
    PathfindingQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PathfindingQuery.AddRequirement<FCrowd_PathfindingFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddTagRequirement<FCrowd_PathfindingTag>(EMassFragmentPresence::All);

    // Configure behavior query
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddTagRequirement<FCrowd_BehaviorTag>(EMassFragmentPresence::All);
}

void UCrowd_MassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Update timers
    LastPathfindingUpdate += DeltaTime;
    LastBehaviorUpdate += DeltaTime;

    // Process movement every frame
    ProcessMovement(EntityManager, Context);

    // Process pathfinding at intervals
    if (LastPathfindingUpdate >= PathfindingUpdateInterval)
    {
        ProcessPathfinding(EntityManager, Context);
        LastPathfindingUpdate = 0.0f;
    }

    // Process behavior at intervals
    if (LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        ProcessBehavior(EntityManager, Context);
        LastBehaviorUpdate = 0.0f;
    }
}

void UCrowd_MassProcessor::ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();

    MovementQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FTransformFragment& Transform = TransformList[EntityIndex];

            // Apply velocity to transform
            FVector NewLocation = Transform.GetTransform().GetLocation() + (Velocity.Value * DeltaTime);
            
            // Keep entities within reasonable bounds
            NewLocation.X = FMath::Clamp(NewLocation.X, -MaxCrowdDistance, MaxCrowdDistance);
            NewLocation.Y = FMath::Clamp(NewLocation.Y, -MaxCrowdDistance, MaxCrowdDistance);
            NewLocation.Z = FMath::Max(NewLocation.Z, 0.0f);

            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Apply some damping to velocity
            Velocity.Value *= 0.98f;
        }
    });
}

void UCrowd_MassProcessor::ProcessPathfinding(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    PathfindingQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_PathfindingFragment> PathfindingList = Context.GetMutableFragmentView<FCrowd_PathfindingFragment>();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_PathfindingFragment& Pathfinding = PathfindingList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            
            // Simple pathfinding - move towards target
            if (Pathfinding.HasTarget)
            {
                FVector Direction = (Pathfinding.TargetLocation - CurrentLocation).GetSafeNormal();
                float Distance = FVector::Dist(CurrentLocation, Pathfinding.TargetLocation);

                if (Distance > Pathfinding.AcceptanceRadius)
                {
                    Velocity.Value = Direction * MovementSpeed;
                }
                else
                {
                    // Reached target
                    Pathfinding.HasTarget = false;
                    Velocity.Value = FVector::ZeroVector;
                }
            }
            else
            {
                // No target - wander randomly
                if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance to change direction
                {
                    FVector RandomDirection = FMath::VRand();
                    RandomDirection.Z = 0.0f; // Keep movement horizontal
                    RandomDirection.Normalize();
                    
                    Pathfinding.TargetLocation = CurrentLocation + (RandomDirection * FMath::RandRange(500.0f, 2000.0f));
                    Pathfinding.HasTarget = true;
                }
            }
        }
    });
}

void UCrowd_MassProcessor::ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];
            const FTransformFragment& Transform = TransformList[EntityIndex];

            // Update behavior timer
            Behavior.StateTimer += BehaviorUpdateInterval;

            // Simple state machine
            switch (Behavior.CurrentState)
            {
                case ECrowd_BehaviorState::Idle:
                    if (Behavior.StateTimer > 5.0f) // Idle for 5 seconds
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Walking;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;

                case ECrowd_BehaviorState::Walking:
                    if (Behavior.StateTimer > 10.0f) // Walk for 10 seconds
                    {
                        Behavior.CurrentState = FMath::RandBool() ? ECrowd_BehaviorState::Idle : ECrowd_BehaviorState::Gathering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;

                case ECrowd_BehaviorState::Gathering:
                    if (Behavior.StateTimer > 8.0f) // Gather for 8 seconds
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Walking;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;

                case ECrowd_BehaviorState::Fleeing:
                    if (Behavior.StateTimer > 3.0f) // Flee for 3 seconds
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Idle;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
            }
        }
    });
}
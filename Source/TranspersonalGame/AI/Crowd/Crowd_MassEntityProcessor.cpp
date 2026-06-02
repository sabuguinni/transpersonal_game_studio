#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassSignalSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Movement Processor Implementation
UCrowd_MovementProcessor::UCrowd_MovementProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UCrowd_MovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TConstArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            const FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

            if (Behavior.CurrentState == ECrowd_BehaviorState::Moving && !Behavior.TargetLocation.IsZero())
            {
                FVector CurrentLocation = Transform.GetTransform().GetLocation();
                FVector Direction = (Behavior.TargetLocation - CurrentLocation).GetSafeNormal();
                
                // Apply steering force
                FVector DesiredVelocity = Direction * Movement.MaxSpeed;
                FVector SteeringForce = (DesiredVelocity - Movement.Velocity) * SteeringForce;
                
                Movement.Velocity += SteeringForce * DeltaTime;
                Movement.Velocity = Movement.Velocity.GetClampedToMaxSize(Movement.MaxSpeed);
                
                // Update position
                FVector NewLocation = CurrentLocation + Movement.Velocity * DeltaTime;
                Transform.GetMutableTransform().SetLocation(NewLocation);
                
                Movement.bIsMoving = !Movement.Velocity.IsNearlyZero();
                
                // Update rotation to face movement direction
                if (!Movement.Velocity.IsNearlyZero())
                {
                    FRotator NewRotation = Movement.Velocity.Rotation();
                    Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
                }
            }
            else
            {
                // Gradually stop movement
                Movement.Velocity = FMath::VInterpTo(Movement.Velocity, FVector::ZeroVector, DeltaTime, 5.0f);
                Movement.bIsMoving = !Movement.Velocity.IsNearlyZero();
            }
        }
    });
}

// Behavior Processor Implementation
UCrowd_BehaviorProcessor::UCrowd_BehaviorProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UCrowd_BehaviorProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_BehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TConstArrayView<FCrowd_LODFragment> LODList = Context.GetFragmentView<FCrowd_LODFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];
            const FCrowd_LODFragment& LOD = LODList[EntityIndex];

            // Skip behavior updates for distant entities
            if (LOD.CurrentLOD == ECrowd_LODLevel::Off)
            {
                continue;
            }

            UpdateBehaviorState(Behavior, Transform, DeltaTime);
        }
    });
}

void UCrowd_BehaviorProcessor::UpdateBehaviorState(FCrowd_BehaviorFragment& BehaviorFragment, const FTransformFragment& Transform, float DeltaTime)
{
    BehaviorFragment.StateTimer += DeltaTime;

    switch (BehaviorFragment.CurrentState)
    {
        case ECrowd_BehaviorState::Idle:
            if (BehaviorFragment.StateTimer >= BehaviorFragment.MaxStateTime)
            {
                BehaviorFragment.CurrentState = ECrowd_BehaviorState::Moving;
                BehaviorFragment.TargetLocation = GetWanderTarget(Transform.GetTransform().GetLocation());
                BehaviorFragment.StateTimer = 0.0f;
                BehaviorFragment.MaxStateTime = FMath::RandRange(3.0f, 8.0f);
            }
            break;

        case ECrowd_BehaviorState::Moving:
            {
                FVector CurrentLocation = Transform.GetTransform().GetLocation();
                float DistanceToTarget = FVector::Dist(CurrentLocation, BehaviorFragment.TargetLocation);
                
                if (DistanceToTarget < 100.0f || BehaviorFragment.StateTimer >= BehaviorFragment.MaxStateTime)
                {
                    BehaviorFragment.CurrentState = ECrowd_BehaviorState::Idle;
                    BehaviorFragment.TargetLocation = FVector::ZeroVector;
                    BehaviorFragment.StateTimer = 0.0f;
                    BehaviorFragment.MaxStateTime = FMath::RandRange(2.0f, 6.0f);
                }
            }
            break;

        case ECrowd_BehaviorState::Fleeing:
            if (BehaviorFragment.StateTimer >= 5.0f)
            {
                BehaviorFragment.CurrentState = ECrowd_BehaviorState::Idle;
                BehaviorFragment.StateTimer = 0.0f;
                BehaviorFragment.MaxStateTime = FMath::RandRange(3.0f, 7.0f);
            }
            break;
    }
}

FVector UCrowd_BehaviorProcessor::GetWanderTarget(const FVector& CurrentLocation)
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float WanderDistance = FMath::RandRange(200.0f, WanderRadius);
    return CurrentLocation + (RandomDirection * WanderDistance);
}

// LOD Processor Implementation
UCrowd_LODProcessor::UCrowd_LODProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
}

void UCrowd_LODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_LODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    FVector PlayerLocation = GetPlayerLocation();
    
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();

        const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_LODFragment& LOD = LODList[EntityIndex];

            // Update LOD only at intervals
            if (CurrentTime - LOD.LastUpdateTime < LODUpdateInterval)
            {
                continue;
            }

            LOD.LastUpdateTime = CurrentTime;
            
            FVector EntityLocation = Transform.GetTransform().GetLocation();
            LOD.DistanceToPlayer = FVector::Dist(EntityLocation, PlayerLocation);
            
            ECrowd_LODLevel NewLOD = CalculateLODLevel(LOD.DistanceToPlayer);
            
            if (NewLOD != LOD.CurrentLOD)
            {
                LOD.CurrentLOD = NewLOD;
                LOD.bIsVisible = (NewLOD != ECrowd_LODLevel::Off);
            }
        }
    });
}

ECrowd_LODLevel UCrowd_LODProcessor::CalculateLODLevel(float Distance) const
{
    if (Distance <= HighLODDistance)
    {
        return ECrowd_LODLevel::High;
    }
    else if (Distance <= MediumLODDistance)
    {
        return ECrowd_LODLevel::Medium;
    }
    else if (Distance <= LowLODDistance)
    {
        return ECrowd_LODLevel::Low;
    }
    else
    {
        return ECrowd_LODLevel::Off;
    }
}

FVector UCrowd_LODProcessor::GetPlayerLocation() const
{
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            return PlayerPawn->GetActorLocation();
        }
    }
    return FVector::ZeroVector;
}
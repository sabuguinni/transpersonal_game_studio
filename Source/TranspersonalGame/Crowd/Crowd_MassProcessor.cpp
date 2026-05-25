#include "Crowd_MassProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MovementProcessor::UCrowd_MovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
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

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector DesiredVelocity = FVector::ZeroVector;

            // Behavior-based movement
            switch (Behavior.CurrentState)
            {
                case ECrowd_BehaviorState::Wandering:
                {
                    if (!Movement.bHasTarget || FVector::Dist(CurrentLocation, Movement.TargetLocation) < 100.0f)
                    {
                        // Generate new random target within wander radius
                        FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
                        Movement.TargetLocation = CurrentLocation + (RandomDirection * WanderRadius);
                        Movement.bHasTarget = true;
                    }
                    DesiredVelocity = (Movement.TargetLocation - CurrentLocation).GetSafeNormal() * Movement.MaxSpeed * 0.5f;
                    break;
                }
                case ECrowd_BehaviorState::Fleeing:
                {
                    if (Behavior.ThreatActor)
                    {
                        FVector ThreatLocation = Behavior.ThreatActor->GetActorLocation();
                        FVector FleeDirection = (CurrentLocation - ThreatLocation).GetSafeNormal();
                        DesiredVelocity = FleeDirection * Movement.MaxSpeed;
                    }
                    break;
                }
                case ECrowd_BehaviorState::Gathering:
                {
                    // Move towards group center (simplified)
                    if (Movement.bHasTarget)
                    {
                        DesiredVelocity = (Movement.TargetLocation - CurrentLocation).GetSafeNormal() * Movement.MaxSpeed * 0.3f;
                    }
                    break;
                }
            }

            // Apply steering and update velocity
            FVector SteeringForce = DesiredVelocity - Movement.Velocity;
            Movement.Velocity += SteeringForce * DeltaTime * 2.0f;
            Movement.Velocity = Movement.Velocity.GetClampedToMaxSize(Movement.MaxSpeed);

            // Update position
            FVector NewLocation = CurrentLocation + (Movement.Velocity * DeltaTime);
            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Update rotation to face movement direction
            if (!Movement.Velocity.IsNearlyZero())
            {
                FRotator NewRotation = Movement.Velocity.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }
        }
    });
}

UCrowd_BehaviorProcessor::UCrowd_BehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UCrowd_BehaviorProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_BehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        UWorld* World = Context.GetWorld();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            Behavior.StateTimer += DeltaTime;

            // Threat detection
            bool bThreatDetected = false;
            if (World)
            {
                // Check for dangerous actors (dinosaurs, predators)
                TArray<AActor*> NearbyActors;
                // Simplified threat detection - in real implementation would use spatial queries
                for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
                {
                    AActor* Actor = *ActorItr;
                    if (Actor && Actor->GetName().Contains(TEXT("TRex")) || Actor->GetName().Contains(TEXT("Raptor")))
                    {
                        float Distance = FVector::Dist(CurrentLocation, Actor->GetActorLocation());
                        if (Distance < ThreatDetectionRadius)
                        {
                            Behavior.ThreatActor = Actor;
                            bThreatDetected = true;
                            break;
                        }
                    }
                }
            }

            // State transitions
            switch (Behavior.CurrentState)
            {
                case ECrowd_BehaviorState::Wandering:
                {
                    if (bThreatDetected)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Fleeing;
                        Behavior.StateTimer = 0.0f;
                    }
                    else if (Behavior.StateTimer > 10.0f)
                    {
                        // Occasionally gather with others
                        if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
                        {
                            Behavior.CurrentState = ECrowd_BehaviorState::Gathering;
                            Behavior.StateTimer = 0.0f;
                        }
                    }
                    break;
                }
                case ECrowd_BehaviorState::Fleeing:
                {
                    if (!bThreatDetected || Behavior.StateTimer > 15.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                        Behavior.StateTimer = 0.0f;
                        Behavior.ThreatActor = nullptr;
                    }
                    break;
                }
                case ECrowd_BehaviorState::Gathering:
                {
                    if (bThreatDetected)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Fleeing;
                        Behavior.StateTimer = 0.0f;
                    }
                    else if (Behavior.StateTimer > 8.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                }
            }
        }
    });
}
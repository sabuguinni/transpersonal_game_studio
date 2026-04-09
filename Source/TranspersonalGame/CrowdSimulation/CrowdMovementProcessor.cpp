// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CrowdMovementProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "ZoneGraphSubsystem.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogCrowdMovement, Log, All);

UCrowdMovementProcessor::UCrowdMovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);

    // Initialize default values
    WanderUpdateInterval = 2.0f;
    PerceptionUpdateInterval = 0.5f;
    MaxMovementSpeed = 600.0f;
    MinMovementSpeed = 50.0f;
    FlockingRadius = 200.0f;
    AvoidanceRadius = 150.0f;
    ObstacleCheckDistance = 300.0f;
    
    WanderWeight = 1.0f;
    FlockingWeight = 0.8f;
    AvoidanceWeight = 1.5f;
    FollowWeight = 2.0f;
    FleeWeight = 3.0f;
}

void UCrowdMovementProcessor::ConfigureQueries()
{
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowdMovementFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowdBehaviorFragment>(EMassFragmentAccess::ReadOnly);
    MovementQuery.AddRequirement<FCrowdPerceptionFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowdAvoidanceFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddOptionalRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddChunkRequirement<FMassVisualizationLODFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowdMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SCOPE_CYCLE_COUNTER(STAT_CrowdMovementProcessor);

    const float DeltaTime = Context.GetDeltaTimeSeconds();
    const float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    
    // Get required subsystems
    if (!ZoneGraphSubsystem)
    {
        ZoneGraphSubsystem = Context.GetWorld()->GetSubsystem<UZoneGraphSubsystem>();
    }
    
    if (!NavigationSystem)
    {
        NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Context.GetWorld());
    }

    ProcessedEntityCount = 0;
    const double StartTime = FPlatformTime::Seconds();

    // Process movement for all entities
    MovementQuery.ForEachEntityChunk(EntityManager, Context, 
        [this, DeltaTime, CurrentTime](FMassExecutionContext& Context)
        {
            const TConstArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
            const TArrayView<FCrowdMovementFragment> MovementList = Context.GetMutableFragmentView<FCrowdMovementFragment>();
            const TConstArrayView<FCrowdBehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowdBehaviorFragment>();
            const TArrayView<FCrowdPerceptionFragment> PerceptionList = Context.GetMutableFragmentView<FCrowdPerceptionFragment>();
            const TArrayView<FCrowdAvoidanceFragment> AvoidanceList = Context.GetMutableFragmentView<FCrowdAvoidanceFragment>();
            const TConstArrayView<FMassVisualizationLODFragment> LODList = Context.GetFragmentView<FMassVisualizationLODFragment>();

            const int32 NumEntities = Context.GetNumEntities();
            ProcessedEntityCount += NumEntities;

            for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
            {
                const FTransformFragment& Transform = TransformList[EntityIndex];
                FCrowdMovementFragment& Movement = MovementList[EntityIndex];
                const FCrowdBehaviorFragment& Behavior = BehaviorList[EntityIndex];
                FCrowdPerceptionFragment& Perception = PerceptionList[EntityIndex];
                FCrowdAvoidanceFragment& Avoidance = AvoidanceList[EntityIndex];
                const FMassVisualizationLODFragment& LOD = LODList[EntityIndex];

                // Skip processing for off-LOD entities
                if (LOD.LODLevel == EMassLOD::Off)
                {
                    continue;
                }

                // Update perception periodically
                if (CurrentTime - Perception.LastPerceptionUpdate >= Perception.PerceptionUpdateInterval)
                {
                    UpdatePerception(Perception, Transform, Context.GetEntityManagerChecked(), DeltaTime);
                    Perception.LastPerceptionUpdate = CurrentTime;
                }

                // Update movement state
                UpdateMovementState(Movement, Behavior, Perception, DeltaTime);

                // Calculate movement forces based on current state
                FVector TotalForce = FVector::ZeroVector;

                switch (Movement.MovementState)
                {
                case ECrowdMovementState::Wandering:
                    {
                        const FVector WanderForce = CalculateWanderForce(Transform, Movement, DeltaTime);
                        const FVector FlockingForce = CalculateFlockingForce(Transform, Movement, Perception);
                        const FVector AvoidanceForce = CalculateAvoidanceForce(Transform, Movement, Avoidance, Perception);
                        
                        TotalForce = (WanderForce * WanderWeight) + 
                                   (FlockingForce * FlockingWeight) + 
                                   (AvoidanceForce * AvoidanceWeight);
                    }
                    break;

                case ECrowdMovementState::Following:
                    {
                        const FVector FollowForce = CalculateFollowForce(Transform, Behavior);
                        const FVector AvoidanceForce = CalculateAvoidanceForce(Transform, Movement, Avoidance, Perception);
                        
                        TotalForce = (FollowForce * FollowWeight) + (AvoidanceForce * AvoidanceWeight);
                    }
                    break;

                case ECrowdMovementState::Fleeing:
                    {
                        const FVector FleeForce = CalculateFleeForce(Transform, Behavior);
                        const FVector AvoidanceForce = CalculateAvoidanceForce(Transform, Movement, Avoidance, Perception);
                        
                        TotalForce = (FleeForce * FleeWeight) + (AvoidanceForce * AvoidanceWeight * 0.5f);
                    }
                    break;

                case ECrowdMovementState::Investigating:
                    {
                        const FVector Direction = (Behavior.InterestPoint - Transform.GetTransform().GetLocation()).GetSafeNormal();
                        const FVector InvestigateForce = Direction * Movement.MaxSpeed;
                        const FVector AvoidanceForce = CalculateAvoidanceForce(Transform, Movement, Avoidance, Perception);
                        
                        TotalForce = InvestigateForce + (AvoidanceForce * AvoidanceWeight);
                    }
                    break;

                case ECrowdMovementState::Grazing:
                case ECrowdMovementState::Resting:
                    {
                        // Minimal movement, just avoidance
                        const FVector AvoidanceForce = CalculateAvoidanceForce(Transform, Movement, Avoidance, Perception);
                        TotalForce = AvoidanceForce * AvoidanceWeight * 0.3f;
                    }
                    break;

                default:
                    // Default to wandering behavior
                    TotalForce = CalculateWanderForce(Transform, Movement, DeltaTime);
                    break;
                }

                // Add obstacle avoidance
                const FVector ObstacleForce = CalculateObstacleAvoidance(Transform, Movement);
                TotalForce += ObstacleForce;

                // Apply force to velocity
                Movement.CurrentVelocity += TotalForce * DeltaTime;
                Movement.CurrentVelocity = ClampVelocity(Movement.CurrentVelocity, Movement.MaxSpeed);

                // Apply velocity to transform
                if (!Movement.CurrentVelocity.IsNearlyZero())
                {
                    const FVector NewLocation = Transform.GetTransform().GetLocation() + (Movement.CurrentVelocity * DeltaTime);
                    
                    // Validate the new location
                    if (IsValidNavLocation(NewLocation))
                    {
                        FTransform NewTransform = Transform.GetTransform();
                        NewTransform.SetLocation(NewLocation);
                        
                        // Update rotation to face movement direction
                        if (Movement.CurrentVelocity.SizeSquared() > 1.0f)
                        {
                            const FVector ForwardDirection = Movement.CurrentVelocity.GetSafeNormal();
                            const FRotator NewRotation = FRotationMatrix::MakeFromX(ForwardDirection).Rotator();
                            NewTransform.SetRotation(NewRotation.Quaternion());
                        }
                        
                        // Update the transform fragment
                        const_cast<FTransformFragment&>(Transform).SetTransform(NewTransform);
                    }
                    else
                    {
                        // If location is invalid, reduce velocity and try to steer away
                        Movement.CurrentVelocity *= 0.5f;
                    }
                }

                // Update state timer
                Movement.StateTimer += DeltaTime;
            }
        });

    // Track processing time
    const double EndTime = FPlatformTime::Seconds();
    LastProcessTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds

    UE_LOG(LogCrowdMovement, VeryVerbose, TEXT("Processed %d entities in %.2fms"), 
           ProcessedEntityCount, LastProcessTime);
}

FVector UCrowdMovementProcessor::CalculateWanderForce(const FTransform& Transform, FCrowdMovementFragment& Movement, float DeltaTime)
{
    const FVector CurrentLocation = Transform.GetLocation();
    
    // Update wander target periodically
    if (Movement.StateTimer >= WanderUpdateInterval)
    {
        // Generate new random wander target
        const float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
        const float RandomDistance = FMath::RandRange(Movement.WanderDistance * 0.5f, Movement.WanderDistance);
        
        const FVector RandomOffset = FVector(
            FMath::Cos(RandomAngle) * RandomDistance,
            FMath::Sin(RandomAngle) * RandomDistance,
            0.0f
        );
        
        Movement.WanderTarget = CurrentLocation + RandomOffset;
        Movement.StateTimer = 0.0f;
    }
    
    // Calculate force towards wander target
    const FVector ToTarget = Movement.WanderTarget - CurrentLocation;
    if (ToTarget.SizeSquared() > 100.0f) // 10 units squared
    {
        return ToTarget.GetSafeNormal() * Movement.MaxSpeed;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateFlockingForce(const FTransform& Transform, const FCrowdMovementFragment& Movement, 
                                                       const FCrowdPerceptionFragment& Perception)
{
    const FVector CurrentLocation = Transform.GetLocation();
    
    // Calculate flocking behaviors
    const FVector Separation = CalculateSeparation(Transform, Perception, AvoidanceRadius);
    const FVector Alignment = CalculateAlignment(Transform, Perception, FlockingRadius);
    const FVector Cohesion = CalculateCohesion(Transform, Perception, FlockingRadius);
    
    // Combine forces with different weights
    return (Separation * 1.5f) + (Alignment * 1.0f) + (Cohesion * 1.0f);
}

FVector UCrowdMovementProcessor::CalculateAvoidanceForce(const FTransform& Transform, const FCrowdMovementFragment& Movement,
                                                        const FCrowdAvoidanceFragment& Avoidance, const FCrowdPerceptionFragment& Perception)
{
    FVector TotalAvoidance = FVector::ZeroVector;
    const FVector CurrentLocation = Transform.GetLocation();
    
    // Avoid nearby agents
    for (const TWeakObjectPtr<AActor>& NearbyAgent : Perception.NearbyAgents)
    {
        if (NearbyAgent.IsValid())
        {
            const FVector ToAgent = NearbyAgent->GetActorLocation() - CurrentLocation;
            const float Distance = ToAgent.Size();
            
            if (Distance > 0.0f && Distance < Avoidance.AvoidanceRadius)
            {
                const FVector AvoidDirection = -ToAgent.GetSafeNormal();
                const float AvoidStrength = (Avoidance.AvoidanceRadius - Distance) / Avoidance.AvoidanceRadius;
                TotalAvoidance += AvoidDirection * AvoidStrength * Movement.MaxSpeed;
            }
        }
    }
    
    return TotalAvoidance;
}

FVector UCrowdMovementProcessor::CalculateFollowForce(const FTransform& Transform, const FCrowdBehaviorFragment& Behavior)
{
    if (Behavior.FollowTarget.IsValid())
    {
        const FVector CurrentLocation = Transform.GetLocation();
        const FVector TargetLocation = Behavior.FollowTarget->GetActorLocation();
        const FVector ToTarget = TargetLocation - CurrentLocation;
        
        // Follow at a reasonable distance
        const float FollowDistance = 200.0f;
        if (ToTarget.Size() > FollowDistance)
        {
            return ToTarget.GetSafeNormal() * MaxMovementSpeed;
        }
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateFleeForce(const FTransform& Transform, const FCrowdBehaviorFragment& Behavior)
{
    if (Behavior.FleeTarget.IsValid())
    {
        const FVector CurrentLocation = Transform.GetLocation();
        const FVector ThreatLocation = Behavior.FleeTarget->GetActorLocation();
        const FVector FromThreat = CurrentLocation - ThreatLocation;
        
        // Flee with maximum speed
        return FromThreat.GetSafeNormal() * MaxMovementSpeed;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateObstacleAvoidance(const FTransform& Transform, const FCrowdMovementFragment& Movement)
{
    if (!NavigationSystem || !Movement.bAvoidObstacles)
    {
        return FVector::ZeroVector;
    }
    
    const FVector CurrentLocation = Transform.GetLocation();
    const FVector ForwardDirection = Transform.GetRotation().GetForwardVector();
    const FVector CheckLocation = CurrentLocation + (ForwardDirection * ObstacleCheckDistance);
    
    // Simple obstacle check using navigation system
    FNavLocation NavResult;
    if (!NavigationSystem->ProjectPointToNavigation(CheckLocation, NavResult, FVector(100.0f)))
    {
        // No valid navigation point ahead, steer away
        const FVector RightDirection = Transform.GetRotation().GetRightVector();
        const float SteerDirection = FMath::RandBool() ? 1.0f : -1.0f;
        return RightDirection * SteerDirection * Movement.MaxSpeed;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateSeparation(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius)
{
    FVector SeparationForce = FVector::ZeroVector;
    const FVector CurrentLocation = Transform.GetLocation();
    int32 NeighborCount = 0;
    
    for (const TWeakObjectPtr<AActor>& NearbyAgent : Perception.NearbyAgents)
    {
        if (NearbyAgent.IsValid())
        {
            const FVector ToAgent = NearbyAgent->GetActorLocation() - CurrentLocation;
            const float Distance = ToAgent.Size();
            
            if (Distance > 0.0f && Distance < Radius)
            {
                const FVector AwayFromAgent = -ToAgent.GetSafeNormal();
                const float Weight = (Radius - Distance) / Radius; // Closer = stronger force
                SeparationForce += AwayFromAgent * Weight;
                NeighborCount++;
            }
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce = SeparationForce.GetSafeNormal() * MaxMovementSpeed;
    }
    
    return SeparationForce;
}

FVector UCrowdMovementProcessor::CalculateAlignment(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius)
{
    FVector AverageVelocity = FVector::ZeroVector;
    const FVector CurrentLocation = Transform.GetLocation();
    int32 NeighborCount = 0;
    
    for (const TWeakObjectPtr<AActor>& NearbyAgent : Perception.NearbyAgents)
    {
        if (NearbyAgent.IsValid())
        {
            const FVector ToAgent = NearbyAgent->GetActorLocation() - CurrentLocation;
            const float Distance = ToAgent.Size();
            
            if (Distance > 0.0f && Distance < Radius)
            {
                // Get agent's velocity (simplified - would need velocity component)
                const FVector AgentVelocity = NearbyAgent->GetVelocity();
                AverageVelocity += AgentVelocity;
                NeighborCount++;
            }
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        return AverageVelocity.GetSafeNormal() * MaxMovementSpeed;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateCohesion(const FTransform& Transform, const FCrowdPerceptionFragment& Perception, float Radius)
{
    FVector CenterOfMass = FVector::ZeroVector;
    const FVector CurrentLocation = Transform.GetLocation();
    int32 NeighborCount = 0;
    
    for (const TWeakObjectPtr<AActor>& NearbyAgent : Perception.NearbyAgents)
    {
        if (NearbyAgent.IsValid())
        {
            const FVector AgentLocation = NearbyAgent->GetActorLocation();
            const float Distance = FVector::Dist(CurrentLocation, AgentLocation);
            
            if (Distance > 0.0f && Distance < Radius)
            {
                CenterOfMass += AgentLocation;
                NeighborCount++;
            }
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        const FVector ToCenterOfMass = CenterOfMass - CurrentLocation;
        return ToCenterOfMass.GetSafeNormal() * MaxMovementSpeed;
    }
    
    return FVector::ZeroVector;
}

void UCrowdMovementProcessor::UpdateMovementState(FCrowdMovementFragment& Movement, const FCrowdBehaviorFragment& Behavior, 
                                                 const FCrowdPerceptionFragment& Perception, float DeltaTime)
{
    // Simple state machine for movement behaviors
    
    // Check for flee conditions
    if (Behavior.FleeTarget.IsValid() && Movement.MovementState != ECrowdMovementState::Fleeing)
    {
        Movement.MovementState = ECrowdMovementState::Fleeing;
        Movement.StateTimer = 0.0f;
        Movement.NextStateChangeTime = FMath::RandRange(3.0f, 8.0f);
        return;
    }
    
    // Check for follow conditions
    if (Behavior.FollowTarget.IsValid() && Movement.MovementState != ECrowdMovementState::Following)
    {
        Movement.MovementState = ECrowdMovementState::Following;
        Movement.StateTimer = 0.0f;
        Movement.NextStateChangeTime = FMath::RandRange(5.0f, 15.0f);
        return;
    }
    
    // Check for state change timer
    if (Movement.StateTimer >= Movement.NextStateChangeTime)
    {
        // Randomly change state based on behavior
        const float RandomValue = FMath::RandRange(0.0f, 1.0f);
        
        if (RandomValue < 0.6f)
        {
            Movement.MovementState = ECrowdMovementState::Wandering;
        }
        else if (RandomValue < 0.8f && Behavior.Energy > 0.5f)
        {
            Movement.MovementState = ECrowdMovementState::Socializing;
        }
        else if (RandomValue < 0.9f && Behavior.Hunger > 0.3f)
        {
            Movement.MovementState = ECrowdMovementState::Grazing;
        }
        else
        {
            Movement.MovementState = ECrowdMovementState::Resting;
        }
        
        Movement.StateTimer = 0.0f;
        Movement.NextStateChangeTime = FMath::RandRange(3.0f, 12.0f);
    }
}

void UCrowdMovementProcessor::UpdatePerception(FCrowdPerceptionFragment& Perception, const FTransform& Transform, 
                                              FMassEntityManager& EntityManager, float DeltaTime)
{
    // Clear previous perception data
    Perception.VisibleActors.Empty();
    Perception.NearbyAgents.Empty();
    Perception.NearestThreat = nullptr;
    Perception.NearestFood = nullptr;
    Perception.NearestAlly = nullptr;
    
    // This is a simplified perception system
    // In a full implementation, this would use spatial queries to find nearby entities
    // and perform line-of-sight checks for visibility
    
    const FVector CurrentLocation = Transform.GetLocation();
    
    // For now, we'll just simulate some basic perception
    // In a real implementation, this would query the spatial hash or use other spatial data structures
}

bool UCrowdMovementProcessor::IsValidNavLocation(const FVector& Location) const
{
    if (!NavigationSystem)
    {
        return true; // If no nav system, assume valid
    }
    
    FNavLocation NavResult;
    return NavigationSystem->ProjectPointToNavigation(Location, NavResult, FVector(50.0f));
}

FVector UCrowdMovementProcessor::ClampVelocity(const FVector& Velocity, float MaxSpeed) const
{
    if (Velocity.SizeSquared() > MaxSpeed * MaxSpeed)
    {
        return Velocity.GetSafeNormal() * MaxSpeed;
    }
    return Velocity;
}

FVector UCrowdMovementProcessor::SteerTowards(const FVector& CurrentVelocity, const FVector& DesiredDirection, float MaxForce) const
{
    const FVector DesiredVelocity = DesiredDirection.GetSafeNormal() * MaxMovementSpeed;
    const FVector SteeringForce = DesiredVelocity - CurrentVelocity;
    
    if (SteeringForce.SizeSquared() > MaxForce * MaxForce)
    {
        return SteeringForce.GetSafeNormal() * MaxForce;
    }
    
    return SteeringForce;
}
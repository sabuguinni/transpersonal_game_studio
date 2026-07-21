#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassLODFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassEntityProcessor::UCrowd_MassEntityProcessor()
{
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassEntityProcessor::ConfigureQueries()
{
    // Movement query - entities with transform and velocity
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddTagRequirement<FCrowd_MovementTag>(EMassFragmentPresence::All);
    
    // Behavior query - entities with behavior state
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddTagRequirement<FCrowd_BehaviorTag>(EMassFragmentPresence::All);
    
    // LOD query - all crowd entities for distance-based processing
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FMassViewerInfoFragment>(EMassFragmentAccess::ReadWrite);
    LODQuery.AddTagRequirement<FCrowd_EntityTag>(EMassFragmentPresence::All);
}

void UCrowd_MassEntityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Throttle updates for performance
    if (CurrentTime - LastUpdateTime < UpdateInterval)
    {
        return;
    }
    
    const float StartTime = FPlatformTime::Seconds();
    
    // Reset performance counters
    ProcessedEntities = 0;
    HighLODCount = 0;
    MediumLODCount = 0;
    LowLODCount = 0;
    
    // Process in order: LOD -> Behavior -> Movement
    ProcessLOD(EntityManager, Context);
    ProcessBehavior(EntityManager, Context);
    ProcessMovement(EntityManager, Context);
    
    // Update performance metrics
    LastProcessingTime = FPlatformTime::Seconds() - StartTime;
    LastUpdateTime = CurrentTime;
}

void UCrowd_MassEntityProcessor::ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    MovementQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            FTransformFragment& Transform = TransformList[i];
            FMassVelocityFragment& Velocity = VelocityList[i];
            
            // Apply movement with speed limiting
            FVector CurrentVelocity = Velocity.Value;
            if (CurrentVelocity.Size() > MaxSpeed)
            {
                CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
                Velocity.Value = CurrentVelocity;
            }
            
            // Update position
            const FVector NewLocation = Transform.GetTransform().GetLocation() + (CurrentVelocity * DeltaTime);
            Transform.GetMutableTransform().SetLocation(NewLocation);
            
            // Update rotation to face movement direction
            if (!CurrentVelocity.IsNearlyZero())
            {
                const FRotator TargetRotation = CurrentVelocity.Rotation();
                const FRotator CurrentRotation = Transform.GetTransform().GetRotation().Rotator();
                const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, TurnRate);
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }
            
            ProcessedEntities++;
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        
        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            const FTransformFragment& Transform = TransformList[i];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[i];
            
            const FVector EntityLocation = Transform.GetTransform().GetLocation();
            
            // Check if reached current waypoint
            if (Behavior.CurrentWaypointIndex < Behavior.WaypointPath.Num())
            {
                const FVector WaypointLocation = Behavior.WaypointPath[Behavior.CurrentWaypointIndex];
                const float DistanceToWaypoint = FVector::Dist(EntityLocation, WaypointLocation);
                
                if (DistanceToWaypoint <= WaypointReachDistance)
                {
                    // Move to next waypoint
                    Behavior.CurrentWaypointIndex++;
                    if (Behavior.CurrentWaypointIndex >= Behavior.WaypointPath.Num())
                    {
                        // Loop back to first waypoint
                        Behavior.CurrentWaypointIndex = 0;
                    }
                }
            }
            
            // Update behavior state based on current activity
            switch (Behavior.CurrentBehavior)
            {
                case ECrowd_BehaviorType::Wandering:
                    Behavior.StateTimer += Context.GetDeltaTimeSeconds();
                    if (Behavior.StateTimer > 5.0f) // Change behavior every 5 seconds
                    {
                        Behavior.CurrentBehavior = ECrowd_BehaviorType::MovingToWaypoint;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                    
                case ECrowd_BehaviorType::MovingToWaypoint:
                    // Continue moving until waypoint reached
                    break;
                    
                case ECrowd_BehaviorType::Idle:
                    Behavior.StateTimer += Context.GetDeltaTimeSeconds();
                    if (Behavior.StateTimer > 3.0f)
                    {
                        Behavior.CurrentBehavior = ECrowd_BehaviorType::Wandering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                    
                case ECrowd_BehaviorType::Fleeing:
                    // Emergency behavior - move away from danger
                    Behavior.StateTimer += Context.GetDeltaTimeSeconds();
                    if (Behavior.StateTimer > 10.0f)
                    {
                        Behavior.CurrentBehavior = ECrowd_BehaviorType::Wandering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
            }
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get player location for LOD calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* PlayerPawn = PC->GetPawn())
            {
                PlayerLocation = PlayerPawn->GetActorLocation();
            }
        }
    }
    
    LODQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassViewerInfoFragment> ViewerInfoList = Context.GetMutableFragmentView<FMassViewerInfoFragment>();
        
        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            const FTransformFragment& Transform = TransformList[i];
            FMassViewerInfoFragment& ViewerInfo = ViewerInfoList[i];
            
            const FVector EntityLocation = Transform.GetTransform().GetLocation();
            const ECrowd_LODLevel LODLevel = DetermineLODLevel(EntityLocation, PlayerLocation);
            
            // Update LOD counters
            switch (LODLevel)
            {
                case ECrowd_LODLevel::High:
                    HighLODCount++;
                    ViewerInfo.ClosestViewerDistanceSq = FVector::DistSquared(EntityLocation, PlayerLocation);
                    break;
                case ECrowd_LODLevel::Medium:
                    MediumLODCount++;
                    break;
                case ECrowd_LODLevel::Low:
                    LowLODCount++;
                    break;
            }
        }
    });
}

FVector UCrowd_MassEntityProcessor::CalculateFlockingForce(const FVector& EntityLocation, const TArray<FVector>& NearbyEntities)
{
    if (NearbyEntities.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate center of mass
    FVector CenterOfMass = FVector::ZeroVector;
    for (const FVector& Location : NearbyEntities)
    {
        CenterOfMass += Location;
    }
    CenterOfMass /= NearbyEntities.Num();
    
    // Move towards center of mass
    const FVector FlockingForce = (CenterOfMass - EntityLocation).GetSafeNormal() * FlockingStrength;
    return FlockingForce;
}

FVector UCrowd_MassEntityProcessor::CalculateAvoidanceForce(const FVector& EntityLocation, const TArray<FVector>& Obstacles)
{
    FVector AvoidanceForce = FVector::ZeroVector;
    
    for (const FVector& Obstacle : Obstacles)
    {
        const float Distance = FVector::Dist(EntityLocation, Obstacle);
        if (Distance < AvoidanceRadius && Distance > 0.0f)
        {
            const FVector AvoidDirection = (EntityLocation - Obstacle).GetSafeNormal();
            const float AvoidanceStrength = (AvoidanceRadius - Distance) / AvoidanceRadius;
            AvoidanceForce += AvoidDirection * AvoidanceStrength;
        }
    }
    
    return AvoidanceForce;
}

ECrowd_LODLevel UCrowd_MassEntityProcessor::DetermineLODLevel(const FVector& EntityLocation, const FVector& ViewerLocation)
{
    const float DistanceSquared = FVector::DistSquared(EntityLocation, ViewerLocation);
    
    if (DistanceSquared <= FMath::Square(HighLODDistance))
    {
        return ECrowd_LODLevel::High;
    }
    else if (DistanceSquared <= FMath::Square(MediumLODDistance))
    {
        return ECrowd_LODLevel::Medium;
    }
    else
    {
        return ECrowd_LODLevel::Low;
    }
}
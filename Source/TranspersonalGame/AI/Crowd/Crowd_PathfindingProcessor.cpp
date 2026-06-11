#include "Crowd_PathfindingProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntitySubsystem.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_PathfindingProcessor::UCrowd_PathfindingProcessor()
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UCrowd_PathfindingProcessor::ConfigureQueries()
{
    PathfindingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddRequirement<FCrowd_PathfindingFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddRequirement<FCrowd_AvoidanceFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_PathfindingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    PathfindingQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_PathfindingFragment> PathfindingList = Context.GetMutableFragmentView<FCrowd_PathfindingFragment>();
        const TArrayView<FCrowd_AvoidanceFragment> AvoidanceList = Context.GetMutableFragmentView<FCrowd_AvoidanceFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_PathfindingFragment& PathData = PathfindingList[EntityIndex];
            FCrowd_AvoidanceFragment& AvoidanceData = AvoidanceList[EntityIndex];

            const FVector CurrentLocation = Transform.GetTransform().GetLocation();

            // Update nearby entities for avoidance
            UpdateNearbyEntities(EntityManager, CurrentLocation, NeighborSearchRadius, AvoidanceData.NearbyEntities);

            // Check if we need to find a new path
            if (!PathData.bHasValidPath || PathData.PathPoints.Num() == 0)
            {
                // Generate random wander target if no specific target
                if (PathData.TargetLocation.IsZero())
                {
                    PathData.TargetLocation = GetRandomWanderTarget(CurrentLocation, 1000.0f);
                }

                // Find path to target
                TArray<FVector> NewPath;
                if (FindPathToTarget(CurrentLocation, PathData.TargetLocation, NewPath))
                {
                    PathData.PathPoints = NewPath;
                    PathData.CurrentPathIndex = 0;
                    PathData.bHasValidPath = true;
                }
            }

            // Calculate steering forces
            FVector SteeringForce = CalculateSteeringForce(PathData, CurrentLocation);
            
            // Calculate avoidance forces
            TArray<FVector> NearbyPositions;
            for (const FMassEntityHandle& NearbyEntity : AvoidanceData.NearbyEntities)
            {
                if (EntityManager.IsEntityValid(NearbyEntity))
                {
                    const FTransformFragment* NearbyTransform = EntityManager.GetFragmentDataPtr<FTransformFragment>(NearbyEntity);
                    if (NearbyTransform)
                    {
                        NearbyPositions.Add(NearbyTransform->GetTransform().GetLocation());
                    }
                }
            }

            FVector AvoidanceForce = CalculateAvoidanceForce(AvoidanceData, CurrentLocation, NearbyPositions);

            // Combine forces
            FVector TotalForce = SteeringForce + AvoidanceForce;
            TotalForce = TotalForce.GetClampedToMaxSize(PathData.MaxSpeed);

            // Update velocity
            PathData.CurrentVelocity = FMath::VInterpTo(PathData.CurrentVelocity, TotalForce, DeltaTime, 2.0f);
            Velocity.Value = PathData.CurrentVelocity;

            // Update transform
            FVector NewLocation = CurrentLocation + (PathData.CurrentVelocity * DeltaTime);
            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Check if reached current path point
            if (PathData.bHasValidPath && PathData.CurrentPathIndex < PathData.PathPoints.Num())
            {
                const FVector& CurrentTarget = PathData.PathPoints[PathData.CurrentPathIndex];
                float DistanceToTarget = FVector::Dist(CurrentLocation, CurrentTarget);

                if (DistanceToTarget < PathData.AcceptanceRadius)
                {
                    PathData.CurrentPathIndex++;
                    
                    // If reached end of path, generate new target
                    if (PathData.CurrentPathIndex >= PathData.PathPoints.Num())
                    {
                        PathData.bHasValidPath = false;
                        PathData.TargetLocation = GetRandomWanderTarget(CurrentLocation, 1500.0f);
                    }
                }
            }
        }
    });
}

bool UCrowd_PathfindingProcessor::FindPathToTarget(const FVector& StartLocation, const FVector& TargetLocation, TArray<FVector>& OutPath)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        // Fallback: direct line path
        OutPath.Empty();
        OutPath.Add(TargetLocation);
        return true;
    }

    FNavLocation StartNavLocation, TargetNavLocation;
    if (!NavSys->ProjectPointToNavigation(StartLocation, StartNavLocation) ||
        !NavSys->ProjectPointToNavigation(TargetLocation, TargetNavLocation))
    {
        // Fallback: direct line path
        OutPath.Empty();
        OutPath.Add(TargetLocation);
        return true;
    }

    FPathFindingQuery Query;
    Query.StartLocation = StartNavLocation.Location;
    Query.EndLocation = TargetNavLocation.Location;
    Query.NavData = NavSys->GetDefaultNavDataInstance();

    FPathFindingResult Result = NavSys->FindPathSync(Query);
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        OutPath.Empty();
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        for (const FNavPathPoint& Point : PathPoints)
        {
            OutPath.Add(Point.Location);
        }
        return true;
    }

    return false;
}

FVector UCrowd_PathfindingProcessor::CalculateSteeringForce(const FCrowd_PathfindingFragment& PathData, const FVector& CurrentLocation)
{
    if (!PathData.bHasValidPath || PathData.CurrentPathIndex >= PathData.PathPoints.Num())
    {
        return FVector::ZeroVector;
    }

    const FVector& TargetPoint = PathData.PathPoints[PathData.CurrentPathIndex];
    FVector DesiredVelocity = (TargetPoint - CurrentLocation).GetSafeNormal() * PathData.MaxSpeed;
    FVector SteeringForce = DesiredVelocity - PathData.CurrentVelocity;

    return SteeringForce.GetClampedToMaxSize(PathData.MaxSpeed * 0.5f);
}

FVector UCrowd_PathfindingProcessor::CalculateAvoidanceForce(const FCrowd_AvoidanceFragment& AvoidanceData, const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions)
{
    if (NearbyPositions.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    // Calculate flocking forces
    FVector Separation = CalculateSeparation(CurrentLocation, NearbyPositions, AvoidanceData.AvoidanceRadius);
    FVector Alignment = FVector::ZeroVector; // Would need nearby velocities for this
    FVector Cohesion = CalculateCohesion(CurrentLocation, NearbyPositions);

    // Combine forces
    FVector TotalAvoidance = (Separation * SeparationWeight) + (Alignment * AlignmentWeight) + (Cohesion * CohesionWeight);
    
    return TotalAvoidance.GetClampedToMaxSize(300.0f);
}

FVector UCrowd_PathfindingProcessor::CalculateSeparation(const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions, float Radius)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NearbyPos : NearbyPositions)
    {
        float Distance = FVector::Dist(CurrentLocation, NearbyPos);
        if (Distance > 0.0f && Distance < Radius)
        {
            FVector Diff = CurrentLocation - NearbyPos;
            Diff.Normalize();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= 200.0f; // Desired separation speed
    }

    return SeparationForce;
}

FVector UCrowd_PathfindingProcessor::CalculateAlignment(const FVector& CurrentVelocity, const TArray<FVector>& NearbyVelocities)
{
    if (NearbyVelocities.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector AverageVelocity = FVector::ZeroVector;
    for (const FVector& Velocity : NearbyVelocities)
    {
        AverageVelocity += Velocity;
    }
    AverageVelocity /= NearbyVelocities.Num();

    FVector AlignmentForce = AverageVelocity - CurrentVelocity;
    return AlignmentForce.GetClampedToMaxSize(100.0f);
}

FVector UCrowd_PathfindingProcessor::CalculateCohesion(const FVector& CurrentLocation, const TArray<FVector>& NearbyPositions)
{
    if (NearbyPositions.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector CenterOfMass = FVector::ZeroVector;
    for (const FVector& Pos : NearbyPositions)
    {
        CenterOfMass += Pos;
    }
    CenterOfMass /= NearbyPositions.Num();

    FVector CohesionForce = (CenterOfMass - CurrentLocation).GetSafeNormal() * 150.0f;
    return CohesionForce;
}

void UCrowd_PathfindingProcessor::UpdateNearbyEntities(FMassEntityManager& EntityManager, const FVector& Location, float SearchRadius, TArray<FMassEntityHandle>& OutNearbyEntities)
{
    OutNearbyEntities.Empty();

    // Simple brute force search - in production would use spatial partitioning
    EntityManager.ForEachEntityWithFragments<FTransformFragment>([&](FMassEntityHandle Entity, const FTransformFragment& Transform)
    {
        float Distance = FVector::Dist(Location, Transform.GetTransform().GetLocation());
        if (Distance > 0.0f && Distance < SearchRadius)
        {
            OutNearbyEntities.Add(Entity);
        }
    });
}

bool UCrowd_PathfindingProcessor::IsLocationReachable(const FVector& Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
    if (!NavSys)
    {
        return true; // Assume reachable if no nav system
    }

    FNavLocation NavLocation;
    return NavSys->ProjectPointToNavigation(Location, NavLocation);
}

FVector UCrowd_PathfindingProcessor::GetRandomWanderTarget(const FVector& CurrentLocation, float WanderRadius)
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground plane
    RandomDirection.Normalize();

    FVector WanderTarget = CurrentLocation + (RandomDirection * FMath::RandRange(WanderRadius * 0.5f, WanderRadius));
    
    // Try to project to navigation mesh
    UWorld* World = GetWorld();
    if (World)
    {
        UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World);
        if (NavSys)
        {
            FNavLocation NavLocation;
            if (NavSys->ProjectPointToNavigation(WanderTarget, NavLocation))
            {
                WanderTarget = NavLocation.Location;
            }
        }
    }

    return WanderTarget;
}
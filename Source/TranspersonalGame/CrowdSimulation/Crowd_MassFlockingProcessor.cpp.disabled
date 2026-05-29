#include "Crowd_MassFlockingProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_MassFlockingProcessor::UCrowd_MassFlockingProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassFlockingProcessor::ConfigureQueries()
{
    FlockingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FCrowd_FlockingFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddOptionalRequirement<FCrowd_GroupFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassFlockingProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get current time
    float CurrentTime = Context.GetWorld()->GetTimeSeconds();
    float DeltaTime = Context.GetDeltaTimeSeconds();

    // Update group data first
    UpdateGroupData(EntityManager, Context);

    // Process flocking behavior
    FlockingQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto FlockingList = Context.GetMutableFragmentView<FCrowd_FlockingFragment>();
        const auto GroupList = Context.GetOptionalMutableFragmentView<FCrowd_GroupFragment>();

        // Get all entities for neighbor search
        TArray<FMassEntityHandle> AllEntities;
        EntityManager.GetAllEntities(AllEntities);

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_FlockingFragment& Flocking = FlockingList[EntityIndex];

            // Check if we need to update (performance optimization)
            if (CurrentTime - Flocking.LastUpdateTime < (1.0f / UpdateFrequency))
            {
                continue;
            }

            Flocking.LastUpdateTime = CurrentTime;

            // Find neighbors
            TArray<FVector> NeighborPositions;
            TArray<FVector> NeighborVelocities;
            FindNeighbors(Transform.GetTransform().GetLocation(), Flocking.NeighborRadius, 
                         AllEntities, EntityManager, NeighborPositions, NeighborVelocities);

            // Calculate flocking forces
            FVector Separation = CalculateSeparation(Transform.GetTransform().GetLocation(), 
                                                   NeighborPositions, Flocking.SeparationDistance);
            FVector Alignment = CalculateAlignment(Velocity.Value, NeighborVelocities);
            FVector Cohesion = CalculateCohesion(Transform.GetTransform().GetLocation(), NeighborPositions);

            // Combine forces with weights
            FVector TotalForce = (Separation * Flocking.SeparationWeight) +
                               (Alignment * Flocking.AlignmentWeight) +
                               (Cohesion * Flocking.CohesionWeight);

            // Apply group behavior if entity is part of a group
            if (GroupList.IsValidIndex(EntityIndex))
            {
                FCrowd_GroupFragment& Group = GroupList[EntityIndex];
                
                if (Group.GroupRole == ECrowd_GroupRole::Leader)
                {
                    // Leaders have more independent movement
                    TotalForce *= 0.5f;
                }
                else if (Group.GroupRole == ECrowd_GroupRole::Follower)
                {
                    // Followers are more attracted to group center
                    FVector ToGroupCenter = (Group.GroupCenter - Transform.GetTransform().GetLocation()).GetSafeNormal();
                    TotalForce += ToGroupCenter * 200.0f; // Group attraction force
                }
            }

            // Store flocking force
            Flocking.FlockingForce = TotalForce;

            // Apply force to velocity
            Velocity.Value += TotalForce * DeltaTime;

            // Limit speed
            if (Velocity.Value.Size() > Flocking.MaxSpeed)
            {
                Velocity.Value = Velocity.Value.GetSafeNormal() * Flocking.MaxSpeed;
            }

            // Update rotation to face movement direction
            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }

            // Debug visualization in development builds
            #if WITH_EDITOR
            if (CVarDebugCrowdFlocking.GetValueOnGameThread())
            {
                UWorld* World = Context.GetWorld();
                FVector Position = Transform.GetTransform().GetLocation();
                
                // Draw flocking force
                DrawDebugLine(World, Position, Position + TotalForce, FColor::Green, false, 0.1f, 0, 2.0f);
                
                // Draw neighbor connections
                for (const FVector& NeighborPos : NeighborPositions)
                {
                    DrawDebugLine(World, Position, NeighborPos, FColor::Blue, false, 0.1f, 0, 1.0f);
                }
            }
            #endif
        }
    });
}

FVector UCrowd_MassFlockingProcessor::CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions, float SeparationDistance)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborPos : NeighborPositions)
    {
        float Distance = FVector::Dist(Position, NeighborPos);
        if (Distance > 0.0f && Distance < SeparationDistance)
        {
            FVector Diff = (Position - NeighborPos).GetSafeNormal();
            Diff /= Distance; // Weight by distance (closer = stronger repulsion)
            SeparationForce += Diff;
            Count++;
        }
    }

    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * DefaultMaxSpeed;
    }

    return SeparationForce;
}

FVector UCrowd_MassFlockingProcessor::CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities)
{
    if (NeighborVelocities.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector AverageVelocity = FVector::ZeroVector;
    for (const FVector& NeighborVel : NeighborVelocities)
    {
        AverageVelocity += NeighborVel;
    }
    AverageVelocity /= NeighborVelocities.Num();

    FVector AlignmentForce = (AverageVelocity - Velocity).GetSafeNormal() * DefaultMaxSpeed;
    return AlignmentForce;
}

FVector UCrowd_MassFlockingProcessor::CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions)
{
    if (NeighborPositions.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector CenterOfMass = FVector::ZeroVector;
    for (const FVector& NeighborPos : NeighborPositions)
    {
        CenterOfMass += NeighborPos;
    }
    CenterOfMass /= NeighborPositions.Num();

    FVector CohesionForce = (CenterOfMass - Position).GetSafeNormal() * DefaultMaxSpeed;
    return CohesionForce;
}

void UCrowd_MassFlockingProcessor::FindNeighbors(const FVector& Position, float Radius, const TArray<FMassEntityHandle>& AllEntities,
                                               FMassEntityManager& EntityManager, TArray<FVector>& OutPositions, TArray<FVector>& OutVelocities)
{
    OutPositions.Reset();
    OutVelocities.Reset();

    int32 CheckedCount = 0;
    for (const FMassEntityHandle& Entity : AllEntities)
    {
        if (CheckedCount >= MaxNeighborsToCheck)
        {
            break;
        }

        if (EntityManager.IsEntityValid(Entity))
        {
            // Check if entity has required fragments
            if (EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity) &&
                EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity) &&
                EntityManager.GetFragmentDataPtr<FCrowd_FlockingFragment>(Entity))
            {
                const FTransformFragment* Transform = EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity);
                const FMassVelocityFragment* Velocity = EntityManager.GetFragmentDataPtr<FMassVelocityFragment>(Entity);

                FVector NeighborPos = Transform->GetTransform().GetLocation();
                float Distance = FVector::Dist(Position, NeighborPos);

                if (Distance > 0.0f && Distance <= Radius)
                {
                    OutPositions.Add(NeighborPos);
                    OutVelocities.Add(Velocity->Value);
                }
            }
        }
        CheckedCount++;
    }
}

void UCrowd_MassFlockingProcessor::UpdateGroupData(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // This would be expanded to update group centers, sizes, etc.
    // For now, it's a placeholder for group-based behavior
    
    // In a full implementation, this would:
    // 1. Calculate group centers of mass
    // 2. Update group velocities
    // 3. Handle leader/follower relationships
    // 4. Manage group formation and splitting
}
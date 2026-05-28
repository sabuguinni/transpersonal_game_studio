#include "Crowd_MassFlockingBehavior.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_MassFlockingBehaviorProcessor::UCrowd_MassFlockingBehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassFlockingBehaviorProcessor::ConfigureQueries()
{
    FlockingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddRequirement<FCrowd_FlockingFragment>(EMassFragmentAccess::ReadWrite);
    FlockingQuery.AddOptionalRequirement<FCrowd_HerdMemberFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassFlockingBehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    const UWorld* World = EntityManager.GetWorld();
    
    if (!World)
    {
        return;
    }

    // Collect all entity positions and velocities for neighbor calculations
    TArray<FVector> AllPositions;
    TArray<FVector> AllVelocities;
    TArray<int32> FlockIDs;
    TArray<FMassEntityHandle> EntityHandles;

    FlockingQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const int32 NumEntities = QueryContext.GetNumEntities();
        const auto& TransformList = QueryContext.GetFragmentView<FTransformFragment>();
        const auto& VelocityList = QueryContext.GetFragmentView<FMassVelocityFragment>();
        const auto& FlockingList = QueryContext.GetFragmentView<FCrowd_FlockingFragment>();

        for (int32 i = 0; i < NumEntities; ++i)
        {
            AllPositions.Add(TransformList[i].GetTransform().GetLocation());
            AllVelocities.Add(VelocityList[i].Value);
            FlockIDs.Add(FlockingList[i].FlockID);
            EntityHandles.Add(QueryContext.GetEntity(i));
        }
    });

    // Process flocking behavior for each entity
    FlockingQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassExecutionContext& QueryContext)
    {
        const int32 NumEntities = QueryContext.GetNumEntities();
        auto& TransformList = QueryContext.GetMutableFragmentView<FTransformFragment>();
        auto& VelocityList = QueryContext.GetMutableFragmentView<FMassVelocityFragment>();
        auto& FlockingList = QueryContext.GetMutableFragmentView<FCrowd_FlockingFragment>();
        auto* HerdList = QueryContext.GetMutableFragmentView<FCrowd_HerdMemberFragment>().GetData();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FVector CurrentPosition = TransformList[EntityIndex].GetTransform().GetLocation();
            const FVector CurrentVelocity = VelocityList[EntityIndex].Value;
            FCrowd_FlockingFragment& FlockingData = FlockingList[EntityIndex];

            // Find neighbors within the same flock
            TArray<FVector> NeighborPositions;
            TArray<FVector> NeighborVelocities;
            
            for (int32 i = 0; i < AllPositions.Num(); ++i)
            {
                if (i == EntityIndex || FlockIDs[i] != FlockingData.FlockID)
                {
                    continue;
                }

                const float Distance = FVector::Dist(CurrentPosition, AllPositions[i]);
                if (Distance < MaxNeighborDistance && NeighborPositions.Num() < MaxNeighbors)
                {
                    NeighborPositions.Add(AllPositions[i]);
                    NeighborVelocities.Add(AllVelocities[i]);
                }
            }

            // Calculate flocking forces
            FVector Separation = CalculateSeparation(CurrentPosition, NeighborPositions, FlockingData.SeparationRadius);
            FVector Alignment = CalculateAlignment(CurrentVelocity, NeighborVelocities, FlockingData.AlignmentRadius);
            FVector Cohesion = CalculateCohesion(CurrentPosition, NeighborPositions, FlockingData.CohesionRadius);

            // Apply weights
            Separation *= FlockingData.SeparationWeight;
            Alignment *= FlockingData.AlignmentWeight;
            Cohesion *= FlockingData.CohesionWeight;

            // Combine forces
            FVector TotalForce = Separation + Alignment + Cohesion;

            // Add herd behavior if available
            if (HerdList)
            {
                FCrowd_HerdMemberFragment& HerdData = HerdList[EntityIndex];
                
                // Find herd members
                TArray<FVector> HerdPositions;
                for (int32 i = 0; i < AllPositions.Num(); ++i)
                {
                    if (i != EntityIndex && HerdList && HerdList[i].HerdID == HerdData.HerdID)
                    {
                        HerdPositions.Add(AllPositions[i]);
                    }
                }

                FVector HerdForce = CalculateHerdBehavior(HerdData, CurrentPosition, HerdPositions);
                TotalForce += HerdForce;

                // Update stress level
                bool bThreatDetected = false; // TODO: Implement threat detection
                UpdateStressLevel(HerdData, DeltaTime, bThreatDetected);
            }

            // Apply desired direction for leaders
            if (FlockingData.bIsLeader)
            {
                TotalForce += FlockingData.DesiredDirection * 100.0f;
            }

            // Limit force magnitude
            if (TotalForce.SizeSquared() > FlockingData.MaxForce * FlockingData.MaxForce)
            {
                TotalForce = TotalForce.GetSafeNormal() * FlockingData.MaxForce;
            }

            // Update velocity
            FVector NewVelocity = CurrentVelocity + (TotalForce * DeltaTime);
            
            // Limit speed
            if (NewVelocity.SizeSquared() > FlockingData.MaxSpeed * FlockingData.MaxSpeed)
            {
                NewVelocity = NewVelocity.GetSafeNormal() * FlockingData.MaxSpeed;
            }

            VelocityList[EntityIndex].Value = NewVelocity;

            // Update transform
            if (!NewVelocity.IsNearlyZero())
            {
                FTransform& Transform = TransformList[EntityIndex].GetMutableTransform();
                Transform.SetLocation(CurrentPosition + (NewVelocity * DeltaTime));
                Transform.SetRotation(FQuat::FindBetweenNormals(FVector::ForwardVector, NewVelocity.GetSafeNormal()));
            }
        }
    });
}

FVector UCrowd_MassFlockingBehaviorProcessor::CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions, float Radius) const
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborPos : NeighborPositions)
    {
        const float Distance = FVector::Dist(Position, NeighborPos);
        if (Distance > 0.0f && Distance < Radius)
        {
            FVector Diff = Position - NeighborPos;
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
    }

    return SeparationForce;
}

FVector UCrowd_MassFlockingBehaviorProcessor::CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities, float Radius) const
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborVel : NeighborVelocities)
    {
        AlignmentForce += NeighborVel;
        Count++;
    }

    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce.Normalize();
    }

    return AlignmentForce;
}

FVector UCrowd_MassFlockingBehaviorProcessor::CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions, float Radius) const
{
    FVector CohesionForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborPos : NeighborPositions)
    {
        const float Distance = FVector::Dist(Position, NeighborPos);
        if (Distance < Radius)
        {
            CohesionForce += NeighborPos;
            Count++;
        }
    }

    if (Count > 0)
    {
        CohesionForce /= Count;
        CohesionForce = (CohesionForce - Position).GetSafeNormal();
    }

    return CohesionForce;
}

FVector UCrowd_MassFlockingBehaviorProcessor::CalculateHerdBehavior(const FCrowd_HerdMemberFragment& HerdMember, const FVector& Position, const TArray<FVector>& HerdPositions) const
{
    FVector HerdForce = FVector::ZeroVector;

    if (HerdPositions.Num() == 0)
    {
        return HerdForce;
    }

    // Calculate herd center
    FVector HerdCenter = FVector::ZeroVector;
    for (const FVector& HerdPos : HerdPositions)
    {
        HerdCenter += HerdPos;
    }
    HerdCenter /= HerdPositions.Num();

    // Move towards herd center if too far
    const float DistanceToCenter = FVector::Dist(Position, HerdCenter);
    if (DistanceToCenter > HerdMember.HerdRadius)
    {
        HerdForce = (HerdCenter - Position).GetSafeNormal() * 150.0f;
    }

    // Apply stress-based behavior
    if (HerdMember.StressLevel > 0.5f)
    {
        // High stress: move closer to herd
        HerdForce += (HerdCenter - Position).GetSafeNormal() * (HerdMember.StressLevel * 100.0f);
    }

    return HerdForce;
}

void UCrowd_MassFlockingBehaviorProcessor::UpdateStressLevel(FCrowd_HerdMemberFragment& HerdMember, float DeltaTime, bool bThreatDetected) const
{
    if (bThreatDetected)
    {
        HerdMember.StressLevel = FMath::Min(1.0f, HerdMember.StressLevel + (DeltaTime * 2.0f));
        HerdMember.TimeSinceLastThreat = 0.0f;
    }
    else
    {
        HerdMember.TimeSinceLastThreat += DeltaTime;
        
        // Decay stress over time
        if (HerdMember.TimeSinceLastThreat > 5.0f)
        {
            HerdMember.StressLevel = FMath::Max(0.0f, HerdMember.StressLevel - (DeltaTime * StressDecayRate));
        }
    }
}
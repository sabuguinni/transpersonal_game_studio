#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassEntityProcessor::UCrowd_MassEntityProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void UCrowd_MassEntityProcessor::ConfigureQueries()
{
    CrowdQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    CrowdQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    CrowdQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    CrowdQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassEntityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Update LOD system first for performance
    UpdateLODSystem(EntityManager, Context);

    // Process crowd behavior
    ProcessCrowdBehavior(EntityManager, Context);

    // Handle group formation
    ProcessGroupFormation(EntityManager, Context);
}

void UCrowd_MassEntityProcessor::ProcessCrowdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    CrowdQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetFragmentView<FCrowd_LODFragment>();

        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            // Skip if entity is not visible (LOD culled)
            if (!LODList[i].bIsVisible)
            {
                continue;
            }

            FTransformFragment& Transform = TransformList[i];
            FMassVelocityFragment& Velocity = VelocityList[i];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[i];

            // Simple flocking behavior
            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector DesiredVelocity = FVector::ZeroVector;

            // Separation - avoid crowding neighbors
            FVector Separation = FVector::ZeroVector;
            int32 SeparationCount = 0;

            // Alignment - steer towards average heading of neighbors
            FVector Alignment = FVector::ZeroVector;
            int32 AlignmentCount = 0;

            // Cohesion - steer towards average position of neighbors
            FVector Cohesion = FVector::ZeroVector;
            int32 CohesionCount = 0;

            // Check neighbors within social radius
            for (int32 j = 0; j < NumEntities; ++j)
            {
                if (i == j || !LODList[j].bIsVisible) continue;

                FVector NeighborLocation = TransformList[j].GetTransform().GetLocation();
                float Distance = FVector::Dist(CurrentLocation, NeighborLocation);

                if (Distance < Behavior.SocialRadius && Distance > 0.0f)
                {
                    // Separation
                    FVector Diff = CurrentLocation - NeighborLocation;
                    Diff.Normalize();
                    Diff /= Distance; // Weight by distance
                    Separation += Diff;
                    SeparationCount++;

                    // Alignment
                    Alignment += VelocityList[j].Value;
                    AlignmentCount++;

                    // Cohesion
                    Cohesion += NeighborLocation;
                    CohesionCount++;
                }
            }

            // Calculate steering forces
            if (SeparationCount > 0)
            {
                Separation /= SeparationCount;
                Separation.Normalize();
                DesiredVelocity += Separation * 2.0f; // Higher weight for separation
            }

            if (AlignmentCount > 0)
            {
                Alignment /= AlignmentCount;
                Alignment.Normalize();
                DesiredVelocity += Alignment * 1.0f;
            }

            if (CohesionCount > 0)
            {
                Cohesion /= CohesionCount;
                Cohesion = (Cohesion - CurrentLocation).GetSafeNormal();
                DesiredVelocity += Cohesion * 1.0f;
            }

            // Apply movement speed
            DesiredVelocity *= Behavior.MovementSpeed;

            // Smooth velocity transition
            FVector CurrentVelocity = Velocity.Value;
            FVector NewVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, Context.GetDeltaTimeSeconds(), 2.0f);

            // Update velocity
            Velocity.Value = NewVelocity;
        }
    });
}

void UCrowd_MassEntityProcessor::UpdateLODSystem(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get player location for distance calculations
    UWorld* World = Context.GetWorld();
    if (!World) return;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    CrowdQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();

        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            FVector EntityLocation = TransformList[i].GetTransform().GetLocation();
            float DistanceToPlayer = FVector::Dist(PlayerLocation, EntityLocation);

            FCrowd_LODFragment& LOD = LODList[i];
            LOD.DistanceToPlayer = DistanceToPlayer;

            // Determine LOD level
            if (DistanceToPlayer <= LOD0Distance)
            {
                LOD.CurrentLOD = 0;
                LOD.bIsVisible = true;
            }
            else if (DistanceToPlayer <= LOD1Distance)
            {
                LOD.CurrentLOD = 1;
                LOD.bIsVisible = true;
            }
            else if (DistanceToPlayer <= LOD2Distance)
            {
                LOD.CurrentLOD = 2;
                LOD.bIsVisible = true;
            }
            else
            {
                LOD.CurrentLOD = 3;
                LOD.bIsVisible = false; // Cull entities beyond max distance
            }

            LOD.LastUpdateTime = Context.GetTime();
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessGroupFormation(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    CrowdQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetFragmentView<FCrowd_LODFragment>();

        const int32 NumEntities = Context.GetNumEntities();
        
        // Process group leaders first
        for (int32 i = 0; i < NumEntities; ++i)
        {
            if (!BehaviorList[i].bIsLeader || !LODList[i].bIsVisible) continue;

            FTransformFragment& LeaderTransform = TransformList[i];
            FMassVelocityFragment& LeaderVelocity = VelocityList[i];
            const FCrowd_BehaviorFragment& LeaderBehavior = BehaviorList[i];

            // Leader behavior - move towards objectives or patrol
            FVector LeaderLocation = LeaderTransform.GetTransform().GetLocation();
            
            // Simple patrol behavior for leaders
            FVector PatrolDirection = FVector(FMath::Cos(Context.GetTime() * 0.5f), FMath::Sin(Context.GetTime() * 0.5f), 0.0f);
            PatrolDirection.Normalize();
            
            FVector DesiredVelocity = PatrolDirection * LeaderBehavior.MovementSpeed * 0.8f;
            LeaderVelocity.Value = FMath::VInterpTo(LeaderVelocity.Value, DesiredVelocity, Context.GetDeltaTimeSeconds(), 1.5f);

            // Influence followers in the same group
            for (int32 j = 0; j < NumEntities; ++j)
            {
                if (i == j || BehaviorList[j].bIsLeader || !LODList[j].bIsVisible) continue;
                if (BehaviorList[j].GroupID != LeaderBehavior.GroupID) continue;

                FVector FollowerLocation = TransformList[j].GetTransform().GetLocation();
                float Distance = FVector::Dist(LeaderLocation, FollowerLocation);

                if (Distance < LeaderBehavior.SocialRadius * 2.0f)
                {
                    // Followers try to stay near their leader
                    FVector ToLeader = (LeaderLocation - FollowerLocation).GetSafeNormal();
                    FVector FollowVelocity = ToLeader * BehaviorList[j].MovementSpeed * 0.6f;
                    
                    VelocityList[j].Value += FollowVelocity * Context.GetDeltaTimeSeconds();
                }
            }
        }
    });
}
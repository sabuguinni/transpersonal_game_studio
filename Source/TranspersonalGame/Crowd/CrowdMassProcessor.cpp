#include "CrowdMassProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UCrowdMassProcessor::UCrowdMassProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowdMassProcessor::ConfigureQueries()
{
    // Configure movement query for basic crowd movement
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddTagRequirement<FCrowd_MovingTag>(EMassFragmentPresence::All);

    // Configure behavior query for crowd AI decisions
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddTagRequirement<FCrowd_TribalTag>(EMassFragmentPresence::All);

    // Configure LOD query for performance optimization
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FMassVisualizationLODFragment>(EMassFragmentAccess::ReadWrite);

    // Configure social interaction query
    SocialQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    SocialQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    SocialQuery.AddTagRequirement<FCrowd_SocialTag>(EMassFragmentPresence::All);
}

void UCrowdMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Reset frame counters
    ProcessedEntitiesThisFrame = 0;
    float FrameStartTime = FPlatformTime::Seconds();

    // Process different crowd behaviors in priority order
    ProcessCrowdLOD(EntityManager, Context);
    ProcessCrowdMovement(EntityManager, Context);
    ProcessCrowdBehavior(EntityManager, Context);

    // Track processing time for performance monitoring
    LastProcessTime = FPlatformTime::Seconds() - FrameStartTime;
    
    if (LastProcessTime > ProcessingTimeLimit)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdMassProcessor exceeded time limit: %.3fms"), LastProcessTime * 1000.0f);
    }
}

void UCrowdMassProcessor::ProcessCrowdMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    MovementQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            if (ProcessedEntitiesThisFrame >= MaxEntitiesPerFrame)
            {
                break;
            }

            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];

            // Basic crowd movement logic
            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            
            // Simple wandering behavior for now
            if (Velocity.Value.IsNearlyZero())
            {
                // Generate random direction
                FVector RandomDirection = FVector(
                    FMath::RandRange(-1.0f, 1.0f),
                    FMath::RandRange(-1.0f, 1.0f),
                    0.0f
                ).GetSafeNormal();

                Velocity.Value = RandomDirection * MovementSpeed;
            }

            // Apply movement
            FVector NewLocation = CurrentLocation + Velocity.Value * Context.GetDeltaTimeSeconds();
            Transform.GetMutableTransform().SetLocation(NewLocation);

            ProcessedEntitiesThisFrame++;
        }
    });
}

void UCrowdMassProcessor::ProcessCrowdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UpdateTribalBehavior(EntityManager, Context);
    UpdateGatheringBehavior(EntityManager, Context);
    UpdateSocialInteractions(EntityManager, Context);
}

void UCrowdMassProcessor::ProcessCrowdLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    LODQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVisualizationLODFragment> LODList = Context.GetMutableFragmentView<FMassVisualizationLODFragment>();

        // Get player location for distance calculations
        FVector PlayerLocation = FVector::ZeroVector;
        if (UWorld* World = Context.GetWorld())
        {
            if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
            {
                PlayerLocation = PlayerPawn->GetActorLocation();
            }
        }

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVisualizationLODFragment& LOD = LODList[EntityIndex];

            FVector EntityLocation = Transform.GetTransform().GetLocation();
            float DistanceToPlayer = FVector::Dist(EntityLocation, PlayerLocation);

            // Set LOD based on distance
            if (DistanceToPlayer < HighDetailDistance)
            {
                LOD.LODLevel = EMassLOD::High;
            }
            else if (DistanceToPlayer < MediumDetailDistance)
            {
                LOD.LODLevel = EMassLOD::Medium;
            }
            else if (DistanceToPlayer < LowDetailDistance)
            {
                LOD.LODLevel = EMassLOD::Low;
            }
            else
            {
                LOD.LODLevel = EMassLOD::Off;
            }
        }
    });
}

void UCrowdMassProcessor::UpdateTribalBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];

            // Tribal cohesion behavior - stay near tribal center
            FVector EntityLocation = Transform.GetTransform().GetLocation();
            
            // For now, use a simple tribal center at origin
            FVector TribalCenter = FVector::ZeroVector;
            FVector ToCenter = TribalCenter - EntityLocation;
            
            if (ToCenter.Size() > TribalCohesionRadius)
            {
                // Move towards tribal center
                FVector CohesionForce = ToCenter.GetSafeNormal() * MovementSpeed * 0.5f;
                Velocity.Value += CohesionForce * Context.GetDeltaTimeSeconds();
            }
        }
    });
}

void UCrowdMassProcessor::UpdateGatheringBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Implement gathering behavior around resource points
    // For now, this is a placeholder for future resource system integration
}

void UCrowdMassProcessor::UpdateSocialInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SocialQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const int32 NumEntities = Context.GetNumEntities();
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

        // Simple social interaction: avoid crowding
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];

            FVector EntityLocation = Transform.GetTransform().GetLocation();
            FVector AvoidanceForce = FVector::ZeroVector;

            // Check against other entities in this chunk
            for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
            {
                if (EntityIndex == OtherIndex) continue;

                FVector OtherLocation = TransformList[OtherIndex].GetTransform().GetLocation();
                FVector ToOther = OtherLocation - EntityLocation;
                float Distance = ToOther.Size();

                if (Distance < SocialInteractionRadius && Distance > 0.0f)
                {
                    // Add avoidance force
                    FVector AvoidDirection = -ToOther.GetSafeNormal();
                    float AvoidStrength = (SocialInteractionRadius - Distance) / SocialInteractionRadius;
                    AvoidanceForce += AvoidDirection * AvoidStrength * MovementSpeed;
                }
            }

            // Apply avoidance force
            Velocity.Value += AvoidanceForce * Context.GetDeltaTimeSeconds() * 0.5f;
        }
    });
}
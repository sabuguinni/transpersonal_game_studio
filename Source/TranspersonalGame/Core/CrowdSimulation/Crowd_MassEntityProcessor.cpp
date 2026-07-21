#include "Crowd_MassEntityProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// Movement Processor Implementation
UCrowd_MovementProcessor::UCrowd_MovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UCrowd_MovementProcessor::ConfigureQueries()
{
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    MovementQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];

            if (Movement.bIsMoving)
            {
                FVector Direction = (Movement.TargetLocation - Transform.GetTransform().GetLocation()).GetSafeNormal();
                float DistanceToTarget = FVector::Dist(Transform.GetTransform().GetLocation(), Movement.TargetLocation);

                if (DistanceToTarget > 50.0f)
                {
                    Velocity.Value = Direction * Movement.MovementSpeed;
                    Transform.GetMutableTransform().SetLocation(Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime);
                }
                else
                {
                    Movement.bIsMoving = false;
                    Velocity.Value = FVector::ZeroVector;
                    
                    // Generate new random target within wander radius
                    FVector CurrentLocation = Transform.GetTransform().GetLocation();
                    FVector RandomDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
                    Movement.TargetLocation = CurrentLocation + RandomDirection * FMath::RandRange(100.0f, Movement.WanderRadius);
                    Movement.bIsMoving = true;
                }
            }
        }
    });
}

// Behavior Processor Implementation
UCrowd_BehaviorProcessor::UCrowd_BehaviorProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UCrowd_BehaviorProcessor::ConfigureQueries()
{
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_BehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];

            UpdateBehavior(Behavior, Movement, Transform, DeltaTime);
        }
    });
}

void UCrowd_BehaviorProcessor::UpdateBehavior(FCrowd_BehaviorFragment& BehaviorFragment, FCrowd_MovementFragment& MovementFragment, const FTransformFragment& TransformFragment, float DeltaTime)
{
    BehaviorFragment.BehaviorTimer += DeltaTime;

    if (BehaviorFragment.BehaviorTimer >= BehaviorFragment.BehaviorDuration)
    {
        // Switch to new behavior
        int32 RandomBehavior = FMath::RandRange(0, 3);
        switch (RandomBehavior)
        {
        case 0:
            BehaviorFragment.CurrentBehavior = ECrowd_NPCBehavior::Wandering;
            MovementFragment.MovementSpeed = 100.0f;
            MovementFragment.WanderRadius = 800.0f;
            break;
        case 1:
            BehaviorFragment.CurrentBehavior = ECrowd_NPCBehavior::Gathering;
            MovementFragment.MovementSpeed = 80.0f;
            MovementFragment.WanderRadius = 300.0f;
            break;
        case 2:
            BehaviorFragment.CurrentBehavior = ECrowd_NPCBehavior::Socializing;
            MovementFragment.MovementSpeed = 50.0f;
            MovementFragment.WanderRadius = 200.0f;
            break;
        case 3:
            BehaviorFragment.CurrentBehavior = ECrowd_NPCBehavior::Working;
            MovementFragment.MovementSpeed = 120.0f;
            MovementFragment.WanderRadius = 400.0f;
            break;
        }

        BehaviorFragment.BehaviorTimer = 0.0f;
        BehaviorFragment.BehaviorDuration = FMath::RandRange(3.0f, 8.0f);
    }
}

// LOD Processor Implementation
UCrowd_LODProcessor::UCrowd_LODProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
}

void UCrowd_LODProcessor::ConfigureQueries()
{
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_LODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    UWorld* World = Context.GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    LODQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_LODFragment& LOD = LODList[EntityIndex];

            float Distance = FVector::Dist(Transform.GetTransform().GetLocation(), PlayerLocation);
            LOD.DistanceToPlayer = Distance;

            // Update LOD level based on distance
            if (Distance <= HighLODDistance)
            {
                LOD.CurrentLOD = ECrowd_LODLevel::High;
                LOD.bIsVisible = true;
            }
            else if (Distance <= MediumLODDistance)
            {
                LOD.CurrentLOD = ECrowd_LODLevel::Medium;
                LOD.bIsVisible = true;
            }
            else if (Distance <= LowLODDistance)
            {
                LOD.CurrentLOD = ECrowd_LODLevel::Low;
                LOD.bIsVisible = true;
            }
            else
            {
                LOD.CurrentLOD = ECrowd_LODLevel::Culled;
                LOD.bIsVisible = false;
            }
        }
    });
}
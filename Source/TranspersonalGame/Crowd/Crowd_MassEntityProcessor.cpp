#include "Crowd_MassEntityProcessor.h"
#include "MassEntityManager.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityView.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"

UCrowd_MassEntityProcessor::UCrowd_MassEntityProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = (int32)EProcessorExecutionFlags::All;
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    
    MaxEntitiesPerFrame = 1000;
    ProcessingTimeLimit = 0.016f; // 16ms target
    CrowdDensityThreshold = 100.0f;
    AvoidanceRadius = 150.0f;
}

void UCrowd_MassEntityProcessor::ConfigureQueries()
{
    // Movement query - entities that need position updates
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddTagRequirement<FCrowd_MovingTag>(EMassFragmentPresence::All);
    
    // Behavior query - entities with AI behavior
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddTagRequirement<FCrowd_ActiveTag>(EMassFragmentPresence::All);
    
    // LOD query - all entities for level-of-detail processing
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassEntityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    double StartTime = FPlatformTime::Seconds();
    
    // Process movement first
    ProcessMovement(EntityManager, Context);
    
    // Check time budget
    if (FPlatformTime::Seconds() - StartTime > ProcessingTimeLimit * 0.5)
    {
        return; // Skip other processing if running out of time
    }
    
    // Process behavior
    ProcessBehavior(EntityManager, Context);
    
    // Check time budget again
    if (FPlatformTime::Seconds() - StartTime > ProcessingTimeLimit * 0.8)
    {
        return; // Skip LOD processing if needed
    }
    
    // Process LOD
    ProcessLOD(EntityManager, Context);
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
            
            // Simple movement integration
            FVector NewLocation = Transform.GetTransform().GetLocation() + (Velocity.Value * DeltaTime);
            
            // Basic collision avoidance with other entities
            for (int32 j = 0; j < NumEntities; ++j)
            {
                if (i == j) continue;
                
                FVector OtherLocation = TransformList[j].GetTransform().GetLocation();
                float Distance = FVector::Dist(NewLocation, OtherLocation);
                
                if (Distance < AvoidanceRadius)
                {
                    FVector AvoidanceVector = (NewLocation - OtherLocation).GetSafeNormal();
                    NewLocation += AvoidanceVector * (AvoidanceRadius - Distance) * 0.5f;
                }
            }
            
            // Update transform
            FTransform NewTransform = Transform.GetTransform();
            NewTransform.SetLocation(NewLocation);
            Transform.SetTransform(NewTransform);
            
            // Apply drag
            Velocity.Value *= 0.95f;
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        
        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            const FTransformFragment& Transform = TransformList[i];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[i];
            
            // Update behavior timer
            Behavior.StateTimer += DeltaTime;
            
            // Simple state machine
            switch (Behavior.CurrentState)
            {
                case ECrowd_BehaviorState::Wandering:
                    if (Behavior.StateTimer > 5.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Idle;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
                    
                case ECrowd_BehaviorState::Idle:
                    if (Behavior.StateTimer > 3.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                        Behavior.StateTimer = 0.0f;
                        // Set random target
                        Behavior.TargetLocation = Transform.GetTransform().GetLocation() + 
                            FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
                    }
                    break;
                    
                case ECrowd_BehaviorState::Following:
                    // Follow behavior logic would go here
                    break;
                    
                case ECrowd_BehaviorState::Fleeing:
                    // Flee behavior logic would go here
                    if (Behavior.StateTimer > 10.0f)
                    {
                        Behavior.CurrentState = ECrowd_BehaviorState::Wandering;
                        Behavior.StateTimer = 0.0f;
                    }
                    break;
            }
        }
    });
}

void UCrowd_MassEntityProcessor::ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    LODQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();
        
        const int32 NumEntities = Context.GetNumEntities();
        
        // Get player location for distance calculations
        UWorld* World = Context.GetWorld();
        FVector PlayerLocation = FVector::ZeroVector;
        if (World && World->GetFirstPlayerController() && World->GetFirstPlayerController()->GetPawn())
        {
            PlayerLocation = World->GetFirstPlayerController()->GetPawn()->GetActorLocation();
        }
        
        for (int32 i = 0; i < NumEntities; ++i)
        {
            const FTransformFragment& Transform = TransformList[i];
            FCrowd_LODFragment& LOD = LODList[i];
            
            // Calculate distance to player
            float DistanceToPlayer = FVector::Dist(Transform.GetTransform().GetLocation(), PlayerLocation);
            
            // Determine LOD level
            ECrowd_LODLevel NewLODLevel;
            if (DistanceToPlayer < 1000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::High;
            }
            else if (DistanceToPlayer < 5000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::Medium;
            }
            else if (DistanceToPlayer < 15000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::Low;
            }
            else
            {
                NewLODLevel = ECrowd_LODLevel::Culled;
            }
            
            // Update LOD if changed
            if (LOD.CurrentLOD != NewLODLevel)
            {
                LOD.CurrentLOD = NewLODLevel;
                LOD.bLODChanged = true;
            }
            else
            {
                LOD.bLODChanged = false;
            }
        }
    });
}
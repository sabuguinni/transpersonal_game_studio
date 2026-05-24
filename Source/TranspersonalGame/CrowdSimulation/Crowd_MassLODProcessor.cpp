#include "Crowd_MassLODProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassLODProcessor::UCrowd_MassLODProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UCrowd_MassLODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FCrowd_TransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassLODProcessor::Execute(UMassEntitySubsystem& EntitySubsystem, FMassExecutionContext& Context)
{
    // Get player location for distance calculations
    UWorld* World = EntitySubsystem.GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (!PlayerController || !PlayerController->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();

    // Process all entities with LOD fragments
    EntityQuery.ForEachEntityChunk(EntitySubsystem, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const int32 NumEntities = EntityCollection.GetNumEntities();
        const TArrayView<FCrowd_TransformFragment> TransformList = EntityCollection.GetFragmentView<FCrowd_TransformFragment>();
        const TArrayView<FCrowd_LODFragment> LODList = EntityCollection.GetFragmentView<FCrowd_LODFragment>();
        const TArrayView<FCrowd_MovementFragment> MovementList = EntityCollection.GetFragmentView<FCrowd_MovementFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FCrowd_TransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_LODFragment& LOD = LODList[EntityIndex];
            const FCrowd_MovementFragment& Movement = MovementList[EntityIndex];

            // Calculate distance to player
            float DistanceToPlayer = FVector::Dist(Transform.Transform.GetLocation(), PlayerLocation);

            // Determine LOD level based on distance
            ECrowd_LODLevel NewLODLevel = ECrowd_LODLevel::High;
            
            if (DistanceToPlayer > 10000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::Culled;
            }
            else if (DistanceToPlayer > 5000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::Low;
            }
            else if (DistanceToPlayer > 2000.0f)
            {
                NewLODLevel = ECrowd_LODLevel::Medium;
            }
            else
            {
                NewLODLevel = ECrowd_LODLevel::High;
            }

            // Update LOD if changed
            if (LOD.LODLevel != NewLODLevel)
            {
                LOD.LODLevel = NewLODLevel;
                LOD.LastLODUpdateTime = World->GetTimeSeconds();

                // Adjust update frequency based on LOD level
                switch (NewLODLevel)
                {
                case ECrowd_LODLevel::High:
                    LOD.UpdateFrequency = 60.0f; // 60 FPS
                    LOD.bIsVisible = true;
                    break;
                case ECrowd_LODLevel::Medium:
                    LOD.UpdateFrequency = 30.0f; // 30 FPS
                    LOD.bIsVisible = true;
                    break;
                case ECrowd_LODLevel::Low:
                    LOD.UpdateFrequency = 10.0f; // 10 FPS
                    LOD.bIsVisible = true;
                    break;
                case ECrowd_LODLevel::Culled:
                    LOD.UpdateFrequency = 1.0f;  // 1 FPS for distance checks only
                    LOD.bIsVisible = false;
                    break;
                }
            }

            // Update visibility distance for rendering
            LOD.VisibilityDistance = DistanceToPlayer;
            
            // Calculate screen size for additional LOD decisions
            if (DistanceToPlayer > 0.0f)
            {
                // Estimate screen size based on distance (simplified calculation)
                float EstimatedSize = 100.0f / DistanceToPlayer; // Arbitrary scale factor
                LOD.ScreenSize = FMath::Clamp(EstimatedSize, 0.0f, 1.0f);
            }
            else
            {
                LOD.ScreenSize = 1.0f;
            }

            // Apply movement speed scaling based on LOD
            if (Movement.Speed > 0.0f)
            {
                switch (LOD.LODLevel)
                {
                case ECrowd_LODLevel::High:
                    LOD.MovementSpeedMultiplier = 1.0f;
                    break;
                case ECrowd_LODLevel::Medium:
                    LOD.MovementSpeedMultiplier = 0.8f;
                    break;
                case ECrowd_LODLevel::Low:
                    LOD.MovementSpeedMultiplier = 0.5f;
                    break;
                case ECrowd_LODLevel::Culled:
                    LOD.MovementSpeedMultiplier = 0.1f; // Very slow for distant entities
                    break;
                }
            }
        }
    });
}

void UCrowd_MassLODProcessor::Initialize(UObject& Owner)
{
    Super::Initialize(Owner);
    
    // Initialize LOD settings
    LODDistances.Add(ECrowd_LODLevel::High, 2000.0f);
    LODDistances.Add(ECrowd_LODLevel::Medium, 5000.0f);
    LODDistances.Add(ECrowd_LODLevel::Low, 10000.0f);
    LODDistances.Add(ECrowd_LODLevel::Culled, 20000.0f);
}

float UCrowd_MassLODProcessor::GetLODDistance(ECrowd_LODLevel LODLevel) const
{
    const float* Distance = LODDistances.Find(LODLevel);
    return Distance ? *Distance : 10000.0f;
}

void UCrowd_MassLODProcessor::SetLODDistance(ECrowd_LODLevel LODLevel, float Distance)
{
    LODDistances.Add(LODLevel, Distance);
}
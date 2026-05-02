#include "Crowd_MassLODProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Crowd_SharedTypes.h"

UCrowd_MassLODProcessor::UCrowd_MassLODProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
    
    LODDistances.Add(500.0f);   // LOD 0 to 1
    LODDistances.Add(1500.0f);  // LOD 1 to 2
    LODDistances.Add(3000.0f);  // LOD 2 to 3
    LODDistances.Add(5000.0f);  // LOD 3 to cull
}

void UCrowd_MassLODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FMassRepresentationLODFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_DinosaurFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get player location for distance calculations
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

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [&](FMassArchetypeEntityCollection& EntityCollection)
    {
        const int32 NumEntities = EntityCollection.GetNumEntities();
        const TArrayView<FMassRepresentationLODFragment> LODList = EntityCollection.GetMutableFragmentView<FMassRepresentationLODFragment>();
        const TConstArrayView<FTransformFragment> TransformList = EntityCollection.GetFragmentView<FTransformFragment>();
        const TConstArrayView<FCrowd_DinosaurFragment> DinosaurList = EntityCollection.GetFragmentView<FCrowd_DinosaurFragment>();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FMassRepresentationLODFragment& LODFragment = LODList[EntityIndex];
            const FTransformFragment& TransformFragment = TransformList[EntityIndex];
            const FCrowd_DinosaurFragment& DinosaurFragment = DinosaurList[EntityIndex];

            // Calculate distance to player
            const float DistanceToPlayer = FVector::Dist(PlayerLocation, TransformFragment.GetTransform().GetLocation());
            
            // Determine LOD level based on distance and dinosaur type
            EMassLOD::Type NewLOD = CalculateLODLevel(DistanceToPlayer, DinosaurFragment.Species);
            
            // Update LOD if changed
            if (LODFragment.LOD != NewLOD)
            {
                LODFragment.LOD = NewLOD;
                LODFragment.PrevLOD = LODFragment.LOD;
                
                // Mark for representation update
                Context.Defer().PushCommand<FMassCommandAddFragmentInstances>(EntityCollection.GetEntity(EntityIndex), 
                    FMassRepresentationFragment::StaticStruct());
            }
        }
    });
}

EMassLOD::Type UCrowd_MassLODProcessor::CalculateLODLevel(float Distance, ECrowd_DinosaurSpecies Species) const
{
    // Adjust LOD distances based on dinosaur importance
    float LODMultiplier = 1.0f;
    
    switch (Species)
    {
        case ECrowd_DinosaurSpecies::TRex:
            LODMultiplier = 2.0f; // Keep T-Rex visible longer
            break;
        case ECrowd_DinosaurSpecies::Raptor:
            LODMultiplier = 1.5f; // Predators stay visible longer
            break;
        case ECrowd_DinosaurSpecies::Triceratops:
        case ECrowd_DinosaurSpecies::Brachiosaurus:
            LODMultiplier = 1.2f; // Large herbivores
            break;
        default:
            LODMultiplier = 1.0f;
            break;
    }

    // Apply multiplier to distances
    const float AdjustedDistance = Distance / LODMultiplier;
    
    if (AdjustedDistance < LODDistances[0])
        return EMassLOD::High;
    else if (AdjustedDistance < LODDistances[1])
        return EMassLOD::Medium;
    else if (AdjustedDistance < LODDistances[2])
        return EMassLOD::Low;
    else if (AdjustedDistance < LODDistances[3])
        return EMassLOD::Off;
    else
        return EMassLOD::Max; // Culled
}

void UCrowd_MassLODProcessor::SetLODDistances(const TArray<float>& NewDistances)
{
    if (NewDistances.Num() >= 4)
    {
        LODDistances = NewDistances;
    }
}

float UCrowd_MassLODProcessor::GetLODDistance(int32 LODLevel) const
{
    if (LODDistances.IsValidIndex(LODLevel))
    {
        return LODDistances[LODLevel];
    }
    return 1000.0f;
}
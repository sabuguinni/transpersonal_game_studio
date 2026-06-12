#include "Crowd_MassEntityTraits.h"
#include "MassEntityTemplateRegistry.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"

UCrowd_MassEntityTrait::UCrowd_MassEntityTrait()
{
    MovementSpeed = 250.0f;
    FlockingRadius = 150.0f;
    MaxEntitiesPerGroup = 20;
    HighLODDistance = 500.0f;
    MediumLODDistance = 1500.0f;
    LowLODDistance = 3000.0f;
    MeshVariants = 5;
    bEnableShadowCasting = true;
}

void UCrowd_MassEntityTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
    // Add core Mass fragments
    BuildContext.AddFragment<FTransformFragment>();
    BuildContext.AddFragment<FMassVelocityFragment>();
    BuildContext.AddFragment<FMassRepresentationFragment>();
    
    // Add our custom crowd fragments
    BuildContext.AddFragment<FCrowd_MovementFragment>();
    BuildContext.AddFragment<FCrowd_BehaviorFragment>();
    BuildContext.AddFragment<FCrowd_LODFragment>();
    BuildContext.AddFragment<FCrowd_VisualFragment>();
    
    // Configure movement fragment
    FCrowd_MovementFragment& MovementFragment = BuildContext.GetMutableFragment<FCrowd_MovementFragment>();
    MovementFragment.MaxSpeed = MovementSpeed;
    MovementFragment.Velocity = FVector::ZeroVector;
    MovementFragment.TargetLocation = FVector::ZeroVector;
    MovementFragment.bHasTarget = false;
    
    // Configure behavior fragment
    FCrowd_BehaviorFragment& BehaviorFragment = BuildContext.GetMutableFragment<FCrowd_BehaviorFragment>();
    BehaviorFragment.BehaviorState = 0; // Start idle
    BehaviorFragment.StateTimer = 0.0f;
    BehaviorFragment.GroupID = -1; // Unassigned
    BehaviorFragment.FlockingRadius = FlockingRadius;
    
    // Configure LOD fragment
    FCrowd_LODFragment& LODFragment = BuildContext.GetMutableFragment<FCrowd_LODFragment>();
    LODFragment.LODLevel = 0; // Start at high LOD
    LODFragment.DistanceToPlayer = 0.0f;
    LODFragment.bIsVisible = true;
    LODFragment.LastUpdateTime = 0.0f;
    
    // Configure visual fragment
    FCrowd_VisualFragment& VisualFragment = BuildContext.GetMutableFragment<FCrowd_VisualFragment>();
    VisualFragment.MeshVariant = FMath::RandRange(0, MeshVariants - 1);
    VisualFragment.TintColor = FLinearColor(
        FMath::RandRange(0.8f, 1.0f),
        FMath::RandRange(0.8f, 1.0f),
        FMath::RandRange(0.8f, 1.0f),
        1.0f
    );
    VisualFragment.Scale = FMath::RandRange(0.9f, 1.1f);
    VisualFragment.bCastShadow = bEnableShadowCasting;
    
    // Configure core Mass velocity fragment
    FMassVelocityFragment& VelocityFragment = BuildContext.GetMutableFragment<FMassVelocityFragment>();
    VelocityFragment.Value = FVector::ZeroVector;
    
    // Add tags for different crowd behaviors
    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
    {
        BuildContext.AddTag<FMassTag>(); // Could be used for "leader" entities
    }
}
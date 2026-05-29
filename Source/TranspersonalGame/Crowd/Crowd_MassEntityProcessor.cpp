#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "TransformFragment.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UCrowd_MassMovementProcessor::UCrowd_MassMovementProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
}

void UCrowd_MassMovementProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    EntityQuery.AddRequirement<FCrowd_BiomeFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const auto MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const auto BiomeList = Context.GetFragmentView<FCrowd_BiomeFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            const FCrowd_BiomeFragment& Biome = BiomeList[EntityIndex];

            // Simple wander behavior
            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector ToTarget = Movement.WanderTarget - CurrentLocation;
            
            // If close to target or no target, pick new wander target
            if (ToTarget.Size() < 50.0f || Movement.WanderTarget.IsZero())
            {
                float Angle = FMath::RandRange(0.0f, 2.0f * PI);
                float Distance = FMath::RandRange(100.0f, Movement.WanderRadius);
                Movement.WanderTarget = CurrentLocation + FVector(
                    FMath::Cos(Angle) * Distance,
                    FMath::Sin(Angle) * Distance,
                    0.0f
                );
            }

            // Calculate desired velocity
            FVector DesiredVelocity = ToTarget.GetSafeNormal() * Movement.MaxSpeed;
            
            // Apply biome constraints
            if (!Biome.bCanLeaveBiome)
            {
                // Keep entities within their biome bounds (simplified)
                FVector BiomeCenter = FVector::ZeroVector;
                if (Biome.BiomeType == 1) BiomeCenter = FVector(-45000, 40000, 0); // Forest
                else if (Biome.BiomeType == 2) BiomeCenter = FVector(50000, -40000, 0); // Desert
                
                float DistanceFromCenter = FVector::Dist(CurrentLocation, BiomeCenter);
                if (DistanceFromCenter > 20000.0f)
                {
                    FVector ToBiomeCenter = (BiomeCenter - CurrentLocation).GetSafeNormal();
                    DesiredVelocity = ToBiomeCenter * Movement.MaxSpeed;
                }
            }

            // Smooth velocity change
            Movement.Velocity = FMath::VInterpTo(Movement.Velocity, DesiredVelocity, DeltaTime, 2.0f);
            
            // Update position
            FVector NewLocation = CurrentLocation + Movement.Velocity * DeltaTime;
            NewLocation.Z = FMath::Max(NewLocation.Z, 0.0f); // Keep above ground
            
            Transform.GetMutableTransform().SetLocation(NewLocation);
        }
    });
}

UCrowd_MassLODProcessor::UCrowd_MassLODProcessor()
{
    bAutoRegisterWithProcessingPhases = true;
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
}

void UCrowd_MassLODProcessor::ConfigureQueries()
{
    EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    EntityQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get player location for distance calculations
    UWorld* World = EntityManager.GetWorld();
    if (!World) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();

    EntityQuery.ForEachEntityChunk(EntityManager, Context, [this, PlayerLocation](FMassExecutionContext& Context)
    {
        const auto TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();

        const int32 NumEntities = Context.GetNumEntities();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_LODFragment& LOD = LODList[EntityIndex];

            FVector EntityLocation = Transform.GetTransform().GetLocation();
            float Distance = FVector::Dist(PlayerLocation, EntityLocation);
            
            LOD.DistanceToPlayer = Distance;

            // Determine LOD level based on distance
            if (Distance < HighLODDistance)
            {
                LOD.CurrentLOD = 0; // High detail
                LOD.bIsVisible = true;
            }
            else if (Distance < MediumLODDistance)
            {
                LOD.CurrentLOD = 1; // Medium detail
                LOD.bIsVisible = true;
            }
            else if (Distance < LowLODDistance)
            {
                LOD.CurrentLOD = 2; // Low detail
                LOD.bIsVisible = true;
            }
            else if (Distance < CullDistance)
            {
                LOD.CurrentLOD = 3; // Very low detail
                LOD.bIsVisible = true;
            }
            else
            {
                LOD.CurrentLOD = 3; // Culled
                LOD.bIsVisible = false;
            }
        }
    });
}
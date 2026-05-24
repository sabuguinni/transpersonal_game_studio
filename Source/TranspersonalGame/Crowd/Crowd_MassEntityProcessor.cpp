#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// UCrowd_MassMovementProcessor Implementation
UCrowd_MassMovementProcessor::UCrowd_MassMovementProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowd_MassMovementProcessor::ConfigureQueries()
{
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowd_MassMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    MovementQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();
        const auto BehaviorList = Context.GetFragmentView<FCrowd_BehaviorFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        // Collect nearby entity positions for flocking calculations
        TArray<FVector> EntityLocations;
        TArray<FVector> EntityVelocities;
        EntityLocations.Reserve(NumEntities);
        EntityVelocities.Reserve(NumEntities);

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            EntityLocations.Add(TransformList[EntityIndex].GetTransform().GetLocation());
            EntityVelocities.Add(VelocityList[EntityIndex].Value);
        }

        // Process each entity
        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];
            const FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();
            FVector CurrentVelocity = Velocity.Value;

            // Skip if entity is fleeing (handled by behavior processor)
            if (Behavior.bIsFleeing)
            {
                continue;
            }

            // Calculate flocking forces
            FVector FlockingForce = FVector::ZeroVector;
            
            if (Movement.FlockingStrength > 0.0f)
            {
                // Find nearby entities for flocking
                TArray<FVector> NearbyLocations;
                TArray<FVector> NearbyVelocities;

                for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
                {
                    if (OtherIndex == EntityIndex) continue;

                    float Distance = FVector::Dist(CurrentLocation, EntityLocations[OtherIndex]);
                    if (Distance <= FlockingRadius)
                    {
                        NearbyLocations.Add(EntityLocations[OtherIndex]);
                        NearbyVelocities.Add(EntityVelocities[OtherIndex]);
                    }
                }

                // Apply flocking behaviors
                if (NearbyLocations.Num() > 0)
                {
                    FVector Separation = CalculateSeparation(CurrentLocation, NearbyLocations) * SeparationWeight;
                    FVector Alignment = CalculateAlignment(CurrentVelocity, NearbyVelocities) * AlignmentWeight;
                    FVector Cohesion = CalculateCohesion(CurrentLocation, NearbyLocations) * CohesionWeight;

                    FlockingForce = (Separation + Alignment + Cohesion) * Movement.FlockingStrength;
                }
            }

            // Wandering behavior
            FVector WanderForce = FVector::ZeroVector;
            float DistanceToTarget = FVector::Dist(CurrentLocation, Movement.TargetLocation);
            
            if (DistanceToTarget < 100.0f || Movement.TargetLocation.IsZero())
            {
                // Generate new random target within wander radius
                FVector RandomDirection = FMath::VRand();
                RandomDirection.Z = 0.0f; // Keep on ground level
                Movement.TargetLocation = CurrentLocation + (RandomDirection * Movement.WanderRadius);
            }

            FVector DirectionToTarget = (Movement.TargetLocation - CurrentLocation).GetSafeNormal();
            WanderForce = DirectionToTarget * Movement.MovementSpeed;

            // Combine forces
            FVector TotalForce = WanderForce + FlockingForce;
            TotalForce = TotalForce.GetClampedToMaxSize(Movement.MovementSpeed);

            // Apply movement
            Velocity.Value = FMath::VInterpTo(CurrentVelocity, TotalForce, DeltaTime, 2.0f);
            
            // Update position
            FVector NewLocation = CurrentLocation + (Velocity.Value * DeltaTime);
            NewLocation.Z = CurrentLocation.Z; // Maintain ground level

            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Update rotation to face movement direction
            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                NewRotation.Pitch = 0.0f; // Keep upright
                NewRotation.Roll = 0.0f;
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }
        }
    });
}

FVector UCrowd_MassMovementProcessor::CalculateSeparation(const FVector& EntityLocation, const TArray<FVector>& NearbyLocations)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NearbyLocation : NearbyLocations)
    {
        float Distance = FVector::Dist(EntityLocation, NearbyLocation);
        if (Distance > 0.0f && Distance < 150.0f) // Separation distance
        {
            FVector Diff = EntityLocation - NearbyLocation;
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

FVector UCrowd_MassMovementProcessor::CalculateAlignment(const FVector& EntityVelocity, const TArray<FVector>& NearbyVelocities)
{
    FVector AverageVelocity = FVector::ZeroVector;
    
    for (const FVector& NearbyVelocity : NearbyVelocities)
    {
        AverageVelocity += NearbyVelocity;
    }

    if (NearbyVelocities.Num() > 0)
    {
        AverageVelocity /= NearbyVelocities.Num();
        AverageVelocity.Normalize();
    }

    return AverageVelocity;
}

FVector UCrowd_MassMovementProcessor::CalculateCohesion(const FVector& EntityLocation, const TArray<FVector>& NearbyLocations)
{
    FVector CenterOfMass = FVector::ZeroVector;
    
    for (const FVector& NearbyLocation : NearbyLocations)
    {
        CenterOfMass += NearbyLocation;
    }

    if (NearbyLocations.Num() > 0)
    {
        CenterOfMass /= NearbyLocations.Num();
        FVector CohesionForce = CenterOfMass - EntityLocation;
        CohesionForce.Normalize();
        return CohesionForce;
    }

    return FVector::ZeroVector;
}

// UCrowd_MassBehaviorProcessor Implementation
UCrowd_MassBehaviorProcessor::UCrowd_MassBehaviorProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Behavior;
}

void UCrowd_MassBehaviorProcessor::ConfigureQueries()
{
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FCrowd_MovementFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassBehaviorProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    BehaviorQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const auto BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();
        const auto MovementList = Context.GetMutableFragmentView<FCrowd_MovementFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();
        UWorld* World = Context.GetWorld();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];
            FCrowd_MovementFragment& Movement = MovementList[EntityIndex];

            FVector CurrentLocation = Transform.GetTransform().GetLocation();

            // Update idle timer
            Behavior.IdleTimer += DeltaTime;

            // Detect threats
            bool bThreatDetected = DetectThreats(CurrentLocation, World);
            
            if (bThreatDetected)
            {
                Behavior.AlertLevel = FMath::Min(1.0f, Behavior.AlertLevel + (DeltaTime * 2.0f));
            }
            else
            {
                Behavior.AlertLevel = FMath::Max(0.0f, Behavior.AlertLevel - (DeltaTime * 0.5f));
            }

            // Check if should start fleeing
            if (Behavior.AlertLevel >= Behavior.FleeThreshold && !Behavior.bIsFleeing)
            {
                Behavior.bIsFleeing = true;
                // Set flee direction away from threat
                if (bThreatDetected)
                {
                    // For now, flee in random direction
                    FVector FleeDir = FMath::VRand();
                    FleeDir.Z = 0.0f;
                    FleeDir.Normalize();
                    Behavior.FleeDirection = FleeDir;
                }
            }

            // Handle fleeing behavior
            if (Behavior.bIsFleeing)
            {
                // Apply flee velocity
                FVector FleeVelocity = Behavior.FleeDirection * FleeSpeed;
                Velocity.Value = FleeVelocity;

                // Check if fled far enough
                // For simplicity, flee for a fixed time
                static float FleeTime = 0.0f;
                FleeTime += DeltaTime;
                if (FleeTime > 3.0f) // Flee for 3 seconds
                {
                    Behavior.bIsFleeing = false;
                    Behavior.AlertLevel = 0.0f;
                    FleeTime = 0.0f;
                }
            }

            // Random behavior changes
            if (Behavior.IdleTimer >= Behavior.MaxIdleTime)
            {
                // Change movement target randomly
                FVector RandomDirection = FMath::VRand();
                RandomDirection.Z = 0.0f;
                Movement.TargetLocation = CurrentLocation + (RandomDirection * Movement.WanderRadius);
                
                Behavior.IdleTimer = 0.0f;
                Behavior.MaxIdleTime = FMath::RandRange(3.0f, 8.0f);
            }
        }
    });
}

bool UCrowd_MassBehaviorProcessor::DetectThreats(const FVector& EntityLocation, UWorld* World)
{
    if (!World) return false;

    // Simple threat detection - look for player or large dinosaurs nearby
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        float DistanceToPlayer = FVector::Dist(EntityLocation, PlayerPawn->GetActorLocation());
        if (DistanceToPlayer < ThreatDetectionRadius)
        {
            return true;
        }
    }

    // Could add detection for dinosaurs or other threats here
    return false;
}

FVector UCrowd_MassBehaviorProcessor::CalculateFleeDirection(const FVector& EntityLocation, const FVector& ThreatLocation)
{
    FVector FleeDirection = EntityLocation - ThreatLocation;
    FleeDirection.Z = 0.0f; // Keep on ground
    FleeDirection.Normalize();
    return FleeDirection;
}

// UCrowd_MassLODProcessor Implementation
UCrowd_MassLODProcessor::UCrowd_MassLODProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::LOD;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Movement);
}

void UCrowd_MassLODProcessor::ConfigureQueries()
{
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FCrowd_LODFragment>(EMassFragmentAccess::ReadWrite);
    LODQuery.AddRequirement<FMassRepresentationFragment>(EMassFragmentAccess::ReadWrite);
}

void UCrowd_MassLODProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    LODQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto TransformList = Context.GetFragmentView<FTransformFragment>();
        const auto LODList = Context.GetMutableFragmentView<FCrowd_LODFragment>();
        const auto RepresentationList = Context.GetMutableFragmentView<FMassRepresentationFragment>();

        const int32 NumEntities = Context.GetNumEntities();
        UWorld* World = Context.GetWorld();
        FVector PlayerLocation = GetPlayerLocation(World);

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FCrowd_LODFragment& LOD = LODList[EntityIndex];
            FMassRepresentationFragment& Representation = RepresentationList[EntityIndex];

            FVector EntityLocation = Transform.GetTransform().GetLocation();
            float DistanceToPlayer = FVector::Dist(EntityLocation, PlayerLocation);
            
            LOD.DistanceToPlayer = DistanceToPlayer;
            
            // Calculate LOD level
            int32 NewLODLevel = CalculateLODLevel(DistanceToPlayer);
            
            if (NewLODLevel != LOD.LODLevel)
            {
                LOD.LODLevel = NewLODLevel;
                
                // Adjust update frequency based on LOD
                switch (NewLODLevel)
                {
                    case 0: // High detail
                        LOD.UpdateFrequency = 1.0f;
                        LOD.bIsVisible = true;
                        break;
                    case 1: // Medium detail
                        LOD.UpdateFrequency = 0.5f;
                        LOD.bIsVisible = true;
                        break;
                    case 2: // Low detail
                        LOD.UpdateFrequency = 0.25f;
                        LOD.bIsVisible = true;
                        break;
                    default: // Culled
                        LOD.UpdateFrequency = 0.1f;
                        LOD.bIsVisible = false;
                        break;
                }
                
                // Update representation LOD
                Representation.CurrentRepresentation = static_cast<EMassRepresentationType>(NewLODLevel);
            }
        }
    });
}

int32 UCrowd_MassLODProcessor::CalculateLODLevel(float DistanceToPlayer)
{
    if (DistanceToPlayer <= LOD0Distance)
    {
        return 0; // High detail
    }
    else if (DistanceToPlayer <= LOD1Distance)
    {
        return 1; // Medium detail
    }
    else if (DistanceToPlayer <= LOD2Distance)
    {
        return 2; // Low detail
    }
    else if (DistanceToPlayer <= CullingDistance)
    {
        return 3; // Very low detail
    }
    else
    {
        return 4; // Culled
    }
}

FVector UCrowd_MassLODProcessor::GetPlayerLocation(UWorld* World)
{
    if (!World) return FVector::ZeroVector;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (PlayerPawn)
    {
        return PlayerPawn->GetActorLocation();
    }

    return FVector::ZeroVector;
}
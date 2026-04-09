#include "CrowdMovementProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassExecutionContext.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"

UCrowdMovementProcessor::UCrowdMovementProcessor()
{
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Avoidance);
}

void UCrowdMovementProcessor::ConfigureQueries()
{
    // Configure main crowd movement query
    CrowdMovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    CrowdMovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    CrowdMovementQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
    CrowdMovementQuery.AddRequirement<FCrowdAgentFragment>(EMassFragmentAccess::ReadWrite);
    CrowdMovementQuery.AddChunkRequirement<FMassSimulationLODFragment>(EMassFragmentAccess::ReadOnly);
    
    // Configure nearby entities query for flocking
    NearbyEntitiesQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    NearbyEntitiesQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
    NearbyEntitiesQuery.AddRequirement<FCrowdAgentFragment>(EMassFragmentAccess::ReadOnly);
}

void UCrowdMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get world for timing
    UWorld* World = EntityManager.GetWorld();
    if (!World)
    {
        return;
    }
    
    const float DeltaTime = World->GetDeltaSeconds();
    
    // Process crowd movement
    CrowdMovementQuery.ForEachEntityChunk(EntityManager, Context, 
        [this, DeltaTime](FMassExecutionContext& Context)
        {
            const int32 NumEntities = Context.GetNumEntities();
            const auto& TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const auto& VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const auto& MoveTargetList = Context.GetFragmentView<FMassMoveTargetFragment>();
            const auto& CrowdAgentList = Context.GetMutableFragmentView<FCrowdAgentFragment>();
            const auto& LODList = Context.GetChunkFragmentView<FMassSimulationLODFragment>();
            
            // Skip if LOD is too low for detailed simulation
            if (LODList.Num() > 0 && LODList[0].LOD > EMassLOD::Medium)
            {
                return;
            }
            
            for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
            {
                FTransformFragment& Transform = TransformList[EntityIndex];
                FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
                const FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];
                FCrowdAgentFragment& CrowdAgent = CrowdAgentList[EntityIndex];
                
                const FMassEntityHandle EntityHandle = Context.GetEntity(EntityIndex);
                
                // Calculate steering forces
                FVector SteeringForce = FVector::ZeroVector;
                
                // Basic movement toward target
                FVector DesiredVelocity = (MoveTarget.Center - Transform.GetTransform().GetLocation()).GetSafeNormal() * CrowdAgent.PreferredSpeed;
                FVector SeekForce = (DesiredVelocity - Velocity.Value).GetClampedToMaxSize(MaxForce);
                SteeringForce += SeekForce;
                
                // Add flocking forces
                FVector FlockingForce = CalculateFlockingForce(EntityHandle, Transform, CrowdAgent, Context);
                SteeringForce += FlockingForce;
                
                // Emergency behavior
                if (CrowdAgent.BehaviorState == FCrowdAgentFragment::ECrowdBehaviorState::Fleeing)
                {
                    FVector FleeForce = CalculateFleeForce(Transform.GetTransform().GetLocation(), 
                                                           CrowdAgent.LastKnownThreatLocation, 1000.0f);
                    SteeringForce += FleeForce * FleeWeight;
                    
                    // Increase speed when panicking
                    CrowdAgent.PreferredSpeed *= (1.0f + CrowdAgent.PanicLevel * PanicSpeedMultiplier);
                }
                
                // Apply forces
                Velocity.Value += SteeringForce * DeltaTime;
                Velocity.Value = Velocity.Value.GetClampedToMaxSize(MaxSpeed * (1.0f + CrowdAgent.PanicLevel));
                
                // Update position
                FVector NewLocation = Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime;
                Transform.GetMutableTransform().SetLocation(NewLocation);
                
                // Update rotation to face movement direction
                if (!Velocity.Value.IsNearlyZero())
                {
                    FRotator NewRotation = Velocity.Value.Rotation();
                    Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
                }
                
                // Decay panic over time
                if (CrowdAgent.PanicLevel > 0.0f)
                {
                    CrowdAgent.PanicLevel = FMath::Max(0.0f, CrowdAgent.PanicLevel - PanicDecayRate * DeltaTime);
                    
                    if (CrowdAgent.PanicLevel <= 0.1f)
                    {
                        CrowdAgent.BehaviorState = FCrowdAgentFragment::ECrowdBehaviorState::Wandering;
                    }
                }
            }
        });
}

FVector UCrowdMovementProcessor::CalculateFlockingForce(const FMassEntityHandle& Entity, const FTransformFragment& Transform, 
                                                        const FCrowdAgentFragment& CrowdAgent, FMassExecutionContext& Context)
{
    // In a full implementation, this would:
    // 1. Query nearby entities within flocking radius
    // 2. Calculate separation, alignment, and cohesion forces
    // 3. Weight and combine the forces
    
    FVector FlockingForce = FVector::ZeroVector;
    const FVector CurrentPosition = Transform.GetTransform().GetLocation();
    
    // Simplified flocking - would need spatial partitioning for performance
    TArray<FMassEntityHandle> NearbyEntities;
    
    // Calculate separation (avoid crowding)
    FVector SeparationForce = CalculateSeparationForce(CurrentPosition, CrowdAgent, NearbyEntities, Context);
    FlockingForce += SeparationForce * SeparationWeight;
    
    // Calculate alignment (steer towards average heading)
    FVector AlignmentForce = CalculateAlignmentForce(FVector::ZeroVector, NearbyEntities, Context);
    FlockingForce += AlignmentForce * AlignmentWeight;
    
    // Calculate cohesion (steer towards average position)
    FVector CohesionForce = CalculateCohesionForce(CurrentPosition, NearbyEntities, Context);
    FlockingForce += CohesionForce * CohesionWeight;
    
    return FlockingForce.GetClampedToMaxSize(MaxForce);
}

FVector UCrowdMovementProcessor::CalculateSeparationForce(const FVector& Position, const FCrowdAgentFragment& CrowdAgent, 
                                                          const TArray<FMassEntityHandle>& NearbyEntities, FMassExecutionContext& Context)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    // In a full implementation, iterate through nearby entities
    // and calculate repulsion forces based on distance
    
    for (const FMassEntityHandle& NearbyEntity : NearbyEntities)
    {
        // Calculate separation force from this neighbor
        // SeparationForce += (Position - NeighborPosition).GetSafeNormal() / Distance;
        Count++;
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * CrowdAgent.PreferredSpeed;
    }
    
    return SeparationForce;
}

FVector UCrowdMovementProcessor::CalculateAlignmentForce(const FVector& Velocity, const TArray<FMassEntityHandle>& NearbyEntities, 
                                                         FMassExecutionContext& Context)
{
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;
    
    // In a full implementation, average the velocities of nearby entities
    for (const FMassEntityHandle& NearbyEntity : NearbyEntities)
    {
        // AverageVelocity += NeighborVelocity;
        Count++;
    }
    
    if (Count > 0)
    {
        AverageVelocity /= Count;
        return (AverageVelocity - Velocity).GetClampedToMaxSize(MaxForce);
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateCohesionForce(const FVector& Position, const TArray<FMassEntityHandle>& NearbyEntities, 
                                                        FMassExecutionContext& Context)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    // In a full implementation, calculate center of mass of nearby entities
    for (const FMassEntityHandle& NearbyEntity : NearbyEntities)
    {
        // CenterOfMass += NeighborPosition;
        Count++;
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        FVector DesiredVelocity = (CenterOfMass - Position).GetSafeNormal() * MaxSpeed;
        return DesiredVelocity.GetClampedToMaxSize(MaxForce);
    }
    
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateAvoidanceForce(const FVector& Position, const FVector& Velocity, 
                                                         const FCrowdAgentFragment& CrowdAgent, FMassExecutionContext& Context)
{
    // Obstacle avoidance using raycasting
    // In a full implementation, this would cast rays ahead and calculate avoidance forces
    return FVector::ZeroVector;
}

FVector UCrowdMovementProcessor::CalculateFleeForce(const FVector& Position, const FVector& ThreatLocation, float ThreatRadius)
{
    FVector FleeDirection = (Position - ThreatLocation).GetSafeNormal();
    float Distance = FVector::Dist(Position, ThreatLocation);
    
    if (Distance < ThreatRadius)
    {
        float FleeStrength = 1.0f - (Distance / ThreatRadius);
        return FleeDirection * FleeStrength * MaxForce;
    }
    
    return FVector::ZeroVector;
}

void UCrowdMovementProcessor::UpdatePanicLevel(FCrowdAgentFragment& CrowdAgent, const FVector& Position, 
                                               const FVector& ThreatLocation, float ThreatRadius, float DeltaTime)
{
    float Distance = FVector::Dist(Position, ThreatLocation);
    
    if (Distance < ThreatRadius)
    {
        float ThreatIntensity = 1.0f - (Distance / ThreatRadius);
        CrowdAgent.PanicLevel = FMath::Min(1.0f, CrowdAgent.PanicLevel + ThreatIntensity * DeltaTime);
        CrowdAgent.BehaviorState = FCrowdAgentFragment::ECrowdBehaviorState::Fleeing;
        CrowdAgent.LastKnownThreatLocation = ThreatLocation;
    }
}
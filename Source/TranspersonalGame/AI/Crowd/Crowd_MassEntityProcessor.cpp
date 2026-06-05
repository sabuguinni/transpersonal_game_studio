#include "Crowd_MassEntityProcessor.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_MassEntityProcessor::UCrowd_MassEntityProcessor()
{
    ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
    ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Movement;
    ExecutionOrder.ExecuteAfter.Add(UE::Mass::ProcessorGroupNames::Behavior);
}

void UCrowd_MassEntityProcessor::ConfigureQueries()
{
    // Movement query - processes agent movement and velocity
    MovementQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    MovementQuery.RegisterWithProcessor(*this);

    // Behavior query - processes flocking and social behaviors
    BehaviorQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
    BehaviorQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    BehaviorQuery.RegisterWithProcessor(*this);

    // LOD query - manages level of detail for performance
    LODQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    LODQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    LODQuery.RegisterWithProcessor(*this);

    // Pathfinding query - handles navigation and obstacle avoidance
    PathfindingQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    PathfindingQuery.AddRequirement<FCrowd_AgentFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.AddRequirement<FCrowd_BehaviorFragment>(EMassFragmentAccess::ReadWrite);
    PathfindingQuery.RegisterWithProcessor(*this);
}

void UCrowd_MassEntityProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Reset frame counters
    ProcessedAgentsThisFrame = 0;
    AccumulatedProcessingTime = 0.0f;

    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    // Process in order of importance for performance
    ProcessLOD(EntityManager, Context);
    ProcessBehavior(EntityManager, Context);
    ProcessPathfinding(EntityManager, Context);
    ProcessMovement(EntityManager, Context);

    // Debug output for performance monitoring
    if (ProcessedAgentsThisFrame > 0)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Crowd Processor: %d agents processed in %.3fms"), 
               ProcessedAgentsThisFrame, AccumulatedProcessingTime * 1000.0f);
    }
}

void UCrowd_MassEntityProcessor::ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();
    
    MovementQuery.ForEachEntityChunk(EntityManager, Context, 
        [this, DeltaTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
            const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
            const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();

            for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
            {
                if (!ShouldProcessAgent(EntityIndex, DeltaTime))
                    continue;

                FTransformFragment& Transform = TransformList[EntityIndex];
                FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
                FCrowd_AgentFragment& Agent = AgentList[EntityIndex];

                // Apply movement based on current velocity
                const FVector NewLocation = Transform.GetTransform().GetLocation() + (Velocity.Value * DeltaTime);
                Transform.GetMutableTransform().SetLocation(NewLocation);

                // Update agent's current velocity for behavior calculations
                Agent.CurrentVelocity = Velocity.Value;

                // Clamp velocity to max speed
                if (Velocity.Value.SizeSquared() > FMath::Square(MaxSpeed))
                {
                    Velocity.Value = Velocity.Value.GetSafeNormal() * MaxSpeed;
                }

                ProcessedAgentsThisFrame++;
            }
        });
}

void UCrowd_MassEntityProcessor::ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    const float DeltaTime = Context.GetDeltaTimeSeconds();

    BehaviorQuery.ForEachEntityChunk(EntityManager, Context,
        [this, DeltaTime](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
            const TArrayView<FMassVelocityFragment> VelocityList = Context.GetFragmentView<FMassVelocityFragment>();
            const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();
            const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();

            // Collect neighbor data for flocking calculations
            TArray<FVector> Positions;
            TArray<FVector> Velocities;
            Positions.Reserve(Context.GetNumEntities());
            Velocities.Reserve(Context.GetNumEntities());

            for (int32 i = 0; i < Context.GetNumEntities(); ++i)
            {
                Positions.Add(TransformList[i].GetTransform().GetLocation());
                Velocities.Add(VelocityList[i].Value);
            }

            for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
            {
                FCrowd_AgentFragment& Agent = AgentList[EntityIndex];
                FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

                if (Agent.LODLevel > 2) // Skip high LOD agents for behavior processing
                    continue;

                const FVector CurrentPos = Positions[EntityIndex];
                const FVector CurrentVel = Velocities[EntityIndex];

                // Calculate flocking force
                FVector FlockingForce = CalculateFlockingForce(CurrentPos, CurrentVel, Positions, Velocities);

                // Apply behavior weights
                FlockingForce *= (Behavior.SeparationWeight + Behavior.AlignmentWeight + Behavior.CohesionWeight);

                // Add seeking behavior toward target
                if (!Agent.TargetLocation.IsZero())
                {
                    FVector SeekForce = (Agent.TargetLocation - CurrentPos).GetSafeNormal() * MaxForce;
                    FlockingForce += SeekForce * Behavior.SeekWeight;
                }

                // Update agent's target velocity
                FVector DesiredVelocity = CurrentVel + (FlockingForce * DeltaTime);
                DesiredVelocity = DesiredVelocity.GetClampedToMaxSize(Agent.MovementSpeed);

                // Update behavior state timer
                Behavior.CurrentStress = FMath::Clamp(Behavior.CurrentStress + (FlockingForce.Size() * 0.001f * DeltaTime), 0.0f, 1.0f);
                
                // State transitions based on stress
                if (Behavior.CurrentStress > Behavior.PanicThreshold)
                {
                    Agent.BehaviorState = ECrowd_BehaviorState::Panicking;
                    Agent.MovementSpeed = MaxSpeed * 1.5f; // Panic speed boost
                }
                else if (Behavior.CurrentStress < 0.3f)
                {
                    Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
                    Agent.MovementSpeed = MaxSpeed * 0.7f; // Calm wandering
                }
            }
        });
}

void UCrowd_MassEntityProcessor::ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Get viewer position (camera/player location)
    FVector ViewerPosition = FVector::ZeroVector;
    if (UWorld* World = EntityManager.GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                ViewerPosition = Pawn->GetActorLocation();
            }
        }
    }

    LODQuery.ForEachEntityChunk(EntityManager, Context,
        [this, ViewerPosition](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
            const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();

            for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
            {
                FCrowd_AgentFragment& Agent = AgentList[EntityIndex];
                const FVector AgentPosition = TransformList[EntityIndex].GetTransform().GetLocation();

                // Calculate distance to viewer
                const float Distance = FVector::Dist(AgentPosition, ViewerPosition);
                Agent.DistanceToViewer = Distance;

                // Determine LOD level based on distance
                Agent.LODLevel = static_cast<int32>(CalculateLODLevel(AgentPosition, ViewerPosition));
            }
        });
}

void UCrowd_MassEntityProcessor::ProcessPathfinding(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    PathfindingQuery.ForEachEntityChunk(EntityManager, Context,
        [this](FMassExecutionContext& Context)
        {
            const TArrayView<FTransformFragment> TransformList = Context.GetFragmentView<FTransformFragment>();
            const TArrayView<FCrowd_AgentFragment> AgentList = Context.GetMutableFragmentView<FCrowd_AgentFragment>();
            const TArrayView<FCrowd_BehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FCrowd_BehaviorFragment>();

            for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
            {
                FCrowd_AgentFragment& Agent = AgentList[EntityIndex];
                FCrowd_BehaviorFragment& Behavior = BehaviorList[EntityIndex];

                if (Agent.LODLevel > 1) // Only process pathfinding for close agents
                    continue;

                const FVector CurrentPos = TransformList[EntityIndex].GetTransform().GetLocation();

                // Update pathfinding if we have a valid path
                if (Behavior.bHasValidPath && Behavior.PathPoints.Num() > 0)
                {
                    // Check if we've reached current path point
                    if (Behavior.CurrentPathIndex < Behavior.PathPoints.Num())
                    {
                        const FVector TargetPoint = Behavior.PathPoints[Behavior.CurrentPathIndex];
                        const float DistanceToTarget = FVector::Dist(CurrentPos, TargetPoint);

                        if (DistanceToTarget < 100.0f) // Reached waypoint
                        {
                            Behavior.CurrentPathIndex++;
                            if (Behavior.CurrentPathIndex >= Behavior.PathPoints.Num())
                            {
                                // Path completed
                                Behavior.bHasValidPath = false;
                                Agent.BehaviorState = ECrowd_BehaviorState::Wandering;
                            }
                        }
                        else
                        {
                            // Update target location to current path point
                            Agent.TargetLocation = TargetPoint;
                        }
                    }
                }
                else
                {
                    // Generate new random target for wandering
                    if (Agent.BehaviorState == ECrowd_BehaviorState::Wandering)
                    {
                        const float WanderRadius = 1000.0f;
                        const FVector RandomOffset = FVector(
                            FMath::RandRange(-WanderRadius, WanderRadius),
                            FMath::RandRange(-WanderRadius, WanderRadius),
                            0.0f
                        );
                        Agent.TargetLocation = CurrentPos + RandomOffset;
                    }
                }
            }
        });
}

FVector UCrowd_MassEntityProcessor::CalculateFlockingForce(const FVector& Position, const FVector& Velocity,
    const TArray<FVector>& Neighbors, const TArray<FVector>& NeighborVelocities)
{
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    
    int32 SeparationCount = 0;
    int32 AlignmentCount = 0;
    int32 CohesionCount = 0;

    for (int32 i = 0; i < Neighbors.Num(); ++i)
    {
        const float Distance = FVector::Dist(Position, Neighbors[i]);
        
        if (Distance > 0.0f && Distance < SeparationRadius)
        {
            // Separation - steer away from neighbors
            FVector Diff = (Position - Neighbors[i]).GetSafeNormal();
            Diff /= Distance; // Weight by distance
            Separation += Diff;
            SeparationCount++;
        }
        
        if (Distance > 0.0f && Distance < AlignmentRadius)
        {
            // Alignment - steer towards average heading of neighbors
            Alignment += NeighborVelocities[i];
            AlignmentCount++;
        }
        
        if (Distance > 0.0f && Distance < CohesionRadius)
        {
            // Cohesion - steer towards average position of neighbors
            Cohesion += Neighbors[i];
            CohesionCount++;
        }
    }

    // Average and normalize forces
    if (SeparationCount > 0)
    {
        Separation /= SeparationCount;
        Separation = Separation.GetSafeNormal() * MaxForce;
    }
    
    if (AlignmentCount > 0)
    {
        Alignment /= AlignmentCount;
        Alignment = Alignment.GetSafeNormal() * MaxForce;
    }
    
    if (CohesionCount > 0)
    {
        Cohesion /= CohesionCount;
        Cohesion = (Cohesion - Position).GetSafeNormal() * MaxForce;
    }

    return Separation + Alignment + Cohesion;
}

float UCrowd_MassEntityProcessor::CalculateLODLevel(const FVector& AgentPosition, const FVector& ViewerPosition)
{
    const float Distance = FVector::Dist(AgentPosition, ViewerPosition);
    
    // LOD thresholds
    const float LOD0_Distance = 500.0f;   // Full detail
    const float LOD1_Distance = 1500.0f;  // Reduced detail
    const float LOD2_Distance = 3000.0f;  // Minimal detail
    const float LOD3_Distance = 5000.0f;  // Culled
    
    if (Distance < LOD0_Distance) return 0.0f;
    if (Distance < LOD1_Distance) return 1.0f;
    if (Distance < LOD2_Distance) return 2.0f;
    if (Distance < LOD3_Distance) return 3.0f;
    return 4.0f; // Completely culled
}

bool UCrowd_MassEntityProcessor::ShouldProcessAgent(int32 AgentIndex, float DeltaTime)
{
    // Implement time-slicing to maintain performance
    if (ProcessedAgentsThisFrame >= MaxAgentsPerFrame)
        return false;
        
    if (AccumulatedProcessingTime >= ProcessingTimeSlice)
        return false;
        
    return true;
}
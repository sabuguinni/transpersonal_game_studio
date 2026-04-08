#include "DinosaurMassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "DinosaurMassFragments.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"

UDinosaurMassProcessor::UDinosaurMassProcessor()
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UDinosaurMassProcessor::ConfigureQueries()
{
    // Configure herbivore herd query
    HerbivoreQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    HerbivoreQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    HerbivoreQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadOnly);
    HerbivoreQuery.AddTagRequirement<FDinosaurHerbivoreTag>(EMassFragmentPresence::All);

    // Configure carnivore query
    CarnivoreQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    CarnivoreQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    CarnivoreQuery.AddTagRequirement<FDinosaurCarnivoreTag>(EMassFragmentPresence::All);

    // Configure solitary query
    SolitaryQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
    SolitaryQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    SolitaryQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    SolitaryQuery.AddRequirement<FDinosaurBehaviorFragment>(EMassFragmentAccess::ReadWrite);
    SolitaryQuery.AddTagRequirement<FDinosaurSolitaryTag>(EMassFragmentPresence::All);
}

void UDinosaurMassProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    // Process different dinosaur types
    ProcessHerbivoreHerds(EntityManager, Context);
    ProcessCarnivores(EntityManager, Context);
    ProcessSolitaryDinosaurs(EntityManager, Context);
}

void UDinosaurMassProcessor::ProcessHerbivoreHerds(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    HerbivoreQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
        const TArrayView<FDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();
        const TArrayView<FDinosaurHerdFragment> HerdList = Context.GetFragmentView<FDinosaurHerdFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];
            const FDinosaurHerdFragment& Herd = HerdList[EntityIndex];

            // Gather nearby herd members
            TArray<FVector> NeighborPositions;
            TArray<FVector> NeighborVelocities;
            
            for (int32 OtherIndex = 0; OtherIndex < NumEntities; ++OtherIndex)
            {
                if (EntityIndex == OtherIndex) continue;
                
                const FDinosaurHerdFragment& OtherHerd = HerdList[OtherIndex];
                if (Herd.HerdID != OtherHerd.HerdID) continue;
                
                const FVector& OtherPosition = TransformList[OtherIndex].GetTransform().GetLocation();
                const float Distance = FVector::Dist(Transform.GetTransform().GetLocation(), OtherPosition);
                
                if (Distance < CohesionRadius)
                {
                    NeighborPositions.Add(OtherPosition);
                    NeighborVelocities.Add(VelocityList[OtherIndex].Value);
                }
            }

            // Calculate flocking forces
            FVector Separation = CalculateSeparation(Transform.GetTransform().GetLocation(), NeighborPositions) * SeparationWeight;
            FVector Alignment = CalculateAlignment(Velocity.Value, NeighborVelocities) * AlignmentWeight;
            FVector Cohesion = CalculateCohesion(Transform.GetTransform().GetLocation(), NeighborPositions) * CohesionWeight;
            FVector Wander = CalculateWander(Velocity.Value, DeltaTime) * WanderWeight;

            // Apply forces
            FVector TotalForce = Separation + Alignment + Cohesion + Wander;
            TotalForce = TotalForce.GetClampedToMaxSize(MaxForce);

            // Update velocity
            Velocity.Value += TotalForce * DeltaTime;
            Velocity.Value = Velocity.Value.GetClampedToMaxSize(MaxSpeed * Species.SpeedMultiplier);

            // Update position
            FVector NewLocation = Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime;
            Transform.GetMutableTransform().SetLocation(NewLocation);

            // Update facing direction
            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }

            // Update behavior state based on herd dynamics
            if (NeighborPositions.Num() > 5)
            {
                Behavior.CurrentState = EDinosaurBehaviorState::Grazing;
            }
            else if (NeighborPositions.Num() > 0)
            {
                Behavior.CurrentState = EDinosaurBehaviorState::Moving;
            }
            else
            {
                Behavior.CurrentState = EDinosaurBehaviorState::Searching;
            }
        }
    });
}

void UDinosaurMassProcessor::ProcessCarnivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    CarnivoreQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
        const TArrayView<FDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];

            // Carnivores have different behavior - more territorial, hunt in packs or alone
            FVector WanderForce = CalculateWander(Velocity.Value, DeltaTime);
            
            // Apply hunting behavior modifications here
            // TODO: Add prey detection and pursuit logic
            
            Velocity.Value += WanderForce * DeltaTime;
            Velocity.Value = Velocity.Value.GetClampedToMaxSize(MaxSpeed * Species.SpeedMultiplier);

            FVector NewLocation = Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime;
            Transform.GetMutableTransform().SetLocation(NewLocation);

            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }

            Behavior.CurrentState = EDinosaurBehaviorState::Hunting;
        }
    });
}

void UDinosaurMassProcessor::ProcessSolitaryDinosaurs(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    SolitaryQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const TArrayView<FTransformFragment> TransformList = Context.GetMutableFragmentView<FTransformFragment>();
        const TArrayView<FMassVelocityFragment> VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        const TArrayView<FDinosaurSpeciesFragment> SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
        const TArrayView<FDinosaurBehaviorFragment> BehaviorList = Context.GetMutableFragmentView<FDinosaurBehaviorFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();
        const int32 NumEntities = Context.GetNumEntities();

        for (int32 EntityIndex = 0; EntityIndex < NumEntities; ++EntityIndex)
        {
            FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FDinosaurBehaviorFragment& Behavior = BehaviorList[EntityIndex];

            // Solitary dinosaurs wander independently
            FVector WanderForce = CalculateWander(Velocity.Value, DeltaTime);
            
            Velocity.Value += WanderForce * DeltaTime;
            Velocity.Value = Velocity.Value.GetClampedToMaxSize(MaxSpeed * Species.SpeedMultiplier);

            FVector NewLocation = Transform.GetTransform().GetLocation() + Velocity.Value * DeltaTime;
            Transform.GetMutableTransform().SetLocation(NewLocation);

            if (!Velocity.Value.IsNearlyZero())
            {
                FRotator NewRotation = Velocity.Value.Rotation();
                Transform.GetMutableTransform().SetRotation(NewRotation.Quaternion());
            }

            Behavior.CurrentState = EDinosaurBehaviorState::Wandering;
        }
    });
}

FVector UDinosaurMassProcessor::CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborPos : NeighborPositions)
    {
        float Distance = FVector::Dist(Position, NeighborPos);
        if (Distance > 0 && Distance < SeparationRadius)
        {
            FVector Diff = Position - NeighborPos;
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
        SeparationForce *= MaxSpeed;
    }

    return SeparationForce;
}

FVector UDinosaurMassProcessor::CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities)
{
    FVector Sum = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborVel : NeighborVelocities)
    {
        Sum += NeighborVel;
        Count++;
    }

    if (Count > 0)
    {
        Sum /= Count;
        Sum.Normalize();
        Sum *= MaxSpeed;
        return Sum - Velocity;
    }

    return FVector::ZeroVector;
}

FVector UDinosaurMassProcessor::CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions)
{
    FVector Sum = FVector::ZeroVector;
    int32 Count = 0;

    for (const FVector& NeighborPos : NeighborPositions)
    {
        Sum += NeighborPos;
        Count++;
    }

    if (Count > 0)
    {
        Sum /= Count;
        FVector Desired = Sum - Position;
        Desired.Normalize();
        Desired *= MaxSpeed;
        return Desired;
    }

    return FVector::ZeroVector;
}

FVector UDinosaurMassProcessor::CalculateWander(const FVector& CurrentVelocity, float DeltaTime)
{
    // Simple wander behavior - add some randomness to movement
    FVector WanderForce = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    );
    
    WanderForce.Normalize();
    return WanderForce * MaxForce * 0.1f; // Reduced wander influence
}
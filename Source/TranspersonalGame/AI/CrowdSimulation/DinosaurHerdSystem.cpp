#include "DinosaurHerdSystem.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

// ===== UDinosaurHerdProcessor =====

UDinosaurHerdProcessor::UDinosaurHerdProcessor()
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UDinosaurHerdProcessor::ConfigureQueries()
{
    HerdQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    HerdQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadWrite);
    HerdQuery.AddRequirement<FDinosaurHerdFragment>(EMassFragmentAccess::ReadWrite);
    HerdQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    HerdQuery.AddRequirement<FMassForceFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurHerdProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    HerdQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        auto& VelocityList = Context.GetMutableFragmentView<FMassVelocityFragment>();
        auto& HerdList = Context.GetMutableFragmentView<FDinosaurHerdFragment>();
        const auto& SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
        auto& ForceList = Context.GetMutableFragmentView<FMassForceFragment>();

        const float DeltaTime = Context.GetDeltaTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FMassVelocityFragment& Velocity = VelocityList[EntityIndex];
            FDinosaurHerdFragment& HerdData = HerdList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassForceFragment& Force = ForceList[EntityIndex];

            // Update panic state
            if (HerdData.bInPanic)
            {
                HerdData.CurrentPanicTime += DeltaTime;
                if (HerdData.CurrentPanicTime >= HerdData.PanicDuration)
                {
                    HerdData.bInPanic = false;
                    HerdData.CurrentPanicTime = 0.0f;
                }
            }

            // Check for predators if not already in panic
            if (!HerdData.bInPanic)
            {
                if (DetectPredatorNearby(Context, Transform, HerdData.PanicRadius))
                {
                    HerdData.bInPanic = true;
                    HerdData.CurrentPanicTime = 0.0f;
                }
            }

            // Calculate herd forces
            FVector HerdForce = FVector::ZeroVector;

            if (HerdData.bInPanic)
            {
                // Panic behavior - flee from threat
                ProcessPanicBehavior(Context, HerdData, Velocity);
            }
            else
            {
                // Normal herd behavior
                FVector Cohesion = CalculateCohesion(Context, Transform, HerdData);
                FVector Separation = CalculateSeparation(Context, Transform, HerdData);
                FVector Alignment = CalculateAlignment(Context, Transform, HerdData);

                // Weight the forces
                HerdForce = Cohesion * 0.3f + Separation * 0.5f + Alignment * 0.2f;

                // Leaders have reduced cohesion
                if (HerdData.bIsHerdLeader)
                {
                    HerdForce = Cohesion * 0.1f + Separation * 0.6f + Alignment * 0.3f;
                }
            }

            // Apply speed limits
            float CurrentSpeed = Velocity.Value.Size();
            float MaxSpeed = HerdData.bInPanic ? Species.MaxSpeed : Species.PreferredSpeed;

            if (CurrentSpeed > MaxSpeed)
            {
                Velocity.Value = Velocity.Value.GetSafeNormal() * MaxSpeed;
            }

            // Add herd force to total force
            Force.Value += HerdForce;
        }
    });
}

FVector UDinosaurHerdProcessor::CalculateCohesion(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 NeighborCount = 0;

    // This is a simplified version - in a full implementation, we'd use spatial queries
    // to find nearby herd members more efficiently
    
    // For now, return a small force towards a general herd center
    // In practice, this would query nearby entities with the same HerdID
    return FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal() * 100.0f;
}

FVector UDinosaurHerdProcessor::CalculateSeparation(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData)
{
    FVector SeparationForce = FVector::ZeroVector;
    
    // Simplified separation - push away from overcrowded areas
    // In practice, this would query nearby entities and calculate repulsion forces
    return FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal() * 200.0f;
}

FVector UDinosaurHerdProcessor::CalculateAlignment(const FMassExecutionContext& Context, const FTransformFragment& Transform, const FDinosaurHerdFragment& HerdData)
{
    FVector AverageVelocity = FVector::ZeroVector;
    
    // Simplified alignment - align with general movement direction
    // In practice, this would average the velocities of nearby herd members
    return FVector(1.0f, 0.0f, 0.0f) * 50.0f;
}

void UDinosaurHerdProcessor::ProcessPanicBehavior(FMassExecutionContext& Context, FDinosaurHerdFragment& HerdData, FMassVelocityFragment& Velocity)
{
    // In panic, dinosaurs flee in random directions at maximum speed
    if (Velocity.Value.SizeSquared() < 100.0f) // If nearly stationary
    {
        // Pick a random flee direction
        FVector FleeDirection = FVector(FMath::RandRange(-1.0f, 1.0f), FMath::RandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
        Velocity.Value = FleeDirection * 800.0f; // High panic speed
    }
}

bool UDinosaurHerdProcessor::DetectPredatorNearby(const FMassExecutionContext& Context, const FTransformFragment& Transform, float DetectionRadius)
{
    // Simplified predator detection
    // In practice, this would query for nearby carnivorous dinosaurs
    // For now, randomly trigger panic occasionally to simulate predator encounters
    return FMath::RandRange(0.0f, 1.0f) < 0.001f; // 0.1% chance per frame
}

// ===== UDinosaurRoutineProcessor =====

UDinosaurRoutineProcessor::UDinosaurRoutineProcessor()
{
    ProcessingPhase = EMassProcessingPhase::PrePhysics;
    ExecutionFlags = (int32)(EProcessorExecutionFlags::All);
}

void UDinosaurRoutineProcessor::ConfigureQueries()
{
    RoutineQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
    RoutineQuery.AddRequirement<FDinosaurRoutineFragment>(EMassFragmentAccess::ReadWrite);
    RoutineQuery.AddRequirement<FDinosaurSpeciesFragment>(EMassFragmentAccess::ReadOnly);
    RoutineQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadWrite);
}

void UDinosaurRoutineProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
    RoutineQuery.ForEachEntityChunk(EntityManager, Context, [this](FMassExecutionContext& Context)
    {
        const auto& TransformList = Context.GetFragmentView<FTransformFragment>();
        auto& RoutineList = Context.GetMutableFragmentView<FDinosaurRoutineFragment>();
        const auto& SpeciesList = Context.GetFragmentView<FDinosaurSpeciesFragment>();
        auto& MoveTargetList = Context.GetMutableFragmentView<FMassMoveTargetFragment>();

        const float CurrentTime = Context.GetWorld()->GetTimeSeconds();

        for (int32 EntityIndex = 0; EntityIndex < Context.GetNumEntities(); ++EntityIndex)
        {
            const FTransformFragment& Transform = TransformList[EntityIndex];
            FDinosaurRoutineFragment& Routine = RoutineList[EntityIndex];
            const FDinosaurSpeciesFragment& Species = SpeciesList[EntityIndex];
            FMassMoveTargetFragment& MoveTarget = MoveTargetList[EntityIndex];

            // Update current activity based on time and needs
            UpdateCurrentActivity(Routine, Species, CurrentTime);

            // Set movement target based on current activity
            switch (Routine.CurrentActivity)
            {
                case 1: // Grazing
                    if (Species.bIsHerbivore)
                    {
                        MoveTarget.Center = FindGrazingLocation(Transform, Species);
                        MoveTarget.SlackRadius = Species.GrazingRadius;
                    }
                    break;

                case 2: // Drinking
                    MoveTarget.Center = FindWaterSource(Transform);
                    MoveTarget.SlackRadius = 200.0f;
                    break;

                case 3: // Resting
                    MoveTarget.Center = FindRestingSpot(Transform);
                    MoveTarget.SlackRadius = 100.0f;
                    break;

                default: // Wandering
                    // Wander within home range
                    FVector WanderTarget = Routine.HomeLocation + FVector(
                        FMath::RandRange(-Routine.HomeRadius, Routine.HomeRadius),
                        FMath::RandRange(-Routine.HomeRadius, Routine.HomeRadius),
                        0.0f
                    );
                    MoveTarget.Center = WanderTarget;
                    MoveTarget.SlackRadius = 500.0f;
                    break;
            }
        }
    });
}

void UDinosaurRoutineProcessor::UpdateCurrentActivity(FDinosaurRoutineFragment& Routine, const FDinosaurSpeciesFragment& Species, float CurrentTime)
{
    const float TimeSinceActivityStart = CurrentTime - Routine.ActivityStartTime;

    // Check if current activity should end
    switch (Routine.CurrentActivity)
    {
        case 1: // Grazing
            if (TimeSinceActivityStart >= Species.GrazingDuration)
            {
                Routine.CurrentActivity = 0; // Back to wandering
                Routine.ActivityStartTime = CurrentTime;
                Routine.LastGrazingTime = CurrentTime;
            }
            break;

        case 2: // Drinking
            if (TimeSinceActivityStart >= 10.0f) // 10 seconds to drink
            {
                Routine.CurrentActivity = 0;
                Routine.ActivityStartTime = CurrentTime;
                Routine.LastDrinkingTime = CurrentTime;
            }
            break;

        case 3: // Resting
            if (TimeSinceActivityStart >= 60.0f) // 1 minute rest
            {
                Routine.CurrentActivity = 0;
                Routine.ActivityStartTime = CurrentTime;
                Routine.LastRestTime = CurrentTime;
            }
            break;

        default: // Wandering - check if needs require activity change
            if (Species.bIsHerbivore && (CurrentTime - Routine.LastGrazingTime) >= Species.TimeBetweenGrazing)
            {
                Routine.CurrentActivity = 1; // Start grazing
                Routine.ActivityStartTime = CurrentTime;
            }
            else if ((CurrentTime - Routine.LastDrinkingTime) >= 300.0f) // Drink every 5 minutes
            {
                Routine.CurrentActivity = 2; // Go drink
                Routine.ActivityStartTime = CurrentTime;
            }
            else if ((CurrentTime - Routine.LastRestTime) >= 600.0f) // Rest every 10 minutes
            {
                Routine.CurrentActivity = 3; // Go rest
                Routine.ActivityStartTime = CurrentTime;
            }
            break;
    }
}

FVector UDinosaurRoutineProcessor::FindGrazingLocation(const FTransformFragment& Transform, const FDinosaurSpeciesFragment& Species)
{
    // Find nearby vegetation-rich area
    // For now, return a location within grazing radius
    return Transform.GetTransform().GetLocation() + FVector(
        FMath::RandRange(-Species.GrazingRadius, Species.GrazingRadius),
        FMath::RandRange(-Species.GrazingRadius, Species.GrazingRadius),
        0.0f
    );
}

FVector UDinosaurRoutineProcessor::FindWaterSource(const FTransformFragment& Transform)
{
    // Find nearest water source
    // For now, return a location that represents a water source
    // In practice, this would query for actual water bodies in the world
    return Transform.GetTransform().GetLocation() + FVector(
        FMath::RandRange(-2000.0f, 2000.0f),
        FMath::RandRange(-2000.0f, 2000.0f),
        0.0f
    );
}

FVector UDinosaurRoutineProcessor::FindRestingSpot(const FTransformFragment& Transform)
{
    // Find a safe resting spot
    // For now, return a nearby location
    return Transform.GetTransform().GetLocation() + FVector(
        FMath::RandRange(-500.0f, 500.0f),
        FMath::RandRange(-500.0f, 500.0f),
        0.0f
    );
}

// ===== UDinosaurHerdSubsystem =====

void UDinosaurHerdSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize default species population targets
    SpeciesPopulationTargets.Add(TEXT("Triceratops"), 500);
    SpeciesPopulationTargets.Add(TEXT("Parasaurolophus"), 800);
    SpeciesPopulationTargets.Add(TEXT("Stegosaurus"), 300);
    SpeciesPopulationTargets.Add(TEXT("TRex"), 20);
    SpeciesPopulationTargets.Add(TEXT("Velociraptor"), 100);
    SpeciesPopulationTargets.Add(TEXT("Compsognathus"), 1000);
}

void UDinosaurHerdSubsystem::Deinitialize()
{
    HerdEntities.Empty();
    Super::Deinitialize();
}

int32 UDinosaurHerdSubsystem::CreateHerd(FName SpeciesName, const FVector& Location, int32 HerdSize)
{
    int32 HerdID = NextHerdID++;
    
    TArray<FMassEntityHandle> NewHerd;
    NewHerd.Reserve(HerdSize);

    // Spawn herd leader first
    SpawnDinosaurEntity(SpeciesName, Location, HerdID, true);
    
    // Spawn herd members in formation around leader
    for (int32 i = 1; i < HerdSize; ++i)
    {
        FVector MemberLocation = Location + FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );
        SpawnDinosaurEntity(SpeciesName, MemberLocation, HerdID, false);
    }

    return HerdID;
}

void UDinosaurHerdSubsystem::SpawnHerdAtLocation(int32 HerdID, const FVector& Location)
{
    // Implementation for spawning existing herd at new location
    // This would move all entities in the herd to the new location
}

void UDinosaurHerdSubsystem::TriggerHerdPanic(int32 HerdID, const FVector& ThreatLocation)
{
    // Implementation for triggering panic in a specific herd
    // This would set panic state on all herd members
}

void UDinosaurHerdSubsystem::SetSpeciesPopulationTarget(FName SpeciesName, int32 TargetPopulation)
{
    SpeciesPopulationTargets.Add(SpeciesName, TargetPopulation);
}

int32 UDinosaurHerdSubsystem::GetCurrentSpeciesPopulation(FName SpeciesName) const
{
    const int32* Population = CurrentSpeciesPopulations.Find(SpeciesName);
    return Population ? *Population : 0;
}

void UDinosaurHerdSubsystem::SpawnDinosaurEntity(FName SpeciesName, const FVector& Location, int32 HerdID, bool bIsLeader)
{
    // This would interface with the Mass Entity system to spawn a new dinosaur
    // For now, this is a placeholder that would be implemented with the actual Mass spawning system
    
    UE_LOG(LogTemp, Log, TEXT("Spawning %s at location %s for herd %d (Leader: %s)"), 
        *SpeciesName.ToString(), 
        *Location.ToString(), 
        HerdID, 
        bIsLeader ? TEXT("Yes") : TEXT("No"));
}

void UDinosaurHerdSubsystem::UpdatePopulationCounts()
{
    // Update current population counts for each species
    // This would query the Mass Entity system for current entity counts by species
}
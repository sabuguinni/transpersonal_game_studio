#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurMassProcessor_CrowdSimulation.generated.h"

// Forward declarations
struct FMassVelocityFragment;
struct FTransformFragment;
struct FDinosaurBehaviorFragment;
struct FDinosaurSpeciesFragment;
struct FDinosaurHerdFragment;

/**
 * Mass processor for dinosaur crowd simulation
 * Handles movement, flocking, and basic behaviors for large numbers of dinosaurs
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSim_DinosaurMassProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdSim_DinosaurMassProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for herbivore herd behavior
    FMassEntityQuery HerbivoreQuery;
    
    // Query for carnivore pack behavior  
    FMassEntityQuery CarnivoreQuery;
    
    // Query for solitary dinosaurs
    FMassEntityQuery SolitaryQuery;

    // Flocking parameters
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float SeparationRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float AlignmentRadius = 800.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float CohesionRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float MaxSpeed = 600.0f;
    
    UPROPERTY(EditAnywhere, Category = "Flocking")
    float MaxForce = 100.0f;

    // Behavior weights
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float SeparationWeight = 2.0f;
    
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float AlignmentWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float CohesionWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, Category = "Behavior")
    float WanderWeight = 0.5f;

    // Helper functions
    FVector CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions);
    FVector CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities);
    FVector CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions);
    FVector CalculateWander(const FVector& CurrentVelocity, float DeltaTime);
    
    void ProcessHerbivoreHerds(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCarnivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessSolitaryDinosaurs(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
};
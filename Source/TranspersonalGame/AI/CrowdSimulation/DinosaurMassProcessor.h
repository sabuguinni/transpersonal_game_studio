#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurMassFragments.h"
#include "DinosaurMassProcessor.generated.h"

/**
 * Main processor for dinosaur crowd simulation using Mass Entity Framework
 * Handles up to 50,000 simultaneous dinosaur agents with emergent behaviors
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurMassProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurMassProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Core behavior queries
    FMassEntityQuery HerbivoreBehaviorQuery;
    FMassEntityQuery CarnivoreBehaviorQuery;
    FMassEntityQuery FlockingQuery;
    FMassEntityQuery TerritoryQuery;
    
    // Interaction queries
    FMassEntityQuery PredatorPreyQuery;
    FMassEntityQuery DomesticationQuery;
    FMassEntityQuery PlayerProximityQuery;
    
    // Environmental queries
    FMassEntityQuery WaterSourceQuery;
    FMassEntityQuery FoodSourceQuery;
    FMassEntityQuery ShelterQuery;

    // Behavior execution methods
    void ProcessHerbivoreRoutines(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCarnivoreHunting(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessFlockingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPredatorPreyInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessDomesticationSystem(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPlayerAwareness(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Utility methods
    FVector FindNearestWaterSource(const FVector& Position) const;
    FVector FindNearestFoodSource(const FVector& Position, EDinosaurDiet DietType) const;
    bool IsInPlayerProximity(const FVector& Position, float ProximityRadius = 5000.0f) const;
    float CalculateThreatLevel(const FDinosaurSpeciesFragment& Species, const FVector& Position) const;
};
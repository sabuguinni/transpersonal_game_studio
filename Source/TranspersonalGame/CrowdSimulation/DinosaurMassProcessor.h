#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "DinosaurMassFragments.h"
#include "DinosaurMassProcessor.generated.h"

/**
 * Main processor for dinosaur crowd simulation using Mass Entity framework
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
    // Query for herbivore behavior
    FMassEntityQuery HerbivoreQuery;
    
    // Query for carnivore behavior  
    FMassEntityQuery CarnivoreQuery;
    
    // Query for pack behavior (raptors, etc)
    FMassEntityQuery PackQuery;
    
    // Query for territorial behavior (T-Rex, etc)
    FMassEntityQuery TerritorialQuery;

    // Behavior execution functions
    void ProcessHerbivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCarnivores(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessPackBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Ecosystem interaction functions
    void HandlePredatorPreyInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateTerritorialBoundaries(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessMigrationPatterns(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

public:
    // Configuration parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float MaxSimulationDistance = 5000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    int32 MaxActiveAgents = 50000;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Simulation")
    float UpdateFrequency = 0.1f; // 10 times per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float PredatorDetectionRange = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float TerritoryRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODSystem = true;
};
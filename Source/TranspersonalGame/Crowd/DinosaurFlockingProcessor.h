#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "DinosaurCrowdFragments.h"
#include "DinosaurFlockingProcessor.generated.h"

/**
 * Mass Processor that handles boids flocking behavior for dinosaur crowds
 * Implements cohesion, separation, alignment, and avoidance forces
 */
UCLASS()
class TRANSPERSONALGAME_API UDinosaurFlockingProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurFlockingProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for all dinosaurs that need flocking updates
    FMassEntityQuery FlockingQuery;

    // Flocking parameters
    UPROPERTY(EditAnywhere, Category = "Flocking Parameters")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking Parameters")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking Parameters")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking Parameters")
    float AvoidanceWeight = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking Parameters")
    float MaxFlockingForce = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxNeighborChecks = 20; // Limit neighbor checks for performance

    UPROPERTY(EditAnywhere, Category = "Performance")
    float NeighborSearchRadius = 800.0f; // Maximum distance to search for neighbors

    // Helper functions for boids algorithm
    FVector CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions) const;
    FVector CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions, float SeparationRadius) const;
    FVector CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities) const;
    FVector CalculateAvoidance(const FVector& Position, const FVector& Velocity, const TArray<FVector>& ObstaclePositions) const;
    
    // Environmental avoidance
    FVector CalculateTerrainAvoidance(const FVector& Position, const FVector& Velocity) const;
    FVector CalculateWaterAvoidance(const FVector& Position, const FVector& Velocity) const;
    
    // Predator-specific flocking
    FVector CalculatePredatorHunting(const FVector& Position, const FVector& Velocity, const TArray<FVector>& PreyPositions) const;
    
    // Performance optimization
    void SpatialHashNeighbors(const TArray<FVector>& Positions, TMap<int32, TArray<int32>>& SpatialHash) const;
    int32 GetSpatialHashKey(const FVector& Position) const;
};
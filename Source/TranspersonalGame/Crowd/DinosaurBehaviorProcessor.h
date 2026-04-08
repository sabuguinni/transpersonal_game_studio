#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "DinosaurBehaviorFragments.h"
#include "DinosaurBehaviorProcessor.generated.h"

UCLASS()
class TRANSPERSONALGAME_API UDinosaurBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery BehaviorQuery;
    FMassEntityQuery FlockingQuery;
    FMassEntityQuery NeedsQuery;

    void ProcessBehaviorStates(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessFlockingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessNeedsSystem(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    void UpdateBehaviorState(FDinosaurBehaviorFragment& Behavior, const FDinosaurNeedsFragment& Needs, 
                           const FDinosaurSpeciesFragment& Species, float DeltaTime);
    
    EDinosaurBehaviorState DetermineNextState(const FDinosaurBehaviorFragment& Behavior, 
                                            const FDinosaurNeedsFragment& Needs,
                                            const FDinosaurSpeciesFragment& Species);
    
    FVector CalculateFlockingForces(const FTransformFragment& Transform, 
                                  const FDinosaurFlockingFragment& Flocking,
                                  const FDinosaurSpeciesFragment& Species);
};

UCLASS()
class TRANSPERSONALGAME_API UDinosaurNeedsProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurNeedsProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery NeedsUpdateQuery;
    
    void UpdateNeedsOverTime(FDinosaurNeedsFragment& Needs, const FDinosaurSpeciesFragment& Species, 
                           const FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    
    void ProcessHunger(FDinosaurNeedsFragment& Needs, const FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    void ProcessThirst(FDinosaurNeedsFragment& Needs, const FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    void ProcessEnergy(FDinosaurNeedsFragment& Needs, const FDinosaurBehaviorFragment& Behavior, float DeltaTime);
    void ProcessSocial(FDinosaurNeedsFragment& Needs, const FDinosaurFlockingFragment& Flocking, float DeltaTime);
};

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
    FMassEntityQuery FlockingUpdateQuery;
    FMassEntityQuery NearbyEntitiesQuery;
    
    void UpdateFlockingForces(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void FindNearbyEntities(FMassEntityManager& EntityManager, const FTransformFragment& Transform,
                          FDinosaurFlockingFragment& Flocking, const FDinosaurSpeciesFragment& Species);
    
    FVector CalculateSeparation(const FTransformFragment& Transform, const FDinosaurFlockingFragment& Flocking);
    FVector CalculateAlignment(const FTransformFragment& Transform, const FDinosaurFlockingFragment& Flocking,
                             FMassEntityManager& EntityManager);
    FVector CalculateCohesion(const FTransformFragment& Transform, const FDinosaurFlockingFragment& Flocking,
                            FMassEntityManager& EntityManager);
};

UCLASS()
class TRANSPERSONALGAME_API UDinosaurTerritoryProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UDinosaurTerritoryProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery TerritoryQuery;
    
    void ProcessTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void EstablishTerritory(FDinosaurTerritoryFragment& Territory, const FTransformFragment& Transform,
                          const FDinosaurSpeciesFragment& Species);
    void DefendTerritory(FDinosaurTerritoryFragment& Territory, FDinosaurBehaviorFragment& Behavior,
                       const FTransformFragment& Transform, const FDinosaurSpeciesFragment& Species);
};
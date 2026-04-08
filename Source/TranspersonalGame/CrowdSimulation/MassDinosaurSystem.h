#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassDinosaurFragments.h"
#include "MassDinosaurSystem.generated.h"

/**
 * Mass Entity system for simulating dinosaur herds and flocks
 * Handles up to 50,000 simultaneous dinosaur agents
 */

UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassDinosaurMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;
};

UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurHerdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassDinosaurHerdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery HerdQuery;
    
    // Herd behavior parameters
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float CohesionRadius = 1000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float SeparationRadius = 300.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float AlignmentRadius = 500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float LeaderFollowRadius = 2000.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurPredatorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassDinosaurPredatorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PredatorQuery;
    FMassEntityQuery PreyQuery;
    
    // Hunting behavior parameters
    UPROPERTY(EditAnywhere, Category = "Hunting")
    float HuntingRadius = 3000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hunting")
    float PackCoordinationRadius = 1500.0f;
    
    UPROPERTY(EditAnywhere, Category = "Hunting")
    float AmbushDistance = 500.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UMassDinosaurEcosystemProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassDinosaurEcosystemProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EcosystemQuery;
    
    // Ecosystem simulation parameters
    UPROPERTY(EditAnywhere, Category = "Ecosystem")
    float TerritoryRadius = 5000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem")
    float MigrationTriggerDistance = 10000.0f;
    
    UPROPERTY(EditAnywhere, Category = "Ecosystem")
    float ResourceCompetitionRadius = 2000.0f;
};
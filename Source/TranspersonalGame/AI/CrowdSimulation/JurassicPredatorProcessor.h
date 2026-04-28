#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "JurassicCrowdTypes.h"
#include "MassEntityTypes.h"
#include "JurassicPredatorProcessor.generated.h"

/**
 * Processor para comportamento de predadores solitários
 * Implementa patrulhamento territorial, caça e comportamento agressivo
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicSolitaryPredatorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicSolitaryPredatorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery SolitaryPredatorQuery;
    FMassEntityQuery PreyDetectionQuery;

    // Parâmetros de comportamento
    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float HuntingRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float HuntingSpeed = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float HuntCooldown = 300.0f; // 5 minutos

    UPROPERTY(EditAnywhere, Category = "Predator Behavior")
    float TerritoryPatrolRadius = 1500.0f;

    void ProcessPatrolBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessHuntingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateTerritorialBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    bool FindNearestPrey(const FVector& PredatorPosition, FMassEntityManager& EntityManager, FVector& OutPreyLocation);
    void GeneratePatrolPoints(FJurassicTerritoryFragment& Territory, const FVector& CenterPosition);
};

/**
 * Processor para comportamento de predadores em matilha
 */
UCLASS()
class TRANSPERSONALGAME_API UJurassicPackPredatorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UJurassicPackPredatorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery PackMemberQuery;
    FMassEntityQuery PackLeaderQuery;
    FMassEntityQuery PackPreyQuery;

    // Parâmetros de matilha
    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float PackCohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float PackSeparationDistance = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float CoordinatedHuntRadius = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float FlankingDistance = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    float PackHuntSpeed = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    int32 MinPackSize = 3;

    UPROPERTY(EditAnywhere, Category = "Pack Behavior")
    int32 MaxPackSize = 8;

    void ProcessPackCohesion(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCoordinatedHunt(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void AssignHuntingRoles(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    struct FPackHuntData
    {
        FVector TargetLocation;
        TArray<FMassEntityHandle> Hunters;
        TArray<FVector> FlankingPositions;
        bool bHuntActive;
        float HuntStartTime;
    };
    
    TMap<int32, FPackHuntData> ActiveHunts;
};

/**
 * Processor para comportamento territorial e dominância
 */
UCLASS()
class TRANSPERSONALGAME_API UAI_JurassicTerritorialProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UAI_JurassicTerritorialProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery TerritorialQuery;
    FMassEntityQuery IntruderDetectionQuery;

    // Parâmetros territoriais
    UPROPERTY(EditAnywhere, Category = "Territorial Behavior")
    float TerritoryDefenseRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Territorial Behavior")
    float IntruderDetectionRadius = 1200.0f;

    UPROPERTY(EditAnywhere, Category = "Territorial Behavior")
    float ThreatDisplayDistance = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Territorial Behavior")
    float ChaseDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Territorial Behavior")
    float TerritorialAggressionLevel = 0.8f;

    void ProcessTerritoryDefense(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessIntruderResponse(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateTerritoryBoundaries(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    bool IsInTerritory(const FVector& Position, const FJurassicTerritoryFragment& Territory);
    FVector CalculateOptimalTerritoryCenter(const FVector& CurrentPosition, const TArray<FVector>& ResourceLocations);
};
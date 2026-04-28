#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityQuery.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassDinosaurTypes.h"
#include "MassEntityTypes.h"
#include "MassHerdBehaviorProcessor.generated.h"

/**
 * Processor que gerencia comportamento de manada para dinossauros
 * Implementa flocking behaviors (separation, alignment, cohesion) + liderança
 */
UCLASS()
class TRANSPERSONALGAME_API UMassHerdBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UMassHerdBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query para líderes de manada
    FMassEntityQuery HerdLeaderQuery;
    
    // Query para membros de manada
    FMassEntityQuery HerdMemberQuery;
    
    // Query para dinossauros em fuga
    FMassEntityQuery FleeingDinosaursQuery;

    // Parâmetros de comportamento
    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float MaxFlockingDistance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float LeaderInfluenceRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float SeparationForceMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float AlignmentForceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float CohesionForceMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float LeaderFollowForceMultiplier = 3.0f;

    UPROPERTY(EditAnywhere, Category = "Herd Behavior")
    float FleeForceMultiplier = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Migration")
    float MigrationDecisionInterval = 30.0f; // segundos

    UPROPERTY(EditAnywhere, Category = "Migration")
    float MigrationTriggerDistance = 5000.0f; // distância para iniciar migração

    // Funções auxiliares
    void ProcessHerdLeaders(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessHerdMembers(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessFleeingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    FVector CalculateFlockingForce(const FTransformFragment& Transform, 
                                   const FMassFlockingFragment& Flocking,
                                   const TArray<FMassEntityHandle>& NearbyEntities,
                                   FMassEntityManager& EntityManager);
    
    FVector CalculateSeparationForce(const FVector& Position, 
                                     const TArray<FVector>& NearbyPositions,
                                     float SeparationRadius);
    
    FVector CalculateAlignmentForce(const FVector& Velocity,
                                    const TArray<FVector>& NearbyVelocities);
    
    FVector CalculateCohesionForce(const FVector& Position,
                                   const TArray<FVector>& NearbyPositions);
    
    FVector CalculateLeaderFollowForce(const FVector& Position,
                                       const FVector& LeaderPosition,
                                       float FollowDistance);

    bool ShouldStartMigration(const FMassHerdLeaderFragment& Leader,
                              const FMassDinosaurFragment& Dinosaur,
                              const FTransformFragment& Transform);

    FVector FindMigrationTarget(const FVector& CurrentPosition,
                                EDinosaurHerdType HerdType);

    void UpdateHerdMembership(FMassEntityManager& EntityManager, 
                              FMassExecutionContext& Context);
};
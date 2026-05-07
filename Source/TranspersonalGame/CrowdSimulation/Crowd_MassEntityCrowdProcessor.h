#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassEntityCrowdProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector FlockingForce = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    ECrowd_BiomeType BiomeType = ECrowd_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeAttraction = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius = 500.0f;
};

/**
 * Processador principal para simulação de multidões usando Mass Entity
 * Implementa comportamento de flocking, navegação por biomas e LOD dinâmico
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntityCrowdProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityCrowdProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    // Query para entidades com comportamento de flocking
    FMassEntityQuery FlockingQuery;

    // Query para entidades com comportamento de bioma
    FMassEntityQuery BiomeQuery;

    // Query para entidades com comportamento geral
    FMassEntityQuery BehaviorQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxEntitiesPerFrame = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ProcessingTimeLimit = 0.016f; // 16ms target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float GlobalFlockingStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bUseBiomeAttraction = true;

private:
    void ProcessFlockingBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessBiomeBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessGeneralBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    FVector CalculateFlockingForce(const FVector& EntityLocation, const FVector& EntityVelocity, 
                                   const TArray<FVector>& NeighborLocations, 
                                   const TArray<FVector>& NeighborVelocities,
                                   const FCrowd_FlockingFragment& FlockingData);

    FVector CalculateSeparation(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations, float Radius, float Weight);
    FVector CalculateAlignment(const FVector& EntityVelocity, const TArray<FVector>& NeighborVelocities, float Weight);
    FVector CalculateCohesion(const FVector& EntityLocation, const TArray<FVector>& NeighborLocations, float Weight);

    FVector GetBiomeCenter(ECrowd_BiomeType BiomeType);
    bool IsInBiome(const FVector& Location, ECrowd_BiomeType BiomeType);

    double LastExecutionTime = 0.0;
    int32 ProcessedEntitiesThisFrame = 0;
};
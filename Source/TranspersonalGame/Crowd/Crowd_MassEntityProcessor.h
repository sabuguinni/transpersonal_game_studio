#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassLODFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_AgentFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float PersonalSpace = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float ViewRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 AgentType = 0; // 0=Villager, 1=Hunter, 2=Gatherer, 3=Child

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    float StressLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    bool bIsInGroup = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Agent")
    int32 GroupID = -1;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector SeparationForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector CohesionForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector AlignmentForce = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.5f;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxSpeed = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxAcceleration = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float NeighborRadius = 150.0f;

    void CalculateFlockingForces(const FMassExecutionContext& Context, 
                                FCrowd_AgentFragment& Agent, 
                                FCrowd_FlockingFragment& Flocking,
                                const FTransformFragment& Transform);

    FVector CalculateSeparation(const FVector& Position, const FMassExecutionContext& Context, float Radius);
    FVector CalculateCohesion(const FVector& Position, const FMassExecutionContext& Context, float Radius);
    FVector CalculateAlignment(const FVector& Velocity, const FMassExecutionContext& Context, float Radius);
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_LODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_LODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    void UpdateLODLevel(FMassLODFragment& LODFragment, float DistanceToPlayer);
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_BehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_BehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    void UpdateAgentBehavior(FCrowd_AgentFragment& Agent, const FTransformFragment& Transform, float DeltaTime);
    void HandleGroupBehavior(FCrowd_AgentFragment& Agent, const FMassExecutionContext& Context);
    FVector FindNearestGatheringPoint(const FVector& Position);
    FVector FindNearestShelter(const FVector& Position);
};
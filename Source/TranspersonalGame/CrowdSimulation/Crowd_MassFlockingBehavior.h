#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "Engine/World.h"
#include "Crowd_SharedTypes.h"
#include "Crowd_MassFlockingBehavior.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlockingFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float SeparationWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    float MaxForce = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    int32 FlockID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flocking")
    FVector DesiredDirection = FVector::ForwardVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_HerdMemberFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    ECrowd_DinosaurSpecies Species = ECrowd_DinosaurSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float HerdRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float FollowDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float StressLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    FVector LastKnownThreatLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Herd")
    float TimeSinceLastThreat = 0.0f;
};

UCLASS(meta = (DisplayName = "Crowd Flocking Behavior Processor"))
class TRANSPERSONALGAME_API UCrowd_MassFlockingBehaviorProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassFlockingBehaviorProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery FlockingQuery;

    // Flocking behavior calculations
    FVector CalculateSeparation(const FVector& Position, const TArray<FVector>& NeighborPositions, float Radius) const;
    FVector CalculateAlignment(const FVector& Velocity, const TArray<FVector>& NeighborVelocities, float Radius) const;
    FVector CalculateCohesion(const FVector& Position, const TArray<FVector>& NeighborPositions, float Radius) const;
    
    // Herd behavior calculations
    FVector CalculateHerdBehavior(const FCrowd_HerdMemberFragment& HerdMember, const FVector& Position, const TArray<FVector>& HerdPositions) const;
    void UpdateStressLevel(FCrowd_HerdMemberFragment& HerdMember, float DeltaTime, bool bThreatDetected) const;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    float MaxNeighborDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Flocking")
    int32 MaxNeighbors = 10;

    UPROPERTY(EditAnywhere, Category = "Herd")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Herd")
    float StressDecayRate = 0.5f;
};
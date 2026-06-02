#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

class UMassSignalSubsystem;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Acceleration = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MaxStateTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    int32 GroupID = -1;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastUpdateTime = 0.0f;
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
    float MaxMovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SteeringForce = 800.0f;
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

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float BehaviorUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float WanderRadius = 1000.0f;

    void UpdateBehaviorState(FCrowd_BehaviorFragment& BehaviorFragment, const FTransformFragment& Transform, float DeltaTime);
    FVector GetWanderTarget(const FVector& CurrentLocation);
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
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumLODDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowLODDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LODUpdateInterval = 1.0f;

    ECrowd_LODLevel CalculateLODLevel(float Distance) const;
    FVector GetPlayerLocation() const;
};

#include "Crowd_MassEntityProcessor.generated.h"
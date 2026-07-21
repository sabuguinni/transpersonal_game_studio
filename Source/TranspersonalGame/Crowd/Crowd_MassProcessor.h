#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MaxSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bHasTarget = false;
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
    float FleeDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    AActor* ThreatActor = nullptr;
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
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AvoidanceRadius = 200.0f;
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
    float ThreatDetectionRadius = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Behavior")
    float GroupingRadius = 300.0f;
};
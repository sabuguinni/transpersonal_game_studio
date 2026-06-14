#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsMoving = false;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_NPCBehavior CurrentBehavior = ECrowd_NPCBehavior::Wandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BehaviorDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GroupID = -1;
};

USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisible = true;
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
    FMassEntityQuery MovementQuery;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxMovementSpeed = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinMovementSpeed = 50.0f;
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
    FMassEntityQuery BehaviorQuery;

    void UpdateBehavior(FCrowd_BehaviorFragment& BehaviorFragment, FCrowd_MovementFragment& MovementFragment, const FTransformFragment& TransformFragment, float DeltaTime);
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
    FMassEntityQuery LODQuery;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumLODDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowLODDistance = 5000.0f;
};
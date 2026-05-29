#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_MovementFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WanderRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector WanderTarget = FVector::ZeroVector;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BiomeFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 BiomeType = 0; // 0=Savana, 1=Forest, 2=Desert

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BiomeAffinity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanLeaveBiome = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    uint8 CurrentLOD = 0; // 0=High, 1=Medium, 2=Low, 3=Culled

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsVisible = true;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassMovementProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassMovementProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float WanderStrength = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float SeparationRadius = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AlignmentRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float CohesionRadius = 300.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassLODProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassLODProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    FMassEntityQuery EntityQuery;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighLODDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumLODDistance = 3000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowLODDistance = 8000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float CullDistance = 15000.0f;
};

#include "Crowd_MassEntityProcessor.generated.h"
#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "Engine/World.h"
#include "Crowd_MassEntityProcessor.generated.h"

// Forward declarations
struct FCrowd_BehaviorFragment;
struct FCrowd_LODFragment;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float SocialRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    int32 GroupID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODFragment : public FMassFragment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 CurrentLOD = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastUpdateTime = 0.0f;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowd_MassEntityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityProcessor();

protected:
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
    // Query for crowd entities with behavior and LOD
    FMassEntityQuery CrowdQuery;

    // Process crowd behavior patterns
    void ProcessCrowdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Update LOD based on distance to player
    void UpdateLODSystem(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Handle group formation and flocking
    void ProcessGroupFormation(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    float MaxProcessingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Settings")
    int32 MaxEntitiesPerFrame = 1000;

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD1Distance = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD Settings")
    float LOD2Distance = 3000.0f;
};
#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassProcessor.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassEntityView.h"
#include "MassExecutionContext.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

/**
 * Mass Entity Processor for crowd simulation
 * Handles movement, behavior, and LOD for up to 50,000 entities
 */
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
    // Movement processing
    void ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // Behavior processing
    void ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    
    // LOD processing
    void ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Queries for different entity types
    FMassEntityQuery MovementQuery;
    FMassEntityQuery BehaviorQuery;
    FMassEntityQuery LODQuery;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxEntitiesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float ProcessingTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd", meta = (AllowPrivateAccess = "true"))
    float CrowdDensityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crowd", meta = (AllowPrivateAccess = "true"))
    float AvoidanceRadius;
};
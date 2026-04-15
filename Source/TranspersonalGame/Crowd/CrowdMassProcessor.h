#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "../SharedTypes.h"
#include "CrowdMassProcessor.generated.h"

// Forward declarations
class UMassEntitySubsystem;
struct FMassEntityHandle;

/**
 * Mass processor for crowd simulation in prehistoric tribal settlements
 * Handles movement, behavior, and LOD for up to 50,000 simultaneous agents
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdMassProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowdMassProcessor();

protected:
    // UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

    // Crowd behavior processing
    void ProcessCrowdMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCrowdBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessCrowdLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Crowd AI logic
    void UpdateTribalBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateGatheringBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void UpdateSocialInteractions(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

private:
    // Mass queries for different crowd behaviors
    FMassEntityQuery MovementQuery;
    FMassEntityQuery BehaviorQuery;
    FMassEntityQuery LODQuery;
    FMassEntityQuery SocialQuery;

    // Performance settings
    UPROPERTY(EditAnywhere, Category = "Performance")
    int32 MaxEntitiesPerFrame = 1000;

    UPROPERTY(EditAnywhere, Category = "Performance")
    float ProcessingTimeLimit = 0.016f; // 16ms target

    // Crowd behavior parameters
    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float TribalCohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float GatheringRadius = 200.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float SocialInteractionRadius = 150.0f;

    UPROPERTY(EditAnywhere, Category = "Crowd Behavior")
    float MovementSpeed = 100.0f;

    // LOD distances
    UPROPERTY(EditAnywhere, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float MediumDetailDistance = 2500.0f;

    UPROPERTY(EditAnywhere, Category = "LOD")
    float LowDetailDistance = 5000.0f;

    // Timing tracking
    float LastProcessTime = 0.0f;
    int32 ProcessedEntitiesThisFrame = 0;
};
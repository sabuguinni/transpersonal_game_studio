#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "MassEntityTypes.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityProcessor.generated.h"

class UMassEntitySubsystem;
struct FCrowd_MovementFragment;
struct FCrowd_BehaviorFragment;

/**
 * Mass Entity processor for crowd simulation
 * Handles movement, pathfinding, and LOD for up to 50,000 entities
 */
UCLASS(BlueprintType, meta = (DisplayName = "Crowd Mass Entity Processor"))
class TRANSPERSONALGAME_API UCrowd_MassEntityProcessor : public UMassProcessor
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityProcessor();

    // UMassProcessor interface
    virtual void ConfigureQueries() override;
    virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

protected:
    // Query for crowd entities with movement
    FMassEntityQuery MovementQuery;
    
    // Query for crowd entities with behavior
    FMassEntityQuery BehaviorQuery;
    
    // Query for LOD processing
    FMassEntityQuery LODQuery;

    // Processing functions
    void ProcessMovement(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessBehavior(FMassEntityManager& EntityManager, FMassExecutionContext& Context);
    void ProcessLOD(FMassEntityManager& EntityManager, FMassExecutionContext& Context);

    // Movement parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float MaxSpeed = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float AccelerationRate = 600.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Movement")
    float TurnRate = 180.0f;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float WaypointReachDistance = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float AvoidanceRadius = 150.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Behavior")
    float FlockingStrength = 0.5f;

    // LOD parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float HighLODDistance = 500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float MediumLODDistance = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd LOD")
    float LowLODDistance = 3000.0f;

    // Performance tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 ProcessedEntities = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    float LastProcessingTime = 0.0f;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 HighLODCount = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MediumLODCount = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 LowLODCount = 0;

private:
    // Helper functions
    FVector CalculateFlockingForce(const FVector& EntityLocation, const TArray<FVector>& NearbyEntities);
    FVector CalculateAvoidanceForce(const FVector& EntityLocation, const TArray<FVector>& Obstacles);
    ECrowd_LODLevel DetermineLODLevel(const FVector& EntityLocation, const FVector& ViewerLocation);
    
    // Performance optimization
    float LastUpdateTime = 0.0f;
    static constexpr float UpdateInterval = 0.016f; // 60 FPS target
};
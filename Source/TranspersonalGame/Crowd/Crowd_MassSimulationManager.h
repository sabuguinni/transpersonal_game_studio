#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Shared/SharedTypes.h"
#include "Crowd_MassSimulationManager.generated.h"

// Forward declarations
class UCrowd_EntitySpawner;
class UCrowd_PathfindingComponent;
class UCrowd_LODManager;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    ECrowd_EntityBehavior BehaviorState;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        MovementSpeed = 300.0f;
        TargetLocation = FVector::ZeroVector;
        EntityID = -1;
        BehaviorState = ECrowd_EntityBehavior::Idle;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    FVector SpawnCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinDistanceBetweenEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    ECrowd_EntityType EntityType;

    FCrowd_SpawnParameters()
    {
        MaxEntities = 1000;
        SpawnRadius = 5000.0f;
        SpawnCenter = FVector::ZeroVector;
        MinDistanceBetweenEntities = 200.0f;
        EntityType = ECrowd_EntityType::Tribal;
    }
};

/**
 * Mass Entity-based crowd simulation manager for prehistoric tribal populations
 * Handles up to 50,000 simultaneous entities using UE5 Mass Entity framework
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_MassSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_MassSimulationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void InitializeMassSimulation();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void SpawnEntityGroup(const FCrowd_SpawnParameters& SpawnParams);

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    void DespawnAllEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Simulation")
    int32 GetActiveEntityCount() const;

    // Pathfinding and Movement
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetGroupDestination(const FVector& Destination, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateEntityMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPathToLocation(const FVector& StartLocation, const FVector& EndLocation, TArray<FVector>& OutPath);

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void UpdateLODLevels(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetLODDistance(int32 LODLevel, float Distance);

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetEntityBehavior(int32 EntityID, ECrowd_EntityBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetGatheringMode(bool bEnabled, const FVector& GatherLocation);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetSimulationPerformance() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(ECrowd_PerformanceMode Mode);

protected:
    // Entity Data Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entities")
    TArray<FCrowd_EntityData> EntityDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FCrowd_SpawnParameters DefaultSpawnParams;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulationEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    ECrowd_PerformanceMode CurrentPerformanceMode;

    // LOD Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TArray<float> LODDistances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MaxRenderDistance;

    // Pathfinding
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    UCrowd_PathfindingComponent* PathfindingComponent;

    // Entity Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
    UCrowd_EntitySpawner* EntitySpawner;

    // LOD Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD")
    UCrowd_LODManager* LODManager;

private:
    // Internal state
    bool bIsSimulationActive;
    float LastUpdateTime;
    int32 NextEntityID;
    FVector LastPlayerLocation;

    // Performance tracking
    float AverageFrameTime;
    int32 FrameCounter;

    // Internal methods
    void UpdateEntityBehaviors(float DeltaTime);
    void ProcessEntityCollisions();
    void OptimizeEntityDistribution();
    FVector CalculateFlockingForce(const FCrowd_EntityData& Entity) const;
    FVector CalculateAvoidanceForce(const FCrowd_EntityData& Entity) const;
    bool IsEntityInLODRange(const FCrowd_EntityData& Entity, const FVector& ViewerLocation) const;
};
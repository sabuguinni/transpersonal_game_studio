#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    ECrowd_BehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    int32 EntityID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd Entity")
    float DistanceToTarget;

    FCrowd_EntityData()
    {
        Position = FVector::ZeroVector;
        Velocity = FVector::ZeroVector;
        Speed = 150.0f;
        BehaviorState = ECrowd_BehaviorState::Wandering;
        EntityID = 0;
        DistanceToTarget = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_SpawnZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    int32 CurrentEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Zone")
    ECrowd_BehaviorState DefaultBehavior;

    FCrowd_SpawnZone()
    {
        Center = FVector::ZeroVector;
        Radius = 500.0f;
        MaxEntities = 50;
        CurrentEntities = 0;
        DefaultBehavior = ECrowd_BehaviorState::Wandering;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_MassEntityManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_MassEntityManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Mass Entity Management
    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void InitializeMassSystem();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SpawnCrowdEntities(int32 EntityCount, const FCrowd_SpawnZone& SpawnZone);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetEntityBehaviorState(int32 EntityID, ECrowd_BehaviorState NewState);

    // Crowd Simulation
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ProcessCrowdMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void HandleCrowdCollisions();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdLOD();

    // Pathfinding
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector FindNearestWaypoint(const FVector& EntityPosition);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void AddWaypoint(const FVector& WaypointLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ClearWaypoints();

    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerFleeResponse(const FVector& ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetGatheringPoint(const FVector& GatherLocation);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartPatrolBehavior(const TArray<FVector>& PatrolPoints);

protected:
    // Entity Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entities")
    TArray<FCrowd_EntityData> CrowdEntities;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entities")
    TArray<FCrowd_SpawnZone> SpawnZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
    TArray<FVector> Waypoints;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    int32 MaxCrowdEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float EntityUpdateDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float LODDistance1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float LODDistance2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float CollisionRadius;

    // State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bMassSystemInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    int32 ActiveEntityCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FVector GatheringPoint;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    TArray<FVector> PatrolRoute;

private:
    // Internal Methods
    void UpdateEntityPosition(FCrowd_EntityData& Entity, float DeltaTime);
    void ApplyBehaviorLogic(FCrowd_EntityData& Entity, float DeltaTime);
    FVector CalculateFleeDirection(const FCrowd_EntityData& Entity, const FVector& ThreatLocation);
    FVector CalculateGatherDirection(const FCrowd_EntityData& Entity);
    FVector CalculatePatrolDirection(const FCrowd_EntityData& Entity);
    int32 GetEntityLODLevel(const FCrowd_EntityData& Entity);
    bool CheckEntityCollision(const FCrowd_EntityData& EntityA, const FCrowd_EntityData& EntityB);
};

#include "Crowd_MassEntityManager.generated.h"
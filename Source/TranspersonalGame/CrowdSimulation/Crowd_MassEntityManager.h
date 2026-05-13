#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Crowd_MassEntityManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    int32 EntityCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    FVector MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity")
    bool bIsActive;

    FCrowd_EntityCluster()
    {
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 500.0f;
        EntityCount = 100;
        MovementSpeed = 200.0f;
        MovementDirection = FVector::ForwardVector;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxHighDetailEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxMediumDetailEntities;

    FCrowd_LODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 8000.0f;
        MaxHighDetailEntities = 50;
        MaxMediumDetailEntities = 200;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_EntityBehavior
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FlockingStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SeparationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float CohesionStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bAvoidObstacles;

    FCrowd_EntityBehavior()
    {
        WanderRadius = 300.0f;
        FlockingStrength = 0.5f;
        SeparationDistance = 100.0f;
        CohesionStrength = 0.3f;
        bAvoidObstacles = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity Settings")
    TArray<FCrowd_EntityCluster> EntityClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity Settings")
    FCrowd_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Entity Settings")
    FCrowd_EntityBehavior DefaultBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTotalEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCulling;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CreateEntityCluster(FVector Location, int32 EntityCount, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void DestroyEntityCluster(int32 ClusterIndex);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateClusterMovement(FCrowd_EntityCluster& Cluster, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void ApplyLODOptimization();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void UpdateEntityBehaviors(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    FVector CalculateFlockingForce(const FCrowd_EntityCluster& Cluster, FVector EntityPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    FVector CalculateSeparationForce(const FCrowd_EntityCluster& Cluster, FVector EntityPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    FVector CalculateCohesionForce(const FCrowd_EntityCluster& Cluster, FVector EntityPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    bool IsEntityVisible(FVector EntityPosition);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void CullDistantEntities();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    int32 GetActiveEntityCount();

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SetClusterBehavior(int32 ClusterIndex, const FCrowd_EntityBehavior& NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void MergeClusters(int32 ClusterA, int32 ClusterB);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void SplitCluster(int32 ClusterIndex, int32 NewClusterSize);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void HandleClusterCollision(FCrowd_EntityCluster& ClusterA, FCrowd_EntityCluster& ClusterB);

    UFUNCTION(BlueprintCallable, Category = "Mass Entity")
    void OptimizePerformance();

private:
    float LastUpdateTime;
    int32 CurrentActiveEntities;
    TArray<FVector> EntityPositions;
    TArray<FVector> EntityVelocities;

    void InitializeEntityData();
    void UpdateEntityPositions(float DeltaTime);
    void ProcessEntityInteractions();
    FVector GetPlayerLocation();
    float CalculateDistanceToPlayer(FVector EntityPosition);
    void UpdateClusterLOD(FCrowd_EntityCluster& Cluster);
};
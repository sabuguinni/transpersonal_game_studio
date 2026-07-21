#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingNetwork.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float TrafficDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    ECrowd_PathType PathType;

    FCrowd_PathfindingNode()
    {
        Position = FVector::ZeroVector;
        TrafficDensity = 0.0f;
        PathType = ECrowd_PathType::Normal;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PathfindingNetwork : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PathfindingNetwork();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NetworkMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathfindingNode> PathNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float NodeSpacing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 MaxNodesPerRing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float NetworkRadius;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void GenerateCircularNetwork(FVector Center, float Radius, int32 NodeCount);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void GenerateGridNetwork(FVector Origin, int32 GridSize, float CellSize);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<int32> FindPath(int32 StartNode, int32 EndNode);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetNearestNode(FVector WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateTrafficDensity(int32 NodeIndex, float DensityChange);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    float GetPathCost(int32 FromNode, int32 ToNode);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void OptimizeNetworkForPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawNetwork();

private:
    void ConnectNearbyNodes();
    void CalculateNodeConnections();
    float CalculateHeuristic(int32 NodeA, int32 NodeB);
};
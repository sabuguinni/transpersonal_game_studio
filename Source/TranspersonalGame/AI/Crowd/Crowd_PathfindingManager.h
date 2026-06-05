#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 NodeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    FCrowd_PathNode()
    {
        Position = FVector::ZeroVector;
        Weight = 1.0f;
        NodeID = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FMassEntityHandle EntityHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsValid;

    FCrowd_PathRequest()
    {
        StartLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
        bIsValid = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PathfindingManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PathfindingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Network")
    TArray<FCrowd_PathNode> PathNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Network")
    float MaxPathDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Network")
    int32 MaxSimultaneousRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PathfindingBudgetPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPathNodesPerRequest;

private:
    UPROPERTY()
    TArray<FCrowd_PathRequest> PendingRequests;

    UPROPERTY()
    UNavigationSystemV1* NavSystem;

    UPROPERTY()
    ARecastNavMesh* NavMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool RequestPath(const FVector& Start, const FVector& Target, FMassEntityHandle EntityHandle, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GetPath(const FVector& Start, const FVector& Target);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding Network")
    void BuildPathNetwork();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding Network")
    void AddPathNode(const FVector& Position, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding Network")
    void ConnectNodes(int32 NodeA, int32 NodeB);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ProcessPathRequests(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawPathNetwork();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawPath(const TArray<FVector>& Path);
};
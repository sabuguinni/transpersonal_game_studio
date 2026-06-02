#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AgentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid;

    FCrowd_PathfindingRequest()
    {
        StartLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        AgentRadius = 50.0f;
        RequestID = -1;
        bIsValid = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PathLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPathFound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RequestID;

    FCrowd_PathResult()
    {
        PathLength = 0.0f;
        bPathFound = false;
        RequestID = -1;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PathfindingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Pathfinding request management
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 RequestPath(const FVector& StartLocation, const FVector& TargetLocation, float AgentRadius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool GetPathResult(int32 RequestID, FCrowd_PathResult& OutResult);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void CancelPathRequest(int32 RequestID);

    // Batch pathfinding for crowd optimization
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void ProcessBatchPathfinding(const TArray<FCrowd_PathfindingRequest>& Requests);

    // Navigation mesh validation
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationNavigable(const FVector& Location, float AgentRadius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector GetRandomNavigablePoint(const FVector& Origin, float Radius = 1000.0f);

    // Flow field generation for crowd movement
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void GenerateFlowField(const FVector& TargetLocation, float CellSize = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector GetFlowFieldDirection(const FVector& Location);

protected:
    // Internal pathfinding processing
    void ProcessPendingRequests();
    bool FindPathInternal(const FCrowd_PathfindingRequest& Request, FCrowd_PathResult& OutResult);
    void OptimizePath(TArray<FVector>& PathPoints);

private:
    UPROPERTY()
    TArray<FCrowd_PathfindingRequest> PendingRequests;

    UPROPERTY()
    TMap<int32, FCrowd_PathResult> CompletedPaths;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    int32 MaxRequestsPerFrame;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    float PathOptimizationDistance;

    UPROPERTY(EditAnywhere, Category = "Pathfinding Settings")
    bool bUseAsyncPathfinding;

    // Flow field data
    UPROPERTY()
    TMap<FVector2D, FVector> FlowFieldGrid;

    UPROPERTY(EditAnywhere, Category = "Flow Field")
    float FlowFieldCellSize;

    UPROPERTY(EditAnywhere, Category = "Flow Field")
    FVector FlowFieldTarget;

    int32 NextRequestID;
    class UNavigationSystemV1* NavSystem;
};
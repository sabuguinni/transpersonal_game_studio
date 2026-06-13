#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingSystem.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_Waypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    int32 WaypointID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    TArray<int32> ConnectedWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waypoint")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    bool bPathFound = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    float PathLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Request")
    int32 CurrentPathIndex = 0;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PathfindingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_Waypoint> WaypointNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathRequest> ActivePathRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 MaxPathRequests = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseNavigationMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseWaypointNetwork = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float WaypointSpacing = 500.0f;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void InitializePathfindingSystem();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void GenerateWaypointNetwork();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 RequestPath(int32 AgentID, const FVector& StartLocation, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool GetPathForAgent(int32 AgentID, TArray<FVector>& OutPathPoints);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextPathPoint(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool AdvancePathPoint(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ClearPathRequest(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 FindNearestWaypoint(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<int32> FindPathBetweenWaypoints(int32 StartWaypointID, int32 EndWaypointID);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsLocationReachable(const FVector& StartLocation, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdatePathRequests();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetActivePathRequestCount() const;

private:
    void ProcessPathRequest(FCrowd_PathRequest& PathRequest);
    bool FindNavMeshPath(const FVector& Start, const FVector& End, TArray<FVector>& OutPath);
    bool FindWaypointPath(const FVector& Start, const FVector& End, TArray<FVector>& OutPath);
    void ConnectNearbyWaypoints();
    
    float PathUpdateTimer = 0.0f;
    float PathUpdateInterval = 0.1f;
    int32 PathRequestsPerFrame = 5;
    int32 CurrentRequestIndex = 0;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingSystem.generated.h"

USTRUCT(BlueprintType)
struct FCrowd_PathfindingRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> PathPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsPathValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathLength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 CurrentWaypointIndex = 0;
};

USTRUCT(BlueprintType)
struct FCrowd_NavigationArea
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    int32 MaxAgentsInArea = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TArray<int32> AgentsInArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    bool bIsHighTrafficArea = false;
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
    int32 MaxConcurrentPaths = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float WaypointReachDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathfindingRequest> ActivePathRequests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_NavigationArea> NavigationAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bEnableFlowFields = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bEnableAvoidance = true;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void InitializePathfindingSystem();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 RequestPath(int32 AgentID, const FVector& StartLocation, const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool GetPathForAgent(int32 AgentID, FCrowd_PathfindingRequest& OutPathRequest);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdatePathfinding(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextWaypoint(int32 AgentID, const FVector& CurrentLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsWaypointReached(const FVector& AgentLocation, const FVector& WaypointLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void AdvanceToNextWaypoint(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void CreateNavigationAreas();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateNavigationAreas();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector CalculateAvoidanceForce(int32 AgentID, const FVector& AgentLocation, const FVector& DesiredVelocity);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> GenerateFlowField(const FVector& TargetLocation, float FieldRadius);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ClearCompletedPaths();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetActivePathCount() const;

private:
    float PathfindingTimer = 0.0f;
    int32 NextRequestID = 0;
    
    UPROPERTY()
    class UNavigationSystemV1* NavigationSystem = nullptr;
};
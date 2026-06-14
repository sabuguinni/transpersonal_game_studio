#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_Waypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsActive;

    FCrowd_Waypoint()
    {
        Location = FVector::ZeroVector;
        Radius = 200.0f;
        Priority = 1;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MovementCost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsBlocked;

    FCrowd_PathNode()
    {
        Position = FVector::ZeroVector;
        MovementCost = 1.0f;
        bIsBlocked = false;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Pathfinding Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // Waypoint System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_Waypoint> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathNode> PathNodes;

    // Pathfinding Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float WaypointRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float MaxPathDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxPathNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bEnableDynamicObstacles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bEnableFlowFields;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float PathUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxConcurrentPaths;

    // Pathfinding Methods
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector> FindPath(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNearestWaypoint(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsPathBlocked(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void AddDynamicObstacle(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void RemoveDynamicObstacle(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdatePathNetwork();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void OptimizePathNodes();

    // Flow Field Methods
    UFUNCTION(BlueprintCallable, Category = "FlowField")
    FVector GetFlowFieldDirection(const FVector& Location, const FVector& Target);

    UFUNCTION(BlueprintCallable, Category = "FlowField")
    void GenerateFlowField(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "FlowField")
    void ClearFlowField();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawPathNetwork();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawWaypoints();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void TogglePathVisualization();

private:
    // Internal pathfinding data
    TArray<FVector> DynamicObstacles;
    TArray<float> ObstacleRadii;
    TMap<FVector, FVector> FlowField;
    float LastPathUpdate;
    bool bPathVisualizationEnabled;

    // Internal methods
    void InitializePathNetwork();
    void UpdateDynamicObstacles();
    float CalculatePathCost(const FVector& Start, const FVector& End);
    bool IsLocationBlocked(const FVector& Location);
    void GeneratePathNodes();
    void ConnectPathNodes();
};
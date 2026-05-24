#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NodeIndex;

    FCrowd_PathNode()
    {
        Location = FVector::ZeroVector;
        Cost = 0.0f;
        bIsBlocked = false;
        NodeIndex = -1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AgentID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGroupPath;

    FCrowd_PathRequest()
    {
        StartLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        AgentID = -1;
        Priority = 1.0f;
        bIsGroupPath = false;
    }
};

/**
 * Advanced pathfinding manager for massive crowd simulation
 * Handles up to 50,000 simultaneous path requests using hierarchical pathfinding
 * Integrates with UE5 Navigation System and Mass Entity framework
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PathfindingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core pathfinding functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool RequestPath(const FCrowd_PathRequest& Request);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    TArray<FVector> GetPath(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void CancelPathRequest(int32 AgentID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsPathReady(int32 AgentID);

    // Hierarchical pathfinding
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void BuildHierarchicalGraph();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    TArray<FVector> FindHierarchicalPath(const FVector& Start, const FVector& Goal);

    // Flow field pathfinding for groups
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void GenerateFlowField(const FVector& TargetLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector GetFlowDirection(const FVector& Location);

    // Dynamic obstacle avoidance
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void RegisterDynamicObstacle(AActor* Obstacle);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UnregisterDynamicObstacle(AActor* Obstacle);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationBlocked(const FVector& Location, float Radius = 100.0f);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void SetMaxPathRequestsPerFrame(int32 MaxRequests);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void EnableLODPathfinding(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void SetPathfindingLOD(int32 AgentID, int32 LODLevel);

    // Debug and visualization
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding", CallInEditor = true)
    void DebugDrawPaths();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding", CallInEditor = true)
    void DebugDrawFlowField();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 GetActivePathCount() const { return ActivePaths.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 GetPendingRequestCount() const { return PendingRequests.Num(); }

protected:
    // Navigation system integration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
    class UNavigationSystemV1* NavSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Navigation")
    class ANavMeshBoundsVolume* NavMeshBounds;

    // Pathfinding data structures
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    TMap<int32, TArray<FVector>> ActivePaths;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    TArray<FCrowd_PathRequest> PendingRequests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    TArray<FCrowd_PathNode> HierarchicalNodes;

    // Flow field data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Flow Field")
    TMap<FIntPoint, FVector> FlowFieldGrid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    float FlowFieldCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    FVector FlowFieldTarget;

    // Dynamic obstacles
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Obstacles")
    TArray<AActor*> DynamicObstacles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacles")
    float ObstacleUpdateFrequency;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPathRequestsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseLODPathfinding;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PathUpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePathsLOD0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePathsLOD1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActivePathsLOD2;

    // Debug settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawDebugPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDrawFlowField;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    float DebugDrawDuration;

private:
    // Internal pathfinding functions
    void ProcessPathRequests(float DeltaTime);
    void UpdateFlowField();
    void UpdateDynamicObstacles();
    void CleanupCompletedPaths();
    
    // Hierarchical pathfinding helpers
    void BuildClusterGraph();
    TArray<int32> FindClusterPath(int32 StartCluster, int32 GoalCluster);
    TArray<FVector> RefineClusterPath(const TArray<int32>& ClusterPath, const FVector& Start, const FVector& Goal);

    // Flow field helpers
    FIntPoint WorldToGridCoord(const FVector& WorldLocation);
    FVector GridToWorldCoord(const FIntPoint& GridCoord);
    void CalculateFlowFieldCell(const FIntPoint& GridCoord);

    // Performance tracking
    float LastPathUpdateTime;
    float LastObstacleUpdateTime;
    int32 PathRequestsProcessedThisFrame;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "AI/Navigation/NavigationTypes.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 MaxOccupants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    FCrowd_PathfindingNode()
    {
        Location = FVector::ZeroVector;
        Priority = 1.0f;
        bIsOccupied = false;
        MaxOccupants = 5;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_NavigationPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float PathLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    bool bIsValid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Navigation")
    float EstimatedTravelTime;

    FCrowd_NavigationPath()
    {
        PathLength = 0.0f;
        bIsValid = false;
        EstimatedTravelTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PathfindingManager : public UObject
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingManager();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void InitializePathfindingSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FCrowd_NavigationPath FindPathBetweenPoints(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationNavigable(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    TArray<FVector> GetNearbyNavigableLocations(const FVector& CenterLocation, float SearchRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void RegisterPathfindingNode(const FCrowd_PathfindingNode& Node);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UpdateNodeOccupancy(int32 NodeIndex, bool bOccupied);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector GetOptimalGatheringPoint(const TArray<FVector>& CrowdPositions);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void OptimizePathForCrowdDensity(FCrowd_NavigationPath& Path, float CrowdDensity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathfindingNode> PathfindingNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxPathfindingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float NodeSearchRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 MaxPathfindingIterations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseHierarchicalPathfinding;

private:
    UNavigationSystemV1* NavigationSystem;
    
    void BuildPathfindingGraph();
    float CalculatePathCost(const TArray<FVector>& Waypoints);
    bool ValidatePathSafety(const TArray<FVector>& Waypoints);
    TArray<FVector> SmoothPath(const TArray<FVector>& RawPath);
};
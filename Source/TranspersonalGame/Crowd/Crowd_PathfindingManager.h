#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "AI/NavigationSystemBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Cost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsBlocked;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<int32> ConnectedNodes;

    FCrowd_PathNode()
    {
        Location = FVector::ZeroVector;
        Cost = 1.0f;
        bIsBlocked = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathfindingRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AgentRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 RequestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsCompleted;

    FCrowd_PathfindingRequest()
    {
        StartLocation = FVector::ZeroVector;
        TargetLocation = FVector::ZeroVector;
        AgentRadius = 50.0f;
        RequestID = -1;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_PathfindingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Pathfinding functions
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 RequestPath(const FVector& StartLocation, const FVector& TargetLocation, float AgentRadius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool GetPathResult(int32 RequestID, TArray<FVector>& OutPath);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void CancelPathRequest(int32 RequestID);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsPathValid(const TArray<FVector>& Path);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector GetRandomReachablePoint(const FVector& Origin, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void UpdateNavMeshData();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool CanReachLocation(const FVector& StartLocation, const FVector& TargetLocation);

    // Crowd flow management
    UFUNCTION(BlueprintCallable, Category = "Crowd Flow")
    void RegisterCrowdDensity(const FVector& Location, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd Flow")
    float GetCrowdDensityAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Crowd Flow")
    FVector GetFlowDirection(const FVector& Location);

protected:
    // Internal pathfinding data
    UPROPERTY()
    TArray<FCrowd_PathfindingRequest> ActiveRequests;

    UPROPERTY()
    TMap<int32, TArray<FVector>> CompletedPaths;

    UPROPERTY()
    TMap<FVector, float> CrowdDensityMap;

    UPROPERTY()
    class UNavigationSystemV1* NavSystem;

    // Request management
    int32 NextRequestID;
    
    // Internal helper functions
    void ProcessPathfindingRequests();
    bool FindPathInternal(const FCrowd_PathfindingRequest& Request, TArray<FVector>& OutPath);
    void CleanupCompletedRequests();
    FVector QuantizeLocation(const FVector& Location, float GridSize = 100.0f);

    // Crowd density tracking
    void UpdateCrowdDensityGrid();
    void DecayCrowdDensity(float DeltaTime);

private:
    FTimerHandle PathfindingTimerHandle;
    FTimerHandle DensityUpdateTimerHandle;
    
    static const float PATHFINDING_UPDATE_INTERVAL;
    static const float DENSITY_DECAY_RATE;
    static const int32 MAX_ACTIVE_REQUESTS;
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Engine/World.h"
#include "Crowd_PathfindingSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Cost = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsBlocked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 NodeID = -1;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 RequestID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FVector> PathPoints;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowd_PathfindingSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_PathfindingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Request a path for crowd entity
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 RequestPath(const FVector& StartLocation, const FVector& TargetLocation, float Priority = 1.0f);

    // Get path result if ready
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool GetPathResult(int32 RequestID, TArray<FVector>& OutPath);

    // Check if path request is complete
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsPathReady(int32 RequestID);

    // Cancel path request
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void CancelPathRequest(int32 RequestID);

    // Get navigation mesh bounds
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FBox GetNavigationBounds();

    // Check if location is navigable
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationNavigable(const FVector& Location, float Radius = 50.0f);

    // Find nearest navigable location
    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector FindNearestNavigableLocation(const FVector& Location, float SearchRadius = 500.0f);

protected:
    // Process pending path requests
    void ProcessPathRequests();

    // Calculate path using navigation mesh
    bool CalculatePath(const FVector& Start, const FVector& End, TArray<FVector>& OutPath);

    // Smooth path for crowd movement
    void SmoothPath(TArray<FVector>& Path);

    // Check for dynamic obstacles
    bool CheckForObstacles(const FVector& Start, const FVector& End);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    class UNavigationSystemV1* NavigationSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Settings", meta = (AllowPrivateAccess = "true"))
    int32 MaxPathRequestsPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Settings", meta = (AllowPrivateAccess = "true"))
    float PathSmoothingRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Settings", meta = (AllowPrivateAccess = "true"))
    float ObstacleCheckRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Settings", meta = (AllowPrivateAccess = "true"))
    float MaxPathLength = 5000.0f;

    // Path request queue
    UPROPERTY()
    TArray<FCrowd_PathRequest> PendingRequests;

    UPROPERTY()
    TArray<FCrowd_PathRequest> CompletedRequests;

    // Request ID counter
    int32 NextRequestID = 1;

    // Performance tracking
    float LastProcessTime = 0.0f;
    int32 PathsProcessedThisFrame = 0;
};
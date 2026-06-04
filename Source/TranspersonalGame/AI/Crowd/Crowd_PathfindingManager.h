#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "SharedTypes.h"
#include "Crowd_PathfindingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathRequest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector StartLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EndLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EntityID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsComplete = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PathPoints;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_FlowField
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D GridOrigin = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CellSize = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridWidth = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GridHeight = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector2D> FlowDirections;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation = FVector::ZeroVector;
};

UCLASS(BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PathfindingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void RequestPath(const FCrowd_PathRequest& PathRequest);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void GenerateFlowField(FVector TargetLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    FVector2D GetFlowDirection(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void ProcessPathRequests(int32 MaxRequestsPerFrame = 10);

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    bool IsLocationNavigable(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    void ClearAllPaths();

    UFUNCTION(BlueprintCallable, Category = "Crowd Pathfinding")
    int32 GetPendingRequestCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Config")
    int32 MaxPathRequestsPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding Config")
    float PathfindingTimeout = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow Field")
    FCrowd_FlowField CurrentFlowField;

    UPROPERTY()
    TArray<FCrowd_PathRequest> PendingPathRequests;

    UPROPERTY()
    TArray<FCrowd_PathRequest> CompletedPathRequests;

    UPROPERTY()
    TObjectPtr<UNavigationSystemV1> NavigationSystem;

    void InitializeNavigationSystem();
    void UpdateFlowField();
    bool FindPathToLocation(const FVector& Start, const FVector& End, TArray<FVector>& OutPath);
    void CalculateFlowFieldDirections();
    FVector2D WorldToGrid(FVector WorldLocation) const;
    FVector GridToWorld(FVector2D GridCoord) const;
    bool IsValidGridCoordinate(int32 X, int32 Y) const;
};
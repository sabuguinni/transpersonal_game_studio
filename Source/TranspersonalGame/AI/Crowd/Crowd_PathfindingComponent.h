#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_PathfindingComponent.generated.h"

UENUM(BlueprintType)
enum class ECrowd_PathfindingState : uint8
{
    Idle,
    Planning,
    Following,
    Blocked,
    Arrived
};

USTRUCT(BlueprintType)
struct FCrowd_PathPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsWaypoint = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_PathfindingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_PathfindingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    ECrowd_PathfindingState CurrentState = ECrowd_PathfindingState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathRecalculationInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUseNavMesh = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    TArray<FCrowd_PathPoint> CurrentPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 CurrentPathIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool SetDestination(const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool SetWaypointPath(const TArray<FVector>& Waypoints);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void StopMovement();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsPathValid() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void RecalculatePath();

protected:
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool FindPathToLocation(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void UpdateMovement(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool HasReachedCurrentTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void AdvanceToNextPathPoint();

private:
    FVector TargetDestination;
    float LastPathCalculationTime;
    UNavigationSystemV1* NavigationSystem;
    bool bHasValidPath;
    
    UPROPERTY()
    TObjectPtr<APawn> OwnerPawn;
};
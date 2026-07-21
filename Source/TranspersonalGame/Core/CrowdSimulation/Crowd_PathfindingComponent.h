#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Crowd_PathfindingComponent.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_Waypoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Pathfinding")
    FVector Position;

    UPROPERTY(BlueprintReadWrite, Category = "Pathfinding")
    float Radius;

    UPROPERTY(BlueprintReadWrite, Category = "Pathfinding")
    bool bIsOccupied;

    UPROPERTY(BlueprintReadWrite, Category = "Pathfinding")
    int32 Priority;

    FCrowd_Waypoint()
    {
        Position = FVector::ZeroVector;
        Radius = 100.0f;
        bIsOccupied = false;
        Priority = 0;
    }
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

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetDestination(const FVector& TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void AddWaypoint(const FVector& WaypointLocation, float WaypointRadius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsAtDestination() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void PauseMovement(bool bPause);

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    TArray<FCrowd_Waypoint> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    FVector CurrentDestination;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    float AcceptanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pathfinding", meta = (AllowPrivateAccess = "true"))
    int32 CurrentWaypointIndex;

    void MoveTowardsDestination(float DeltaTime);
    FVector FindNearestWaypoint(const FVector& FromLocation);
    void UpdateWaypointOccupancy();
};
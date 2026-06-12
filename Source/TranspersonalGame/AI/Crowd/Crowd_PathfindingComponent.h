#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "AI/NavigationSystemBase.h"
#include "Crowd_PathfindingComponent.generated.h"

UENUM(BlueprintType)
enum class ECrowd_PathfindingState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Seeking UMETA(DisplayName = "Seeking Path"),
    Following UMETA(DisplayName = "Following Path"),
    Avoiding UMETA(DisplayName = "Avoiding Obstacle"),
    Blocked UMETA(DisplayName = "Path Blocked"),
    Arrived UMETA(DisplayName = "Destination Reached")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowd_PathNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsBlocked;

    FCrowd_PathNode()
    {
        Location = FVector::ZeroVector;
        Radius = 50.0f;
        bIsBlocked = false;
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

    // Pathfinding state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    ECrowd_PathfindingState CurrentState;

    // Current path
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    TArray<FCrowd_PathNode> CurrentPath;

    // Current path index
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    int32 CurrentPathIndex;

    // Target destination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector TargetDestination;

    // Movement speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MovementSpeed;

    // Pathfinding radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathfindingRadius;

    // Arrival threshold
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float ArrivalThreshold;

    // Obstacle avoidance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Avoidance")
    float AvoidanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Avoidance")
    float AvoidanceForce;

    // Path recalculation timer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathRecalculationInterval;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool SetDestination(const FVector& NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ClearPath();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsPathValid() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextPathPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    float GetDistanceToDestination() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void PausePathfinding();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ResumePathfinding();

private:
    // Internal pathfinding logic
    bool FindPath(const FVector& Start, const FVector& End);
    void UpdatePathFollowing(float DeltaTime);
    void HandleObstacleAvoidance(float DeltaTime);
    bool IsDestinationReachable(const FVector& Destination) const;
    void RecalculatePathIfNeeded();

    // Navigation system reference
    UPROPERTY()
    UNavigationSystemV1* NavigationSystem;

    // Timers
    float PathRecalculationTimer;
    float LastPathUpdateTime;

    // Internal state
    bool bIsPaused;
    bool bPathNeedsRecalculation;
    FVector LastKnownPosition;
};
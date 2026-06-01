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
    FindingPath,
    FollowingPath,
    Blocked,
    ReachedDestination,
    Failed
};

USTRUCT(BlueprintType)
struct FCrowd_PathPoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
    float Radius = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
    bool bIsWaypoint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path")
    float WaitTime = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_PathfindingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AcceptanceRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MaxPathLength = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bUsePartialPaths = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bProjectDestinationToNavigation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float PathRecalculationInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float MovementSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bAvoidOtherAgents = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    float AvoidanceRadius = 150.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathfindingStateChanged, ECrowd_PathfindingState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestinationReached, FVector, Destination);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPathfindingFailed, FVector, FailedDestination);

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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
    FCrowd_PathfindingSettings PathfindingSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    ECrowd_PathfindingState CurrentState = ECrowd_PathfindingState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    FVector CurrentDestination = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    TArray<FCrowd_PathPoint> CurrentPath;

    UPROPERTY(BlueprintReadOnly, Category = "Pathfinding")
    int32 CurrentPathIndex = 0;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPathfindingStateChanged OnPathfindingStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnDestinationReached OnDestinationReached;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPathfindingFailed OnPathfindingFailed;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool SetDestination(const FVector& Destination);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void StopPathfinding();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void PausePathfinding();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void ResumePathfinding();

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    FVector GetNextPathPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    float GetDistanceToDestination() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool IsPathfindingActive() const;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    void SetMovementSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool HasValidPath() const;

private:
    UPROPERTY()
    TObjectPtr<UNavigationSystemV1> NavigationSystem;

    UPROPERTY()
    TObjectPtr<AActor> OwnerActor;

    float LastPathRecalculationTime = 0.0f;
    bool bIsPaused = false;
    FVector LastOwnerLocation = FVector::ZeroVector;

    void SetPathfindingState(ECrowd_PathfindingState NewState);
    bool FindPathToDestination(const FVector& Destination);
    void UpdatePathFollowing(float DeltaTime);
    bool ShouldRecalculatePath() const;
    FVector GetAvoidanceVector() const;
    bool IsNearOtherAgents(float CheckRadius) const;
    void MoveTowardsNextPoint(float DeltaTime);
    bool HasReachedCurrentPathPoint() const;
    void AdvanceToNextPathPoint();
};
#include "Crowd_PathfindingComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCrowd_PathfindingComponent::UCrowd_PathfindingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentState = ECrowd_PathfindingState::Idle;
    CurrentPathIndex = 0;
    TargetDestination = FVector::ZeroVector;
    MovementSpeed = 300.0f;
    PathfindingRadius = 50.0f;
    ArrivalThreshold = 100.0f;
    AvoidanceRadius = 150.0f;
    AvoidanceForce = 500.0f;
    PathRecalculationInterval = 2.0f;
    
    // Internal state
    PathRecalculationTimer = 0.0f;
    LastPathUpdateTime = 0.0f;
    bIsPaused = false;
    bPathNeedsRecalculation = false;
    LastKnownPosition = FVector::ZeroVector;
    NavigationSystem = nullptr;
}

void UCrowd_PathfindingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get navigation system reference
    if (UWorld* World = GetWorld())
    {
        NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!NavigationSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Navigation system not found"));
        }
    }
    
    // Initialize position tracking
    if (AActor* Owner = GetOwner())
    {
        LastKnownPosition = Owner->GetActorLocation();
    }
}

void UCrowd_PathfindingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsPaused || !NavigationSystem)
    {
        return;
    }
    
    // Update timers
    PathRecalculationTimer += DeltaTime;
    LastPathUpdateTime += DeltaTime;
    
    // Check if we need to recalculate path
    if (PathRecalculationTimer >= PathRecalculationInterval)
    {
        RecalculatePathIfNeeded();
        PathRecalculationTimer = 0.0f;
    }
    
    // Update pathfinding behavior based on current state
    switch (CurrentState)
    {
        case ECrowd_PathfindingState::Seeking:
            // Path calculation is handled in SetDestination
            break;
            
        case ECrowd_PathfindingState::Following:
            UpdatePathFollowing(DeltaTime);
            break;
            
        case ECrowd_PathfindingState::Avoiding:
            HandleObstacleAvoidance(DeltaTime);
            break;
            
        case ECrowd_PathfindingState::Blocked:
            // Try to find alternative path
            if (LastPathUpdateTime > 1.0f)
            {
                bPathNeedsRecalculation = true;
                LastPathUpdateTime = 0.0f;
            }
            break;
            
        default:
            break;
    }
    
    // Update position tracking
    if (AActor* Owner = GetOwner())
    {
        LastKnownPosition = Owner->GetActorLocation();
    }
}

bool UCrowd_PathfindingComponent::SetDestination(const FVector& NewDestination)
{
    if (!NavigationSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Cannot set destination - no navigation system"));
        return false;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return false;
    }
    
    TargetDestination = NewDestination;
    CurrentState = ECrowd_PathfindingState::Seeking;
    
    // Check if destination is reachable
    if (!IsDestinationReachable(NewDestination))
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Destination is not reachable"));
        CurrentState = ECrowd_PathfindingState::Blocked;
        return false;
    }
    
    // Find path to destination
    FVector StartLocation = Owner->GetActorLocation();
    if (FindPath(StartLocation, NewDestination))
    {
        CurrentState = ECrowd_PathfindingState::Following;
        CurrentPathIndex = 0;
        return true;
    }
    else
    {
        CurrentState = ECrowd_PathfindingState::Blocked;
        return false;
    }
}

void UCrowd_PathfindingComponent::ClearPath()
{
    CurrentPath.Empty();
    CurrentPathIndex = 0;
    CurrentState = ECrowd_PathfindingState::Idle;
    TargetDestination = FVector::ZeroVector;
}

bool UCrowd_PathfindingComponent::IsPathValid() const
{
    return CurrentPath.Num() > 0 && CurrentPathIndex < CurrentPath.Num();
}

FVector UCrowd_PathfindingComponent::GetNextPathPoint() const
{
    if (IsPathValid())
    {
        return CurrentPath[CurrentPathIndex].Location;
    }
    return FVector::ZeroVector;
}

float UCrowd_PathfindingComponent::GetDistanceToDestination() const
{
    if (AActor* Owner = GetOwner())
    {
        return FVector::Dist(Owner->GetActorLocation(), TargetDestination);
    }
    return -1.0f;
}

void UCrowd_PathfindingComponent::PausePathfinding()
{
    bIsPaused = true;
}

void UCrowd_PathfindingComponent::ResumePathfinding()
{
    bIsPaused = false;
}

bool UCrowd_PathfindingComponent::FindPath(const FVector& Start, const FVector& End)
{
    if (!NavigationSystem)
    {
        return false;
    }
    
    // Clear existing path
    CurrentPath.Empty();
    
    // Use UE5 navigation system to find path
    FNavLocation StartNavLocation, EndNavLocation;
    
    // Project start and end points to navigation mesh
    if (!NavigationSystem->ProjectPointToNavigation(Start, StartNavLocation, FVector(PathfindingRadius)))
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Cannot project start point to navmesh"));
        return false;
    }
    
    if (!NavigationSystem->ProjectPointToNavigation(End, EndNavLocation, FVector(PathfindingRadius)))
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Cannot project end point to navmesh"));
        return false;
    }
    
    // Find navigation path
    FPathFindingQuery Query;
    Query.StartLocation = StartNavLocation.Location;
    Query.EndLocation = EndNavLocation.Location;
    Query.Owner = GetOwner();
    
    FPathFindingResult Result = NavigationSystem->FindPathSync(Query);
    
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        // Convert navigation path to our path nodes
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        
        for (const FNavPathPoint& Point : PathPoints)
        {
            FCrowd_PathNode Node;
            Node.Location = Point.Location;
            Node.Radius = PathfindingRadius;
            Node.bIsBlocked = false;
            CurrentPath.Add(Node);
        }
        
        UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingComponent: Path found with %d nodes"), CurrentPath.Num());
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Path finding failed"));
    return false;
}

void UCrowd_PathfindingComponent::UpdatePathFollowing(float DeltaTime)
{
    if (!IsPathValid())
    {
        CurrentState = ECrowd_PathfindingState::Idle;
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetPoint = CurrentPath[CurrentPathIndex].Location;
    
    // Check if we've reached the current path point
    float DistanceToPoint = FVector::Dist2D(CurrentLocation, TargetPoint);
    
    if (DistanceToPoint <= ArrivalThreshold)
    {
        // Move to next path point
        CurrentPathIndex++;
        
        if (CurrentPathIndex >= CurrentPath.Num())
        {
            // Reached destination
            CurrentState = ECrowd_PathfindingState::Arrived;
            UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingComponent: Destination reached"));
            return;
        }
    }
    
    // Move towards current path point
    FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;
    
    // Simple obstacle avoidance check
    if (GetWorld())
    {
        FHitResult HitResult;
        FVector TraceStart = CurrentLocation;
        FVector TraceEnd = NewLocation;
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            // Obstacle detected - switch to avoidance mode
            CurrentState = ECrowd_PathfindingState::Avoiding;
            return;
        }
    }
    
    // Apply movement (this would typically be handled by a movement component)
    // For now, we just update the actor's location directly
    Owner->SetActorLocation(NewLocation);
}

void UCrowd_PathfindingComponent::HandleObstacleAvoidance(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || !GetWorld())
    {
        return;
    }
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector AvoidanceDirection = FVector::ZeroVector;
    
    // Perform sphere trace to detect nearby obstacles
    TArray<FHitResult> HitResults;
    FVector TraceStart = CurrentLocation;
    FVector TraceEnd = CurrentLocation; // Same location for sphere overlap
    
    if (GetWorld()->SweepMultiByChannel(
        HitResults,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(AvoidanceRadius)))
    {
        // Calculate avoidance direction
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.GetActor() && Hit.GetActor() != Owner)
            {
                FVector ToObstacle = Hit.Location - CurrentLocation;
                ToObstacle.Normalize();
                AvoidanceDirection -= ToObstacle; // Move away from obstacle
            }
        }
    }
    
    if (!AvoidanceDirection.IsNearlyZero())
    {
        // Apply avoidance movement
        FVector NewLocation = CurrentLocation + AvoidanceDirection * AvoidanceForce * DeltaTime;
        Owner->SetActorLocation(NewLocation);
        
        // Check if we can return to path following
        if (FVector::Dist(CurrentLocation, GetNextPathPoint()) < AvoidanceRadius * 1.5f)
        {
            CurrentState = ECrowd_PathfindingState::Following;
        }
    }
    else
    {
        // No obstacles detected, return to path following
        CurrentState = ECrowd_PathfindingState::Following;
    }
}

bool UCrowd_PathfindingComponent::IsDestinationReachable(const FVector& Destination) const
{
    if (!NavigationSystem)
    {
        return false;
    }
    
    FNavLocation NavLocation;
    return NavigationSystem->ProjectPointToNavigation(Destination, NavLocation, FVector(PathfindingRadius));
}

void UCrowd_PathfindingComponent::RecalculatePathIfNeeded()
{
    if (!bPathNeedsRecalculation && CurrentState != ECrowd_PathfindingState::Blocked)
    {
        return;
    }
    
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    // Recalculate path from current position to destination
    FVector CurrentLocation = Owner->GetActorLocation();
    
    if (FindPath(CurrentLocation, TargetDestination))
    {
        CurrentState = ECrowd_PathfindingState::Following;
        CurrentPathIndex = 0;
        bPathNeedsRecalculation = false;
        UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingComponent: Path recalculated successfully"));
    }
    else
    {
        CurrentState = ECrowd_PathfindingState::Blocked;
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: Path recalculation failed"));
    }
}
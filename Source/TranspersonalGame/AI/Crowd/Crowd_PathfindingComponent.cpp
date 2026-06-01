#include "Crowd_PathfindingComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"

UCrowd_PathfindingComponent::UCrowd_PathfindingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    CurrentState = ECrowd_PathfindingState::Idle;
    LastPathCalculationTime = 0.0f;
    bHasValidPath = false;
    TargetDestination = FVector::ZeroVector;
    OwnerPawn = nullptr;
}

void UCrowd_PathfindingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    OwnerPawn = Cast<APawn>(GetOwner());
    NavigationSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    
    if (!NavigationSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingComponent: No NavigationSystem found"));
        bUseNavMesh = false;
    }
}

void UCrowd_PathfindingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn || CurrentState == ECrowd_PathfindingState::Idle)
    {
        return;
    }

    // Check if we need to recalculate path
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPathCalculationTime > PathRecalculationInterval)
    {
        RecalculatePath();
        LastPathCalculationTime = CurrentTime;
    }

    // Update movement based on current state
    switch (CurrentState)
    {
        case ECrowd_PathfindingState::Following:
            UpdateMovement(DeltaTime);
            break;
            
        case ECrowd_PathfindingState::Blocked:
            // Try to recalculate path more frequently when blocked
            if (CurrentTime - LastPathCalculationTime > 0.5f)
            {
                RecalculatePath();
                LastPathCalculationTime = CurrentTime;
            }
            break;
            
        default:
            break;
    }
}

bool UCrowd_PathfindingComponent::SetDestination(const FVector& Destination)
{
    if (!OwnerPawn)
    {
        return false;
    }

    TargetDestination = Destination;
    CurrentPathIndex = 0;
    CurrentPath.Empty();

    if (FindPathToLocation(Destination))
    {
        CurrentState = ECrowd_PathfindingState::Following;
        return true;
    }
    else
    {
        CurrentState = ECrowd_PathfindingState::Blocked;
        return false;
    }
}

bool UCrowd_PathfindingComponent::SetWaypointPath(const TArray<FVector>& Waypoints)
{
    if (Waypoints.Num() == 0)
    {
        return false;
    }

    CurrentPath.Empty();
    CurrentPathIndex = 0;

    for (const FVector& Waypoint : Waypoints)
    {
        FCrowd_PathPoint PathPoint;
        PathPoint.Location = Waypoint;
        PathPoint.Radius = AcceptanceRadius;
        PathPoint.bIsWaypoint = true;
        CurrentPath.Add(PathPoint);
    }

    bHasValidPath = true;
    CurrentState = ECrowd_PathfindingState::Following;
    return true;
}

void UCrowd_PathfindingComponent::StopMovement()
{
    CurrentState = ECrowd_PathfindingState::Idle;
    CurrentPath.Empty();
    CurrentPathIndex = 0;
    bHasValidPath = false;
}

FVector UCrowd_PathfindingComponent::GetCurrentTarget() const
{
    if (CurrentPath.IsValidIndex(CurrentPathIndex))
    {
        return CurrentPath[CurrentPathIndex].Location;
    }
    return TargetDestination;
}

float UCrowd_PathfindingComponent::GetDistanceToTarget() const
{
    if (!OwnerPawn)
    {
        return -1.0f;
    }

    FVector CurrentTarget = GetCurrentTarget();
    return FVector::Dist(OwnerPawn->GetActorLocation(), CurrentTarget);
}

bool UCrowd_PathfindingComponent::IsPathValid() const
{
    return bHasValidPath && CurrentPath.Num() > 0;
}

void UCrowd_PathfindingComponent::RecalculatePath()
{
    if (TargetDestination != FVector::ZeroVector)
    {
        FindPathToLocation(TargetDestination);
    }
}

bool UCrowd_PathfindingComponent::FindPathToLocation(const FVector& TargetLocation)
{
    if (!OwnerPawn || !NavigationSystem || !bUseNavMesh)
    {
        // Simple direct path without NavMesh
        CurrentPath.Empty();
        FCrowd_PathPoint DirectPoint;
        DirectPoint.Location = TargetLocation;
        DirectPoint.Radius = AcceptanceRadius;
        CurrentPath.Add(DirectPoint);
        bHasValidPath = true;
        return true;
    }

    FVector StartLocation = OwnerPawn->GetActorLocation();
    
    // Use NavigationSystem to find path
    FPathFindingQuery Query;
    Query.StartLocation = StartLocation;
    Query.EndLocation = TargetLocation;
    Query.NavData = NavigationSystem->GetDefaultNavDataInstance();
    
    FPathFindingResult Result = NavigationSystem->FindPathSync(Query);
    
    if (Result.IsSuccessful() && Result.Path.IsValid())
    {
        CurrentPath.Empty();
        
        const TArray<FNavPathPoint>& PathPoints = Result.Path->GetPathPoints();
        for (const FNavPathPoint& NavPoint : PathPoints)
        {
            FCrowd_PathPoint PathPoint;
            PathPoint.Location = NavPoint.Location;
            PathPoint.Radius = AcceptanceRadius;
            CurrentPath.Add(PathPoint);
        }
        
        bHasValidPath = true;
        CurrentPathIndex = 0;
        return true;
    }
    else
    {
        // Fallback to direct path
        CurrentPath.Empty();
        FCrowd_PathPoint DirectPoint;
        DirectPoint.Location = TargetLocation;
        DirectPoint.Radius = AcceptanceRadius;
        CurrentPath.Add(DirectPoint);
        bHasValidPath = true;
        return true;
    }
}

void UCrowd_PathfindingComponent::UpdateMovement(float DeltaTime)
{
    if (!OwnerPawn || !IsPathValid())
    {
        return;
    }

    // Check if we've reached the current target
    if (HasReachedCurrentTarget())
    {
        AdvanceToNextPathPoint();
        return;
    }

    // Move towards current target
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    FVector TargetLocation = GetCurrentTarget();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();

    if (!Direction.IsZero())
    {
        FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
        OwnerPawn->SetActorLocation(NewLocation, true);
        
        // Orient towards movement direction
        FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, TargetLocation);
        OwnerPawn->SetActorRotation(FMath::RInterpTo(OwnerPawn->GetActorRotation(), NewRotation, DeltaTime, 3.0f));
    }
}

bool UCrowd_PathfindingComponent::HasReachedCurrentTarget() const
{
    if (!OwnerPawn || !CurrentPath.IsValidIndex(CurrentPathIndex))
    {
        return false;
    }

    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    FVector TargetLocation = CurrentPath[CurrentPathIndex].Location;
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    return Distance <= CurrentPath[CurrentPathIndex].Radius;
}

void UCrowd_PathfindingComponent::AdvanceToNextPathPoint()
{
    CurrentPathIndex++;
    
    if (CurrentPathIndex >= CurrentPath.Num())
    {
        // Reached the end of the path
        CurrentState = ECrowd_PathfindingState::Arrived;
        StopMovement();
    }
}
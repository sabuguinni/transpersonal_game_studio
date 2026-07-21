#include "Crowd_PathfindingComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_PathfindingComponent::UCrowd_PathfindingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    MovementSpeed = 150.0f;
    AcceptanceRadius = 50.0f;
    bIsMoving = false;
    CurrentWaypointIndex = 0;
    CurrentDestination = FVector::ZeroVector;
}

void UCrowd_PathfindingComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize waypoint system
    Waypoints.Reserve(50);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Pathfinding Component initialized"));
}

void UCrowd_PathfindingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsMoving && !CurrentDestination.IsZero())
    {
        MoveTowardsDestination(DeltaTime);
        UpdateWaypointOccupancy();
    }
}

void UCrowd_PathfindingComponent::SetDestination(const FVector& TargetLocation)
{
    CurrentDestination = TargetLocation;
    bIsMoving = true;
    
    // Find path through waypoints if available
    if (Waypoints.Num() > 0)
    {
        FVector NearestWaypoint = FindNearestWaypoint(GetOwner()->GetActorLocation());
        if (!NearestWaypoint.IsZero())
        {
            CurrentDestination = NearestWaypoint;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Pathfinding destination set to: %s"), *TargetLocation.ToString());
}

void UCrowd_PathfindingComponent::AddWaypoint(const FVector& WaypointLocation, float WaypointRadius)
{
    FCrowd_Waypoint NewWaypoint;
    NewWaypoint.Position = WaypointLocation;
    NewWaypoint.Radius = WaypointRadius;
    NewWaypoint.bIsOccupied = false;
    NewWaypoint.Priority = Waypoints.Num(); // Simple priority based on order
    
    Waypoints.Add(NewWaypoint);
    
    UE_LOG(LogTemp, Log, TEXT("Added waypoint at: %s"), *WaypointLocation.ToString());
}

FVector UCrowd_PathfindingComponent::GetNextWaypoint()
{
    if (Waypoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Cycle through waypoints
    CurrentWaypointIndex = (CurrentWaypointIndex + 1) % Waypoints.Num();
    return Waypoints[CurrentWaypointIndex].Position;
}

bool UCrowd_PathfindingComponent::IsAtDestination() const
{
    if (CurrentDestination.IsZero())
    {
        return true;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentDestination);
    return Distance <= AcceptanceRadius;
}

void UCrowd_PathfindingComponent::SetMovementSpeed(float NewSpeed)
{
    MovementSpeed = FMath::Clamp(NewSpeed, 0.0f, 1000.0f);
}

void UCrowd_PathfindingComponent::PauseMovement(bool bPause)
{
    bIsMoving = !bPause;
}

void UCrowd_PathfindingComponent::MoveTowardsDestination(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector Direction = (CurrentDestination - CurrentLocation).GetSafeNormal();
    
    // Calculate new position
    FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
    
    // Check if we've reached the destination
    if (IsAtDestination())
    {
        // Move to next waypoint if available
        FVector NextWaypoint = GetNextWaypoint();
        if (!NextWaypoint.IsZero())
        {
            CurrentDestination = NextWaypoint;
        }
        else
        {
            bIsMoving = false;
            UE_LOG(LogTemp, Log, TEXT("Reached final destination"));
        }
    }
    else
    {
        // Move towards destination
        Owner->SetActorLocation(NewLocation);
        
        // Rotate to face movement direction
        if (!Direction.IsZero())
        {
            FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, CurrentDestination);
            NewRotation.Pitch = 0.0f; // Keep level
            NewRotation.Roll = 0.0f;
            Owner->SetActorRotation(NewRotation);
        }
    }
}

FVector UCrowd_PathfindingComponent::FindNearestWaypoint(const FVector& FromLocation)
{
    if (Waypoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    float MinDistance = FLT_MAX;
    FVector NearestWaypoint = FVector::ZeroVector;
    
    for (const FCrowd_Waypoint& Waypoint : Waypoints)
    {
        if (Waypoint.bIsOccupied)
        {
            continue; // Skip occupied waypoints
        }
        
        float Distance = FVector::Dist(FromLocation, Waypoint.Position);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestWaypoint = Waypoint.Position;
        }
    }
    
    return NearestWaypoint;
}

void UCrowd_PathfindingComponent::UpdateWaypointOccupancy()
{
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Update occupancy status for all waypoints
    for (FCrowd_Waypoint& Waypoint : Waypoints)
    {
        float Distance = FVector::Dist(CurrentLocation, Waypoint.Position);
        Waypoint.bIsOccupied = (Distance <= Waypoint.Radius);
    }
}
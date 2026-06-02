#include "Crowd_PathfindingSystem.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UCrowd_PathfindingSystem::UCrowd_PathfindingSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for pathfinding updates
    
    MaxRequestsPerFrame = 5;
    PathOptimizationDistance = 200.0f;
    bUseAsyncPathfinding = true;
    FlowFieldCellSize = 100.0f;
    FlowFieldTarget = FVector::ZeroVector;
    NextRequestID = 1;
    NavSystem = nullptr;
}

void UCrowd_PathfindingSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get navigation system reference
    UWorld* World = GetWorld();
    if (World)
    {
        NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
        if (!NavSystem)
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingSystem: Navigation system not found"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Crowd_PathfindingSystem: Navigation system initialized"));
        }
    }
}

void UCrowd_PathfindingSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process pending pathfinding requests
    ProcessPendingRequests();
}

int32 UCrowd_PathfindingSystem::RequestPath(const FVector& StartLocation, const FVector& TargetLocation, float AgentRadius)
{
    FCrowd_PathfindingRequest NewRequest;
    NewRequest.StartLocation = StartLocation;
    NewRequest.TargetLocation = TargetLocation;
    NewRequest.AgentRadius = AgentRadius;
    NewRequest.RequestID = NextRequestID++;
    NewRequest.bIsValid = true;
    
    PendingRequests.Add(NewRequest);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_PathfindingSystem: Path request %d queued"), NewRequest.RequestID);
    return NewRequest.RequestID;
}

bool UCrowd_PathfindingSystem::GetPathResult(int32 RequestID, FCrowd_PathResult& OutResult)
{
    if (CompletedPaths.Contains(RequestID))
    {
        OutResult = CompletedPaths[RequestID];
        CompletedPaths.Remove(RequestID); // Remove after retrieval
        return true;
    }
    return false;
}

void UCrowd_PathfindingSystem::CancelPathRequest(int32 RequestID)
{
    // Remove from pending requests
    PendingRequests.RemoveAll([RequestID](const FCrowd_PathfindingRequest& Request)
    {
        return Request.RequestID == RequestID;
    });
    
    // Remove from completed paths
    CompletedPaths.Remove(RequestID);
}

void UCrowd_PathfindingSystem::ProcessBatchPathfinding(const TArray<FCrowd_PathfindingRequest>& Requests)
{
    for (const FCrowd_PathfindingRequest& Request : Requests)
    {
        if (Request.bIsValid)
        {
            PendingRequests.Add(Request);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_PathfindingSystem: %d batch requests added"), Requests.Num());
}

bool UCrowd_PathfindingSystem::IsLocationNavigable(const FVector& Location, float AgentRadius)
{
    if (!NavSystem)
    {
        return false;
    }
    
    FNavLocation NavLocation;
    bool bFound = NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(AgentRadius, AgentRadius, 500.0f));
    
    return bFound && (FVector::Dist(Location, NavLocation.Location) < AgentRadius * 2.0f);
}

FVector UCrowd_PathfindingSystem::GetRandomNavigablePoint(const FVector& Origin, float Radius)
{
    if (!NavSystem)
    {
        return Origin;
    }
    
    FNavLocation RandomLocation;
    bool bFound = NavSystem->GetRandomPointInNavigableRadius(Origin, Radius, RandomLocation);
    
    return bFound ? RandomLocation.Location : Origin;
}

void UCrowd_PathfindingSystem::GenerateFlowField(const FVector& TargetLocation, float CellSize)
{
    FlowFieldTarget = TargetLocation;
    FlowFieldCellSize = CellSize;
    FlowFieldGrid.Empty();
    
    if (!NavSystem)
    {
        return;
    }
    
    // Generate flow field in a grid around the target
    const float GridRadius = 2000.0f; // 20m radius
    const int32 GridSize = FMath::CeilToInt(GridRadius * 2.0f / CellSize);
    
    for (int32 X = -GridSize/2; X <= GridSize/2; X++)
    {
        for (int32 Y = -GridSize/2; Y <= GridSize/2; Y++)
        {
            FVector CellLocation = TargetLocation + FVector(X * CellSize, Y * CellSize, 0.0f);
            FVector2D CellKey(X, Y);
            
            // Calculate direction to target
            FVector Direction = (TargetLocation - CellLocation).GetSafeNormal();
            
            // Check if location is navigable
            if (IsLocationNavigable(CellLocation))
            {
                FlowFieldGrid.Add(CellKey, Direction);
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_PathfindingSystem: Flow field generated with %d cells"), FlowFieldGrid.Num());
}

FVector UCrowd_PathfindingSystem::GetFlowFieldDirection(const FVector& Location)
{
    if (FlowFieldGrid.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Convert world location to grid coordinates
    FVector RelativeLocation = Location - FlowFieldTarget;
    int32 GridX = FMath::RoundToInt(RelativeLocation.X / FlowFieldCellSize);
    int32 GridY = FMath::RoundToInt(RelativeLocation.Y / FlowFieldCellSize);
    
    FVector2D CellKey(GridX, GridY);
    
    if (FlowFieldGrid.Contains(CellKey))
    {
        return FlowFieldGrid[CellKey];
    }
    
    // Fallback: direct direction to target
    return (FlowFieldTarget - Location).GetSafeNormal();
}

void UCrowd_PathfindingSystem::ProcessPendingRequests()
{
    if (!NavSystem || PendingRequests.Num() == 0)
    {
        return;
    }
    
    int32 ProcessedThisFrame = 0;
    
    for (int32 i = PendingRequests.Num() - 1; i >= 0 && ProcessedThisFrame < MaxRequestsPerFrame; i--)
    {
        FCrowd_PathfindingRequest& Request = PendingRequests[i];
        FCrowd_PathResult Result;
        
        if (FindPathInternal(Request, Result))
        {
            CompletedPaths.Add(Request.RequestID, Result);
            UE_LOG(LogTemp, Log, TEXT("Crowd_PathfindingSystem: Path %d completed with %d points"), 
                Request.RequestID, Result.PathPoints.Num());
        }
        else
        {
            // Failed pathfinding - still store result for error handling
            Result.RequestID = Request.RequestID;
            Result.bPathFound = false;
            CompletedPaths.Add(Request.RequestID, Result);
            UE_LOG(LogTemp, Warning, TEXT("Crowd_PathfindingSystem: Path %d failed"), Request.RequestID);
        }
        
        PendingRequests.RemoveAt(i);
        ProcessedThisFrame++;
    }
}

bool UCrowd_PathfindingSystem::FindPathInternal(const FCrowd_PathfindingRequest& Request, FCrowd_PathResult& OutResult)
{
    if (!NavSystem)
    {
        return false;
    }
    
    OutResult.RequestID = Request.RequestID;
    OutResult.bPathFound = false;
    OutResult.PathLength = 0.0f;
    OutResult.PathPoints.Empty();
    
    // Create navigation query filter
    FSharedConstNavQueryFilter QueryFilter = UNavigationQueryFilter::GetQueryFilter(*NavSystem, nullptr, nullptr);
    
    // Find path
    FPathFindingQuery Query(GetOwner(), *NavSystem->GetDefaultNavDataInstance(), Request.StartLocation, Request.TargetLocation, QueryFilter);
    FPathFindingResult PathResult = NavSystem->FindPathSync(Query);
    
    if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
    {
        const TArray<FNavPathPoint>& PathPoints = PathResult.Path->GetPathPoints();
        
        for (const FNavPathPoint& Point : PathPoints)
        {
            OutResult.PathPoints.Add(Point.Location);
        }
        
        // Calculate path length
        for (int32 i = 1; i < OutResult.PathPoints.Num(); i++)
        {
            OutResult.PathLength += FVector::Dist(OutResult.PathPoints[i-1], OutResult.PathPoints[i]);
        }
        
        // Optimize path
        OptimizePath(OutResult.PathPoints);
        
        OutResult.bPathFound = true;
        return true;
    }
    
    return false;
}

void UCrowd_PathfindingSystem::OptimizePath(TArray<FVector>& PathPoints)
{
    if (PathPoints.Num() < 3)
    {
        return;
    }
    
    // Simple path optimization - remove unnecessary waypoints
    TArray<FVector> OptimizedPath;
    OptimizedPath.Add(PathPoints[0]); // Always keep start point
    
    for (int32 i = 1; i < PathPoints.Num() - 1; i++)
    {
        FVector PrevPoint = OptimizedPath.Last();
        FVector CurrentPoint = PathPoints[i];
        FVector NextPoint = PathPoints[i + 1];
        
        // Check if we can skip current point by going directly to next
        float DirectDistance = FVector::Dist(PrevPoint, NextPoint);
        float PathDistance = FVector::Dist(PrevPoint, CurrentPoint) + FVector::Dist(CurrentPoint, NextPoint);
        
        // If direct path is not much longer, skip the intermediate point
        if (DirectDistance > PathDistance + PathOptimizationDistance)
        {
            OptimizedPath.Add(CurrentPoint);
        }
    }
    
    OptimizedPath.Add(PathPoints.Last()); // Always keep end point
    PathPoints = OptimizedPath;
}
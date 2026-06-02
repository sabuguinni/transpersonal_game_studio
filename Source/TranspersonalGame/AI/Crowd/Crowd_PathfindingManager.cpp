#include "Crowd_PathfindingManager.h"
#include "Engine/World.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationPath.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_PathfindingManager::UCrowd_PathfindingManager()
{
    MaxPathfindingDistance = 10000.0f;
    NodeSearchRadius = 500.0f;
    MaxPathfindingIterations = 1000;
    bUseHierarchicalPathfinding = true;
    NavigationSystem = nullptr;
}

void UCrowd_PathfindingManager::InitializePathfindingSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (!NavigationSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavigationSystem not found - pathfinding may be limited"));
        return;
    }

    // Build initial pathfinding graph
    BuildPathfindingGraph();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Pathfinding System initialized with %d nodes"), PathfindingNodes.Num());
}

FCrowd_NavigationPath UCrowd_PathfindingManager::FindPathBetweenPoints(const FVector& StartLocation, const FVector& EndLocation)
{
    FCrowd_NavigationPath ResultPath;
    
    if (!NavigationSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("NavigationSystem not available for pathfinding"));
        return ResultPath;
    }

    // Use UE5 navigation system for basic pathfinding
    FNavLocation StartNavLocation, EndNavLocation;
    
    bool bStartValid = NavigationSystem->ProjectPointToNavigation(StartLocation, StartNavLocation, FVector(100, 100, 100));
    bool bEndValid = NavigationSystem->ProjectPointToNavigation(EndLocation, EndNavLocation, FVector(100, 100, 100));
    
    if (!bStartValid || !bEndValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Start or End location not on NavMesh"));
        return ResultPath;
    }

    // Find navigation path
    FPathFindingQuery Query;
    Query.StartLocation = StartNavLocation.Location;
    Query.EndLocation = EndNavLocation.Location;
    Query.NavData = NavigationSystem->GetDefaultNavDataInstance();
    
    FPathFindingResult PathResult = NavigationSystem->FindPathSync(Query);
    
    if (PathResult.IsSuccessful() && PathResult.Path.IsValid())
    {
        TArray<FNavPathPoint> PathPoints = PathResult.Path->GetPathPoints();
        
        for (const FNavPathPoint& Point : PathPoints)
        {
            ResultPath.Waypoints.Add(Point.Location);
        }
        
        ResultPath.PathLength = CalculatePathCost(ResultPath.Waypoints);
        ResultPath.bIsValid = true;
        ResultPath.EstimatedTravelTime = ResultPath.PathLength / 300.0f; // Assume 300 units/second movement
        
        // Smooth the path for more natural crowd movement
        ResultPath.Waypoints = SmoothPath(ResultPath.Waypoints);
    }
    
    return ResultPath;
}

bool UCrowd_PathfindingManager::IsLocationNavigable(const FVector& Location)
{
    if (!NavigationSystem)
    {
        return false;
    }
    
    FNavLocation NavLocation;
    return NavigationSystem->ProjectPointToNavigation(Location, NavLocation, FVector(50, 50, 50));
}

TArray<FVector> UCrowd_PathfindingManager::GetNearbyNavigableLocations(const FVector& CenterLocation, float SearchRadius)
{
    TArray<FVector> NavigableLocations;
    
    if (!NavigationSystem)
    {
        return NavigableLocations;
    }
    
    // Sample points in a circle around the center
    int32 NumSamples = 16;
    float AngleStep = 360.0f / NumSamples;
    
    for (int32 i = 0; i < NumSamples; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector SampleLocation = CenterLocation + FVector(
            FMath::Cos(RadianAngle) * SearchRadius,
            FMath::Sin(RadianAngle) * SearchRadius,
            0.0f
        );
        
        if (IsLocationNavigable(SampleLocation))
        {
            NavigableLocations.Add(SampleLocation);
        }
    }
    
    return NavigableLocations;
}

void UCrowd_PathfindingManager::RegisterPathfindingNode(const FCrowd_PathfindingNode& Node)
{
    PathfindingNodes.Add(Node);
    
    // Connect to nearby nodes
    int32 NewNodeIndex = PathfindingNodes.Num() - 1;
    FCrowd_PathfindingNode& NewNode = PathfindingNodes[NewNodeIndex];
    
    for (int32 i = 0; i < PathfindingNodes.Num() - 1; i++)
    {
        FCrowd_PathfindingNode& ExistingNode = PathfindingNodes[i];
        float Distance = FVector::Dist(NewNode.Location, ExistingNode.Location);
        
        if (Distance <= NodeSearchRadius)
        {
            NewNode.ConnectedNodes.Add(i);
            ExistingNode.ConnectedNodes.Add(NewNodeIndex);
        }
    }
}

void UCrowd_PathfindingManager::UpdateNodeOccupancy(int32 NodeIndex, bool bOccupied)
{
    if (PathfindingNodes.IsValidIndex(NodeIndex))
    {
        PathfindingNodes[NodeIndex].bIsOccupied = bOccupied;
    }
}

FVector UCrowd_PathfindingManager::GetOptimalGatheringPoint(const TArray<FVector>& CrowdPositions)
{
    if (CrowdPositions.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    // Calculate centroid of crowd positions
    FVector Centroid = FVector::ZeroVector;
    for (const FVector& Position : CrowdPositions)
    {
        Centroid += Position;
    }
    Centroid /= CrowdPositions.Num();
    
    // Find nearest navigable location to centroid
    FNavLocation NavLocation;
    if (NavigationSystem && NavigationSystem->ProjectPointToNavigation(Centroid, NavLocation, FVector(200, 200, 200)))
    {
        return NavLocation.Location;
    }
    
    return Centroid;
}

void UCrowd_PathfindingManager::OptimizePathForCrowdDensity(FCrowd_NavigationPath& Path, float CrowdDensity)
{
    if (!Path.bIsValid || Path.Waypoints.Num() < 2)
    {
        return;
    }
    
    // Adjust waypoint spacing based on crowd density
    float DensityMultiplier = FMath::Clamp(CrowdDensity, 0.1f, 2.0f);
    float OptimalSpacing = 150.0f * DensityMultiplier;
    
    TArray<FVector> OptimizedWaypoints;
    OptimizedWaypoints.Add(Path.Waypoints[0]); // Keep start point
    
    for (int32 i = 1; i < Path.Waypoints.Num() - 1; i++)
    {
        FVector CurrentPoint = Path.Waypoints[i];
        FVector LastAddedPoint = OptimizedWaypoints.Last();
        
        float DistanceToLast = FVector::Dist(CurrentPoint, LastAddedPoint);
        
        if (DistanceToLast >= OptimalSpacing)
        {
            OptimizedWaypoints.Add(CurrentPoint);
        }
    }
    
    OptimizedWaypoints.Add(Path.Waypoints.Last()); // Keep end point
    Path.Waypoints = OptimizedWaypoints;
}

void UCrowd_PathfindingManager::BuildPathfindingGraph()
{
    // Create initial pathfinding nodes at strategic locations
    TArray<FVector> StrategicLocations = {
        FVector(0, 0, 100),      // Center
        FVector(2000, 0, 100),   // East
        FVector(-2000, 0, 100),  // West
        FVector(0, 2000, 100),   // North
        FVector(0, -2000, 100),  // South
        FVector(1500, 1500, 100), // Northeast
        FVector(-1500, 1500, 100), // Northwest
        FVector(1500, -1500, 100), // Southeast
        FVector(-1500, -1500, 100) // Southwest
    };
    
    for (const FVector& Location : StrategicLocations)
    {
        if (IsLocationNavigable(Location))
        {
            FCrowd_PathfindingNode Node;
            Node.Location = Location;
            Node.Priority = 1.0f;
            Node.MaxOccupants = 10;
            RegisterPathfindingNode(Node);
        }
    }
}

float UCrowd_PathfindingManager::CalculatePathCost(const TArray<FVector>& Waypoints)
{
    float TotalDistance = 0.0f;
    
    for (int32 i = 1; i < Waypoints.Num(); i++)
    {
        TotalDistance += FVector::Dist(Waypoints[i-1], Waypoints[i]);
    }
    
    return TotalDistance;
}

bool UCrowd_PathfindingManager::ValidatePathSafety(const TArray<FVector>& Waypoints)
{
    // Check if path avoids dangerous areas (can be expanded with threat detection)
    for (const FVector& Waypoint : Waypoints)
    {
        if (!IsLocationNavigable(Waypoint))
        {
            return false;
        }
    }
    
    return true;
}

TArray<FVector> UCrowd_PathfindingManager::SmoothPath(const TArray<FVector>& RawPath)
{
    if (RawPath.Num() < 3)
    {
        return RawPath;
    }
    
    TArray<FVector> SmoothedPath;
    SmoothedPath.Add(RawPath[0]); // Keep start point
    
    // Apply simple smoothing algorithm
    for (int32 i = 1; i < RawPath.Num() - 1; i++)
    {
        FVector PrevPoint = RawPath[i-1];
        FVector CurrentPoint = RawPath[i];
        FVector NextPoint = RawPath[i+1];
        
        // Average with neighboring points for smoother curves
        FVector SmoothedPoint = (PrevPoint + CurrentPoint * 2.0f + NextPoint) / 4.0f;
        SmoothedPath.Add(SmoothedPoint);
    }
    
    SmoothedPath.Add(RawPath.Last()); // Keep end point
    return SmoothedPath;
}
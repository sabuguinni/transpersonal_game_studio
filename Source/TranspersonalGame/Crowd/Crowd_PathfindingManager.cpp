#include "Crowd_PathfindingManager.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/KismetMathLibrary.h"

ACrowd_PathfindingManager::ACrowd_PathfindingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;
    
    // Initialize default values
    WaypointRadius = 200.0f;
    MaxPathDistance = 3000.0f;
    MaxPathNodes = 50;
    bEnableDynamicObstacles = true;
    bEnableFlowFields = true;
    PathUpdateInterval = 1.0f;
    MaxConcurrentPaths = 100;
    LastPathUpdate = 0.0f;
    bPathVisualizationEnabled = false;
}

void ACrowd_PathfindingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize pathfinding system
    InitializePathNetwork();
    GeneratePathNodes();
    ConnectPathNodes();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Pathfinding Manager initialized with %d waypoints"), Waypoints.Num());
}

void ACrowd_PathfindingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastPathUpdate += DeltaTime;
    
    // Update pathfinding system periodically
    if (LastPathUpdate >= PathUpdateInterval)
    {
        UpdateDynamicObstacles();
        UpdatePathNetwork();
        LastPathUpdate = 0.0f;
    }
    
    // Draw debug visualization if enabled
    if (bPathVisualizationEnabled)
    {
        DrawPathNetwork();
        DrawWaypoints();
    }
}

TArray<FVector> ACrowd_PathfindingManager::FindPath(const FVector& StartLocation, const FVector& EndLocation)
{
    TArray<FVector> Path;
    
    // Simple A* pathfinding implementation
    FVector CurrentLocation = StartLocation;
    FVector TargetLocation = EndLocation;
    
    // Check if direct path is possible
    if (!IsPathBlocked(StartLocation, EndLocation))
    {
        Path.Add(StartLocation);
        Path.Add(EndLocation);
        return Path;
    }
    
    // Use waypoint-based pathfinding
    FVector NearestStartWaypoint = GetNearestWaypoint(StartLocation);
    FVector NearestEndWaypoint = GetNearestWaypoint(EndLocation);
    
    Path.Add(StartLocation);
    
    if (NearestStartWaypoint != FVector::ZeroVector)
    {
        Path.Add(NearestStartWaypoint);
    }
    
    // Add intermediate waypoints if needed
    float DistanceToTarget = FVector::Dist(NearestStartWaypoint, NearestEndWaypoint);
    if (DistanceToTarget > WaypointRadius * 2.0f)
    {
        // Find intermediate waypoints
        for (const FCrowd_Waypoint& Waypoint : Waypoints)
        {
            if (Waypoint.bIsActive)
            {
                float DistToStart = FVector::Dist(Waypoint.Location, NearestStartWaypoint);
                float DistToEnd = FVector::Dist(Waypoint.Location, NearestEndWaypoint);
                
                if (DistToStart < DistanceToTarget && DistToEnd < DistanceToTarget)
                {
                    Path.Add(Waypoint.Location);
                }
            }
        }
    }
    
    if (NearestEndWaypoint != FVector::ZeroVector)
    {
        Path.Add(NearestEndWaypoint);
    }
    
    Path.Add(EndLocation);
    
    return Path;
}

FVector ACrowd_PathfindingManager::GetNearestWaypoint(const FVector& Location)
{
    FVector NearestWaypoint = FVector::ZeroVector;
    float MinDistance = MAX_FLT;
    
    for (const FCrowd_Waypoint& Waypoint : Waypoints)
    {
        if (Waypoint.bIsActive)
        {
            float Distance = FVector::Dist(Location, Waypoint.Location);
            if (Distance < MinDistance)
            {
                MinDistance = Distance;
                NearestWaypoint = Waypoint.Location;
            }
        }
    }
    
    return NearestWaypoint;
}

bool ACrowd_PathfindingManager::IsPathBlocked(const FVector& StartLocation, const FVector& EndLocation)
{
    // Check for dynamic obstacles
    for (int32 i = 0; i < DynamicObstacles.Num(); i++)
    {
        float DistanceToObstacle = FMath::PointDistToSegment(DynamicObstacles[i], StartLocation, EndLocation);
        if (DistanceToObstacle < ObstacleRadii[i])
        {
            return true;
        }
    }
    
    // Use navigation system for more accurate pathfinding
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavStart, NavEnd;
        bool bValidStart = NavSys->ProjectPointToNavigation(StartLocation, NavStart);
        bool bValidEnd = NavSys->ProjectPointToNavigation(EndLocation, NavEnd);
        
        if (!bValidStart || !bValidEnd)
        {
            return true;
        }
    }
    
    return false;
}

void ACrowd_PathfindingManager::AddDynamicObstacle(const FVector& Location, float Radius)
{
    DynamicObstacles.Add(Location);
    ObstacleRadii.Add(Radius);
    
    // Limit the number of dynamic obstacles for performance
    if (DynamicObstacles.Num() > 100)
    {
        DynamicObstacles.RemoveAt(0);
        ObstacleRadii.RemoveAt(0);
    }
}

void ACrowd_PathfindingManager::RemoveDynamicObstacle(const FVector& Location)
{
    for (int32 i = DynamicObstacles.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(DynamicObstacles[i], Location) < 50.0f)
        {
            DynamicObstacles.RemoveAt(i);
            ObstacleRadii.RemoveAt(i);
            break;
        }
    }
}

void ACrowd_PathfindingManager::UpdatePathNetwork()
{
    // Update waypoint connectivity based on current obstacles
    for (FCrowd_Waypoint& Waypoint : Waypoints)
    {
        if (IsLocationBlocked(Waypoint.Location))
        {
            Waypoint.bIsActive = false;
        }
        else
        {
            Waypoint.bIsActive = true;
        }
    }
}

void ACrowd_PathfindingManager::OptimizePathNodes()
{
    // Remove redundant path nodes
    for (int32 i = PathNodes.Num() - 1; i >= 0; i--)
    {
        if (PathNodes[i].bIsBlocked || PathNodes[i].ConnectedNodes.Num() == 0)
        {
            PathNodes.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Path network optimized: %d nodes remaining"), PathNodes.Num());
}

FVector ACrowd_PathfindingManager::GetFlowFieldDirection(const FVector& Location, const FVector& Target)
{
    if (!bEnableFlowFields)
    {
        return (Target - Location).GetSafeNormal();
    }
    
    // Check flow field cache
    if (FlowField.Contains(Location))
    {
        return FlowField[Location];
    }
    
    // Calculate flow field direction
    FVector Direction = (Target - Location).GetSafeNormal();
    
    // Adjust for obstacles
    for (int32 i = 0; i < DynamicObstacles.Num(); i++)
    {
        FVector ToObstacle = DynamicObstacles[i] - Location;
        float Distance = ToObstacle.Size();
        
        if (Distance < ObstacleRadii[i] * 2.0f)
        {
            FVector AvoidanceVector = ToObstacle.GetSafeNormal() * -1.0f;
            Direction += AvoidanceVector * (1.0f - Distance / (ObstacleRadii[i] * 2.0f));
        }
    }
    
    Direction.Normalize();
    FlowField.Add(Location, Direction);
    
    return Direction;
}

void ACrowd_PathfindingManager::GenerateFlowField(const FVector& TargetLocation)
{
    FlowField.Empty();
    
    // Generate flow field for the entire area
    float GridSize = 100.0f;
    int32 GridWidth = 100;
    int32 GridHeight = 100;
    
    FVector StartLocation = GetActorLocation() - FVector(GridWidth * GridSize * 0.5f, GridHeight * GridSize * 0.5f, 0.0f);
    
    for (int32 x = 0; x < GridWidth; x++)
    {
        for (int32 y = 0; y < GridHeight; y++)
        {
            FVector GridLocation = StartLocation + FVector(x * GridSize, y * GridSize, 0.0f);
            FVector Direction = GetFlowFieldDirection(GridLocation, TargetLocation);
            FlowField.Add(GridLocation, Direction);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Flow field generated with %d grid points"), FlowField.Num());
}

void ACrowd_PathfindingManager::ClearFlowField()
{
    FlowField.Empty();
}

void ACrowd_PathfindingManager::DrawPathNetwork()
{
    if (!GetWorld()) return;
    
    // Draw waypoints
    for (const FCrowd_Waypoint& Waypoint : Waypoints)
    {
        FColor WaypointColor = Waypoint.bIsActive ? FColor::Green : FColor::Red;
        DrawDebugSphere(GetWorld(), Waypoint.Location, Waypoint.Radius, 12, WaypointColor, false, 0.1f);
    }
    
    // Draw path nodes
    for (const FCrowd_PathNode& Node : PathNodes)
    {
        FColor NodeColor = Node.bIsBlocked ? FColor::Red : FColor::Blue;
        DrawDebugSphere(GetWorld(), Node.Position, 25.0f, 8, NodeColor, false, 0.1f);
        
        // Draw connections
        for (int32 ConnectedIndex : Node.ConnectedNodes)
        {
            if (PathNodes.IsValidIndex(ConnectedIndex))
            {
                DrawDebugLine(GetWorld(), Node.Position, PathNodes[ConnectedIndex].Position, FColor::Yellow, false, 0.1f);
            }
        }
    }
    
    // Draw dynamic obstacles
    for (int32 i = 0; i < DynamicObstacles.Num(); i++)
    {
        DrawDebugSphere(GetWorld(), DynamicObstacles[i], ObstacleRadii[i], 12, FColor::Orange, false, 0.1f);
    }
}

void ACrowd_PathfindingManager::DrawWaypoints()
{
    if (!GetWorld()) return;
    
    for (int32 i = 0; i < Waypoints.Num(); i++)
    {
        const FCrowd_Waypoint& Waypoint = Waypoints[i];
        FColor Color = Waypoint.bIsActive ? FColor::Cyan : FColor::Magenta;
        
        DrawDebugSphere(GetWorld(), Waypoint.Location, Waypoint.Radius, 16, Color, false, 0.1f);
        DrawDebugString(GetWorld(), Waypoint.Location + FVector(0, 0, 100), FString::Printf(TEXT("WP_%d"), i), nullptr, Color, 0.1f);
    }
}

void ACrowd_PathfindingManager::TogglePathVisualization()
{
    bPathVisualizationEnabled = !bPathVisualizationEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Path visualization %s"), bPathVisualizationEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_PathfindingManager::InitializePathNetwork()
{
    // Create default waypoints in a grid pattern
    Waypoints.Empty();
    
    int32 GridSize = 8;
    float Spacing = 500.0f;
    FVector CenterLocation = GetActorLocation();
    
    for (int32 x = 0; x < GridSize; x++)
    {
        for (int32 y = 0; y < GridSize; y++)
        {
            FCrowd_Waypoint NewWaypoint;
            NewWaypoint.Location = CenterLocation + FVector(
                (x - GridSize * 0.5f) * Spacing,
                (y - GridSize * 0.5f) * Spacing,
                0.0f
            );
            NewWaypoint.Radius = WaypointRadius;
            NewWaypoint.Priority = FMath::RandRange(1, 5);
            NewWaypoint.bIsActive = true;
            
            Waypoints.Add(NewWaypoint);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized %d waypoints"), Waypoints.Num());
}

void ACrowd_PathfindingManager::UpdateDynamicObstacles()
{
    // Update obstacle positions and remove expired ones
    for (int32 i = DynamicObstacles.Num() - 1; i >= 0; i--)
    {
        // Remove obstacles that are too far from any waypoint
        bool bNearWaypoint = false;
        for (const FCrowd_Waypoint& Waypoint : Waypoints)
        {
            if (FVector::Dist(DynamicObstacles[i], Waypoint.Location) < MaxPathDistance)
            {
                bNearWaypoint = true;
                break;
            }
        }
        
        if (!bNearWaypoint)
        {
            DynamicObstacles.RemoveAt(i);
            ObstacleRadii.RemoveAt(i);
        }
    }
}

float ACrowd_PathfindingManager::CalculatePathCost(const FVector& Start, const FVector& End)
{
    float Distance = FVector::Dist(Start, End);
    float Cost = Distance;
    
    // Add penalty for obstacles
    for (int32 i = 0; i < DynamicObstacles.Num(); i++)
    {
        float DistanceToObstacle = FMath::PointDistToSegment(DynamicObstacles[i], Start, End);
        if (DistanceToObstacle < ObstacleRadii[i] * 2.0f)
        {
            Cost += 1000.0f; // High penalty for paths near obstacles
        }
    }
    
    return Cost;
}

bool ACrowd_PathfindingManager::IsLocationBlocked(const FVector& Location)
{
    for (int32 i = 0; i < DynamicObstacles.Num(); i++)
    {
        if (FVector::Dist(Location, DynamicObstacles[i]) < ObstacleRadii[i])
        {
            return true;
        }
    }
    
    return false;
}

void ACrowd_PathfindingManager::GeneratePathNodes()
{
    PathNodes.Empty();
    
    // Generate path nodes based on waypoints
    for (const FCrowd_Waypoint& Waypoint : Waypoints)
    {
        FCrowd_PathNode NewNode;
        NewNode.Position = Waypoint.Location;
        NewNode.MovementCost = 1.0f / FMath::Max(1.0f, float(Waypoint.Priority));
        NewNode.bIsBlocked = !Waypoint.bIsActive;
        
        PathNodes.Add(NewNode);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generated %d path nodes"), PathNodes.Num());
}

void ACrowd_PathfindingManager::ConnectPathNodes()
{
    // Connect nearby path nodes
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        for (int32 j = i + 1; j < PathNodes.Num(); j++)
        {
            float Distance = FVector::Dist(PathNodes[i].Position, PathNodes[j].Position);
            
            if (Distance <= WaypointRadius * 2.0f && !IsPathBlocked(PathNodes[i].Position, PathNodes[j].Position))
            {
                PathNodes[i].ConnectedNodes.Add(j);
                PathNodes[j].ConnectedNodes.Add(i);
            }
        }
    }
    
    // Log connectivity statistics
    int32 TotalConnections = 0;
    for (const FCrowd_PathNode& Node : PathNodes)
    {
        TotalConnections += Node.ConnectedNodes.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Path nodes connected: %d total connections"), TotalConnections);
}
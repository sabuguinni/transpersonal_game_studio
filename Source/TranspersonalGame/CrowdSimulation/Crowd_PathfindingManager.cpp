#include "Crowd_PathfindingManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/GameplayStatics.h"

UCrowd_PathfindingManager::UCrowd_PathfindingManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS

    // Performance settings
    MaxPathRequestsPerFrame = 100;
    bUseLODPathfinding = true;
    PathUpdateFrequency = 0.1f;
    ObstacleUpdateFrequency = 0.2f;
    
    MaxActivePathsLOD0 = 500;   // High detail
    MaxActivePathsLOD1 = 2000;  // Medium detail
    MaxActivePathsLOD2 = 10000; // Low detail

    // Flow field settings
    FlowFieldCellSize = 200.0f;
    FlowFieldTarget = FVector::ZeroVector;

    // Debug settings
    bDrawDebugPaths = false;
    bDrawFlowField = false;
    DebugDrawDuration = 0.1f;

    // Internal tracking
    LastPathUpdateTime = 0.0f;
    LastObstacleUpdateTime = 0.0f;
    PathRequestsProcessedThisFrame = 0;
}

void UCrowd_PathfindingManager::BeginPlay()
{
    Super::BeginPlay();

    // Get navigation system
    NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("UCrowd_PathfindingManager: No Navigation System found"));
        return;
    }

    // Find NavMesh bounds volume
    TArray<AActor*> NavMeshActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANavMeshBoundsVolume::StaticClass(), NavMeshActors);
    if (NavMeshActors.Num() > 0)
    {
        NavMeshBounds = Cast<ANavMeshBoundsVolume>(NavMeshActors[0]);
        UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingManager: Found NavMesh bounds volume"));
    }

    // Build hierarchical pathfinding graph
    BuildHierarchicalGraph();

    UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingManager: Initialized with max %d requests per frame"), MaxPathRequestsPerFrame);
}

void UCrowd_PathfindingManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!NavSystem)
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Reset frame counter
    PathRequestsProcessedThisFrame = 0;

    // Process pending path requests
    ProcessPathRequests(DeltaTime);

    // Update flow field periodically
    if (CurrentTime - LastPathUpdateTime >= PathUpdateFrequency)
    {
        UpdateFlowField();
        LastPathUpdateTime = CurrentTime;
    }

    // Update dynamic obstacles
    if (CurrentTime - LastObstacleUpdateTime >= ObstacleUpdateFrequency)
    {
        UpdateDynamicObstacles();
        LastObstacleUpdateTime = CurrentTime;
    }

    // Cleanup completed paths
    CleanupCompletedPaths();

    // Debug visualization
    if (bDrawDebugPaths)
    {
        DebugDrawPaths();
    }
    if (bDrawFlowField)
    {
        DebugDrawFlowField();
    }
}

bool UCrowd_PathfindingManager::RequestPath(const FCrowd_PathRequest& Request)
{
    if (!NavSystem)
        return false;

    // Check if agent already has a pending request
    for (const FCrowd_PathRequest& PendingRequest : PendingRequests)
    {
        if (PendingRequest.AgentID == Request.AgentID)
        {
            // Update existing request
            const_cast<FCrowd_PathRequest&>(PendingRequest) = Request;
            return true;
        }
    }

    // Add new request
    PendingRequests.Add(Request);
    UE_LOG(LogTemp, VeryVerbose, TEXT("UCrowd_PathfindingManager: Path requested for agent %d"), Request.AgentID);
    return true;
}

TArray<FVector> UCrowd_PathfindingManager::GetPath(int32 AgentID)
{
    if (ActivePaths.Contains(AgentID))
    {
        return ActivePaths[AgentID];
    }
    return TArray<FVector>();
}

void UCrowd_PathfindingManager::CancelPathRequest(int32 AgentID)
{
    // Remove from pending requests
    PendingRequests.RemoveAll([AgentID](const FCrowd_PathRequest& Request)
    {
        return Request.AgentID == AgentID;
    });

    // Remove from active paths
    ActivePaths.Remove(AgentID);
}

bool UCrowd_PathfindingManager::IsPathReady(int32 AgentID)
{
    return ActivePaths.Contains(AgentID);
}

void UCrowd_PathfindingManager::BuildHierarchicalGraph()
{
    if (!NavMeshBounds)
        return;

    HierarchicalNodes.Empty();

    // Create a grid of hierarchical nodes across the NavMesh bounds
    FVector BoundsOrigin, BoundsExtent;
    NavMeshBounds->GetActorBounds(false, BoundsOrigin, BoundsExtent);

    const float NodeSpacing = 1000.0f; // 10m spacing between hierarchical nodes
    const int32 NodesX = FMath::CeilToInt(BoundsExtent.X * 2.0f / NodeSpacing);
    const int32 NodesY = FMath::CeilToInt(BoundsExtent.Y * 2.0f / NodeSpacing);

    for (int32 X = 0; X < NodesX; X++)
    {
        for (int32 Y = 0; Y < NodesY; Y++)
        {
            FVector NodeLocation = BoundsOrigin + FVector(
                (X - NodesX / 2) * NodeSpacing,
                (Y - NodesY / 2) * NodeSpacing,
                0.0f
            );

            // Project to NavMesh
            FNavLocation NavLocation;
            if (NavSystem->ProjectPointToNavigation(NodeLocation, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
            {
                FCrowd_PathNode NewNode;
                NewNode.Location = NavLocation.Location;
                NewNode.Cost = 0.0f;
                NewNode.bIsBlocked = false;
                NewNode.NodeIndex = HierarchicalNodes.Num();
                HierarchicalNodes.Add(NewNode);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingManager: Built hierarchical graph with %d nodes"), HierarchicalNodes.Num());
}

TArray<FVector> UCrowd_PathfindingManager::FindHierarchicalPath(const FVector& Start, const FVector& Goal)
{
    TArray<FVector> Path;

    if (HierarchicalNodes.Num() == 0)
    {
        // Fallback to direct pathfinding
        FNavLocation StartNav, GoalNav;
        if (NavSystem->ProjectPointToNavigation(Start, StartNav) && 
            NavSystem->ProjectPointToNavigation(Goal, GoalNav))
        {
            Path.Add(StartNav.Location);
            Path.Add(GoalNav.Location);
        }
        return Path;
    }

    // Find closest hierarchical nodes to start and goal
    int32 StartNodeIndex = -1, GoalNodeIndex = -1;
    float StartDistSq = FLT_MAX, GoalDistSq = FLT_MAX;

    for (int32 i = 0; i < HierarchicalNodes.Num(); i++)
    {
        float StartDist = FVector::DistSquared(Start, HierarchicalNodes[i].Location);
        float GoalDist = FVector::DistSquared(Goal, HierarchicalNodes[i].Location);

        if (StartDist < StartDistSq)
        {
            StartDistSq = StartDist;
            StartNodeIndex = i;
        }
        if (GoalDist < GoalDistSq)
        {
            GoalDistSq = GoalDist;
            GoalNodeIndex = i;
        }
    }

    if (StartNodeIndex >= 0 && GoalNodeIndex >= 0)
    {
        Path.Add(Start);
        if (StartNodeIndex != GoalNodeIndex)
        {
            Path.Add(HierarchicalNodes[StartNodeIndex].Location);
            Path.Add(HierarchicalNodes[GoalNodeIndex].Location);
        }
        Path.Add(Goal);
    }

    return Path;
}

void UCrowd_PathfindingManager::GenerateFlowField(const FVector& TargetLocation, float Radius)
{
    FlowFieldTarget = TargetLocation;
    FlowFieldGrid.Empty();

    // Create a grid around the target location
    const int32 GridSize = FMath::CeilToInt(Radius * 2.0f / FlowFieldCellSize);
    const FVector GridOrigin = TargetLocation - FVector(Radius, Radius, 0.0f);

    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FIntPoint GridCoord(X, Y);
            CalculateFlowFieldCell(GridCoord);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("UCrowd_PathfindingManager: Generated flow field with %d cells"), FlowFieldGrid.Num());
}

FVector UCrowd_PathfindingManager::GetFlowDirection(const FVector& Location)
{
    FIntPoint GridCoord = WorldToGridCoord(Location);
    
    if (FlowFieldGrid.Contains(GridCoord))
    {
        return FlowFieldGrid[GridCoord];
    }

    // Fallback: direct direction to target
    FVector Direction = (FlowFieldTarget - Location).GetSafeNormal();
    return Direction;
}

void UCrowd_PathfindingManager::RegisterDynamicObstacle(AActor* Obstacle)
{
    if (Obstacle && !DynamicObstacles.Contains(Obstacle))
    {
        DynamicObstacles.Add(Obstacle);
        UE_LOG(LogTemp, VeryVerbose, TEXT("UCrowd_PathfindingManager: Registered dynamic obstacle %s"), *Obstacle->GetName());
    }
}

void UCrowd_PathfindingManager::UnregisterDynamicObstacle(AActor* Obstacle)
{
    DynamicObstacles.Remove(Obstacle);
}

bool UCrowd_PathfindingManager::IsLocationBlocked(const FVector& Location, float Radius)
{
    // Check against dynamic obstacles
    for (AActor* Obstacle : DynamicObstacles)
    {
        if (IsValid(Obstacle))
        {
            float Distance = FVector::Dist(Location, Obstacle->GetActorLocation());
            if (Distance < Radius + 100.0f) // 1m buffer
            {
                return true;
            }
        }
    }

    // Check NavMesh validity
    FNavLocation NavLocation;
    return !NavSystem->ProjectPointToNavigation(Location, NavLocation, FVector(Radius, Radius, 500.0f));
}

void UCrowd_PathfindingManager::SetMaxPathRequestsPerFrame(int32 MaxRequests)
{
    MaxPathRequestsPerFrame = FMath::Max(1, MaxRequests);
}

void UCrowd_PathfindingManager::EnableLODPathfinding(bool bEnable)
{
    bUseLODPathfinding = bEnable;
}

void UCrowd_PathfindingManager::SetPathfindingLOD(int32 AgentID, int32 LODLevel)
{
    // Implementation would store LOD level per agent
    // For now, just log the request
    UE_LOG(LogTemp, VeryVerbose, TEXT("UCrowd_PathfindingManager: Set LOD %d for agent %d"), LODLevel, AgentID);
}

void UCrowd_PathfindingManager::DebugDrawPaths()
{
    if (!GetWorld())
        return;

    for (const auto& PathPair : ActivePaths)
    {
        const TArray<FVector>& Path = PathPair.Value;
        for (int32 i = 0; i < Path.Num() - 1; i++)
        {
            DrawDebugLine(GetWorld(), Path[i], Path[i + 1], FColor::Green, false, DebugDrawDuration, 0, 2.0f);
        }
    }
}

void UCrowd_PathfindingManager::DebugDrawFlowField()
{
    if (!GetWorld())
        return;

    for (const auto& FlowPair : FlowFieldGrid)
    {
        FVector WorldPos = GridToWorldCoord(FlowPair.Key);
        FVector FlowDir = FlowPair.Value;
        
        DrawDebugLine(GetWorld(), WorldPos, WorldPos + FlowDir * 100.0f, FColor::Blue, false, DebugDrawDuration, 0, 1.0f);
        DrawDebugSphere(GetWorld(), WorldPos, 10.0f, 8, FColor::Cyan, false, DebugDrawDuration);
    }
}

void UCrowd_PathfindingManager::ProcessPathRequests(float DeltaTime)
{
    if (PendingRequests.Num() == 0)
        return;

    // Sort requests by priority
    PendingRequests.Sort([](const FCrowd_PathRequest& A, const FCrowd_PathRequest& B)
    {
        return A.Priority > B.Priority;
    });

    // Process up to MaxPathRequestsPerFrame requests
    int32 ProcessedCount = 0;
    for (int32 i = PendingRequests.Num() - 1; i >= 0 && ProcessedCount < MaxPathRequestsPerFrame; i--)
    {
        const FCrowd_PathRequest& Request = PendingRequests[i];

        // Generate path using hierarchical pathfinding
        TArray<FVector> NewPath = FindHierarchicalPath(Request.StartLocation, Request.TargetLocation);
        
        if (NewPath.Num() > 0)
        {
            ActivePaths.Add(Request.AgentID, NewPath);
            UE_LOG(LogTemp, VeryVerbose, TEXT("UCrowd_PathfindingManager: Generated path for agent %d with %d waypoints"), Request.AgentID, NewPath.Num());
        }

        PendingRequests.RemoveAt(i);
        ProcessedCount++;
    }

    PathRequestsProcessedThisFrame = ProcessedCount;
}

void UCrowd_PathfindingManager::UpdateFlowField()
{
    // Update flow field cells that might have changed
    for (auto& FlowPair : FlowFieldGrid)
    {
        CalculateFlowFieldCell(FlowPair.Key);
    }
}

void UCrowd_PathfindingManager::UpdateDynamicObstacles()
{
    // Remove invalid obstacles
    DynamicObstacles.RemoveAll([](AActor* Obstacle)
    {
        return !IsValid(Obstacle);
    });
}

void UCrowd_PathfindingManager::CleanupCompletedPaths()
{
    // Remove paths for agents that no longer exist or have reached their destination
    // This would require integration with the crowd management system
    // For now, keep all paths until explicitly cancelled
}

FIntPoint UCrowd_PathfindingManager::WorldToGridCoord(const FVector& WorldLocation)
{
    FVector RelativePos = WorldLocation - (FlowFieldTarget - FVector(5000.0f, 5000.0f, 0.0f));
    return FIntPoint(
        FMath::FloorToInt(RelativePos.X / FlowFieldCellSize),
        FMath::FloorToInt(RelativePos.Y / FlowFieldCellSize)
    );
}

FVector UCrowd_PathfindingManager::GridToWorldCoord(const FIntPoint& GridCoord)
{
    return FlowFieldTarget - FVector(5000.0f, 5000.0f, 0.0f) + FVector(
        GridCoord.X * FlowFieldCellSize + FlowFieldCellSize * 0.5f,
        GridCoord.Y * FlowFieldCellSize + FlowFieldCellSize * 0.5f,
        0.0f
    );
}

void UCrowd_PathfindingManager::CalculateFlowFieldCell(const FIntPoint& GridCoord)
{
    FVector CellWorldPos = GridToWorldCoord(GridCoord);
    
    // Calculate flow direction towards target, avoiding obstacles
    FVector DirectionToTarget = (FlowFieldTarget - CellWorldPos).GetSafeNormal();
    
    // Check for obstacles and adjust direction
    if (IsLocationBlocked(CellWorldPos, FlowFieldCellSize * 0.5f))
    {
        DirectionToTarget = FVector::ZeroVector; // No flow in blocked areas
    }
    
    FlowFieldGrid.Add(GridCoord, DirectionToTarget);
}
#include "Crowd_PathfindingManager.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "NavigationPath.h"
#include "AI/NavigationSystemBase.h"
#include "Kismet/KismetMathLibrary.h"

ACrowd_PathfindingManager::ACrowd_PathfindingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    MaxPathDistance = 10000.0f;
    MaxSimultaneousRequests = 100;
    PathfindingBudgetPerFrame = 0.016f; // 16ms budget
    MaxPathNodesPerRequest = 50;
    
    NavSystem = nullptr;
    NavMesh = nullptr;
}

void ACrowd_PathfindingManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize navigation system
    NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (NavSystem)
    {
        NavMesh = Cast<ARecastNavMesh>(NavSystem->GetDefaultNavDataInstance());
        UE_LOG(LogTemp, Warning, TEXT("Crowd Pathfinding Manager initialized with NavMesh"));
    }
    
    // Build initial path network
    BuildPathNetwork();
}

void ACrowd_PathfindingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Process pathfinding requests within frame budget
    ProcessPathRequests(DeltaTime);
}

bool ACrowd_PathfindingManager::RequestPath(const FVector& Start, const FVector& Target, FMassEntityHandle EntityHandle, float Priority)
{
    if (PendingRequests.Num() >= MaxSimultaneousRequests)
    {
        return false;
    }
    
    FCrowd_PathRequest NewRequest;
    NewRequest.StartLocation = Start;
    NewRequest.TargetLocation = Target;
    NewRequest.EntityHandle = EntityHandle;
    NewRequest.Priority = Priority;
    NewRequest.bIsValid = true;
    
    // Insert based on priority (higher priority first)
    int32 InsertIndex = 0;
    for (int32 i = 0; i < PendingRequests.Num(); i++)
    {
        if (PendingRequests[i].Priority < Priority)
        {
            InsertIndex = i;
            break;
        }
        InsertIndex = i + 1;
    }
    
    PendingRequests.Insert(NewRequest, InsertIndex);
    return true;
}

TArray<FVector> ACrowd_PathfindingManager::GetPath(const FVector& Start, const FVector& Target)
{
    TArray<FVector> PathPoints;
    
    if (!NavSystem || !NavMesh)
    {
        return PathPoints;
    }
    
    // Use UE5 navigation system for pathfinding
    FNavLocation StartNavLocation, TargetNavLocation;
    
    if (NavSystem->ProjectPointToNavigation(Start, StartNavLocation) &&
        NavSystem->ProjectPointToNavigation(Target, TargetNavLocation))
    {
        FPathFindingQuery Query;
        Query.StartLocation = StartNavLocation.Location;
        Query.EndLocation = TargetNavLocation.Location;
        Query.NavData = NavMesh;
        
        FPathFindingResult Result = NavSystem->FindPathSync(Query);
        
        if (Result.IsSuccessful() && Result.Path.IsValid())
        {
            const TArray<FNavPathPoint>& Points = Result.Path->GetPathPoints();
            for (const FNavPathPoint& Point : Points)
            {
                PathPoints.Add(Point.Location);
            }
        }
    }
    
    return PathPoints;
}

void ACrowd_PathfindingManager::BuildPathNetwork()
{
    PathNodes.Empty();
    
    // Create a grid-based path network for crowd simulation
    const int32 GridSize = 20;
    const float Spacing = 500.0f;
    const FVector Center = GetActorLocation();
    
    // Generate grid nodes
    for (int32 X = -GridSize/2; X <= GridSize/2; X++)
    {
        for (int32 Y = -GridSize/2; Y <= GridSize/2; Y++)
        {
            FVector NodePosition = Center + FVector(X * Spacing, Y * Spacing, 0);
            
            // Project to navigation mesh
            if (NavSystem)
            {
                FNavLocation NavLocation;
                if (NavSystem->ProjectPointToNavigation(NodePosition, NavLocation))
                {
                    AddPathNode(NavLocation.Location);
                }
            }
        }
    }
    
    // Connect adjacent nodes
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        for (int32 j = i + 1; j < PathNodes.Num(); j++)
        {
            float Distance = FVector::Dist(PathNodes[i].Position, PathNodes[j].Position);
            if (Distance <= Spacing * 1.5f) // Connect if within 1.5x spacing
            {
                ConnectNodes(i, j);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Built path network with %d nodes"), PathNodes.Num());
}

void ACrowd_PathfindingManager::AddPathNode(const FVector& Position, float Weight)
{
    FCrowd_PathNode NewNode;
    NewNode.Position = Position;
    NewNode.Weight = Weight;
    NewNode.NodeID = PathNodes.Num();
    
    PathNodes.Add(NewNode);
}

void ACrowd_PathfindingManager::ConnectNodes(int32 NodeA, int32 NodeB)
{
    if (PathNodes.IsValidIndex(NodeA) && PathNodes.IsValidIndex(NodeB))
    {
        PathNodes[NodeA].ConnectedNodes.AddUnique(NodeB);
        PathNodes[NodeB].ConnectedNodes.AddUnique(NodeA);
    }
}

void ACrowd_PathfindingManager::ProcessPathRequests(float DeltaTime)
{
    if (PendingRequests.Num() == 0)
    {
        return;
    }
    
    float TimeUsed = 0.0f;
    int32 RequestsProcessed = 0;
    
    while (TimeUsed < PathfindingBudgetPerFrame && PendingRequests.Num() > 0)
    {
        float StartTime = FPlatformTime::Seconds();
        
        FCrowd_PathRequest& Request = PendingRequests[0];
        
        // Process the pathfinding request
        TArray<FVector> Path = GetPath(Request.StartLocation, Request.TargetLocation);
        
        // TODO: Send path result back to Mass Entity system
        // This would integrate with the Mass Entity framework
        
        PendingRequests.RemoveAt(0);
        RequestsProcessed++;
        
        float EndTime = FPlatformTime::Seconds();
        TimeUsed += (EndTime - StartTime);
    }
    
    if (RequestsProcessed > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Processed %d pathfinding requests in %.2fms"), 
               RequestsProcessed, TimeUsed * 1000.0f);
    }
}

void ACrowd_PathfindingManager::DrawPathNetwork()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw nodes
    for (const FCrowd_PathNode& Node : PathNodes)
    {
        DrawDebugSphere(GetWorld(), Node.Position, 50.0f, 8, FColor::Blue, false, 1.0f);
    }
    
    // Draw connections
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        const FCrowd_PathNode& Node = PathNodes[i];
        for (int32 ConnectedID : Node.ConnectedNodes)
        {
            if (PathNodes.IsValidIndex(ConnectedID))
            {
                DrawDebugLine(GetWorld(), Node.Position, PathNodes[ConnectedID].Position, 
                             FColor::Green, false, 1.0f, 0, 2.0f);
            }
        }
    }
}

void ACrowd_PathfindingManager::DebugDrawPath(const TArray<FVector>& Path)
{
    if (!GetWorld() || Path.Num() < 2)
    {
        return;
    }
    
    for (int32 i = 0; i < Path.Num() - 1; i++)
    {
        DrawDebugLine(GetWorld(), Path[i], Path[i + 1], FColor::Red, false, 5.0f, 0, 5.0f);
        DrawDebugSphere(GetWorld(), Path[i], 25.0f, 8, FColor::Orange, false, 5.0f);
    }
    
    // Draw final destination
    DrawDebugSphere(GetWorld(), Path.Last(), 50.0f, 8, FColor::Red, false, 5.0f);
}
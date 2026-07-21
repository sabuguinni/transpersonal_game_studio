#include "Crowd_PathfindingNetwork.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ACrowd_PathfindingNetwork::ACrowd_PathfindingNetwork()
{
    PrimaryActorTick.bCanEverTick = true;

    NetworkMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NetworkMesh"));
    RootComponent = NetworkMesh;

    // Set default values
    NodeSpacing = 500.0f;
    MaxNodesPerRing = 12;
    NetworkRadius = 2000.0f;

    // Load a basic sphere mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        NetworkMesh->SetStaticMesh(SphereMeshAsset.Object);
        NetworkMesh->SetWorldScale3D(FVector(0.5f, 0.5f, 0.5f));
    }
}

void ACrowd_PathfindingNetwork::BeginPlay()
{
    Super::BeginPlay();
    
    // Generate default circular network
    GenerateCircularNetwork(GetActorLocation(), NetworkRadius, MaxNodesPerRing);
}

void ACrowd_PathfindingNetwork::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update traffic density over time
    for (auto& Node : PathNodes)
    {
        // Gradually reduce traffic density
        Node.TrafficDensity = FMath::Max(0.0f, Node.TrafficDensity - DeltaTime * 0.1f);
    }
}

void ACrowd_PathfindingNetwork::GenerateCircularNetwork(FVector Center, float Radius, int32 NodeCount)
{
    PathNodes.Empty();
    
    // Create outer ring
    for (int32 i = 0; i < NodeCount; i++)
    {
        float Angle = (2.0f * PI * i) / NodeCount;
        FVector NodePos = Center + FVector(
            Radius * FMath::Cos(Angle),
            Radius * FMath::Sin(Angle),
            0.0f
        );
        
        FCrowd_PathfindingNode NewNode;
        NewNode.Position = NodePos;
        NewNode.TrafficDensity = 0.0f;
        NewNode.PathType = ECrowd_PathType::Normal;
        PathNodes.Add(NewNode);
    }
    
    // Create inner ring
    int32 InnerNodeCount = NodeCount / 2;
    float InnerRadius = Radius * 0.5f;
    
    for (int32 i = 0; i < InnerNodeCount; i++)
    {
        float Angle = (2.0f * PI * i) / InnerNodeCount;
        FVector NodePos = Center + FVector(
            InnerRadius * FMath::Cos(Angle),
            InnerRadius * FMath::Sin(Angle),
            50.0f
        );
        
        FCrowd_PathfindingNode NewNode;
        NewNode.Position = NodePos;
        NewNode.TrafficDensity = 0.0f;
        NewNode.PathType = ECrowd_PathType::Priority;
        PathNodes.Add(NewNode);
    }
    
    ConnectNearbyNodes();
}

void ACrowd_PathfindingNetwork::GenerateGridNetwork(FVector Origin, int32 GridSize, float CellSize)
{
    PathNodes.Empty();
    
    for (int32 X = 0; X < GridSize; X++)
    {
        for (int32 Y = 0; Y < GridSize; Y++)
        {
            FVector NodePos = Origin + FVector(X * CellSize, Y * CellSize, 0.0f);
            
            FCrowd_PathfindingNode NewNode;
            NewNode.Position = NodePos;
            NewNode.TrafficDensity = 0.0f;
            NewNode.PathType = ECrowd_PathType::Normal;
            PathNodes.Add(NewNode);
        }
    }
    
    CalculateNodeConnections();
}

void ACrowd_PathfindingNetwork::ConnectNearbyNodes()
{
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        PathNodes[i].ConnectedNodes.Empty();
        
        for (int32 j = 0; j < PathNodes.Num(); j++)
        {
            if (i != j)
            {
                float Distance = FVector::Dist(PathNodes[i].Position, PathNodes[j].Position);
                
                // Connect nodes within reasonable distance
                if (Distance <= NodeSpacing * 1.5f)
                {
                    PathNodes[i].ConnectedNodes.Add(j);
                }
            }
        }
    }
}

void ACrowd_PathfindingNetwork::CalculateNodeConnections()
{
    // Grid-based connections (4-directional)
    int32 GridSize = FMath::Sqrt(PathNodes.Num());
    
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        PathNodes[i].ConnectedNodes.Empty();
        
        int32 X = i % GridSize;
        int32 Y = i / GridSize;
        
        // Connect to adjacent grid cells
        TArray<FIntPoint> Directions = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
        
        for (const FIntPoint& Dir : Directions)
        {
            int32 NewX = X + Dir.X;
            int32 NewY = Y + Dir.Y;
            
            if (NewX >= 0 && NewX < GridSize && NewY >= 0 && NewY < GridSize)
            {
                int32 ConnectedIndex = NewY * GridSize + NewX;
                PathNodes[i].ConnectedNodes.Add(ConnectedIndex);
            }
        }
    }
}

TArray<int32> ACrowd_PathfindingNetwork::FindPath(int32 StartNode, int32 EndNode)
{
    TArray<int32> Path;
    
    if (StartNode < 0 || StartNode >= PathNodes.Num() || EndNode < 0 || EndNode >= PathNodes.Num())
    {
        return Path;
    }
    
    // Simple A* pathfinding implementation
    TArray<float> GScore;
    TArray<float> FScore;
    TArray<int32> CameFrom;
    TArray<bool> InOpenSet;
    TArray<bool> InClosedSet;
    
    GScore.Init(FLT_MAX, PathNodes.Num());
    FScore.Init(FLT_MAX, PathNodes.Num());
    CameFrom.Init(-1, PathNodes.Num());
    InOpenSet.Init(false, PathNodes.Num());
    InClosedSet.Init(false, PathNodes.Num());
    
    GScore[StartNode] = 0.0f;
    FScore[StartNode] = CalculateHeuristic(StartNode, EndNode);
    InOpenSet[StartNode] = true;
    
    while (true)
    {
        // Find node in open set with lowest F score
        int32 Current = -1;
        float LowestF = FLT_MAX;
        
        for (int32 i = 0; i < PathNodes.Num(); i++)
        {
            if (InOpenSet[i] && FScore[i] < LowestF)
            {
                LowestF = FScore[i];
                Current = i;
            }
        }
        
        if (Current == -1)
        {
            break; // No path found
        }
        
        if (Current == EndNode)
        {
            // Reconstruct path
            int32 Node = EndNode;
            while (Node != -1)
            {
                Path.Insert(Node, 0);
                Node = CameFrom[Node];
            }
            break;
        }
        
        InOpenSet[Current] = false;
        InClosedSet[Current] = true;
        
        // Check neighbors
        for (int32 Neighbor : PathNodes[Current].ConnectedNodes)
        {
            if (InClosedSet[Neighbor])
            {
                continue;
            }
            
            float TentativeG = GScore[Current] + GetPathCost(Current, Neighbor);
            
            if (!InOpenSet[Neighbor])
            {
                InOpenSet[Neighbor] = true;
            }
            else if (TentativeG >= GScore[Neighbor])
            {
                continue;
            }
            
            CameFrom[Neighbor] = Current;
            GScore[Neighbor] = TentativeG;
            FScore[Neighbor] = GScore[Neighbor] + CalculateHeuristic(Neighbor, EndNode);
        }
    }
    
    return Path;
}

int32 ACrowd_PathfindingNetwork::GetNearestNode(FVector WorldPosition)
{
    int32 NearestNode = -1;
    float MinDistance = FLT_MAX;
    
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        float Distance = FVector::Dist(WorldPosition, PathNodes[i].Position);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestNode = i;
        }
    }
    
    return NearestNode;
}

void ACrowd_PathfindingNetwork::UpdateTrafficDensity(int32 NodeIndex, float DensityChange)
{
    if (NodeIndex >= 0 && NodeIndex < PathNodes.Num())
    {
        PathNodes[NodeIndex].TrafficDensity = FMath::Clamp(
            PathNodes[NodeIndex].TrafficDensity + DensityChange,
            0.0f,
            10.0f
        );
    }
}

float ACrowd_PathfindingNetwork::GetPathCost(int32 FromNode, int32 ToNode)
{
    if (FromNode < 0 || FromNode >= PathNodes.Num() || ToNode < 0 || ToNode >= PathNodes.Num())
    {
        return FLT_MAX;
    }
    
    float BaseCost = FVector::Dist(PathNodes[FromNode].Position, PathNodes[ToNode].Position);
    float TrafficPenalty = PathNodes[ToNode].TrafficDensity * 50.0f;
    
    // Priority paths have lower cost
    if (PathNodes[ToNode].PathType == ECrowd_PathType::Priority)
    {
        BaseCost *= 0.5f;
    }
    
    return BaseCost + TrafficPenalty;
}

void ACrowd_PathfindingNetwork::OptimizeNetworkForPerformance()
{
    // Remove nodes with very low usage
    for (int32 i = PathNodes.Num() - 1; i >= 0; i--)
    {
        if (PathNodes[i].TrafficDensity < 0.1f && PathNodes[i].ConnectedNodes.Num() <= 2)
        {
            // Remove this node and update connections
            for (auto& Node : PathNodes)
            {
                Node.ConnectedNodes.Remove(i);
                // Adjust indices for removed node
                for (int32& ConnectedIndex : Node.ConnectedNodes)
                {
                    if (ConnectedIndex > i)
                    {
                        ConnectedIndex--;
                    }
                }
            }
            PathNodes.RemoveAt(i);
        }
    }
}

void ACrowd_PathfindingNetwork::DebugDrawNetwork()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Draw nodes
    for (int32 i = 0; i < PathNodes.Num(); i++)
    {
        FColor NodeColor = FColor::Green;
        if (PathNodes[i].PathType == ECrowd_PathType::Priority)
        {
            NodeColor = FColor::Blue;
        }
        
        float SphereSize = 20.0f + PathNodes[i].TrafficDensity * 10.0f;
        DrawDebugSphere(World, PathNodes[i].Position, SphereSize, 8, NodeColor, false, 1.0f);
        
        // Draw connections
        for (int32 ConnectedIndex : PathNodes[i].ConnectedNodes)
        {
            if (ConnectedIndex < PathNodes.Num())
            {
                DrawDebugLine(
                    World,
                    PathNodes[i].Position,
                    PathNodes[ConnectedIndex].Position,
                    FColor::Yellow,
                    false,
                    1.0f,
                    0,
                    2.0f
                );
            }
        }
    }
}

float ACrowd_PathfindingNetwork::CalculateHeuristic(int32 NodeA, int32 NodeB)
{
    if (NodeA < 0 || NodeA >= PathNodes.Num() || NodeB < 0 || NodeB >= PathNodes.Num())
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(PathNodes[NodeA].Position, PathNodes[NodeB].Position);
}
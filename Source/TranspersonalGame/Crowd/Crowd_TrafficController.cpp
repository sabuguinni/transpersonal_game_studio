#include "Crowd_TrafficController.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

ACrowd_TrafficController::ACrowd_TrafficController()
{
    PrimaryActorTick.bCanEverTick = true;
    UpdateInterval = 0.1f;
    MaxActiveFlows = 100;
    LODDistance = 2000.0f;
    LastUpdateTime = 0.0f;
}

void ACrowd_TrafficController::BeginPlay()
{
    Super::BeginPlay();
    InitializeTrafficNetwork();
}

void ACrowd_TrafficController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateTrafficFlow(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_TrafficController::InitializeTrafficNetwork()
{
    TrafficNodes.Empty();
    TrafficFlows.Empty();
    ActiveFlowIndices.Empty();

    // Create default traffic network
    // Central hub
    AddTrafficNode(FVector(0, 0, 50), 20.0f, ECrowd_TrafficType::Pedestrian);
    
    // Village areas
    AddTrafficNode(FVector(500, 500, 50), 15.0f, ECrowd_TrafficType::Pedestrian);
    AddTrafficNode(FVector(-500, 300, 50), 12.0f, ECrowd_TrafficType::Pedestrian);
    AddTrafficNode(FVector(800, -400, 50), 10.0f, ECrowd_TrafficType::Pedestrian);
    AddTrafficNode(FVector(-300, -600, 50), 8.0f, ECrowd_TrafficType::Pedestrian);

    // Resource gathering points
    AddTrafficNode(FVector(1200, 0, 50), 5.0f, ECrowd_TrafficType::Resource);
    AddTrafficNode(FVector(0, 1200, 50), 5.0f, ECrowd_TrafficType::Resource);
    AddTrafficNode(FVector(-1200, 0, 50), 5.0f, ECrowd_TrafficType::Resource);

    // Connect nodes to create traffic flows
    ConnectNodes(0, 1, 2.0f); // Central to village 1
    ConnectNodes(0, 2, 2.0f); // Central to village 2
    ConnectNodes(0, 3, 1.5f); // Central to village 3
    ConnectNodes(0, 4, 1.5f); // Central to village 4
    
    ConnectNodes(1, 5, 1.0f); // Village 1 to resource 1
    ConnectNodes(2, 6, 1.0f); // Village 2 to resource 2
    ConnectNodes(3, 7, 1.0f); // Village 3 to resource 3
    
    // Inter-village connections
    ConnectNodes(1, 2, 0.5f);
    ConnectNodes(2, 3, 0.5f);
    ConnectNodes(3, 4, 0.5f);

    UE_LOG(LogTemp, Warning, TEXT("Traffic network initialized with %d nodes and %d flows"), 
           TrafficNodes.Num(), TrafficFlows.Num());
}

int32 ACrowd_TrafficController::AddTrafficNode(const FVector& Position, float Capacity, ECrowd_TrafficType TrafficType)
{
    FCrowd_TrafficNode NewNode;
    NewNode.Position = Position;
    NewNode.Capacity = Capacity;
    NewNode.TrafficType = TrafficType;
    
    return TrafficNodes.Add(NewNode);
}

void ACrowd_TrafficController::ConnectNodes(int32 NodeA, int32 NodeB, float FlowRate)
{
    if (NodeA >= 0 && NodeA < TrafficNodes.Num() && NodeB >= 0 && NodeB < TrafficNodes.Num())
    {
        // Add connection to node A
        TrafficNodes[NodeA].ConnectedNodes.AddUnique(NodeB);
        TrafficNodes[NodeB].ConnectedNodes.AddUnique(NodeA);

        // Create traffic flow
        FCrowd_TrafficFlow NewFlow;
        NewFlow.SourceNodeIndex = NodeA;
        NewFlow.DestinationNodeIndex = NodeB;
        NewFlow.FlowRate = FlowRate;
        NewFlow.CurrentLoad = 0.0f;
        
        TrafficFlows.Add(NewFlow);

        // Create reverse flow
        FCrowd_TrafficFlow ReverseFlow;
        ReverseFlow.SourceNodeIndex = NodeB;
        ReverseFlow.DestinationNodeIndex = NodeA;
        ReverseFlow.FlowRate = FlowRate * 0.8f; // Slightly lower reverse flow
        ReverseFlow.CurrentLoad = 0.0f;
        
        TrafficFlows.Add(ReverseFlow);
    }
}

void ACrowd_TrafficController::UpdateTrafficFlow(float DeltaTime)
{
    // Update active flows based on LOD
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        ApplyLODOptimization(PlayerPawn->GetActorLocation());
    }

    // Update flow loads
    for (int32 FlowIndex : ActiveFlowIndices)
    {
        if (FlowIndex >= 0 && FlowIndex < TrafficFlows.Num())
        {
            FCrowd_TrafficFlow& Flow = TrafficFlows[FlowIndex];
            
            // Simulate traffic flow dynamics
            float TargetLoad = Flow.FlowRate * FMath::Sin(GetWorld()->GetTimeSeconds() * 0.5f + FlowIndex) * 0.5f + 0.5f;
            Flow.CurrentLoad = FMath::FInterpTo(Flow.CurrentLoad, TargetLoad, DeltaTime, 2.0f);
        }
    }
}

TArray<int32> ACrowd_TrafficController::FindOptimalPath(int32 StartNode, int32 EndNode)
{
    TArray<int32> Path;
    
    if (StartNode < 0 || StartNode >= TrafficNodes.Num() || 
        EndNode < 0 || EndNode >= TrafficNodes.Num())
    {
        return Path;
    }

    // Simple A* pathfinding implementation
    TArray<bool> Visited;
    TArray<int32> Parent;
    TArray<float> Distance;
    
    Visited.SetNum(TrafficNodes.Num());
    Parent.SetNum(TrafficNodes.Num());
    Distance.SetNum(TrafficNodes.Num());
    
    for (int32 i = 0; i < TrafficNodes.Num(); i++)
    {
        Visited[i] = false;
        Parent[i] = -1;
        Distance[i] = FLT_MAX;
    }
    
    Distance[StartNode] = 0.0f;
    
    for (int32 Count = 0; Count < TrafficNodes.Num() - 1; Count++)
    {
        int32 MinIndex = -1;
        float MinDistance = FLT_MAX;
        
        for (int32 i = 0; i < TrafficNodes.Num(); i++)
        {
            if (!Visited[i] && Distance[i] < MinDistance)
            {
                MinDistance = Distance[i];
                MinIndex = i;
            }
        }
        
        if (MinIndex == -1) break;
        
        Visited[MinIndex] = true;
        
        if (MinIndex == EndNode) break;
        
        for (int32 ConnectedNode : TrafficNodes[MinIndex].ConnectedNodes)
        {
            if (!Visited[ConnectedNode])
            {
                float EdgeWeight = FVector::Dist(TrafficNodes[MinIndex].Position, TrafficNodes[ConnectedNode].Position);
                float NewDistance = Distance[MinIndex] + EdgeWeight;
                
                if (NewDistance < Distance[ConnectedNode])
                {
                    Distance[ConnectedNode] = NewDistance;
                    Parent[ConnectedNode] = MinIndex;
                }
            }
        }
    }
    
    // Reconstruct path
    if (Distance[EndNode] != FLT_MAX)
    {
        int32 Current = EndNode;
        while (Current != -1)
        {
            Path.Insert(Current, 0);
            Current = Parent[Current];
        }
    }
    
    return Path;
}

void ACrowd_TrafficController::OptimizeTrafficLoad()
{
    // Balance traffic loads across the network
    for (FCrowd_TrafficFlow& Flow : TrafficFlows)
    {
        if (Flow.SourceNodeIndex >= 0 && Flow.SourceNodeIndex < TrafficNodes.Num() &&
            Flow.DestinationNodeIndex >= 0 && Flow.DestinationNodeIndex < TrafficNodes.Num())
        {
            FCrowd_TrafficNode& SourceNode = TrafficNodes[Flow.SourceNodeIndex];
            FCrowd_TrafficNode& DestNode = TrafficNodes[Flow.DestinationNodeIndex];
            
            // Adjust flow rate based on node capacities
            float CapacityRatio = FMath::Min(SourceNode.Capacity, DestNode.Capacity) / FMath::Max(SourceNode.Capacity, DestNode.Capacity);
            Flow.FlowRate = Flow.FlowRate * CapacityRatio;
        }
    }
}

void ACrowd_TrafficController::ApplyLODOptimization(const FVector& ViewerPosition)
{
    ActiveFlowIndices.Empty();
    
    for (int32 i = 0; i < TrafficFlows.Num(); i++)
    {
        const FCrowd_TrafficFlow& Flow = TrafficFlows[i];
        
        if (Flow.SourceNodeIndex >= 0 && Flow.SourceNodeIndex < TrafficNodes.Num())
        {
            float Distance = FVector::Dist(ViewerPosition, TrafficNodes[Flow.SourceNodeIndex].Position);
            
            if (Distance <= LODDistance && ActiveFlowIndices.Num() < MaxActiveFlows)
            {
                ActiveFlowIndices.Add(i);
            }
        }
    }
}

void ACrowd_TrafficController::DrawTrafficNetwork()
{
    if (!GetWorld()) return;
    
    // Draw nodes
    for (int32 i = 0; i < TrafficNodes.Num(); i++)
    {
        const FCrowd_TrafficNode& Node = TrafficNodes[i];
        FColor NodeColor = FColor::Green;
        
        switch (Node.TrafficType)
        {
            case ECrowd_TrafficType::Pedestrian:
                NodeColor = FColor::Blue;
                break;
            case ECrowd_TrafficType::Resource:
                NodeColor = FColor::Yellow;
                break;
            case ECrowd_TrafficType::Emergency:
                NodeColor = FColor::Red;
                break;
        }
        
        DrawDebugSphere(GetWorld(), Node.Position, Node.Capacity * 10.0f, 12, NodeColor, false, 5.0f);
    }
    
    // Draw connections
    for (const FCrowd_TrafficFlow& Flow : TrafficFlows)
    {
        if (Flow.SourceNodeIndex >= 0 && Flow.SourceNodeIndex < TrafficNodes.Num() &&
            Flow.DestinationNodeIndex >= 0 && Flow.DestinationNodeIndex < TrafficNodes.Num())
        {
            const FVector& StartPos = TrafficNodes[Flow.SourceNodeIndex].Position;
            const FVector& EndPos = TrafficNodes[Flow.DestinationNodeIndex].Position;
            
            float LoadIntensity = FMath::Clamp(Flow.CurrentLoad / Flow.FlowRate, 0.0f, 1.0f);
            FColor LineColor = FMath::Lerp(FColor::Green, FColor::Red, LoadIntensity);
            
            DrawDebugLine(GetWorld(), StartPos, EndPos, LineColor, false, 5.0f, 0, 2.0f);
        }
    }
}
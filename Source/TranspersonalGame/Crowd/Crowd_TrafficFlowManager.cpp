#include "Crowd_TrafficFlowManager.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

ACrowd_TrafficFlowManager::ACrowd_TrafficFlowManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    GridCellSize = 500.0f;
    MaxFlowSpeed = 600.0f;
    CongestionThreshold = 0.7f;
}

void ACrowd_TrafficFlowManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeTrafficNetwork();
}

void ACrowd_TrafficFlowManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateFlowFields();
}

void ACrowd_TrafficFlowManager::InitializeTrafficNetwork()
{
    // Clear existing network
    TrafficNodes.Empty();
    FlowFieldGrid.Empty();

    // Create major traffic nodes around the map
    AddTrafficNode(FVector(0, 0, 0), 200.0f);           // Center hub
    AddTrafficNode(FVector(2000, 0, 0), 150.0f);        // East node
    AddTrafficNode(FVector(-2000, 0, 0), 150.0f);       // West node  
    AddTrafficNode(FVector(0, 2000, 0), 150.0f);        // North node
    AddTrafficNode(FVector(0, -2000, 0), 150.0f);       // South node
    AddTrafficNode(FVector(1500, 1500, 0), 100.0f);     // NE residential
    AddTrafficNode(FVector(-1500, 1500, 0), 100.0f);    // NW residential
    AddTrafficNode(FVector(1500, -1500, 0), 100.0f);    // SE residential
    AddTrafficNode(FVector(-1500, -1500, 0), 100.0f);   // SW residential

    // Connect nodes to form traffic network
    ConnectNodes(0, 1); // Center to East
    ConnectNodes(0, 2); // Center to West
    ConnectNodes(0, 3); // Center to North
    ConnectNodes(0, 4); // Center to South
    ConnectNodes(1, 5); // East to NE
    ConnectNodes(1, 7); // East to SE
    ConnectNodes(2, 6); // West to NW
    ConnectNodes(2, 8); // West to SW
    ConnectNodes(3, 5); // North to NE
    ConnectNodes(3, 6); // North to NW
    ConnectNodes(4, 7); // South to SE
    ConnectNodes(4, 8); // South to SW

    UE_LOG(LogTemp, Warning, TEXT("Traffic network initialized with %d nodes"), TrafficNodes.Num());
}

void ACrowd_TrafficFlowManager::UpdateFlowFields()
{
    // Update flow fields based on current traffic conditions
    for (auto& FlowFieldPair : FlowFieldGrid)
    {
        CalculateFlowField(FlowFieldPair.Key);
    }

    PropagateFlow();
}

FVector ACrowd_TrafficFlowManager::GetFlowDirection(const FVector& WorldLocation)
{
    FVector2D GridPos = WorldToGrid(WorldLocation);
    
    if (FCrowd_FlowField* FlowField = FlowFieldGrid.Find(GridPos))
    {
        return FlowField->FlowDirection * FlowField->FlowStrength;
    }

    // Default flow toward nearest traffic node
    float MinDistance = FLT_MAX;
    FVector BestDirection = FVector::ZeroVector;

    for (const FCrowd_TrafficNode& Node : TrafficNodes)
    {
        float Distance = FVector::Dist(WorldLocation, Node.Location);
        if (Distance < MinDistance && Node.CurrentDensity < Node.Capacity * CongestionThreshold)
        {
            MinDistance = Distance;
            BestDirection = (Node.Location - WorldLocation).GetSafeNormal();
        }
    }

    return BestDirection;
}

float ACrowd_TrafficFlowManager::GetCongestionLevel(const FVector& WorldLocation)
{
    FVector2D GridPos = WorldToGrid(WorldLocation);
    
    if (FCrowd_FlowField* FlowField = FlowFieldGrid.Find(GridPos))
    {
        return FlowField->Congestion;
    }

    return 0.0f;
}

void ACrowd_TrafficFlowManager::AddTrafficNode(const FVector& Location, float Capacity)
{
    FCrowd_TrafficNode NewNode;
    NewNode.Location = Location;
    NewNode.Capacity = Capacity;
    NewNode.CurrentDensity = 0.0f;
    
    TrafficNodes.Add(NewNode);

    // Create flow field around this node
    for (int32 X = -2; X <= 2; X++)
    {
        for (int32 Y = -2; Y <= 2; Y++)
        {
            FVector2D GridPos = WorldToGrid(Location) + FVector2D(X, Y);
            
            if (!FlowFieldGrid.Contains(GridPos))
            {
                FCrowd_FlowField NewFlowField;
                NewFlowField.FlowDirection = FVector::ForwardVector;
                NewFlowField.FlowStrength = 1.0f;
                NewFlowField.Congestion = 0.0f;
                
                FlowFieldGrid.Add(GridPos, NewFlowField);
            }
        }
    }
}

void ACrowd_TrafficFlowManager::ConnectNodes(int32 NodeA, int32 NodeB)
{
    if (TrafficNodes.IsValidIndex(NodeA) && TrafficNodes.IsValidIndex(NodeB))
    {
        TrafficNodes[NodeA].ConnectedNodes.AddUnique(NodeB);
        TrafficNodes[NodeB].ConnectedNodes.AddUnique(NodeA);
    }
}

void ACrowd_TrafficFlowManager::UpdateNodeDensity(int32 NodeIndex, float DensityChange)
{
    if (TrafficNodes.IsValidIndex(NodeIndex))
    {
        TrafficNodes[NodeIndex].CurrentDensity = FMath::Clamp(
            TrafficNodes[NodeIndex].CurrentDensity + DensityChange,
            0.0f,
            TrafficNodes[NodeIndex].Capacity
        );
    }
}

void ACrowd_TrafficFlowManager::CalculateFlowField(const FVector2D& GridPosition)
{
    if (FCrowd_FlowField* FlowField = FlowFieldGrid.Find(GridPosition))
    {
        FVector WorldPos = GridToWorld(GridPosition);
        
        // Find best flow direction based on traffic nodes
        FVector BestDirection = FVector::ZeroVector;
        float BestScore = -1.0f;

        for (int32 i = 0; i < TrafficNodes.Num(); i++)
        {
            const FCrowd_TrafficNode& Node = TrafficNodes[i];
            FVector Direction = (Node.Location - WorldPos).GetSafeNormal();
            float Distance = FVector::Dist(WorldPos, Node.Location);
            
            // Score based on capacity availability and distance
            float CapacityRatio = 1.0f - (Node.CurrentDensity / Node.Capacity);
            float DistanceScore = 1.0f / (1.0f + Distance / 1000.0f);
            float Score = CapacityRatio * DistanceScore;

            if (Score > BestScore)
            {
                BestScore = Score;
                BestDirection = Direction;
            }
        }

        FlowField->FlowDirection = BestDirection;
        FlowField->FlowStrength = FMath::Clamp(BestScore, 0.1f, 1.0f);
        
        // Calculate congestion based on nearby density
        float LocalCongestion = 0.0f;
        int32 NearbyNodes = 0;
        
        for (const FCrowd_TrafficNode& Node : TrafficNodes)
        {
            float Distance = FVector::Dist(WorldPos, Node.Location);
            if (Distance < GridCellSize * 3.0f)
            {
                LocalCongestion += Node.CurrentDensity / Node.Capacity;
                NearbyNodes++;
            }
        }
        
        FlowField->Congestion = NearbyNodes > 0 ? LocalCongestion / NearbyNodes : 0.0f;
    }
}

void ACrowd_TrafficFlowManager::PropagateFlow()
{
    // Smooth flow fields by averaging with neighbors
    TMap<FVector2D, FCrowd_FlowField> SmoothedFields = FlowFieldGrid;

    for (auto& FlowFieldPair : FlowFieldGrid)
    {
        FVector2D GridPos = FlowFieldPair.Key;
        FCrowd_FlowField& FlowField = FlowFieldPair.Value;
        
        FVector AverageDirection = FlowField.FlowDirection;
        float AverageStrength = FlowField.FlowStrength;
        int32 Neighbors = 1;

        // Check 8 neighboring cells
        for (int32 X = -1; X <= 1; X++)
        {
            for (int32 Y = -1; Y <= 1; Y++)
            {
                if (X == 0 && Y == 0) continue;
                
                FVector2D NeighborPos = GridPos + FVector2D(X, Y);
                if (FCrowd_FlowField* NeighborField = FlowFieldGrid.Find(NeighborPos))
                {
                    AverageDirection += NeighborField->FlowDirection;
                    AverageStrength += NeighborField->FlowStrength;
                    Neighbors++;
                }
            }
        }

        if (Neighbors > 1)
        {
            SmoothedFields[GridPos].FlowDirection = (AverageDirection / Neighbors).GetSafeNormal();
            SmoothedFields[GridPos].FlowStrength = AverageStrength / Neighbors;
        }
    }

    FlowFieldGrid = SmoothedFields;
}

FVector2D ACrowd_TrafficFlowManager::WorldToGrid(const FVector& WorldLocation)
{
    return FVector2D(
        FMath::FloorToInt(WorldLocation.X / GridCellSize),
        FMath::FloorToInt(WorldLocation.Y / GridCellSize)
    );
}

FVector ACrowd_TrafficFlowManager::GridToWorld(const FVector2D& GridPosition)
{
    return FVector(
        GridPosition.X * GridCellSize + GridCellSize * 0.5f,
        GridPosition.Y * GridCellSize + GridCellSize * 0.5f,
        0.0f
    );
}
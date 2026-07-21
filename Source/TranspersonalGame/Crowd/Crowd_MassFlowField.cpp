#include "Crowd_MassFlowField.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "MassEntitySubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"

UCrowd_MassFlowField::UCrowd_MassFlowField()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    DefaultCellSize = 100.0f;
    MaxFlowDistance = 5000.0f;
    ObstacleCost = 1000.0f;
    bAutoUpdateFlowField = true;
    UpdateFrequency = 2.0f;
    LastUpdateTime = 0.0f;
    bFlowFieldValid = false;
    CurrentTargetLocation = FVector::ZeroVector;
}

void UCrowd_MassFlowField::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default grid around the owner actor
    if (AActor* Owner = GetOwner())
    {
        FVector OwnerLocation = Owner->GetActorLocation();
        InitializeGrid(OwnerLocation - FVector(2500.0f, 2500.0f, 0.0f), 50, 50, DefaultCellSize);
        DetectStaticObstacles();
    }
}

void UCrowd_MassFlowField::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoUpdateFlowField && bFlowFieldValid)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastUpdateTime >= UpdateFrequency)
        {
            GenerateFlowField(CurrentTargetLocation, MaxFlowDistance);
            LastUpdateTime = CurrentTime;
        }
    }
}

void UCrowd_MassFlowField::GenerateFlowField(const FVector& TargetLocation, float MaxDistance)
{
    if (FlowGrid.Cells.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Flow field grid not initialized"));
        return;
    }
    
    CurrentTargetLocation = TargetLocation;
    MaxFlowDistance = MaxDistance;
    
    // Clear previous flow field
    for (FCrowd_FlowFieldCell& Cell : FlowGrid.Cells)
    {
        Cell.FlowDirection = FVector::ZeroVector;
        Cell.DistanceToTarget = FLT_MAX;
    }
    
    // Calculate distance field using Dijkstra-like algorithm
    CalculateDistanceField(TargetLocation);
    
    // Generate flow directions based on distance gradients
    CalculateFlowField();
    
    bFlowFieldValid = true;
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Flow field generated for target at %s"), *TargetLocation.ToString());
}

void UCrowd_MassFlowField::ClearFlowField()
{
    for (FCrowd_FlowFieldCell& Cell : FlowGrid.Cells)
    {
        Cell.FlowDirection = FVector::ZeroVector;
        Cell.DistanceToTarget = FLT_MAX;
    }
    
    bFlowFieldValid = false;
    UE_LOG(LogTemp, Log, TEXT("Flow field cleared"));
}

FVector UCrowd_MassFlowField::GetFlowDirection(const FVector& WorldPosition) const
{
    FIntPoint GridPos = WorldToGrid(WorldPosition);
    int32 Index = GridToIndex(GridPos);
    
    if (Index >= 0 && Index < FlowGrid.Cells.Num())
    {
        return FlowGrid.Cells[Index].FlowDirection;
    }
    
    return FVector::ZeroVector;
}

float UCrowd_MassFlowField::GetFlowCost(const FVector& WorldPosition) const
{
    FIntPoint GridPos = WorldToGrid(WorldPosition);
    int32 Index = GridToIndex(GridPos);
    
    if (Index >= 0 && Index < FlowGrid.Cells.Num())
    {
        return FlowGrid.Cells[Index].Cost;
    }
    
    return 1.0f;
}

bool UCrowd_MassFlowField::IsObstacle(const FVector& WorldPosition) const
{
    FIntPoint GridPos = WorldToGrid(WorldPosition);
    int32 Index = GridToIndex(GridPos);
    
    if (Index >= 0 && Index < FlowGrid.Cells.Num())
    {
        return FlowGrid.Cells[Index].bIsObstacle;
    }
    
    return false;
}

void UCrowd_MassFlowField::InitializeGrid(const FVector& Origin, int32 Width, int32 Height, float CellSize)
{
    FlowGrid.WorldOrigin = Origin;
    FlowGrid.GridWidth = Width;
    FlowGrid.GridHeight = Height;
    FlowGrid.CellSize = CellSize;
    
    int32 TotalCells = Width * Height;
    FlowGrid.Cells.Empty(TotalCells);
    FlowGrid.Cells.AddDefaulted(TotalCells);
    
    // Initialize all cells with default values
    for (int32 i = 0; i < TotalCells; i++)
    {
        FlowGrid.Cells[i].Cost = 1.0f;
        FlowGrid.Cells[i].bIsObstacle = false;
        FlowGrid.Cells[i].FlowDirection = FVector::ZeroVector;
        FlowGrid.Cells[i].DistanceToTarget = FLT_MAX;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Flow field grid initialized: %dx%d cells, cell size %.1f"), Width, Height, CellSize);
}

void UCrowd_MassFlowField::AddObstacle(const FVector& WorldPosition, float Radius)
{
    FIntPoint CenterGrid = WorldToGrid(WorldPosition);
    int32 CellRadius = FMath::CeilToInt(Radius / FlowGrid.CellSize);
    
    for (int32 x = CenterGrid.X - CellRadius; x <= CenterGrid.X + CellRadius; x++)
    {
        for (int32 y = CenterGrid.Y - CellRadius; y <= CenterGrid.Y + CellRadius; y++)
        {
            if (x >= 0 && x < FlowGrid.GridWidth && y >= 0 && y < FlowGrid.GridHeight)
            {
                FVector CellWorldPos = GridToWorld(FIntPoint(x, y));
                float Distance = FVector::Dist2D(CellWorldPos, WorldPosition);
                
                if (Distance <= Radius)
                {
                    int32 Index = GridToIndex(FIntPoint(x, y));
                    if (Index >= 0 && Index < FlowGrid.Cells.Num())
                    {
                        FlowGrid.Cells[Index].bIsObstacle = true;
                        FlowGrid.Cells[Index].Cost = ObstacleCost;
                    }
                }
            }
        }
    }
}

void UCrowd_MassFlowField::RemoveObstacle(const FVector& WorldPosition, float Radius)
{
    FIntPoint CenterGrid = WorldToGrid(WorldPosition);
    int32 CellRadius = FMath::CeilToInt(Radius / FlowGrid.CellSize);
    
    for (int32 x = CenterGrid.X - CellRadius; x <= CenterGrid.X + CellRadius; x++)
    {
        for (int32 y = CenterGrid.Y - CellRadius; y <= CenterGrid.Y + CellRadius; y++)
        {
            if (x >= 0 && x < FlowGrid.GridWidth && y >= 0 && y < FlowGrid.GridHeight)
            {
                FVector CellWorldPos = GridToWorld(FIntPoint(x, y));
                float Distance = FVector::Dist2D(CellWorldPos, WorldPosition);
                
                if (Distance <= Radius)
                {
                    int32 Index = GridToIndex(FIntPoint(x, y));
                    if (Index >= 0 && Index < FlowGrid.Cells.Num())
                    {
                        FlowGrid.Cells[Index].bIsObstacle = false;
                        FlowGrid.Cells[Index].Cost = 1.0f;
                    }
                }
            }
        }
    }
}

void UCrowd_MassFlowField::UpdateMassAgentFlowField(FMassEntityHandle EntityHandle, const FVector& AgentPosition)
{
    if (!bFlowFieldValid)
        return;
    
    UMassEntitySubsystem* MassEntitySubsystem = UWorld::GetSubsystem<UMassEntitySubsystem>(GetWorld());
    if (!MassEntitySubsystem)
        return;
    
    // Get flow direction for agent position
    FVector FlowDirection = GetFlowDirection(AgentPosition);
    
    if (!FlowDirection.IsZero())
    {
        // Apply flow field influence to mass entity velocity
        if (FMassVelocityFragment* VelocityFragment = MassEntitySubsystem->GetFragmentDataPtr<FMassVelocityFragment>(EntityHandle))
        {
            FVector CurrentVelocity = VelocityFragment->Value;
            FVector DesiredVelocity = FlowDirection * 300.0f; // Base speed
            
            // Blend current velocity with flow field direction
            FVector BlendedVelocity = FMath::VInterpTo(CurrentVelocity, DesiredVelocity, GetWorld()->GetDeltaSeconds(), 2.0f);
            VelocityFragment->Value = BlendedVelocity;
        }
    }
}

FVector UCrowd_MassFlowField::CalculateMassAgentDirection(const FVector& AgentPosition, const FVector& AgentVelocity) const
{
    if (!bFlowFieldValid)
        return AgentVelocity.GetSafeNormal();
    
    FVector FlowDirection = GetFlowDirection(AgentPosition);
    
    if (FlowDirection.IsZero())
        return AgentVelocity.GetSafeNormal();
    
    // Combine flow field with current velocity for smooth movement
    FVector CurrentDirection = AgentVelocity.GetSafeNormal();
    FVector BlendedDirection = FMath::VInterpTo(CurrentDirection, FlowDirection, GetWorld()->GetDeltaSeconds(), 3.0f);
    
    return BlendedDirection.GetSafeNormal();
}

FIntPoint UCrowd_MassFlowField::WorldToGrid(const FVector& WorldPosition) const
{
    FVector LocalPosition = WorldPosition - FlowGrid.WorldOrigin;
    int32 X = FMath::FloorToInt(LocalPosition.X / FlowGrid.CellSize);
    int32 Y = FMath::FloorToInt(LocalPosition.Y / FlowGrid.CellSize);
    
    return FIntPoint(FMath::Clamp(X, 0, FlowGrid.GridWidth - 1), FMath::Clamp(Y, 0, FlowGrid.GridHeight - 1));
}

FVector UCrowd_MassFlowField::GridToWorld(const FIntPoint& GridPosition) const
{
    FVector WorldPosition = FlowGrid.WorldOrigin;
    WorldPosition.X += (GridPosition.X + 0.5f) * FlowGrid.CellSize;
    WorldPosition.Y += (GridPosition.Y + 0.5f) * FlowGrid.CellSize;
    
    return WorldPosition;
}

int32 UCrowd_MassFlowField::GridToIndex(const FIntPoint& GridPosition) const
{
    if (GridPosition.X < 0 || GridPosition.X >= FlowGrid.GridWidth ||
        GridPosition.Y < 0 || GridPosition.Y >= FlowGrid.GridHeight)
    {
        return -1;
    }
    
    return GridPosition.Y * FlowGrid.GridWidth + GridPosition.X;
}

FIntPoint UCrowd_MassFlowField::IndexToGrid(int32 Index) const
{
    if (Index < 0 || Index >= FlowGrid.Cells.Num())
        return FIntPoint(-1, -1);
    
    int32 X = Index % FlowGrid.GridWidth;
    int32 Y = Index / FlowGrid.GridWidth;
    
    return FIntPoint(X, Y);
}

void UCrowd_MassFlowField::CalculateDistanceField(const FVector& TargetLocation)
{
    // Find target cell
    FIntPoint TargetGrid = WorldToGrid(TargetLocation);
    int32 TargetIndex = GridToIndex(TargetGrid);
    
    if (TargetIndex < 0 || TargetIndex >= FlowGrid.Cells.Num())
    {
        UE_LOG(LogTemp, Warning, TEXT("Target location outside flow field grid"));
        return;
    }
    
    // Initialize target cell
    FlowGrid.Cells[TargetIndex].DistanceToTarget = 0.0f;
    
    // Use simple flood-fill algorithm for distance propagation
    PropagateDistances(TargetGrid);
}

void UCrowd_MassFlowField::CalculateFlowField()
{
    // Calculate flow direction for each cell based on distance gradients
    for (int32 x = 0; x < FlowGrid.GridWidth; x++)
    {
        for (int32 y = 0; y < FlowGrid.GridHeight; y++)
        {
            int32 Index = GridToIndex(FIntPoint(x, y));
            if (Index < 0 || Index >= FlowGrid.Cells.Num())
                continue;
            
            FCrowd_FlowFieldCell& Cell = FlowGrid.Cells[Index];
            
            if (Cell.bIsObstacle)
                continue;
            
            // Find neighbor with lowest distance
            float MinDistance = Cell.DistanceToTarget;
            FVector BestDirection = FVector::ZeroVector;
            
            // Check 8 neighbors
            for (int32 dx = -1; dx <= 1; dx++)
            {
                for (int32 dy = -1; dy <= 1; dy++)
                {
                    if (dx == 0 && dy == 0)
                        continue;
                    
                    int32 NeighborX = x + dx;
                    int32 NeighborY = y + dy;
                    
                    if (NeighborX >= 0 && NeighborX < FlowGrid.GridWidth &&
                        NeighborY >= 0 && NeighborY < FlowGrid.GridHeight)
                    {
                        int32 NeighborIndex = GridToIndex(FIntPoint(NeighborX, NeighborY));
                        if (NeighborIndex >= 0 && NeighborIndex < FlowGrid.Cells.Num())
                        {
                            const FCrowd_FlowFieldCell& NeighborCell = FlowGrid.Cells[NeighborIndex];
                            
                            if (!NeighborCell.bIsObstacle && NeighborCell.DistanceToTarget < MinDistance)
                            {
                                MinDistance = NeighborCell.DistanceToTarget;
                                BestDirection = FVector(dx, dy, 0.0f).GetSafeNormal();
                            }
                        }
                    }
                }
            }
            
            Cell.FlowDirection = BestDirection;
        }
    }
}

void UCrowd_MassFlowField::PropagateDistances(const FIntPoint& StartCell)
{
    TArray<FIntPoint> Queue;
    Queue.Add(StartCell);
    
    while (Queue.Num() > 0)
    {
        FIntPoint CurrentCell = Queue[0];
        Queue.RemoveAt(0);
        
        int32 CurrentIndex = GridToIndex(CurrentCell);
        if (CurrentIndex < 0 || CurrentIndex >= FlowGrid.Cells.Num())
            continue;
        
        float CurrentDistance = FlowGrid.Cells[CurrentIndex].DistanceToTarget;
        
        // Check 4-connected neighbors
        TArray<FIntPoint> Neighbors = {
            FIntPoint(CurrentCell.X + 1, CurrentCell.Y),
            FIntPoint(CurrentCell.X - 1, CurrentCell.Y),
            FIntPoint(CurrentCell.X, CurrentCell.Y + 1),
            FIntPoint(CurrentCell.X, CurrentCell.Y - 1)
        };
        
        for (const FIntPoint& Neighbor : Neighbors)
        {
            if (Neighbor.X >= 0 && Neighbor.X < FlowGrid.GridWidth &&
                Neighbor.Y >= 0 && Neighbor.Y < FlowGrid.GridHeight)
            {
                int32 NeighborIndex = GridToIndex(Neighbor);
                if (NeighborIndex >= 0 && NeighborIndex < FlowGrid.Cells.Num())
                {
                    FCrowd_FlowFieldCell& NeighborCell = FlowGrid.Cells[NeighborIndex];
                    
                    if (!NeighborCell.bIsObstacle)
                    {
                        float NewDistance = CurrentDistance + NeighborCell.Cost;
                        
                        if (NewDistance < NeighborCell.DistanceToTarget)
                        {
                            NeighborCell.DistanceToTarget = NewDistance;
                            Queue.Add(Neighbor);
                        }
                    }
                }
            }
        }
    }
}

void UCrowd_MassFlowField::DetectStaticObstacles()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Detect static obstacles using collision queries
    for (int32 x = 0; x < FlowGrid.GridWidth; x++)
    {
        for (int32 y = 0; y < FlowGrid.GridHeight; y++)
        {
            FVector CellWorldPos = GridToWorld(FIntPoint(x, y));
            
            if (IsWorldPositionBlocked(CellWorldPos))
            {
                int32 Index = GridToIndex(FIntPoint(x, y));
                if (Index >= 0 && Index < FlowGrid.Cells.Num())
                {
                    FlowGrid.Cells[Index].bIsObstacle = true;
                    FlowGrid.Cells[Index].Cost = ObstacleCost;
                }
            }
        }
    }
}

bool UCrowd_MassFlowField::IsWorldPositionBlocked(const FVector& WorldPosition) const
{
    UWorld* World = GetWorld();
    if (!World)
        return false;
    
    FVector Start = WorldPosition + FVector(0.0f, 0.0f, 100.0f);
    FVector End = WorldPosition - FVector(0.0f, 0.0f, 100.0f);
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    // Check if there's solid geometry blocking this position
    bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
    
    return !bHit; // If no hit, position is blocked (no ground)
}
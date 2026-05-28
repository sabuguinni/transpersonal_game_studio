#include "Crowd_FlockingBehavior.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_FlockingBehavior::UCrowd_FlockingBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Initialize LOD distances
    LODDistances.Add(1000.0f);  // LOD 0
    LODDistances.Add(2500.0f);  // LOD 1
    LODDistances.Add(5000.0f);  // LOD 2
    LODDistances.Add(10000.0f); // LOD 3
}

void UCrowd_FlockingBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize flocking system with default parameters
    InitializeFlockingSystem(5000);
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_FlockingBehavior: System initialized with %d max agents"), FlockingAgents.Max());
}

void UCrowd_FlockingBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update flocking behavior
    UpdateFlockingBehavior(DeltaTime);
    
    // Update spatial grid periodically
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime > (1.0f / UpdateFrequency))
    {
        UpdateSpatialGrid();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
    
    // Draw debug visualization
    if (bDrawDebug)
    {
        DrawFlockingDebug(true);
    }
}

void UCrowd_FlockingBehavior::InitializeFlockingSystem(int32 MaxAgents)
{
    FlockingAgents.Empty();
    FlockingAgents.Reserve(MaxAgents);
    
    SpatialGrid.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_FlockingBehavior: Flocking system initialized for %d agents"), MaxAgents);
}

void UCrowd_FlockingBehavior::AddFlockingAgent(const FVector& Position, const FVector& InitialVelocity, int32 FlockID)
{
    FCrowd_FlockingAgent NewAgent;
    NewAgent.Position = Position;
    NewAgent.Velocity = InitialVelocity;
    NewAgent.Acceleration = FVector::ZeroVector;
    NewAgent.Mass = 1.0f;
    NewAgent.bIsActive = true;
    NewAgent.FlockID = FlockID;
    
    FlockingAgents.Add(NewAgent);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_FlockingBehavior: Added agent at position %s"), *Position.ToString());
}

void UCrowd_FlockingBehavior::RemoveFlockingAgent(int32 AgentIndex)
{
    if (IsValidAgentIndex(AgentIndex))
    {
        FlockingAgents[AgentIndex].bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Crowd_FlockingBehavior: Removed agent %d"), AgentIndex);
    }
}

void UCrowd_FlockingBehavior::UpdateFlockingBehavior(float DeltaTime)
{
    if (FlockingAgents.Num() == 0) return;
    
    // Performance mode: update only a subset of agents per frame
    int32 AgentsToUpdate = bUsePerformanceMode ? 
        FMath::Min(MaxAgentsPerFrame, FlockingAgents.Num()) : 
        FlockingAgents.Num();
    
    for (int32 i = 0; i < AgentsToUpdate; ++i)
    {
        int32 AgentIndex = (CurrentUpdateIndex + i) % FlockingAgents.Num();
        
        if (!FlockingAgents[AgentIndex].bIsActive) continue;
        
        // Get neighbors for this agent
        TArray<int32> Neighbors = GetNeighbors(AgentIndex, FlockingParams.CohesionRadius);
        
        // Calculate flocking forces
        FVector Separation = CalculateSeparation(AgentIndex, Neighbors);
        FVector Alignment = CalculateAlignment(AgentIndex, Neighbors);
        FVector Cohesion = CalculateCohesion(AgentIndex, Neighbors);
        FVector Avoidance = CalculateObstacleAvoidance(AgentIndex);
        
        // Apply weights
        Separation *= FlockingParams.SeparationWeight;
        Alignment *= FlockingParams.AlignmentWeight;
        Cohesion *= FlockingParams.CohesionWeight;
        Avoidance *= 3.0f; // Higher weight for obstacle avoidance
        
        // Combine forces
        FVector TotalForce = Separation + Alignment + Cohesion + Avoidance;
        TotalForce = LimitVector(TotalForce, FlockingParams.MaxForce);
        
        // Apply force to acceleration
        FlockingAgents[AgentIndex].Acceleration = TotalForce / FlockingAgents[AgentIndex].Mass;
        
        // Update agent position
        UpdateAgentPosition(AgentIndex, DeltaTime);
    }
    
    // Update the starting index for next frame
    CurrentUpdateIndex = (CurrentUpdateIndex + AgentsToUpdate) % FlockingAgents.Num();
}

FVector UCrowd_FlockingBehavior::CalculateSeparation(int32 AgentIndex, const TArray<int32>& Neighbors)
{
    if (!IsValidAgentIndex(AgentIndex)) return FVector::ZeroVector;
    
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    for (int32 NeighborIndex : Neighbors)
    {
        if (NeighborIndex == AgentIndex || !IsValidAgentIndex(NeighborIndex)) continue;
        
        const FCrowd_FlockingAgent& Neighbor = FlockingAgents[NeighborIndex];
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingParams.SeparationRadius)
        {
            FVector Diff = Agent.Position - Neighbor.Position;
            Diff.Normalize();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce.Normalize();
        SeparationForce *= FlockingParams.MaxSpeed;
        SeparationForce -= Agent.Velocity;
        SeparationForce = LimitVector(SeparationForce, FlockingParams.MaxForce);
    }
    
    return SeparationForce;
}

FVector UCrowd_FlockingBehavior::CalculateAlignment(int32 AgentIndex, const TArray<int32>& Neighbors)
{
    if (!IsValidAgentIndex(AgentIndex)) return FVector::ZeroVector;
    
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    for (int32 NeighborIndex : Neighbors)
    {
        if (NeighborIndex == AgentIndex || !IsValidAgentIndex(NeighborIndex)) continue;
        
        const FCrowd_FlockingAgent& Neighbor = FlockingAgents[NeighborIndex];
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingParams.AlignmentRadius)
        {
            AverageVelocity += Neighbor.Velocity;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        AverageVelocity /= Count;
        AverageVelocity.Normalize();
        AverageVelocity *= FlockingParams.MaxSpeed;
        
        FVector AlignmentForce = AverageVelocity - Agent.Velocity;
        AlignmentForce = LimitVector(AlignmentForce, FlockingParams.MaxForce);
        return AlignmentForce;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_FlockingBehavior::CalculateCohesion(int32 AgentIndex, const TArray<int32>& Neighbors)
{
    if (!IsValidAgentIndex(AgentIndex)) return FVector::ZeroVector;
    
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    for (int32 NeighborIndex : Neighbors)
    {
        if (NeighborIndex == AgentIndex || !IsValidAgentIndex(NeighborIndex)) continue;
        
        const FCrowd_FlockingAgent& Neighbor = FlockingAgents[NeighborIndex];
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingParams.CohesionRadius)
        {
            CenterOfMass += Neighbor.Position;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        
        FVector DesiredVelocity = CenterOfMass - Agent.Position;
        DesiredVelocity.Normalize();
        DesiredVelocity *= FlockingParams.MaxSpeed;
        
        FVector CohesionForce = DesiredVelocity - Agent.Velocity;
        CohesionForce = LimitVector(CohesionForce, FlockingParams.MaxForce);
        return CohesionForce;
    }
    
    return FVector::ZeroVector;
}

TArray<int32> UCrowd_FlockingBehavior::GetNeighbors(int32 AgentIndex, float Radius)
{
    TArray<int32> Neighbors;
    
    if (!IsValidAgentIndex(AgentIndex)) return Neighbors;
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    // Use spatial grid for optimization
    int32 GridIndex = GetGridIndex(Agent.Position);
    
    if (SpatialGrid.Contains(GridIndex))
    {
        for (int32 CandidateIndex : SpatialGrid[GridIndex])
        {
            if (CandidateIndex != AgentIndex && IsValidAgentIndex(CandidateIndex))
            {
                float Distance = FVector::Dist(Agent.Position, FlockingAgents[CandidateIndex].Position);
                if (Distance <= Radius)
                {
                    Neighbors.Add(CandidateIndex);
                }
            }
        }
    }
    
    return Neighbors;
}

void UCrowd_FlockingBehavior::UpdateSpatialGrid()
{
    SpatialGrid.Empty();
    
    for (int32 i = 0; i < FlockingAgents.Num(); ++i)
    {
        if (FlockingAgents[i].bIsActive)
        {
            int32 GridIndex = GetGridIndex(FlockingAgents[i].Position);
            if (!SpatialGrid.Contains(GridIndex))
            {
                SpatialGrid.Add(GridIndex, TArray<int32>());
            }
            SpatialGrid[GridIndex].Add(i);
        }
    }
}

FVector UCrowd_FlockingBehavior::CalculateObstacleAvoidance(int32 AgentIndex)
{
    if (!IsValidAgentIndex(AgentIndex)) return FVector::ZeroVector;
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    FVector AvoidanceForce = FVector::ZeroVector;
    
    for (int32 i = 0; i < ObstaclePositions.Num(); ++i)
    {
        float Distance = FVector::Dist(Agent.Position, ObstaclePositions[i]);
        float InfluenceRadius = ObstacleRadii[i] + 200.0f; // Safety margin
        
        if (Distance < InfluenceRadius)
        {
            FVector AvoidDirection = Agent.Position - ObstaclePositions[i];
            AvoidDirection.Normalize();
            
            float Strength = (InfluenceRadius - Distance) / InfluenceRadius;
            AvoidanceForce += AvoidDirection * Strength * FlockingParams.MaxForce;
        }
    }
    
    return LimitVector(AvoidanceForce, FlockingParams.MaxForce);
}

void UCrowd_FlockingBehavior::AddObstacle(const FVector& Position, float Radius)
{
    ObstaclePositions.Add(Position);
    ObstacleRadii.Add(Radius);
    
    UE_LOG(LogTemp, Log, TEXT("Crowd_FlockingBehavior: Added obstacle at %s with radius %f"), *Position.ToString(), Radius);
}

void UCrowd_FlockingBehavior::SetLODLevel(int32 Level)
{
    CurrentLODLevel = FMath::Clamp(Level, 0, LODDistances.Num() - 1);
    
    // Adjust update frequency based on LOD
    switch (CurrentLODLevel)
    {
        case 0: UpdateFrequency = 60.0f; MaxAgentsPerFrame = 1000; break;
        case 1: UpdateFrequency = 30.0f; MaxAgentsPerFrame = 500; break;
        case 2: UpdateFrequency = 15.0f; MaxAgentsPerFrame = 250; break;
        case 3: UpdateFrequency = 10.0f; MaxAgentsPerFrame = 100; break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd_FlockingBehavior: LOD level set to %d"), CurrentLODLevel);
}

void UCrowd_FlockingBehavior::EnablePerformanceMode(bool bEnabled)
{
    bUsePerformanceMode = bEnabled;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_FlockingBehavior: Performance mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCrowd_FlockingBehavior::DrawFlockingDebug(bool bEnabled)
{
    if (!bEnabled || !GetWorld()) return;
    
    for (int32 i = 0; i < FlockingAgents.Num(); ++i)
    {
        if (!FlockingAgents[i].bIsActive) continue;
        
        const FCrowd_FlockingAgent& Agent = FlockingAgents[i];
        
        // Draw agent position
        DrawDebugSphere(GetWorld(), Agent.Position, 10.0f, 8, FColor::Blue, false, 0.1f);
        
        // Draw velocity vector
        if (bShowVelocityVectors)
        {
            FVector VelEnd = Agent.Position + (Agent.Velocity * 0.5f);
            DrawDebugDirectionalArrow(GetWorld(), Agent.Position, VelEnd, 20.0f, FColor::Green, false, 0.1f);
        }
        
        // Draw neighbor connections
        if (bShowNeighborConnections)
        {
            TArray<int32> Neighbors = GetNeighbors(i, FlockingParams.CohesionRadius);
            for (int32 NeighborIndex : Neighbors)
            {
                if (IsValidAgentIndex(NeighborIndex))
                {
                    DrawDebugLine(GetWorld(), Agent.Position, FlockingAgents[NeighborIndex].Position, FColor::Yellow, false, 0.1f);
                }
            }
        }
    }
    
    // Draw obstacles
    for (int32 i = 0; i < ObstaclePositions.Num(); ++i)
    {
        DrawDebugSphere(GetWorld(), ObstaclePositions[i], ObstacleRadii[i], 16, FColor::Red, false, 0.1f);
    }
}

int32 UCrowd_FlockingBehavior::GetActiveAgentCount() const
{
    int32 Count = 0;
    for (const FCrowd_FlockingAgent& Agent : FlockingAgents)
    {
        if (Agent.bIsActive) Count++;
    }
    return Count;
}

float UCrowd_FlockingBehavior::GetAverageSpeed() const
{
    float TotalSpeed = 0.0f;
    int32 Count = 0;
    
    for (const FCrowd_FlockingAgent& Agent : FlockingAgents)
    {
        if (Agent.bIsActive)
        {
            TotalSpeed += Agent.Velocity.Size();
            Count++;
        }
    }
    
    return Count > 0 ? TotalSpeed / Count : 0.0f;
}

FVector UCrowd_FlockingBehavior::LimitVector(const FVector& Vector, float MaxMagnitude)
{
    if (Vector.SizeSquared() > MaxMagnitude * MaxMagnitude)
    {
        FVector Limited = Vector;
        Limited.Normalize();
        return Limited * MaxMagnitude;
    }
    return Vector;
}

int32 UCrowd_FlockingBehavior::GetGridIndex(const FVector& Position)
{
    int32 X = FMath::FloorToInt(Position.X / GridCellSize) % GridWidth;
    int32 Y = FMath::FloorToInt(Position.Y / GridCellSize) % GridHeight;
    
    // Ensure positive indices
    X = (X + GridWidth) % GridWidth;
    Y = (Y + GridHeight) % GridHeight;
    
    return Y * GridWidth + X;
}

void UCrowd_FlockingBehavior::UpdateAgentPosition(int32 AgentIndex, float DeltaTime)
{
    if (!IsValidAgentIndex(AgentIndex)) return;
    
    FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    // Update velocity
    Agent.Velocity += Agent.Acceleration * DeltaTime;
    Agent.Velocity = LimitVector(Agent.Velocity, FlockingParams.MaxSpeed);
    
    // Update position
    Agent.Position += Agent.Velocity * DeltaTime;
    
    // Reset acceleration
    Agent.Acceleration = FVector::ZeroVector;
}

bool UCrowd_FlockingBehavior::IsValidAgentIndex(int32 Index) const
{
    return Index >= 0 && Index < FlockingAgents.Num();
}
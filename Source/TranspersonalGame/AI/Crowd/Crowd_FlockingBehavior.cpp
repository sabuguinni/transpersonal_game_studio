#include "Crowd_FlockingBehavior.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UCrowd_FlockingBehavior::UCrowd_FlockingBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize default flocking rules
    FlockingRules.SeparationWeight = 1.5f;
    FlockingRules.AlignmentWeight = 1.0f;
    FlockingRules.CohesionWeight = 1.0f;
    FlockingRules.SeparationRadius = 100.0f;
    FlockingRules.AlignmentRadius = 200.0f;
    FlockingRules.CohesionRadius = 300.0f;
    FlockingRules.MaxSpeed = 400.0f;
    FlockingRules.MaxForce = 200.0f;
    
    UpdateFrequency = 30.0f;
    bFlockingEnabled = true;
}

void UCrowd_FlockingBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    LastUpdateTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Flocking Behavior component initialized with %d agents"), FlockingAgents.Num());
}

void UCrowd_FlockingBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bFlockingEnabled || FlockingAgents.Num() == 0)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    // Update at specified frequency to maintain performance
    if (LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateFlockingBehavior(LastUpdateTime);
        LastUpdateTime = 0.0f;
    }
}

void UCrowd_FlockingBehavior::InitializeFlocking(const FCrowd_FlockingRules& Rules)
{
    FlockingRules = Rules;
    
    UE_LOG(LogTemp, Warning, TEXT("Flocking initialized - Sep:%.1f Align:%.1f Coh:%.1f MaxSpeed:%.1f"), 
           Rules.SeparationWeight, Rules.AlignmentWeight, Rules.CohesionWeight, Rules.MaxSpeed);
}

void UCrowd_FlockingBehavior::AddFlockingAgent(const FCrowd_FlockingAgent& Agent)
{
    FlockingAgents.Add(Agent);
    
    UE_LOG(LogTemp, Log, TEXT("Added flocking agent at %s. Total agents: %d"), 
           *Agent.Position.ToString(), FlockingAgents.Num());
}

void UCrowd_FlockingBehavior::RemoveFlockingAgent(int32 AgentIndex)
{
    if (FlockingAgents.IsValidIndex(AgentIndex))
    {
        FlockingAgents.RemoveAt(AgentIndex);
        UE_LOG(LogTemp, Log, TEXT("Removed flocking agent at index %d. Remaining: %d"), 
               AgentIndex, FlockingAgents.Num());
    }
}

void UCrowd_FlockingBehavior::UpdateFlockingRules(const FCrowd_FlockingRules& NewRules)
{
    FlockingRules = NewRules;
    UE_LOG(LogTemp, Log, TEXT("Flocking rules updated"));
}

FVector UCrowd_FlockingBehavior::CalculateSeparation(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors)
{
    if (!FlockingAgents.IsValidIndex(AgentIndex) || Neighbors.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_FlockingAgent& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingRules.SeparationRadius)
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
        SeparationForce *= FlockingRules.MaxSpeed;
        SeparationForce -= Agent.Velocity;
        SeparationForce = LimitVector(SeparationForce, FlockingRules.MaxForce);
    }
    
    return SeparationForce * FlockingRules.SeparationWeight;
}

FVector UCrowd_FlockingBehavior::CalculateAlignment(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors)
{
    if (!FlockingAgents.IsValidIndex(AgentIndex) || Neighbors.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    FVector AverageVelocity = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_FlockingAgent& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingRules.AlignmentRadius)
        {
            AverageVelocity += Neighbor.Velocity;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        AverageVelocity /= Count;
        AverageVelocity.Normalize();
        AverageVelocity *= FlockingRules.MaxSpeed;
        
        FVector AlignmentForce = AverageVelocity - Agent.Velocity;
        AlignmentForce = LimitVector(AlignmentForce, FlockingRules.MaxForce);
        
        return AlignmentForce * FlockingRules.AlignmentWeight;
    }
    
    return FVector::ZeroVector;
}

FVector UCrowd_FlockingBehavior::CalculateCohesion(int32 AgentIndex, const TArray<FCrowd_FlockingAgent>& Neighbors)
{
    if (!FlockingAgents.IsValidIndex(AgentIndex) || Neighbors.Num() == 0)
    {
        return FVector::ZeroVector;
    }
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    for (const FCrowd_FlockingAgent& Neighbor : Neighbors)
    {
        float Distance = FVector::Dist(Agent.Position, Neighbor.Position);
        
        if (Distance > 0.0f && Distance < FlockingRules.CohesionRadius)
        {
            CenterOfMass += Neighbor.Position;
            Count++;
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        
        FVector CohesionForce = CenterOfMass - Agent.Position;
        CohesionForce.Normalize();
        CohesionForce *= FlockingRules.MaxSpeed;
        CohesionForce -= Agent.Velocity;
        CohesionForce = LimitVector(CohesionForce, FlockingRules.MaxForce);
        
        return CohesionForce * FlockingRules.CohesionWeight;
    }
    
    return FVector::ZeroVector;
}

TArray<FCrowd_FlockingAgent> UCrowd_FlockingBehavior::FindNeighbors(int32 AgentIndex, float SearchRadius)
{
    TArray<FCrowd_FlockingAgent> Neighbors;
    
    if (!FlockingAgents.IsValidIndex(AgentIndex))
    {
        return Neighbors;
    }
    
    const FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    for (int32 i = 0; i < FlockingAgents.Num(); i++)
    {
        if (i != AgentIndex && FlockingAgents[i].bIsActive)
        {
            float Distance = FVector::Dist(Agent.Position, FlockingAgents[i].Position);
            if (Distance <= SearchRadius)
            {
                Neighbors.Add(FlockingAgents[i]);
            }
        }
    }
    
    return Neighbors;
}

void UCrowd_FlockingBehavior::UpdateFlockingBehavior(float DeltaTime)
{
    // Update each flocking agent
    for (int32 i = 0; i < FlockingAgents.Num(); i++)
    {
        if (FlockingAgents[i].bIsActive)
        {
            ApplyFlockingForces(i, DeltaTime);
        }
    }
}

FVector UCrowd_FlockingBehavior::LimitVector(const FVector& Vector, float MaxMagnitude)
{
    if (Vector.Size() > MaxMagnitude)
    {
        FVector LimitedVector = Vector;
        LimitedVector.Normalize();
        return LimitedVector * MaxMagnitude;
    }
    
    return Vector;
}

void UCrowd_FlockingBehavior::ApplyFlockingForces(int32 AgentIndex, float DeltaTime)
{
    if (!FlockingAgents.IsValidIndex(AgentIndex))
    {
        return;
    }
    
    FCrowd_FlockingAgent& Agent = FlockingAgents[AgentIndex];
    
    // Find neighbors for this agent
    TArray<FCrowd_FlockingAgent> Neighbors = FindNeighbors(AgentIndex, FlockingRules.CohesionRadius);
    
    // Calculate flocking forces
    FVector Separation = CalculateSeparation(AgentIndex, Neighbors);
    FVector Alignment = CalculateAlignment(AgentIndex, Neighbors);
    FVector Cohesion = CalculateCohesion(AgentIndex, Neighbors);
    
    // Apply forces to acceleration
    Agent.Acceleration = Separation + Alignment + Cohesion;
    
    // Update velocity and position
    Agent.Velocity += Agent.Acceleration * DeltaTime;
    Agent.Velocity = LimitVector(Agent.Velocity, FlockingRules.MaxSpeed);
    
    Agent.Position += Agent.Velocity * DeltaTime;
    
    // Reset acceleration for next frame
    Agent.Acceleration = FVector::ZeroVector;
}
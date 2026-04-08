#include "CrowdManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Combat/CombatSystem.h"

ACrowdManager::ACrowdManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    MaxCrowdSize = 100;
    SpawnRadius = 2000.0f;
    DespawnRadius = 3000.0f;
    
    SeparationRadius = 100.0f;
    AlignmentRadius = 200.0f;
    CohesionRadius = 300.0f;
    AvoidanceRadius = 150.0f;
    
    ConsciousnessInfluenceRadius = 500.0f;
    StressDecayRate = 0.5f;
    
    GridCellSize = 500.0f;
    UpdateBatchSize = 10;
    CurrentUpdateIndex = 0;
    
    // Create instanced mesh component
    CrowdMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CrowdMeshComponent"));
    RootComponent = CrowdMeshComponent;
}

void ACrowdManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize spatial grid
    UpdateSpatialGrid();
    
    // Spawn initial crowd
    for (int32 i = 0; i < MaxCrowdSize / 2; ++i)
    {
        FVector SpawnLocation = GetActorLocation() + FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        SpawnCrowdAgent(SpawnLocation);
    }
}

void ACrowdManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (CrowdAgents.Num() == 0) return;
    
    // Update spatial grid periodically
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= 0.5f)
    {
        UpdateSpatialGrid();
        LastUpdateTime = 0.0f;
    }
    
    // Update crowd behavior in batches for performance
    UpdateCrowdBehavior(DeltaTime);
    UpdateCrowdPositions(DeltaTime);
    UpdateInstancedMeshes();
}

void ACrowdManager::SpawnCrowdAgent(FVector Location)
{
    if (CrowdAgents.Num() >= MaxCrowdSize) return;
    
    FCrowdAgent NewAgent;
    NewAgent.Position = Location;
    NewAgent.Destination = Location + FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );
    NewAgent.Speed = FMath::RandRange(100.0f, 200.0f);
    NewAgent.ConsciousnessLevel = FMath::RandRange(0.2f, 0.8f);
    
    CrowdAgents.Add(NewAgent);
    
    // Add instance to mesh component
    FTransform InstanceTransform;
    InstanceTransform.SetLocation(Location);
    CrowdMeshComponent->AddInstance(InstanceTransform);
}

void ACrowdManager::RemoveCrowdAgent(int32 AgentIndex)
{
    if (AgentIndex >= 0 && AgentIndex < CrowdAgents.Num())
    {
        CrowdAgents.RemoveAt(AgentIndex);
        CrowdMeshComponent->RemoveInstance(AgentIndex);
    }
}

void ACrowdManager::SetCrowdDestination(FVector NewDestination)
{
    for (FCrowdAgent& Agent : CrowdAgents)
    {
        Agent.Destination = NewDestination + FVector(
            FMath::RandRange(-200.0f, 200.0f),
            FMath::RandRange(-200.0f, 200.0f),
            0.0f
        );
    }
}

void ACrowdManager::TriggerCombatResponse(FVector CombatLocation, float Radius)
{
    for (FCrowdAgent& Agent : CrowdAgents)
    {
        float Distance = FVector::Dist(Agent.Position, CombatLocation);
        if (Distance <= Radius)
        {
            Agent.bIsInCombat = true;
            Agent.StressLevel = FMath::Min(1.0f, Agent.StressLevel + 0.5f);
            
            // Flee from combat location
            FVector FleeDirection = (Agent.Position - CombatLocation).GetSafeNormal();
            Agent.Destination = Agent.Position + FleeDirection * 1000.0f;
            Agent.Speed *= 1.5f; // Panic speed boost
        }
    }
}

void ACrowdManager::UpdateConsciousnessInfluence(FVector Location, float Influence, float Radius)
{
    for (FCrowdAgent& Agent : CrowdAgents)
    {
        float Distance = FVector::Dist(Agent.Position, Location);
        if (Distance <= Radius)
        {
            float InfluenceStrength = 1.0f - (Distance / Radius);
            Agent.ConsciousnessLevel = FMath::Clamp(
                Agent.ConsciousnessLevel + Influence * InfluenceStrength * 0.1f,
                0.0f, 1.0f
            );
        }
    }
}

void ACrowdManager::UpdateCrowdBehavior(float DeltaTime)
{
    int32 BatchEnd = FMath::Min(CurrentUpdateIndex + UpdateBatchSize, CrowdAgents.Num());
    
    for (int32 i = CurrentUpdateIndex; i < BatchEnd; ++i)
    {
        FCrowdAgent& Agent = CrowdAgents[i];
        
        // Decay stress over time
        Agent.StressLevel = FMath::Max(0.0f, Agent.StressLevel - StressDecayRate * DeltaTime);
        
        if (Agent.bIsInCombat && Agent.StressLevel < 0.2f)
        {
            Agent.bIsInCombat = false;
            Agent.Speed /= 1.5f; // Return to normal speed
        }
        
        if (Agent.bIsInCombat)
        {
            HandleCombatBehavior(Agent, DeltaTime);
        }
        else
        {
            HandleNormalBehavior(Agent, DeltaTime);
        }
    }
    
    CurrentUpdateIndex = BatchEnd;
    if (CurrentUpdateIndex >= CrowdAgents.Num())
    {
        CurrentUpdateIndex = 0;
    }
}

void ACrowdManager::HandleCombatBehavior(FCrowdAgent& Agent, float DeltaTime)
{
    // In combat: prioritize fleeing and avoiding others
    FVector FleeForce = CalculateSeek(Agent, Agent.Destination) * 2.0f;
    FVector SeparationForce = CalculateSeparation(Agent, CrowdAgents.IndexOfByKey(Agent)) * 3.0f;
    FVector AvoidanceForce = CalculateAvoidance(Agent) * 4.0f;
    
    FVector TotalForce = FleeForce + SeparationForce + AvoidanceForce;
    Agent.Velocity = (Agent.Velocity + TotalForce * DeltaTime).GetClampedToMaxSize(Agent.Speed);
}

void ACrowdManager::HandleNormalBehavior(FCrowdAgent& Agent, float DeltaTime)
{
    // Normal behavior: flocking with consciousness influence
    FVector SeekForce = CalculateSeek(Agent, Agent.Destination);
    FVector SeparationForce = CalculateSeparation(Agent, CrowdAgents.IndexOfByKey(Agent)) * 1.5f;
    FVector AlignmentForce = CalculateAlignment(Agent, CrowdAgents.IndexOfByKey(Agent));
    FVector CohesionForce = CalculateCohesion(Agent, CrowdAgents.IndexOfByKey(Agent)) * 0.5f;
    FVector AvoidanceForce = CalculateAvoidance(Agent) * 2.0f;
    
    // Consciousness affects behavior weights
    float ConsciousnessWeight = Agent.ConsciousnessLevel;
    AlignmentForce *= ConsciousnessWeight;
    CohesionForce *= ConsciousnessWeight;
    
    FVector TotalForce = SeekForce + SeparationForce + AlignmentForce + CohesionForce + AvoidanceForce;
    Agent.Velocity = (Agent.Velocity + TotalForce * DeltaTime).GetClampedToMaxSize(Agent.Speed);
    
    // Update destination occasionally
    if (FVector::Dist(Agent.Position, Agent.Destination) < 100.0f)
    {
        Agent.Destination = GetActorLocation() + FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
    }
}

void ACrowdManager::UpdateCrowdPositions(float DeltaTime)
{
    for (int32 i = CrowdAgents.Num() - 1; i >= 0; --i)
    {
        FCrowdAgent& Agent = CrowdAgents[i];
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Remove agents that are too far away
        float DistanceFromCenter = FVector::Dist(Agent.Position, GetActorLocation());
        if (DistanceFromCenter > DespawnRadius)
        {
            RemoveCrowdAgent(i);
            continue;
        }
        
        // Spawn new agents if needed
        if (CrowdAgents.Num() < MaxCrowdSize && FMath::RandRange(0.0f, 1.0f) < 0.01f)
        {
            FVector SpawnLocation = GetActorLocation() + FVector(
                FMath::RandRange(-SpawnRadius, SpawnRadius),
                FMath::RandRange(-SpawnRadius, SpawnRadius),
                0.0f
            );
            SpawnCrowdAgent(SpawnLocation);
        }
    }
}

void ACrowdManager::UpdateInstancedMeshes()
{
    CrowdMeshComponent->ClearInstances();
    
    for (const FCrowdAgent& Agent : CrowdAgents)
    {
        FTransform InstanceTransform;
        InstanceTransform.SetLocation(Agent.Position);
        
        // Rotate based on velocity
        if (!Agent.Velocity.IsZero())
        {
            FRotator Rotation = Agent.Velocity.Rotation();
            InstanceTransform.SetRotation(Rotation.Quaternion());
        }
        
        // Scale based on consciousness level
        float Scale = 0.8f + (Agent.ConsciousnessLevel * 0.4f);
        InstanceTransform.SetScale3D(FVector(Scale));
        
        CrowdMeshComponent->AddInstance(InstanceTransform);
    }
}

FVector ACrowdManager::CalculateSeparation(const FCrowdAgent& Agent, int32 AgentIndex)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 Count = 0;
    
    TArray<int32> NearbyAgents = GetNearbyAgents(Agent.Position, SeparationRadius);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex && OtherIndex < CrowdAgents.Num())
        {
            const FCrowdAgent& Other = CrowdAgents[OtherIndex];
            float Distance = FVector::Dist(Agent.Position, Other.Position);
            
            if (Distance > 0.0f && Distance < SeparationRadius)
            {
                FVector Diff = (Agent.Position - Other.Position).GetSafeNormal();
                Diff /= Distance; // Weight by distance
                SeparationForce += Diff;
                Count++;
            }
        }
    }
    
    if (Count > 0)
    {
        SeparationForce /= Count;
        SeparationForce = SeparationForce.GetSafeNormal() * Agent.Speed;
        SeparationForce -= Agent.Velocity;
    }
    
    return SeparationForce;
}

FVector ACrowdManager::CalculateAlignment(const FCrowdAgent& Agent, int32 AgentIndex)
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 Count = 0;
    
    TArray<int32> NearbyAgents = GetNearbyAgents(Agent.Position, AlignmentRadius);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex && OtherIndex < CrowdAgents.Num())
        {
            const FCrowdAgent& Other = CrowdAgents[OtherIndex];
            float Distance = FVector::Dist(Agent.Position, Other.Position);
            
            if (Distance > 0.0f && Distance < AlignmentRadius)
            {
                AlignmentForce += Other.Velocity;
                Count++;
            }
        }
    }
    
    if (Count > 0)
    {
        AlignmentForce /= Count;
        AlignmentForce = AlignmentForce.GetSafeNormal() * Agent.Speed;
        AlignmentForce -= Agent.Velocity;
    }
    
    return AlignmentForce;
}

FVector ACrowdManager::CalculateCohesion(const FCrowdAgent& Agent, int32 AgentIndex)
{
    FVector CenterOfMass = FVector::ZeroVector;
    int32 Count = 0;
    
    TArray<int32> NearbyAgents = GetNearbyAgents(Agent.Position, CohesionRadius);
    
    for (int32 OtherIndex : NearbyAgents)
    {
        if (OtherIndex != AgentIndex && OtherIndex < CrowdAgents.Num())
        {
            const FCrowdAgent& Other = CrowdAgents[OtherIndex];
            float Distance = FVector::Dist(Agent.Position, Other.Position);
            
            if (Distance > 0.0f && Distance < CohesionRadius)
            {
                CenterOfMass += Other.Position;
                Count++;
            }
        }
    }
    
    if (Count > 0)
    {
        CenterOfMass /= Count;
        return CalculateSeek(Agent, CenterOfMass);
    }
    
    return FVector::ZeroVector;
}

FVector ACrowdManager::CalculateAvoidance(const FCrowdAgent& Agent)
{
    FVector AvoidanceForce = FVector::ZeroVector;
    
    // Simple obstacle avoidance using raycasting
    UWorld* World = GetWorld();
    if (!World) return AvoidanceForce;
    
    FVector ForwardDirection = Agent.Velocity.GetSafeNormal();
    FVector StartLocation = Agent.Position;
    FVector EndLocation = StartLocation + ForwardDirection * AvoidanceRadius;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    
    if (World->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        FVector AvoidDirection = FVector::CrossProduct(ForwardDirection, FVector::UpVector);
        if (FMath::RandBool())
        {
            AvoidDirection *= -1.0f;
        }
        AvoidanceForce = AvoidDirection * Agent.Speed;
    }
    
    return AvoidanceForce;
}

FVector ACrowdManager::CalculateSeek(const FCrowdAgent& Agent, FVector Target)
{
    FVector Desired = (Target - Agent.Position).GetSafeNormal() * Agent.Speed;
    return Desired - Agent.Velocity;
}

void ACrowdManager::UpdateSpatialGrid()
{
    SpatialGrid.Empty();
    
    for (int32 i = 0; i < CrowdAgents.Num(); ++i)
    {
        const FCrowdAgent& Agent = CrowdAgents[i];
        FIntPoint GridCoord = FIntPoint(
            FMath::FloorToInt(Agent.Position.X / GridCellSize),
            FMath::FloorToInt(Agent.Position.Y / GridCellSize)
        );
        
        SpatialGrid.FindOrAdd(GridCoord).Add(i);
    }
}

TArray<int32> ACrowdManager::GetNearbyAgents(const FVector& Position, float Radius)
{
    TArray<int32> NearbyAgents;
    
    int32 GridRadius = FMath::CeilToInt(Radius / GridCellSize);
    FIntPoint CenterGrid = FIntPoint(
        FMath::FloorToInt(Position.X / GridCellSize),
        FMath::FloorToInt(Position.Y / GridCellSize)
    );
    
    for (int32 x = -GridRadius; x <= GridRadius; ++x)
    {
        for (int32 y = -GridRadius; y <= GridRadius; ++y)
        {
            FIntPoint GridCoord = CenterGrid + FIntPoint(x, y);
            if (SpatialGrid.Contains(GridCoord))
            {
                NearbyAgents.Append(SpatialGrid[GridCoord]);
            }
        }
    }
    
    return NearbyAgents;
}
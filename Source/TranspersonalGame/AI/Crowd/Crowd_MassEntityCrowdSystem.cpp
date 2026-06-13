#include "Crowd_MassEntityCrowdSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_MassEntityCrowdSystem::UCrowd_MassEntityCrowdSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    MaxCrowdSize = 1000;
    SpawnRadius = 2000.0f;
    bEnableFlocking = true;
    bEnablePathfinding = true;
    bEnableLODSystem = true;
    
    // Initialize flocking parameters
    FlockingParams.SeparationWeight = 2.0f;
    FlockingParams.AlignmentWeight = 1.0f;
    FlockingParams.CohesionWeight = 1.0f;
    FlockingParams.NeighborRadius = 200.0f;
    FlockingParams.MaxSpeed = 300.0f;
    FlockingParams.MaxForce = 500.0f;
}

void UCrowd_MassEntityCrowdSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCrowdSystem();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd System initialized with max size: %d"), MaxCrowdSize);
}

void UCrowd_MassEntityCrowdSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CrowdAgents.Num() > 0)
    {
        UpdateCrowdBehavior(DeltaTime);
        
        if (bEnableLODSystem)
        {
            LODUpdateTimer += DeltaTime;
            if (LODUpdateTimer >= LODUpdateInterval)
            {
                UpdateLODSystem();
                LODUpdateTimer = 0.0f;
            }
        }
    }
}

void UCrowd_MassEntityCrowdSystem::InitializeCrowdSystem()
{
    CrowdAgents.Empty();
    HighDetailAgents.Empty();
    MediumDetailAgents.Empty();
    LowDetailAgents.Empty();
    
    // Spawn initial crowd agents
    SpawnCrowdAgents(FMath::Min(MaxCrowdSize, 500)); // Start with 500 agents
}

void UCrowd_MassEntityCrowdSystem::SpawnCrowdAgents(int32 NumAgents)
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    for (int32 i = 0; i < NumAgents; i++)
    {
        FCrowd_CrowdAgent NewAgent;
        
        // Random spawn position within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(100.0f, SpawnRadius);
        
        NewAgent.Position = OwnerLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.Speed = FMath::RandRange(80.0f, 120.0f);
        NewAgent.Radius = FMath::RandRange(40.0f, 60.0f);
        NewAgent.BehaviorState = ECrowd_CrowdBehaviorState::Wandering;
        NewAgent.AgentID = CrowdAgents.Num();
        NewAgent.bIsActive = true;
        
        CrowdAgents.Add(NewAgent);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd agents"), NumAgents);
}

void UCrowd_MassEntityCrowdSystem::UpdateCrowdBehavior(float DeltaTime)
{
    if (!bEnableFlocking || CrowdAgents.Num() == 0)
    {
        return;
    }
    
    // Update each active agent
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        FCrowd_CrowdAgent& Agent = CrowdAgents[i];
        
        if (!Agent.bIsActive)
        {
            continue;
        }
        
        // Calculate flocking forces
        FVector FlockingForce = CalculateFlockingForce(Agent);
        
        // Apply forces to velocity
        Agent.Velocity += FlockingForce * DeltaTime;
        
        // Limit velocity to max speed
        if (Agent.Velocity.Size() > FlockingParams.MaxSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * FlockingParams.MaxSpeed;
        }
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Keep agents within bounds
        FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
        float DistanceFromCenter = FVector::Dist(Agent.Position, OwnerLocation);
        
        if (DistanceFromCenter > SpawnRadius * 1.5f)
        {
            FVector DirectionToCenter = (OwnerLocation - Agent.Position).GetSafeNormal();
            Agent.Velocity += DirectionToCenter * FlockingParams.MaxForce * DeltaTime;
        }
    }
}

void UCrowd_MassEntityCrowdSystem::SetCrowdBehaviorState(ECrowd_CrowdBehaviorState NewState)
{
    for (FCrowd_CrowdAgent& Agent : CrowdAgents)
    {
        Agent.BehaviorState = NewState;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Set crowd behavior state to: %d"), (int32)NewState);
}

FVector UCrowd_MassEntityCrowdSystem::CalculateFlockingForce(const FCrowd_CrowdAgent& Agent)
{
    FVector SeparationForce = CalculateSeparationForce(Agent);
    FVector AlignmentForce = CalculateAlignmentForce(Agent);
    FVector CohesionForce = CalculateCohesionForce(Agent);
    
    // Weight the forces
    FVector TotalForce = 
        SeparationForce * FlockingParams.SeparationWeight +
        AlignmentForce * FlockingParams.AlignmentWeight +
        CohesionForce * FlockingParams.CohesionWeight;
    
    // Limit force magnitude
    if (TotalForce.Size() > FlockingParams.MaxForce)
    {
        TotalForce = TotalForce.GetSafeNormal() * FlockingParams.MaxForce;
    }
    
    return TotalForce;
}

FVector UCrowd_MassEntityCrowdSystem::CalculateSeparationForce(const FCrowd_CrowdAgent& Agent)
{
    FVector SeparationForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_CrowdAgent& OtherAgent : CrowdAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID || !OtherAgent.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        
        if (Distance < FlockingParams.NeighborRadius && Distance > 0.0f)
        {
            FVector Difference = Agent.Position - OtherAgent.Position;
            Difference = Difference.GetSafeNormal() / Distance; // Weight by distance
            SeparationForce += Difference;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce = SeparationForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        SeparationForce -= Agent.Velocity;
    }
    
    return SeparationForce;
}

FVector UCrowd_MassEntityCrowdSystem::CalculateAlignmentForce(const FCrowd_CrowdAgent& Agent)
{
    FVector AlignmentForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_CrowdAgent& OtherAgent : CrowdAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID || !OtherAgent.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        
        if (Distance < FlockingParams.NeighborRadius)
        {
            AlignmentForce += OtherAgent.Velocity;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AlignmentForce /= NeighborCount;
        AlignmentForce = AlignmentForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        AlignmentForce -= Agent.Velocity;
    }
    
    return AlignmentForce;
}

FVector UCrowd_MassEntityCrowdSystem::CalculateCohesionForce(const FCrowd_CrowdAgent& Agent)
{
    FVector CohesionForce = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    for (const FCrowd_CrowdAgent& OtherAgent : CrowdAgents)
    {
        if (OtherAgent.AgentID == Agent.AgentID || !OtherAgent.bIsActive)
        {
            continue;
        }
        
        float Distance = FVector::Dist(Agent.Position, OtherAgent.Position);
        
        if (Distance < FlockingParams.NeighborRadius)
        {
            CohesionForce += OtherAgent.Position;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CohesionForce /= NeighborCount;
        CohesionForce -= Agent.Position;
        CohesionForce = CohesionForce.GetSafeNormal() * FlockingParams.MaxSpeed;
        CohesionForce -= Agent.Velocity;
    }
    
    return CohesionForce;
}

void UCrowd_MassEntityCrowdSystem::UpdateLODSystem()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location for LOD calculations
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    FVector PlayerLocation = FVector::ZeroVector;
    
    if (PlayerController && PlayerController->GetPawn())
    {
        PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
    }
    
    // Clear LOD arrays
    HighDetailAgents.Empty();
    MediumDetailAgents.Empty();
    LowDetailAgents.Empty();
    
    // Categorize agents by distance from player
    for (int32 i = 0; i < CrowdAgents.Num(); i++)
    {
        const FCrowd_CrowdAgent& Agent = CrowdAgents[i];
        
        if (!Agent.bIsActive)
        {
            continue;
        }
        
        float DistanceToPlayer = FVector::Dist(Agent.Position, PlayerLocation);
        
        if (DistanceToPlayer < 500.0f)
        {
            HighDetailAgents.Add(i);
        }
        else if (DistanceToPlayer < 1500.0f)
        {
            MediumDetailAgents.Add(i);
        }
        else
        {
            LowDetailAgents.Add(i);
        }
    }
    
    // Deactivate distant agents if we have too many
    if (CrowdAgents.Num() > MaxCrowdSize)
    {
        int32 AgentsToDeactivate = CrowdAgents.Num() - MaxCrowdSize;
        int32 DeactivatedCount = 0;
        
        // Start with the most distant agents
        for (int32 AgentIndex : LowDetailAgents)
        {
            if (DeactivatedCount >= AgentsToDeactivate)
            {
                break;
            }
            
            CrowdAgents[AgentIndex].bIsActive = false;
            DeactivatedCount++;
        }
    }
}

int32 UCrowd_MassEntityCrowdSystem::GetActiveCrowdAgentCount() const
{
    int32 ActiveCount = 0;
    
    for (const FCrowd_CrowdAgent& Agent : CrowdAgents)
    {
        if (Agent.bIsActive)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}
#include "Crowd_MassEntityCrowdSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UCrowd_MassEntityCrowdSystem::UCrowd_MassEntityCrowdSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.016f; // 60 FPS
    
    MaxCrowdSize = 1000;
    SpawnRadius = 2000.0f;
    bEnableFlocking = true;
    bEnablePathfinding = true;
    bEnableLODSystem = true;
    
    FlockingParams.SeparationWeight = 2.0f;
    FlockingParams.AlignmentWeight = 1.0f;
    FlockingParams.CohesionWeight = 1.0f;
    FlockingParams.NeighborRadius = 200.0f;
    FlockingParams.MaxSpeed = 300.0f;
    FlockingParams.MaxForce = 500.0f;
    
    LODUpdateTimer = 0.0f;
    LODUpdateInterval = 0.5f;
}

void UCrowd_MassEntityCrowdSystem::BeginPlay()
{
    Super::BeginPlay();
    InitializeCrowdSystem();
}

void UCrowd_MassEntityCrowdSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableLODSystem)
    {
        LODUpdateTimer += DeltaTime;
        if (LODUpdateTimer >= LODUpdateInterval)
        {
            UpdateLODSystem();
            LODUpdateTimer = 0.0f;
        }
    }
    
    UpdateCrowdBehavior(DeltaTime);
}

void UCrowd_MassEntityCrowdSystem::InitializeCrowdSystem()
{
    CrowdAgents.Empty();
    HighDetailAgents.Empty();
    MediumDetailAgents.Empty();
    LowDetailAgents.Empty();
    
    SpawnCrowdAgents(FMath::Min(MaxCrowdSize, 500)); // Start with 500 agents
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd System Initialized with %d agents"), CrowdAgents.Num());
}

void UCrowd_MassEntityCrowdSystem::SpawnCrowdAgents(int32 NumAgents)
{
    if (!GetWorld())
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    
    for (int32 i = 0; i < NumAgents; ++i)
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
    
    // Update only high and medium detail agents for performance
    TArray<int32> ActiveAgents;
    ActiveAgents.Append(HighDetailAgents);
    ActiveAgents.Append(MediumDetailAgents);
    
    // If no LOD system, update all agents
    if (!bEnableLODSystem)
    {
        ActiveAgents.Empty();
        for (int32 i = 0; i < CrowdAgents.Num(); ++i)
        {
            if (CrowdAgents[i].bIsActive)
            {
                ActiveAgents.Add(i);
            }
        }
    }
    
    for (int32 AgentIndex : ActiveAgents)
    {
        if (!CrowdAgents.IsValidIndex(AgentIndex) || !CrowdAgents[AgentIndex].bIsActive)
        {
            continue;
        }
        
        FCrowd_CrowdAgent& Agent = CrowdAgents[AgentIndex];
        
        // Calculate flocking forces
        FVector FlockingForce = CalculateFlockingForce(Agent);
        
        // Apply forces to velocity
        Agent.Velocity += FlockingForce * DeltaTime;
        
        // Clamp velocity to max speed
        if (Agent.Velocity.Size() > FlockingParams.MaxSpeed)
        {
            Agent.Velocity = Agent.Velocity.GetSafeNormal() * FlockingParams.MaxSpeed;
        }
        
        // Update position
        Agent.Position += Agent.Velocity * DeltaTime;
        
        // Simple boundary wrapping
        if (GetOwner())
        {
            FVector OwnerLocation = GetOwner()->GetActorLocation();
            float MaxDistance = SpawnRadius * 1.5f;
            
            FVector ToOwner = OwnerLocation - Agent.Position;
            if (ToOwner.Size() > MaxDistance)
            {
                Agent.Position = OwnerLocation + ToOwner.GetSafeNormal() * (MaxDistance * 0.8f);
            }
        }
    }
}

void UCrowd_MassEntityCrowdSystem::SetCrowdBehaviorState(ECrowd_CrowdBehaviorState NewState)
{
    for (FCrowd_CrowdAgent& Agent : CrowdAgents)
    {
        Agent.BehaviorState = NewState;
    }
}

FVector UCrowd_MassEntityCrowdSystem::CalculateFlockingForce(const FCrowd_CrowdAgent& Agent)
{
    FVector SeparationForce = CalculateSeparationForce(Agent) * FlockingParams.SeparationWeight;
    FVector AlignmentForce = CalculateAlignmentForce(Agent) * FlockingParams.AlignmentWeight;
    FVector CohesionForce = CalculateCohesionForce(Agent) * FlockingParams.CohesionWeight;
    
    FVector TotalForce = SeparationForce + AlignmentForce + CohesionForce;
    
    // Clamp total force
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
        if (Distance > 0.0f && Distance < FlockingParams.NeighborRadius * 0.5f)
        {
            FVector Diff = Agent.Position - OtherAgent.Position;
            Diff.Normalize();
            Diff /= Distance; // Weight by distance
            SeparationForce += Diff;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce.Normalize();
        SeparationForce *= FlockingParams.MaxSpeed;
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
        if (Distance > 0.0f && Distance < FlockingParams.NeighborRadius)
        {
            AlignmentForce += OtherAgent.Velocity;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AlignmentForce /= NeighborCount;
        AlignmentForce.Normalize();
        AlignmentForce *= FlockingParams.MaxSpeed;
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
        if (Distance > 0.0f && Distance < FlockingParams.NeighborRadius)
        {
            CohesionForce += OtherAgent.Position;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CohesionForce /= NeighborCount;
        CohesionForce -= Agent.Position;
        CohesionForce.Normalize();
        CohesionForce *= FlockingParams.MaxSpeed;
        CohesionForce -= Agent.Velocity;
    }
    
    return CohesionForce;
}

void UCrowd_MassEntityCrowdSystem::UpdateLODSystem()
{
    if (!GetWorld() || CrowdAgents.Num() == 0)
    {
        return;
    }
    
    // Clear LOD arrays
    HighDetailAgents.Empty();
    MediumDetailAgents.Empty();
    LowDetailAgents.Empty();
    
    // Get player location for distance calculations
    FVector PlayerLocation = FVector::ZeroVector;
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            PlayerLocation = PlayerPawn->GetActorLocation();
        }
    }
    
    // Categorize agents by distance from player
    for (int32 i = 0; i < CrowdAgents.Num(); ++i)
    {
        if (!CrowdAgents[i].bIsActive)
        {
            continue;
        }
        
        float DistanceToPlayer = FVector::Dist(CrowdAgents[i].Position, PlayerLocation);
        
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
    
    // Limit high detail agents for performance
    if (HighDetailAgents.Num() > 100)
    {
        HighDetailAgents.SetNum(100);
    }
    
    // Limit medium detail agents
    if (MediumDetailAgents.Num() > 200)
    {
        MediumDetailAgents.SetNum(200);
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
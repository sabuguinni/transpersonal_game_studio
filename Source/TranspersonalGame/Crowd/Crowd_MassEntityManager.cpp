#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
    RootComponent = RootMeshComponent;

    // Initialize crowd settings
    MaxTotalAgents = 1000;
    UpdateFrequency = 30.0f;
    LODDistance_High = 500.0f;
    LODDistance_Medium = 1500.0f;
    LODDistance_Low = 3000.0f;

    // Initialize runtime data
    CurrentAgentCount = 0;
    LastUpdateTime = 0.0f;
    PerformanceTimer = 0.0f;
    bIsInitialized = false;

    // Reserve memory for agents
    ActiveAgents.Reserve(MaxTotalAgents);
    CrowdZones.Reserve(10);
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();

    // Initialize crowd system
    bIsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager initialized with max %d agents"), MaxTotalAgents);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsInitialized)
    {
        return;
    }

    // Update crowd simulation at specified frequency
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= (1.0f / UpdateFrequency))
    {
        UpdateAgentPositions(LastUpdateTime);
        LastUpdateTime = 0.0f;
    }

    // Performance optimization check
    PerformanceTimer += DeltaTime;
    if (PerformanceTimer >= 2.0f)
    {
        OptimizePerformance();
        PerformanceTimer = 0.0f;
    }
}

void ACrowd_MassEntityManager::SpawnCrowdZone(FVector Center, float Radius, int32 AgentCount)
{
    if (CurrentAgentCount + AgentCount > MaxTotalAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn crowd zone: would exceed max agent limit"));
        return;
    }

    // Create new zone
    FCrowd_ZoneData NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.MaxAgents = AgentCount;

    // Spawn agents in zone
    for (int32 i = 0; i < AgentCount; i++)
    {
        FCrowd_AgentData NewAgent;
        NewAgent.AgentID = CurrentAgentCount + i;
        
        // Random position within zone radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Radius);
        
        NewAgent.Position = Center + FVector(
            Distance * FMath::Cos(Angle),
            Distance * FMath::Sin(Angle),
            0.0f
        );
        
        NewAgent.Speed = FMath::RandRange(80.0f, 120.0f);
        NewAgent.bIsActive = true;

        ActiveAgents.Add(NewAgent);
        NewZone.ActiveAgents.Add(NewAgent.AgentID);
    }

    CrowdZones.Add(NewZone);
    CurrentAgentCount += AgentCount;

    UE_LOG(LogTemp, Log, TEXT("Spawned crowd zone at %s with %d agents"), *Center.ToString(), AgentCount);
}

void ACrowd_MassEntityManager::UpdateCrowdLOD(FVector PlayerPosition)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        float DistanceToPlayer = FVector::Dist(Agent.Position, PlayerPosition);
        ECrowd_LODLevel LODLevel = CalculateLODLevel(DistanceToPlayer);

        // Adjust update behavior based on LOD
        switch (LODLevel)
        {
        case ECrowd_LODLevel::High:
            // Full update rate, detailed behavior
            break;
        case ECrowd_LODLevel::Medium:
            // Reduced update rate
            break;
        case ECrowd_LODLevel::Low:
            // Minimal updates
            break;
        case ECrowd_LODLevel::Culled:
            // No updates, agent effectively disabled
            Agent.bIsActive = false;
            break;
        }
    }
}

void ACrowd_MassEntityManager::SetCrowdBehavior(int32 ZoneIndex, const FString& BehaviorType)
{
    if (!CrowdZones.IsValidIndex(ZoneIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid zone index for behavior setting"));
        return;
    }

    // Apply behavior to all agents in zone
    const FCrowd_ZoneData& Zone = CrowdZones[ZoneIndex];
    
    for (int32 AgentID : Zone.ActiveAgents)
    {
        for (FCrowd_AgentData& Agent : ActiveAgents)
        {
            if (Agent.AgentID == AgentID)
            {
                // Set behavior-specific parameters
                if (BehaviorType == "Panic")
                {
                    Agent.Speed = FMath::RandRange(150.0f, 200.0f);
                }
                else if (BehaviorType == "Gather")
                {
                    Agent.Speed = FMath::RandRange(50.0f, 80.0f);
                }
                else if (BehaviorType == "Patrol")
                {
                    Agent.Speed = FMath::RandRange(60.0f, 100.0f);
                }
                break;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Set behavior '%s' for zone %d"), *BehaviorType, ZoneIndex);
}

int32 ACrowd_MassEntityManager::GetActiveAgentCount() const
{
    int32 ActiveCount = 0;
    for (const FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.bIsActive)
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

float ACrowd_MassEntityManager::GetCurrentFPS() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GetWorld()->GetDeltaSeconds();
    }
    return 60.0f; // Default fallback
}

void ACrowd_MassEntityManager::UpdateAgentPositions(float DeltaTime)
{
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (!Agent.bIsActive) continue;

        // Simple movement update (can be expanded with pathfinding)
        FVector MovementDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();

        Agent.Velocity = MovementDirection * Agent.Speed;
        Agent.Position += Agent.Velocity * DeltaTime;
    }
}

ECrowd_LODLevel ACrowd_MassEntityManager::CalculateLODLevel(float DistanceToPlayer) const
{
    if (DistanceToPlayer <= LODDistance_High)
    {
        return ECrowd_LODLevel::High;
    }
    else if (DistanceToPlayer <= LODDistance_Medium)
    {
        return ECrowd_LODLevel::Medium;
    }
    else if (DistanceToPlayer <= LODDistance_Low)
    {
        return ECrowd_LODLevel::Low;
    }
    else
    {
        return ECrowd_LODLevel::Culled;
    }
}

void ACrowd_MassEntityManager::OptimizePerformance()
{
    float CurrentFPS = GetCurrentFPS();
    
    // Dynamic LOD adjustment based on performance
    if (CurrentFPS < 30.0f)
    {
        // Reduce LOD distances to improve performance
        LODDistance_High *= 0.9f;
        LODDistance_Medium *= 0.9f;
        LODDistance_Low *= 0.9f;
        
        UE_LOG(LogTemp, Warning, TEXT("Performance optimization: Reduced LOD distances due to low FPS (%f)"), CurrentFPS);
    }
    else if (CurrentFPS > 50.0f)
    {
        // Increase LOD distances for better quality
        LODDistance_High = FMath::Min(LODDistance_High * 1.05f, 600.0f);
        LODDistance_Medium = FMath::Min(LODDistance_Medium * 1.05f, 1800.0f);
        LODDistance_Low = FMath::Min(LODDistance_Low * 1.05f, 3600.0f);
    }

    // Cull inactive agents that are too far
    int32 CulledCount = 0;
    for (FCrowd_AgentData& Agent : ActiveAgents)
    {
        if (Agent.bIsActive && FVector::Dist(Agent.Position, FVector::ZeroVector) > LODDistance_Low * 2.0f)
        {
            Agent.bIsActive = false;
            CulledCount++;
        }
    }

    if (CulledCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance optimization: Culled %d distant agents"), CulledCount);
    }
}
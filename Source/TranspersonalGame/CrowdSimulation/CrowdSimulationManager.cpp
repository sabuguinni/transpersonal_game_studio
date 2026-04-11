#include "CrowdSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    MaxCrowdAgents = 10000;
    CrowdDensityMultiplier = 1.0f;
    bEnableDynamicDensity = true;
    bEnableEmergencyBehavior = true;
    
    HighDetailDistance = 1000.0f;
    MediumDetailDistance = 3000.0f;
    LowDetailDistance = 8000.0f;
    
    SpawnInterval = 5.0f;
    AgentsPerSpawn = 10;
    
    // Initialize runtime values
    CurrentActiveAgents = 0;
    CurrentFrameTime = 0.0f;
    HighDetailAgents = 0;
    MediumDetailAgents = 0;
    LowDetailAgents = 0;
    
    LastSpawnTime = 0.0f;
    LastPerformanceCheck = 0.0f;
    PerformanceCheckInterval = 1.0f;
    
    bEmergencyActive = false;
    CurrentEmergencyType = ECrowdEmergencyType::None;
    EmergencyRadius = 0.0f;
    EmergencyStartTime = 0.0f;
    
    GridCellSize = 1000.0f;
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Starting crowd simulation system"));
    
    // Initialize agent pool
    AgentPool.Reserve(MaxCrowdAgents);
    ActiveAgents.Reserve(MaxCrowdAgents);
    
    // Auto-find spawn points if none assigned
    if (SpawnPoints.Num() == 0)
    {
        TArray<AActor*> FoundSpawnPoints;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrowdSpawnPoint::StaticClass(), FoundSpawnPoints);
        
        for (AActor* Actor : FoundSpawnPoints)
        {
            if (ACrowdSpawnPoint* SpawnPoint = Cast<ACrowdSpawnPoint>(Actor))
            {
                SpawnPoints.Add(SpawnPoint);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Found %d spawn points"), SpawnPoints.Num());
    }
    
    // Start simulation
    StartCrowdSimulation();
}

void ACrowdSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopCrowdSimulation();
    Super::EndPlay(EndPlayReason);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CurrentFrameTime = DeltaTime;
    
    // Performance monitoring
    if (GetWorld()->GetTimeSeconds() - LastPerformanceCheck >= PerformanceCheckInterval)
    {
        OptimizePerformance();
        LastPerformanceCheck = GetWorld()->GetTimeSeconds();
    }
    
    // Update spatial grid
    UpdateSpatialGrid();
    
    // Process emergency behavior
    if (bEmergencyActive && bEnableEmergencyBehavior)
    {
        ProcessEmergencyBehavior(DeltaTime);
    }
    
    // Spawn new agents
    if (GetWorld()->GetTimeSeconds() - LastSpawnTime >= SpawnInterval)
    {
        for (ACrowdSpawnPoint* SpawnPoint : SpawnPoints)
        {
            if (SpawnPoint && SpawnPoint->IsActive())
            {
                int32 SpawnCount = FMath::Min(AgentsPerSpawn, MaxCrowdAgents - CurrentActiveAgents);
                if (SpawnCount > 0)
                {
                    SpawnAgentsAtPoint(SpawnPoint, SpawnCount);
                }
            }
        }
        LastSpawnTime = GetWorld()->GetTimeSeconds();
    }
    
    // Update agent LODs based on distance to player
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (PC && PC->GetPawn())
    {
        FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
        
        HighDetailAgents = 0;
        MediumDetailAgents = 0;
        LowDetailAgents = 0;
        
        for (FCrowdAgent& Agent : ActiveAgents)
        {
            float DistanceToPlayer = FVector::Dist(Agent.Location, PlayerLocation);
            UpdateAgentLOD(Agent, DistanceToPlayer);
        }
    }
    
    // Update density tracking
    float CurrentDensity = GetCurrentDensity();
    OnCrowdDensityChanged.Broadcast(CurrentDensity, 1.0f);
}

void ACrowdSimulationManager::StartCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Starting crowd simulation"));
    SetActorTickEnabled(true);
    
    // Initialize first wave of agents
    for (ACrowdSpawnPoint* SpawnPoint : SpawnPoints)
    {
        if (SpawnPoint && SpawnPoint->IsActive())
        {
            SpawnAgentsAtPoint(SpawnPoint, AgentsPerSpawn);
        }
    }
}

void ACrowdSimulationManager::StopCrowdSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Stopping crowd simulation"));
    SetActorTickEnabled(false);
    
    // Clear all agents
    ActiveAgents.Empty();
    CurrentActiveAgents = 0;
    
    // Clear emergency state
    ClearEmergencyEvent();
}

void ACrowdSimulationManager::PauseCrowdSimulation()
{
    SetActorTickEnabled(!IsActorTickEnabled());
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Simulation %s"), 
           IsActorTickEnabled() ? TEXT("Resumed") : TEXT("Paused"));
}

void ACrowdSimulationManager::SetCrowdDensity(float NewDensity)
{
    CrowdDensityMultiplier = FMath::Clamp(NewDensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Density set to %.2f"), CrowdDensityMultiplier);
}

void ACrowdSimulationManager::TriggerEmergencyEvent(ECrowdEmergencyType EventType, FVector Location, float Radius)
{
    bEmergencyActive = true;
    CurrentEmergencyType = EventType;
    EmergencyLocation = Location;
    EmergencyRadius = Radius;
    EmergencyStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Emergency event triggered at %s"), 
           *Location.ToString());
    
    OnCrowdEmergencyEvent.Broadcast(EventType);
}

void ACrowdSimulationManager::ClearEmergencyEvent()
{
    if (bEmergencyActive)
    {
        bEmergencyActive = false;
        CurrentEmergencyType = ECrowdEmergencyType::None;
        EmergencyRadius = 0.0f;
        
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Emergency event cleared"));
    }
}

int32 ACrowdSimulationManager::GetAgentsInRadius(FVector Center, float Radius) const
{
    int32 Count = 0;
    float RadiusSquared = Radius * Radius;
    
    for (const FCrowdAgent& Agent : ActiveAgents)
    {
        if (FVector::DistSquared(Agent.Location, Center) <= RadiusSquared)
        {
            Count++;
        }
    }
    
    return Count;
}

float ACrowdSimulationManager::GetCrowdDensityAtLocation(FVector Location, float SampleRadius) const
{
    int32 AgentsInArea = GetAgentsInRadius(Location, SampleRadius);
    float AreaSize = PI * SampleRadius * SampleRadius;
    return AgentsInArea / AreaSize * 10000.0f; // Agents per 10,000 square units
}

TArray<FVector> ACrowdSimulationManager::GetHighTrafficAreas() const
{
    TArray<FVector> HighTrafficAreas;
    
    // Sample grid points across the level
    for (const auto& GridCell : SpatialGrid)
    {
        if (GridCell.Value.Num() > 20) // Threshold for "high traffic"
        {
            FVector CellCenter = FVector(GridCell.Key) * GridCellSize;
            HighTrafficAreas.Add(CellCenter);
        }
    }
    
    return HighTrafficAreas;
}

void ACrowdSimulationManager::SpawnAgentsAtPoint(ACrowdSpawnPoint* SpawnPoint, int32 Count)
{
    if (!SpawnPoint || CurrentActiveAgents >= MaxCrowdAgents)
    {
        return;
    }
    
    for (int32 i = 0; i < Count && CurrentActiveAgents < MaxCrowdAgents; i++)
    {
        FCrowdAgent NewAgent;
        NewAgent.AgentID = ActiveAgents.Num();
        NewAgent.Location = SpawnPoint->GetRandomSpawnLocation();
        NewAgent.Velocity = FVector::ZeroVector;
        NewAgent.TargetLocation = SpawnPoint->GetRandomDestination();
        NewAgent.BehaviorState = ECrowdBehaviorState::Walking;
        NewAgent.LODLevel = ECrowdLODLevel::High;
        NewAgent.LastUpdateTime = GetWorld()->GetTimeSeconds();
        NewAgent.SpawnTime = GetWorld()->GetTimeSeconds();
        
        ActiveAgents.Add(NewAgent);
        CurrentActiveAgents++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Spawned %d agents at %s"), 
           Count, *SpawnPoint->GetName());
}

void ACrowdSimulationManager::UpdateAgentLOD(FCrowdAgent& Agent, float DistanceToPlayer)
{
    ECrowdLODLevel NewLOD;
    
    if (DistanceToPlayer <= HighDetailDistance)
    {
        NewLOD = ECrowdLODLevel::High;
        HighDetailAgents++;
    }
    else if (DistanceToPlayer <= MediumDetailDistance)
    {
        NewLOD = ECrowdLODLevel::Medium;
        MediumDetailAgents++;
    }
    else if (DistanceToPlayer <= LowDetailDistance)
    {
        NewLOD = ECrowdLODLevel::Low;
        LowDetailAgents++;
    }
    else
    {
        NewLOD = ECrowdLODLevel::Culled;
    }
    
    Agent.LODLevel = NewLOD;
}

void ACrowdSimulationManager::UpdateSpatialGrid()
{
    SpatialGrid.Empty();
    
    for (int32 i = 0; i < ActiveAgents.Num(); i++)
    {
        const FCrowdAgent& Agent = ActiveAgents[i];
        FIntVector GridCoord = FIntVector(
            FMath::FloorToInt(Agent.Location.X / GridCellSize),
            FMath::FloorToInt(Agent.Location.Y / GridCellSize),
            FMath::FloorToInt(Agent.Location.Z / GridCellSize)
        );
        
        SpatialGrid.FindOrAdd(GridCoord).Add(i);
    }
}

void ACrowdSimulationManager::ProcessEmergencyBehavior(float DeltaTime)
{
    float EmergencyDuration = GetWorld()->GetTimeSeconds() - EmergencyStartTime;
    
    // Auto-clear emergency after 30 seconds
    if (EmergencyDuration > 30.0f)
    {
        ClearEmergencyEvent();
        return;
    }
    
    // Update agent behaviors based on emergency
    for (FCrowdAgent& Agent : ActiveAgents)
    {
        float DistanceToEmergency = FVector::Dist(Agent.Location, EmergencyLocation);
        
        if (DistanceToEmergency <= EmergencyRadius)
        {
            switch (CurrentEmergencyType)
            {
                case ECrowdEmergencyType::Fire:
                case ECrowdEmergencyType::Predator:
                    Agent.BehaviorState = ECrowdBehaviorState::Fleeing;
                    // Set target away from emergency
                    Agent.TargetLocation = Agent.Location + (Agent.Location - EmergencyLocation).GetSafeNormal() * 2000.0f;
                    break;
                    
                case ECrowdEmergencyType::Curiosity:
                    Agent.BehaviorState = ECrowdBehaviorState::Investigating;
                    // Move towards emergency (but not too close)
                    Agent.TargetLocation = EmergencyLocation + FMath::VRand() * 500.0f;
                    break;
            }
        }
    }
}

void ACrowdSimulationManager::OptimizePerformance()
{
    // Remove agents that are too far from player or have been active too long
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn())
    {
        return;
    }
    
    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    float MaxDistance = LowDetailDistance * 1.5f;
    float MaxLifetime = 300.0f; // 5 minutes
    
    for (int32 i = ActiveAgents.Num() - 1; i >= 0; i--)
    {
        const FCrowdAgent& Agent = ActiveAgents[i];
        float DistanceToPlayer = FVector::Dist(Agent.Location, PlayerLocation);
        float Lifetime = GetWorld()->GetTimeSeconds() - Agent.SpawnTime;
        
        if (DistanceToPlayer > MaxDistance || Lifetime > MaxLifetime)
        {
            ActiveAgents.RemoveAt(i);
            CurrentActiveAgents--;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Performance optimization - %d active agents"), 
           CurrentActiveAgents);
}
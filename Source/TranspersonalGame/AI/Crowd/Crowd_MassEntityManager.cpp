#include "Crowd_MassEntityManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = TickInterval;
    
    // Initialize default spawn zone
    FCrowd_SpawnZone DefaultZone;
    DefaultZone.Center = FVector::ZeroVector;
    DefaultZone.Radius = 2000.0f;
    DefaultZone.MaxAgents = 5000;
    DefaultZone.DensityLevel = ECrowd_DensityLevel::Medium;
    
    FCrowd_AgentConfig DefaultConfig;
    DefaultConfig.MovementSpeed = 150.0f;
    DefaultConfig.PersonalSpace = 100.0f;
    DefaultConfig.ViewRange = 500.0f;
    DefaultConfig.BehaviorType = ECrowd_BehaviorType::Wandering;
    DefaultZone.AgentConfigs.Add(DefaultConfig);
    
    SpawnZones.Add(DefaultZone);
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager: Initializing..."));
    
    InitializeMassEntitySystem();
    
    // Create initial path network
    CreatePathNetwork();
    
    // Spawn initial crowds
    for (const FCrowd_SpawnZone& Zone : SpawnZones)
    {
        SpawnCrowdInZone(Zone);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager: Initialization complete. Active agents: %d"), ActiveAgentCount);
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DespawnAllAgents();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    CurrentFrameTime = DeltaTime;
    
    // Update agent behaviors
    UpdateAgentBehaviors(DeltaTime);
    
    // Process culling for performance
    ProcessCulling();
    
    // Update LOD system periodically
    LastLODUpdate += DeltaTime;
    if (LastLODUpdate >= LODUpdateInterval)
    {
        UpdateLODSystem();
        LastLODUpdate = 0.0f;
    }
    
    // Performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= 1.0f)
    {
        OptimizePerformance();
        LastPerformanceCheck = 0.0f;
    }
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Crowd Manager: No world found"));
        return;
    }
    
    // Get Mass Entity subsystems
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem initialized"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Entity Subsystem not available - using fallback crowd system"));
    }
    
    // Initialize spawner subsystem
    MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
    if (MassSpawnerSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Spawner Subsystem initialized"));
    }
    
    // Initialize simulation subsystem
    MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    if (MassSimulationSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Simulation Subsystem initialized"));
    }
}

void ACrowd_MassEntityManager::SpawnCrowdInZone(const FCrowd_SpawnZone& Zone)
{
    if (ActiveAgentCount >= MaxTotalAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn more agents - max limit reached: %d"), MaxTotalAgents);
        return;
    }
    
    int32 AgentsToSpawn = FMath::Min(Zone.MaxAgents, MaxTotalAgents - ActiveAgentCount);
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %d agents in zone at %s (radius: %.1f)"), 
           AgentsToSpawn, *Zone.Center.ToString(), Zone.Radius);
    
    // Spawn agents in circular pattern
    for (int32 i = 0; i < AgentsToSpawn; i++)
    {
        // Random position within zone radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Zone.Radius);
        
        FVector SpawnLocation = Zone.Center + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Adjust Z to ground level
        FHitResult HitResult;
        FVector TraceStart = SpawnLocation + FVector(0, 0, 1000);
        FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000);
        
        if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
        {
            SpawnLocation.Z = HitResult.Location.Z + 50.0f; // Offset above ground
        }
        
        // For now, create debug visualization (will be replaced with Mass Entity spawning)
        if (i % 100 == 0) // Only visualize every 100th agent for performance
        {
            DrawDebugSphere(GetWorld(), SpawnLocation, 25.0f, 8, FColor::Blue, false, 5.0f);
        }
        
        ActiveAgentCount++;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d agents. Total active: %d"), AgentsToSpawn, ActiveAgentCount);
}

void ACrowd_MassEntityManager::DespawnAllAgents()
{
    UE_LOG(LogTemp, Warning, TEXT("Despawning all %d agents"), ActiveAgentCount);
    
    if (MassEntitySubsystem)
    {
        // Mass Entity despawn logic would go here
    }
    
    ActiveAgentCount = 0;
    VisibleAgentCount = 0;
}

void ACrowd_MassEntityManager::UpdateLODSystem()
{
    if (!GetWorld()) return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD based on distance to player
    for (const FCrowd_SpawnZone& Zone : SpawnZones)
    {
        float DistanceToPlayer = FVector::Dist(PlayerLocation, Zone.Center);
        
        // Determine LOD level based on distance
        int32 NewLODLevel = 0;
        if (DistanceToPlayer > 2000.0f) NewLODLevel = 2;      // Low detail
        else if (DistanceToPlayer > 1000.0f) NewLODLevel = 1; // Medium detail
        else NewLODLevel = 0;                                  // High detail
        
        // Apply LOD optimizations
        AdaptiveLOD();
    }
}

void ACrowd_MassEntityManager::SetDensityLevel(ECrowd_DensityLevel NewDensity)
{
    UE_LOG(LogTemp, Warning, TEXT("Setting crowd density to level: %d"), (int32)NewDensity);
    
    // Adjust spawn zones based on density
    for (FCrowd_SpawnZone& Zone : SpawnZones)
    {
        Zone.DensityLevel = NewDensity;
        
        switch (NewDensity)
        {
            case ECrowd_DensityLevel::Low:
                Zone.MaxAgents = FMath::Min(Zone.MaxAgents, 1000);
                break;
            case ECrowd_DensityLevel::Medium:
                Zone.MaxAgents = FMath::Min(Zone.MaxAgents, 5000);
                break;
            case ECrowd_DensityLevel::High:
                Zone.MaxAgents = FMath::Min(Zone.MaxAgents, 15000);
                break;
            case ECrowd_DensityLevel::Ultra:
                Zone.MaxAgents = FMath::Min(Zone.MaxAgents, 50000);
                break;
        }
    }
}

void ACrowd_MassEntityManager::RegisterWaypoint(const FVector& Location, const FString& WaypointName)
{
    WaypointNetwork.Add(Location);
    NamedWaypoints.Add(WaypointName, Location);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered waypoint '%s' at %s"), *WaypointName, *Location.ToString());
}

void ACrowd_MassEntityManager::CreatePathNetwork()
{
    // Create default waypoint network in a grid pattern
    const float GridSize = 1000.0f;
    const int32 GridDimension = 5;
    
    WaypointNetwork.Empty();
    NamedWaypoints.Empty();
    
    for (int32 X = 0; X < GridDimension; X++)
    {
        for (int32 Y = 0; Y < GridDimension; Y++)
        {
            FVector WaypointLocation = FVector(
                (X - GridDimension/2) * GridSize,
                (Y - GridDimension/2) * GridSize,
                100.0f
            );
            
            FString WaypointName = FString::Printf(TEXT("Waypoint_%d_%d"), X, Y);
            RegisterWaypoint(WaypointLocation, WaypointName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Created path network with %d waypoints"), WaypointNetwork.Num());
}

TArray<FVector> ACrowd_MassEntityManager::GetOptimalPath(const FVector& Start, const FVector& End)
{
    TArray<FVector> Path;
    
    // Simple pathfinding - find nearest waypoints and create path
    FVector NearestStart = Start;
    FVector NearestEnd = End;
    
    float MinStartDist = MAX_FLT;
    float MinEndDist = MAX_FLT;
    
    for (const FVector& Waypoint : WaypointNetwork)
    {
        float StartDist = FVector::Dist(Start, Waypoint);
        if (StartDist < MinStartDist)
        {
            MinStartDist = StartDist;
            NearestStart = Waypoint;
        }
        
        float EndDist = FVector::Dist(End, Waypoint);
        if (EndDist < MinEndDist)
        {
            MinEndDist = EndDist;
            NearestEnd = Waypoint;
        }
    }
    
    Path.Add(Start);
    Path.Add(NearestStart);
    Path.Add(NearestEnd);
    Path.Add(End);
    
    return Path;
}

float ACrowd_MassEntityManager::GetCurrentPerformanceMetric() const
{
    // Simple performance metric based on frame time and agent count
    float BaselineFrameTime = 0.016f; // 60 FPS target
    float PerformanceRatio = BaselineFrameTime / FMath::Max(CurrentFrameTime, 0.001f);
    
    // Factor in agent count
    float AgentLoadFactor = (float)ActiveAgentCount / (float)MaxTotalAgents;
    
    return PerformanceRatio * (1.0f - AgentLoadFactor * 0.5f);
}

void ACrowd_MassEntityManager::OptimizePerformance()
{
    float PerformanceMetric = GetCurrentPerformanceMetric();
    
    if (PerformanceMetric < 0.8f) // Performance below 80%
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance low (%.2f) - optimizing crowd system"), PerformanceMetric);
        
        // Reduce visible agents
        MaxVisibleAgents = FMath::Max(MaxVisibleAgents - 100, 500);
        
        // Increase culling distance
        CullingDistance = FMath::Min(CullingDistance * 1.1f, 10000.0f);
        
        // Force LOD update
        UpdateLODSystem();
    }
    else if (PerformanceMetric > 1.2f) // Performance above 120%
    {
        // Increase visible agents
        MaxVisibleAgents = FMath::Min(MaxVisibleAgents + 50, 5000);
        
        // Decrease culling distance
        CullingDistance = FMath::Max(CullingDistance * 0.95f, 2000.0f);
    }
}

void ACrowd_MassEntityManager::UpdateAgentBehaviors(float DeltaTime)
{
    // Update agent AI behaviors (simplified for now)
    // In full implementation, this would update Mass Entity components
    
    if (ActiveAgentCount > 0)
    {
        // Simulate behavior processing time
        float BehaviorProcessingTime = ActiveAgentCount * 0.00001f; // Very small per-agent cost
        
        // Update pathfinding
        UpdatePathfinding();
        
        // Handle collision avoidance
        HandleCollisionAvoidance();
    }
}

void ACrowd_MassEntityManager::ProcessCulling()
{
    if (!GetWorld()) return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Dynamic culling based on distance and occlusion
    DynamicCulling();
    
    // Update visible agent count (simplified)
    VisibleAgentCount = FMath::Min(ActiveAgentCount, MaxVisibleAgents);
}

void ACrowd_MassEntityManager::HandleCollisionAvoidance()
{
    // Simplified collision avoidance system
    // In full implementation, this would use Mass Entity spatial partitioning
}

void ACrowd_MassEntityManager::UpdatePathfinding()
{
    // Update pathfinding for active agents
    // In full implementation, this would update Mass Entity movement components
}

void ACrowd_MassEntityManager::AdaptiveLOD()
{
    // Adaptive LOD system based on performance and distance
    float PerformanceMetric = GetCurrentPerformanceMetric();
    
    if (PerformanceMetric < 0.7f)
    {
        // Aggressive LOD reduction
        MaxVisibleAgents = FMath::Max(MaxVisibleAgents - 200, 300);
    }
    else if (PerformanceMetric > 1.1f)
    {
        // Increase LOD quality
        MaxVisibleAgents = FMath::Min(MaxVisibleAgents + 100, 3000);
    }
}

void ACrowd_MassEntityManager::DynamicCulling()
{
    // Dynamic culling based on view frustum and occlusion
    if (bEnableOcclusion)
    {
        // Occlusion culling logic would go here
        // For now, simple distance-based culling
        VisibleAgentCount = FMath::Min(ActiveAgentCount, MaxVisibleAgents);
    }
}

void ACrowd_MassEntityManager::BatchProcessing()
{
    // Batch processing for performance optimization
    // Process agents in batches to spread load across frames
}
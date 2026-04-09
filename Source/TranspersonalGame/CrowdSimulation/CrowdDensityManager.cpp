#include "CrowdDensityManager.h"
#include "MassEntitySubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCrowdDensityManager::UCrowdDensityManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for density management
    
    GlobalDensityMultiplier = 1.0f;
    bUsePlayerProximityScaling = true;
    PlayerProximityMaxDistance = 5000.0f;
    bHighPerformanceMode = false;
    
    DensityUpdateFrequency = 2.0f;
    SpawnUpdateFrequency = 10.0f;
    
    GlobalMaxAgents = 50000;
    MaxAgentsPerFrame = 100;
    
    TotalActiveAgents = 0;
    CurrentAverageDensity = 0.0f;
}

void UCrowdDensityManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass Entity subsystem
    if (UWorld* World = GetWorld())
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        if (!MassEntitySubsystem)
        {
            UE_LOG(LogTemp, Error, TEXT("CrowdDensityManager: Failed to get MassEntitySubsystem"));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Initialized with %d spawn zones"), SpawnZones.Num());
}

void UCrowdDensityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    LastFrameTime = DeltaTime;
    AgentsSpawnedThisFrame = 0;
    
    // Update spawn zones
    if (GetWorld()->GetTimeSeconds() - LastSpawnUpdate > (1.0f / SpawnUpdateFrequency))
    {
        UpdateSpawnZones(DeltaTime);
        LastSpawnUpdate = GetWorld()->GetTimeSeconds();
    }
    
    // Update density management
    if (GetWorld()->GetTimeSeconds() - LastDensityUpdate > (1.0f / DensityUpdateFrequency))
    {
        UpdateFlowPaths(DeltaTime);
        ManageAgentCulling();
        UpdatePerformanceMetrics();
        LastDensityUpdate = GetWorld()->GetTimeSeconds();
    }
}

void UCrowdDensityManager::AddSpawnZone(const FCrowdSpawnZone& NewZone)
{
    SpawnZones.Add(NewZone);
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Added spawn zone at %s with max %d agents"), 
           *NewZone.ZoneCenter.ToString(), NewZone.MaxAgents);
}

void UCrowdDensityManager::RemoveSpawnZone(int32 ZoneIndex)
{
    if (SpawnZones.IsValidIndex(ZoneIndex))
    {
        SpawnZones.RemoveAt(ZoneIndex);
        UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Removed spawn zone at index %d"), ZoneIndex);
    }
}

void UCrowdDensityManager::SetZoneActive(int32 ZoneIndex, bool bActive)
{
    if (SpawnZones.IsValidIndex(ZoneIndex))
    {
        SpawnZones[ZoneIndex].bActiveZone = bActive;
        UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Set zone %d active state to %s"), 
               ZoneIndex, bActive ? TEXT("true") : TEXT("false"));
    }
}

void UCrowdDensityManager::UpdateZoneDensity(int32 ZoneIndex, int32 NewMaxAgents)
{
    if (SpawnZones.IsValidIndex(ZoneIndex))
    {
        SpawnZones[ZoneIndex].MaxAgents = NewMaxAgents;
        UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Updated zone %d max agents to %d"), 
               ZoneIndex, NewMaxAgents);
    }
}

void UCrowdDensityManager::AddFlowPath(const FCrowdFlowPath& NewPath)
{
    FlowPaths.Add(NewPath);
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Added flow path with %d points"), NewPath.PathPoints.Num());
}

void UCrowdDensityManager::SetFlowPathActive(int32 PathIndex, bool bActive)
{
    if (FlowPaths.IsValidIndex(PathIndex))
    {
        // In a full implementation, we'd have an active flag on FCrowdFlowPath
        UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Set flow path %d active state to %s"), 
               PathIndex, bActive ? TEXT("true") : TEXT("false"));
    }
}

void UCrowdDensityManager::SetGlobalDensityMultiplier(float Multiplier)
{
    GlobalDensityMultiplier = FMath::Clamp(Multiplier, 0.0f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Set global density multiplier to %f"), GlobalDensityMultiplier);
}

void UCrowdDensityManager::SetPlayerProximityScaling(bool bEnable, float MaxDistance)
{
    bUsePlayerProximityScaling = bEnable;
    PlayerProximityMaxDistance = MaxDistance;
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Player proximity scaling %s with max distance %f"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"), MaxDistance);
}

void UCrowdDensityManager::SetPerformanceMode(bool bHighPerformance)
{
    bHighPerformanceMode = bHighPerformance;
    
    if (bHighPerformance)
    {
        // Reduce update frequencies and limits for better performance
        DensityUpdateFrequency = 1.0f;
        SpawnUpdateFrequency = 5.0f;
        MaxAgentsPerFrame = 50;
    }
    else
    {
        // Restore normal update rates
        DensityUpdateFrequency = 2.0f;
        SpawnUpdateFrequency = 10.0f;
        MaxAgentsPerFrame = 100;
    }
    
    UE_LOG(LogTemp, Log, TEXT("CrowdDensityManager: Performance mode set to %s"), 
           bHighPerformance ? TEXT("high performance") : TEXT("normal"));
}

int32 UCrowdDensityManager::GetTotalActiveAgents() const
{
    return TotalActiveAgents;
}

float UCrowdDensityManager::GetAverageDensity() const
{
    return CurrentAverageDensity;
}

TArray<int32> UCrowdDensityManager::GetZoneAgentCounts() const
{
    TArray<int32> Counts;
    for (const FCrowdSpawnZone& Zone : SpawnZones)
    {
        Counts.Add(Zone.CurrentAgentCount);
    }
    return Counts;
}

void UCrowdDensityManager::UpdateSpawnZones(float DeltaTime)
{
    int32 TotalAgents = 0;
    
    for (FCrowdSpawnZone& Zone : SpawnZones)
    {
        if (!Zone.bActiveZone || !ShouldSpawnInZone(Zone))
        {
            continue;
        }
        
        // Calculate desired agent count with scaling
        float DistanceScaling = bUsePlayerProximityScaling ? CalculatePlayerDistanceScaling(Zone.ZoneCenter) : 1.0f;
        int32 TargetAgents = FMath::RoundToInt(Zone.MaxAgents * GlobalDensityMultiplier * DistanceScaling);
        
        // Spawn agents if below target
        if (Zone.CurrentAgentCount < TargetAgents && AgentsSpawnedThisFrame < MaxAgentsPerFrame)
        {
            float TimeSinceLastSpawn = GetWorld()->GetTimeSeconds() - Zone.LastSpawnTime;
            float SpawnInterval = 1.0f / Zone.SpawnRate;
            
            if (TimeSinceLastSpawn >= SpawnInterval)
            {
                int32 AgentsToSpawn = FMath::Min(TargetAgents - Zone.CurrentAgentCount, 
                                                 MaxAgentsPerFrame - AgentsSpawnedThisFrame);
                AgentsToSpawn = FMath::Min(AgentsToSpawn, 10); // Limit per zone per frame
                
                if (AgentsToSpawn > 0)
                {
                    SpawnAgentsInZone(Zone, AgentsToSpawn);
                    Zone.LastSpawnTime = GetWorld()->GetTimeSeconds();
                    AgentsSpawnedThisFrame += AgentsToSpawn;
                }
            }
        }
        
        TotalAgents += Zone.CurrentAgentCount;
    }
    
    TotalActiveAgents = TotalAgents;
}

void UCrowdDensityManager::UpdateFlowPaths(float DeltaTime)
{
    for (const FCrowdFlowPath& Path : FlowPaths)
    {
        if (Path.PathPoints.Num() < 2 || AgentsSpawnedThisFrame >= MaxAgentsPerFrame)
        {
            continue;
        }
        
        // Calculate agents to spawn on this path
        int32 AgentsToSpawn = FMath::RoundToInt(Path.FlowRate * DeltaTime);
        AgentsToSpawn = FMath::Min(AgentsToSpawn, MaxAgentsPerFrame - AgentsSpawnedThisFrame);
        
        if (AgentsToSpawn > 0)
        {
            SpawnAgentsOnPath(Path, AgentsToSpawn);
            AgentsSpawnedThisFrame += AgentsToSpawn;
        }
    }
}

void UCrowdDensityManager::ManageAgentCulling()
{
    // In a full implementation, this would:
    // 1. Query all crowd entities
    // 2. Check distance from player
    // 3. Despawn entities beyond culling distance
    // 4. Update zone agent counts
    
    if (TotalActiveAgents > GlobalMaxAgents)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdDensityManager: Agent count (%d) exceeds global limit (%d), culling needed"), 
               TotalActiveAgents, GlobalMaxAgents);
    }
}

void UCrowdDensityManager::UpdatePerformanceMetrics()
{
    // Calculate average density across all zones
    float TotalDensity = 0.0f;
    int32 ActiveZones = 0;
    
    for (const FCrowdSpawnZone& Zone : SpawnZones)
    {
        if (Zone.bActiveZone)
        {
            float ZoneArea = Zone.ZoneExtents.X * Zone.ZoneExtents.Y * 4.0f; // Approximate area
            float ZoneDensity = ZoneArea > 0.0f ? Zone.CurrentAgentCount / ZoneArea : 0.0f;
            TotalDensity += ZoneDensity;
            ActiveZones++;
        }
    }
    
    CurrentAverageDensity = ActiveZones > 0 ? TotalDensity / ActiveZones : 0.0f;
    
    // Performance warnings
    if (LastFrameTime > 0.033f) // 30 FPS threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdDensityManager: Performance warning - frame time %f ms"), 
               LastFrameTime * 1000.0f);
    }
}

void UCrowdDensityManager::SpawnAgentsInZone(FCrowdSpawnZone& Zone, int32 AgentsToSpawn)
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // In a full implementation, this would:
    // 1. Generate random spawn positions within zone bounds
    // 2. Create Mass entities with appropriate fragments
    // 3. Set initial movement targets
    // 4. Update zone agent count
    
    Zone.CurrentAgentCount += AgentsToSpawn;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("CrowdDensityManager: Spawned %d agents in zone at %s"), 
           AgentsToSpawn, *Zone.ZoneCenter.ToString());
}

void UCrowdDensityManager::SpawnAgentsOnPath(const FCrowdFlowPath& Path, int32 AgentsToSpawn)
{
    if (!MassEntitySubsystem || Path.PathPoints.Num() < 2)
    {
        return;
    }
    
    // In a full implementation, this would:
    // 1. Choose random positions along the path
    // 2. Create Mass entities with path-following behavior
    // 3. Set movement targets along the path
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("CrowdDensityManager: Spawned %d agents on flow path"), AgentsToSpawn);
}

float UCrowdDensityManager::CalculatePlayerDistanceScaling(const FVector& Position)
{
    if (!bUsePlayerProximityScaling)
    {
        return 1.0f;
    }
    
    // Get player location
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = FVector::Dist(Position, PlayerPawn->GetActorLocation());
            
            if (Distance <= PlayerProximityMaxDistance)
            {
                // Scale from 1.0 (close) to 0.1 (far)
                float DistanceRatio = Distance / PlayerProximityMaxDistance;
                return FMath::Lerp(1.0f, 0.1f, DistanceRatio);
            }
        }
    }
    
    return 0.1f; // Minimal density when player is far away
}

bool UCrowdDensityManager::ShouldSpawnInZone(const FCrowdSpawnZone& Zone)
{
    // Performance checks
    if (bHighPerformanceMode && TotalActiveAgents > GlobalMaxAgents * 0.8f)
    {
        return false;
    }
    
    // Zone-specific checks
    if (Zone.CurrentAgentCount >= Zone.MaxAgents)
    {
        return false;
    }
    
    return true;
}
#include "Crowd_MassEntityManager.h"
#include "Crowd_AgentComponent.h"
#include "Crowd_SpawnPoint.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCrowd_MassEntityManager::UCrowd_MassEntityManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default settings
    PerformanceSettings = FCrowd_PerformanceSettings();
    PathfindingConfig = FCrowd_PathfindingConfig();
    
    ActiveAgentCount = 0;
    MaxAllowedAgents = PerformanceSettings.MaxAgents;
    bSimulationEnabled = true;
    LODUpdateFrequency = 0.1f;
}

void UCrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Entity Manager: BeginPlay"));
    
    // Initialize the Mass Entity system
    InitializeMassSystem();
    
    // Find all spawn points in the level
    TArray<AActor*> FoundSpawnPoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACrowd_SpawnPoint::StaticClass(), FoundSpawnPoints);
    
    for (AActor* Actor : FoundSpawnPoints)
    {
        if (ACrowd_SpawnPoint* SpawnPoint = Cast<ACrowd_SpawnPoint>(Actor))
        {
            SpawnPoints.Add(SpawnPoint);
            UE_LOG(LogTemp, Log, TEXT("Found spawn point: %s"), *SpawnPoint->GetName());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Manager initialized with %d spawn points"), SpawnPoints.Num());
}

void UCrowd_MassEntityManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bSimulationEnabled)
        return;
    
    // Update LOD levels periodically
    TimeSinceLastLODUpdate += DeltaTime;
    if (TimeSinceLastLODUpdate >= LODUpdateFrequency)
    {
        // Get camera location for LOD calculations
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            FVector CameraLocation = PlayerPawn->GetActorLocation();
            UpdateLODLevels(CameraLocation);
            LastCameraLocation = CameraLocation;
        }
        
        TimeSinceLastLODUpdate = 0.0f;
    }
    
    // Optimize performance if needed
    OptimizePerformance();
    
    // Update flow fields for pathfinding
    if (PathfindingConfig.bUseFlowFields)
    {
        UpdateFlowFields();
    }
    
    // Debug visualization
    if (bShowDebugInfo)
    {
        FString DebugText = FString::Printf(TEXT("Active Agents: %d/%d"), ActiveAgentCount, MaxAllowedAgents);
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(1, 0.1f, FColor::Green, DebugText);
        }
    }
}

void UCrowd_MassEntityManager::InitializeMassSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Mass Entity System for Crowd Simulation"));
    
    // Set up performance limits based on hardware
    MaxAllowedAgents = PerformanceSettings.MaxAgents;
    
    // Initialize agent pools and memory allocation
    ManagedAgents.Reserve(MaxAllowedAgents / 10); // Reserve space for component references
    
    UE_LOG(LogTemp, Log, TEXT("Mass Entity System initialized - Max Agents: %d"), MaxAllowedAgents);
}

void UCrowd_MassEntityManager::SpawnCrowdAgents(FVector SpawnLocation, const FCrowd_SpawnConfig& SpawnConfig)
{
    if (!bSimulationEnabled)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn agents - simulation disabled"));
        return;
    }
    
    int32 AgentsToSpawn = FMath::Min(SpawnConfig.AgentCount, MaxAllowedAgents - ActiveAgentCount);
    if (AgentsToSpawn <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn agents - limit reached (%d/%d)"), ActiveAgentCount, MaxAllowedAgents);
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawning %d crowd agents at location %s"), AgentsToSpawn, *SpawnLocation.ToString());
    
    for (int32 i = 0; i < AgentsToSpawn; i++)
    {
        // Calculate spawn position based on formation type
        FVector AgentSpawnLocation = SpawnLocation;
        
        switch (SpawnConfig.FormationType)
        {
            case ECrowd_FormationType::Circle:
            {
                float Angle = (2.0f * PI * i) / AgentsToSpawn;
                float Radius = SpawnConfig.SpawnRadius * FMath::Sqrt(FMath::RandRange(0.1f, 1.0f));
                AgentSpawnLocation.X += Radius * FMath::Cos(Angle);
                AgentSpawnLocation.Y += Radius * FMath::Sin(Angle);
                break;
            }
            case ECrowd_FormationType::Line:
            {
                float LinePosition = (i - AgentsToSpawn * 0.5f) * 100.0f;
                AgentSpawnLocation.Y += LinePosition;
                break;
            }
            case ECrowd_FormationType::Scatter:
            default:
            {
                FVector RandomOffset = FVector(
                    FMath::RandRange(-SpawnConfig.SpawnRadius, SpawnConfig.SpawnRadius),
                    FMath::RandRange(-SpawnConfig.SpawnRadius, SpawnConfig.SpawnRadius),
                    0.0f
                );
                AgentSpawnLocation += RandomOffset;
                break;
            }
        }
        
        // Spawn actor with crowd agent component
        AActor* AgentActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), AgentSpawnLocation, FRotator::ZeroRotator);
        if (AgentActor)
        {
            UCrowd_AgentComponent* AgentComponent = NewObject<UCrowd_AgentComponent>(AgentActor);
            AgentActor->AddInstanceComponent(AgentComponent);
            AgentComponent->RegisterComponent();
            
            // Configure agent initial state
            AgentComponent->SetAgentState(SpawnConfig.InitialState);
            AgentComponent->SetPathfindingConfig(PathfindingConfig);
            
            ManagedAgents.Add(AgentComponent);
            ActiveAgentCount++;
            
            AgentActor->SetActorLabel(FString::Printf(TEXT("CrowdAgent_%d"), ActiveAgentCount));
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Successfully spawned %d agents. Total active: %d"), AgentsToSpawn, ActiveAgentCount);
}

void UCrowd_MassEntityManager::UpdateCrowdDensity(FVector PlayerLocation, float Radius)
{
    if (!bSimulationEnabled)
        return;
    
    // Calculate desired density based on distance from player
    ECrowd_DensityLevel DesiredDensity = ECrowd_DensityLevel::Medium;
    
    // Higher density near player for immersion
    if (Radius < 500.0f)
        DesiredDensity = ECrowd_DensityLevel::High;
    else if (Radius < 1000.0f)
        DesiredDensity = ECrowd_DensityLevel::Medium;
    else
        DesiredDensity = ECrowd_DensityLevel::Low;
    
    // Adjust agent count based on density
    int32 TargetAgentCount = 0;
    switch (DesiredDensity)
    {
        case ECrowd_DensityLevel::Low:
            TargetAgentCount = 500;
            break;
        case ECrowd_DensityLevel::Medium:
            TargetAgentCount = 2000;
            break;
        case ECrowd_DensityLevel::High:
            TargetAgentCount = 10000;
            break;
        case ECrowd_DensityLevel::Extreme:
            TargetAgentCount = MaxAllowedAgents;
            break;
        default:
            TargetAgentCount = 0;
            break;
    }
    
    // Spawn or despawn agents to reach target count
    if (ActiveAgentCount < TargetAgentCount && SpawnPoints.Num() > 0)
    {
        // Find closest spawn point
        ACrowd_SpawnPoint* ClosestSpawnPoint = nullptr;
        float ClosestDistance = FLT_MAX;
        
        for (ACrowd_SpawnPoint* SpawnPoint : SpawnPoints)
        {
            float Distance = FVector::Dist(PlayerLocation, SpawnPoint->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestSpawnPoint = SpawnPoint;
            }
        }
        
        if (ClosestSpawnPoint)
        {
            FCrowd_SpawnConfig SpawnConfig;
            SpawnConfig.AgentCount = FMath::Min(100, TargetAgentCount - ActiveAgentCount);
            SpawnCrowdAgents(ClosestSpawnPoint->GetActorLocation(), SpawnConfig);
        }
    }
}

void UCrowd_MassEntityManager::SetPerformanceSettings(const FCrowd_PerformanceSettings& NewSettings)
{
    PerformanceSettings = NewSettings;
    MaxAllowedAgents = PerformanceSettings.MaxAgents;
    LODUpdateFrequency = 1.0f / PerformanceSettings.DistantUpdateRate;
    
    UE_LOG(LogTemp, Log, TEXT("Updated performance settings - Max Agents: %d"), MaxAllowedAgents);
}

void UCrowd_MassEntityManager::SetCrowdSimulationEnabled(bool bEnabled)
{
    bSimulationEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Crowd simulation %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UCrowd_MassEntityManager::ClearAllAgents()
{
    UE_LOG(LogTemp, Log, TEXT("Clearing all crowd agents (%d)"), ActiveAgentCount);
    
    for (UCrowd_AgentComponent* Agent : ManagedAgents)
    {
        if (IsValid(Agent) && IsValid(Agent->GetOwner()))
        {
            Agent->GetOwner()->Destroy();
        }
    }
    
    ManagedAgents.Empty();
    ActiveAgentCount = 0;
}

void UCrowd_MassEntityManager::UpdateLODLevels(FVector CameraLocation)
{
    for (UCrowd_AgentComponent* Agent : ManagedAgents)
    {
        if (IsValid(Agent) && IsValid(Agent->GetOwner()))
        {
            float Distance = FVector::Dist(CameraLocation, Agent->GetOwner()->GetActorLocation());
            UpdateAgentLOD(Agent, Distance);
        }
    }
}

void UCrowd_MassEntityManager::UpdateAgentLOD(UCrowd_AgentComponent* Agent, float DistanceToCamera)
{
    if (!IsValid(Agent))
        return;
    
    ECrowd_LODLevel NewLODLevel = CalculateLODLevel(DistanceToCamera);
    Agent->SetLODLevel(NewLODLevel);
}

void UCrowd_MassEntityManager::OptimizePerformance()
{
    // Remove invalid agents
    ManagedAgents.RemoveAll([this](UCrowd_AgentComponent* Agent) {
        if (!IsValid(Agent) || !IsValid(Agent->GetOwner()))
        {
            ActiveAgentCount--;
            return true;
        }
        return false;
    });
    
    // Enforce agent limits
    if (ActiveAgentCount > MaxAllowedAgents)
    {
        int32 AgentsToRemove = ActiveAgentCount - MaxAllowedAgents;
        UE_LOG(LogTemp, Warning, TEXT("Removing %d agents to maintain performance"), AgentsToRemove);
        
        // Remove furthest agents first
        // This would require sorting by distance, simplified for now
        for (int32 i = 0; i < AgentsToRemove && ManagedAgents.Num() > 0; i++)
        {
            UCrowd_AgentComponent* Agent = ManagedAgents.Last();
            if (IsValid(Agent) && IsValid(Agent->GetOwner()))
            {
                Agent->GetOwner()->Destroy();
            }
            ManagedAgents.RemoveAt(ManagedAgents.Num() - 1);
            ActiveAgentCount--;
        }
    }
}

void UCrowd_MassEntityManager::UpdateFlowFields()
{
    // Flow field pathfinding update
    // This would integrate with UE5's navigation system
    // Simplified implementation for now
    
    if (ManagedAgents.Num() > 0)
    {
        // Update flow field data for efficient pathfinding
        // This would be GPU-accelerated in a full implementation
    }
}

ECrowd_LODLevel UCrowd_MassEntityManager::CalculateLODLevel(float Distance) const
{
    if (Distance <= PerformanceSettings.LOD0Distance)
        return ECrowd_LODLevel::FullDetail;
    else if (Distance <= PerformanceSettings.LOD1Distance)
        return ECrowd_LODLevel::Medium;
    else if (Distance <= PerformanceSettings.LOD2Distance)
        return ECrowd_LODLevel::Low;
    else if (Distance <= PerformanceSettings.LOD3Distance)
        return ECrowd_LODLevel::Impostor;
    else
        return ECrowd_LODLevel::Hidden;
}
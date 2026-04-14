#include "CrowdSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "DrawDebugHelpers.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // 30 FPS for crowd updates

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);

    // Initialize state
    CurrentState = ECrowd_SimulationState::Inactive;
    LastUpdateTime = 0.0f;
    AccumulatedDeltaTime = 0.0f;

    // Set default simulation settings
    SimulationSettings = FCrowd_SimulationSettings();
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: BeginPlay - Initializing crowd simulation system"));

    // Load visualization mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMesh.Succeeded() && VisualizationMesh)
    {
        VisualizationMesh->SetStaticMesh(SphereMesh.Object);
        VisualizationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }

    InitializeSimulation();
}

void ACrowdSimulationManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupSimulation();
    Super::EndPlay(EndPlayReason);
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ECrowd_SimulationState::Active)
    {
        AccumulatedDeltaTime += DeltaTime;
        
        // Update at specified frequency
        if (AccumulatedDeltaTime >= (1.0f / SimulationSettings.UpdateFrequency))
        {
            UpdateSimulation(AccumulatedDeltaTime);
            AccumulatedDeltaTime = 0.0f;
        }

        UpdatePerformanceStats();
    }
}

void ACrowdSimulationManager::StartSimulation()
{
    if (CurrentState == ECrowd_SimulationState::Inactive || CurrentState == ECrowd_SimulationState::Paused)
    {
        CurrentState = ECrowd_SimulationState::Initializing;
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Starting crowd simulation"));
        
        InitializeSimulation();
        CurrentState = ECrowd_SimulationState::Active;
    }
}

void ACrowdSimulationManager::StopSimulation()
{
    if (CurrentState != ECrowd_SimulationState::Inactive)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Stopping crowd simulation"));
        CurrentState = ECrowd_SimulationState::Cleanup;
        CleanupSimulation();
        CurrentState = ECrowd_SimulationState::Inactive;
    }
}

void ACrowdSimulationManager::PauseSimulation()
{
    if (CurrentState == ECrowd_SimulationState::Active)
    {
        CurrentState = ECrowd_SimulationState::Paused;
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Simulation paused"));
    }
}

void ACrowdSimulationManager::ResumeSimulation()
{
    if (CurrentState == ECrowd_SimulationState::Paused)
    {
        CurrentState = ECrowd_SimulationState::Active;
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Simulation resumed"));
    }
}

void ACrowdSimulationManager::SpawnCrowdAgents(int32 Count, FVector SpawnCenter, float SpawnRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Spawning %d crowd agents at %s"), Count, *SpawnCenter.ToString());

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("CrowdSimulationManager: No valid world for spawning agents"));
        return;
    }

    // Clamp count to max agents
    Count = FMath::Min(Count, SimulationSettings.MaxAgents - ActiveAgents.Num());

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomSpawnLocation(SpawnCenter, SpawnRadius);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            // For now, spawn basic actors as placeholders for Mass Entity agents
            AActor* AgentActor = World->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
            if (AgentActor)
            {
                AgentActor->SetActorLabel(FString::Printf(TEXT("CrowdAgent_%d"), i));
                
                // Add basic visualization
                UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(AgentActor);
                if (MeshComp)
                {
                    AgentActor->SetRootComponent(MeshComp);
                    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
                    if (CubeMesh.Succeeded())
                    {
                        MeshComp->SetStaticMesh(CubeMesh.Object);
                        MeshComp->SetWorldScale3D(FVector(0.3f, 0.3f, 0.3f));
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Successfully spawned %d agents"), Count);
}

void ACrowdSimulationManager::DespawnAllAgents()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Despawning all crowd agents"));

    for (UCrowd_AgentComponent* Agent : ActiveAgents)
    {
        if (Agent && Agent->GetOwner())
        {
            Agent->GetOwner()->Destroy();
        }
    }

    ActiveAgents.Empty();
    PerformanceStats.ActiveAgents = 0;
}

void ACrowdSimulationManager::SetCrowdDensity(ECrowd_DensityLevel DensityLevel)
{
    int32 TargetAgentCount = 0;

    switch (DensityLevel)
    {
        case ECrowd_DensityLevel::Sparse:
            TargetAgentCount = 10;
            break;
        case ECrowd_DensityLevel::Light:
            TargetAgentCount = 50;
            break;
        case ECrowd_DensityLevel::Medium:
            TargetAgentCount = 200;
            break;
        case ECrowd_DensityLevel::Dense:
            TargetAgentCount = 1000;
            break;
        case ECrowd_DensityLevel::VeryDense:
            TargetAgentCount = 5000;
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Setting crowd density to %d agents"), TargetAgentCount);

    // Adjust current agent count
    int32 CurrentAgentCount = ActiveAgents.Num();
    if (TargetAgentCount > CurrentAgentCount)
    {
        SpawnCrowdAgents(TargetAgentCount - CurrentAgentCount, GetActorLocation(), SimulationSettings.SpawnRadius);
    }
    else if (TargetAgentCount < CurrentAgentCount)
    {
        // Remove excess agents
        int32 AgentsToRemove = CurrentAgentCount - TargetAgentCount;
        for (int32 i = 0; i < AgentsToRemove && ActiveAgents.Num() > 0; i++)
        {
            if (ActiveAgents.Last() && ActiveAgents.Last()->GetOwner())
            {
                ActiveAgents.Last()->GetOwner()->Destroy();
            }
            ActiveAgents.RemoveAt(ActiveAgents.Num() - 1);
        }
    }
}

FCrowd_PerformanceStats ACrowdSimulationManager::GetPerformanceStats() const
{
    return PerformanceStats;
}

void ACrowdSimulationManager::UpdateLODSystem()
{
    if (!SimulationSettings.bEnableLODSystem)
        return;

    ProcessAgentLOD();
}

void ACrowdSimulationManager::RegisterMovementZone(ACrowd_MovementZone* Zone)
{
    if (Zone && !MovementZones.Contains(Zone))
    {
        MovementZones.Add(Zone);
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Registered movement zone %s"), *Zone->GetName());
    }
}

void ACrowdSimulationManager::UnregisterMovementZone(ACrowd_MovementZone* Zone)
{
    if (Zone)
    {
        MovementZones.Remove(Zone);
        UE_LOG(LogTemp, Log, TEXT("CrowdSimulationManager: Unregistered movement zone %s"), *Zone->GetName());
    }
}

void ACrowdSimulationManager::SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings)
{
    SimulationSettings = NewSettings;
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Updated simulation settings - MaxAgents: %d"), NewSettings.MaxAgents);
}

void ACrowdSimulationManager::DebugSpawnTestCrowd()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Debug spawning test crowd"));
    SpawnCrowdAgents(50, GetActorLocation(), 500.0f);
}

void ACrowdSimulationManager::DebugClearAllCrowds()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Debug clearing all crowds"));
    DespawnAllAgents();
}

void ACrowdSimulationManager::SetDebugVisualization(bool bEnabled)
{
    if (VisualizationMesh)
    {
        VisualizationMesh->SetVisibility(bEnabled);
    }
}

void ACrowdSimulationManager::InitializeSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Initializing simulation system"));

    // Clear any existing agents
    ActiveAgents.Empty();

    // Reset performance stats
    PerformanceStats = FCrowd_PerformanceStats();

    // Initialize navigation system
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        NavSys->Build();
        UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Navigation system initialized"));
    }

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void ACrowdSimulationManager::UpdateSimulation(float DeltaTime)
{
    // Update agent behaviors, pathfinding, and LOD
    UpdateLODSystem();

    // Process movement zones
    for (ACrowd_MovementZone* Zone : MovementZones)
    {
        if (Zone)
        {
            // Zone-specific crowd behavior updates would go here
        }
    }

    LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void ACrowdSimulationManager::CleanupSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager: Cleaning up simulation"));

    DespawnAllAgents();
    MovementZones.Empty();
}

void ACrowdSimulationManager::UpdatePerformanceStats()
{
    PerformanceStats.ActiveAgents = ActiveAgents.Num();
    PerformanceStats.FrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds

    // Count LOD levels
    PerformanceStats.HighLODAgents = 0;
    PerformanceStats.MediumLODAgents = 0;
    PerformanceStats.LowLODAgents = 0;

    // This would be implemented when we have actual agent components
    // For now, distribute evenly for testing
    int32 TotalAgents = PerformanceStats.ActiveAgents;
    PerformanceStats.HighLODAgents = TotalAgents / 3;
    PerformanceStats.MediumLODAgents = TotalAgents / 3;
    PerformanceStats.LowLODAgents = TotalAgents - PerformanceStats.HighLODAgents - PerformanceStats.MediumLODAgents;
}

void ACrowdSimulationManager::ProcessAgentLOD()
{
    // Get player camera location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Process each agent for LOD
    for (UCrowd_AgentComponent* Agent : ActiveAgents)
    {
        if (!Agent || !Agent->GetOwner())
            continue;

        float Distance = FVector::Dist(Agent->GetOwner()->GetActorLocation(), PlayerLocation);

        // Determine LOD level based on distance
        if (Distance <= SimulationSettings.LODDistanceHigh)
        {
            // High detail LOD
        }
        else if (Distance <= SimulationSettings.LODDistanceMedium)
        {
            // Medium detail LOD
        }
        else if (Distance <= SimulationSettings.LODDistanceLow)
        {
            // Low detail LOD
        }
        else
        {
            // Cull or use impostor
        }
    }
}

FVector ACrowdSimulationManager::GetRandomSpawnLocation(FVector Center, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomRadius = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset;
    RandomOffset.X = RandomRadius * FMath::Cos(RandomAngle);
    RandomOffset.Y = RandomRadius * FMath::Sin(RandomAngle);
    RandomOffset.Z = 0.0f;

    return Center + RandomOffset;
}

bool ACrowdSimulationManager::IsValidSpawnLocation(FVector Location)
{
    // Check if location is on navigation mesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        return NavSys->ProjectPointToNavigation(Location, NavLocation);
    }

    return true; // Default to valid if no nav system
}
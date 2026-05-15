#include "Crowd_MassEntityManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "Components/StaticMeshComponent.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for crowd updates
    
    // Initialize default settings
    SpawnSettings.MaxEntities = 1000;
    SpawnSettings.SpawnRadius = 2000.0f;
    SpawnSettings.MinSpawnDistance = 50.0f;
    SpawnSettings.SpawnCenter = FVector::ZeroVector;
    SpawnSettings.bSpawnOnNavMesh = true;
    
    MovementSettings.BaseMovementSpeed = 300.0f;
    MovementSettings.MaxMovementSpeed = 600.0f;
    MovementSettings.AvoidanceRadius = 100.0f;
    MovementSettings.WanderRadius = 500.0f;
    MovementSettings.FleeDistance = 1000.0f;
    MovementSettings.bEnableFlocking = true;
    
    LODSettings.HighDetailDistance = 500.0f;
    LODSettings.MediumDetailDistance = 1500.0f;
    LODSettings.LowDetailDistance = 3000.0f;
    LODSettings.CullDistance = 5000.0f;
    LODSettings.MaxHighDetailEntities = 50;
    LODSettings.MaxMediumDetailEntities = 200;
    
    // State initialization
    bCrowdSystemInitialized = false;
    ActiveEntityCount = 0;
    CurrentCrowdTarget = FVector::ZeroVector;
    bDebugVisualization = false;
    
    // Performance tracking
    LastLODUpdateTime = 0.0f;
    LODUpdateInterval = 0.5f; // Update LOD twice per second
    HighDetailEntityCount = 0;
    MediumDetailEntityCount = 0;
    LowDetailEntityCount = 0;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager::BeginPlay - Initializing crowd system"));
    
    // Initialize Mass Entity System
    InitializeCrowdSystem();
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clean up all entities before destruction
    DespawnAllEntities();
    
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bCrowdSystemInitialized || !MassEntitySubsystem)
    {
        return;
    }
    
    // Update crowd movement and behavior
    UpdateCrowdMovement(DeltaTime);
    
    // Update LOD system
    UpdateEntityLOD(DeltaTime);
    
    // Process flee responses
    ProcessFleeResponse(DeltaTime);
    
    // Draw debug visualization if enabled
    if (bDebugVisualization)
    {
        DrawDebugVisualization();
    }
}

void ACrowd_MassEntityManager::InitializeCrowdSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager::InitializeCrowdSystem - No valid world"));
        return;
    }
    
    // Get Mass Entity Subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("ACrowd_MassEntityManager::InitializeCrowdSystem - Failed to get MassEntitySubsystem"));
        return;
    }
    
    // Initialize Mass Entity System
    InitializeMassEntitySystem();
    
    // Create crowd archetype
    CreateCrowdArchetype();
    
    bCrowdSystemInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("ACrowd_MassEntityManager::InitializeCrowdSystem - Crowd system initialized successfully"));
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 EntityCount, FVector SpawnLocation)
{
    if (!bCrowdSystemInitialized || !MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("SpawnCrowdEntities - Crowd system not initialized"));
        return;
    }
    
    // Clamp entity count to maximum
    EntityCount = FMath::Min(EntityCount, SpawnSettings.MaxEntities - ActiveEntityCount);
    
    UE_LOG(LogTemp, Warning, TEXT("SpawnCrowdEntities - Spawning %d entities at location %s"), EntityCount, *SpawnLocation.ToString());
    
    // Spawn entities in a circular pattern
    for (int32 i = 0; i < EntityCount; i++)
    {
        // Calculate spawn position
        float Angle = (2.0f * PI * i) / EntityCount;
        float Distance = FMath::RandRange(SpawnSettings.MinSpawnDistance, SpawnSettings.SpawnRadius);
        
        FVector EntitySpawnLocation = SpawnLocation + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Adjust for NavMesh if enabled
        if (SpawnSettings.bSpawnOnNavMesh)
        {
            UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
            if (NavSystem)
            {
                FNavLocation NavLocation;
                if (NavSystem->ProjectPointToNavigation(EntitySpawnLocation, NavLocation, FVector(200.0f, 200.0f, 500.0f)))
                {
                    EntitySpawnLocation = NavLocation.Location;
                }
            }
        }
        
        // Create entity through Mass Entity System
        FMassEntityHandle EntityHandle = MassEntitySubsystem->CreateEntity();
        if (EntityHandle.IsValid())
        {
            CrowdEntities.Add(EntityHandle);
            ActiveEntityCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("SpawnCrowdEntities - Successfully spawned %d entities. Total active: %d"), EntityCount, ActiveEntityCount);
}

void ACrowd_MassEntityManager::DespawnAllEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("DespawnAllEntities - Removing %d entities"), CrowdEntities.Num());
    
    // Destroy all entities
    for (const FMassEntityHandle& EntityHandle : CrowdEntities)
    {
        if (EntityHandle.IsValid())
        {
            MassEntitySubsystem->DestroyEntity(EntityHandle);
        }
    }
    
    CrowdEntities.Empty();
    ActiveEntityCount = 0;
}

void ACrowd_MassEntityManager::SetCrowdTarget(FVector TargetLocation)
{
    CurrentCrowdTarget = TargetLocation;
    UE_LOG(LogTemp, Warning, TEXT("SetCrowdTarget - New target location: %s"), *TargetLocation.ToString());
}

void ACrowd_MassEntityManager::TriggerFleeResponse(FVector ThreatLocation, float ThreatRadius)
{
    if (!bCrowdSystemInitialized)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("TriggerFleeResponse - Threat at %s with radius %f"), *ThreatLocation.ToString(), ThreatRadius);
    
    // Calculate flee direction for each entity
    for (const FMassEntityHandle& EntityHandle : CrowdEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        // This would normally update entity fragments with flee behavior
        // For now, we log the flee response
    }
}

void ACrowd_MassEntityManager::UpdateEntityLOD(float DeltaTime)
{
    LastLODUpdateTime += DeltaTime;
    
    if (LastLODUpdateTime < LODUpdateInterval)
    {
        return;
    }
    
    LastLODUpdateTime = 0.0f;
    
    // Reset LOD counters
    HighDetailEntityCount = 0;
    MediumDetailEntityCount = 0;
    LowDetailEntityCount = 0;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for each entity based on distance to player
    for (const FMassEntityHandle& EntityHandle : CrowdEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        // This would normally calculate distance and update LOD fragments
        // For now, we distribute entities across LOD levels
        float RandomDistance = FMath::RandRange(0.0f, LODSettings.CullDistance);
        
        if (RandomDistance < LODSettings.HighDetailDistance && HighDetailEntityCount < LODSettings.MaxHighDetailEntities)
        {
            HighDetailEntityCount++;
        }
        else if (RandomDistance < LODSettings.MediumDetailDistance && MediumDetailEntityCount < LODSettings.MaxMediumDetailEntities)
        {
            MediumDetailEntityCount++;
        }
        else if (RandomDistance < LODSettings.LowDetailDistance)
        {
            LowDetailEntityCount++;
        }
    }
}

void ACrowd_MassEntityManager::SetLODSettings(const FCrowd_LODSettings& NewLODSettings)
{
    LODSettings = NewLODSettings;
    UE_LOG(LogTemp, Warning, TEXT("SetLODSettings - Updated LOD configuration"));
}

void ACrowd_MassEntityManager::SetMovementSettings(const FCrowd_MovementSettings& NewMovementSettings)
{
    MovementSettings = NewMovementSettings;
    UE_LOG(LogTemp, Warning, TEXT("SetMovementSettings - Updated movement configuration"));
}

void ACrowd_MassEntityManager::AddCrowdWaypoint(FVector WaypointLocation)
{
    CrowdWaypoints.Add(WaypointLocation);
    UE_LOG(LogTemp, Warning, TEXT("AddCrowdWaypoint - Added waypoint at %s. Total waypoints: %d"), *WaypointLocation.ToString(), CrowdWaypoints.Num());
}

void ACrowd_MassEntityManager::ClearCrowdWaypoints()
{
    CrowdWaypoints.Empty();
    UE_LOG(LogTemp, Warning, TEXT("ClearCrowdWaypoints - Cleared all waypoints"));
}

int32 ACrowd_MassEntityManager::GetActiveEntityCount() const
{
    return ActiveEntityCount;
}

void ACrowd_MassEntityManager::ToggleDebugVisualization()
{
    bDebugVisualization = !bDebugVisualization;
    UE_LOG(LogTemp, Warning, TEXT("ToggleDebugVisualization - Debug visualization %s"), bDebugVisualization ? TEXT("enabled") : TEXT("disabled"));
}

void ACrowd_MassEntityManager::TestSpawnCrowd()
{
    if (!bCrowdSystemInitialized)
    {
        InitializeCrowdSystem();
    }
    
    FVector SpawnLocation = GetActorLocation();
    SpawnCrowdEntities(100, SpawnLocation);
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("InitializeMassEntitySystem - Setting up Mass Entity framework"));
    
    // Mass Entity System is initialized by the subsystem
    // Additional configuration would go here
}

void ACrowd_MassEntityManager::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CreateCrowdArchetype - Creating crowd entity archetype"));
    
    // This would normally create a Mass Entity archetype with required fragments
    // For now, we log the archetype creation
}

void ACrowd_MassEntityManager::UpdateCrowdMovement(float DeltaTime)
{
    if (CrowdEntities.Num() == 0)
    {
        return;
    }
    
    // Update movement for all entities
    for (const FMassEntityHandle& EntityHandle : CrowdEntities)
    {
        if (!EntityHandle.IsValid())
        {
            continue;
        }
        
        // This would normally update movement fragments
        // For now, we simulate movement updates
    }
}

void ACrowd_MassEntityManager::ProcessFleeResponse(float DeltaTime)
{
    // Process any active flee responses
    // This would normally update entity behavior based on threats
}

void ACrowd_MassEntityManager::DrawDebugVisualization()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Draw spawn area
    DrawDebugSphere(GetWorld(), SpawnSettings.SpawnCenter, SpawnSettings.SpawnRadius, 32, FColor::Green, false, -1.0f, 0, 5.0f);
    
    // Draw waypoints
    for (int32 i = 0; i < CrowdWaypoints.Num(); i++)
    {
        DrawDebugSphere(GetWorld(), CrowdWaypoints[i], 50.0f, 16, FColor::Blue, false, -1.0f, 0, 3.0f);
        
        if (i > 0)
        {
            DrawDebugLine(GetWorld(), CrowdWaypoints[i-1], CrowdWaypoints[i], FColor::Blue, false, -1.0f, 0, 2.0f);
        }
    }
    
    // Draw current target
    if (CurrentCrowdTarget != FVector::ZeroVector)
    {
        DrawDebugSphere(GetWorld(), CurrentCrowdTarget, 100.0f, 16, FColor::Red, false, -1.0f, 0, 5.0f);
    }
    
    // Draw LOD information
    FVector DebugLocation = GetActorLocation() + FVector(0, 0, 200);
    DrawDebugString(GetWorld(), DebugLocation, 
        FString::Printf(TEXT("Crowd Entities: %d\nHigh LOD: %d\nMedium LOD: %d\nLow LOD: %d"), 
        ActiveEntityCount, HighDetailEntityCount, MediumDetailEntityCount, LowDetailEntityCount),
        nullptr, FColor::White, 0.0f);
}
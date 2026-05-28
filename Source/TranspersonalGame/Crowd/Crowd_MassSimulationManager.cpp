#include "Crowd_MassSimulationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "DrawDebugHelpers.h"

ACrowd_MassSimulationManager::ACrowd_MassSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.033f; // 30 FPS update

    // Initialize default settings
    SimulationSettings.MaxEntities = 50000;
    SimulationSettings.SpawnRadius = 10000.0f;
    SimulationSettings.UpdateFrequency = 30.0f;
    SimulationSettings.bEnableLODSystem = true;
    SimulationSettings.LODDistance1 = 1000.0f;
    SimulationSettings.LODDistance2 = 5000.0f;
    SimulationSettings.LODDistance3 = 15000.0f;

    CurrentEntityCount = 0;
    bSimulationInitialized = false;
    EntityManager = nullptr;
}

void ACrowd_MassSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd Mass Simulation Manager - BeginPlay"));
    
    // Initialize the simulation system
    InitializeMassSimulation();
}

void ACrowd_MassSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSimulationInitialized)
    {
        return;
    }

    // Update simulation at specified frequency
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= (1.0f / SimulationSettings.UpdateFrequency))
    {
        UpdateEntityMovement(DeltaTime);
        
        if (SimulationSettings.bEnableLODSystem)
        {
            ProcessLODSystem();
        }
        
        LastUpdateTime = 0.0f;
    }
}

void ACrowd_MassSimulationManager::InitializeMassSimulation()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Mass Simulation System"));
    
    // Clear existing entities
    ActiveEntities.Empty();
    CurrentEntityCount = 0;
    
    // Initialize entity manager
    UWorld* World = GetWorld();
    if (World)
    {
        // Reserve memory for maximum entities
        ActiveEntities.Reserve(SimulationSettings.MaxEntities);
        bSimulationInitialized = true;
        
        UE_LOG(LogTemp, Warning, TEXT("Mass Simulation initialized - Max Entities: %d"), SimulationSettings.MaxEntities);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to initialize Mass Simulation - No valid world"));
    }
}

void ACrowd_MassSimulationManager::SpawnEntities(int32 Count, FVector CenterLocation)
{
    if (!bSimulationInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn entities - Simulation not initialized"));
        return;
    }

    // Clamp spawn count to available slots
    int32 AvailableSlots = SimulationSettings.MaxEntities - CurrentEntityCount;
    int32 ActualSpawnCount = FMath::Min(Count, AvailableSlots);
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning %d entities at location: %s"), ActualSpawnCount, *CenterLocation.ToString());

    for (int32 i = 0; i < ActualSpawnCount; i++)
    {
        FCrowd_EntityData NewEntity;
        
        // Generate random spawn location
        FVector SpawnLocation = GetRandomSpawnLocation(CenterLocation, SimulationSettings.SpawnRadius);
        
        if (IsValidSpawnLocation(SpawnLocation))
        {
            NewEntity.Position = SpawnLocation;
            NewEntity.Velocity = FVector::ZeroVector;
            NewEntity.MovementSpeed = FMath::RandRange(200.0f, 400.0f);
            NewEntity.LODLevel = 0;
            NewEntity.bIsActive = true;
            
            ActiveEntities.Add(NewEntity);
            CurrentEntityCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Successfully spawned %d entities. Total active: %d"), ActualSpawnCount, CurrentEntityCount);
}

void ACrowd_MassSimulationManager::UpdateEntityLOD()
{
    if (!bSimulationInitialized)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    for (FCrowd_EntityData& Entity : ActiveEntities)
    {
        if (!Entity.bIsActive)
        {
            continue;
        }

        float Distance = FVector::Dist(Entity.Position, PlayerLocation);
        
        // Determine LOD level based on distance
        if (Distance <= SimulationSettings.LODDistance1)
        {
            Entity.LODLevel = 0; // Highest detail
        }
        else if (Distance <= SimulationSettings.LODDistance2)
        {
            Entity.LODLevel = 1; // Medium detail
        }
        else if (Distance <= SimulationSettings.LODDistance3)
        {
            Entity.LODLevel = 2; // Low detail
        }
        else
        {
            Entity.LODLevel = 3; // Culled/Invisible
        }
    }
}

void ACrowd_MassSimulationManager::SetSimulationSettings(const FCrowd_SimulationSettings& NewSettings)
{
    SimulationSettings = NewSettings;
    
    // Adjust tick interval based on update frequency
    if (SimulationSettings.UpdateFrequency > 0.0f)
    {
        PrimaryActorTick.TickInterval = 1.0f / SimulationSettings.UpdateFrequency;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Simulation settings updated - Max Entities: %d, Update Freq: %.2f"), 
           SimulationSettings.MaxEntities, SimulationSettings.UpdateFrequency);
}

void ACrowd_MassSimulationManager::ClearAllEntities()
{
    ActiveEntities.Empty();
    CurrentEntityCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("All entities cleared from simulation"));
}

float ACrowd_MassSimulationManager::GetPerformanceMetric() const
{
    if (SimulationSettings.MaxEntities <= 0)
    {
        return 0.0f;
    }
    
    return (float)CurrentEntityCount / (float)SimulationSettings.MaxEntities;
}

void ACrowd_MassSimulationManager::UpdateEntityMovement(float DeltaTime)
{
    for (FCrowd_EntityData& Entity : ActiveEntities)
    {
        if (!Entity.bIsActive || Entity.LODLevel >= 3)
        {
            continue; // Skip inactive or culled entities
        }
        
        // Simple movement pattern - random walk
        FVector RandomDirection = FVector(
            FMath::RandRange(-1.0f, 1.0f),
            FMath::RandRange(-1.0f, 1.0f),
            0.0f
        ).GetSafeNormal();
        
        Entity.Velocity = RandomDirection * Entity.MovementSpeed;
        Entity.Position += Entity.Velocity * DeltaTime;
        
        // Keep entities within simulation bounds
        FVector ManagerLocation = GetActorLocation();
        float MaxDistance = SimulationSettings.SpawnRadius * 1.5f;
        
        if (FVector::Dist2D(Entity.Position, ManagerLocation) > MaxDistance)
        {
            FVector DirectionToCenter = (ManagerLocation - Entity.Position).GetSafeNormal();
            Entity.Velocity = DirectionToCenter * Entity.MovementSpeed;
        }
    }
}

void ACrowd_MassSimulationManager::ProcessLODSystem()
{
    UpdateEntityLOD();
    
    // Count entities by LOD level for performance monitoring
    int32 LODCounts[4] = {0, 0, 0, 0};
    
    for (const FCrowd_EntityData& Entity : ActiveEntities)
    {
        if (Entity.bIsActive && Entity.LODLevel >= 0 && Entity.LODLevel < 4)
        {
            LODCounts[Entity.LODLevel]++;
        }
    }
    
    // Optional: Log LOD distribution for debugging
    static float LastLODLogTime = 0.0f;
    LastLODLogTime += GetWorld()->GetDeltaSeconds();
    
    if (LastLODLogTime >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("LOD Distribution - L0:%d L1:%d L2:%d L3:%d"), 
               LODCounts[0], LODCounts[1], LODCounts[2], LODCounts[3]);
        LastLODLogTime = 0.0f;
    }
}

FVector ACrowd_MassSimulationManager::GetRandomSpawnLocation(FVector Center, float Radius)
{
    // Generate random point in circle
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Radius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
    
    FVector SpawnLocation = Center + RandomOffset;
    
    // Trace to ground
    FHitResult HitResult;
    FVector TraceStart = SpawnLocation + FVector(0, 0, 1000.0f);
    FVector TraceEnd = SpawnLocation - FVector(0, 0, 1000.0f);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
    {
        SpawnLocation.Z = HitResult.Location.Z + 100.0f; // Offset above ground
    }
    
    return SpawnLocation;
}

bool ACrowd_MassSimulationManager::IsValidSpawnLocation(FVector Location)
{
    // Basic validation - check if location is within world bounds
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if location is too close to other important actors
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), NearbyActors);
    
    for (AActor* Actor : NearbyActors)
    {
        if (FVector::Dist(Location, Actor->GetActorLocation()) < 500.0f)
        {
            return false; // Too close to existing pawn
        }
    }
    
    return true;
}
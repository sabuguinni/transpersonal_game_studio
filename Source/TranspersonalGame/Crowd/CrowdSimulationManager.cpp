#include "CrowdSimulationManager.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

ACrowdSimulationManager::ACrowdSimulationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for management logic
    
    CurrentSeasonTime = 0.0f;
    ActiveEntityCount = 0;
    LastPerformanceCheck = 0.0f;
    AverageFrameTime = 16.67f; // Target 60 FPS
}

void ACrowdSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Get Mass subsystems
    UWorld* World = GetWorld();
    if (World)
    {
        MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = World->GetSubsystem<UMassSpawnerSubsystem>();
        MassSimulationSubsystem = World->GetSubsystem<UMassSimulationSubsystem>();
    }
    
    // Initialize migration waypoints if empty
    if (MigrationWaypoints.Num() == 0)
    {
        // Default circular migration pattern
        float Radius = 5000.0f;
        int32 WaypointCount = 8;
        FVector Center = GetActorLocation();
        
        for (int32 i = 0; i < WaypointCount; i++)
        {
            float Angle = (2.0f * PI * i) / WaypointCount;
            FVector Waypoint = Center + FVector(
                FMath::Cos(Angle) * Radius,
                FMath::Sin(Angle) * Radius,
                0.0f
            );
            MigrationWaypoints.Add(Waypoint);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdSimulationManager initialized with %d migration waypoints"), MigrationWaypoints.Num());
}

void ACrowdSimulationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update seasonal cycle
    CurrentSeasonTime += DeltaTime;
    if (CurrentSeasonTime >= SeasonalCycleLength)
    {
        CurrentSeasonTime = 0.0f;
        // Trigger seasonal migration
        SetMigrationActive(true);
    }
    
    // Performance monitoring
    LastPerformanceCheck += DeltaTime;
    if (LastPerformanceCheck >= 1.0f) // Check every second
    {
        LastPerformanceCheck = 0.0f;
        
        // Calculate average frame time
        AverageFrameTime = (AverageFrameTime * 0.9f) + (DeltaTime * 1000.0f * 0.1f);
        
        // Adaptive LOD based on performance
        if (AverageFrameTime > 20.0f) // Below 50 FPS
        {
            // Reduce simulation complexity
            MaxSimultaneousEntities = FMath::Max(MaxSimultaneousEntities - 1000, 10000);
            UE_LOG(LogTemp, Warning, TEXT("Performance issue detected. Reducing max entities to %d"), MaxSimultaneousEntities);
        }
        else if (AverageFrameTime < 14.0f) // Above 70 FPS
        {
            // Increase simulation complexity
            MaxSimultaneousEntities = FMath::Min(MaxSimultaneousEntities + 500, 50000);
        }
    }
    
    // Update active entity count from Mass subsystem
    if (MassEntitySubsystem)
    {
        ActiveEntityCount = MassEntitySubsystem->GetNumEntities();
    }
}

void ACrowdSimulationManager::SpawnHerd(FVector Location, int32 HerdSize, TSubclassOf<class ADinosaur> DinosaurClass)
{
    if (!MassSpawnerSubsystem || !DinosaurClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn herd: Missing subsystem or dinosaur class"));
        return;
    }
    
    // Check if we're at entity limit
    if (ActiveEntityCount + HerdSize > MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn herd: Would exceed entity limit (%d + %d > %d)"), 
               ActiveEntityCount, HerdSize, MaxSimultaneousEntities);
        return;
    }
    
    // Spawn entities in a natural formation
    float SpreadRadius = FMath::Sqrt(HerdSize) * 50.0f; // Spread based on herd size
    
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Random position within spread radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            FMath::RandRange(-SpreadRadius, SpreadRadius),
            0.0f
        );
        
        FVector SpawnLocation = Location + RandomOffset;
        
        // TODO: Use Mass spawning system here
        // This is a placeholder for the actual Mass Entity spawning
        UE_LOG(LogTemp, Log, TEXT("Spawning herd member %d at location %s"), i, *SpawnLocation.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned herd of %d entities at %s"), HerdSize, *Location.ToString());
}

void ACrowdSimulationManager::SpawnFlock(FVector Location, int32 FlockSize, TSubclassOf<class AFlyingDinosaur> FlyingDinosaurClass)
{
    if (!MassSpawnerSubsystem || !FlyingDinosaurClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn flock: Missing subsystem or flying dinosaur class"));
        return;
    }
    
    if (ActiveEntityCount + FlockSize > MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn flock: Would exceed entity limit"));
        return;
    }
    
    // Spawn flying entities in V-formation
    float FormationSpacing = 100.0f;
    FVector LeaderPosition = Location;
    
    for (int32 i = 0; i < FlockSize; i++)
    {
        FVector SpawnLocation;
        
        if (i == 0)
        {
            // Leader at front
            SpawnLocation = LeaderPosition;
        }
        else
        {
            // Formation based on index
            int32 Side = (i % 2 == 0) ? 1 : -1; // Alternate sides
            int32 Row = (i + 1) / 2;
            
            SpawnLocation = LeaderPosition + FVector(
                -Row * FormationSpacing * 0.8f, // Slightly behind
                Side * Row * FormationSpacing,   // To the side
                FMath::RandRange(-50.0f, 50.0f) // Small height variation
            );
        }
        
        // TODO: Use Mass spawning system for flying entities
        UE_LOG(LogTemp, Log, TEXT("Spawning flock member %d at location %s"), i, *SpawnLocation.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned flock of %d entities at %s"), FlockSize, *Location.ToString());
}

void ACrowdSimulationManager::SpawnPack(FVector Location, int32 PackSize, TSubclassOf<class APredatorDinosaur> PredatorClass)
{
    if (!MassSpawnerSubsystem || !PredatorClass)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot spawn pack: Missing subsystem or predator class"));
        return;
    }
    
    if (ActiveEntityCount + PackSize > MaxSimultaneousEntities)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn pack: Would exceed entity limit"));
        return;
    }
    
    // Spawn predators in hunting formation
    float PackRadius = PackSize * 25.0f; // Tighter formation for predators
    
    for (int32 i = 0; i < PackSize; i++)
    {
        float Angle = (2.0f * PI * i) / PackSize;
        FVector SpawnLocation = Location + FVector(
            FMath::Cos(Angle) * PackRadius,
            FMath::Sin(Angle) * PackRadius,
            0.0f
        );
        
        // TODO: Use Mass spawning system for predators
        UE_LOG(LogTemp, Log, TEXT("Spawning pack member %d at location %s"), i, *SpawnLocation.ToString());
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned pack of %d entities at %s"), PackSize, *Location.ToString());
}

void ACrowdSimulationManager::TriggerStampede(FVector ThreatLocation, float ThreatRadius)
{
    UE_LOG(LogTemp, Warning, TEXT("STAMPEDE TRIGGERED at %s with radius %f"), *ThreatLocation.ToString(), ThreatRadius);
    
    // TODO: Implement stampede behavior using Mass system
    // This would involve:
    // 1. Finding all herbivore entities within ThreatRadius
    // 2. Setting their behavior state to "Fleeing"
    // 3. Calculating flee direction away from ThreatLocation
    // 4. Increasing movement speed temporarily
    // 5. Adding panic spreading to nearby entities
    
    if (MassSimulationSubsystem)
    {
        // Placeholder for Mass system stampede logic
        UE_LOG(LogTemp, Warning, TEXT("Mass simulation system will handle stampede behavior"));
    }
}

void ACrowdSimulationManager::SetMigrationActive(bool bActive)
{
    UE_LOG(LogTemp, Warning, TEXT("Migration set to: %s"), bActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    
    // TODO: Implement migration behavior using Mass system
    // This would involve:
    // 1. Setting migration waypoints for all herd entities
    // 2. Changing movement patterns to follow migration routes
    // 3. Coordinating multiple herds to move together
    // 4. Handling seasonal timing and triggers
    
    if (bActive && MigrationWaypoints.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Starting migration with %d waypoints"), MigrationWaypoints.Num());
        // Set all herds to follow migration pattern
    }
}
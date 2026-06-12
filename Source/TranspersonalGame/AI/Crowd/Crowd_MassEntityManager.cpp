#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second for performance

    // Initialize default values
    MassEntitySubsystem = nullptr;
    MassSpawnerSubsystem = nullptr;
    CurrentSimulationFPS = 60.0f;
    LastFrameTime = 0.0f;
    ActiveEntityCount = 0;

    // Set default spawn configuration
    DefaultSpawnConfig.MaxEntities = 100;
    DefaultSpawnConfig.SpawnRadius = 1000.0f;
    DefaultSpawnConfig.SpawnRate = 5.0f;
    DefaultSpawnConfig.DefaultBehavior = ECrowd_BehaviorType::Wandering;

    // Set default LOD settings
    LODSettings.HighDetailDistance = 500.0f;
    LODSettings.MediumDetailDistance = 1500.0f;
    LODSettings.LowDetailDistance = 3000.0f;
    LODSettings.CullDistance = 5000.0f;

    // Performance settings
    PerformanceTargetFPS = 60.0f;
    bEnableAdaptiveLOD = true;
    bEnablePerformanceMonitoring = true;
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMassEntitySystem();
    
    UE_LOG(LogTemp, Log, TEXT("Crowd Mass Entity Manager initialized"));
}

void ACrowd_MassEntityManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupMassEntitySystem();
    Super::EndPlay(EndPlayReason);
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnablePerformanceMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
    }

    if (bEnableAdaptiveLOD)
    {
        AdaptLODBasedOnPerformance();
    }

    UpdateCrowdLOD();
}

void ACrowd_MassEntityManager::InitializeMassEntitySystem()
{
    if (!IsValid(GetWorld()))
    {
        UE_LOG(LogTemp, Error, TEXT("World is not valid for Mass Entity initialization"));
        return;
    }

    // Try to get Mass Entity subsystem
    UGameInstance* GameInstance = GetWorld()->GetGameInstance();
    if (IsValid(GameInstance))
    {
        // Note: Mass Entity subsystem may not be available in all UE5 builds
        // This is a placeholder for when Mass Entity plugin is enabled
        UE_LOG(LogTemp, Log, TEXT("Attempting to initialize Mass Entity subsystem..."));
        
        // For now, we'll simulate the system without actual Mass Entity dependency
        MassEntitySubsystem = nullptr; // Would be: GameInstance->GetSubsystem<UMassEntitySubsystem>();
        MassSpawnerSubsystem = nullptr; // Would be: GameInstance->GetSubsystem<UMassSpawnerSubsystem>();
        
        if (!IsMassEntitySystemAvailable())
        {
            UE_LOG(LogTemp, Warning, TEXT("Mass Entity system not available - using fallback crowd simulation"));
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("Mass Entity system initialized successfully"));
        }
    }
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(const FVector& Location, const FCrowd_EntitySpawnConfig& Config)
{
    if (!IsMassEntitySystemAvailable())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot spawn crowd entities - Mass Entity system not available"));
        return;
    }

    // Implementation would use Mass Entity spawning here
    // For now, we'll log the spawn request
    UE_LOG(LogTemp, Log, TEXT("Spawning %d crowd entities at location (%f, %f, %f)"), 
           Config.MaxEntities, Location.X, Location.Y, Location.Z);

    // Simulate entity spawning
    ActiveEntityCount += Config.MaxEntities;
    
    // Clamp to reasonable limits for performance
    if (ActiveEntityCount > 1000)
    {
        ActiveEntityCount = 1000;
        UE_LOG(LogTemp, Warning, TEXT("Crowd entity count clamped to 1000 for performance"));
    }
}

void ACrowd_MassEntityManager::DespawnCrowdEntities(const FVector& Location, float Radius)
{
    if (!IsMassEntitySystemAvailable())
    {
        return;
    }

    // Implementation would despawn entities within radius
    UE_LOG(LogTemp, Log, TEXT("Despawning crowd entities within %f units of (%f, %f, %f)"), 
           Radius, Location.X, Location.Y, Location.Z);

    // Simulate entity despawning
    int32 EntitiesToRemove = FMath::Min(50, ActiveEntityCount);
    ActiveEntityCount = FMath::Max(0, ActiveEntityCount - EntitiesToRemove);
}

void ACrowd_MassEntityManager::UpdateCrowdLOD()
{
    if (!IsValid(GetWorld()))
    {
        return;
    }

    // Get player location for LOD calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!IsValid(PlayerPawn))
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FVector ManagerLocation = GetActorLocation();
    float DistanceToPlayer = FVector::Dist(PlayerLocation, ManagerLocation);

    // Determine LOD level based on distance
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::High;
    
    if (DistanceToPlayer > LODSettings.CullDistance)
    {
        CurrentLOD = ECrowd_LODLevel::Culled;
    }
    else if (DistanceToPlayer > LODSettings.LowDetailDistance)
    {
        CurrentLOD = ECrowd_LODLevel::Low;
    }
    else if (DistanceToPlayer > LODSettings.MediumDetailDistance)
    {
        CurrentLOD = ECrowd_LODLevel::Medium;
    }
    else if (DistanceToPlayer > LODSettings.HighDetailDistance)
    {
        CurrentLOD = ECrowd_LODLevel::High;
    }

    // Apply LOD settings (would affect Mass Entity rendering/simulation detail)
    // For now, just log the LOD changes
    static ECrowd_LODLevel LastLOD = ECrowd_LODLevel::High;
    if (CurrentLOD != LastLOD)
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd LOD changed to level %d (distance: %f)"), 
               (int32)CurrentLOD, DistanceToPlayer);
        LastLOD = CurrentLOD;
    }
}

int32 ACrowd_MassEntityManager::GetActiveCrowdEntityCount() const
{
    return ActiveEntityCount;
}

void ACrowd_MassEntityManager::SetCrowdBehavior(ECrowd_BehaviorType NewBehavior, const FVector& TargetLocation)
{
    UE_LOG(LogTemp, Log, TEXT("Setting crowd behavior to %d at location (%f, %f, %f)"), 
           (int32)NewBehavior, TargetLocation.X, TargetLocation.Y, TargetLocation.Z);

    // Implementation would update Mass Entity behavior processors
    // This would affect how entities move and interact
}

void ACrowd_MassEntityManager::AddGatheringPoint(const FVector& Location, float Radius, ECrowd_BehaviorType BehaviorType)
{
    GatheringPoints.Add(Location);
    GatheringPointBehaviors.Add(BehaviorType);
    GatheringPointRadii.Add(Radius);

    UE_LOG(LogTemp, Log, TEXT("Added gathering point at (%f, %f, %f) with radius %f"), 
           Location.X, Location.Y, Location.Z, Radius);
}

void ACrowd_MassEntityManager::RemoveGatheringPoint(const FVector& Location)
{
    for (int32 i = GatheringPoints.Num() - 1; i >= 0; i--)
    {
        if (FVector::Dist(GatheringPoints[i], Location) < 100.0f)
        {
            GatheringPoints.RemoveAt(i);
            GatheringPointBehaviors.RemoveAt(i);
            GatheringPointRadii.RemoveAt(i);
            
            UE_LOG(LogTemp, Log, TEXT("Removed gathering point near (%f, %f, %f)"), 
                   Location.X, Location.Y, Location.Z);
            break;
        }
    }
}

float ACrowd_MassEntityManager::GetCrowdSimulationFPS() const
{
    return CurrentSimulationFPS;
}

void ACrowd_MassEntityManager::SetPerformanceTarget(float TargetFPS)
{
    PerformanceTargetFPS = FMath::Clamp(TargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Set crowd simulation performance target to %f FPS"), PerformanceTargetFPS);
}

void ACrowd_MassEntityManager::UpdatePerformanceMetrics(float DeltaTime)
{
    LastFrameTime = DeltaTime;
    CurrentSimulationFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 60.0f;

    // Log performance issues
    if (CurrentSimulationFPS < PerformanceTargetFPS * 0.8f)
    {
        static float LastWarningTime = 0.0f;
        float CurrentTime = GetWorld()->GetTimeSeconds();
        
        if (CurrentTime - LastWarningTime > 5.0f) // Log warning every 5 seconds max
        {
            UE_LOG(LogTemp, Warning, TEXT("Crowd simulation FPS below target: %f (target: %f)"), 
                   CurrentSimulationFPS, PerformanceTargetFPS);
            LastWarningTime = CurrentTime;
        }
    }
}

void ACrowd_MassEntityManager::AdaptLODBasedOnPerformance()
{
    if (CurrentSimulationFPS < PerformanceTargetFPS * 0.7f)
    {
        // Reduce LOD distances to improve performance
        LODSettings.HighDetailDistance *= 0.95f;
        LODSettings.MediumDetailDistance *= 0.95f;
        LODSettings.LowDetailDistance *= 0.95f;
        
        // Clamp to minimum values
        LODSettings.HighDetailDistance = FMath::Max(LODSettings.HighDetailDistance, 200.0f);
        LODSettings.MediumDetailDistance = FMath::Max(LODSettings.MediumDetailDistance, 500.0f);
        LODSettings.LowDetailDistance = FMath::Max(LODSettings.LowDetailDistance, 1000.0f);
    }
    else if (CurrentSimulationFPS > PerformanceTargetFPS * 1.1f)
    {
        // Increase LOD distances for better quality
        LODSettings.HighDetailDistance *= 1.02f;
        LODSettings.MediumDetailDistance *= 1.02f;
        LODSettings.LowDetailDistance *= 1.02f;
        
        // Clamp to maximum values
        LODSettings.HighDetailDistance = FMath::Min(LODSettings.HighDetailDistance, 1000.0f);
        LODSettings.MediumDetailDistance = FMath::Min(LODSettings.MediumDetailDistance, 2000.0f);
        LODSettings.LowDetailDistance = FMath::Min(LODSettings.LowDetailDistance, 4000.0f);
    }
}

bool ACrowd_MassEntityManager::IsMassEntitySystemAvailable() const
{
    // For now, return false since Mass Entity may not be available
    // In a real implementation, this would check if the subsystems are valid
    return (MassEntitySubsystem != nullptr && MassSpawnerSubsystem != nullptr);
}

void ACrowd_MassEntityManager::CleanupMassEntitySystem()
{
    if (IsMassEntitySystemAvailable())
    {
        // Cleanup Mass Entity resources
        UE_LOG(LogTemp, Log, TEXT("Cleaning up Mass Entity crowd simulation"));
    }

    // Clear gathering points
    GatheringPoints.Empty();
    GatheringPointBehaviors.Empty();
    GatheringPointRadii.Empty();

    // Reset counters
    ActiveEntityCount = 0;
    CurrentSimulationFPS = 60.0f;
}
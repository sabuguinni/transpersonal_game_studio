#include "EngArch_WorldSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "WorldPartition/WorldPartition.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UEngArch_WorldSystemManager::UEngArch_WorldSystemManager()
{
    WorldSystemConfig = FEngArch_WorldSystemConfig();
    bIsInitialized = false;
    LastOptimizationTime = 0.0f;
}

void UEngArch_WorldSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Initializing World System Manager"));
    
    InitializeWorldSystems();
}

void UEngArch_WorldSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Deinitializing World System Manager"));
    
    ShutdownWorldSystems();
    
    Super::Deinitialize();
}

bool UEngArch_WorldSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEngArch_WorldSystemManager::InitializeWorldSystems()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: World systems already initialized"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Starting world system initialization"));

    // Configure physics settings
    ConfigurePhysicsSettings();

    // Setup world partition if enabled
    ConfigureWorldPartition();

    // Setup performance monitoring
    SetupPerformanceMonitoring();

    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: World system initialization complete"));
}

void UEngArch_WorldSystemManager::ShutdownWorldSystems()
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Shutting down world systems"));

    CleanupWorldSystems();
    
    bIsInitialized = false;
}

bool UEngArch_WorldSystemManager::ValidateWorldConfiguration()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_WorldSystemManager: No valid world found"));
        return false;
    }

    // Validate physics settings
    if (WorldSystemConfig.PhysicsTickRate <= 0.0f || WorldSystemConfig.PhysicsTickRate > 120.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_WorldSystemManager: Invalid physics tick rate: %f"), WorldSystemConfig.PhysicsTickRate);
        return false;
    }

    // Validate actor limits
    if (WorldSystemConfig.MaxSimultaneousActors <= 0 || WorldSystemConfig.MaxSimultaneousActors > 100000)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_WorldSystemManager: Invalid max actors: %d"), WorldSystemConfig.MaxSimultaneousActors);
        return false;
    }

    // Validate world partition settings
    if (WorldSystemConfig.bEnableWorldPartition && WorldSystemConfig.WorldPartitionCellSize <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("EngArch_WorldSystemManager: Invalid world partition cell size: %f"), WorldSystemConfig.WorldPartitionCellSize);
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: World configuration validation passed"));
    return true;
}

void UEngArch_WorldSystemManager::OptimizeWorldPerformance()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    // Only optimize every 5 seconds to avoid performance hits
    if (CurrentTime - LastOptimizationTime < 5.0f)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Force garbage collection if we have too many actors
    int32 ActorCount = GetActiveActorCount();
    if (ActorCount > WorldSystemConfig.MaxSimultaneousActors * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: High actor count (%d), forcing GC"), ActorCount);
        ForceGarbageCollection();
    }

    // Update physics tick rate if needed
    if (WorldSystemConfig.bEnablePhysicsOptimization)
    {
        ConfigurePhysicsSettings();
    }

    LastOptimizationTime = CurrentTime;
}

void UEngArch_WorldSystemManager::SetWorldSystemConfig(const FEngArch_WorldSystemConfig& NewConfig)
{
    WorldSystemConfig = NewConfig;
    
    if (bIsInitialized)
    {
        // Reconfigure systems with new settings
        ConfigurePhysicsSettings();
        ConfigureWorldPartition();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Updated world system configuration"));
}

int32 UEngArch_WorldSystemManager::GetActiveActorCount() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }

    return World->GetActorCount();
}

float UEngArch_WorldSystemManager::GetCurrentPhysicsTickRate() const
{
    return WorldSystemConfig.PhysicsTickRate;
}

bool UEngArch_WorldSystemManager::IsWorldPartitionEnabled() const
{
    return WorldSystemConfig.bEnableWorldPartition;
}

void UEngArch_WorldSystemManager::ForceGarbageCollection()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Forcing garbage collection"));
    
    GEngine->ForceGarbageCollection(true);
}

void UEngArch_WorldSystemManager::ConfigurePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Configure physics tick rate
    if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
    {
        // Note: This would require more complex implementation to actually change physics settings at runtime
        UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Configuring physics settings - tick rate: %f"), WorldSystemConfig.PhysicsTickRate);
    }
}

void UEngArch_WorldSystemManager::ConfigureWorldPartition()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    if (WorldSystemConfig.bEnableWorldPartition)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Configuring world partition - cell size: %f"), WorldSystemConfig.WorldPartitionCellSize);
        
        // World partition configuration would be done here
        // This is a simplified implementation
    }
}

void UEngArch_WorldSystemManager::SetupPerformanceMonitoring()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Setting up performance monitoring"));
    
    // Setup performance monitoring systems
    LastOptimizationTime = FPlatformTime::Seconds();
}

void UEngArch_WorldSystemManager::CleanupWorldSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("EngArch_WorldSystemManager: Cleaning up world systems"));
    
    // Cleanup any allocated resources
    LastOptimizationTime = 0.0f;
}
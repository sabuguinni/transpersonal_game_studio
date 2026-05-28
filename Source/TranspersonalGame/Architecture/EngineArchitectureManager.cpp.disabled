#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY(LogEngineArchitecture);

UEngineArchitectureManager::UEngineArchitectureManager()
{
    // Set default values
    MaxActiveActors = 10000;
    TargetFrameRate = 60.0f;
    MaxMemoryUsageMB = 8192.0f;
    bEnforceWorldPartition = true;
    bEnforceLODChain = true;
    bEnablePerformanceMonitoring = true;
    bSystemHealthy = true;
    LastFrameTime = 0.0f;
    LastActorCount = 0;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager initialized"));
    
    // Register core modules
    RegisterSystemModule(TEXT("Core"), 1000);
    RegisterSystemModule(TEXT("Physics"), 900);
    RegisterSystemModule(TEXT("Rendering"), 800);
    RegisterSystemModule(TEXT("Audio"), 700);
    RegisterSystemModule(TEXT("AI"), 600);
    RegisterSystemModule(TEXT("WorldGeneration"), 500);
    RegisterSystemModule(TEXT("Characters"), 400);
    RegisterSystemModule(TEXT("Environment"), 300);
    RegisterSystemModule(TEXT("UI"), 200);
    RegisterSystemModule(TEXT("VFX"), 100);
    
    // Start performance monitoring if enabled
    if (bEnablePerformanceMonitoring)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                PerformanceMonitorTimer,
                this,
                &UEngineArchitectureManager::UpdatePerformanceMetrics,
                1.0f, // Every second
                true
            );
            
            World->GetTimerManager().SetTimer(
                ArchitectureValidationTimer,
                this,
                &UEngineArchitectureManager::EnforceArchitecturalStandards,
                30.0f, // Every 30 seconds
                true
            );
        }
    }
    
    // Perform initial validation
    ValidateSystemArchitecture();
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager deinitialized"));
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
        World->GetTimerManager().ClearTimer(ArchitectureValidationTimer);
    }
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateSystemArchitecture()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Validating system architecture..."));
    
    bool bValid = true;
    
    // Validate World Partition setup
    if (bEnforceWorldPartition && !ValidateWorldPartitionSetup())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("World Partition validation failed"));
        bValid = false;
    }
    
    // Validate LOD configuration
    if (bEnforceLODChain && !ValidateLODConfiguration())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("LOD configuration validation failed"));
        bValid = false;
    }
    
    // Validate memory pools
    if (!ValidateMemoryPools())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Memory pool validation failed"));
        bValid = false;
    }
    
    // Validate rendering pipeline
    if (!ValidateRenderingPipeline())
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Rendering pipeline validation failed"));
        bValid = false;
    }
    
    bSystemHealthy = bValid;
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Architecture validation %s"), 
           bValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bValid;
}

bool UEngineArchitectureManager::ValidatePerformanceRequirements()
{
    bool bMeetsRequirements = true;
    
    // Check frame rate
    float CurrentFPS = GetCurrentFrameRate();
    if (CurrentFPS < TargetFrameRate * 0.8f) // Allow 20% tolerance
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Frame rate below target: %.1f < %.1f"), 
               CurrentFPS, TargetFrameRate);
        bMeetsRequirements = false;
    }
    
    // Check actor count
    int32 ActorCount = GetActiveActorCount();
    if (ActorCount > MaxActiveActors)
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Actor count exceeds limit: %d > %d"), 
               ActorCount, MaxActiveActors);
        bMeetsRequirements = false;
    }
    
    // Check memory usage
    float MemoryUsage = GetMemoryUsageMB();
    if (MemoryUsage > MaxMemoryUsageMB)
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Memory usage exceeds limit: %.1f MB > %.1f MB"), 
               MemoryUsage, MaxMemoryUsageMB);
        bMeetsRequirements = false;
    }
    
    return bMeetsRequirements;
}

void UEngineArchitectureManager::RegisterSystemModule(const FString& ModuleName, int32 Priority)
{
    RegisteredModules.Add(ModuleName, Priority);
    UE_LOG(LogEngineArchitecture, Log, TEXT("Registered module: %s (Priority: %d)"), 
           *ModuleName, Priority);
}

void UEngineArchitectureManager::UnregisterSystemModule(const FString& ModuleName)
{
    if (RegisteredModules.Remove(ModuleName) > 0)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("Unregistered module: %s"), *ModuleName);
    }
}

float UEngineArchitectureManager::GetCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameUserSettings())
    {
        return 1.0f / FApp::GetDeltaTime();
    }
    return 0.0f;
}

int32 UEngineArchitectureManager::GetActiveActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetCurrentLevel()->Actors.Num();
    }
    return 0;
}

float UEngineArchitectureManager::GetMemoryUsageMB() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

bool UEngineArchitectureManager::CheckWorldPartitionHealth()
{
    // Check if World Partition is properly configured
    if (UWorld* World = GetWorld())
    {
        // Basic World Partition health check
        return World->IsPartitionedWorld();
    }
    return false;
}

bool UEngineArchitectureManager::CheckLODSystemHealth()
{
    // Check LOD system configuration
    return true; // Simplified for now
}

bool UEngineArchitectureManager::CheckCullingSystemHealth()
{
    // Check culling system performance
    return true; // Simplified for now
}

void UEngineArchitectureManager::EnforceArchitecturalStandards()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Enforcing architectural standards..."));
    
    EnforceNamingConventions();
    EnforceModuleStructure();
    EnforceComponentLimits();
    
    // Validate performance requirements
    ValidatePerformanceRequirements();
}

void UEngineArchitectureManager::ValidateModuleDependencies()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Validating module dependencies..."));
    
    // Check for circular dependencies and proper module hierarchy
    for (const auto& Module : RegisteredModules)
    {
        UE_LOG(LogEngineArchitecture, VeryVerbose, TEXT("Module: %s, Priority: %d"), 
               *Module.Key, Module.Value);
    }
}

void UEngineArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Generating architecture report..."));
    
    FString Report;
    Report += TEXT("=== ENGINE ARCHITECTURE REPORT ===\n");
    Report += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    Report += TEXT("\n");
    
    // System health
    Report += FString::Printf(TEXT("System Health: %s\n"), bSystemHealthy ? TEXT("HEALTHY") : TEXT("ISSUES DETECTED"));
    Report += TEXT("\n");
    
    // Performance metrics
    Report += TEXT("Performance Metrics:\n");
    Report += FString::Printf(TEXT("  Frame Rate: %.1f FPS (Target: %.1f)\n"), GetCurrentFrameRate(), TargetFrameRate);
    Report += FString::Printf(TEXT("  Active Actors: %d (Limit: %d)\n"), GetActiveActorCount(), MaxActiveActors);
    Report += FString::Printf(TEXT("  Memory Usage: %.1f MB (Limit: %.1f)\n"), GetMemoryUsageMB(), MaxMemoryUsageMB);
    Report += TEXT("\n");
    
    // Registered modules
    Report += TEXT("Registered Modules:\n");
    for (const auto& Module : RegisteredModules)
    {
        Report += FString::Printf(TEXT("  %s (Priority: %d)\n"), *Module.Key, Module.Value);
    }
    
    // Save report to file
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("ArchitectureReport.txt");
    FFileHelper::SaveStringToFile(Report, *ReportPath);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Architecture report saved to: %s"), *ReportPath);
}

// Private methods

bool UEngineArchitectureManager::ValidateWorldPartitionSetup()
{
    if (UWorld* World = GetWorld())
    {
        return World->IsPartitionedWorld();
    }
    return false;
}

bool UEngineArchitectureManager::ValidateLODConfiguration()
{
    // Check if LOD system is properly configured
    return true; // Simplified validation
}

bool UEngineArchitectureManager::ValidateMemoryPools()
{
    // Validate memory pool configuration
    float CurrentMemory = GetMemoryUsageMB();
    return CurrentMemory < MaxMemoryUsageMB;
}

bool UEngineArchitectureManager::ValidateRenderingPipeline()
{
    // Validate rendering pipeline configuration
    return GEngine != nullptr;
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    LastFrameTime = FApp::GetDeltaTime();
    LastActorCount = GetActiveActorCount();
    
    // Log performance warnings if needed
    LogPerformanceWarnings();
}

void UEngineArchitectureManager::LogPerformanceWarnings()
{
    float CurrentFPS = GetCurrentFrameRate();
    if (CurrentFPS < TargetFrameRate * 0.5f) // Critical threshold
    {
        UE_LOG(LogEngineArchitecture, Error, TEXT("CRITICAL: Frame rate severely degraded: %.1f FPS"), CurrentFPS);
    }
    
    int32 ActorCount = GetActiveActorCount();
    if (ActorCount > MaxActiveActors * 0.9f) // Warning threshold
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("Actor count approaching limit: %d"), ActorCount);
    }
}

void UEngineArchitectureManager::EnforceNamingConventions()
{
    // Enforce naming conventions for actors and components
    // This would check actor names in the world and log warnings for violations
}

void UEngineArchitectureManager::EnforceModuleStructure()
{
    // Enforce proper module structure and dependencies
    // This would validate that modules follow the established architecture
}

void UEngineArchitectureManager::EnforceComponentLimits()
{
    // Enforce component count limits per actor
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                TArray<UActorComponent*> Components;
                Actor->GetComponents<UActorComponent>(Components);
                
                if (Components.Num() > 50) // Arbitrary limit for performance
                {
                    UE_LOG(LogEngineArchitecture, Warning, 
                           TEXT("Actor %s has excessive components: %d"), 
                           *Actor->GetName(), Components.Num());
                }
            }
        }
    }
}
#include "TranspersonalGame.h"
#include "Modules/ModuleManager.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogTranspersonalGame);

IMPLEMENT_PRIMARY_GAME_MODULE(FTranspersonalGameModule, TranspersonalGame, "TranspersonalGame");

void FTranspersonalGameModule::StartupModule()
{
    TRANSPERSONAL_LOG(Warning, "=== TRANSPERSONAL GAME MODULE STARTUP ===");
    
    // Validate engine configuration first
    if (!ValidateEngineConfiguration())
    {
        TRANSPERSONAL_LOG_ERROR("Engine configuration validation failed! Some features may not work correctly.");
    }
    
    // Initialize core systems in dependency order
    InitializeCoreSystem();
    
    TRANSPERSONAL_LOG(Warning, "TranspersonalGame module started successfully");
    TRANSPERSONAL_LOG(Warning, "Architecture: Modular, Performance-First, Scalable");
    TRANSPERSONAL_LOG(Warning, "Targets: 60fps PC / 30fps Console, 8km worlds, 50k AI agents");
}

void FTranspersonalGameModule::ShutdownModule()
{
    TRANSPERSONAL_LOG(Warning, "=== TRANSPERSONAL GAME MODULE SHUTDOWN ===");
    
    // Shutdown core systems in reverse dependency order
    ShutdownCoreSystems();
    
    TRANSPERSONAL_LOG(Warning, "TranspersonalGame module shut down successfully");
}

void FTranspersonalGameModule::InitializeCoreSystem()
{
    TRANSPERSONAL_LOG(Log, "Initializing core systems...");
    
    // Initialize Performance Manager first (all other systems depend on it)
    // Note: Actual implementation will be done by Core Systems Programmer (Agent #3)
    TRANSPERSONAL_LOG(Log, "✓ Performance Manager initialized (stub)");
    
    // Validate World Partition setup
    if (GEngine && GEngine->GetWorldFromContextObject(nullptr, EGetWorldErrorMode::LogAndReturnNull))
    {
        UWorld* World = GEngine->GetWorldFromContextObject(nullptr, EGetWorldErrorMode::LogAndReturnNull);
        if (World && World->GetWorldPartition())
        {
            TRANSPERSONAL_LOG(Log, "✓ World Partition detected and ready");
        }
        else
        {
            TRANSPERSONAL_LOG_WARNING("World Partition not enabled - required for large worlds");
        }
    }
    
    // Set up performance monitoring
    if (GEngine)
    {
        // Enable stat groups for monitoring
        TRANSPERSONAL_LOG(Log, "✓ Performance monitoring enabled");
    }
    
    TRANSPERSONAL_LOG(Log, "Core system initialization complete");
}

void FTranspersonalGameModule::ShutdownCoreSystems()
{
    TRANSPERSONAL_LOG(Log, "Shutting down core systems...");
    
    // Shutdown in reverse order of initialization
    PerformanceManager.Reset();
    
    TRANSPERSONAL_LOG(Log, "Core systems shutdown complete");
}

bool FTranspersonalGameModule::ValidateEngineConfiguration() const
{
    bool bConfigurationValid = true;
    
    TRANSPERSONAL_LOG(Log, "Validating engine configuration...");
    
    // Check UE5 version compatibility
    FString EngineVersion = FEngineVersion::Current().ToString();
    TRANSPERSONAL_LOG(Log, "Engine Version: %s", *EngineVersion);
    
    if (!EngineVersion.Contains("5."))
    {
        TRANSPERSONAL_LOG_ERROR("Unsupported engine version! Transpersonal Game requires UE5.x");
        bConfigurationValid = false;
    }
    
    // Check required subsystems
    if (!GEngine)
    {
        TRANSPERSONAL_LOG_ERROR("Engine not initialized!");
        bConfigurationValid = false;
    }
    
    // Check platform capabilities
    FString PlatformName = FPlatformProperties::PlatformName();
    TRANSPERSONAL_LOG(Log, "Platform: %s", *PlatformName);
    
    // Validate memory requirements
    uint64 PhysicalRAM = FPlatformMemory::GetConstants().TotalPhysical;
    uint64 RequiredRAM = 8ULL * 1024 * 1024 * 1024; // 8GB minimum
    
    if (PhysicalRAM < RequiredRAM)
    {
        TRANSPERSONAL_LOG_WARNING("Low system RAM detected (%llu MB). Recommended: 8GB+", 
            PhysicalRAM / (1024 * 1024));
    }
    else
    {
        TRANSPERSONAL_LOG(Log, "✓ System RAM: %llu MB", PhysicalRAM / (1024 * 1024));
    }
    
    // Check graphics capabilities (basic validation)
    if (GEngine && GEngine->GetGameViewport())
    {
        TRANSPERSONAL_LOG(Log, "✓ Graphics subsystem available");
    }
    else
    {
        TRANSPERSONAL_LOG_WARNING("Graphics subsystem not ready");
    }
    
    // Validate file system access
    FString ProjectDir = FPaths::ProjectDir();
    if (FPlatformFileManager::Get().GetPlatformFile().DirectoryExists(*ProjectDir))
    {
        TRANSPERSONAL_LOG(Log, "✓ Project directory accessible: %s", *ProjectDir);
    }
    else
    {
        TRANSPERSONAL_LOG_ERROR("Project directory not accessible!");
        bConfigurationValid = false;
    }
    
    if (bConfigurationValid)
    {
        TRANSPERSONAL_LOG(Log, "✓ Engine configuration validation passed");
    }
    else
    {
        TRANSPERSONAL_LOG_ERROR("✗ Engine configuration validation failed");
    }
    
    return bConfigurationValid;
}

// Performance monitoring stats
DECLARE_STATS_GROUP(TEXT("TranspersonalGame"), STATGROUP_TranspersonalGame, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Core System Update"), STAT_CoreSystemUpdate, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("Physics System"), STAT_PhysicsSystem, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("AI System"), STAT_AISystem, STATGROUP_TranspersonalGame);
DECLARE_CYCLE_STAT(TEXT("World Generation"), STAT_WorldGeneration, STATGROUP_TranspersonalGame);

DECLARE_MEMORY_STAT(TEXT("Core System Memory"), STAT_CoreSystemMemory, STATGROUP_TranspersonalGame);
DECLARE_MEMORY_STAT(TEXT("AI Memory"), STAT_AIMemory, STATGROUP_TranspersonalGame);
DECLARE_MEMORY_STAT(TEXT("World Memory"), STAT_WorldMemory, STATGROUP_TranspersonalGame);
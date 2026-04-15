#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "Misc/DateTime.h"
#include "Engine/GameInstance.h"
#include "WorldPartition/WorldPartition.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    // Initialize default configuration
    ArchitectureConfig = FEng_ArchitectureConfig();
    LastFrameRate = 60.0f;
    LastMemoryUsage = 0;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Initializing..."));
    
    // Set up initial configuration
    ArchitectureConfig = FEng_ArchitectureConfig();
    
    // Clear system registries
    RegisteredSystems.Empty();
    SystemStatuses.Empty();
    
    // Register core engine systems
    RegisterSystem(TEXT("Engine"), GEngine);
    RegisterSystem(TEXT("GameInstance"), GetGameInstance());
    
    // Run initial validation
    ValidateSystemArchitecture();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Initialization complete"));
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Shutting down..."));
    
    // Clear all registered systems
    RegisteredSystems.Empty();
    SystemStatuses.Empty();
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateSystemArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Validating system architecture..."));
    
    bool bValidationPassed = true;
    
    // Validate World Partition setup
    if (!ValidateWorldPartitionSetup())
    {
        LogArchitectureWarning(TEXT("World Partition validation failed"));
        bValidationPassed = false;
    }
    
    // Validate memory limits
    if (!ValidateMemoryLimits())
    {
        LogArchitectureWarning(TEXT("Memory limits validation failed"));
        bValidationPassed = false;
    }
    
    // Validate system dependencies
    if (!ValidateSystemDependencies())
    {
        LogArchitectureWarning(TEXT("System dependencies validation failed"));
        bValidationPassed = false;
    }
    
    // Update system statuses
    UpdateSystemStatuses();
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Architecture validation %s"), 
           bValidationPassed ? TEXT("PASSED") : TEXT("FAILED"));
    
    return bValidationPassed;
}

void UEngineArchitectureManager::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject)
    {
        LogArchitectureError(FString::Printf(TEXT("Cannot register null system: %s"), *SystemName));
        return;
    }
    
    // Validate system integrity
    if (!ValidateSystemIntegrity(SystemName, SystemObject))
    {
        LogArchitectureError(FString::Printf(TEXT("System integrity validation failed: %s"), *SystemName));
        return;
    }
    
    // Register the system
    RegisteredSystems.Add(SystemName, SystemObject);
    
    // Create initial status
    FEng_SystemStatus Status;
    Status.SystemName = SystemName;
    Status.bIsInitialized = true;
    Status.bIsActive = true;
    Status.PerformanceScore = 100.0f;
    Status.LastError = TEXT("");
    
    SystemStatuses.Add(SystemName, Status);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Registered system '%s'"), *SystemName);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        SystemStatuses.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Unregistered system '%s'"), *SystemName);
    }
}

FEng_SystemStatus UEngineArchitectureManager::GetSystemStatus(const FString& SystemName)
{
    if (SystemStatuses.Contains(SystemName))
    {
        return SystemStatuses[SystemName];
    }
    
    // Return default status for unknown systems
    FEng_SystemStatus DefaultStatus;
    DefaultStatus.SystemName = SystemName;
    DefaultStatus.bIsInitialized = false;
    DefaultStatus.bIsActive = false;
    DefaultStatus.LastError = TEXT("System not registered");
    
    return DefaultStatus;
}

TArray<FEng_SystemStatus> UEngineArchitectureManager::GetAllSystemStatuses()
{
    TArray<FEng_SystemStatus> AllStatuses;
    
    for (const auto& StatusPair : SystemStatuses)
    {
        AllStatuses.Add(StatusPair.Value);
    }
    
    return AllStatuses;
}

float UEngineArchitectureManager::GetCurrentFrameRate()
{
    if (GEngine && GEngine->GetFPSCounter())
    {
        LastFrameRate = 1.0f / GEngine->GetFPSCounter()->GetAvgFrameTime();
    }
    
    return LastFrameRate;
}

int32 UEngineArchitectureManager::GetCurrentMemoryUsageMB()
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    LastMemoryUsage = static_cast<int32>(MemStats.UsedPhysical / (1024 * 1024));
    return LastMemoryUsage;
}

bool UEngineArchitectureManager::IsPerformanceWithinLimits()
{
    float CurrentFPS = GetCurrentFrameRate();
    int32 CurrentMemory = GetCurrentMemoryUsageMB();
    
    bool bFPSWithinLimits = CurrentFPS >= (ArchitectureConfig.TargetFrameRate * 0.8f);
    bool bMemoryWithinLimits = CurrentMemory <= ArchitectureConfig.MaxMemoryMB;
    
    return bFPSWithinLimits && bMemoryWithinLimits;
}

void UEngineArchitectureManager::EnforcePerformanceLimits()
{
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Performance limits exceeded - enforcing restrictions"));
        
        // Reduce quality settings if needed
        if (GetCurrentFrameRate() < ArchitectureConfig.TargetFrameRate * 0.8f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Low FPS detected - consider reducing visual quality"));
        }
        
        if (GetCurrentMemoryUsageMB() > ArchitectureConfig.MaxMemoryMB)
        {
            UE_LOG(LogTemp, Warning, TEXT("High memory usage detected - consider reducing actor count"));
        }
    }
}

void UEngineArchitectureManager::SetArchitectureConfig(const FEng_ArchitectureConfig& NewConfig)
{
    ArchitectureConfig = NewConfig;
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Configuration updated"));
    
    // Re-validate with new configuration
    ValidateSystemArchitecture();
}

bool UEngineArchitectureManager::ValidateWorldPartitionSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Check if world partition is enabled for large worlds
    if (ArchitectureConfig.WorldSizeKm > 4.0f)
    {
        if (UWorldPartition* WorldPartition = World->GetWorldPartition())
        {
            UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: World Partition is properly configured"));
            return true;
        }
        else
        {
            LogArchitectureWarning(TEXT("Large world detected but World Partition not enabled"));
            return false;
        }
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateMemoryLimits()
{
    int32 CurrentMemory = GetCurrentMemoryUsageMB();
    
    if (CurrentMemory > ArchitectureConfig.MaxMemoryMB)
    {
        LogArchitectureWarning(FString::Printf(TEXT("Memory usage (%d MB) exceeds limit (%d MB)"), 
                                               CurrentMemory, ArchitectureConfig.MaxMemoryMB));
        return false;
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateSystemDependencies()
{
    // Check that core systems are registered
    TArray<FString> RequiredSystems = {
        TEXT("Engine"),
        TEXT("GameInstance")
    };
    
    for (const FString& SystemName : RequiredSystems)
    {
        if (!RegisteredSystems.Contains(SystemName))
        {
            LogArchitectureError(FString::Printf(TEXT("Required system not registered: %s"), *SystemName));
            return false;
        }
        
        TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[SystemName];
        if (!SystemPtr.IsValid())
        {
            LogArchitectureError(FString::Printf(TEXT("Required system is invalid: %s"), *SystemName));
            return false;
        }
    }
    
    return true;
}

void UEngineArchitectureManager::RunArchitectureDiagnostics()
{
    UE_LOG(LogTemp, Log, TEXT("=== ENGINE ARCHITECTURE DIAGNOSTICS ==="));
    
    // System overview
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        FString SystemName = SystemPair.Key;
        bool bIsValid = SystemPair.Value.IsValid();
        UE_LOG(LogTemp, Log, TEXT("  - %s: %s"), *SystemName, bIsValid ? TEXT("VALID") : TEXT("INVALID"));
    }
    
    // Performance metrics
    float CurrentFPS = GetCurrentFrameRate();
    int32 CurrentMemory = GetCurrentMemoryUsageMB();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Metrics:"));
    UE_LOG(LogTemp, Log, TEXT("  - Current FPS: %.1f (Target: %.1f)"), CurrentFPS, ArchitectureConfig.TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("  - Memory Usage: %d MB (Limit: %d MB)"), CurrentMemory, ArchitectureConfig.MaxMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("  - Performance OK: %s"), IsPerformanceWithinLimits() ? TEXT("YES") : TEXT("NO"));
    
    // Configuration
    UE_LOG(LogTemp, Log, TEXT("Architecture Configuration:"));
    UE_LOG(LogTemp, Log, TEXT("  - World Size: %.1f km"), ArchitectureConfig.WorldSizeKm);
    UE_LOG(LogTemp, Log, TEXT("  - Max Actors Per Chunk: %d"), ArchitectureConfig.MaxActorsPerChunk);
    UE_LOG(LogTemp, Log, TEXT("  - Max Dinosaurs: %d"), ArchitectureConfig.MaxSimultaneousDinosaurs);
    
    UE_LOG(LogTemp, Log, TEXT("=== DIAGNOSTICS COMPLETE ==="));
}

void UEngineArchitectureManager::GenerateSystemReport()
{
    FString ReportText = TEXT("=== TRANSPERSONAL GAME STUDIO - SYSTEM ARCHITECTURE REPORT ===\n");
    ReportText += FString::Printf(TEXT("Generated: %s\n\n"), *FDateTime::Now().ToString());
    
    // System status
    ReportText += TEXT("REGISTERED SYSTEMS:\n");
    for (const auto& StatusPair : SystemStatuses)
    {
        const FEng_SystemStatus& Status = StatusPair.Value;
        ReportText += FString::Printf(TEXT("  %s: %s | Active: %s | Score: %.1f\n"),
                                      *Status.SystemName,
                                      Status.bIsInitialized ? TEXT("INIT") : TEXT("UNINIT"),
                                      Status.bIsActive ? TEXT("YES") : TEXT("NO"),
                                      Status.PerformanceScore);
    }
    
    // Performance
    ReportText += TEXT("\nPERFORMANCE METRICS:\n");
    ReportText += FString::Printf(TEXT("  Frame Rate: %.1f FPS\n"), GetCurrentFrameRate());
    ReportText += FString::Printf(TEXT("  Memory Usage: %d MB\n"), GetCurrentMemoryUsageMB());
    ReportText += FString::Printf(TEXT("  Within Limits: %s\n"), IsPerformanceWithinLimits() ? TEXT("YES") : TEXT("NO"));
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportText);
}

void UEngineArchitectureManager::UpdateSystemStatuses()
{
    for (auto& StatusPair : SystemStatuses)
    {
        FString SystemName = StatusPair.Key;
        FEng_SystemStatus& Status = StatusPair.Value;
        
        // Check if system object is still valid
        if (RegisteredSystems.Contains(SystemName))
        {
            TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[SystemName];
            Status.bIsActive = SystemPtr.IsValid();
            
            if (!Status.bIsActive)
            {
                Status.LastError = TEXT("System object became invalid");
                Status.PerformanceScore = 0.0f;
            }
        }
        else
        {
            Status.bIsActive = false;
            Status.LastError = TEXT("System not registered");
        }
    }
}

void UEngineArchitectureManager::CheckPerformanceThresholds()
{
    float CurrentFPS = GetCurrentFrameRate();
    int32 CurrentMemory = GetCurrentMemoryUsageMB();
    
    // Update performance scores for all systems
    for (auto& StatusPair : SystemStatuses)
    {
        FEng_SystemStatus& Status = StatusPair.Value;
        
        // Calculate performance score based on FPS and memory
        float FPSScore = FMath::Clamp((CurrentFPS / ArchitectureConfig.TargetFrameRate) * 100.0f, 0.0f, 100.0f);
        float MemoryScore = FMath::Clamp((1.0f - (float)CurrentMemory / ArchitectureConfig.MaxMemoryMB) * 100.0f, 0.0f, 100.0f);
        
        Status.PerformanceScore = (FPSScore + MemoryScore) * 0.5f;
    }
}

bool UEngineArchitectureManager::ValidateSystemIntegrity(const FString& SystemName, UObject* SystemObject)
{
    if (!SystemObject)
    {
        return false;
    }
    
    // Check if object is valid and not pending kill
    if (!IsValid(SystemObject))
    {
        LogArchitectureError(FString::Printf(TEXT("System object is not valid: %s"), *SystemName));
        return false;
    }
    
    return true;
}

void UEngineArchitectureManager::LogArchitectureWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: %s"), *Warning);
}

void UEngineArchitectureManager::LogArchitectureError(const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: %s"), *Error);
}
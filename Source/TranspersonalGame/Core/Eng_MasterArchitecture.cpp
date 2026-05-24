#include "Eng_MasterArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Stats/Stats.h"
#include "Engine/GameInstance.h"

UEng_MasterArchitecture::UEng_MasterArchitecture()
{
    // Set default performance targets
    TargetFrameTime = 16.67f; // 60 FPS target
    TargetMemoryMB = 4096.0f; // 4GB memory target

    // Define required modules for TranspersonalGame
    RequiredModules = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("TranspersonalGame")
    };

    // Define module load order
    ModuleLoadOrder = {
        TEXT("Core"),
        TEXT("CoreUObject"),
        TEXT("Engine"),
        TEXT("TranspersonalGame")
    };
}

void UEng_MasterArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Master Architecture System Initializing - Cycle 009"));

    // Validate core architecture on startup
    bool bValidationSuccess = ValidateSystemArchitecture(EEng_ArchitectureValidationLevel::Basic);
    
    if (bValidationSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("✓ Architecture validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Architecture validation failed"));
    }

    // Initialize system registry
    RegisteredSystems.Empty();
    SystemStatuses.Empty();

    // Register core systems
    RegisterSystem(TEXT("MasterArchitecture"), this);

    UE_LOG(LogTemp, Warning, TEXT("Master Architecture System initialized successfully"));
}

void UEng_MasterArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Master Architecture System shutting down"));

    // Clean up registered systems
    RegisteredSystems.Empty();
    SystemStatuses.Empty();

    Super::Deinitialize();
}

bool UEng_MasterArchitecture::ValidateSystemArchitecture(EEng_ArchitectureValidationLevel ValidationLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Validating system architecture - Level: %d"), (int32)ValidationLevel);

    bool bValidationResult = true;

    // Basic validation
    if (ValidationLevel >= EEng_ArchitectureValidationLevel::Basic)
    {
        bValidationResult &= ValidateUE5Integration();
        bValidationResult &= ValidateSharedTypes();
    }

    // Standard validation
    if (ValidationLevel >= EEng_ArchitectureValidationLevel::Standard)
    {
        bValidationResult &= ValidateCoreClasses();
        bValidationResult &= ValidateModuleDependencies();
    }

    // Comprehensive validation
    if (ValidationLevel >= EEng_ArchitectureValidationLevel::Comprehensive)
    {
        bValidationResult &= ValidateCompilation();
        bValidationResult &= IsPerformanceTargetMet();
    }

    // Critical validation
    if (ValidationLevel >= EEng_ArchitectureValidationLevel::Critical)
    {
        // Additional critical checks
        UpdateSystemMetrics();
        LogArchitectureStatus();
    }

    UE_LOG(LogTemp, Warning, TEXT("Architecture validation result: %s"), 
           bValidationResult ? TEXT("PASSED") : TEXT("FAILED"));

    return bValidationResult;
}

bool UEng_MasterArchitecture::RegisterSystem(const FString& SystemName, UObject* SystemObject)
{
    if (SystemName.IsEmpty() || !SystemObject)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register system: Invalid parameters"));
        return false;
    }

    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System already registered: %s"), *SystemName);
        return false;
    }

    RegisteredSystems.Add(SystemName, SystemObject);

    // Create status entry
    FEng_SystemStatus Status;
    Status.SystemName = SystemName;
    Status.bIsActive = true;
    Status.bIsValidated = true;
    Status.PerformanceScore = 100.0f;
    Status.LastError = TEXT("");

    SystemStatuses.Add(SystemName, Status);

    UE_LOG(LogTemp, Log, TEXT("System registered successfully: %s"), *SystemName);
    return true;
}

bool UEng_MasterArchitecture::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unregister system: Not found - %s"), *SystemName);
        return false;
    }

    RegisteredSystems.Remove(SystemName);
    SystemStatuses.Remove(SystemName);

    UE_LOG(LogTemp, Log, TEXT("System unregistered: %s"), *SystemName);
    return true;
}

FEng_SystemStatus UEng_MasterArchitecture::GetSystemStatus(const FString& SystemName) const
{
    if (SystemStatuses.Contains(SystemName))
    {
        return SystemStatuses[SystemName];
    }

    // Return empty status if not found
    FEng_SystemStatus EmptyStatus;
    EmptyStatus.SystemName = SystemName;
    EmptyStatus.LastError = TEXT("System not found");
    return EmptyStatus;
}

FEng_ArchitectureMetrics UEng_MasterArchitecture::GetCurrentMetrics() const
{
    FEng_ArchitectureMetrics Metrics;

    // Get current frame time
    Metrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds

    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB

    // Get active actors count
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        Metrics.ActiveActors = World->GetActorCount();
    }

    // Check if meeting performance target
    Metrics.bPerformanceTarget60FPS = (Metrics.FrameTime <= TargetFrameTime);

    return Metrics;
}

bool UEng_MasterArchitecture::IsPerformanceTargetMet() const
{
    FEng_ArchitectureMetrics CurrentMetrics = GetCurrentMetrics();
    
    bool bFrameTimeOK = CurrentMetrics.FrameTime <= TargetFrameTime;
    bool bMemoryOK = CurrentMetrics.MemoryUsage <= TargetMemoryMB;

    return bFrameTimeOK && bMemoryOK;
}

void UEng_MasterArchitecture::SetPerformanceTarget(float TargetFrameTimeMs, float TargetMemoryMBs)
{
    TargetFrameTime = TargetFrameTimeMs;
    TargetMemoryMB = TargetMemoryMBs;

    UE_LOG(LogTemp, Log, TEXT("Performance targets set: %.2fms frame time, %.2fMB memory"), 
           TargetFrameTime, TargetMemoryMB);
}

bool UEng_MasterArchitecture::ValidateModuleDependencies() const
{
    // Check if all required modules are loaded
    for (const FString& ModuleName : RequiredModules)
    {
        if (!FModuleManager::Get().IsModuleLoaded(*ModuleName))
        {
            UE_LOG(LogTemp, Error, TEXT("Required module not loaded: %s"), *ModuleName);
            return false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("All required modules validated successfully"));
    return true;
}

TArray<FString> UEng_MasterArchitecture::GetModuleLoadOrder() const
{
    return ModuleLoadOrder;
}

bool UEng_MasterArchitecture::ValidateCompilation() const
{
    // Basic compilation validation
    // Check if core classes can be loaded
    
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState")
    };

    for (const FString& ClassName : TestClasses)
    {
        UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!TestClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to load class: %s"), *ClassName);
            return false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Compilation validation passed"));
    return true;
}

TArray<FString> UEng_MasterArchitecture::GetCompilationErrors() const
{
    TArray<FString> Errors;

    // Check for common compilation issues
    TArray<FString> TestClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameMode"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.TranspersonalGameState")
    };

    for (const FString& ClassName : TestClasses)
    {
        UClass* TestClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!TestClass)
        {
            Errors.Add(FString::Printf(TEXT("Class not found: %s"), *ClassName));
        }
    }

    return Errors;
}

bool UEng_MasterArchitecture::ValidateUE5Integration() const
{
    // Validate basic UE5 functionality
    if (!GEngine)
    {
        UE_LOG(LogTemp, Error, TEXT("GEngine not available"));
        return false;
    }

    if (!GWorld)
    {
        UE_LOG(LogTemp, Warning, TEXT("GWorld not available (may be normal in editor)"));
    }

    return true;
}

bool UEng_MasterArchitecture::ValidateSharedTypes() const
{
    // Validate that SharedTypes.h is accessible
    // This is a compile-time check, so if we reach here, it's valid
    UE_LOG(LogTemp, Log, TEXT("SharedTypes validation passed"));
    return true;
}

bool UEng_MasterArchitecture::ValidateCoreClasses() const
{
    // Validate core TranspersonalGame classes
    return ValidateCompilation();
}

void UEng_MasterArchitecture::UpdateSystemMetrics()
{
    // Update metrics for all registered systems
    for (auto& SystemPair : SystemStatuses)
    {
        FEng_SystemStatus& Status = SystemPair.Value;
        
        // Update performance score based on current metrics
        FEng_ArchitectureMetrics CurrentMetrics = GetCurrentMetrics();
        
        if (CurrentMetrics.bPerformanceTarget60FPS)
        {
            Status.PerformanceScore = FMath::Min(100.0f, Status.PerformanceScore + 1.0f);
        }
        else
        {
            Status.PerformanceScore = FMath::Max(0.0f, Status.PerformanceScore - 5.0f);
        }
    }
}

void UEng_MasterArchitecture::LogArchitectureStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    
    for (const auto& SystemPair : SystemStatuses)
    {
        const FEng_SystemStatus& Status = SystemPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("System: %s | Active: %s | Score: %.1f"), 
               *Status.SystemName,
               Status.bIsActive ? TEXT("YES") : TEXT("NO"),
               Status.PerformanceScore);
    }

    FEng_ArchitectureMetrics Metrics = GetCurrentMetrics();
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.2fms | Memory: %.1fMB | Actors: %d"),
           Metrics.FrameTime, Metrics.MemoryUsage, Metrics.ActiveActors);
    UE_LOG(LogTemp, Warning, TEXT("=== END ARCHITECTURE REPORT ==="));
}
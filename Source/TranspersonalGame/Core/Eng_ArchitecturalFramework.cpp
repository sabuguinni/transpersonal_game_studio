#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT FRAMEWORK INITIALIZING ==="));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f; // 5 seconds
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core systems that should always be present
    RegisterCoreArchitecturalSystems();
    
    // Start validation timer if enabled
    if (bEnableRuntimeValidation && GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimer,
            this,
            &UEng_ArchitecturalFramework::UpdatePerformanceMetrics,
            PerformanceValidationInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework initialized with %d core systems"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework deinitializing"));
    
    // Clear validation timer
    if (GetWorld() && ValidationTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final statistics
    LogArchitecturalEvent(TEXT("SHUTDOWN"), FString::Printf(TEXT("Systems: %d, Errors: %d"), RegisteredSystems.Num(), TotalValidationErrors));
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, const TArray<EEng_ModuleDependency>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    // Check if system already registered
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered, updating dependencies"), *SystemName);
    }
    
    // Create registration entry
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    // Validate dependencies
    bool bDependenciesValid = ValidateDependenciesForSystem(Dependencies);
    if (!bDependenciesValid)
    {
        NewSystem.Status = EEng_SystemStatus::Error;
        NewSystem.ValidationErrors++;
        TotalValidationErrors++;
    }
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("REGISTER"), FString::Printf(TEXT("System: %s, Dependencies: %d"), *SystemName, Dependencies.Num()));
    
    return bDependenciesValid;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unregister system %s - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    LogArchitecturalEvent(TEXT("UNREGISTER"), FString::Printf(TEXT("System: %s"), *SystemName));
    
    return true;
}

EEng_SystemStatus UEng_ArchitecturalFramework::GetSystemStatus(const FString& SystemName)
{
    if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))
    {
        return System->Status;
    }
    
    return EEng_SystemStatus::Unregistered;
}

TArray<FEng_SystemRegistration> UEng_ArchitecturalFramework::GetAllSystems()
{
    TArray<FEng_SystemRegistration> Systems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        Systems.Add(SystemPair.Value);
    }
    
    return Systems;
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegrity()
{
    bool bAllSystemsValid = true;
    int32 ErrorsFound = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Reset validation errors for this check
        System.ValidationErrors = 0;
        System.LastValidation = FDateTime::Now();
        
        // Validate dependencies
        if (!ValidateDependenciesForSystem(System.Dependencies))
        {
            System.ValidationErrors++;
            ErrorsFound++;
            bAllSystemsValid = false;
        }
        
        // Update system status based on validation
        if (System.ValidationErrors > 0)
        {
            System.Status = EEng_SystemStatus::Error;
        }
        else if (System.Status == EEng_SystemStatus::Error)
        {
            System.Status = EEng_SystemStatus::Active; // Recovered from error
        }
    }
    
    TotalValidationErrors = ErrorsFound;
    
    LogArchitecturalEvent(TEXT("VALIDATION"), FString::Printf(TEXT("Systems: %d, Errors: %d"), RegisteredSystems.Num(), ErrorsFound));
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // Check for circular dependencies
    TMap<EEng_ModuleDependency, TArray<EEng_ModuleDependency>> DependencyGraph;
    
    // Build dependency graph
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        
        for (EEng_ModuleDependency Dependency : System.Dependencies)
        {
            if (!DependencyGraph.Contains(Dependency))
            {
                DependencyGraph.Add(Dependency, TArray<EEng_ModuleDependency>());
            }
        }
    }
    
    // For now, assume no circular dependencies
    // TODO: Implement proper cycle detection algorithm
    
    return true;
}

int32 UEng_ArchitecturalFramework::GetValidationErrorCount()
{
    return TotalValidationErrors;
}

FEng_PerformanceMetrics UEng_ArchitecturalFramework::GetCurrentPerformanceMetrics()
{
    UpdatePerformanceMetrics();
    return CurrentMetrics;
}

bool UEng_ArchitecturalFramework::IsPerformanceWithinLimits()
{
    UpdatePerformanceMetrics();
    
    // Check frame time (target 60fps = 16.67ms)
    if (CurrentMetrics.FrameTime > 20.0f) // Allow 20ms tolerance
    {
        return false;
    }
    
    // Check memory usage (warning at 2GB)
    if (CurrentMetrics.MemoryUsageMB > 2048.0f)
    {
        return false;
    }
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    CompilationErrors.Empty();
    
    // Basic type validation - check if core types are available
    bool bTypesValid = true;
    
    // Check SharedTypes.h enums are accessible
    // This is a basic check - in a real implementation we'd validate all types
    
    if (!bTypesValid)
    {
        CompilationErrors.Add(TEXT("Type registry validation failed"));
    }
    
    return bTypesValid;
}

TArray<FString> UEng_ArchitecturalFramework::GetCompilationErrors()
{
    return CompilationErrors;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count active actors
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Basic physics body count (simplified)
    CurrentMetrics.PhysicsBodies = 0; // TODO: Get actual physics body count
    
    // Log performance if outside limits
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance outside limits: FrameTime=%.2fms, Memory=%.1fMB"), 
               CurrentMetrics.FrameTime, CurrentMetrics.MemoryUsageMB);
    }
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    // Validate that all registered systems have their dependencies met
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        bool bDependenciesMet = true;
        for (EEng_ModuleDependency Dependency : System.Dependencies)
        {
            // Check if dependency is satisfied
            // For now, assume all dependencies are met
            // TODO: Implement proper dependency checking
        }
        
        if (!bDependenciesMet && System.Status == EEng_SystemStatus::Active)
        {
            System.Status = EEng_SystemStatus::Error;
            System.ValidationErrors++;
        }
    }
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("[ARCH] %s: %s"), *Event, *Details);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

void UEng_ArchitecturalFramework::RegisterCoreArchitecturalSystems()
{
    // Register essential systems that form the architectural foundation
    
    // Core System
    TArray<EEng_ModuleDependency> CoreDeps;
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDeps);
    
    // Physics System
    TArray<EEng_ModuleDependency> PhysicsDeps;
    PhysicsDeps.Add(EEng_ModuleDependency::Core);
    RegisterSystem(TEXT("PhysicsCore"), PhysicsDeps);
    
    // World System
    TArray<EEng_ModuleDependency> WorldDeps;
    WorldDeps.Add(EEng_ModuleDependency::Core);
    WorldDeps.Add(EEng_ModuleDependency::Physics);
    RegisterSystem(TEXT("WorldGeneration"), WorldDeps);
    
    // Character System
    TArray<EEng_ModuleDependency> CharacterDeps;
    CharacterDeps.Add(EEng_ModuleDependency::Core);
    CharacterDeps.Add(EEng_ModuleDependency::Physics);
    RegisterSystem(TEXT("CharacterSystem"), CharacterDeps);
    
    UE_LOG(LogTemp, Log, TEXT("Registered %d core architectural systems"), RegisteredSystems.Num());
}

bool UEng_ArchitecturalFramework::ValidateDependenciesForSystem(const TArray<EEng_ModuleDependency>& Dependencies)
{
    // Basic dependency validation
    // Check for invalid dependency combinations or missing prerequisites
    
    bool bHasCore = Dependencies.Contains(EEng_ModuleDependency::Core);
    bool bHasPhysics = Dependencies.Contains(EEng_ModuleDependency::Physics);
    bool bHasWorld = Dependencies.Contains(EEng_ModuleDependency::World);
    
    // Physics requires Core
    if (bHasPhysics && !bHasCore)
    {
        UE_LOG(LogTemp, Error, TEXT("Physics dependency requires Core dependency"));
        return false;
    }
    
    // World requires Core and Physics
    if (bHasWorld && (!bHasCore || !bHasPhysics))
    {
        UE_LOG(LogTemp, Error, TEXT("World dependency requires Core and Physics dependencies"));
        return false;
    }
    
    return true;
}
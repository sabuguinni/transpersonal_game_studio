#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/World.h"
#include "TimerManager.h"

/**
 * ENGINE ARCHITECT CYCLE 005 - ARCHITECTURAL FRAMEWORK IMPLEMENTATION
 * 
 * This implements the foundational architectural framework that:
 * 1. Manages system registration and lifecycle
 * 2. Enforces cross-system communication protocols
 * 3. Monitors performance and validates integrity
 * 4. Resolves module dependencies
 * 5. Ensures compilation safety
 */

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Initializing Architectural Framework"));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f; // 5 seconds
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Clear any existing systems
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Register core architectural systems
    RegisterCoreArchitecturalSystems();
    
    // Start performance monitoring timer
    if (bEnableRuntimeValidation)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimer,
            this,
            &UEng_ArchitecturalFramework::UpdatePerformanceMetrics,
            PerformanceValidationInterval,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework initialized successfully"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Deinitializing Architectural Framework"));
    
    // Clear validation timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsValidHandle(ValidationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final statistics
    LogArchitecturalEvent(TEXT("SHUTDOWN"), 
        FString::Printf(TEXT("Systems: %d, Errors: %d"), 
            RegisteredSystems.Num(), TotalValidationErrors));
    
    // Clear all systems
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, const TArray<EEng_ModuleDependency>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System %s already registered, updating"), *SystemName);
    }
    
    // Create system registration
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    // Validate dependencies
    bool bDependenciesValid = ValidateDependencies(Dependencies);
    if (!bDependenciesValid)
    {
        NewSystem.Status = EEng_SystemStatus::Error;
        NewSystem.ValidationErrors++;
        TotalValidationErrors++;
    }
    
    // Register the system
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("REGISTER_SYSTEM"), 
        FString::Printf(TEXT("System: %s, Dependencies: %d, Valid: %s"), 
            *SystemName, Dependencies.Num(), bDependenciesValid ? TEXT("Yes") : TEXT("No")));
    
    return bDependenciesValid;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cannot unregister unknown system %s"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    
    LogArchitecturalEvent(TEXT("UNREGISTER_SYSTEM"), 
        FString::Printf(TEXT("System: %s"), *SystemName));
    
    return true;
}

EEng_SystemStatus UEng_ArchitecturalFramework::GetSystemStatus(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Status;
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
    int32 ErrorCount = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Reset validation
        System.LastValidation = FDateTime::Now();
        int32 PreviousErrors = System.ValidationErrors;
        System.ValidationErrors = 0;
        
        // Validate dependencies
        if (!ValidateDependencies(System.Dependencies))
        {
            System.ValidationErrors++;
            System.Status = EEng_SystemStatus::Error;
            bAllSystemsValid = false;
        }
        
        // Update error counts
        if (System.ValidationErrors > PreviousErrors)
        {
            ErrorCount += (System.ValidationErrors - PreviousErrors);
        }
    }
    
    TotalValidationErrors += ErrorCount;
    
    LogArchitecturalEvent(TEXT("VALIDATE_INTEGRITY"), 
        FString::Printf(TEXT("Valid: %s, New Errors: %d"), 
            bAllSystemsValid ? TEXT("Yes") : TEXT("No"), ErrorCount));
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // This validates that all module dependencies are properly resolved
    bool bAllDependenciesResolved = true;
    
    // Check each registered system's dependencies
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        
        for (EEng_ModuleDependency Dependency : System.Dependencies)
        {
            if (!IsModuleDependencyResolved(Dependency))
            {
                bAllDependenciesResolved = false;
                UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Unresolved dependency for system %s"), *System.SystemName);
            }
        }
    }
    
    return bAllDependenciesResolved;
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
    bool bFrameTimeOK = CurrentMetrics.FrameTime <= 20.0f; // Allow 20ms tolerance
    
    // Check actor count (reasonable limit for survival game)
    bool bActorCountOK = CurrentMetrics.ActiveActors <= 10000;
    
    // Check memory usage (reasonable limit)
    bool bMemoryOK = CurrentMetrics.MemoryUsageMB <= 4096.0f; // 4GB limit
    
    return bFrameTimeOK && bActorCountOK && bMemoryOK;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    // This validates that all types are properly registered and no conflicts exist
    CompilationErrors.Empty();
    
    // Check for type conflicts in SharedTypes.h
    bool bTypesValid = true;
    
    // Validate enum registrations
    if (!ValidateEnumRegistry())
    {
        bTypesValid = false;
        CompilationErrors.Add(TEXT("Enum registry validation failed"));
    }
    
    // Validate struct registrations
    if (!ValidateStructRegistry())
    {
        bTypesValid = false;
        CompilationErrors.Add(TEXT("Struct registry validation failed"));
    }
    
    LogArchitecturalEvent(TEXT("VALIDATE_TYPES"), 
        FString::Printf(TEXT("Valid: %s, Errors: %d"), 
            bTypesValid ? TEXT("Yes") : TEXT("No"), CompilationErrors.Num()));
    
    return bTypesValid;
}

TArray<FString> UEng_ArchitecturalFramework::GetCompilationErrors()
{
    return CompilationErrors;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Update frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Update thread times (approximation)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f;
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.4f;
    
    // Update actor count
    if (GetWorld())
    {
        CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    }
    
    // Update physics bodies (approximation)
    CurrentMetrics.PhysicsBodies = CurrentMetrics.ActiveActors / 4; // Rough estimate
    
    // Update memory usage (approximation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    // This is called periodically to ensure system dependencies remain valid
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        if (System.Status == EEng_SystemStatus::Active)
        {
            if (!ValidateDependencies(System.Dependencies))
            {
                System.Status = EEng_SystemStatus::Error;
                System.ValidationErrors++;
                TotalValidationErrors++;
            }
        }
    }
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("ENGINE ARCHITECT [%s]: %s"), *Event, *Details);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

void UEng_ArchitecturalFramework::RegisterCoreArchitecturalSystems()
{
    // Register the core systems that are always required
    TArray<EEng_ModuleDependency> CoreDeps;
    CoreDeps.Add(EEng_ModuleDependency::Core);
    
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDeps);
    
    TArray<EEng_ModuleDependency> PhysicsDeps;
    PhysicsDeps.Add(EEng_ModuleDependency::Core);
    PhysicsDeps.Add(EEng_ModuleDependency::Physics);
    
    RegisterSystem(TEXT("PhysicsCore"), PhysicsDeps);
    
    TArray<EEng_ModuleDependency> WorldDeps;
    WorldDeps.Add(EEng_ModuleDependency::Core);
    WorldDeps.Add(EEng_ModuleDependency::World);
    
    RegisterSystem(TEXT("WorldGeneration"), WorldDeps);
}

bool UEng_ArchitecturalFramework::ValidateDependencies(const TArray<EEng_ModuleDependency>& Dependencies)
{
    // Validate that all dependencies can be resolved
    for (EEng_ModuleDependency Dependency : Dependencies)
    {
        if (!IsModuleDependencyResolved(Dependency))
        {
            return false;
        }
    }
    
    return true;
}

bool UEng_ArchitecturalFramework::IsModuleDependencyResolved(EEng_ModuleDependency Dependency)
{
    // Check if the module dependency is available
    switch (Dependency)
    {
        case EEng_ModuleDependency::Core:
            return true; // Core is always available
            
        case EEng_ModuleDependency::Physics:
            return GEngine != nullptr; // Physics available if engine is loaded
            
        case EEng_ModuleDependency::World:
            return GetWorld() != nullptr; // World available if we have a world context
            
        case EEng_ModuleDependency::AI:
        case EEng_ModuleDependency::Character:
        case EEng_ModuleDependency::Combat:
        case EEng_ModuleDependency::Quest:
        case EEng_ModuleDependency::Audio:
        case EEng_ModuleDependency::VFX:
        case EEng_ModuleDependency::UI:
            return true; // These are considered available for now
            
        default:
            return false;
    }
}

bool UEng_ArchitecturalFramework::ValidateEnumRegistry()
{
    // Validate that all enums are properly registered
    // This is a placeholder for more sophisticated validation
    return true;
}

bool UEng_ArchitecturalFramework::ValidateStructRegistry()
{
    // Validate that all structs are properly registered
    // This is a placeholder for more sophisticated validation
    return true;
}
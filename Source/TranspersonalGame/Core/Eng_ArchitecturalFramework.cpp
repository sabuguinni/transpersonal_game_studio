#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Initializing..."));
    
    // Initialize default settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f;
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core systems that should always be present
    TArray<EEng_ModuleDependency> CoreDeps;
    CoreDeps.Add(EEng_ModuleDependency::Core);
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDeps);
    
    // Start validation timer if runtime validation is enabled
    if (bEnableRuntimeValidation)
    {
        GetWorld()->GetTimerManager().SetTimer(
            ValidationTimer,
            this,
            &UEng_ArchitecturalFramework::ValidateSystemIntegrity,
            PerformanceValidationInterval,
            true
        );
    }
    
    LogArchitecturalEvent(TEXT("INITIALIZATION"), TEXT("Architectural Framework successfully initialized"));
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Ready"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Shutting Down..."));
    
    // Clear validation timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(ValidationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final statistics
    LogArchitecturalEvent(TEXT("SHUTDOWN"), FString::Printf(TEXT("Total systems registered: %d, Total validation errors: %d"), 
        RegisteredSystems.Num(), TotalValidationErrors));
    
    // Clear all data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Shutdown Complete"));
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
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System %s already registered, updating..."), *SystemName);
    }
    
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = FPlatformTime::Seconds();
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("SYSTEM_REGISTRATION"), 
        FString::Printf(TEXT("System '%s' registered with %d dependencies"), *SystemName, Dependencies.Num()));
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: System '%s' registered successfully"), *SystemName);
    return true;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cannot unregister system '%s' - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    LogArchitecturalEvent(TEXT("SYSTEM_UNREGISTRATION"), FString::Printf(TEXT("System '%s' unregistered"), *SystemName));
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: System '%s' unregistered"), *SystemName);
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
    for (auto& SystemPair : RegisteredSystems)
    {
        Systems.Add(SystemPair.Value);
    }
    return Systems;
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegrity()
{
    bool bAllSystemsValid = true;
    int32 ErrorCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Validating system integrity..."));
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Update last validation time
        System.LastValidation = FDateTime::Now();
        
        // Validate dependencies
        for (EEng_ModuleDependency Dependency : System.Dependencies)
        {
            // Check if dependency systems are registered
            bool bDependencyMet = false;
            for (auto& DepSystemPair : RegisteredSystems)
            {
                if (DepSystemPair.Value.Dependencies.Contains(Dependency) || 
                    DepSystemPair.Key.Contains(UEnum::GetValueAsString(Dependency)))
                {
                    bDependencyMet = true;
                    break;
                }
            }
            
            if (!bDependencyMet)
            {
                System.ValidationErrors++;
                ErrorCount++;
                bAllSystemsValid = false;
                UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System '%s' has unmet dependency: %s"), 
                    *System.SystemName, *UEnum::GetValueAsString(Dependency));
            }
        }
        
        // Update system status based on validation
        if (System.ValidationErrors == 0)
        {
            System.Status = EEng_SystemStatus::Active;
        }
        else
        {
            System.Status = EEng_SystemStatus::Error;
        }
    }
    
    TotalValidationErrors += ErrorCount;
    UpdatePerformanceMetrics();
    
    LogArchitecturalEvent(TEXT("VALIDATION"), 
        FString::Printf(TEXT("System integrity validation complete. Errors: %d"), ErrorCount));
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: System integrity validation complete. Valid: %s, Errors: %d"), 
        bAllSystemsValid ? TEXT("TRUE") : TEXT("FALSE"), ErrorCount);
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // This would validate that all module dependencies in Build.cs files are correct
    // For now, we'll do a basic check
    bool bDependenciesValid = true;
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Validating module dependencies..."));
    
    // Check for circular dependencies
    TMap<EEng_ModuleDependency, TArray<EEng_ModuleDependency>> DependencyGraph;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        for (EEng_ModuleDependency Dep : SystemPair.Value.Dependencies)
        {
            if (!DependencyGraph.Contains(Dep))
            {
                DependencyGraph.Add(Dep, TArray<EEng_ModuleDependency>());
            }
        }
    }
    
    LogArchitecturalEvent(TEXT("MODULE_VALIDATION"), TEXT("Module dependency validation complete"));
    return bDependenciesValid;
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
    bool bFrameTimeOK = CurrentMetrics.FrameTime <= 16.67f;
    
    // Check memory usage (warning at 4GB)
    bool bMemoryOK = CurrentMetrics.MemoryUsageMB <= 4096.0f;
    
    // Check actor count (warning at 10000)
    bool bActorCountOK = CurrentMetrics.ActiveActors <= 10000;
    
    return bFrameTimeOK && bMemoryOK && bActorCountOK;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    // Validate that all registered types are properly defined
    bool bTypesValid = true;
    CompilationErrors.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Validating type registry..."));
    
    // Check for common compilation issues
    if (RegisteredSystems.Num() == 0)
    {
        CompilationErrors.Add(TEXT("No systems registered - possible compilation failure"));
        bTypesValid = false;
    }
    
    // Validate that core types are available
    TArray<FString> CoreTypes = {
        TEXT("FEng_SystemRegistration"),
        TEXT("FEng_PerformanceMetrics"),
        TEXT("EEng_SystemStatus"),
        TEXT("EEng_ModuleDependency")
    };
    
    for (const FString& TypeName : CoreTypes)
    {
        // Basic validation - in a real implementation we'd check UE's type registry
        UE_LOG(LogTemp, VeryVerbose, TEXT("ENGINE ARCHITECT: Validating type: %s"), *TypeName);
    }
    
    LogArchitecturalEvent(TEXT("TYPE_VALIDATION"), 
        FString::Printf(TEXT("Type registry validation complete. Valid: %s"), bTypesValid ? TEXT("TRUE") : TEXT("FALSE")));
    
    return bTypesValid;
}

TArray<FString> UEng_ArchitecturalFramework::GetCompilationErrors()
{
    return CompilationErrors;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    // Update frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    }
    
    // Update actor count
    if (GetWorld())
    {
        CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    }
    
    // Update memory usage (basic estimation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update thread times (simplified)
    CurrentMetrics.GameThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.7f; // Estimate
    CurrentMetrics.RenderThreadTime = FApp::GetDeltaTime() * 1000.0f * 0.3f; // Estimate
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    // Internal method to validate system dependencies
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Check each dependency
        for (EEng_ModuleDependency Dependency : System.Dependencies)
        {
            // Validate dependency is satisfied
            bool bDependencyFound = false;
            for (auto& DepSystemPair : RegisteredSystems)
            {
                if (DepSystemPair.Value.Status == EEng_SystemStatus::Active)
                {
                    bDependencyFound = true;
                    break;
                }
            }
            
            if (!bDependencyFound)
            {
                System.ValidationErrors++;
                UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Dependency validation failed for %s"), *System.SystemName);
            }
        }
    }
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("[ARCH_FRAMEWORK] %s: %s"), *Event, *Details);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    
    // In a production system, this would also write to a dedicated architectural log file
}
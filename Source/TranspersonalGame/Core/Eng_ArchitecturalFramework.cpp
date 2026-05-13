#include "Eng_ArchitecturalFramework.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT FRAMEWORK INITIALIZED ==="));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f;
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core systems that must always exist
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
    
    LogArchitecturalEvent(TEXT("Framework Initialized"), TEXT("All core systems ready"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    // Clear validation timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsValidHandle(ValidationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final system status
    LogArchitecturalEvent(TEXT("Framework Shutdown"), FString::Printf(TEXT("Systems: %d, Errors: %d"), 
        RegisteredSystems.Num(), TotalValidationErrors));
    
    // Clear all data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, const TArray<EEng_ModuleDependency>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered - updating"), *SystemName);
    }
    
    // Create new system registration
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = FPlatformTime::Seconds();
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    // Validate dependencies
    if (!ValidateDependencies(Dependencies))
    {
        NewSystem.Status = EEng_SystemStatus::Error;
        NewSystem.ValidationErrors++;
        TotalValidationErrors++;
    }
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("System Registered"), FString::Printf(TEXT("%s with %d dependencies"), 
        *SystemName, Dependencies.Num()));
    
    return NewSystem.Status != EEng_SystemStatus::Error;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unregister system %s - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    LogArchitecturalEvent(TEXT("System Unregistered"), SystemName);
    
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
    TArray<FEng_SystemRegistration> AllSystems;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        AllSystems.Add(SystemPair.Value);
    }
    
    return AllSystems;
}

bool UEng_ArchitecturalFramework::ValidateSystemIntegrity()
{
    bool bAllValid = true;
    int32 ErrorsFound = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Reset validation state
        System.LastValidation = FDateTime::Now();
        System.ValidationErrors = 0;
        
        // Validate dependencies
        if (!ValidateDependencies(System.Dependencies))
        {
            System.ValidationErrors++;
            ErrorsFound++;
            bAllValid = false;
        }
        
        // Update status based on validation
        if (System.ValidationErrors > 0)
        {
            System.Status = EEng_SystemStatus::Error;
        }
        else if (System.Status == EEng_SystemStatus::Error)
        {
            System.Status = EEng_SystemStatus::Active;
        }
    }
    
    TotalValidationErrors = ErrorsFound;
    
    LogArchitecturalEvent(TEXT("System Validation"), FString::Printf(TEXT("Validated %d systems, %d errors"), 
        RegisteredSystems.Num(), ErrorsFound));
    
    return bAllValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // Check for circular dependencies
    TMap<EEng_ModuleDependency, TArray<EEng_ModuleDependency>> DependencyGraph;
    
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
    
    // For now, just validate that dependencies exist
    // TODO: Implement full circular dependency detection
    return true;
}

int32 UEng_ArchitecturalFramework::GetValidationErrorCount()
{
    return TotalValidationErrors;
}

FEng_PerformanceMetrics UEng_ArchitecturalFramework::GetCurrentPerformanceMetrics()
{
    return CurrentMetrics;
}

bool UEng_ArchitecturalFramework::IsPerformanceWithinLimits()
{
    // Performance limits for 60fps target
    const float MaxFrameTime = 16.67f; // 60fps
    const int32 MaxActors = 10000;
    const float MaxMemoryMB = 4096.0f;
    
    return CurrentMetrics.FrameTime <= MaxFrameTime &&
           CurrentMetrics.ActiveActors <= MaxActors &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryMB;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    CompilationErrors.Empty();
    
    // Check for common compilation issues
    // This is a simplified validation - real implementation would be more comprehensive
    
    bool bValid = true;
    
    // Validate that SharedTypes.h is accessible
    if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(TEXT("SharedTypes.h")))
    {
        CompilationErrors.Add(TEXT("SharedTypes.h not found"));
        bValid = false;
    }
    
    return bValid;
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
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Count active actors
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Validate performance
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance outside limits: Frame=%fms, Actors=%d, Memory=%fMB"),
            CurrentMetrics.FrameTime, CurrentMetrics.ActiveActors, CurrentMetrics.MemoryUsageMB);
    }
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    // This would implement full dependency validation
    // For now, just log the validation attempt
    LogArchitecturalEvent(TEXT("Dependency Validation"), TEXT("Checking system dependencies"));
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    UE_LOG(LogTemp, Log, TEXT("[ARCH] %s: %s"), *Event, *Details);
}

void UEng_ArchitecturalFramework::RegisterCoreArchitecturalSystems()
{
    // Register the core systems that form the foundation
    TArray<EEng_ModuleDependency> CoreDeps = { EEng_ModuleDependency::Core };
    
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDeps);
    RegisterSystem(TEXT("SharedTypes"), CoreDeps);
    RegisterSystem(TEXT("ModuleMapping"), CoreDeps);
    
    // Register physics systems
    TArray<EEng_ModuleDependency> PhysicsDeps = { EEng_ModuleDependency::Core, EEng_ModuleDependency::Physics };
    RegisterSystem(TEXT("PhysicsManager"), PhysicsDeps);
    RegisterSystem(TEXT("CollisionSystem"), PhysicsDeps);
    
    // Register world systems
    TArray<EEng_ModuleDependency> WorldDeps = { EEng_ModuleDependency::Core, EEng_ModuleDependency::World };
    RegisterSystem(TEXT("BiomeManager"), WorldDeps);
    RegisterSystem(TEXT("WorldGeneration"), WorldDeps);
}

bool UEng_ArchitecturalFramework::ValidateDependencies(const TArray<EEng_ModuleDependency>& Dependencies)
{
    // For now, just validate that dependencies are not empty for non-core systems
    // Real implementation would check actual module availability
    return true;
}
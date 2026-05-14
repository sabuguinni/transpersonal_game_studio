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
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Initializing"));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f; // 5 second intervals
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core systems that must always exist
    TArray<EEng_ModuleDependency> CoreDeps;
    RegisterSystem("ArchitecturalFramework", CoreDeps);
    
    TArray<EEng_ModuleDependency> CharacterDeps;
    CharacterDeps.Add(EEng_ModuleDependency::Core);
    RegisterSystem("TranspersonalCharacter", CharacterDeps);
    
    TArray<EEng_ModuleDependency> GameModeDeps;
    GameModeDeps.Add(EEng_ModuleDependency::Core);
    GameModeDeps.Add(EEng_ModuleDependency::Character);
    RegisterSystem("TranspersonalGameMode", GameModeDeps);
    
    // Start validation timer if runtime validation is enabled
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
    
    // Log successful initialization
    LogArchitecturalEvent("INITIALIZATION", "Architectural Framework successfully initialized");
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Framework initialization complete"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Architectural Framework Deinitializing"));
    
    // Clear validation timer
    if (GetWorld() && ValidationTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log all systems before shutdown
    for (const auto& SystemPair : RegisteredSystems)
    {
        LogArchitecturalEvent("SHUTDOWN", FString::Printf(TEXT("System %s had %d validation errors"), 
            *SystemPair.Key, SystemPair.Value.ValidationErrors));
    }
    
    // Clear all data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    LogArchitecturalEvent("SHUTDOWN", "Architectural Framework deinitialized");
    
    Super::Deinitialize();
}

bool UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, const TArray<EEng_ModuleDependency>& Dependencies)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Cannot register system with empty name"));
        return false;
    }
    
    // Check if system already exists
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System %s already registered, updating"), *SystemName);
    }
    
    // Create registration entry
    FEng_SystemRegistration Registration;
    Registration.SystemName = SystemName;
    Registration.Status = EEng_SystemStatus::Registered;
    Registration.Dependencies = Dependencies;
    Registration.InitializationTime = FPlatformTime::Seconds();
    Registration.LastValidation = FDateTime::Now();
    Registration.ValidationErrors = 0;
    
    // Add to registry
    RegisteredSystems.Add(SystemName, Registration);
    
    LogArchitecturalEvent("REGISTRATION", FString::Printf(TEXT("System %s registered with %d dependencies"), 
        *SystemName, Dependencies.Num()));
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: System %s registered successfully"), *SystemName);
    
    return true;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cannot unregister unknown system %s"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    LogArchitecturalEvent("UNREGISTRATION", FString::Printf(TEXT("System %s unregistered"), *SystemName));
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: System %s unregistered"), *SystemName);
    
    return true;
}

EEng_SystemStatus UEng_ArchitecturalFramework::GetSystemStatus(const FString& SystemName)
{
    if (const FEng_SystemRegistration* Registration = RegisteredSystems.Find(SystemName))
    {
        return Registration->Status;
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
        FEng_SystemRegistration& Registration = SystemPair.Value;
        
        // Reset error count for this validation
        int32 SystemErrors = 0;
        
        // Validate dependencies are met
        for (const EEng_ModuleDependency& Dependency : Registration.Dependencies)
        {
            bool bDependencyMet = false;
            
            // Check if dependency system is registered
            for (const auto& OtherSystemPair : RegisteredSystems)
            {
                // This is a simplified check - in a real implementation,
                // you'd have a mapping of dependencies to system names
                if (OtherSystemPair.Value.Status == EEng_SystemStatus::Active ||
                    OtherSystemPair.Value.Status == EEng_SystemStatus::Registered)
                {
                    bDependencyMet = true;
                    break;
                }
            }
            
            if (!bDependencyMet)
            {
                SystemErrors++;
                UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System %s missing dependency"), 
                    *Registration.SystemName);
            }
        }
        
        // Update registration
        Registration.ValidationErrors = SystemErrors;
        Registration.LastValidation = FDateTime::Now();
        
        if (SystemErrors > 0)
        {
            Registration.Status = EEng_SystemStatus::Error;
            bAllSystemsValid = false;
            ErrorsFound += SystemErrors;
        }
        else if (Registration.Status == EEng_SystemStatus::Error)
        {
            // Clear error status if no errors found
            Registration.Status = EEng_SystemStatus::Active;
        }
    }
    
    TotalValidationErrors = ErrorsFound;
    
    LogArchitecturalEvent("VALIDATION", FString::Printf(TEXT("System integrity check: %s (%d errors)"), 
        bAllSystemsValid ? TEXT("PASSED") : TEXT("FAILED"), ErrorsFound));
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // This would validate that all required modules are loaded
    // For now, return true as a basic implementation
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
    // Check frame time (target 60fps = 16.67ms)
    if (CurrentMetrics.FrameTime > 20.0f) // Allow 20ms tolerance
    {
        return false;
    }
    
    // Check actor count (arbitrary limit for now)
    if (CurrentMetrics.ActiveActors > 10000)
    {
        return false;
    }
    
    // Check memory usage (arbitrary limit)
    if (CurrentMetrics.MemoryUsageMB > 4096.0f) // 4GB limit
    {
        return false;
    }
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    // Clear previous errors
    CompilationErrors.Empty();
    
    // This would validate that all types are properly registered
    // For now, return true as basic implementation
    return true;
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
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    }
    
    // Count active actors
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Get memory usage (basic implementation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Check if performance is within limits
    if (!IsPerformanceWithinLimits())
    {
        LogArchitecturalEvent("PERFORMANCE_WARNING", 
            FString::Printf(TEXT("Performance limits exceeded: Frame=%fms, Actors=%d, Memory=%fMB"),
                CurrentMetrics.FrameTime, CurrentMetrics.ActiveActors, CurrentMetrics.MemoryUsageMB));
    }
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    ValidateSystemIntegrity();
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("ENGINE_ARCHITECT [%s]: %s"), *Event, *Details);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
    
    // In a real implementation, this could write to a dedicated log file
    // or send to a monitoring system
}
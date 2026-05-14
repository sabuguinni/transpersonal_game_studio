#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

/**
 * ENGINE ARCHITECT CYCLE 001 - ARCHITECTURAL FRAMEWORK IMPLEMENTATION
 * 
 * This implements the core architectural framework that all other systems
 * must follow. This is the foundation that ensures:
 * 1. Proper system registration and lifecycle
 * 2. Performance monitoring and validation
 * 3. Module dependency resolution
 * 4. Compilation safety and error tracking
 */

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT FRAMEWORK INITIALIZING ==="));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 1.0f;
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Register core architectural system
    TArray<EEng_ModuleDependency> CoreDependencies;
    CoreDependencies.Add(EEng_ModuleDependency::Core);
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDependencies);
    
    // Start validation timer if runtime validation is enabled
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
    
    UE_LOG(LogTemp, Warning, TEXT("✓ Architectural Framework initialized with %d systems"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT FRAMEWORK DEINITIALIZING ==="));
    
    // Clear validation timer
    if (GetWorld() && ValidationTimer.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final statistics
    UE_LOG(LogTemp, Warning, TEXT("Final system count: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Total validation errors: %d"), TotalValidationErrors);
    
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
        UE_LOG(LogTemp, Warning, TEXT("System '%s' already registered, updating..."), *SystemName);
    }
    
    // Create system registration
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = FPlatformTime::Seconds();
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    // Add to registry
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("SystemRegistered"), FString::Printf(TEXT("System: %s, Dependencies: %d"), *SystemName, Dependencies.Num()));
    
    UE_LOG(LogTemp, Warning, TEXT("✓ System registered: %s"), *SystemName);
    return true;
}

bool UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot unregister system '%s' - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    LogArchitecturalEvent(TEXT("SystemUnregistered"), FString::Printf(TEXT("System: %s"), *SystemName));
    
    UE_LOG(LogTemp, Warning, TEXT("✓ System unregistered: %s"), *SystemName);
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
    int32 ErrorCount = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Check if system has been validated recently
        FTimespan TimeSinceValidation = FDateTime::Now() - System.LastValidation;
        if (TimeSinceValidation.GetTotalMinutes() > 5.0)
        {
            System.ValidationErrors++;
            ErrorCount++;
            UE_LOG(LogTemp, Warning, TEXT("System '%s' validation overdue"), *System.SystemName);
        }
        
        // Update validation timestamp
        System.LastValidation = FDateTime::Now();
    }
    
    TotalValidationErrors += ErrorCount;
    
    if (ErrorCount == 0)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ System integrity validation passed"));
        return true;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ System integrity validation failed with %d errors"), ErrorCount);
        return false;
    }
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    // This would validate that all module dependencies are properly resolved
    // For now, we'll do a basic check
    
    bool bAllDependenciesValid = true;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        
        // Check if system has required dependencies
        if (System.Dependencies.Num() == 0 && System.SystemName != TEXT("ArchitecturalFramework"))
        {
            UE_LOG(LogTemp, Warning, TEXT("System '%s' has no dependencies - potential issue"), *System.SystemName);
            bAllDependenciesValid = false;
        }
    }
    
    if (bAllDependenciesValid)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Module dependency validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("✗ Module dependency validation has warnings"));
    }
    
    return bAllDependenciesValid;
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
    // Check if performance is within acceptable limits
    const float MaxFrameTime = 33.33f; // 30fps minimum
    const int32 MaxActors = 10000;
    const float MaxMemoryMB = 4096.0f;
    
    bool bWithinLimits = true;
    
    if (CurrentMetrics.FrameTime > MaxFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeded: %.2fms > %.2fms"), CurrentMetrics.FrameTime, MaxFrameTime);
        bWithinLimits = false;
    }
    
    if (CurrentMetrics.ActiveActors > MaxActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("Actor count exceeded: %d > %d"), CurrentMetrics.ActiveActors, MaxActors);
        bWithinLimits = false;
    }
    
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage exceeded: %.2fMB > %.2fMB"), CurrentMetrics.MemoryUsageMB, MaxMemoryMB);
        bWithinLimits = false;
    }
    
    return bWithinLimits;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    // This would validate that all UE5 types are properly registered
    // For now, we'll do a basic validation
    
    CompilationErrors.Empty();
    
    // Check if we can access basic UE5 types
    bool bTypesValid = true;
    
    try
    {
        // Test basic type access
        UClass* ActorClass = AActor::StaticClass();
        UClass* ComponentClass = UActorComponent::StaticClass();
        UClass* ObjectClass = UObject::StaticClass();
        
        if (!ActorClass || !ComponentClass || !ObjectClass)
        {
            CompilationErrors.Add(TEXT("Basic UE5 types not accessible"));
            bTypesValid = false;
        }
    }
    catch (...)
    {
        CompilationErrors.Add(TEXT("Exception during type validation"));
        bTypesValid = false;
    }
    
    if (bTypesValid)
    {
        UE_LOG(LogTemp, Log, TEXT("✓ Type registry validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("✗ Type registry validation failed"));
        TotalValidationErrors++;
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
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }
    
    // Update actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActors = AllActors.Num();
    
    // Update memory usage (approximate)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Log performance if validation level is high
    if (ValidationLevel >= EEng_ArchValidationLevel::Strict)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance: %.2fms, %d actors, %.2fMB"), 
               CurrentMetrics.FrameTime, CurrentMetrics.ActiveActors, CurrentMetrics.MemoryUsageMB);
    }
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    // This would perform deep dependency validation
    // Implementation would check circular dependencies, missing modules, etc.
    UE_LOG(LogTemp, Log, TEXT("Validating system dependencies..."));
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("[ARCH] %s: %s"), *Event, *Details);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    
    // In a production system, this could also log to file or external system
}
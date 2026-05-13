#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"

/**
 * ENGINE ARCHITECT CYCLE 004 - ARCHITECTURAL FRAMEWORK IMPLEMENTATION
 * 
 * This implements the foundational architectural framework that ensures:
 * 1. Proper system registration and lifecycle management
 * 2. Cross-system communication through SharedTypes.h
 * 3. Performance monitoring and validation
 * 4. Module dependency resolution
 * 5. Compilation safety and error tracking
 */

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Initializing Architectural Framework"));
    
    // Initialize validation settings
    ValidationLevel = EEng_ArchValidationLevel::Standard;
    PerformanceValidationInterval = 5.0f; // 5 second intervals
    bEnableRuntimeValidation = true;
    TotalValidationErrors = 0;
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    
    // Clear any existing systems
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    // Register core systems that must always exist
    RegisterCoreArchitecturalSystems();
    
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
    
    LogArchitecturalEvent(TEXT("Framework Initialized"), TEXT("All core systems registered"));
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Deinitializing Architectural Framework"));
    
    // Clear validation timer
    if (GetWorld() && GetWorld()->GetTimerManager().IsTimerActive(ValidationTimer))
    {
        GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    // Log final system status
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("System '%s' final status: %d"), *System.SystemName, (int32)System.Status);
    }
    
    // Clear all systems
    RegisteredSystems.Empty();
    CompilationErrors.Empty();
    
    LogArchitecturalEvent(TEXT("Framework Deinitialized"), TEXT("All systems unregistered"));
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::RegisterCoreArchitecturalSystems()
{
    // Register the architectural framework itself
    TArray<EEng_ModuleDependency> CoreDeps;
    CoreDeps.Add(EEng_ModuleDependency::Core);
    RegisterSystem(TEXT("ArchitecturalFramework"), CoreDeps);
    
    // Register expected core systems
    TArray<EEng_ModuleDependency> PhysicsDeps;
    PhysicsDeps.Add(EEng_ModuleDependency::Core);
    PhysicsDeps.Add(EEng_ModuleDependency::Physics);
    RegisterSystem(TEXT("PhysicsCore"), PhysicsDeps);
    
    TArray<EEng_ModuleDependency> WorldDeps;
    WorldDeps.Add(EEng_ModuleDependency::Core);
    WorldDeps.Add(EEng_ModuleDependency::World);
    RegisterSystem(TEXT("WorldGeneration"), WorldDeps);
    
    TArray<EEng_ModuleDependency> CharacterDeps;
    CharacterDeps.Add(EEng_ModuleDependency::Core);
    CharacterDeps.Add(EEng_ModuleDependency::Physics);
    CharacterDeps.Add(EEng_ModuleDependency::Character);
    RegisterSystem(TEXT("CharacterSystem"), CharacterDeps);
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
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System '%s' already registered, updating..."), *SystemName);
    }
    
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Status = EEng_SystemStatus::Registered;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.LastValidation = FDateTime::Now();
    NewSystem.ValidationErrors = 0;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    LogArchitecturalEvent(TEXT("System Registered"), FString::Printf(TEXT("System: %s, Dependencies: %d"), *SystemName, Dependencies.Num()));
    
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
    LogArchitecturalEvent(TEXT("System Unregistered"), FString::Printf(TEXT("System: %s"), *SystemName));
    
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
    int32 ErrorCount = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemRegistration& System = SystemPair.Value;
        
        // Update validation timestamp
        System.LastValidation = FDateTime::Now();
        
        // Reset error count for this validation
        System.ValidationErrors = 0;
        
        // Validate dependencies
        for (const EEng_ModuleDependency& Dependency : System.Dependencies)
        {
            if (!ValidateModuleDependency(Dependency))
            {
                System.ValidationErrors++;
                ErrorCount++;
                bAllSystemsValid = false;
                
                UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: System '%s' has invalid dependency: %d"), 
                    *System.SystemName, (int32)Dependency);
            }
        }
        
        // Update system status based on validation
        if (System.ValidationErrors > 0)
        {
            System.Status = EEng_SystemStatus::Error;
        }
        else if (System.Status == EEng_SystemStatus::Error)
        {
            System.Status = EEng_SystemStatus::Active; // Recover from error state
        }
    }
    
    TotalValidationErrors = ErrorCount;
    
    LogArchitecturalEvent(TEXT("System Validation"), 
        FString::Printf(TEXT("Validated %d systems, %d errors"), RegisteredSystems.Num(), ErrorCount));
    
    return bAllSystemsValid;
}

bool UEng_ArchitecturalFramework::ValidateModuleDependency(EEng_ModuleDependency Dependency)
{
    // This would normally check if the module is properly loaded and available
    // For now, we'll do basic validation
    
    switch (Dependency)
    {
        case EEng_ModuleDependency::Core:
            return true; // Core is always available
            
        case EEng_ModuleDependency::Physics:
            // Check if physics world exists
            return GetWorld() && GetWorld()->GetPhysicsScene();
            
        case EEng_ModuleDependency::World:
            // Check if world exists
            return GetWorld() != nullptr;
            
        case EEng_ModuleDependency::AI:
        case EEng_ModuleDependency::Character:
        case EEng_ModuleDependency::Combat:
        case EEng_ModuleDependency::Quest:
        case EEng_ModuleDependency::Audio:
        case EEng_ModuleDependency::VFX:
        case EEng_ModuleDependency::UI:
            // These modules are optional for now
            return true;
            
        default:
            return false;
    }
}

bool UEng_ArchitecturalFramework::ValidateModuleDependencies()
{
    return ValidateSystemIntegrity();
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
    
    // Check memory usage (warn at 2GB)
    if (CurrentMetrics.MemoryUsageMB > 2048.0f)
    {
        return false;
    }
    
    // Check actor count (warn at 10000)
    if (CurrentMetrics.ActiveActors > 10000)
    {
        return false;
    }
    
    return true;
}

bool UEng_ArchitecturalFramework::ValidateTypeRegistry()
{
    // Clear previous compilation errors
    CompilationErrors.Empty();
    
    // This would normally validate that all USTRUCT/UENUM/UCLASS types
    // are properly registered and don't conflict
    
    // For now, we'll do basic validation
    bool bTypesValid = true;
    
    // Check if SharedTypes.h types are available
    // This is a placeholder - in a real implementation we'd use reflection
    
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
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }
    
    // Update actor count
    CurrentMetrics.ActiveActors = GetWorld()->GetActorCount();
    
    // Update memory usage (basic estimation)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update physics bodies count
    if (GetWorld()->GetPhysicsScene())
    {
        // This would need proper physics scene access
        CurrentMetrics.PhysicsBodies = 0; // Placeholder
    }
    
    // Check performance limits and log warnings
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Performance outside limits - Frame: %.2fms, Memory: %.1fMB, Actors: %d"),
            CurrentMetrics.FrameTime, CurrentMetrics.MemoryUsageMB, CurrentMetrics.ActiveActors);
    }
}

void UEng_ArchitecturalFramework::ValidateSystemDependencies()
{
    ValidateSystemIntegrity();
}

void UEng_ArchitecturalFramework::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("ENGINE ARCHITECT [%s]: %s - %s"), 
        *FDateTime::Now().ToString(), *Event, *Details);
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}
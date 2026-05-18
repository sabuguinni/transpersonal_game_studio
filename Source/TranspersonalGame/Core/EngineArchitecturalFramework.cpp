#include "EngineArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"

UEngineArchitecturalFramework::UEngineArchitecturalFramework()
{
    bIsInitialized = false;
    TotalPerformanceBudget = 16.67f; // 60fps target
    ValidationFrameCounter = 0;
}

void UEngineArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Initializing..."));
    
    // Initialize core architectural systems
    RegisteredSystems.Empty();
    ArchitecturalWarnings.Empty();
    ArchitecturalErrors.Empty();
    
    // Register core engine systems by default
    FEng_SystemInfo EngineSystem;
    EngineSystem.SystemName = TEXT("EngineCore");
    EngineSystem.Priority = EEng_SystemPriority::Critical;
    EngineSystem.Layer = EEng_ArchitecturalLayer::Engine;
    EngineSystem.PerformanceBudget = 5.0f; // 5ms budget for engine core
    RegisterSystem(EngineSystem);
    
    FEng_SystemInfo PhysicsSystem;
    PhysicsSystem.SystemName = TEXT("PhysicsCore");
    PhysicsSystem.Priority = EEng_SystemPriority::Critical;
    PhysicsSystem.Layer = EEng_ArchitecturalLayer::Core;
    PhysicsSystem.PerformanceBudget = 3.0f; // 3ms budget for physics
    RegisterSystem(PhysicsSystem);
    
    FEng_SystemInfo RenderingSystem;
    RenderingSystem.SystemName = TEXT("RenderingCore");
    RenderingSystem.Priority = EEng_SystemPriority::Critical;
    RenderingSystem.Layer = EEng_ArchitecturalLayer::Engine;
    RenderingSystem.PerformanceBudget = 8.0f; // 8ms budget for rendering
    RegisterSystem(RenderingSystem);
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Initialization complete"));
}

void UEngineArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: Shutting down..."));
    
    RegisteredSystems.Empty();
    ArchitecturalWarnings.Empty();
    ArchitecturalErrors.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UEngineArchitecturalFramework::RegisterSystem(const FEng_SystemInfo& SystemInfo)
{
    if (!bIsInitialized)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: Cannot register system - framework not initialized"));
        return false;
    }
    
    if (SystemInfo.SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: Cannot register system with empty name"));
        return false;
    }
    
    // Validate system architecture before registration
    if (!ValidateSystemLayer(SystemInfo) || !ValidateSystemPriority(SystemInfo) || !ValidatePerformanceBudget(SystemInfo))
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitecturalFramework: System validation failed for %s"), *SystemInfo.SystemName);
        return false;
    }
    
    // Check for duplicate registration
    if (RegisteredSystems.Contains(SystemInfo.SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitecturalFramework: System %s already registered - updating"), *SystemInfo.SystemName);
    }
    
    RegisteredSystems.Add(SystemInfo.SystemName, SystemInfo);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitecturalFramework: Registered system %s"), *SystemInfo.SystemName);
    
    return true;
}

bool UEngineArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (!bIsInitialized)
    {
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("EngineArchitecturalFramework: Unregistered system %s"), *SystemName);
        return true;
    }
    
    return false;
}

TArray<FEng_SystemInfo> UEngineArchitecturalFramework::GetRegisteredSystems() const
{
    TArray<FEng_SystemInfo> Systems;
    RegisteredSystems.GenerateValueArray(Systems);
    return Systems;
}

void UEngineArchitecturalFramework::UpdatePerformanceMetrics(const FEng_PerformanceMetrics& Metrics)
{
    CurrentMetrics = Metrics;
    
    // Check for performance budget violations
    if (CurrentMetrics.FrameTime > TotalPerformanceBudget)
    {
        FString Warning = FString::Printf(TEXT("Performance budget exceeded: %.2fms > %.2fms"), 
            CurrentMetrics.FrameTime, TotalPerformanceBudget);
        ArchitecturalWarnings.AddUnique(Warning);
    }
    
    ValidationFrameCounter++;
    if (ValidationFrameCounter % 60 == 0) // Validate every 60 frames
    {
        UpdateValidationStatus();
    }
}

FEng_PerformanceMetrics UEngineArchitecturalFramework::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

EEng_ValidationStatus UEngineArchitecturalFramework::ValidateSystemArchitecture(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return EEng_ValidationStatus::Error;
    }
    
    const FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Validate system dependencies
    if (!ValidateSystemDependencies(SystemName))
    {
        return EEng_ValidationStatus::Warning;
    }
    
    // Validate performance budget
    if (!ValidatePerformanceBudget(SystemInfo))
    {
        return EEng_ValidationStatus::Warning;
    }
    
    return EEng_ValidationStatus::Valid;
}

bool UEngineArchitecturalFramework::EnforceArchitecturalCompliance()
{
    bool bCompliant = true;
    ArchitecturalErrors.Empty();
    ArchitecturalWarnings.Empty();
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        EEng_ValidationStatus Status = ValidateSystemArchitecture(SystemName);
        if (Status == EEng_ValidationStatus::Error || Status == EEng_ValidationStatus::Critical)
        {
            bCompliant = false;
            FString Error = FString::Printf(TEXT("System %s failed architectural compliance"), *SystemName);
            ArchitecturalErrors.AddUnique(Error);
        }
        else if (Status == EEng_ValidationStatus::Warning)
        {
            FString Warning = FString::Printf(TEXT("System %s has architectural warnings"), *SystemName);
            ArchitecturalWarnings.AddUnique(Warning);
        }
    }
    
    return bCompliant;
}

bool UEngineArchitecturalFramework::ValidateSystemDependencies(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    const FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Check if all dependencies are registered
    for (const FString& Dependency : SystemInfo.Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            FString Error = FString::Printf(TEXT("System %s depends on unregistered system %s"), 
                *SystemName, *Dependency);
            ArchitecturalErrors.AddUnique(Error);
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEngineArchitecturalFramework::GetSystemDependencies(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Dependencies;
    }
    return TArray<FString>();
}

bool UEngineArchitecturalFramework::SetSystemPerformanceBudget(const FString& SystemName, float BudgetMs)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        return false;
    }
    
    if (BudgetMs <= 0.0f || BudgetMs > TotalPerformanceBudget)
    {
        return false;
    }
    
    RegisteredSystems[SystemName].PerformanceBudget = BudgetMs;
    return true;
}

float UEngineArchitecturalFramework::GetSystemPerformanceBudget(const FString& SystemName) const
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].PerformanceBudget;
    }
    return 0.0f;
}

void UEngineArchitecturalFramework::LogArchitecturalStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURAL STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Architectural Warnings: %d"), ArchitecturalWarnings.Num());
    UE_LOG(LogTemp, Warning, TEXT("Architectural Errors: %d"), ArchitecturalErrors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Current Frame Time: %.2fms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Performance Budget: %.2fms"), TotalPerformanceBudget);
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("System: %s | Priority: %d | Layer: %d | Budget: %.2fms"), 
            *SystemInfo.SystemName, 
            (int32)SystemInfo.Priority,
            (int32)SystemInfo.Layer,
            SystemInfo.PerformanceBudget);
    }
}

TArray<FString> UEngineArchitecturalFramework::GetArchitecturalWarnings()
{
    return ArchitecturalWarnings;
}

TArray<FString> UEngineArchitecturalFramework::GetArchitecturalErrors()
{
    return ArchitecturalErrors;
}

bool UEngineArchitecturalFramework::ValidateSystemLayer(const FEng_SystemInfo& SystemInfo)
{
    // Validate that system layer is appropriate for its priority
    if (SystemInfo.Priority == EEng_SystemPriority::Critical && 
        SystemInfo.Layer != EEng_ArchitecturalLayer::Engine && 
        SystemInfo.Layer != EEng_ArchitecturalLayer::Core)
    {
        return false;
    }
    
    return true;
}

bool UEngineArchitecturalFramework::ValidateSystemPriority(const FEng_SystemInfo& SystemInfo)
{
    // Validate priority assignments are reasonable
    return true; // For now, all priorities are valid
}

bool UEngineArchitecturalFramework::ValidatePerformanceBudget(const FEng_SystemInfo& SystemInfo)
{
    // Validate performance budget is within reasonable limits
    if (SystemInfo.PerformanceBudget <= 0.0f || SystemInfo.PerformanceBudget > TotalPerformanceBudget)
    {
        return false;
    }
    
    return true;
}

void UEngineArchitecturalFramework::UpdateValidationStatus()
{
    // Periodic validation of all systems
    EnforceArchitecturalCompliance();
    
    // Log status if there are issues
    if (ArchitecturalErrors.Num() > 0 || ArchitecturalWarnings.Num() > 0)
    {
        LogArchitecturalStatus();
    }
}
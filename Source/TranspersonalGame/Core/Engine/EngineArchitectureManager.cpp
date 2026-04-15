#include "EngineArchitectureManager.h"
#include "PerformanceProfiler.h"
#include "SystemValidationManager.h"
#include "ModuleDependencyTracker.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    // Set default performance targets
    CurrentPerformanceTarget = EEng_PerformanceTarget::Balanced;
    bEnforceStrictValidation = true;
    bAutoValidateOnSystemChange = true;
    ValidationInterval = 30.0f; // Validate every 30 seconds
    
    // Set default performance constraints
    MaxFrameTime = 16.67f; // 60 FPS target
    MaxDrawCalls = 2000;
    MaxMemoryUsageMB = 8192; // 8GB limit
    
    // Initialize state
    bArchitectureInitialized = false;
    bValidationPassed = false;
    LastValidationTime = 0.0f;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture Manager: Initializing..."));
    
    InitializeSubsystems();
    
    // Perform initial validation
    if (ValidateSystemArchitecture())
    {
        bArchitectureInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("Engine Architecture Manager: Initialization complete"));
        
        // Start periodic validation if enabled
        if (bAutoValidateOnSystemChange && ValidationInterval > 0.0f)
        {
            if (UWorld* World = GetWorld())
            {
                World->GetTimerManager().SetTimer(
                    ValidationTimerHandle,
                    this,
                    &UEngineArchitectureManager::PerformScheduledValidation,
                    ValidationInterval,
                    true // Loop
                );
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architecture Manager: Initialization failed - validation errors"));
    }
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture Manager: Shutting down..."));
    
    // Clear validation timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    CleanupSubsystems();
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UEngineArchitectureManager::InitializeSubsystems()
{
    // Create performance profiler
    PerformanceProfiler = NewObject<UPerformanceProfiler>(this);
    if (PerformanceProfiler)
    {
        PerformanceProfiler->Initialize();
        UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized"));
    }
    
    // Create validation manager
    ValidationManager = NewObject<USystemValidationManager>(this);
    if (ValidationManager)
    {
        ValidationManager->Initialize();
        UE_LOG(LogTemp, Log, TEXT("System Validation Manager initialized"));
    }
    
    // Create dependency tracker
    DependencyTracker = NewObject<UModuleDependencyTracker>(this);
    if (DependencyTracker)
    {
        DependencyTracker->Initialize();
        UE_LOG(LogTemp, Log, TEXT("Module Dependency Tracker initialized"));
    }
}

void UEngineArchitectureManager::CleanupSubsystems()
{
    if (PerformanceProfiler)
    {
        PerformanceProfiler->Shutdown();
        PerformanceProfiler = nullptr;
    }
    
    if (ValidationManager)
    {
        ValidationManager->Shutdown();
        ValidationManager = nullptr;
    }
    
    if (DependencyTracker)
    {
        DependencyTracker->Shutdown();
        DependencyTracker = nullptr;
    }
}

bool UEngineArchitectureManager::ValidateSystemArchitecture()
{
    if (!ValidationManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot validate architecture - ValidationManager is null"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Validating system architecture..."));
    
    bool bValidationSuccess = true;
    
    // Validate system dependencies
    if (!ValidateSystemDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("System dependency validation failed"));
        bValidationSuccess = false;
    }
    
    // Validate performance targets
    if (!ValidatePerformanceTargets())
    {
        UE_LOG(LogTemp, Error, TEXT("Performance target validation failed"));
        bValidationSuccess = false;
    }
    
    // Validate memory constraints
    if (!ValidateMemoryConstraints())
    {
        UE_LOG(LogTemp, Error, TEXT("Memory constraint validation failed"));
        bValidationSuccess = false;
    }
    
    // Update validation state
    bValidationPassed = bValidationSuccess;
    LastValidationTime = FDateTime::Now().GetTimeOfDay().GetTotalSeconds();
    
    if (bValidationSuccess)
    {
        UE_LOG(LogTemp, Log, TEXT("Architecture validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Architecture validation failed"));
    }
    
    return bValidationSuccess;
}

void UEngineArchitectureManager::EnforcePerformanceConstraints()
{
    if (!PerformanceProfiler)
    {
        return;
    }
    
    FEng_PerformanceMetrics CurrentMetrics = GetCurrentPerformanceMetrics();
    
    // Check frame time
    if (CurrentMetrics.FrameTime > MaxFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time exceeded: %.2fms (limit: %.2fms)"), 
               CurrentMetrics.FrameTime, MaxFrameTime);
    }
    
    // Check draw calls
    if (CurrentMetrics.DrawCalls > MaxDrawCalls)
    {
        UE_LOG(LogTemp, Warning, TEXT("Draw calls exceeded: %d (limit: %d)"), 
               CurrentMetrics.DrawCalls, MaxDrawCalls);
    }
    
    // Check memory usage
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("Memory usage exceeded: %dMB (limit: %dMB)"), 
               CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    }
}

bool UEngineArchitectureManager::CheckModuleDependencies()
{
    if (!DependencyTracker)
    {
        return false;
    }
    
    return DependencyTracker->ValidateAllDependencies();
}

void UEngineArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== ARCHITECTURE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Architecture Initialized: %s"), bArchitectureInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Validation Passed: %s"), bValidationPassed ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Last Validation: %.2f seconds ago"), 
           FDateTime::Now().GetTimeOfDay().GetTotalSeconds() - LastValidationTime);
    
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    for (const auto& SystemPair : RegisteredSystems)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s: %s"), 
               *SystemPair.Key, 
               SystemPair.Value ? *SystemPair.Value->GetClass()->GetName() : TEXT("NULL"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Target: %s"), 
           *UEnum::GetValueAsString(CurrentPerformanceTarget));
    
    if (PerformanceProfiler)
    {
        FEng_PerformanceMetrics Metrics = GetCurrentPerformanceMetrics();
        UE_LOG(LogTemp, Log, TEXT("Current Performance:"));
        UE_LOG(LogTemp, Log, TEXT("  Frame Time: %.2fms"), Metrics.FrameTime);
        UE_LOG(LogTemp, Log, TEXT("  Draw Calls: %d"), Metrics.DrawCalls);
        UE_LOG(LogTemp, Log, TEXT("  Memory: %dMB"), Metrics.MemoryUsageMB);
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== END REPORT ==="));
}

bool UEngineArchitectureManager::RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot register null system: %s"), *SystemName);
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System already registered, replacing: %s"), *SystemName);
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    UE_LOG(LogTemp, Log, TEXT("Registered core system: %s"), *SystemName);
    
    // Trigger validation if auto-validation is enabled
    if (bAutoValidateOnSystemChange)
    {
        ValidateSystemArchitecture();
    }
    
    return true;
}

void UEngineArchitectureManager::UnregisterCoreSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered core system: %s"), *SystemName);
        
        // Trigger validation if auto-validation is enabled
        if (bAutoValidateOnSystemChange)
        {
            ValidateSystemArchitecture();
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempted to unregister unknown system: %s"), *SystemName);
    }
}

UObject* UEngineArchitectureManager::GetCoreSystem(const FString& SystemName)
{
    if (TObjectPtr<UObject>* SystemPtr = RegisteredSystems.Find(SystemName))
    {
        return *SystemPtr;
    }
    
    return nullptr;
}

FEng_PerformanceMetrics UEngineArchitectureManager::GetCurrentPerformanceMetrics()
{
    if (PerformanceProfiler)
    {
        return PerformanceProfiler->GetCurrentMetrics();
    }
    
    // Return default metrics if profiler is not available
    FEng_PerformanceMetrics DefaultMetrics;
    DefaultMetrics.FrameTime = 16.67f;
    DefaultMetrics.DrawCalls = 0;
    DefaultMetrics.MemoryUsageMB = 0;
    DefaultMetrics.TriangleCount = 0;
    DefaultMetrics.TextureMemoryMB = 0;
    DefaultMetrics.AudioMemoryMB = 0;
    
    return DefaultMetrics;
}

bool UEngineArchitectureManager::IsPerformanceWithinLimits()
{
    FEng_PerformanceMetrics CurrentMetrics = GetCurrentPerformanceMetrics();
    
    return (CurrentMetrics.FrameTime <= MaxFrameTime &&
            CurrentMetrics.DrawCalls <= MaxDrawCalls &&
            CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB);
}

void UEngineArchitectureManager::SetPerformanceTarget(EEng_PerformanceTarget Target)
{
    CurrentPerformanceTarget = Target;
    
    // Adjust performance constraints based on target
    switch (Target)
    {
        case EEng_PerformanceTarget::HighEnd:
            MaxFrameTime = 8.33f; // 120 FPS
            MaxDrawCalls = 5000;
            MaxMemoryUsageMB = 16384; // 16GB
            break;
            
        case EEng_PerformanceTarget::Balanced:
            MaxFrameTime = 16.67f; // 60 FPS
            MaxDrawCalls = 2000;
            MaxMemoryUsageMB = 8192; // 8GB
            break;
            
        case EEng_PerformanceTarget::LowEnd:
            MaxFrameTime = 33.33f; // 30 FPS
            MaxDrawCalls = 1000;
            MaxMemoryUsageMB = 4096; // 4GB
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance target set to: %s"), *UEnum::GetValueAsString(Target));
}

TArray<FEng_ValidationResult> UEngineArchitectureManager::ValidateAllSystems()
{
    TArray<FEng_ValidationResult> Results;
    
    if (!ValidationManager)
    {
        FEng_ValidationResult ErrorResult;
        ErrorResult.SystemName = TEXT("ValidationManager");
        ErrorResult.bPassed = false;
        ErrorResult.ErrorMessage = TEXT("ValidationManager is null");
        ErrorResult.Severity = EEng_ValidationSeverity::Critical;
        Results.Add(ErrorResult);
        return Results;
    }
    
    // Validate each registered system
    for (const auto& SystemPair : RegisteredSystems)
    {
        FEng_ValidationResult Result = ValidationManager->ValidateSystem(SystemPair.Key, SystemPair.Value);
        Results.Add(Result);
    }
    
    return Results;
}

bool UEngineArchitectureManager::ValidateSystemIntegration(const FString& SystemA, const FString& SystemB)
{
    if (!ValidationManager)
    {
        return false;
    }
    
    UObject* SystemInstanceA = GetCoreSystem(SystemA);
    UObject* SystemInstanceB = GetCoreSystem(SystemB);
    
    if (!SystemInstanceA || !SystemInstanceB)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot validate integration - one or both systems not found: %s, %s"), 
               *SystemA, *SystemB);
        return false;
    }
    
    return ValidationManager->ValidateSystemIntegration(SystemInstanceA, SystemInstanceB);
}

TArray<FString> UEngineArchitectureManager::GetLoadedModules()
{
    if (DependencyTracker)
    {
        return DependencyTracker->GetLoadedModules();
    }
    
    return TArray<FString>();
}

bool UEngineArchitectureManager::IsModuleLoaded(const FString& ModuleName)
{
    if (DependencyTracker)
    {
        return DependencyTracker->IsModuleLoaded(ModuleName);
    }
    
    return false;
}

void UEngineArchitectureManager::RefreshModuleDependencies()
{
    if (DependencyTracker)
    {
        DependencyTracker->RefreshDependencies();
    }
}

bool UEngineArchitectureManager::ValidateSystemDependencies()
{
    // Check that all required subsystems are available
    if (!PerformanceProfiler || !ValidationManager || !DependencyTracker)
    {
        UE_LOG(LogTemp, Error, TEXT("Missing required subsystems"));
        return false;
    }
    
    // Validate module dependencies
    if (!CheckModuleDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Module dependency validation failed"));
        return false;
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidatePerformanceTargets()
{
    // Check if current performance is within acceptable limits
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance is outside acceptable limits"));
        // Don't fail validation for performance issues, just warn
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateMemoryConstraints()
{
    FEng_PerformanceMetrics CurrentMetrics = GetCurrentPerformanceMetrics();
    
    // Check memory usage
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        UE_LOG(LogTemp, Error, TEXT("Memory usage exceeds limit: %dMB > %dMB"), 
               CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
        return false;
    }
    
    return true;
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    if (PerformanceProfiler)
    {
        PerformanceProfiler->UpdateMetrics();
    }
}

void UEngineArchitectureManager::PerformScheduledValidation()
{
    UE_LOG(LogTemp, Log, TEXT("Performing scheduled architecture validation..."));
    ValidateSystemArchitecture();
}
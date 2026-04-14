#include "EngineArchitectureManager.h"
#include "SystemValidationManager.h"
#include "PerformanceProfiler.h"
#include "ModuleManager.h"
#include "DependencyResolver.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    bArchitectureInitialized = false;
    PerformanceTarget = EEng_PerformanceTarget::Balanced;
    LastPerformanceCheck = 0.0f;

    // Initialize default performance constraints
    CurrentConstraints.MaxFrameTime = 16.67f; // 60 FPS target
    CurrentConstraints.MaxMemoryUsage = 8192; // 8GB
    CurrentConstraints.MaxDrawCalls = 5000;
    CurrentConstraints.MaxTriangles = 2000000; // 2M triangles
    CurrentConstraints.MaxConcurrentSystems = 32;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager: Initializing..."));

    // Get references to core subsystems
    ValidationManager = GetGameInstance()->GetSubsystem<USystemValidationManager>();
    PerformanceProfiler = GetGameInstance()->GetSubsystem<UPerformanceProfiler>();
    ModuleManager = GetGameInstance()->GetSubsystem<UModuleManager>();
    DependencyResolver = GetGameInstance()->GetSubsystem<UDependencyResolver>();

    // Initialize core architecture rules
    InitializeAllSystems();
    
    bArchitectureInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager: Initialized successfully"));
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architecture Manager: Shutting down..."));
    
    ShutdownAllSystems();
    bArchitectureInitialized = false;
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority)
{
    if (!ValidateSystemRegistration(SystemName, SystemType))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to validate system registration for: %s"), *SystemName);
        return false;
    }

    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.SystemType = SystemType;
    SystemInfo.Priority = Priority;
    SystemInfo.bIsActive = false;
    SystemInfo.LastUpdateTime = 0.0f;
    SystemInfo.PerformanceMetrics = FEng_PerformanceMetrics();

    RegisteredSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s (Type: %d, Priority: %d)"), 
           *SystemName, (int32)SystemType, Priority);
    
    UpdateSystemPriorities();
    return true;
}

bool UEngineArchitectureManager::ValidateSystemDependencies(const FString& SystemName, const TArray<FString>& Dependencies)
{
    if (!DependencyResolver)
    {
        UE_LOG(LogTemp, Error, TEXT("Dependency resolver not available"));
        return false;
    }

    // Check if all dependencies are registered
    for (const FString& Dependency : Dependencies)
    {
        if (!RegisteredSystems.Contains(Dependency))
        {
            UE_LOG(LogTemp, Error, TEXT("System %s depends on unregistered system: %s"), 
                   *SystemName, *Dependency);
            ArchitectureViolations.AddUnique(FString::Printf(
                TEXT("Missing dependency: %s -> %s"), *SystemName, *Dependency));
            return false;
        }
    }

    return true;
}

void UEngineArchitectureManager::EnforcePerformanceConstraints(const FEng_PerformanceConstraints& Constraints)
{
    CurrentConstraints = Constraints;
    
    UE_LOG(LogTemp, Log, TEXT("Updated performance constraints - Frame: %.2fms, Memory: %dMB"), 
           Constraints.MaxFrameTime, Constraints.MaxMemoryUsage);
    
    CheckPerformanceCompliance();
}

void UEngineArchitectureManager::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing all registered systems..."));

    // Sort systems by priority
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    
    SystemNames.Sort([this](const FString& A, const FString& B) {
        const FEng_SystemInfo* InfoA = RegisteredSystems.Find(A);
        const FEng_SystemInfo* InfoB = RegisteredSystems.Find(B);
        return InfoA && InfoB && InfoA->Priority > InfoB->Priority;
    });

    // Initialize in priority order
    for (const FString& SystemName : SystemNames)
    {
        InitializeSystemInOrder(SystemName);
    }

    UE_LOG(LogTemp, Warning, TEXT("All systems initialized"));
}

void UEngineArchitectureManager::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Shutting down all systems..."));

    // Shutdown in reverse priority order
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    
    SystemNames.Sort([this](const FString& A, const FString& B) {
        const FEng_SystemInfo* InfoA = RegisteredSystems.Find(A);
        const FEng_SystemInfo* InfoB = RegisteredSystems.Find(B);
        return InfoA && InfoB && InfoA->Priority < InfoB->Priority;
    });

    for (const FString& SystemName : SystemNames)
    {
        ShutdownSystemInOrder(SystemName);
    }
}

bool UEngineArchitectureManager::IsSystemActive(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo && SystemInfo->bIsActive;
}

bool UEngineArchitectureManager::ValidateArchitecturalIntegrity()
{
    ArchitectureViolations.Empty();

    if (!ValidationManager)
    {
        ArchitectureViolations.Add(TEXT("System Validation Manager not available"));
        return false;
    }

    // Check all registered systems
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        const FEng_SystemInfo& SystemInfo = SystemPair.Value;

        // Validate system is properly initialized
        if (!SystemInfo.bIsActive)
        {
            ArchitectureViolations.Add(FString::Printf(
                TEXT("System not active: %s"), *SystemName));
        }
    }

    // Check performance compliance
    CheckPerformanceCompliance();

    bool bIsValid = ArchitectureViolations.Num() == 0;
    UE_LOG(LogTemp, Log, TEXT("Architecture validation: %s (%d violations)"), 
           bIsValid ? TEXT("PASS") : TEXT("FAIL"), ArchitectureViolations.Num());

    return bIsValid;
}

TArray<FString> UEngineArchitectureManager::GetArchitectureViolations() const
{
    return ArchitectureViolations;
}

FEng_PerformanceMetrics UEngineArchitectureManager::GetCurrentPerformanceMetrics() const
{
    if (PerformanceProfiler)
    {
        return PerformanceProfiler->GetCurrentMetrics();
    }

    return FEng_PerformanceMetrics();
}

void UEngineArchitectureManager::SetPerformanceTarget(EEng_PerformanceTarget Target)
{
    PerformanceTarget = Target;

    // Update constraints based on target
    switch (Target)
    {
    case EEng_PerformanceTarget::HighPerformance:
        CurrentConstraints.MaxFrameTime = 8.33f; // 120 FPS
        CurrentConstraints.MaxMemoryUsage = 16384; // 16GB
        break;
    case EEng_PerformanceTarget::Balanced:
        CurrentConstraints.MaxFrameTime = 16.67f; // 60 FPS
        CurrentConstraints.MaxMemoryUsage = 8192; // 8GB
        break;
    case EEng_PerformanceTarget::PowerSaving:
        CurrentConstraints.MaxFrameTime = 33.33f; // 30 FPS
        CurrentConstraints.MaxMemoryUsage = 4096; // 4GB
        break;
    }

    UE_LOG(LogTemp, Log, TEXT("Performance target set to: %d"), (int32)Target);
}

bool UEngineArchitectureManager::CheckModuleDependencies(const FString& ModuleName) const
{
    if (!ModuleManager)
    {
        return false;
    }

    return ModuleManager->ValidateModuleDependencies(ModuleName);
}

void UEngineArchitectureManager::ResolveCircularDependencies()
{
    if (DependencyResolver)
    {
        DependencyResolver->ResolveCircularDependencies();
    }
}

bool UEngineArchitectureManager::ValidateSystemRegistration(const FString& SystemName, EEng_SystemType SystemType)
{
    // Check if system already registered
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("System already registered: %s"), *SystemName);
        return false;
    }

    // Validate system name
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("System name cannot be empty"));
        return false;
    }

    return true;
}

void UEngineArchitectureManager::UpdateSystemPriorities()
{
    // Systems are automatically sorted by priority during initialization
    LogArchitectureState();
}

void UEngineArchitectureManager::CheckPerformanceCompliance()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPerformanceCheck < PerformanceCheckInterval)
    {
        return;
    }

    LastPerformanceCheck = CurrentTime;

    if (PerformanceProfiler)
    {
        FEng_PerformanceMetrics Metrics = PerformanceProfiler->GetCurrentMetrics();
        
        // Check frame time
        if (Metrics.FrameTime > CurrentConstraints.MaxFrameTime)
        {
            ArchitectureViolations.AddUnique(FString::Printf(
                TEXT("Frame time violation: %.2fms > %.2fms"), 
                Metrics.FrameTime, CurrentConstraints.MaxFrameTime));
        }

        // Check memory usage
        if (Metrics.MemoryUsage > CurrentConstraints.MaxMemoryUsage)
        {
            ArchitectureViolations.AddUnique(FString::Printf(
                TEXT("Memory usage violation: %dMB > %dMB"), 
                Metrics.MemoryUsage, CurrentConstraints.MaxMemoryUsage));
        }
    }
}

void UEngineArchitectureManager::LogArchitectureState()
{
    UE_LOG(LogTemp, Log, TEXT("=== Architecture State ==="));
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("Violations: %d"), ArchitectureViolations.Num());
    UE_LOG(LogTemp, Log, TEXT("Performance Target: %d"), (int32)PerformanceTarget);
    UE_LOG(LogTemp, Log, TEXT("========================"));
}

void UEngineArchitectureManager::InitializeSystemInOrder(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (SystemInfo && !SystemInfo->bIsActive)
    {
        SystemInfo->bIsActive = true;
        SystemInfo->LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Initialized system: %s"), *SystemName);
    }
}

void UEngineArchitectureManager::ShutdownSystemInOrder(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (SystemInfo && SystemInfo->bIsActive)
    {
        SystemInfo->bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *SystemName);
    }
}
#include "Eng_SystemsRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

UEng_SystemsRegistry::UEng_SystemsRegistry()
{
    PerformanceUpdateTimer = 0.0f;
    FrameTimeHistory.Reserve(60); // Store last 60 frames for averaging
}

void UEng_SystemsRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Systems Registry initialized"));
    
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
    CurrentMetrics.CurrentProfile = EPerformanceProfile::Development;
    
    // Clear any existing data
    RegisteredSystems.Empty();
    SystemMessages.Empty();
    ArchitecturalViolations.Empty();
    FrameTimeHistory.Empty();
    
    // Register core architectural systems
    RegisterSystem(TEXT("ArchitecturalCore"), 0, TArray<FString>(), true);
    RegisterSystem(TEXT("TechnicalArchitecture"), 1, {TEXT("ArchitecturalCore")}, true);
    RegisterSystem(TEXT("SystemsRegistry"), 2, {TEXT("ArchitecturalCore"), TEXT("TechnicalArchitecture")}, true);
    
    SetSystemState(TEXT("SystemsRegistry"), ESystemState::Active);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Core architectural systems registered"));
}

void UEng_SystemsRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Systems Registry shutting down"));
    
    // Gracefully shutdown all registered systems
    for (auto& SystemPair : RegisteredSystems)
    {
        SetSystemState(SystemPair.Key, ESystemState::ShuttingDown);
    }
    
    RegisteredSystems.Empty();
    SystemMessages.Empty();
    ArchitecturalViolations.Empty();
    
    Super::Deinitialize();
}

bool UEng_SystemsRegistry::RegisterSystem(const FString& SystemName, int32 Priority, const TArray<FString>& Dependencies, bool bIsCritical)
{
    if (SystemName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot register system with empty name"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System '%s' already registered, updating registration"), *SystemName);
    }
    
    FEng_SystemRegistration NewRegistration;
    NewRegistration.SystemName = SystemName;
    NewRegistration.InitializationPriority = Priority;
    NewRegistration.Dependencies = Dependencies;
    NewRegistration.CurrentState = ESystemState::Registered;
    NewRegistration.bIsCriticalSystem = bIsCritical;
    
    // Set performance budget based on system priority and criticality
    if (bIsCritical)
    {
        NewRegistration.PerformanceBudgetMs = 8.33f; // Higher budget for critical systems
    }
    else
    {
        NewRegistration.PerformanceBudgetMs = 4.17f; // Standard budget
    }
    
    RegisteredSystems.Add(SystemName, NewRegistration);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered system '%s' with priority %d"), *SystemName, Priority);
    
    // Validate dependencies
    if (!ValidateDependencies())
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Dependency validation failed after registering '%s'"), *SystemName);
        ArchitecturalViolations.AddUnique(FString::Printf(TEXT("Invalid dependencies for system: %s"), *SystemName));
    }
    
    return true;
}

bool UEng_SystemsRegistry::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Cannot unregister unknown system '%s'"), *SystemName);
        return false;
    }
    
    // Check if other systems depend on this one
    TArray<FString> Dependents = GetSystemDependents(SystemName);
    if (Dependents.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot unregister '%s' - other systems depend on it"), *SystemName);
        return false;
    }
    
    SetSystemState(SystemName, ESystemState::ShuttingDown);
    RegisteredSystems.Remove(SystemName);
    SystemMessages.Remove(SystemName);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Unregistered system '%s'"), *SystemName);
    return true;
}

bool UEng_SystemsRegistry::SetSystemState(const FString& SystemName, ESystemState NewState)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Cannot set state for unknown system '%s'"), *SystemName);
        return false;
    }
    
    FEng_SystemRegistration& System = RegisteredSystems[SystemName];
    ESystemState OldState = System.CurrentState;
    System.CurrentState = NewState;
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: System '%s' state changed from %d to %d"), 
           *SystemName, (int32)OldState, (int32)NewState);
    
    // Validate state transition
    if (NewState == ESystemState::Active && OldState != ESystemState::Initializing)
    {
        // Check if dependencies are satisfied
        for (const FString& Dependency : System.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency) || 
                RegisteredSystems[Dependency].CurrentState != ESystemState::Active)
            {
                UE_LOG(LogTemp, Error, TEXT("Engine Architect: Cannot activate '%s' - dependency '%s' not active"), 
                       *SystemName, *Dependency);
                System.CurrentState = ESystemState::Error;
                return false;
            }
        }
    }
    
    return true;
}

ESystemState UEng_SystemsRegistry::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))
    {
        return System->CurrentState;
    }
    return ESystemState::Uninitialized;
}

TArray<FString> UEng_SystemsRegistry::GetInitializationOrder() const
{
    TArray<FString> InitOrder;
    
    if (!ResolveDependencyOrder(InitOrder))
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architect: Failed to resolve initialization order - circular dependencies detected"));
        return TArray<FString>();
    }
    
    return InitOrder;
}

bool UEng_SystemsRegistry::ValidateDependencies() const
{
    // Check for circular dependencies
    TSet<FString> VisitedSystems;
    for (const auto& SystemPair : RegisteredSystems)
    {
        VisitedSystems.Empty();
        if (HasCircularDependency(SystemPair.Key, VisitedSystems))
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architect: Circular dependency detected involving system '%s'"), *SystemPair.Key);
            return false;
        }
    }
    
    // Check if all dependencies are registered
    for (const auto& SystemPair : RegisteredSystems)
    {
        for (const FString& Dependency : SystemPair.Value.Dependencies)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                UE_LOG(LogTemp, Error, TEXT("Engine Architect: System '%s' depends on unregistered system '%s'"), 
                       *SystemPair.Key, *Dependency);
                return false;
            }
        }
    }
    
    return true;
}

TArray<FString> UEng_SystemsRegistry::GetSystemDependents(const FString& SystemName) const
{
    TArray<FString> Dependents;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Dependencies.Contains(SystemName))
        {
            Dependents.Add(SystemPair.Key);
        }
    }
    
    return Dependents;
}

void UEng_SystemsRegistry::UpdatePerformanceMetrics(float DeltaTime)
{
    PerformanceUpdateTimer += DeltaTime;
    
    // Update frame time history
    float FrameTimeMs = DeltaTime * 1000.0f;
    UpdateFrameTimeHistory(FrameTimeMs);
    
    // Update metrics every second
    if (PerformanceUpdateTimer >= 1.0f)
    {
        CurrentMetrics.AverageFrameTimeMs = CalculateAverageFrameTime();
        CurrentMetrics.PeakFrameTimeMs = FrameTimeHistory.Num() > 0 ? *FMath::MaxElement(FrameTimeHistory) : 0.0f;
        CurrentMetrics.ActiveSystemsCount = 0;
        
        // Count active systems
        for (const auto& SystemPair : RegisteredSystems)
        {
            if (SystemPair.Value.CurrentState == ESystemState::Active)
            {
                CurrentMetrics.ActiveSystemsCount++;
            }
        }
        
        PerformanceUpdateTimer = 0.0f;
    }
}

FEng_PerformanceMetrics UEng_SystemsRegistry::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

bool UEng_SystemsRegistry::IsPerformanceBudgetExceeded() const
{
    float TargetFrameTime = 16.67f; // 60 FPS
    
    switch (CurrentMetrics.CurrentProfile)
    {
        case EPerformanceProfile::Console:
            TargetFrameTime = 33.33f; // 30 FPS
            break;
        case EPerformanceProfile::PC_High:
            TargetFrameTime = 16.67f; // 60 FPS
            break;
        case EPerformanceProfile::PC_Ultra:
            TargetFrameTime = 11.11f; // 90 FPS
            break;
        default:
            TargetFrameTime = 16.67f;
            break;
    }
    
    return CurrentMetrics.AverageFrameTimeMs > TargetFrameTime;
}

void UEng_SystemsRegistry::SetPerformanceProfile(EPerformanceProfile NewProfile)
{
    CurrentMetrics.CurrentProfile = NewProfile;
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Performance profile changed to %d"), (int32)NewProfile);
}

bool UEng_SystemsRegistry::ValidateSystemArchitecture(const FString& SystemName) const
{
    const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName);
    if (!System)
    {
        return false;
    }
    
    return ValidateSystemCompliance(*System);
}

TArray<FString> UEng_SystemsRegistry::GetArchitecturalViolations() const
{
    return ArchitecturalViolations;
}

bool UEng_SystemsRegistry::EnforceArchitecturalCompliance()
{
    ArchitecturalViolations.Empty();
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!ValidateSystemCompliance(SystemPair.Value))
        {
            ArchitecturalViolations.AddUnique(FString::Printf(TEXT("Compliance violation in system: %s"), *SystemPair.Key));
        }
    }
    
    return ArchitecturalViolations.Num() == 0;
}

bool UEng_SystemsRegistry::SendSystemMessage(const FString& FromSystem, const FString& ToSystem, const FString& Message)
{
    if (!RegisteredSystems.Contains(FromSystem) || !RegisteredSystems.Contains(ToSystem))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Cannot send message between unregistered systems"));
        return false;
    }
    
    if (!SystemMessages.Contains(ToSystem))
    {
        SystemMessages.Add(ToSystem, TArray<FString>());
    }
    
    FString FormattedMessage = FString::Printf(TEXT("[%s]: %s"), *FromSystem, *Message);
    SystemMessages[ToSystem].Add(FormattedMessage);
    
    return true;
}

TArray<FString> UEng_SystemsRegistry::GetPendingMessages(const FString& SystemName)
{
    if (TArray<FString>* Messages = SystemMessages.Find(SystemName))
    {
        TArray<FString> PendingMessages = *Messages;
        Messages->Empty(); // Clear after retrieval
        return PendingMessages;
    }
    
    return TArray<FString>();
}

void UEng_SystemsRegistry::DumpSystemsStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECT SYSTEMS REGISTRY STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Performance Profile: %d"), (int32)CurrentMetrics.CurrentProfile);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTimeMs);
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemRegistration& System = SystemPair.Value;
        UE_LOG(LogTemp, Warning, TEXT("System: %s | State: %d | Priority: %d | Critical: %s"), 
               *System.SystemName, (int32)System.CurrentState, System.InitializationPriority,
               System.bIsCriticalSystem ? TEXT("Yes") : TEXT("No"));
    }
    
    if (ArchitecturalViolations.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Architectural Violations: %d"), ArchitecturalViolations.Num());
        for (const FString& Violation : ArchitecturalViolations)
        {
            UE_LOG(LogTemp, Error, TEXT("- %s"), *Violation);
        }
    }
}

bool UEng_SystemsRegistry::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

int32 UEng_SystemsRegistry::GetRegisteredSystemsCount() const
{
    return RegisteredSystems.Num();
}

// Private helper methods

bool UEng_SystemsRegistry::ResolveDependencyOrder(TArray<FString>& OutOrder) const
{
    OutOrder.Empty();
    TSet<FString> ProcessedSystems;
    TSet<FString> ProcessingSystems;
    
    // Topological sort using DFS
    TFunction<bool(const FString&)> VisitSystem = [&](const FString& SystemName) -> bool
    {
        if (ProcessedSystems.Contains(SystemName))
        {
            return true;
        }
        
        if (ProcessingSystems.Contains(SystemName))
        {
            // Circular dependency detected
            return false;
        }
        
        ProcessingSystems.Add(SystemName);
        
        if (const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName))
        {
            for (const FString& Dependency : System->Dependencies)
            {
                if (!VisitSystem(Dependency))
                {
                    return false;
                }
            }
        }
        
        ProcessingSystems.Remove(SystemName);
        ProcessedSystems.Add(SystemName);
        OutOrder.Add(SystemName);
        
        return true;
    };
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (!VisitSystem(SystemPair.Key))
        {
            return false;
        }
    }
    
    return true;
}

bool UEng_SystemsRegistry::HasCircularDependency(const FString& SystemName, TSet<FString>& VisitedSystems) const
{
    if (VisitedSystems.Contains(SystemName))
    {
        return true; // Circular dependency found
    }
    
    const FEng_SystemRegistration* System = RegisteredSystems.Find(SystemName);
    if (!System)
    {
        return false;
    }
    
    VisitedSystems.Add(SystemName);
    
    for (const FString& Dependency : System->Dependencies)
    {
        if (HasCircularDependency(Dependency, VisitedSystems))
        {
            return true;
        }
    }
    
    VisitedSystems.Remove(SystemName);
    return false;
}

void UEng_SystemsRegistry::UpdateFrameTimeHistory(float FrameTime)
{
    FrameTimeHistory.Add(FrameTime);
    
    // Keep only last 60 frames
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UEng_SystemsRegistry::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

bool UEng_SystemsRegistry::ValidateSystemCompliance(const FEng_SystemRegistration& System) const
{
    // Check if system follows naming conventions
    if (!System.SystemName.StartsWith(TEXT("Eng_")) && 
        !System.SystemName.StartsWith(TEXT("Core_")) &&
        !System.SystemName.Contains(TEXT("Manager")) &&
        !System.SystemName.Contains(TEXT("System")))
    {
        return false;
    }
    
    // Check performance budget compliance
    if (System.PerformanceBudgetMs <= 0.0f || System.PerformanceBudgetMs > 33.33f)
    {
        return false;
    }
    
    // Critical systems must have higher priority
    if (System.bIsCriticalSystem && System.InitializationPriority > 50)
    {
        return false;
    }
    
    return true;
}
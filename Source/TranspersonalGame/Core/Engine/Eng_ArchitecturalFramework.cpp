#include "Eng_ArchitecturalFramework.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_ArchitecturalFramework::UEng_ArchitecturalFramework()
{
    CurrentMetrics = FEng_PerformanceMetrics();
}

void UEng_ArchitecturalFramework::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initializing"));
    
    InitializeCoreSystems();
    
    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UEng_ArchitecturalFramework::UpdatePerformanceMetrics,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Initialized with %d systems"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFramework::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architectural Framework Shutting Down"));
    
    // Set all systems to shutdown state
    for (auto& SystemPair : RegisteredSystems)
    {
        SystemPair.Value.State = EEng_SystemState::Shutdown;
    }
    
    RegisteredSystems.Empty();
    SystemDependencies.Empty();
    ArchitecturalErrors.Empty();
    
    Super::Deinitialize();
}

void UEng_ArchitecturalFramework::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (SystemName.IsEmpty())
    {
        ArchitecturalErrors.Add(TEXT("Cannot register system with empty name"));
        return;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered, updating priority"), *SystemName);
        RegisteredSystems[SystemName].Priority = Priority;
        return;
    }
    
    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.State = EEng_SystemState::Uninitialized;
    NewSystem.InitializationTime = FPlatformTime::Seconds();
    NewSystem.DependencyCount = 0;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s with priority %d"), *SystemName, (int32)Priority);
}

void UEng_ArchitecturalFramework::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems[SystemName].State = EEng_SystemState::Shutdown;
        RegisteredSystems.Remove(SystemName);
        SystemDependencies.Remove(SystemName);
        
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

bool UEng_ArchitecturalFramework::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

void UEng_ArchitecturalFramework::SetSystemState(const FString& SystemName, EEng_SystemState NewState)
{
    if (FEng_SystemInfo* System = RegisteredSystems.Find(SystemName))
    {
        EEng_SystemState OldState = System->State;
        System->State = NewState;
        
        if (NewState == EEng_SystemState::Active && OldState != EEng_SystemState::Active)
        {
            System->InitializationTime = FPlatformTime::Seconds() - System->InitializationTime;
        }
        
        UE_LOG(LogTemp, Log, TEXT("System %s state changed from %d to %d"), *SystemName, (int32)OldState, (int32)NewState);
    }
}

EEng_SystemState UEng_ArchitecturalFramework::GetSystemState(const FString& SystemName) const
{
    if (const FEng_SystemInfo* System = RegisteredSystems.Find(SystemName))
    {
        return System->State;
    }
    return EEng_SystemState::Uninitialized;
}

FEng_PerformanceMetrics UEng_ArchitecturalFramework::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEng_ArchitecturalFramework::UpdatePerformanceMetrics()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        // Update frame time
        CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
        
        // Update thread times (simplified)
        CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.6f; // Estimate
        CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.4f; // Estimate
        
        // Update draw calls and triangles (would need render stats in real implementation)
        CurrentMetrics.DrawCalls = 1000; // Placeholder
        CurrentMetrics.TriangleCount = 50000; // Placeholder
        
        // Update memory usage
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        CurrentMetrics.MemoryUsage = MemStats.UsedPhysical / (1024.0f * 1024.0f); // MB
    }
}

void UEng_ArchitecturalFramework::AddSystemDependency(const FString& SystemName, const FString& DependsOn)
{
    if (!RegisteredSystems.Contains(SystemName) || !RegisteredSystems.Contains(DependsOn))
    {
        ArchitecturalErrors.Add(FString::Printf(TEXT("Cannot add dependency: %s -> %s (one or both systems not registered)"), *SystemName, *DependsOn));
        return;
    }
    
    TArray<FString>& Dependencies = SystemDependencies.FindOrAdd(SystemName);
    Dependencies.AddUnique(DependsOn);
    
    // Update dependency count
    if (FEng_SystemInfo* System = RegisteredSystems.Find(SystemName))
    {
        System->DependencyCount = Dependencies.Num();
    }
    
    // Validate for circular dependencies
    ValidateDependencies();
    
    UE_LOG(LogTemp, Log, TEXT("Added dependency: %s depends on %s"), *SystemName, *DependsOn);
}

TArray<FString> UEng_ArchitecturalFramework::GetSystemDependencies(const FString& SystemName) const
{
    if (const TArray<FString>* Dependencies = SystemDependencies.Find(SystemName))
    {
        return *Dependencies;
    }
    return TArray<FString>();
}

bool UEng_ArchitecturalFramework::ValidateSystemArchitecture() const
{
    ArchitecturalErrors.Empty();
    
    // Check for circular dependencies
    for (const auto& SystemPair : RegisteredSystems)
    {
        TSet<FString> Visited;
        TSet<FString> RecursionStack;
        
        if (CheckCircularDependencies(SystemPair.Key, Visited, RecursionStack))
        {
            const_cast<UEng_ArchitecturalFramework*>(this)->ArchitecturalErrors.Add(
                FString::Printf(TEXT("Circular dependency detected involving system: %s"), *SystemPair.Key)
            );
        }
    }
    
    // Check for missing dependencies
    for (const auto& DependencyPair : SystemDependencies)
    {
        for (const FString& Dependency : DependencyPair.Value)
        {
            if (!RegisteredSystems.Contains(Dependency))
            {
                const_cast<UEng_ArchitecturalFramework*>(this)->ArchitecturalErrors.Add(
                    FString::Printf(TEXT("System %s depends on unregistered system: %s"), *DependencyPair.Key, *Dependency)
                );
            }
        }
    }
    
    return ArchitecturalErrors.Num() == 0;
}

TArray<FString> UEng_ArchitecturalFramework::GetArchitecturalErrors() const
{
    return ArchitecturalErrors;
}

void UEng_ArchitecturalFramework::InitializeCoreSystems()
{
    // Register core engine systems
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Physics"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Rendering"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("Audio"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("AI"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("Animation"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("UI"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("Networking"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("Telemetry"), EEng_SystemPriority::Low);
    
    // Set up basic dependencies
    AddSystemDependency(TEXT("WorldGeneration"), TEXT("Physics"));
    AddSystemDependency(TEXT("AI"), TEXT("Physics"));
    AddSystemDependency(TEXT("Animation"), TEXT("Physics"));
    AddSystemDependency(TEXT("Audio"), TEXT("WorldGeneration"));
    AddSystemDependency(TEXT("UI"), TEXT("Rendering"));
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d core systems"), RegisteredSystems.Num());
}

void UEng_ArchitecturalFramework::ValidateDependencies()
{
    ValidateSystemArchitecture();
}

bool UEng_ArchitecturalFramework::CheckCircularDependencies(const FString& SystemName, TSet<FString>& Visited, TSet<FString>& RecursionStack) const
{
    if (RecursionStack.Contains(SystemName))
    {
        return true; // Circular dependency found
    }
    
    if (Visited.Contains(SystemName))
    {
        return false; // Already processed this path
    }
    
    Visited.Add(SystemName);
    RecursionStack.Add(SystemName);
    
    if (const TArray<FString>* Dependencies = SystemDependencies.Find(SystemName))
    {
        for (const FString& Dependency : *Dependencies)
        {
            if (CheckCircularDependencies(Dependency, Visited, RecursionStack))
            {
                return true;
            }
        }
    }
    
    RecursionStack.Remove(SystemName);
    return false;
}
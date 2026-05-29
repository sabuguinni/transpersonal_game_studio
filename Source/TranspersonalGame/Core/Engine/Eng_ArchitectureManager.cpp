#include "Eng_ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_ArchitectureManager::UEng_ArchitectureManager()
{
    bAutoInitializeSystems = true;
    SystemInitializationTimeout = 30.0f;
    bEnablePerformanceMonitoring = true;
    bSystemsInitialized = false;
    LastPerformanceUpdate = 0.0f;
}

void UEng_ArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing core architecture systems"));
    
    // Initialize system ready states
    SystemReadyStates.Empty();
    SystemReadyStates.Add(TEXT("WorldGeneration"), false);
    SystemReadyStates.Add(TEXT("CharacterSystems"), false);
    SystemReadyStates.Add(TEXT("CombatSystems"), false);
    SystemReadyStates.Add(TEXT("AISystems"), false);
    SystemReadyStates.Add(TEXT("AudioSystems"), false);
    SystemReadyStates.Add(TEXT("VFXSystems"), false);
    
    // Initialize performance metrics
    PerformanceMetrics.Empty();
    for (const auto& SystemPair : SystemReadyStates)
    {
        PerformanceMetrics.Add(SystemPair.Key, 60.0f); // Default 60 FPS target
    }
    
    // Auto-initialize if enabled
    if (bAutoInitializeSystems)
    {
        InitializeGameSystems();
    }
}

void UEng_ArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Deinitializing architecture systems"));
    
    // Clean up all systems
    SystemReadyStates.Empty();
    PerformanceMetrics.Empty();
    LoadedModuleNames.Empty();
    ModulePriorities.Empty();
    
    bSystemsInitialized = false;
    
    Super::Deinitialize();
}

bool UEng_ArchitectureManager::InitializeGameSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Starting game systems initialization"));
    
    if (bSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Systems already initialized"));
        return true;
    }
    
    // Initialize systems in priority order
    InitializeWorldGeneration();
    InitializeCharacterSystems();
    InitializeCombatSystems();
    InitializeAISystems();
    InitializeAudioSystems();
    InitializeVFXSystems();
    
    bSystemsInitialized = true;
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Game systems initialization complete"));
    return ValidateSystemIntegrity();
}

bool UEng_ArchitectureManager::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Validating system integrity"));
    
    bool bAllSystemsReady = true;
    
    for (const auto& SystemPair : SystemReadyStates)
    {
        if (!SystemPair.Value)
        {
            UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: System %s is not ready"), *SystemPair.Key);
            bAllSystemsReady = false;
        }
    }
    
    if (bAllSystemsReady)
    {
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: All systems validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ArchitectureManager: System integrity validation failed"));
    }
    
    return bAllSystemsReady;
}

void UEng_ArchitectureManager::RegisterGameModule(const FString& ModuleName, int32 Priority)
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Registering module %s with priority %d"), *ModuleName, Priority);
    
    if (!LoadedModuleNames.Contains(ModuleName))
    {
        LoadedModuleNames.Add(ModuleName);
        ModulePriorities.Add(ModuleName, Priority);
        OnModuleLoaded(ModuleName);
    }
}

bool UEng_ArchitectureManager::IsSystemReady(const FString& SystemName) const
{
    const bool* ReadyState = SystemReadyStates.Find(SystemName);
    return ReadyState ? *ReadyState : false;
}

float UEng_ArchitectureManager::GetSystemPerformanceMetric(const FString& SystemName) const
{
    const float* Metric = PerformanceMetrics.Find(SystemName);
    return Metric ? *Metric : 0.0f;
}

void UEng_ArchitectureManager::SetPerformanceTarget(const FString& SystemName, float TargetFPS)
{
    PerformanceMetrics.Add(SystemName, TargetFPS);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Set performance target for %s to %.2f FPS"), *SystemName, TargetFPS);
}

TArray<FString> UEng_ArchitectureManager::GetLoadedModules() const
{
    return LoadedModuleNames;
}

bool UEng_ArchitectureManager::UnloadModule(const FString& ModuleName)
{
    if (LoadedModuleNames.Contains(ModuleName))
    {
        LoadedModuleNames.Remove(ModuleName);
        ModulePriorities.Remove(ModuleName);
        OnModuleUnloaded(ModuleName);
        UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Unloaded module %s"), *ModuleName);
        return true;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Module %s not found for unloading"), *ModuleName);
    return false;
}

bool UEng_ArchitectureManager::ReloadModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Reloading module %s"), *ModuleName);
    
    if (UnloadModule(ModuleName))
    {
        return LoadGameModule(ModuleName);
    }
    
    return false;
}

void UEng_ArchitectureManager::InitializeWorldGeneration()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing World Generation system"));
    SystemReadyStates[TEXT("WorldGeneration")] = true;
}

void UEng_ArchitectureManager::InitializeCharacterSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing Character Systems"));
    SystemReadyStates[TEXT("CharacterSystems")] = true;
}

void UEng_ArchitectureManager::InitializeCombatSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing Combat Systems"));
    SystemReadyStates[TEXT("CombatSystems")] = true;
}

void UEng_ArchitectureManager::InitializeAISystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing AI Systems"));
    SystemReadyStates[TEXT("AISystems")] = true;
}

void UEng_ArchitectureManager::InitializeAudioSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing Audio Systems"));
    SystemReadyStates[TEXT("AudioSystems")] = true;
}

void UEng_ArchitectureManager::InitializeVFXSystems()
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Initializing VFX Systems"));
    SystemReadyStates[TEXT("VFXSystems")] = true;
}

void UEng_ArchitectureManager::UpdatePerformanceMetrics()
{
    if (!bEnablePerformanceMonitoring)
    {
        return;
    }
    
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (CurrentTime - LastPerformanceUpdate >= 1.0f) // Update every second
    {
        // Update performance metrics for each system
        for (auto& MetricPair : PerformanceMetrics)
        {
            // Simulate performance monitoring - in real implementation,
            // this would gather actual performance data
            float CurrentFPS = FMath::RandRange(55.0f, 65.0f);
            MetricPair.Value = CurrentFPS;
        }
        
        LastPerformanceUpdate = CurrentTime;
    }
}

bool UEng_ArchitectureManager::ValidateSystemPerformance(const FString& SystemName) const
{
    const float* CurrentMetric = PerformanceMetrics.Find(SystemName);
    if (!CurrentMetric)
    {
        return false;
    }
    
    // Performance is valid if above 30 FPS minimum
    return *CurrentMetric >= 30.0f;
}

bool UEng_ArchitectureManager::LoadGameModule(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Loading game module %s"), *ModuleName);
    
    // In a real implementation, this would use the module loading system
    // For now, we simulate successful loading
    RegisterGameModule(ModuleName, 100);
    
    return true;
}

void UEng_ArchitectureManager::OnModuleLoaded(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Module %s loaded successfully"), *ModuleName);
}

void UEng_ArchitectureManager::OnModuleUnloaded(const FString& ModuleName)
{
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Module %s unloaded successfully"), *ModuleName);
}
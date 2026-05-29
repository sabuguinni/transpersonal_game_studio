#include "Eng_SystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_SystemManager::UEng_SystemManager()
{
    bAllSystemsInitialized = false;
    LastPerformanceCheck = 0.0f;
    LastMetricsUpdate = FDateTime::Now();
    
    // Define core systems in initialization order
    CoreSystems = {
        TEXT("Physics"),
        TEXT("WorldGeneration"),
        TEXT("Environment"),
        TEXT("Characters"),
        TEXT("AI"),
        TEXT("Combat"),
        TEXT("Audio"),
        TEXT("VFX"),
        TEXT("UI")
    };
}

void UEng_SystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine System Manager: Initializing"));
    
    // Initialize core system tracking
    for (const FString& SystemName : CoreSystems)
    {
        SystemInitializationStatus.Add(SystemName, false);
        SystemPriorities.Add(SystemName, CoreSystems.Find(SystemName));
        SystemPerformanceMetrics.Add(SystemName, 0.0f);
    }
    
    // Start initialization process
    InitializeAllSystems();
}

void UEng_SystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine System Manager: Shutting down"));
    
    ShutdownAllSystems();
    
    SystemInitializationStatus.Empty();
    SystemPriorities.Empty();
    SystemPerformanceMetrics.Empty();
    
    Super::Deinitialize();
}

void UEng_SystemManager::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine System Manager: Starting system initialization"));
    
    // Initialize systems in priority order
    for (const FString& SystemName : CoreSystems)
    {
        InitializeSystemByName(SystemName);
    }
    
    // Validate all systems are ready
    ValidateSystemDependencies();
    
    bAllSystemsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Engine System Manager: All systems initialized"));
}

void UEng_SystemManager::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine System Manager: Shutting down all systems"));
    
    // Shutdown in reverse order
    for (int32 i = CoreSystems.Num() - 1; i >= 0; i--)
    {
        const FString& SystemName = CoreSystems[i];
        if (SystemInitializationStatus.Contains(SystemName))
        {
            SystemInitializationStatus[SystemName] = false;
            UE_LOG(LogTemp, Log, TEXT("System shutdown: %s"), *SystemName);
        }
    }
    
    bAllSystemsInitialized = false;
}

bool UEng_SystemManager::IsSystemInitialized(const FString& SystemName) const
{
    if (const bool* Status = SystemInitializationStatus.Find(SystemName))
    {
        return *Status;
    }
    return false;
}

float UEng_SystemManager::GetSystemPerformanceMetric(const FString& SystemName) const
{
    if (const float* Metric = SystemPerformanceMetrics.Find(SystemName))
    {
        return *Metric;
    }
    return 0.0f;
}

void UEng_SystemManager::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS REPORT ==="));
    
    for (const FString& SystemName : CoreSystems)
    {
        bool bInitialized = IsSystemInitialized(SystemName);
        float Performance = GetSystemPerformanceMetric(SystemName);
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Performance: %.2f)"), 
            *SystemName, 
            bInitialized ? TEXT("READY") : TEXT("NOT READY"),
            Performance
        );
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All Systems Ready: %s"), 
        bAllSystemsInitialized ? TEXT("YES") : TEXT("NO"));
}

void UEng_SystemManager::RegisterSystem(const FString& SystemName, int32 InitializationPriority)
{
    UE_LOG(LogTemp, Log, TEXT("Registering system: %s (Priority: %d)"), *SystemName, InitializationPriority);
    
    SystemInitializationStatus.Add(SystemName, false);
    SystemPriorities.Add(SystemName, InitializationPriority);
    SystemPerformanceMetrics.Add(SystemName, 0.0f);
}

void UEng_SystemManager::UnregisterSystem(const FString& SystemName)
{
    UE_LOG(LogTemp, Log, TEXT("Unregistering system: %s"), *SystemName);
    
    SystemInitializationStatus.Remove(SystemName);
    SystemPriorities.Remove(SystemName);
    SystemPerformanceMetrics.Remove(SystemName);
}

void UEng_SystemManager::InitializeSystemByName(const FString& SystemName)
{
    UE_LOG(LogTemp, Log, TEXT("Initializing system: %s"), *SystemName);
    
    // Mark system as initialized (actual system initialization would happen here)
    if (SystemInitializationStatus.Contains(SystemName))
    {
        SystemInitializationStatus[SystemName] = true;
        
        // Set initial performance metric
        SystemPerformanceMetrics[SystemName] = 1.0f;
        
        UE_LOG(LogTemp, Log, TEXT("System %s initialized successfully"), *SystemName);
    }
}

void UEng_SystemManager::UpdatePerformanceMetrics()
{
    FDateTime Now = FDateTime::Now();
    float DeltaTime = (Now - LastMetricsUpdate).GetTotalSeconds();
    
    if (DeltaTime >= METRICS_UPDATE_INTERVAL)
    {
        // Update performance metrics for all systems
        for (auto& Pair : SystemPerformanceMetrics)
        {
            // Simulate performance metric (would be real metrics in production)
            float& Metric = Pair.Value;
            Metric = FMath::RandRange(0.8f, 1.2f); // Simulated performance variance
        }
        
        LastMetricsUpdate = Now;
    }
}

void UEng_SystemManager::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Log, TEXT("Validating system dependencies"));
    
    // Check that all core systems are initialized
    bool bAllReady = true;
    for (const FString& SystemName : CoreSystems)
    {
        if (!IsSystemInitialized(SystemName))
        {
            UE_LOG(LogTemp, Warning, TEXT("System not ready: %s"), *SystemName);
            bAllReady = false;
        }
    }
    
    if (bAllReady)
    {
        UE_LOG(LogTemp, Warning, TEXT("All system dependencies validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("System dependency validation failed"));
    }
}
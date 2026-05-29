#include "Eng_CriticalSystemsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_PhysicsCore.h"
#include "Eng_SystemManager.h"
#include "Eng_PerformanceMonitor.h"

UEng_CriticalSystemsManager::UEng_CriticalSystemsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS monitoring
    
    // Initialize critical system states
    bPhysicsSystemActive = false;
    bRenderingSystemActive = false;
    bAudioSystemActive = false;
    bInputSystemActive = false;
    bNetworkingSystemActive = false;
    bMemorySystemActive = false;
    
    // Performance thresholds
    MaxFrameTime = 33.33f; // 30 FPS minimum
    MaxMemoryUsage = 0.85f; // 85% of available memory
    MaxCPUUsage = 0.80f; // 80% CPU usage
    
    // Critical system references
    PhysicsManager = nullptr;
    SystemManager = nullptr;
    PerformanceMonitor = nullptr;
    
    // Monitoring state
    bMonitoringEnabled = true;
    LastSystemCheck = 0.0f;
    SystemCheckInterval = 1.0f; // Check every second
    
    UE_LOG(LogTemp, Log, TEXT("Eng_CriticalSystemsManager: Initialized"));
}

void UEng_CriticalSystemsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize critical system monitoring
    InitializeCriticalSystems();
    
    // Start performance monitoring
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Eng_CriticalSystemsManager: BeginPlay complete"));
}

void UEng_CriticalSystemsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bMonitoringEnabled)
    {
        return;
    }
    
    // Update monitoring timers
    LastSystemCheck += DeltaTime;
    
    // Perform system checks at intervals
    if (LastSystemCheck >= SystemCheckInterval)
    {
        PerformCriticalSystemCheck();
        LastSystemCheck = 0.0f;
    }
    
    // Continuous performance monitoring
    MonitorFramePerformance(DeltaTime);
}

void UEng_CriticalSystemsManager::InitializeCriticalSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Eng_CriticalSystemsManager: Initializing critical systems"));
    
    // Find or create physics manager
    if (AActor* Owner = GetOwner())
    {
        PhysicsManager = Owner->FindComponentByClass<UEng_PhysicsCore>();
        if (!PhysicsManager)
        {
            PhysicsManager = NewObject<UEng_PhysicsCore>(Owner);
            Owner->AddInstanceComponent(PhysicsManager);
            UE_LOG(LogTemp, Warning, TEXT("Created new PhysicsManager component"));
        }
    }
    
    // Find or create system manager
    if (AActor* Owner = GetOwner())
    {
        SystemManager = Owner->FindComponentByClass<UEng_SystemManager>();
        if (!SystemManager)
        {
            SystemManager = NewObject<UEng_SystemManager>(Owner);
            Owner->AddInstanceComponent(SystemManager);
            UE_LOG(LogTemp, Warning, TEXT("Created new SystemManager component"));
        }
    }
    
    // Find or create performance monitor
    if (AActor* Owner = GetOwner())
    {
        PerformanceMonitor = Owner->FindComponentByClass<UEng_PerformanceMonitor>();
        if (!PerformanceMonitor)
        {
            PerformanceMonitor = NewObject<UEng_PerformanceMonitor>(Owner);
            Owner->AddInstanceComponent(PerformanceMonitor);
            UE_LOG(LogTemp, Warning, TEXT("Created new PerformanceMonitor component"));
        }
    }
    
    // Initialize system states
    UpdateSystemStates();
}

void UEng_CriticalSystemsManager::StartPerformanceMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("Eng_CriticalSystemsManager: Starting performance monitoring"));
    
    bMonitoringEnabled = true;
    LastSystemCheck = 0.0f;
    
    // Initialize performance baselines
    if (PerformanceMonitor)
    {
        // Performance monitor will handle detailed metrics
        UE_LOG(LogTemp, Log, TEXT("Performance monitoring delegated to PerformanceMonitor"));
    }
}

void UEng_CriticalSystemsManager::PerformCriticalSystemCheck()
{
    // Update all critical system states
    UpdateSystemStates();
    
    // Check for system failures
    TArray<FString> FailedSystems;
    
    if (!bPhysicsSystemActive)
    {
        FailedSystems.Add(TEXT("Physics"));
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Physics system is not active"));
    }
    
    if (!bRenderingSystemActive)
    {
        FailedSystems.Add(TEXT("Rendering"));
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Rendering system is not active"));
    }
    
    if (!bInputSystemActive)
    {
        FailedSystems.Add(TEXT("Input"));
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Input system is not active"));
    }
    
    // Attempt recovery for failed systems
    if (FailedSystems.Num() > 0)
    {
        AttemptSystemRecovery(FailedSystems);
    }
    
    // Log system status
    UE_LOG(LogTemp, Log, TEXT("System Status - Physics:%s Rendering:%s Audio:%s Input:%s Network:%s Memory:%s"),
        bPhysicsSystemActive ? TEXT("OK") : TEXT("FAIL"),
        bRenderingSystemActive ? TEXT("OK") : TEXT("FAIL"),
        bAudioSystemActive ? TEXT("OK") : TEXT("FAIL"),
        bInputSystemActive ? TEXT("OK") : TEXT("FAIL"),
        bNetworkingSystemActive ? TEXT("OK") : TEXT("FAIL"),
        bMemorySystemActive ? TEXT("OK") : TEXT("FAIL"));
}

void UEng_CriticalSystemsManager::UpdateSystemStates()
{
    // Check physics system
    bPhysicsSystemActive = (PhysicsManager != nullptr) && IsValid(PhysicsManager);
    
    // Check rendering system (always active if we're running)
    bRenderingSystemActive = (GEngine != nullptr) && (GEngine->GetWorld() != nullptr);
    
    // Check audio system
    bAudioSystemActive = (GEngine != nullptr) && (GEngine->GetAudioDeviceManager() != nullptr);
    
    // Check input system
    if (UWorld* World = GetWorld())
    {
        bInputSystemActive = (World->GetFirstPlayerController() != nullptr);
    }
    else
    {
        bInputSystemActive = false;
    }
    
    // Check networking system (optional for single player)
    bNetworkingSystemActive = true; // Assume OK for single player
    
    // Check memory system (simplified check)
    bMemorySystemActive = (FPlatformMemory::GetStats().AvailablePhysical > 0);
}

void UEng_CriticalSystemsManager::MonitorFramePerformance(float DeltaTime)
{
    // Convert DeltaTime to milliseconds
    float FrameTimeMs = DeltaTime * 1000.0f;
    
    // Check against performance thresholds
    if (FrameTimeMs > MaxFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: Frame time %.2fms exceeds threshold %.2fms"), 
            FrameTimeMs, MaxFrameTime);
        
        // Trigger performance optimization if available
        if (PerformanceMonitor)
        {
            // Delegate to performance monitor for detailed analysis
        }
    }
    
    // Update performance history (simple moving average)
    static float FrameTimeHistory[10] = {0};
    static int32 HistoryIndex = 0;
    
    FrameTimeHistory[HistoryIndex] = FrameTimeMs;
    HistoryIndex = (HistoryIndex + 1) % 10;
    
    // Calculate average frame time over last 10 frames
    float AverageFrameTime = 0.0f;
    for (int32 i = 0; i < 10; i++)
    {
        AverageFrameTime += FrameTimeHistory[i];
    }
    AverageFrameTime /= 10.0f;
    
    // Log performance metrics periodically
    static float LastPerfLog = 0.0f;
    LastPerfLog += DeltaTime;
    if (LastPerfLog >= 5.0f) // Log every 5 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("Performance: Current=%.2fms Average=%.2fms Target=%.2fms"), 
            FrameTimeMs, AverageFrameTime, MaxFrameTime);
        LastPerfLog = 0.0f;
    }
}

void UEng_CriticalSystemsManager::AttemptSystemRecovery(const TArray<FString>& FailedSystems)
{
    UE_LOG(LogTemp, Warning, TEXT("Attempting recovery for %d failed systems"), FailedSystems.Num());
    
    for (const FString& SystemName : FailedSystems)
    {
        if (SystemName == TEXT("Physics"))
        {
            // Attempt to reinitialize physics system
            if (!PhysicsManager && GetOwner())
            {
                PhysicsManager = NewObject<UEng_PhysicsCore>(GetOwner());
                GetOwner()->AddInstanceComponent(PhysicsManager);
                UE_LOG(LogTemp, Log, TEXT("Attempted to recreate PhysicsManager"));
            }
        }
        else if (SystemName == TEXT("Input"))
        {
            // Input system recovery is handled by the engine
            UE_LOG(LogTemp, Log, TEXT("Input system recovery delegated to engine"));
        }
        // Add more recovery procedures as needed
    }
}

bool UEng_CriticalSystemsManager::IsSystemHealthy() const
{
    return bPhysicsSystemActive && 
           bRenderingSystemActive && 
           bInputSystemActive && 
           bMemorySystemActive;
}

FEng_SystemHealthReport UEng_CriticalSystemsManager::GetSystemHealthReport() const
{
    FEng_SystemHealthReport Report;
    
    Report.bPhysicsHealthy = bPhysicsSystemActive;
    Report.bRenderingHealthy = bRenderingSystemActive;
    Report.bAudioHealthy = bAudioSystemActive;
    Report.bInputHealthy = bInputSystemActive;
    Report.bNetworkingHealthy = bNetworkingSystemActive;
    Report.bMemoryHealthy = bMemorySystemActive;
    Report.bOverallHealthy = IsSystemHealthy();
    
    // Calculate health percentage
    int32 HealthySystems = 0;
    int32 TotalSystems = 6;
    
    if (Report.bPhysicsHealthy) HealthySystems++;
    if (Report.bRenderingHealthy) HealthySystems++;
    if (Report.bAudioHealthy) HealthySystems++;
    if (Report.bInputHealthy) HealthySystems++;
    if (Report.bNetworkingHealthy) HealthySystems++;
    if (Report.bMemoryHealthy) HealthySystems++;
    
    Report.HealthPercentage = (float)HealthySystems / (float)TotalSystems;
    Report.Timestamp = FDateTime::Now();
    
    return Report;
}

void UEng_CriticalSystemsManager::SetMonitoringEnabled(bool bEnabled)
{
    bMonitoringEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("Critical systems monitoring %s"), 
        bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UEng_CriticalSystemsManager::SetPerformanceThresholds(float FrameTimeMs, float MemoryPercent, float CPUPercent)
{
    MaxFrameTime = FrameTimeMs;
    MaxMemoryUsage = FMath::Clamp(MemoryPercent, 0.1f, 1.0f);
    MaxCPUUsage = FMath::Clamp(CPUPercent, 0.1f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Performance thresholds updated: Frame=%.2fms Memory=%.1f%% CPU=%.1f%%"),
        MaxFrameTime, MaxMemoryUsage * 100.0f, MaxCPUUsage * 100.0f);
}

void UEng_CriticalSystemsManager::ForceSystemCheck()
{
    UE_LOG(LogTemp, Log, TEXT("Forcing immediate critical system check"));
    PerformCriticalSystemCheck();
}
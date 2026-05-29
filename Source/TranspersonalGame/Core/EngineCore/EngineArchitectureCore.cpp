#include "EngineArchitectureCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

AEngineArchitectureCore::AEngineArchitectureCore()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    // Initialize default values
    bEnablePerformanceMonitoring = true;
    bEnableSystemValidation = true;
    MaxFrameTimeMs = 16.67f; // 60 FPS target
    SystemUpdateFrequency = 10.0f; // 10 Hz
    
    // Initialize performance tracking
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    LastValidationTime = 0.0f;
    SystemUpdateTimer = 0.0f;
}

void AEngineArchitectureCore::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Initializing core architecture systems"));
    
    // Initialize core architecture
    InitializeCoreArchitecture();
    
    // Perform initial system validation
    if (bEnableSystemValidation)
    {
        ValidateAllSystems();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Initialization complete"));
}

void AEngineArchitectureCore::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Shutting down architecture systems"));
    
    // Clear all registered systems
    RegisteredSystems.Empty();
    SystemHealthCache.Empty();
    
    Super::EndPlay(EndPlayReason);
}

void AEngineArchitectureCore::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update performance metrics
    if (bEnablePerformanceMonitoring)
    {
        UpdatePerformanceMetrics(DeltaTime);
    }
    
    // Update system validation timer
    SystemUpdateTimer += DeltaTime;
    if (SystemUpdateTimer >= (1.0f / SystemUpdateFrequency))
    {
        SystemUpdateTimer = 0.0f;
        
        // Perform periodic system validation
        if (bEnableSystemValidation)
        {
            ValidateAllSystems();
        }
    }
}

bool AEngineArchitectureCore::RegisterSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (SystemName.IsEmpty() || !SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: Cannot register system - invalid parameters"));
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: System '%s' already registered - updating reference"), *SystemName);
    }
    
    RegisteredSystems.Add(SystemName, SystemInstance);
    SystemHealthCache.Add(SystemName, true); // Assume healthy until proven otherwise
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Successfully registered system '%s'"), *SystemName);
    return true;
}

bool AEngineArchitectureCore::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Cannot unregister system '%s' - not found"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    SystemHealthCache.Remove(SystemName);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Successfully unregistered system '%s'"), *SystemName);
    return true;
}

UObject* AEngineArchitectureCore::GetSystem(const FString& SystemName)
{
    if (UObject** FoundSystem = RegisteredSystems.Find(SystemName))
    {
        return *FoundSystem;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: System '%s' not found"), *SystemName);
    return nullptr;
}

bool AEngineArchitectureCore::ValidateAllSystems()
{
    bool bAllSystemsHealthy = true;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        const FString& SystemName = SystemPair.Key;
        UObject* SystemInstance = SystemPair.Value;
        
        bool bSystemHealthy = ValidateSystem(SystemName, SystemInstance);
        SystemHealthCache.Add(SystemName, bSystemHealthy);
        
        if (!bSystemHealthy)
        {
            bAllSystemsHealthy = false;
        }
    }
    
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    if (bAllSystemsHealthy)
    {
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: All systems validated successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Some systems failed validation"));
        LogSystemStatus();
    }
    
    return bAllSystemsHealthy;
}

TMap<FString, bool> AEngineArchitectureCore::GetSystemHealthStatus()
{
    return SystemHealthCache;
}

float AEngineArchitectureCore::GetCurrentFrameTime() const
{
    if (FrameCount > 0)
    {
        return (FrameTimeAccumulator / FrameCount) * 1000.0f; // Convert to milliseconds
    }
    return 0.0f;
}

FString AEngineArchitectureCore::GetMemoryUsageStats() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    return FString::Printf(TEXT("Memory - Used: %.2f MB, Available: %.2f MB, Peak: %.2f MB"),
        MemStats.UsedPhysical / (1024.0f * 1024.0f),
        MemStats.AvailablePhysical / (1024.0f * 1024.0f),
        MemStats.PeakUsedPhysical / (1024.0f * 1024.0f));
}

void AEngineArchitectureCore::InitializeCoreArchitecture()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Initializing core architecture systems"));
    
    // Register self as the core architecture system
    RegisterSystem(TEXT("EngineArchitectureCore"), this);
    
    // Initialize performance monitoring
    FrameTimeAccumulator = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: Core architecture initialization complete"));
}

bool AEngineArchitectureCore::ValidateSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        HandleSystemFailure(SystemName, TEXT("System instance is null"));
        return false;
    }
    
    if (!IsValid(SystemInstance))
    {
        HandleSystemFailure(SystemName, TEXT("System instance is not valid"));
        return false;
    }
    
    // Check if the system is pending kill
    if (SystemInstance->IsPendingKill())
    {
        HandleSystemFailure(SystemName, TEXT("System instance is pending kill"));
        return false;
    }
    
    // System passed all basic validation checks
    return true;
}

void AEngineArchitectureCore::UpdatePerformanceMetrics(float DeltaTime)
{
    FrameTimeAccumulator += DeltaTime;
    FrameCount++;
    
    // Check for frame time warnings
    float CurrentFrameTimeMs = DeltaTime * 1000.0f;
    if (CurrentFrameTimeMs > MaxFrameTimeMs)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureCore: Frame time exceeded target - %.2f ms (target: %.2f ms)"), 
            CurrentFrameTimeMs, MaxFrameTimeMs);
    }
    
    // Reset accumulator periodically to prevent overflow
    if (FrameCount >= 1000)
    {
        FrameTimeAccumulator = 0.0f;
        FrameCount = 0;
    }
}

void AEngineArchitectureCore::LogSystemStatus()
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureCore: System Status Report"));
    UE_LOG(LogTemp, Log, TEXT("========================================"));
    
    for (auto& HealthPair : SystemHealthCache)
    {
        const FString& SystemName = HealthPair.Key;
        bool bIsHealthy = HealthPair.Value;
        
        FString StatusText = bIsHealthy ? TEXT("HEALTHY") : TEXT("FAILED");
        UE_LOG(LogTemp, Log, TEXT("  %s: %s"), *SystemName, *StatusText);
    }
    
    UE_LOG(LogTemp, Log, TEXT("========================================"));
    UE_LOG(LogTemp, Log, TEXT("Performance: %.2f ms/frame"), GetCurrentFrameTime());
    UE_LOG(LogTemp, Log, TEXT("%s"), *GetMemoryUsageStats());
}

void AEngineArchitectureCore::HandleSystemFailure(const FString& SystemName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("EngineArchitectureCore: System '%s' failed validation - %s"), 
        *SystemName, *ErrorMessage);
    
    // Mark system as unhealthy
    SystemHealthCache.Add(SystemName, false);
    
    // Could implement recovery strategies here in the future
}
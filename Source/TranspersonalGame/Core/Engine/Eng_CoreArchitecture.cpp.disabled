#include "Eng_CoreArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/StaticMeshActor.h"

UEng_CoreArchitecture::UEng_CoreArchitecture()
    : ArchitectureStatus(EEng_ArchitectureStatus::Uninitialized)
    , bIsInitialized(false)
    , InitializationTime(0.0f)
    , LastPerformanceUpdate(0.0f)
    , MaxFrameTime(33.33f)  // 30 FPS limit
    , MaxDrawCalls(5000)
    , MaxMemoryUsageMB(8192.0f)  // 8GB limit
{
    // Initialize performance metrics
    CurrentMetrics = FEng_PerformanceMetrics();
}

void UEng_CoreArchitecture::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Initializing Core Architecture Subsystem"));
    
    ArchitectureStatus = EEng_ArchitectureStatus::Initializing;
    
    // Initialize core systems
    InitializeArchitecture();
}

void UEng_CoreArchitecture::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Deinitializing Core Architecture Subsystem"));
    
    ShutdownArchitecture();
    
    Super::Deinitialize();
}

void UEng_CoreArchitecture::InitializeArchitecture()
{
    if (bIsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architecture: Already initialized"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Starting architecture initialization"));

    // Record initialization start time
    InitializationTime = FPlatformTime::Seconds();

    // Clear any existing data
    RegisteredSystems.Empty();
    SystemHealthData.Empty();
    SystemErrors.Empty();

    // Initialize performance monitoring
    UpdatePerformanceMetrics();

    // Set status to active
    ArchitectureStatus = EEng_ArchitectureStatus::Active;
    bIsInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Core architecture initialized successfully"));
}

void UEng_CoreArchitecture::ShutdownArchitecture()
{
    if (!bIsInitialized)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Shutting down architecture"));

    // Cleanup systems
    CleanupInvalidSystems();
    RegisteredSystems.Empty();
    SystemHealthData.Empty();
    SystemErrors.Empty();

    // Reset status
    ArchitectureStatus = EEng_ArchitectureStatus::Uninitialized;
    bIsInitialized = false;

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Shutdown complete"));
}

bool UEng_CoreArchitecture::IsArchitectureHealthy() const
{
    if (!bIsInitialized || ArchitectureStatus != EEng_ArchitectureStatus::Active)
    {
        return false;
    }

    // Check if performance is within limits
    if (!IsPerformanceWithinLimits())
    {
        return false;
    }

    // Check system health
    for (const FEng_SystemHealth& Health : SystemHealthData)
    {
        if (Health.Status == EEng_ArchitectureStatus::Error)
        {
            return false;
        }
    }

    return true;
}

EEng_ArchitectureStatus UEng_CoreArchitecture::GetArchitectureStatus() const
{
    return ArchitectureStatus;
}

void UEng_CoreArchitecture::RegisterSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("Engine Architecture: Cannot register null system: %s"), *SystemName);
        return;
    }

    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architecture: System already registered: %s"), *SystemName);
        return;
    }

    // Register the system
    RegisteredSystems.Add(SystemName, SystemInstance);

    // Create health data
    FEng_SystemHealth NewHealth;
    NewHealth.SystemName = SystemName;
    NewHealth.Status = EEng_ArchitectureStatus::Active;
    NewHealth.LastUpdateTime = FPlatformTime::Seconds();
    NewHealth.LastError = TEXT("");
    NewHealth.ErrorCount = 0;

    SystemHealthData.Add(NewHealth);

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Registered system: %s"), *SystemName);
}

void UEng_CoreArchitecture::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architecture: System not registered: %s"), *SystemName);
        return;
    }

    // Remove from registry
    RegisteredSystems.Remove(SystemName);

    // Remove health data
    SystemHealthData.RemoveAll([&SystemName](const FEng_SystemHealth& Health)
    {
        return Health.SystemName == SystemName;
    });

    // Clear errors
    SystemErrors.Remove(SystemName);

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Unregistered system: %s"), *SystemName);
}

bool UEng_CoreArchitecture::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FEng_SystemHealth> UEng_CoreArchitecture::GetSystemHealthReport() const
{
    return SystemHealthData;
}

FEng_PerformanceMetrics UEng_CoreArchitecture::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEng_CoreArchitecture::UpdatePerformanceMetrics()
{
    float CurrentTime = FPlatformTime::Seconds();
    
    // Update frame time
    static float LastFrameTime = CurrentTime;
    CurrentMetrics.FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to ms
    LastFrameTime = CurrentTime;

    // Get basic engine stats
    if (GEngine)
    {
        UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::LogAndReturnNull);
        if (World)
        {
            // Count active actors
            CurrentMetrics.ActiveActors = World->GetActorCount();
        }
    }

    // Estimate memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);

    // Update render stats (simplified)
    CurrentMetrics.GameThreadTime = CurrentMetrics.FrameTime * 0.7f; // Estimate
    CurrentMetrics.RenderThreadTime = CurrentMetrics.FrameTime * 0.3f; // Estimate
    CurrentMetrics.DrawCalls = FMath::Max(100, CurrentMetrics.ActiveActors * 2); // Rough estimate

    LastPerformanceUpdate = CurrentTime;
}

bool UEng_CoreArchitecture::IsPerformanceWithinLimits() const
{
    if (CurrentMetrics.FrameTime > MaxFrameTime)
    {
        return false;
    }

    if (CurrentMetrics.DrawCalls > MaxDrawCalls)
    {
        return false;
    }

    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB)
    {
        return false;
    }

    return true;
}

void UEng_CoreArchitecture::ReportSystemError(const FString& SystemName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Engine Architecture: System error in %s: %s"), *SystemName, *ErrorMessage);

    // Add to error list
    if (!SystemErrors.Contains(SystemName))
    {
        SystemErrors.Add(SystemName, TArray<FString>());
    }
    SystemErrors[SystemName].Add(ErrorMessage);

    // Update system health
    FEng_SystemHealth* Health = FindSystemHealth(SystemName);
    if (Health)
    {
        Health->Status = EEng_ArchitectureStatus::Error;
        Health->LastError = ErrorMessage;
        Health->ErrorCount++;
        Health->LastUpdateTime = FPlatformTime::Seconds();
    }
}

void UEng_CoreArchitecture::ClearSystemErrors(const FString& SystemName)
{
    SystemErrors.Remove(SystemName);

    // Reset system health
    FEng_SystemHealth* Health = FindSystemHealth(SystemName);
    if (Health)
    {
        Health->Status = EEng_ArchitectureStatus::Active;
        Health->LastError = TEXT("");
        Health->ErrorCount = 0;
        Health->LastUpdateTime = FPlatformTime::Seconds();
    }

    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Cleared errors for system: %s"), *SystemName);
}

TArray<FString> UEng_CoreArchitecture::GetSystemErrors(const FString& SystemName) const
{
    if (SystemErrors.Contains(SystemName))
    {
        return SystemErrors[SystemName];
    }
    return TArray<FString>();
}

bool UEng_CoreArchitecture::ValidateArchitectureIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Validating architecture integrity"));

    bool bIsValid = true;

    // Check if core systems are responsive
    CleanupInvalidSystems();

    // Update system health
    UpdateSystemHealth();

    // Check performance
    UpdatePerformanceMetrics();
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architecture: Performance outside limits"));
        bIsValid = false;
    }

    // Check for critical errors
    for (const auto& ErrorPair : SystemErrors)
    {
        if (ErrorPair.Value.Num() > 10) // Too many errors
        {
            UE_LOG(LogTemp, Error, TEXT("Engine Architecture: System %s has too many errors (%d)"), 
                *ErrorPair.Key, ErrorPair.Value.Num());
            bIsValid = false;
        }
    }

    return bIsValid;
}

void UEng_CoreArchitecture::RunArchitectureDiagnostics()
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architecture: Running diagnostics"));

    // Update metrics
    UpdatePerformanceMetrics();

    // Log current status
    UE_LOG(LogTemp, Log, TEXT("Architecture Status: %s"), 
        *UEnum::GetValueAsString(ArchitectureStatus));
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActors);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);

    // Check each system
    for (const FEng_SystemHealth& Health : SystemHealthData)
    {
        UE_LOG(LogTemp, Log, TEXT("System %s: %s (Errors: %d)"), 
            *Health.SystemName, 
            *UEnum::GetValueAsString(Health.Status),
            Health.ErrorCount);
    }
}

void UEng_CoreArchitecture::UpdateSystemHealth()
{
    float CurrentTime = FPlatformTime::Seconds();

    for (FEng_SystemHealth& Health : SystemHealthData)
    {
        // Check if system is still valid
        if (RegisteredSystems.Contains(Health.SystemName))
        {
            TWeakObjectPtr<UObject> SystemPtr = RegisteredSystems[Health.SystemName];
            if (!SystemPtr.IsValid())
            {
                Health.Status = EEng_ArchitectureStatus::Error;
                Health.LastError = TEXT("System object is no longer valid");
                Health.ErrorCount++;
            }
            else if (Health.Status == EEng_ArchitectureStatus::Error && Health.ErrorCount == 0)
            {
                // Recover from error state if no errors
                Health.Status = EEng_ArchitectureStatus::Active;
                Health.LastError = TEXT("");
            }
        }
        
        Health.LastUpdateTime = CurrentTime;
    }
}

void UEng_CoreArchitecture::ValidateSystemIntegrity()
{
    // Remove invalid systems
    CleanupInvalidSystems();
    
    // Update health data
    UpdateSystemHealth();
}

FEng_SystemHealth* UEng_CoreArchitecture::FindSystemHealth(const FString& SystemName)
{
    for (FEng_SystemHealth& Health : SystemHealthData)
    {
        if (Health.SystemName == SystemName)
        {
            return &Health;
        }
    }
    return nullptr;
}

void UEng_CoreArchitecture::CleanupInvalidSystems()
{
    TArray<FString> SystemsToRemove;

    for (auto& SystemPair : RegisteredSystems)
    {
        if (!SystemPair.Value.IsValid())
        {
            SystemsToRemove.Add(SystemPair.Key);
        }
    }

    for (const FString& SystemName : SystemsToRemove)
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architecture: Removing invalid system: %s"), *SystemName);
        UnregisterSystem(SystemName);
    }
}

// Architecture Component Implementation

UEng_ArchitectureComponent::UEng_ArchitectureComponent()
    : ComponentSystemName(TEXT("DefaultComponent"))
    , ComponentStatus(EEng_ArchitectureStatus::Uninitialized)
    , bIsRegistered(false)
    , bEnableHealthMonitoring(true)
    , HealthCheckInterval(5.0f)
    , LastHealthCheck(0.0f)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEng_ArchitectureComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get architecture subsystem
    if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
    {
        ArchitectureSubsystem = GameInstance->GetSubsystem<UEng_CoreArchitecture>();
    }

    // Auto-register with architecture
    if (bEnableHealthMonitoring)
    {
        RegisterWithArchitecture();
    }

    ComponentStatus = EEng_ArchitectureStatus::Active;
}

void UEng_ArchitectureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UnregisterFromArchitecture();
    
    Super::EndPlay(EndPlayReason);
}

void UEng_ArchitectureComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bEnableHealthMonitoring && bIsRegistered)
    {
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastHealthCheck >= HealthCheckInterval)
        {
            // Perform health check
            ReportComponentHealth(ComponentStatus);
            LastHealthCheck = CurrentTime;
        }
    }
}

void UEng_ArchitectureComponent::RegisterWithArchitecture()
{
    if (bIsRegistered || !ArchitectureSubsystem.IsValid())
    {
        return;
    }

    // Generate unique system name if needed
    if (ComponentSystemName == TEXT("DefaultComponent"))
    {
        ComponentSystemName = FString::Printf(TEXT("%s_%s"), 
            *GetOwner()->GetClass()->GetName(), 
            *GetOwner()->GetName());
    }

    ArchitectureSubsystem->RegisterSystem(ComponentSystemName, this);
    bIsRegistered = true;

    UE_LOG(LogTemp, Log, TEXT("Architecture Component: Registered %s"), *ComponentSystemName);
}

void UEng_ArchitectureComponent::UnregisterFromArchitecture()
{
    if (!bIsRegistered || !ArchitectureSubsystem.IsValid())
    {
        return;
    }

    ArchitectureSubsystem->UnregisterSystem(ComponentSystemName);
    bIsRegistered = false;

    UE_LOG(LogTemp, Log, TEXT("Architecture Component: Unregistered %s"), *ComponentSystemName);
}

bool UEng_ArchitectureComponent::IsRegisteredWithArchitecture() const
{
    return bIsRegistered && ArchitectureSubsystem.IsValid();
}

void UEng_ArchitectureComponent::ReportComponentHealth(EEng_ArchitectureStatus Status, const FString& Message)
{
    ComponentStatus = Status;

    if (bIsRegistered && ArchitectureSubsystem.IsValid())
    {
        if (Status == EEng_ArchitectureStatus::Error && !Message.IsEmpty())
        {
            ArchitectureSubsystem->ReportSystemError(ComponentSystemName, Message);
        }
        else if (Status == EEng_ArchitectureStatus::Active)
        {
            ArchitectureSubsystem->ClearSystemErrors(ComponentSystemName);
        }
    }
}

EEng_ArchitectureStatus UEng_ArchitectureComponent::GetComponentStatus() const
{
    return ComponentStatus;
}
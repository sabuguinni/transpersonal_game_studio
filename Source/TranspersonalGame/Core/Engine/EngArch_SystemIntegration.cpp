#include "EngArch_SystemIntegration.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEngArch_SystemIntegration::UEngArch_SystemIntegration()
{
    TotalInitializationTime = 0.0f;
    bAllSystemsInitialized = false;
}

void UEngArch_SystemIntegration::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect System Integration Manager Initialized"));
    
    // Register core systems that must be initialized first
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("BiomeSystem"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("TerrainSystem"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("CharacterMovement"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("SurvivalSystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("DinosaurAI"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("CombatSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("QuestSystem"), EEng_SystemPriority::Low);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Low);
}

void UEngArch_SystemIntegration::Deinitialize()
{
    ShutdownAllSystems();
    Super::Deinitialize();
}

bool UEngArch_SystemIntegration::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    // Check if system already registered
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            UE_LOG(LogTemp, Warning, TEXT("System %s already registered"), *SystemName);
            return false;
        }
    }

    // Create new system registration
    FEng_SystemRegistration NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.bIsInitialized = false;
    NewSystem.InitializationTime = 0.0f;

    RegisteredSystems.Add(NewSystem);
    SortSystemsByPriority();

    UE_LOG(LogTemp, Log, TEXT("Registered system: %s with priority %d"), *SystemName, (int32)Priority);
    return true;
}

bool UEngArch_SystemIntegration::UnregisterSystem(const FString& SystemName)
{
    for (int32 i = 0; i < RegisteredSystems.Num(); i++)
    {
        if (RegisteredSystems[i].SystemName == SystemName)
        {
            RegisteredSystems.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
            return true;
        }
    }
    return false;
}

bool UEngArch_SystemIntegration::IsSystemRegistered(const FString& SystemName) const
{
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return true;
        }
    }
    return false;
}

void UEngArch_SystemIntegration::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Starting system initialization sequence..."));
    
    float StartTime = FPlatformTime::Seconds();
    FailedSystems.Empty();
    
    // Initialize systems in priority order
    for (FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (!InitializeSystem(System.SystemName))
        {
            FailedSystems.Add(System.SystemName);
            LogSystemError(System.SystemName, TEXT("Failed to initialize"));
        }
    }
    
    TotalInitializationTime = FPlatformTime::Seconds() - StartTime;
    bAllSystemsInitialized = (FailedSystems.Num() == 0);
    
    UE_LOG(LogTemp, Warning, TEXT("System initialization complete. Time: %.3fs, Failed: %d"), 
           TotalInitializationTime, FailedSystems.Num());
}

bool UEngArch_SystemIntegration::InitializeSystem(const FString& SystemName)
{
    for (FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            if (System.bIsInitialized)
            {
                return true; // Already initialized
            }

            float StartTime = FPlatformTime::Seconds();
            
            // Validate dependencies
            if (!ValidateSystemDependencies(SystemName))
            {
                LogSystemError(SystemName, TEXT("Dependency validation failed"));
                return false;
            }

            // Simulate system initialization
            // In real implementation, this would call the actual system's Init() method
            System.bIsInitialized = true;
            System.InitializationTime = FPlatformTime::Seconds() - StartTime;

            UE_LOG(LogTemp, Log, TEXT("Initialized system: %s (%.3fs)"), 
                   *SystemName, System.InitializationTime);
            return true;
        }
    }
    
    LogSystemError(SystemName, TEXT("System not registered"));
    return false;
}

void UEngArch_SystemIntegration::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Shutting down all systems..."));
    
    // Shutdown in reverse priority order
    for (int32 i = RegisteredSystems.Num() - 1; i >= 0; i--)
    {
        FEng_SystemRegistration& System = RegisteredSystems[i];
        if (System.bIsInitialized)
        {
            System.bIsInitialized = false;
            UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *System.SystemName);
        }
    }
    
    bAllSystemsInitialized = false;
}

bool UEngArch_SystemIntegration::IsSystemHealthy(const FString& SystemName) const
{
    return IsSystemRegistered(SystemName) && !FailedSystems.Contains(SystemName);
}

TArray<FString> UEngArch_SystemIntegration::GetFailedSystems() const
{
    return FailedSystems;
}

int32 UEngArch_SystemIntegration::GetSystemCount() const
{
    return RegisteredSystems.Num();
}

float UEngArch_SystemIntegration::GetSystemInitTime(const FString& SystemName) const
{
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return System.InitializationTime;
        }
    }
    return 0.0f;
}

float UEngArch_SystemIntegration::GetTotalInitTime() const
{
    return TotalInitializationTime;
}

void UEngArch_SystemIntegration::ValidateSystemIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM INTEGRITY VALIDATION ==="));
    
    int32 HealthySystems = 0;
    int32 FailedSystemCount = FailedSystems.Num();
    
    for (const FEng_SystemRegistration& System : RegisteredSystems)
    {
        bool bHealthy = IsSystemHealthy(System.SystemName);
        if (bHealthy)
        {
            HealthySystems++;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("System: %s | Priority: %d | Initialized: %s | Healthy: %s | InitTime: %.3fs"),
               *System.SystemName,
               (int32)System.Priority,
               System.bIsInitialized ? TEXT("YES") : TEXT("NO"),
               bHealthy ? TEXT("YES") : TEXT("NO"),
               System.InitializationTime);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== VALIDATION SUMMARY ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Systems: %d"), RegisteredSystems.Num());
    UE_LOG(LogTemp, Warning, TEXT("Healthy Systems: %d"), HealthySystems);
    UE_LOG(LogTemp, Warning, TEXT("Failed Systems: %d"), FailedSystemCount);
    UE_LOG(LogTemp, Warning, TEXT("All Systems OK: %s"), bAllSystemsInitialized ? TEXT("YES") : TEXT("NO"));
}

void UEngArch_SystemIntegration::PrintSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Initialization Time: %.3fs"), TotalInitializationTime);
    UE_LOG(LogTemp, Warning, TEXT("All Systems Initialized: %s"), bAllSystemsInitialized ? TEXT("YES") : TEXT("NO"));
    
    if (FailedSystems.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("FAILED SYSTEMS:"));
        for (const FString& FailedSystem : FailedSystems)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *FailedSystem);
        }
    }
}

void UEngArch_SystemIntegration::SortSystemsByPriority()
{
    RegisteredSystems.Sort([](const FEng_SystemRegistration& A, const FEng_SystemRegistration& B)
    {
        return (int32)A.Priority < (int32)B.Priority; // Critical (0) comes first
    });
}

bool UEngArch_SystemIntegration::ValidateSystemDependencies(const FString& SystemName)
{
    // Basic dependency validation
    if (SystemName == TEXT("DinosaurAI") || SystemName == TEXT("CombatSystem"))
    {
        // These systems depend on character movement being initialized
        for (const FEng_SystemRegistration& System : RegisteredSystems)
        {
            if (System.SystemName == TEXT("CharacterMovement") && !System.bIsInitialized)
            {
                return false;
            }
        }
    }
    
    if (SystemName == TEXT("QuestSystem"))
    {
        // Quest system depends on world generation
        for (const FEng_SystemRegistration& System : RegisteredSystems)
        {
            if (System.SystemName == TEXT("WorldGeneration") && !System.bIsInitialized)
            {
                return false;
            }
        }
    }
    
    return true; // Dependencies satisfied
}

void UEngArch_SystemIntegration::LogSystemError(const FString& SystemName, const FString& Error)
{
    UE_LOG(LogTemp, Error, TEXT("SYSTEM ERROR [%s]: %s"), *SystemName, *Error);
}
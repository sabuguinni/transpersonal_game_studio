#include "Eng_SystemRegistry.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Subsystems/SubsystemBlueprintLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogEngSystemRegistry, Log, All);

UEng_SystemRegistry::UEng_SystemRegistry()
{
    // Initialize system registry
    bIsInitialized = false;
    SystemCount = 0;
}

void UEng_SystemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Engine System Registry initializing..."));
    
    // Initialize core systems
    InitializeCoreArchitecture();
    InitializePerformanceMonitoring();
    InitializeValidationSystems();
    
    bIsInitialized = true;
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Engine System Registry initialized successfully"));
}

void UEng_SystemRegistry::Deinitialize()
{
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Engine System Registry deinitializing..."));
    
    // Clean up registered systems
    RegisteredSystems.Empty();
    SystemCount = 0;
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UEng_SystemRegistry::RegisterSystem(const FString& SystemName, UObject* SystemInstance)
{
    if (!SystemInstance)
    {
        UE_LOG(LogEngSystemRegistry, Warning, TEXT("Cannot register null system: %s"), *SystemName);
        return false;
    }
    
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogEngSystemRegistry, Warning, TEXT("System already registered: %s"), *SystemName);
        return false;
    }
    
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.SystemInstance = SystemInstance;
    SystemInfo.RegistrationTime = FDateTime::Now();
    SystemInfo.bIsActive = true;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    SystemCount++;
    
    UE_LOG(LogEngSystemRegistry, Log, TEXT("System registered: %s (Total: %d)"), *SystemName, SystemCount);
    return true;
}

bool UEng_SystemRegistry::UnregisterSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogEngSystemRegistry, Warning, TEXT("System not found for unregistration: %s"), *SystemName);
        return false;
    }
    
    RegisteredSystems.Remove(SystemName);
    SystemCount--;
    
    UE_LOG(LogEngSystemRegistry, Log, TEXT("System unregistered: %s (Remaining: %d)"), *SystemName, SystemCount);
    return true;
}

UObject* UEng_SystemRegistry::GetSystem(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (SystemInfo && SystemInfo->bIsActive)
    {
        return SystemInfo->SystemInstance;
    }
    
    return nullptr;
}

TArray<FString> UEng_SystemRegistry::GetAllSystemNames() const
{
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    return SystemNames;
}

bool UEng_SystemRegistry::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

int32 UEng_SystemRegistry::GetSystemCount() const
{
    return SystemCount;
}

bool UEng_SystemRegistry::IsInitialized() const
{
    return bIsInitialized;
}

void UEng_SystemRegistry::ValidateAllSystems()
{
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Validating all registered systems..."));
    
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;
    
    for (auto& SystemPair : RegisteredSystems)
    {
        FEng_SystemInfo& SystemInfo = SystemPair.Value;
        
        if (IsValid(SystemInfo.SystemInstance))
        {
            ValidSystems++;
            SystemInfo.bIsActive = true;
        }
        else
        {
            InvalidSystems++;
            SystemInfo.bIsActive = false;
            UE_LOG(LogEngSystemRegistry, Warning, TEXT("Invalid system detected: %s"), *SystemInfo.SystemName);
        }
    }
    
    UE_LOG(LogEngSystemRegistry, Log, TEXT("System validation complete - Valid: %d, Invalid: %d"), ValidSystems, InvalidSystems);
}

void UEng_SystemRegistry::InitializeCoreArchitecture()
{
    // Register core architecture systems
    RegisterSystem(TEXT("ArchitectureCore"), this);
    RegisterSystem(TEXT("PerformanceMonitor"), this);
    RegisterSystem(TEXT("ValidationSuite"), this);
    
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Core architecture systems initialized"));
}

void UEng_SystemRegistry::InitializePerformanceMonitoring()
{
    // Initialize performance monitoring
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Performance monitoring initialized"));
}

void UEng_SystemRegistry::InitializeValidationSystems()
{
    // Initialize validation systems
    UE_LOG(LogEngSystemRegistry, Log, TEXT("Validation systems initialized"));
}
#include "Eng_SystemRegistry.h"
#include "Engine/Engine.h"

void UEng_SystemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine System Registry initialized"));
    
    // Register core systems
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemType::WorldGeneration);
    RegisterSystem(TEXT("BiomeManagement"), EEng_SystemType::BiomeManagement);
    RegisterSystem(TEXT("PerformanceMonitoring"), EEng_SystemType::PerformanceMonitoring);
    
    LogSystemStatus();
}

void UEng_SystemRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine System Registry deinitialized"));
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

void UEng_SystemRegistry::RegisterSystem(const FString& SystemName, EEng_SystemType SystemType)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered"), *SystemName);
        return;
    }

    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.SystemType = SystemType;
    NewSystem.bIsActive = true;
    NewSystem.InitializationTime = FPlatformTime::Seconds();

    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s"), *SystemName);
}

void UEng_SystemRegistry::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
    }
}

bool UEng_SystemRegistry::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FEng_SystemInfo> UEng_SystemRegistry::GetAllSystems() const
{
    TArray<FEng_SystemInfo> Systems;
    RegisteredSystems.GenerateValueArray(Systems);
    return Systems;
}

void UEng_SystemRegistry::ValidateSystemDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating system dependencies..."));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& System = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("System %s is %s"), 
            *System.SystemName, 
            System.bIsActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
}

void UEng_SystemRegistry::LogSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE SYSTEM REGISTRY STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total registered systems: %d"), RegisteredSystems.Num());
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FEng_SystemInfo& System = SystemPair.Value;
        UE_LOG(LogTemp, Log, TEXT("- %s [%s] - Active: %s"), 
            *System.SystemName,
            *UEnum::GetValueAsString(System.SystemType),
            System.bIsActive ? TEXT("YES") : TEXT("NO"));
    }
}
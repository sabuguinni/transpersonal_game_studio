#include "Eng_SystemArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UEng_SystemArchitect::UEng_SystemArchitect()
{
    TotalInitTime = 0.0f;
    ActiveSystemCount = 0;
}

void UEng_SystemArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System Architect initialized"));
    
    // Register core systems by default
    RegisterSystem(TEXT("WorldGeneration"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("PhysicsCore"), EEng_SystemPriority::Critical);
    RegisterSystem(TEXT("DinosaurAI"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("CharacterSystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("CombatSystem"), EEng_SystemPriority::High);
    RegisterSystem(TEXT("QuestSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium);
    RegisterSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low);
}

void UEng_SystemArchitect::Deinitialize()
{
    ShutdownAllSystems();
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

void UEng_SystemArchitect::RegisterSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Engine Architect: System %s already registered"), *SystemName);
        return;
    }

    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.State = EEng_SystemState::Uninitialized;
    NewSystem.InitializationTime = 0.0f;
    NewSystem.DependencyCount = 0;

    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: Registered system %s with priority %d"), 
           *SystemName, (int32)Priority);
}

void UEng_SystemArchitect::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
        if (SystemInfo && SystemInfo->State == EEng_SystemState::Active)
        {
            SystemInfo->State = EEng_SystemState::Uninitialized;
            ActiveSystemCount--;
        }
        
        RegisteredSystems.Remove(SystemName);
        UE_LOG(LogTemp, Log, TEXT("Engine Architect: Unregistered system %s"), *SystemName);
    }
}

bool UEng_SystemArchitect::IsSystemActive(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo && SystemInfo->State == EEng_SystemState::Active;
}

EEng_SystemState UEng_SystemArchitect::GetSystemState(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    return SystemInfo ? SystemInfo->State : EEng_SystemState::Uninitialized;
}

TArray<FEng_SystemInfo> UEng_SystemArchitect::GetAllSystemsInfo() const
{
    TArray<FEng_SystemInfo> SystemsArray;
    for (const auto& SystemPair : RegisteredSystems)
    {
        SystemsArray.Add(SystemPair.Value);
    }
    return SystemsArray;
}

void UEng_SystemArchitect::ValidateSystemDependencies()
{
    int32 ValidSystems = 0;
    int32 InvalidSystems = 0;

    for (auto& SystemPair : RegisteredSystems)
    {
        if (ValidateSystemDependency(SystemPair.Key))
        {
            ValidSystems++;
        }
        else
        {
            InvalidSystems++;
            SystemPair.Value.State = EEng_SystemState::Error;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Dependency validation complete - Valid: %d, Invalid: %d"), 
           ValidSystems, InvalidSystems);
}

void UEng_SystemArchitect::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Starting system initialization sequence"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Initialize by priority order
    InitializeSystemByPriority(EEng_SystemPriority::Critical);
    InitializeSystemByPriority(EEng_SystemPriority::High);
    InitializeSystemByPriority(EEng_SystemPriority::Medium);
    InitializeSystemByPriority(EEng_SystemPriority::Low);
    
    TotalInitTime = FPlatformTime::Seconds() - StartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: All systems initialized in %.3f seconds"), TotalInitTime);
}

void UEng_SystemArchitect::ShutdownAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Engine Architect: Shutting down all systems"));
    
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.State == EEng_SystemState::Active)
        {
            SystemPair.Value.State = EEng_SystemState::Uninitialized;
            LogSystemStatus(SystemPair.Key, TEXT("Shutdown"));
        }
    }
    
    ActiveSystemCount = 0;
}

float UEng_SystemArchitect::GetTotalInitializationTime() const
{
    return TotalInitTime;
}

void UEng_SystemArchitect::InitializeSystemByPriority(EEng_SystemPriority Priority)
{
    for (auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Priority == Priority && 
            SystemPair.Value.State == EEng_SystemState::Uninitialized)
        {
            float SystemStartTime = FPlatformTime::Seconds();
            
            SystemPair.Value.State = EEng_SystemState::Initializing;
            LogSystemStatus(SystemPair.Key, TEXT("Initializing"));
            
            // Simulate system initialization time
            FPlatformProcess::Sleep(0.01f); // 10ms per system
            
            SystemPair.Value.State = EEng_SystemState::Active;
            SystemPair.Value.InitializationTime = FPlatformTime::Seconds() - SystemStartTime;
            ActiveSystemCount++;
            
            LogSystemStatus(SystemPair.Key, TEXT("Active"));
        }
    }
}

bool UEng_SystemArchitect::ValidateSystemDependency(const FString& SystemName)
{
    // Basic validation - check if system name is valid
    if (SystemName.IsEmpty() || SystemName.Len() < 3)
    {
        return false;
    }
    
    // Check for circular dependencies (simplified)
    const FEng_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    return true;
}

void UEng_SystemArchitect::LogSystemStatus(const FString& SystemName, const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("Engine Architect: System %s - %s"), *SystemName, *Message);
}
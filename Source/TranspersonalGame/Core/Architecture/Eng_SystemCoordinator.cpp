#include "Eng_SystemCoordinator.h"
#include "Engine/Engine.h"

void UEng_SystemCoordinator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Register core systems in priority order
    RegisterSystem("BiomeManager", 100);
    RegisterSystem("WorldGenerator", 90);
    RegisterSystem("FoliageManager", 80);
    RegisterSystem("DinosaurAI", 70);
    RegisterSystem("CombatSystem", 60);
    RegisterSystem("QuestManager", 50);
    RegisterSystem("AudioSystem", 40);
    RegisterSystem("VFXSystem", 30);
    RegisterSystem("CrowdSimulation", 20);
    RegisterSystem("UIManager", 10);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemCoordinator initialized with %d systems"), RegisteredSystems.Num());
}

void UEng_SystemCoordinator::Deinitialize()
{
    RegisteredSystems.Empty();
    InitializationOrder.Empty();
    
    Super::Deinitialize();
}

void UEng_SystemCoordinator::RegisterSystem(const FString& SystemName, int32 Priority)
{
    FEng_SystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Status = EEng_SystemStatus::Inactive;
    SystemInfo.Priority = Priority;
    SystemInfo.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, SystemInfo);
    
    // Update initialization order
    InitializationOrder.Empty();
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    
    // Sort by priority (higher priority first)
    SystemNames.Sort([this](const FString& A, const FString& B) {
        return RegisteredSystems[A].Priority > RegisteredSystems[B].Priority;
    });
    
    InitializationOrder = SystemNames;
    
    UE_LOG(LogTemp, Log, TEXT("System registered: %s (Priority: %d)"), *SystemName, Priority);
}

void UEng_SystemCoordinator::SetSystemStatus(const FString& SystemName, EEng_SystemStatus Status)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        RegisteredSystems[SystemName].Status = Status;
        UE_LOG(LogTemp, Log, TEXT("System %s status changed to: %d"), *SystemName, (int32)Status);
    }
}

EEng_SystemStatus UEng_SystemCoordinator::GetSystemStatus(const FString& SystemName)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        return RegisteredSystems[SystemName].Status;
    }
    
    return EEng_SystemStatus::Inactive;
}

void UEng_SystemCoordinator::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing all systems in order..."));
    
    for (const FString& SystemName : InitializationOrder)
    {
        if (RegisteredSystems.Contains(SystemName))
        {
            SetSystemStatus(SystemName, EEng_SystemStatus::Initializing);
            
            // Simulate initialization time
            float StartTime = FPlatformTime::Seconds();
            
            // Here would be actual system initialization calls
            // For now, just mark as active
            SetSystemStatus(SystemName, EEng_SystemStatus::Active);
            
            float EndTime = FPlatformTime::Seconds();
            RegisteredSystems[SystemName].InitializationTime = EndTime - StartTime;
            
            UE_LOG(LogTemp, Log, TEXT("System %s initialized in %f seconds"), *SystemName, EndTime - StartTime);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("All systems initialization complete"));
}

TArray<FString> UEng_SystemCoordinator::GetSystemsInOrder()
{
    return InitializationOrder;
}

int32 UEng_SystemCoordinator::GetActiveSystemCount()
{
    int32 ActiveCount = 0;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.Status == EEng_SystemStatus::Active)
        {
            ActiveCount++;
        }
    }
    
    return ActiveCount;
}
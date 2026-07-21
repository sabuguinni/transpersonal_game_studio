#include "Eng_SystemRegistry.h"
#include "Engine/Engine.h"

void UEng_SystemRegistry::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Initializing core system registry"));
    
    // Register core engine systems
    RegisterSystem(TEXT("BiomeSystem"), 100);
    RegisterSystem(TEXT("WorldArchitecture"), 90);
    RegisterSystem(TEXT("PerformanceManager"), 80);
    RegisterSystem(TEXT("ProductionDirector"), 70);
    
    LogSystemStatus();
}

void UEng_SystemRegistry::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Shutting down - %d systems registered"), RegisteredSystems.Num());
    RegisteredSystems.Empty();
    Super::Deinitialize();
}

void UEng_SystemRegistry::RegisterSystem(const FString& SystemName, int32 Priority)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: System %s already registered"), *SystemName);
        return;
    }

    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.Priority = Priority;
    NewSystem.bIsInitialized = false;
    NewSystem.InitializationTime = 0.0f;

    RegisteredSystems.Add(SystemName, NewSystem);
    UE_LOG(LogTemp, Log, TEXT("SystemRegistry: Registered system %s with priority %d"), *SystemName, Priority);
}

void UEng_SystemRegistry::MarkSystemInitialized(const FString& SystemName, float InitTime)
{
    if (FEng_SystemInfo* System = RegisteredSystems.Find(SystemName))
    {
        System->bIsInitialized = true;
        System->InitializationTime = InitTime;
        UE_LOG(LogTemp, Log, TEXT("SystemRegistry: System %s initialized in %.3fs"), *SystemName, InitTime);
    }
}

bool UEng_SystemRegistry::IsSystemRegistered(const FString& SystemName) const
{
    return RegisteredSystems.Contains(SystemName);
}

TArray<FEng_SystemInfo> UEng_SystemRegistry::GetAllSystems() const
{
    TArray<FEng_SystemInfo> Systems;
    for (const auto& Pair : RegisteredSystems)
    {
        Systems.Add(Pair.Value);
    }
    
    // Sort by priority (higher first)
    Systems.Sort([](const FEng_SystemInfo& A, const FEng_SystemInfo& B) {
        return A.Priority > B.Priority;
    });
    
    return Systems;
}

void UEng_SystemRegistry::ValidateSystemDependencies()
{
    int32 InitializedCount = 0;
    int32 TotalCount = RegisteredSystems.Num();
    
    for (const auto& Pair : RegisteredSystems)
    {
        if (Pair.Value.bIsInitialized)
        {
            InitializedCount++;
        }
    }
    
    float InitPercentage = TotalCount > 0 ? (float)InitializedCount / TotalCount * 100.0f : 0.0f;
    UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: %d/%d systems initialized (%.1f%%)"), 
           InitializedCount, TotalCount, InitPercentage);
}

void UEng_SystemRegistry::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("SystemRegistry: Current system status:"));
    
    TArray<FEng_SystemInfo> Systems = GetAllSystems();
    for (const FEng_SystemInfo& System : Systems)
    {
        FString Status = System.bIsInitialized ? 
            FString::Printf(TEXT("READY (%.3fs)"), System.InitializationTime) : 
            TEXT("PENDING");
        
        UE_LOG(LogTemp, Warning, TEXT("  [P%d] %s: %s"), 
               System.Priority, *System.SystemName, *Status);
    }
}
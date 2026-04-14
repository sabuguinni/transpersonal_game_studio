#include "DirectorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"

UDir_DirectorComponent::UDir_DirectorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update metrics every second
    
    bAutoInitializeSystems = true;
    SystemTimeoutSeconds = 30.0f;
}

void UDir_DirectorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Director System: Component initialized"));
    
    // Register core systems in dependency order
    RegisterSystem(TEXT("Core"), TArray<FString>());
    RegisterSystem(TEXT("Physics"), TArray<FString>{TEXT("Core")});
    RegisterSystem(TEXT("WorldGeneration"), TArray<FString>{TEXT("Core"), TEXT("Physics")});
    RegisterSystem(TEXT("Environment"), TArray<FString>{TEXT("WorldGeneration")});
    RegisterSystem(TEXT("Characters"), TArray<FString>{TEXT("Core"), TEXT("Physics")});
    RegisterSystem(TEXT("AI"), TArray<FString>{TEXT("Characters")});
    RegisterSystem(TEXT("Combat"), TArray<FString>{TEXT("AI"), TEXT("Physics")});
    RegisterSystem(TEXT("Audio"), TArray<FString>{TEXT("Core")});
    RegisterSystem(TEXT("VFX"), TArray<FString>{TEXT("Core")});
    
    if (bAutoInitializeSystems)
    {
        // Delay initialization to allow other components to register
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UDir_DirectorComponent::InitializeAllSystems, 2.0f, false);
    }
}

void UDir_DirectorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateMetrics();
}

bool UDir_DirectorComponent::RegisterSystem(const FString& SystemName, const TArray<FString>& Dependencies)
{
    if (RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Director System: System %s already registered"), *SystemName);
        return false;
    }
    
    FDir_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.State = EDir_SystemState::Uninitialized;
    NewSystem.Dependencies = Dependencies;
    NewSystem.InitializationTime = 0.0f;
    
    RegisteredSystems.Add(SystemName, NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Director System: Registered system %s with %d dependencies"), 
           *SystemName, Dependencies.Num());
    
    return true;
}

void UDir_DirectorComponent::InitializeSystem(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Director System: Cannot initialize unknown system %s"), *SystemName);
        return;
    }
    
    FDir_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    if (SystemInfo.State != EDir_SystemState::Uninitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Director System: System %s is not in uninitialized state"), *SystemName);
        return;
    }
    
    if (!AreDependenciesMet(SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("Director System: Dependencies not met for system %s"), *SystemName);
        return;
    }
    
    SetSystemState(SystemName, EDir_SystemState::Initializing);
    
    float StartTime = GetWorld()->GetTimeSeconds();
    
    // Simulate system initialization
    // In a real implementation, this would call the actual system initialization
    UE_LOG(LogTemp, Log, TEXT("Director System: Initializing %s..."), *SystemName);
    
    // For now, mark as ready immediately
    SetSystemState(SystemName, EDir_SystemState::Ready);
    SystemInfo.InitializationTime = GetWorld()->GetTimeSeconds() - StartTime;
    
    UE_LOG(LogTemp, Log, TEXT("Director System: %s initialized in %.3f seconds"), 
           *SystemName, SystemInfo.InitializationTime);
}

void UDir_DirectorComponent::InitializeAllSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("Director System: Starting full system initialization"));
    
    // Initialize systems in dependency order
    TArray<FString> InitializationOrder;
    TArray<FString> ProcessedSystems;
    
    // Simple dependency resolution - in production this would be more sophisticated
    TArray<FString> SystemNames;
    RegisteredSystems.GetKeys(SystemNames);
    
    while (ProcessedSystems.Num() < SystemNames.Num())
    {
        bool ProgressMade = false;
        
        for (const FString& SystemName : SystemNames)
        {
            if (ProcessedSystems.Contains(SystemName))
                continue;
                
            if (AreDependenciesMet(SystemName))
            {
                InitializeSystem(SystemName);
                ProcessedSystems.Add(SystemName);
                InitializationOrder.Add(SystemName);
                ProgressMade = true;
            }
        }
        
        if (!ProgressMade)
        {
            UE_LOG(LogTemp, Error, TEXT("Director System: Circular dependency detected or missing dependencies"));
            break;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Director System: Initialization complete. Order: %s"), 
           *FString::Join(InitializationOrder, TEXT(" -> ")));
}

EDir_SystemState UDir_DirectorComponent::GetSystemState(const FString& SystemName) const
{
    if (const FDir_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        return SystemInfo->State;
    }
    return EDir_SystemState::Error;
}

FDir_SystemMetrics UDir_DirectorComponent::GetSystemMetrics() const
{
    return CurrentMetrics;
}

TArray<FString> UDir_DirectorComponent::GetSystemsInState(EDir_SystemState State) const
{
    TArray<FString> Result;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        if (SystemPair.Value.State == State)
        {
            Result.Add(SystemPair.Key);
        }
    }
    
    return Result;
}

void UDir_DirectorComponent::DebugPrintSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== DIRECTOR SYSTEM STATUS ==="));
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FDir_SystemInfo& Info = SystemPair.Value;
        FString StateString;
        
        switch (Info.State)
        {
            case EDir_SystemState::Uninitialized: StateString = TEXT("UNINITIALIZED"); break;
            case EDir_SystemState::Initializing: StateString = TEXT("INITIALIZING"); break;
            case EDir_SystemState::Ready: StateString = TEXT("READY"); break;
            case EDir_SystemState::Error: StateString = TEXT("ERROR"); break;
            case EDir_SystemState::Shutdown: StateString = TEXT("SHUTDOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (%.3fs) [%s]"), 
               *Info.SystemName, 
               *StateString, 
               Info.InitializationTime,
               *FString::Join(Info.Dependencies, TEXT(", ")));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== METRICS: %d/%d Ready, %d Errors ==="), 
           CurrentMetrics.ReadySystems, 
           CurrentMetrics.TotalSystems, 
           CurrentMetrics.ErrorSystems);
}

void UDir_DirectorComponent::UpdateMetrics()
{
    CurrentMetrics.TotalSystems = RegisteredSystems.Num();
    CurrentMetrics.ReadySystems = 0;
    CurrentMetrics.ErrorSystems = 0;
    CurrentMetrics.TotalInitTime = 0.0f;
    
    for (const auto& SystemPair : RegisteredSystems)
    {
        const FDir_SystemInfo& Info = SystemPair.Value;
        
        switch (Info.State)
        {
            case EDir_SystemState::Ready:
                CurrentMetrics.ReadySystems++;
                CurrentMetrics.TotalInitTime += Info.InitializationTime;
                break;
            case EDir_SystemState::Error:
                CurrentMetrics.ErrorSystems++;
                break;
            default:
                break;
        }
    }
}

bool UDir_DirectorComponent::AreDependenciesMet(const FString& SystemName) const
{
    const FDir_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName);
    if (!SystemInfo)
        return false;
    
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        const FDir_SystemInfo* DepInfo = RegisteredSystems.Find(Dependency);
        if (!DepInfo || DepInfo->State != EDir_SystemState::Ready)
        {
            return false;
        }
    }
    
    return true;
}

void UDir_DirectorComponent::SetSystemState(const FString& SystemName, EDir_SystemState NewState)
{
    if (FDir_SystemInfo* SystemInfo = RegisteredSystems.Find(SystemName))
    {
        SystemInfo->State = NewState;
    }
}

// Director Actor Implementation
ADir_DirectorActor::ADir_DirectorActor()
{
    PrimaryActorTick.bCanEverTick = false;
    
    DirectorComponent = CreateDefaultSubobject<UDir_DirectorComponent>(TEXT("DirectorComponent"));
}

void ADir_DirectorActor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Director Actor: Spawned and ready"));
}
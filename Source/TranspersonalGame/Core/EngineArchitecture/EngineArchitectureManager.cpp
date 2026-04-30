#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    PerformanceUpdateInterval = 1.0f;
    TargetFrameRate = 60.0f;
    MaxMemoryUsageMB = 2048.0f;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initializing..."));
    
    InitializeCoreArchitecture();
    SetupSystemDependencies();
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initialization complete"));
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Shutting down..."));
    
    // Shutdown all systems in reverse order
    for (int32 i = RegisteredSystems.Num() - 1; i >= 0; i--)
    {
        ShutdownSystem(RegisteredSystems[i].SystemName);
    }
    
    // Clear performance timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    Super::Deinitialize();
}

void UEngineArchitectureManager::InitializeCoreArchitecture()
{
    // Register core systems
    FEng_SystemInfo CoreSystem;
    CoreSystem.SystemName = TEXT("CoreSystems");
    CoreSystem.SystemType = EEng_SystemType::Core;
    CoreSystem.Status = EEng_SystemStatus::Initializing;
    CoreSystem.Priority = 100;
    RegisterSystem(CoreSystem);
    
    FEng_SystemInfo WorldGenSystem;
    WorldGenSystem.SystemName = TEXT("WorldGeneration");
    WorldGenSystem.SystemType = EEng_SystemType::WorldGeneration;
    WorldGenSystem.Status = EEng_SystemStatus::Inactive;
    WorldGenSystem.Priority = 90;
    WorldGenSystem.Dependencies.Add(TEXT("CoreSystems"));
    RegisterSystem(WorldGenSystem);
    
    FEng_SystemInfo CharacterSystem;
    CharacterSystem.SystemName = TEXT("CharacterSystems");
    CharacterSystem.SystemType = EEng_SystemType::Character;
    CharacterSystem.Status = EEng_SystemStatus::Inactive;
    CharacterSystem.Priority = 80;
    CharacterSystem.Dependencies.Add(TEXT("CoreSystems"));
    RegisterSystem(CharacterSystem);
    
    FEng_SystemInfo AISystem;
    AISystem.SystemName = TEXT("AISystems");
    AISystem.SystemType = EEng_SystemType::AI;
    AISystem.Status = EEng_SystemStatus::Inactive;
    AISystem.Priority = 70;
    AISystem.Dependencies.Add(TEXT("CoreSystems"));
    AISystem.Dependencies.Add(TEXT("CharacterSystems"));
    RegisterSystem(AISystem);
    
    FEng_SystemInfo PerformanceSystem;
    PerformanceSystem.SystemName = TEXT("PerformanceSystems");
    PerformanceSystem.SystemType = EEng_SystemType::Performance;
    PerformanceSystem.Status = EEng_SystemStatus::Active;
    PerformanceSystem.Priority = 95;
    RegisterSystem(PerformanceSystem);
}

void UEngineArchitectureManager::SetupSystemDependencies()
{
    // Validate all system dependencies
    for (FEng_SystemInfo& System : RegisteredSystems)
    {
        if (!CheckSystemDependencies(System.SystemName))
        {
            UE_LOG(LogTemp, Error, TEXT("System %s has unresolved dependencies"), *System.SystemName);
            System.Status = EEng_SystemStatus::Error;
        }
    }
}

void UEngineArchitectureManager::StartPerformanceMonitoring()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceTimerHandle,
            this,
            &UEngineArchitectureManager::OnPerformanceTimer,
            PerformanceUpdateInterval,
            true
        );
    }
}

void UEngineArchitectureManager::OnPerformanceTimer()
{
    UpdatePerformanceMetrics();
    
    if (!IsPerformanceWithinLimits())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance metrics outside acceptable limits"));
    }
}

void UEngineArchitectureManager::RegisterSystem(const FEng_SystemInfo& SystemInfo)
{
    // Check if system already exists
    if (FindSystemInfo(SystemInfo.SystemName))
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s already registered"), *SystemInfo.SystemName);
        return;
    }
    
    FEng_SystemInfo NewSystem = SystemInfo;
    NewSystem.InitializationTime = FPlatformTime::Seconds();
    RegisteredSystems.Add(NewSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Registered system: %s"), *SystemInfo.SystemName);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    for (int32 i = 0; i < RegisteredSystems.Num(); i++)
    {
        if (RegisteredSystems[i].SystemName == SystemName)
        {
            RegisteredSystems.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Unregistered system: %s"), *SystemName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("System %s not found for unregistration"), *SystemName);
}

bool UEngineArchitectureManager::InitializeSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = FindSystemInfo(SystemName);
    if (!SystemInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize unknown system: %s"), *SystemName);
        return false;
    }
    
    if (!CheckSystemDependencies(SystemName))
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot initialize system %s: dependencies not met"), *SystemName);
        SystemInfo->Status = EEng_SystemStatus::Error;
        return false;
    }
    
    SystemInfo->Status = EEng_SystemStatus::Initializing;
    SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
    
    // Simulate initialization process
    SystemInfo->Status = EEng_SystemStatus::Active;
    
    UE_LOG(LogTemp, Log, TEXT("Initialized system: %s"), *SystemName);
    return true;
}

bool UEngineArchitectureManager::ShutdownSystem(const FString& SystemName)
{
    FEng_SystemInfo* SystemInfo = FindSystemInfo(SystemName);
    if (!SystemInfo)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot shutdown unknown system: %s"), *SystemName);
        return false;
    }
    
    SystemInfo->Status = EEng_SystemStatus::Shutdown;
    SystemInfo->LastUpdateTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("Shutdown system: %s"), *SystemName);
    return true;
}

EEng_SystemStatus UEngineArchitectureManager::GetSystemStatus(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = FindSystemInfo(SystemName);
    return SystemInfo ? SystemInfo->Status : EEng_SystemStatus::Error;
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetAllSystems() const
{
    return RegisteredSystems;
}

TArray<FEng_SystemInfo> UEngineArchitectureManager::GetSystemsByType(EEng_SystemType SystemType) const
{
    TArray<FEng_SystemInfo> FilteredSystems;
    
    for (const FEng_SystemInfo& System : RegisteredSystems)
    {
        if (System.SystemType == SystemType)
        {
            FilteredSystems.Add(System);
        }
    }
    
    return FilteredSystems;
}

FEng_PerformanceMetrics UEngineArchitectureManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    // Update frame rate
    CurrentMetrics.FrameRate = 1.0f / FApp::GetDeltaTime();
    
    // Update memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update actor counts
    if (UWorld* World = GetWorld())
    {
        CurrentMetrics.ActiveActorCount = World->GetActorCount();
        // Visible actor count would need more complex calculation
        CurrentMetrics.VisibleActorCount = CurrentMetrics.ActiveActorCount;
    }
    
    // CPU and GPU usage would need platform-specific implementation
    CurrentMetrics.CPUUsagePercent = 0.0f;
    CurrentMetrics.GPUUsagePercent = 0.0f;
}

bool UEngineArchitectureManager::IsPerformanceWithinLimits() const
{
    return CurrentMetrics.FrameRate >= (TargetFrameRate * 0.8f) &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
}

bool UEngineArchitectureManager::CheckSystemDependencies(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = FindSystemInfo(SystemName);
    if (!SystemInfo)
    {
        return false;
    }
    
    for (const FString& Dependency : SystemInfo->Dependencies)
    {
        const FEng_SystemInfo* DepSystem = FindSystemInfo(Dependency);
        if (!DepSystem || DepSystem->Status != EEng_SystemStatus::Active)
        {
            return false;
        }
    }
    
    return true;
}

TArray<FString> UEngineArchitectureManager::GetSystemDependencies(const FString& SystemName) const
{
    const FEng_SystemInfo* SystemInfo = FindSystemInfo(SystemName);
    return SystemInfo ? SystemInfo->Dependencies : TArray<FString>();
}

void UEngineArchitectureManager::PrintSystemStatus() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM STATUS REPORT ==="));
    
    for (const FEng_SystemInfo& System : RegisteredSystems)
    {
        FString StatusString;
        switch (System.Status)
        {
            case EEng_SystemStatus::Inactive: StatusString = TEXT("INACTIVE"); break;
            case EEng_SystemStatus::Initializing: StatusString = TEXT("INITIALIZING"); break;
            case EEng_SystemStatus::Active: StatusString = TEXT("ACTIVE"); break;
            case EEng_SystemStatus::Error: StatusString = TEXT("ERROR"); break;
            case EEng_SystemStatus::Shutdown: StatusString = TEXT("SHUTDOWN"); break;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s: %s (Priority: %d)"), 
               *System.SystemName, *StatusString, System.Priority);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Performance: %.1f FPS, %.1f MB"), 
           CurrentMetrics.FrameRate, CurrentMetrics.MemoryUsageMB);
}

void UEngineArchitectureManager::ValidateAllSystems() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== SYSTEM VALIDATION ==="));
    
    int32 ActiveSystems = 0;
    int32 ErrorSystems = 0;
    
    for (const FEng_SystemInfo& System : RegisteredSystems)
    {
        if (System.Status == EEng_SystemStatus::Active)
        {
            ActiveSystems++;
        }
        else if (System.Status == EEng_SystemStatus::Error)
        {
            ErrorSystems++;
            UE_LOG(LogTemp, Error, TEXT("System in error state: %s"), *System.SystemName);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Validation complete: %d active, %d errors"), 
           ActiveSystems, ErrorSystems);
}

void UEngineArchitectureManager::EditorValidateArchitecture()
{
    PrintSystemStatus();
    ValidateAllSystems();
}

FEng_SystemInfo* UEngineArchitectureManager::FindSystemInfo(const FString& SystemName)
{
    for (FEng_SystemInfo& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return &System;
        }
    }
    return nullptr;
}

const FEng_SystemInfo* UEngineArchitectureManager::FindSystemInfo(const FString& SystemName) const
{
    for (const FEng_SystemInfo& System : RegisteredSystems)
    {
        if (System.SystemName == SystemName)
        {
            return &System;
        }
    }
    return nullptr;
}

// EngineArchitectureActor Implementation

AEngineArchitectureActor::AEngineArchitectureActor()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;
    
    bAutoInitialize = true;
    bShowDebugInfo = false;
    DebugUpdateInterval = 5.0f;
}

void AEngineArchitectureActor::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoInitialize)
    {
        InitializeArchitecture();
    }
    
    if (bShowDebugInfo)
    {
        GetWorldTimerManager().SetTimer(
            DebugTimerHandle,
            this,
            &AEngineArchitectureActor::OnDebugTimer,
            DebugUpdateInterval,
            true
        );
    }
}

void AEngineArchitectureActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    GetWorldTimerManager().ClearTimer(DebugTimerHandle);
    ShutdownArchitecture();
    
    Super::EndPlay(EndPlayReason);
}

void AEngineArchitectureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

UEngineArchitectureManager* AEngineArchitectureActor::GetArchitectureManager() const
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        return GameInstance->GetSubsystem<UEngineArchitectureManager>();
    }
    return nullptr;
}

void AEngineArchitectureActor::InitializeArchitecture()
{
    UEngineArchitectureManager* ArchManager = GetArchitectureManager();
    if (ArchManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureActor: Initializing architecture systems"));
        ArchManager->EditorValidateArchitecture();
    }
}

void AEngineArchitectureActor::ShutdownArchitecture()
{
    UEngineArchitectureManager* ArchManager = GetArchitectureManager();
    if (ArchManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureActor: Shutting down architecture systems"));
    }
}

void AEngineArchitectureActor::OnDebugTimer()
{
    UEngineArchitectureManager* ArchManager = GetArchitectureManager();
    if (ArchManager)
    {
        ArchManager->PrintSystemStatus();
    }
}
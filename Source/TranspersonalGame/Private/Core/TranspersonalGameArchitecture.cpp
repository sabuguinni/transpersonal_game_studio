// Copyright 2024 Transpersonal Game Studio. All Rights Reserved.

#include "Core/TranspersonalGameArchitecture.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

DEFINE_LOG_CATEGORY_STATIC(LogTranspersonalArchitecture, Log, All);

// Performance tracking globals
static TMap<FString, double> GPerformanceProfiles;

//////////////////////////////////////////////////////////////////////////
// UTranspersonalCoreSubsystem

void UTranspersonalCoreSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    TRANSPERSONAL_LOG(Log, "Initializing Transpersonal Core Subsystem");
    
    // Set default architecture limits
    ArchitectureLimits = FTranspersonalArchitectureLimits();
    
    // Initialize performance tracking
    LastFrameTime = 0.0f;
    FrameCounter = 0;
    
    // Bind to engine tick
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, 
        [this](float DeltaTime) -> bool
        {
            TickAllSystems(DeltaTime);
            return true; // Continue ticking
        }));
    
    TRANSPERSONAL_LOG(Log, "Core Subsystem initialized successfully");
}

void UTranspersonalCoreSubsystem::Deinitialize()
{
    TRANSPERSONAL_LOG(Log, "Shutting down Transpersonal Core Subsystem");
    
    // Shutdown all registered systems
    for (auto& System : RegisteredSystems)
    {
        if (System.GetInterface())
        {
            TRANSPERSONAL_LOG(Log, "Shutting down system: %s", *System.GetInterface()->GetSystemName());
            System.GetInterface()->ShutdownSystem();
        }
    }
    
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

bool UTranspersonalCoreSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game instances, not in editor preview worlds
    if (UGameInstance* GameInstance = Cast<UGameInstance>(Outer))
    {
        return !GameInstance->GetWorld() || !GameInstance->GetWorld()->IsPreviewWorld();
    }
    return false;
}

void UTranspersonalCoreSubsystem::RegisterSystem(TScriptInterface<ITranspersonalSystemInterface> System)
{
    if (!System.GetInterface())
    {
        TRANSPERSONAL_LOG(Error, "Attempted to register null system");
        return;
    }
    
    const FString SystemName = System.GetInterface()->GetSystemName();
    
    // Check if system is already registered
    for (const auto& ExistingSystem : RegisteredSystems)
    {
        if (ExistingSystem.GetInterface() && 
            ExistingSystem.GetInterface()->GetSystemName() == SystemName)
        {
            TRANSPERSONAL_LOG(Warning, "System %s is already registered", *SystemName);
            return;
        }
    }
    
    // Register the system
    RegisteredSystems.Add(System);
    
    // Initialize the system
    System.GetInterface()->InitializeSystem();
    
    // Sort systems by priority
    SortSystemsByPriority();
    
    TRANSPERSONAL_LOG(Log, "Registered system: %s (Priority: %d)", 
        *SystemName, 
        (int32)System.GetInterface()->GetSystemPriority());
}

void UTranspersonalCoreSubsystem::UnregisterSystem(TScriptInterface<ITranspersonalSystemInterface> System)
{
    if (!System.GetInterface())
    {
        return;
    }
    
    const FString SystemName = System.GetInterface()->GetSystemName();
    
    // Find and remove the system
    for (int32 i = RegisteredSystems.Num() - 1; i >= 0; --i)
    {
        if (RegisteredSystems[i].GetInterface() && 
            RegisteredSystems[i].GetInterface()->GetSystemName() == SystemName)
        {
            // Shutdown the system
            RegisteredSystems[i].GetInterface()->ShutdownSystem();
            RegisteredSystems.RemoveAt(i);
            
            TRANSPERSONAL_LOG(Log, "Unregistered system: %s", *SystemName);
            break;
        }
    }
}

TArray<TScriptInterface<ITranspersonalSystemInterface>> UTranspersonalCoreSubsystem::GetAllSystems() const
{
    return RegisteredSystems;
}

float UTranspersonalCoreSubsystem::GetTotalCPUUsage() const
{
    float TotalCPU = 0.0f;
    
    for (const auto& System : RegisteredSystems)
    {
        if (System.GetInterface())
        {
            TotalCPU += System.GetInterface()->GetSystemCPUUsage();
        }
    }
    
    return TotalCPU;
}

int32 UTranspersonalCoreSubsystem::GetTotalMemoryUsage() const
{
    int32 TotalMemory = 0;
    
    for (const auto& System : RegisteredSystems)
    {
        if (System.GetInterface())
        {
            TotalMemory += System.GetInterface()->GetSystemMemoryUsage();
        }
    }
    
    return TotalMemory;
}

bool UTranspersonalCoreSubsystem::AreAllSystemsHealthy() const
{
    for (const auto& System : RegisteredSystems)
    {
        if (System.GetInterface() && !System.GetInterface()->IsSystemHealthy())
        {
            return false;
        }
    }
    
    return true;
}

void UTranspersonalCoreSubsystem::SetArchitectureLimits(const FTranspersonalArchitectureLimits& NewLimits)
{
    ArchitectureLimits = NewLimits;
    
    TRANSPERSONAL_LOG(Log, "Updated architecture limits - Max Actors: %d, Max Mass Agents: %d, Target FPS: %.1f",
        ArchitectureLimits.MaxSimultaneousActors,
        ArchitectureLimits.MaxMassAgents,
        ArchitectureLimits.TargetFrameRate);
}

void UTranspersonalCoreSubsystem::TickAllSystems(float DeltaTime)
{
    SCOPE_CYCLE_COUNTER(STAT_TranspersonalCoreTick);
    
    LastFrameTime = DeltaTime;
    FrameCounter++;
    
    // Validate performance every 60 frames (roughly once per second at 60fps)
    if (FrameCounter % 60 == 0)
    {
        ValidateSystemHealth();
    }
    
    // Tick all systems in priority order
    for (const auto& System : RegisteredSystems)
    {
        if (System.GetInterface())
        {
            SCOPE_CYCLE_COUNTER(STAT_TranspersonalSystemTick);
            
            const double StartTime = FPlatformTime::Seconds();
            System.GetInterface()->TickSystem(DeltaTime);
            const double EndTime = FPlatformTime::Seconds();
            
            // Log performance warnings for slow systems
            const float SystemTime = (EndTime - StartTime) * 1000.0f; // Convert to ms
            if (SystemTime > 5.0f) // Warn if system takes more than 5ms
            {
                TRANSPERSONAL_LOG(Warning, "System %s took %.2f ms to tick", 
                    *System.GetInterface()->GetSystemName(), SystemTime);
            }
        }
    }
}

void UTranspersonalCoreSubsystem::SortSystemsByPriority()
{
    RegisteredSystems.Sort([](const TScriptInterface<ITranspersonalSystemInterface>& A, 
                              const TScriptInterface<ITranspersonalSystemInterface>& B)
    {
        if (!A.GetInterface() || !B.GetInterface())
        {
            return false;
        }
        
        return A.GetInterface()->GetSystemPriority() < B.GetInterface()->GetSystemPriority();
    });
}

void UTranspersonalCoreSubsystem::ValidateSystemHealth()
{
    // Check frame rate
    const float CurrentFPS = 1.0f / LastFrameTime;
    if (CurrentFPS < ArchitectureLimits.MinFrameRate)
    {
        TRANSPERSONAL_LOG(Warning, "Frame rate below minimum: %.1f fps (target: %.1f fps)",
            CurrentFPS, ArchitectureLimits.TargetFrameRate);
    }
    
    // Check memory usage
    const int32 TotalMemory = GetTotalMemoryUsage();
    const int32 MaxMemory = ArchitectureLimits.MaxTextureMemory + 
                           ArchitectureLimits.MaxMeshMemory + 
                           ArchitectureLimits.MaxAudioMemory;
    
    if (TotalMemory > MaxMemory)
    {
        TRANSPERSONAL_LOG(Warning, "Memory usage exceeds limits: %d MB (max: %d MB)",
            TotalMemory, MaxMemory);
    }
    
    // Check system health
    if (!AreAllSystemsHealthy())
    {
        TRANSPERSONAL_LOG(Error, "One or more systems are reporting unhealthy status");
    }
}

//////////////////////////////////////////////////////////////////////////
// UTranspersonalArchitectureValidator

bool UTranspersonalArchitectureValidator::ValidateWorldSize(float WorldSize)
{
    const UTranspersonalCoreSubsystem* CoreSubsystem = 
        GEngine->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    
    if (!CoreSubsystem)
    {
        return false;
    }
    
    const float MaxWorldSize = CoreSubsystem->GetArchitectureLimits().MaxWorldSize;
    return WorldSize <= MaxWorldSize;
}

bool UTranspersonalArchitectureValidator::ValidateActorCount(int32 ActorCount)
{
    const UTranspersonalCoreSubsystem* CoreSubsystem = 
        GEngine->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    
    if (!CoreSubsystem)
    {
        return false;
    }
    
    const int32 MaxActors = CoreSubsystem->GetArchitectureLimits().MaxSimultaneousActors;
    return ActorCount <= MaxActors;
}

bool UTranspersonalArchitectureValidator::ValidateMemoryUsage(int32 MemoryUsageMB)
{
    const UTranspersonalCoreSubsystem* CoreSubsystem = 
        GEngine->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    
    if (!CoreSubsystem)
    {
        return false;
    }
    
    const FTranspersonalArchitectureLimits& Limits = CoreSubsystem->GetArchitectureLimits();
    const int32 MaxMemory = Limits.MaxTextureMemory + Limits.MaxMeshMemory + Limits.MaxAudioMemory;
    
    return MemoryUsageMB <= MaxMemory;
}

bool UTranspersonalArchitectureValidator::ValidateFrameRate(float CurrentFPS)
{
    const UTranspersonalCoreSubsystem* CoreSubsystem = 
        GEngine->GetGameInstance()->GetSubsystem<UTranspersonalCoreSubsystem>();
    
    if (!CoreSubsystem)
    {
        return false;
    }
    
    const float MinFPS = CoreSubsystem->GetArchitectureLimits().MinFrameRate;
    return CurrentFPS >= MinFPS;
}

bool UTranspersonalArchitectureValidator::CheckSystemRequirements()
{
    // Check UE5 version
    const FString EngineVersion = FEngineVersion::Current().ToString();
    TRANSPERSONAL_LOG(Log, "Engine Version: %s", *EngineVersion);
    
    // Check platform capabilities
    const FString PlatformName = FPlatformProperties::PlatformName();
    TRANSPERSONAL_LOG(Log, "Platform: %s", *PlatformName);
    
    // Check memory
    const FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    TRANSPERSONAL_LOG(Log, "Available Physical Memory: %.2f GB", 
        MemStats.AvailablePhysical / (1024.0f * 1024.0f * 1024.0f));
    
    // Check CPU cores
    const int32 NumCores = FPlatformMisc::NumberOfCores();
    TRANSPERSONAL_LOG(Log, "CPU Cores: %d", NumCores);
    
    // Minimum requirements check
    const bool HasEnoughMemory = MemStats.AvailablePhysical > (8ULL * 1024 * 1024 * 1024); // 8GB
    const bool HasEnoughCores = NumCores >= 4;
    
    if (!HasEnoughMemory)
    {
        TRANSPERSONAL_LOG(Error, "Insufficient memory for Transpersonal Game (minimum 8GB required)");
    }
    
    if (!HasEnoughCores)
    {
        TRANSPERSONAL_LOG(Error, "Insufficient CPU cores for Transpersonal Game (minimum 4 cores required)");
    }
    
    return HasEnoughMemory && HasEnoughCores;
}

void UTranspersonalArchitectureValidator::StartPerformanceProfile(const FString& ProfileName)
{
    const double CurrentTime = FPlatformTime::Seconds();
    GPerformanceProfiles.Add(ProfileName, CurrentTime);
    
    TRANSPERSONAL_LOG(Log, "Started performance profile: %s", *ProfileName);
}

void UTranspersonalArchitectureValidator::EndPerformanceProfile(const FString& ProfileName)
{
    const double CurrentTime = FPlatformTime::Seconds();
    
    if (const double* StartTime = GPerformanceProfiles.Find(ProfileName))
    {
        const double Duration = (CurrentTime - *StartTime) * 1000.0; // Convert to ms
        TRANSPERSONAL_LOG(Log, "Performance profile %s: %.2f ms", *ProfileName, Duration);
        
        GPerformanceProfiles.Remove(ProfileName);
    }
    else
    {
        TRANSPERSONAL_LOG(Warning, "Performance profile %s was not started", *ProfileName);
    }
}

// Stats definitions
DEFINE_STAT(STAT_TranspersonalCoreTick);
DEFINE_STAT(STAT_TranspersonalSystemTick);
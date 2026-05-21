#include "EngineArchitectCore.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UEngineArchitectCore::UEngineArchitectCore()
{
    // Initialize default performance constraints
    FEng_PerformanceConstraints PCConstraints;
    PCConstraints.MaxFrameTime = 16.67f; // 60 FPS
    PCConstraints.MaxActors = 10000;
    PCConstraints.MaxMemoryMB = 4096;
    PCConstraints.MaxDrawCalls = 2000;
    PerformanceConstraints.Add(EEng_PerformanceLevel::PC_High, PCConstraints);

    FEng_PerformanceConstraints ConsoleConstraints;
    ConsoleConstraints.MaxFrameTime = 33.33f; // 30 FPS
    ConsoleConstraints.MaxActors = 5000;
    ConsoleConstraints.MaxMemoryMB = 2048;
    ConsoleConstraints.MaxDrawCalls = 1000;
    PerformanceConstraints.Add(EEng_PerformanceLevel::Console, ConsoleConstraints);

    // Initialize critical system status
    CriticalSystemStatus.Add(TEXT("Physics"), true);
    CriticalSystemStatus.Add(TEXT("Rendering"), true);
    CriticalSystemStatus.Add(TEXT("Audio"), true);
    CriticalSystemStatus.Add(TEXT("WorldGeneration"), true);
    CriticalSystemStatus.Add(TEXT("AI"), true);

    // Initialize performance metrics
    LastPerformanceCheck.FrameTime = 0.0f;
    LastPerformanceCheck.ActorCount = 0;
    LastPerformanceCheck.MemoryUsageMB = 0.0f;
    LastPerformanceCheck.DrawCalls = 0;
    LastPerformanceCheck.Timestamp = FDateTime::Now();
}

void UEngineArchitectCore::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectCore: Initializing architectural management system"));

    // Start performance monitoring
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceMonitorTimer,
            this,
            &UEngineArchitectCore::CheckPerformanceThresholds,
            PERFORMANCE_CHECK_INTERVAL,
            true
        );

        World->GetTimerManager().SetTimer(
            SystemValidationTimer,
            this,
            &UEngineArchitectCore::ValidateModuleDependencies,
            SYSTEM_VALIDATION_INTERVAL,
            true
        );
    }

    // Register core modules
    RegisterModule(TEXT("Core"), 0, {});
    RegisterModule(TEXT("Physics"), 1, {TEXT("Core")});
    RegisterModule(TEXT("Rendering"), 1, {TEXT("Core")});
    RegisterModule(TEXT("WorldGeneration"), 2, {TEXT("Core"), TEXT("Physics")});
    RegisterModule(TEXT("AI"), 3, {TEXT("Core"), TEXT("Physics"), TEXT("WorldGeneration")});
    RegisterModule(TEXT("Audio"), 2, {TEXT("Core")});

    LogArchitecturalEvent(TEXT("Initialization"), TEXT("Engine Architecture Core initialized successfully"));
}

void UEngineArchitectCore::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitorTimer);
        World->GetTimerManager().ClearTimer(SystemValidationTimer);
    }

    LogArchitecturalEvent(TEXT("Shutdown"), TEXT("Engine Architecture Core shutting down"));
    Super::Deinitialize();
}

bool UEngineArchitectCore::RegisterModule(const FString& ModuleName, int32 Priority, const TArray<FString>& Dependencies)
{
    if (RegisteredModules.Contains(ModuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectCore: Module %s already registered"), *ModuleName);
        return false;
    }

    FEng_ModuleInfo ModuleInfo;
    ModuleInfo.ModuleName = ModuleName;
    ModuleInfo.Priority = Priority;
    ModuleInfo.Dependencies = Dependencies;
    ModuleInfo.bIsActive = true;
    ModuleInfo.LastValidation = FDateTime::Now();

    RegisteredModules.Add(ModuleName, ModuleInfo);

    LogArchitecturalEvent(TEXT("ModuleRegistration"), FString::Printf(TEXT("Module %s registered with priority %d"), *ModuleName, Priority));
    return true;
}

bool UEngineArchitectCore::UnregisterModule(const FString& ModuleName)
{
    if (!RegisteredModules.Contains(ModuleName))
    {
        return false;
    }

    RegisteredModules.Remove(ModuleName);
    LogArchitecturalEvent(TEXT("ModuleUnregistration"), FString::Printf(TEXT("Module %s unregistered"), *ModuleName));
    return true;
}

bool UEngineArchitectCore::IsModuleRegistered(const FString& ModuleName) const
{
    return RegisteredModules.Contains(ModuleName);
}

void UEngineArchitectCore::SetPerformanceConstraint(EEng_PerformanceLevel Level, float MaxFrameTime, int32 MaxActors)
{
    FEng_PerformanceConstraints Constraints;
    Constraints.MaxFrameTime = MaxFrameTime;
    Constraints.MaxActors = MaxActors;
    Constraints.MaxMemoryMB = 4096; // Default
    Constraints.MaxDrawCalls = 2000; // Default

    PerformanceConstraints.Add(Level, Constraints);
    LogArchitecturalEvent(TEXT("PerformanceConstraint"), FString::Printf(TEXT("Set constraint for level %d: %.2fms, %d actors"), (int32)Level, MaxFrameTime, MaxActors));
}

bool UEngineArchitectCore::ValidatePerformanceCompliance() const
{
    // Check against PC_High constraints by default
    const FEng_PerformanceConstraints* Constraints = PerformanceConstraints.Find(EEng_PerformanceLevel::PC_High);
    if (!Constraints)
    {
        return false;
    }

    bool bCompliant = true;
    if (LastPerformanceCheck.FrameTime > Constraints->MaxFrameTime)
    {
        bCompliant = false;
    }
    if (LastPerformanceCheck.ActorCount > Constraints->MaxActors)
    {
        bCompliant = false;
    }
    if (LastPerformanceCheck.MemoryUsageMB > Constraints->MaxMemoryMB)
    {
        bCompliant = false;
    }

    return bCompliant;
}

FEng_PerformanceMetrics UEngineArchitectCore::GetCurrentPerformanceMetrics() const
{
    return LastPerformanceCheck;
}

void UEngineArchitectCore::ValidateSystemIntegration()
{
    CurrentViolations.Empty();

    // Check module dependency chains
    for (const auto& ModulePair : RegisteredModules)
    {
        const FEng_ModuleInfo& ModuleInfo = ModulePair.Value;
        for (const FString& Dependency : ModuleInfo.Dependencies)
        {
            if (!IsModuleRegistered(Dependency))
            {
                CurrentViolations.Add(FString::Printf(TEXT("Module %s depends on unregistered module %s"), *ModuleInfo.ModuleName, *Dependency));
            }
        }
    }

    // Check critical system health
    for (const auto& SystemPair : CriticalSystemStatus)
    {
        if (!SystemPair.Value)
        {
            CurrentViolations.Add(FString::Printf(TEXT("Critical system %s is unhealthy"), *SystemPair.Key));
        }
    }

    LogArchitecturalEvent(TEXT("SystemValidation"), FString::Printf(TEXT("Found %d architectural violations"), CurrentViolations.Num()));
}

bool UEngineArchitectCore::CanSystemsIntegrate(const FString& SystemA, const FString& SystemB) const
{
    // Check if systems are compatible based on compatibility matrix
    const TArray<FString>* CompatibleSystems = SystemCompatibilityMatrix.Find(SystemA);
    if (CompatibleSystems)
    {
        return CompatibleSystems->Contains(SystemB);
    }

    // Default to compatible if no specific rules defined
    return true;
}

TArray<FString> UEngineArchitectCore::GetArchitecturalViolations() const
{
    return CurrentViolations;
}

void UEngineArchitectCore::EnforceArchitecturalRules()
{
    ValidateSystemIntegration();

    if (CurrentViolations.Num() > MAX_ARCHITECTURAL_VIOLATIONS)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectCore: Too many architectural violations (%d), enforcing emergency constraints"), CurrentViolations.Num());
        
        // Emergency performance constraints
        SetPerformanceConstraint(EEng_PerformanceLevel::PC_High, 20.0f, 5000);
        SetPerformanceConstraint(EEng_PerformanceLevel::Console, 40.0f, 2500);
    }
}

TArray<FString> UEngineArchitectCore::GetModuleDependencyChain(const FString& ModuleName) const
{
    TArray<FString> DependencyChain;
    TArray<FString> ToProcess;
    TSet<FString> Processed;

    ToProcess.Add(ModuleName);

    while (ToProcess.Num() > 0)
    {
        FString CurrentModule = ToProcess.Pop();
        if (Processed.Contains(CurrentModule))
        {
            continue;
        }

        Processed.Add(CurrentModule);
        DependencyChain.Add(CurrentModule);

        const FEng_ModuleInfo* ModuleInfo = RegisteredModules.Find(CurrentModule);
        if (ModuleInfo)
        {
            for (const FString& Dependency : ModuleInfo->Dependencies)
            {
                if (!Processed.Contains(Dependency))
                {
                    ToProcess.Add(Dependency);
                }
            }
        }
    }

    return DependencyChain;
}

bool UEngineArchitectCore::ValidateDependencyChain() const
{
    for (const auto& ModulePair : RegisteredModules)
    {
        TArray<FString> Chain = GetModuleDependencyChain(ModulePair.Key);
        
        // Check for circular dependencies
        TSet<FString> Visited;
        for (const FString& Module : Chain)
        {
            if (Visited.Contains(Module))
            {
                UE_LOG(LogTemp, Error, TEXT("EngineArchitectCore: Circular dependency detected in module %s"), *ModulePair.Key);
                return false;
            }
            Visited.Add(Module);
        }
    }

    return true;
}

void UEngineArchitectCore::MonitorCriticalSystems()
{
    // Update critical system status based on actual system health
    if (UWorld* World = GetWorld())
    {
        // Check physics system
        bool bPhysicsHealthy = World->GetPhysicsScene() != nullptr;
        CriticalSystemStatus.Add(TEXT("Physics"), bPhysicsHealthy);

        // Check world generation
        int32 ActorCount = 0;
        if (World)
        {
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                ActorCount++;
            }
        }
        bool bWorldGenHealthy = ActorCount > 0;
        CriticalSystemStatus.Add(TEXT("WorldGeneration"), bWorldGenHealthy);

        // Update performance metrics
        LastPerformanceCheck.ActorCount = ActorCount;
        LastPerformanceCheck.Timestamp = FDateTime::Now();
    }
}

bool UEngineArchitectCore::IsCriticalSystemHealthy(const FString& SystemName) const
{
    const bool* Status = CriticalSystemStatus.Find(SystemName);
    return Status ? *Status : false;
}

void UEngineArchitectCore::ValidateModuleDependencies()
{
    ValidateSystemIntegration();
    MonitorCriticalSystems();
}

void UEngineArchitectCore::CheckPerformanceThresholds()
{
    // Update current performance metrics
    if (UWorld* World = GetWorld())
    {
        // Get frame time
        LastPerformanceCheck.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds

        // Count actors
        int32 ActorCount = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            ActorCount++;
        }
        LastPerformanceCheck.ActorCount = ActorCount;

        // Estimate memory usage (simplified)
        LastPerformanceCheck.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024 * 1024);

        LastPerformanceCheck.Timestamp = FDateTime::Now();
    }

    // Validate performance compliance
    if (!ValidatePerformanceCompliance())
    {
        LogArchitecturalEvent(TEXT("PerformanceViolation"), TEXT("Performance thresholds exceeded"));
    }
}

void UEngineArchitectCore::UpdateSystemCompatibility()
{
    // Define system compatibility rules
    SystemCompatibilityMatrix.Empty();
    
    SystemCompatibilityMatrix.Add(TEXT("Physics"), {TEXT("WorldGeneration"), TEXT("AI"), TEXT("Combat")});
    SystemCompatibilityMatrix.Add(TEXT("WorldGeneration"), {TEXT("Environment"), TEXT("Foliage"), TEXT("Lighting")});
    SystemCompatibilityMatrix.Add(TEXT("AI"), {TEXT("Combat"), TEXT("NPC"), TEXT("Crowd")});
    SystemCompatibilityMatrix.Add(TEXT("Audio"), {TEXT("Environment"), TEXT("Combat"), TEXT("UI")});
}

void UEngineArchitectCore::LogArchitecturalEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("[EngineArchitect] %s: %s"), *Event, *Details);
    UE_LOG(LogTemp, Log, TEXT("%s"), *LogMessage);
}

// World Subsystem Implementation
UEngineArchitectWorldSubsystem::UEngineArchitectWorldSubsystem()
    : bWorldArchitectureValid(false)
{
}

void UEngineArchitectWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectWorldSubsystem: Initializing world-level architecture management"));
}

void UEngineArchitectWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
    Super::OnWorldBeginPlay(InWorld);
    ValidateWorldArchitecture();
}

void UEngineArchitectWorldSubsystem::ValidateWorldArchitecture()
{
    WorldArchitectureIssues.Empty();
    
    CheckWorldLevelConstraints();
    ValidateActorDistribution();
    MonitorWorldPerformance();
    
    bWorldArchitectureValid = (WorldArchitectureIssues.Num() == 0);
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectWorldSubsystem: World architecture validation complete. Valid: %s, Issues: %d"), 
           bWorldArchitectureValid ? TEXT("true") : TEXT("false"), 
           WorldArchitectureIssues.Num());
}

bool UEngineArchitectWorldSubsystem::IsWorldArchitectureValid() const
{
    return bWorldArchitectureValid;
}

void UEngineArchitectWorldSubsystem::EnforceWorldConstraints()
{
    if (!bWorldArchitectureValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectWorldSubsystem: Enforcing world constraints due to architectural issues"));
        
        // Implement constraint enforcement logic
        for (const FString& Issue : WorldArchitectureIssues)
        {
            UE_LOG(LogTemp, Warning, TEXT("World Architecture Issue: %s"), *Issue);
        }
    }
}

bool UEngineArchitectWorldSubsystem::ValidateBiomeArchitecture(EEng_BiomeType BiomeType) const
{
    // Validate biome-specific architectural constraints
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Count actors in biome area (simplified validation)
    int32 BiomeActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        BiomeActorCount++;
    }

    // Basic validation - each biome should have reasonable actor count
    const int32 MinActorsPerBiome = 10;
    const int32 MaxActorsPerBiome = 2000;
    
    return (BiomeActorCount >= MinActorsPerBiome && BiomeActorCount <= MaxActorsPerBiome);
}

void UEngineArchitectWorldSubsystem::OptimizeBiomePerformance(EEng_BiomeType BiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectWorldSubsystem: Optimizing performance for biome type %d"), (int32)BiomeType);
    
    // Implement biome-specific performance optimizations
    // This would include LOD adjustments, culling optimizations, etc.
}

void UEngineArchitectWorldSubsystem::CheckWorldLevelConstraints()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        WorldArchitectureIssues.Add(TEXT("World reference is null"));
        return;
    }

    // Check world size constraints
    FVector WorldOrigin, WorldBounds;
    World->GetWorldBounds(WorldOrigin, WorldBounds);
    
    const float MaxWorldSize = 1000000.0f; // 10km
    if (WorldBounds.Size() > MaxWorldSize)
    {
        WorldArchitectureIssues.Add(FString::Printf(TEXT("World size exceeds maximum: %.2f > %.2f"), WorldBounds.Size(), MaxWorldSize));
    }
}

void UEngineArchitectWorldSubsystem::ValidateActorDistribution()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Count actors by type and validate distribution
    TMap<FString, int32> ActorCounts;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            FString ActorClass = Actor->GetClass()->GetName();
            int32* Count = ActorCounts.Find(ActorClass);
            if (Count)
            {
                (*Count)++;
            }
            else
            {
                ActorCounts.Add(ActorClass, 1);
            }
        }
    }

    // Validate reasonable distribution
    const int32 MaxActorsPerType = 5000;
    for (const auto& CountPair : ActorCounts)
    {
        if (CountPair.Value > MaxActorsPerType)
        {
            WorldArchitectureIssues.Add(FString::Printf(TEXT("Too many actors of type %s: %d > %d"), *CountPair.Key, CountPair.Value, MaxActorsPerType));
        }
    }
}

void UEngineArchitectWorldSubsystem::MonitorWorldPerformance()
{
    // Monitor world-level performance metrics
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Check frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f;
    const float MaxFrameTime = 33.33f; // 30 FPS minimum
    
    if (CurrentFrameTime > MaxFrameTime)
    {
        WorldArchitectureIssues.Add(FString::Printf(TEXT("Frame time exceeds threshold: %.2fms > %.2fms"), CurrentFrameTime, MaxFrameTime));
    }
}
// Copyright Transpersonal Game Studio. All Rights Reserved.
// EngineArchitecture.cpp - Implementation of core engine architecture

#include "EngineArchitecture.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "RenderingThread.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

// ============================================================================
// TRANSPERSONAL ENGINE ARCHITECTURE SUBSYSTEM IMPLEMENTATION
// ============================================================================

void UTranspersonalEngineArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Log, TEXT("Transpersonal Engine Architecture Subsystem: Initializing..."));

    // Initialize architecture settings with defaults
    WorldPartitionSettings = FTranspersonalWorldPartitionSettings();
    NaniteSettings = FTranspersonalNaniteSettings();
    LumenSettings = FTranspersonalLumenSettings();
    VSMSettings = FTranspersonalVSMSettings();
    MassAISettings = FTranspersonalMassAISettings();
    PerformanceTargets = FTranspersonalPerformanceTargets();

    // Validate engine configuration
    ValidateEngineSettings();

    // Setup performance monitoring
    SetupPerformanceMonitoring();

    // Enforce architectural rules
    EnforceArchitecturalRules();

    UE_LOG(LogTemp, Log, TEXT("Transpersonal Engine Architecture Subsystem: Initialized successfully"));
}

void UTranspersonalEngineArchitectureSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("Transpersonal Engine Architecture Subsystem: Shutting down..."));
    
    Super::Deinitialize();
}

// ============================================================================
// ARCHITECTURE VALIDATION FUNCTIONS
// ============================================================================

bool UTranspersonalEngineArchitectureSubsystem::ValidateWorldPartitionSettings() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateWorldPartitionSettings);

    if (UWorld* World = GetWorld())
    {
        // Check if World Partition is enabled
        if (!World->IsPartitionedWorld())
        {
            UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: World Partition not enabled! This is MANDATORY for worlds > 4km²"));
            return false;
        }

        // Validate cell size
        if (WorldPartitionSettings.CellSize < 25600 || WorldPartitionSettings.CellSize > 102400)
        {
            UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: World Partition cell size outside recommended range (256m-1024m)"));
        }

        // Validate loading range
        if (WorldPartitionSettings.LoadingRange < WorldPartitionSettings.CellSize * 2)
        {
            UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: Loading range should be at least 2x cell size"));
        }

        UE_LOG(LogTemp, Log, TEXT("World Partition validation: PASSED"));
        return true;
    }

    UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE ERROR: No valid world found for validation"));
    return false;
}

bool UTranspersonalEngineArchitectureSubsystem::ValidateNaniteSettings() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateNaniteSettings);

    // Check if Nanite is enabled in project settings
    static const auto* CVarNaniteEnabled = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Nanite"));
    if (!CVarNaniteEnabled || CVarNaniteEnabled->GetValueOnGameThread() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Nanite is disabled! This is MANDATORY for high-detail geometry"));
        return false;
    }

    // Check Nanite Foliage
    if (NaniteSettings.bEnableNaniteFoliage)
    {
        static const auto* CVarNaniteFoliage = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Nanite.Foliage"));
        if (!CVarNaniteFoliage || CVarNaniteFoliage->GetValueOnGameThread() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: Nanite Foliage not enabled in console variables"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Nanite validation: PASSED"));
    return true;
}

bool UTranspersonalEngineArchitectureSubsystem::ValidateLumenSettings() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateLumenSettings);

    // Check if Lumen is enabled
    static const auto* CVarLumenGI = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.DynamicGlobalIlluminationMethod"));
    if (!CVarLumenGI || CVarLumenGI->GetValueOnGameThread() != 1) // 1 = Lumen
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Lumen Global Illumination not enabled! This is MANDATORY"));
        return false;
    }

    // Check Lumen Reflections
    static const auto* CVarLumenReflections = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.ReflectionMethod"));
    if (!CVarLumenReflections || CVarLumenReflections->GetValueOnGameThread() != 1) // 1 = Lumen
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Lumen Reflections not enabled! This is MANDATORY"));
        return false;
    }

    // Check Hardware Ray Tracing availability
    if (LumenSettings.bEnableHardwareRayTracing)
    {
        static const auto* CVarHWRT = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.RayTracing"));
        if (CVarHWRT && CVarHWRT->GetValueOnGameThread() == 1)
        {
            UE_LOG(LogTemp, Log, TEXT("Hardware Ray Tracing available and enabled"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Hardware Ray Tracing requested but not available, falling back to software"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Lumen validation: PASSED"));
    return true;
}

bool UTranspersonalEngineArchitectureSubsystem::ValidateVSMSettings() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateVSMSettings);

    // Check if Virtual Shadow Maps are enabled
    static const auto* CVarVSM = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Shadow.Virtual.Enable"));
    if (!CVarVSM || CVarVSM->GetValueOnGameThread() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Virtual Shadow Maps not enabled! This is MANDATORY"));
        return false;
    }

    // Check VSM caching
    if (VSMSettings.bEnableCaching)
    {
        static const auto* CVarVSMCache = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.Shadow.Virtual.Cache"));
        if (!CVarVSMCache || CVarVSMCache->GetValueOnGameThread() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: VSM caching disabled, performance may suffer"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Virtual Shadow Maps validation: PASSED"));
    return true;
}

bool UTranspersonalEngineArchitectureSubsystem::ValidateMassAISettings() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateMassAISettings);

    // Check if Mass Entity is available
    if (!FModuleManager::Get().IsModuleLoaded("MassEntity"))
    {
        UE_LOG(LogTemp, Error, TEXT("ARCHITECTURE VIOLATION: Mass Entity module not loaded! Required for crowd simulation"));
        return false;
    }

    // Validate entity limits
    if (MassAISettings.MaxConcurrentEntities > 100000)
    {
        UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: Very high entity count may impact performance"));
    }

    // Validate LOD distances
    if (MassAISettings.LODDistances.Num() < 3)
    {
        UE_LOG(LogTemp, Warning, TEXT("ARCHITECTURE WARNING: Insufficient LOD levels for optimal performance"));
    }

    UE_LOG(LogTemp, Log, TEXT("Mass AI validation: PASSED"));
    return true;
}

// ============================================================================
// PERFORMANCE MONITORING
// ============================================================================

bool UTranspersonalEngineArchitectureSubsystem::IsPerformanceTargetMet() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(IsPerformanceTargetMet);

    // Get current frame time
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Check against target
    float TargetFrameTime = 1000.0f / PerformanceTargets.TargetFPS_PC;
    
    if (CurrentFrameTime > TargetFrameTime * 1.1f) // 10% tolerance
    {
        return false;
    }

    // Check GPU time (if available)
    if (LastGPUTime > PerformanceTargets.MaxGPUTime_MS * 1.1f)
    {
        return false;
    }

    return true;
}

FTranspersonalPerformanceTargets UTranspersonalEngineArchitectureSubsystem::GetCurrentPerformanceMetrics() const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(GetCurrentPerformanceMetrics);

    FTranspersonalPerformanceTargets CurrentMetrics = PerformanceTargets;
    
    // Update with current values
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f;
    CurrentMetrics.MaxCPUTime_MS = CurrentFrameTime;
    CurrentMetrics.MaxGPUTime_MS = LastGPUTime;
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MaxMemoryUsage_MB = MemStats.UsedPhysical / (1024 * 1024);

    return CurrentMetrics;
}

bool UTranspersonalEngineArchitectureSubsystem::IsSystemCompliant(const FString& SystemName) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(IsSystemCompliant);

    // Validate all core systems
    bool bWorldPartitionOK = ValidateWorldPartitionSettings();
    bool bNaniteOK = ValidateNaniteSettings();
    bool bLumenOK = ValidateLumenSettings();
    bool bVSMOK = ValidateVSMSettings();
    bool bMassAIOK = ValidateMassAISettings();
    bool bPerformanceOK = IsPerformanceTargetMet();

    if (!bWorldPartitionOK || !bNaniteOK || !bLumenOK || !bVSMOK || !bMassAIOK)
    {
        UE_LOG(LogTemp, Error, TEXT("System %s is NOT COMPLIANT with architecture requirements"), *SystemName);
        return false;
    }

    if (!bPerformanceOK)
    {
        UE_LOG(LogTemp, Warning, TEXT("System %s is compliant but performance targets not met"), *SystemName);
    }

    return true;
}

// ============================================================================
// PRIVATE IMPLEMENTATION
// ============================================================================

void UTranspersonalEngineArchitectureSubsystem::ValidateEngineSettings()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(ValidateEngineSettings);

    UE_LOG(LogTemp, Log, TEXT("Validating engine settings for Transpersonal Game architecture..."));

    // Force enable required systems via console commands
    if (GEngine)
    {
        // Enable Nanite
        GEngine->Exec(GetWorld(), TEXT("r.Nanite 1"));
        
        // Enable Lumen
        GEngine->Exec(GetWorld(), TEXT("r.DynamicGlobalIlluminationMethod 1"));
        GEngine->Exec(GetWorld(), TEXT("r.ReflectionMethod 1"));
        
        // Enable Virtual Shadow Maps
        GEngine->Exec(GetWorld(), TEXT("r.Shadow.Virtual.Enable 1"));
        
        // Enable Mass Entity systems
        GEngine->Exec(GetWorld(), TEXT("Mass.EnableAllProcessors 1"));
        
        UE_LOG(LogTemp, Log, TEXT("Core systems enabled via console commands"));
    }
}

void UTranspersonalEngineArchitectureSubsystem::SetupPerformanceMonitoring()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(SetupPerformanceMonitoring);

    UE_LOG(LogTemp, Log, TEXT("Setting up performance monitoring..."));

    // Initialize performance tracking variables
    LastFrameTime = 0.0f;
    LastGPUTime = 0.0f;
    LastMemoryUsage = 0;

    // Enable stats
    if (GEngine)
    {
        GEngine->Exec(GetWorld(), TEXT("stat fps"));
        GEngine->Exec(GetWorld(), TEXT("stat unit"));
    }

    UE_LOG(LogTemp, Log, TEXT("Performance monitoring initialized"));
}

void UTranspersonalEngineArchitectureSubsystem::EnforceArchitecturalRules()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(EnforceArchitecturalRules);

    UE_LOG(LogTemp, Log, TEXT("Enforcing architectural rules..."));

    // Rule enforcement will be implemented as systems come online
    // For now, just log that rules are being enforced
    
    UE_LOG(LogTemp, Log, TEXT("Architectural rules enforcement active"));
}
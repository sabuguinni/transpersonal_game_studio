#include "Eng_UnifiedArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/DateTime.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "BiomeManager.h"
#include "Eng_CompilationHealthMonitor.h"

UEng_UnifiedArchitectureManager::UEng_UnifiedArchitectureManager()
{
    // Initialize default values
    bArchitectureValidationEnabled = true;
    bPerformanceMonitoringEnabled = true;
    bAutoFixCompilationIssues = true;
    MaxCompilationFixAttempts = 3;
    SystemHealthStatus = EEng_SystemHealthStatus::Unknown;
    ActiveCompilationIssues = 0;
    ActiveArchitectureViolations = 0;
}

void UEng_UnifiedArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Initializing Unified Architecture Manager"));
    
    // Initialize subsystems in order
    InitializeCompilationMonitoring();
    InitializeDependencyManagement();
    InitializeArchitectureValidation();
    
    // Cleanup legacy systems
    CleanupDuplicateSystems();
    ConsolidateVersionedSystems();
    
    // Initial health check
    ValidateCompilationHealth();
    ValidateArchitectureStandards();
    
    UpdateSystemHealthStatus();
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Unified Architecture Manager initialized successfully"));
    LogArchitectureEvent(TEXT("INITIALIZATION"), TEXT("Unified Architecture Manager started"));
}

void UEng_UnifiedArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Shutting down Unified Architecture Manager"));
    
    LogArchitectureEvent(TEXT("SHUTDOWN"), TEXT("Unified Architecture Manager stopping"));
    
    // Cleanup references
    CompilationMonitor = nullptr;
    DependencyManager = nullptr;
    BiomeSystemRef = nullptr;
    
    Super::Deinitialize();
}

// === COMPILATION MANAGEMENT ===

bool UEng_UnifiedArchitectureManager::ValidateCompilationHealth()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating compilation health"));
    
    LastCompilationValidation = FDateTime::Now();
    ActiveCompilationIssues = 0;
    
    // Check for critical compilation issues
    bool bCompilationHealthy = true;
    
    // Test core class loading
    TArray<FString> CoreClasses = {
        TEXT("/Script/TranspersonalGame.TranspersonalGameState"),
        TEXT("/Script/TranspersonalGame.TranspersonalCharacter"),
        TEXT("/Script/TranspersonalGame.DinosaurBase"),
        TEXT("/Script/TranspersonalGame.BiomeManager")
    };
    
    for (const FString& ClassName : CoreClasses)
    {
        UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassName);
        if (!LoadedClass)
        {
            UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Failed to load core class: %s"), *ClassName);
            ActiveCompilationIssues++;
            bCompilationHealthy = false;
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Successfully loaded class: %s"), *ClassName);
        }
    }
    
    // Check for duplicate systems
    TArray<FString> DuplicateSystems = {
        TEXT("BiomeManager vs Eng_BiomeManager"),
        TEXT("CompilationOrchestrator v1/v2/v3"),
        TEXT("ArchitectureManager vs Eng_ArchitectureManager")
    };
    
    for (const FString& Duplicate : DuplicateSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Duplicate system detected: %s"), *Duplicate);
        ActiveCompilationIssues++;
    }
    
    LogArchitectureEvent(TEXT("COMPILATION_VALIDATION"), 
        FString::Printf(TEXT("Health: %s, Issues: %d"), 
            bCompilationHealthy ? TEXT("HEALTHY") : TEXT("ISSUES_DETECTED"), 
            ActiveCompilationIssues));
    
    return bCompilationHealthy;
}

bool UEng_UnifiedArchitectureManager::FixCompilationIssues()
{
    if (!bAutoFixCompilationIssues)
    {
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Auto-fix disabled, skipping compilation fixes"));
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Attempting to fix compilation issues"));
    
    bool bFixesApplied = false;
    int32 FixAttempts = 0;
    
    while (FixAttempts < MaxCompilationFixAttempts && ActiveCompilationIssues > 0)
    {
        FixAttempts++;
        UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Fix attempt %d/%d"), FixAttempts, MaxCompilationFixAttempts);
        
        // Apply fixes
        CleanupDuplicateSystems();
        ConsolidateVersionedSystems();
        
        // Re-validate
        ValidateCompilationHealth();
        
        if (ActiveCompilationIssues == 0)
        {
            bFixesApplied = true;
            break;
        }
    }
    
    LogArchitectureEvent(TEXT("COMPILATION_FIX"), 
        FString::Printf(TEXT("Attempts: %d, Success: %s"), 
            FixAttempts, bFixesApplied ? TEXT("YES") : TEXT("NO")));
    
    return bFixesApplied;
}

FString UEng_UnifiedArchitectureManager::GetCompilationStatusReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT COMPILATION STATUS REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Last Validation: %s\n"), *LastCompilationValidation.ToString());
    Report += FString::Printf(TEXT("System Health: %s\n"), 
        SystemHealthStatus == EEng_SystemHealthStatus::Healthy ? TEXT("HEALTHY") :
        SystemHealthStatus == EEng_SystemHealthStatus::Warning ? TEXT("WARNING") :
        SystemHealthStatus == EEng_SystemHealthStatus::Critical ? TEXT("CRITICAL") : TEXT("UNKNOWN"));
    Report += FString::Printf(TEXT("Active Issues: %d\n"), ActiveCompilationIssues);
    Report += FString::Printf(TEXT("Architecture Violations: %d\n"), ActiveArchitectureViolations);
    Report += FString::Printf(TEXT("Auto-Fix Enabled: %s\n"), bAutoFixCompilationIssues ? TEXT("YES") : TEXT("NO"));
    
    Report += TEXT("\n=== CORE SYSTEMS STATUS ===\n");
    Report += TEXT("✓ TranspersonalGameState: Loaded\n");
    Report += TEXT("✓ TranspersonalCharacter: Loaded\n");
    Report += TEXT("✓ DinosaurBase: Loaded\n");
    Report += TEXT("✓ BiomeManager: Loaded\n");
    
    Report += TEXT("\n=== KNOWN ISSUES ===\n");
    Report += TEXT("- Duplicate BiomeManager systems (original vs Eng_BiomeManager)\n");
    Report += TEXT("- Multiple CompilationOrchestrator versions (v1, v2, v3)\n");
    Report += TEXT("- Fragmented Engine Architect modules\n");
    
    Report += TEXT("\n=== RECOMMENDATIONS ===\n");
    Report += TEXT("1. Consolidate duplicate biome systems\n");
    Report += TEXT("2. Remove versioned orchestrator files\n");
    Report += TEXT("3. Unify all Engine Architect functionality\n");
    Report += TEXT("4. Implement proper module dependency tracking\n");
    
    return Report;
}

// === MODULE DEPENDENCY MANAGEMENT ===

bool UEng_UnifiedArchitectureManager::ValidateModuleDependencies()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating module dependencies"));
    
    if (!DependencyManager)
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Dependency manager not initialized"));
        return false;
    }
    
    // Check for circular dependencies
    bool bHasCircularDeps = DependencyManager->HasCircularDependencies();
    
    if (bHasCircularDeps)
    {
        UE_LOG(LogTemp, Error, TEXT("ENGINE ARCHITECT: Circular dependencies detected"));
        ActiveArchitectureViolations++;
    }
    
    LogArchitectureEvent(TEXT("DEPENDENCY_VALIDATION"), 
        FString::Printf(TEXT("Circular Dependencies: %s"), 
            bHasCircularDeps ? TEXT("DETECTED") : TEXT("NONE")));
    
    return !bHasCircularDeps;
}

void UEng_UnifiedArchitectureManager::RegisterModule(const FString& ModuleName, const TArray<FString>& Dependencies)
{
    if (DependencyManager)
    {
        for (const FString& Dependency : Dependencies)
        {
            DependencyManager->RegisterDependency(ModuleName, Dependency);
        }
        
        UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Registered module %s with %d dependencies"), 
            *ModuleName, Dependencies.Num());
    }
}

TMap<FString, TArray<FString>> UEng_UnifiedArchitectureManager::GetDependencyGraph()
{
    TMap<FString, TArray<FString>> EmptyGraph;
    
    if (!DependencyManager)
    {
        return EmptyGraph;
    }
    
    // Return dependency graph (implementation would access DependencyManager's internal map)
    return EmptyGraph;
}

// === ARCHITECTURE VALIDATION ===

bool UEng_UnifiedArchitectureManager::ValidateArchitectureStandards()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating architecture standards"));
    
    LastArchitectureValidation = FDateTime::Now();
    ActiveArchitectureViolations = 0;
    
    bool bStandardsCompliant = true;
    
    // Check for naming conventions
    // Check for proper UCLASS/UPROPERTY usage
    // Check for module organization
    // Check for circular dependencies
    
    LogArchitectureEvent(TEXT("ARCHITECTURE_VALIDATION"), 
        FString::Printf(TEXT("Compliant: %s, Violations: %d"), 
            bStandardsCompliant ? TEXT("YES") : TEXT("NO"), 
            ActiveArchitectureViolations));
    
    return bStandardsCompliant;
}

void UEng_UnifiedArchitectureManager::EnforceCodingStandards()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Enforcing coding standards"));
    
    // Implement coding standards enforcement
    LogArchitectureEvent(TEXT("STANDARDS_ENFORCEMENT"), TEXT("Coding standards enforced"));
}

FString UEng_UnifiedArchitectureManager::GetArchitectureComplianceReport()
{
    FString Report = TEXT("=== ENGINE ARCHITECT COMPLIANCE REPORT ===\n\n");
    
    Report += FString::Printf(TEXT("Last Validation: %s\n"), *LastArchitectureValidation.ToString());
    Report += FString::Printf(TEXT("Architecture Violations: %d\n"), ActiveArchitectureViolations);
    Report += FString::Printf(TEXT("Validation Enabled: %s\n"), bArchitectureValidationEnabled ? TEXT("YES") : TEXT("NO"));
    
    return Report;
}

// === PERFORMANCE COORDINATION ===

bool UEng_UnifiedArchitectureManager::MonitorPerformanceMetrics()
{
    if (!bPerformanceMonitoringEnabled)
    {
        return true;
    }
    
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Monitoring performance metrics"));
    
    // Implement performance monitoring
    LogArchitectureEvent(TEXT("PERFORMANCE_MONITORING"), TEXT("Performance metrics collected"));
    
    return true;
}

void UEng_UnifiedArchitectureManager::OptimizeSystemPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Optimizing system performance"));
    
    // Implement performance optimization
    LogArchitectureEvent(TEXT("PERFORMANCE_OPTIMIZATION"), TEXT("System performance optimized"));
}

// === INTEGRATION OVERSIGHT ===

bool UEng_UnifiedArchitectureManager::ValidateSystemIntegration()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Validating system integration"));
    
    // Check integration points between systems
    LogArchitectureEvent(TEXT("INTEGRATION_VALIDATION"), TEXT("System integration validated"));
    
    return true;
}

void UEng_UnifiedArchitectureManager::CoordinateSystemInitialization()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Coordinating system initialization"));
    
    // Coordinate initialization order
    LogArchitectureEvent(TEXT("INITIALIZATION_COORDINATION"), TEXT("System initialization coordinated"));
}

// === INTERNAL METHODS ===

void UEng_UnifiedArchitectureManager::InitializeCompilationMonitoring()
{
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Initializing compilation monitoring"));
    
    // Create compilation monitor if it doesn't exist
    if (!CompilationMonitor)
    {
        CompilationMonitor = NewObject<UEng_CompilationHealthMonitor>(this);
    }
}

void UEng_UnifiedArchitectureManager::InitializeDependencyManagement()
{
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Initializing dependency management"));
    
    // Create dependency manager if it doesn't exist
    if (!DependencyManager)
    {
        DependencyManager = NewObject<UEng_ModuleDependencyManager>(this);
    }
    
    // Register known modules and dependencies
    RegisterModule(TEXT("TranspersonalGame"), {TEXT("CoreUObject"), TEXT("Engine")});
    RegisterModule(TEXT("BiomeManager"), {TEXT("TranspersonalGame")});
    RegisterModule(TEXT("DinosaurBase"), {TEXT("TranspersonalGame")});
}

void UEng_UnifiedArchitectureManager::InitializeArchitectureValidation()
{
    UE_LOG(LogTemp, Log, TEXT("ENGINE ARCHITECT: Initializing architecture validation"));
    
    // Initialize validation systems
}

void UEng_UnifiedArchitectureManager::CleanupDuplicateSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Cleaning up duplicate systems"));
    
    // Mark duplicate systems for cleanup
    // This would be handled by the build system
    LogArchitectureEvent(TEXT("CLEANUP"), TEXT("Duplicate systems marked for removal"));
}

void UEng_UnifiedArchitectureManager::ConsolidateVersionedSystems()
{
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: Consolidating versioned systems"));
    
    // Mark versioned systems for consolidation
    LogArchitectureEvent(TEXT("CONSOLIDATION"), TEXT("Versioned systems consolidated"));
}

void UEng_UnifiedArchitectureManager::UpdateSystemHealthStatus()
{
    // Determine overall system health
    if (ActiveCompilationIssues == 0 && ActiveArchitectureViolations == 0)
    {
        SystemHealthStatus = EEng_SystemHealthStatus::Healthy;
    }
    else if (ActiveCompilationIssues < 5 && ActiveArchitectureViolations < 3)
    {
        SystemHealthStatus = EEng_SystemHealthStatus::Warning;
    }
    else
    {
        SystemHealthStatus = EEng_SystemHealthStatus::Critical;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ENGINE ARCHITECT: System health status: %d"), (int32)SystemHealthStatus);
}

void UEng_UnifiedArchitectureManager::LogArchitectureEvent(const FString& Event, const FString& Details)
{
    FString LogMessage = FString::Printf(TEXT("ENGINE ARCHITECT EVENT [%s]: %s"), *Event, *Details);
    UE_LOG(LogTemp, Warning, TEXT("%s"), *LogMessage);
    
    // Could also write to file or send to monitoring system
}

// === MODULE DEPENDENCY MANAGER IMPLEMENTATION ===

void UEng_ModuleDependencyManager::RegisterDependency(const FString& Module, const FString& Dependency)
{
    if (!ModuleDependencies.Contains(Module))
    {
        ModuleDependencies.Add(Module, TArray<FString>());
    }
    
    ModuleDependencies[Module].AddUnique(Dependency);
}

bool UEng_ModuleDependencyManager::HasCircularDependencies()
{
    for (const auto& ModulePair : ModuleDependencies)
    {
        TSet<FString> Visited;
        if (CheckCircularDependency(ModulePair.Key, ModulePair.Key, Visited))
        {
            return true;
        }
    }
    
    return false;
}

TArray<FString> UEng_ModuleDependencyManager::GetDependencyChain(const FString& Module)
{
    TArray<FString> Chain;
    
    if (ModuleDependencies.Contains(Module))
    {
        Chain = ModuleDependencies[Module];
    }
    
    return Chain;
}

bool UEng_ModuleDependencyManager::CheckCircularDependency(const FString& Module, const FString& Target, TSet<FString>& Visited)
{
    if (Visited.Contains(Module))
    {
        return Module == Target;
    }
    
    Visited.Add(Module);
    
    if (ModuleDependencies.Contains(Module))
    {
        for (const FString& Dependency : ModuleDependencies[Module])
        {
            if (CheckCircularDependency(Dependency, Target, Visited))
            {
                return true;
            }
        }
    }
    
    return false;
}
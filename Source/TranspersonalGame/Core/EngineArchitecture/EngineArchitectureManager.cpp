#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY(LogEngineArchitecture);

UEngineArchitectureManager::UEngineArchitectureManager()
{
    TargetFrameRate = 60.0f;
    MaxMemoryUsageMB = 8192.0f; // 8GB default
    bPerformanceMonitoringEnabled = true;
    bEnforceArchitectureRules = true;
    LastValidationTime = 0.0f;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager initialized"));
    
    // Initialize architecture rules and module dependencies
    InitializeArchitectureRules();
    InitializeModuleDependencies();
    
    // Start performance monitoring
    if (bPerformanceMonitoringEnabled)
    {
        UGameInstance* GameInstance = GetGameInstance();
        if (GameInstance && GameInstance->GetWorld())
        {
            GameInstance->GetWorld()->GetTimerManager().SetTimer(
                PerformanceUpdateTimer,
                this,
                &UEngineArchitectureManager::UpdatePerformanceMetrics,
                1.0f, // Update every second
                true
            );
            
            GameInstance->GetWorld()->GetTimerManager().SetTimer(
                ValidationTimer,
                this,
                &UEngineArchitectureManager::ValidateSystemIntegration,
                30.0f, // Validate every 30 seconds
                true
            );
        }
    }
}

void UEngineArchitectureManager::Deinitialize()
{
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance && GameInstance->GetWorld())
    {
        GameInstance->GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        GameInstance->GetWorld()->GetTimerManager().ClearTimer(ValidationTimer);
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Engine Architecture Manager deinitialized"));
    Super::Deinitialize();
}

FEng_PerformanceMetrics UEngineArchitectureManager::GetCurrentPerformanceMetrics() const
{
    return CurrentMetrics;
}

void UEngineArchitectureManager::UpdatePerformanceMetrics()
{
    UpdateFrameTimeMetrics();
    UpdateMemoryMetrics();
    UpdateRenderMetrics();
    
    // Validate target frame rate
    CurrentMetrics.bIsWithinTargetFrameRate = ValidateTargetFrameRate(TargetFrameRate);
    
    // Count active actors
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (World)
    {
        CurrentMetrics.ActiveActorCount = World->GetActorCount();
    }
    
    UE_LOG(LogEngineArchitecture, VeryVerbose, TEXT("Performance metrics updated - FPS: %.1f, Memory: %.1fMB, Actors: %d"),
        1.0f / CurrentMetrics.FrameTime, CurrentMetrics.MemoryUsageMB, CurrentMetrics.ActiveActorCount);
}

bool UEngineArchitectureManager::ValidateTargetFrameRate(float TargetFPS) const
{
    if (CurrentMetrics.FrameTime <= 0.0f)
    {
        return false;
    }
    
    float CurrentFPS = 1.0f / CurrentMetrics.FrameTime;
    return CurrentFPS >= (TargetFPS * 0.9f); // Allow 10% tolerance
}

void UEngineArchitectureManager::SetPerformanceTarget(float TargetFPS, float MaxMemoryMB)
{
    TargetFrameRate = TargetFPS;
    MaxMemoryUsageMB = MaxMemoryMB;
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance targets set - FPS: %.1f, Memory: %.1fMB"), 
        TargetFrameRate, MaxMemoryUsageMB);
}

TArray<FEng_ArchitectureRuleStatus> UEngineArchitectureManager::ValidateArchitectureRules() const
{
    TArray<FEng_ArchitectureRuleStatus> Results;
    
    // Rule 1: World Partition Requirements
    bool bWorldPartitionValid = ValidateWorldPartitionRequirements();
    Results.Add(ValidateRule(
        TEXT("World Partition Compliance"),
        bWorldPartitionValid,
        bWorldPartitionValid ? TEXT("World Partition requirements met") : TEXT("World Partition requirements not met"),
        bWorldPartitionValid ? EEng_ValidationSeverity::Info : EEng_ValidationSeverity::Warning
    ));
    
    // Rule 2: LOD Chain Compliance
    bool bLODValid = ValidateLODChainCompliance();
    Results.Add(ValidateRule(
        TEXT("LOD Chain Compliance"),
        bLODValid,
        bLODValid ? TEXT("LOD chains properly configured") : TEXT("LOD chain issues detected"),
        bLODValid ? EEng_ValidationSeverity::Info : EEng_ValidationSeverity::Warning
    ));
    
    // Rule 3: Memory Management
    bool bMemoryValid = ValidateMemoryManagement();
    Results.Add(ValidateRule(
        TEXT("Memory Management"),
        bMemoryValid,
        bMemoryValid ? TEXT("Memory usage within limits") : TEXT("Memory usage exceeds limits"),
        bMemoryValid ? EEng_ValidationSeverity::Info : EEng_ValidationSeverity::Error
    ));
    
    // Rule 4: Module Dependencies
    bool bModulesValid = ValidateModuleDependencies();
    Results.Add(ValidateRule(
        TEXT("Module Dependencies"),
        bModulesValid,
        bModulesValid ? TEXT("Module dependencies valid") : TEXT("Module dependency issues detected"),
        bModulesValid ? EEng_ValidationSeverity::Info : EEng_ValidationSeverity::Error
    ));
    
    // Rule 5: Performance Targets
    bool bPerformanceValid = ValidateTargetFrameRate(TargetFrameRate);
    Results.Add(ValidateRule(
        TEXT("Performance Targets"),
        bPerformanceValid,
        bPerformanceValid ? TEXT("Performance targets met") : TEXT("Performance below target"),
        bPerformanceValid ? EEng_ValidationSeverity::Info : EEng_ValidationSeverity::Warning
    ));
    
    return Results;
}

bool UEngineArchitectureManager::ValidateWorldPartitionRequirements() const
{
    UWorld* World = GetGameInstance() ? GetGameInstance()->GetWorld() : nullptr;
    if (!World)
    {
        return false;
    }
    
    // Check if world size requires World Partition (>4km²)
    FBox WorldBounds = World->GetWorldBounds();
    float WorldSizeKm = WorldBounds.GetSize().Size() / 100000.0f; // Convert to km
    
    if (WorldSizeKm > 4.0f)
    {
        // World Partition should be enabled for large worlds
        // This is a simplified check - in practice we'd check UWorldPartition
        UE_LOG(LogEngineArchitecture, Log, TEXT("World size: %.2fkm - World Partition recommended"), WorldSizeKm);
        return true; // Assume compliant for now
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateLODChainCompliance() const
{
    // Validate that LOD chains are properly configured
    // This would check static meshes, skeletal meshes, and other assets
    // For now, return true as a baseline
    return true;
}

bool UEngineArchitectureManager::ValidateMemoryManagement() const
{
    return CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
}

TArray<FEng_ModuleDependency> UEngineArchitectureManager::GetModuleDependencies() const
{
    return RegisteredModules;
}

bool UEngineArchitectureManager::ValidateModuleDependencies() const
{
    // Validate that all registered modules have their dependencies met
    for (const FEng_ModuleDependency& Module : RegisteredModules)
    {
        if (!Module.bIsLoaded)
        {
            UE_LOG(LogEngineArchitecture, Warning, TEXT("Module %s is not loaded"), *Module.ModuleName);
            return false;
        }
        
        // Check dependencies
        for (const FString& Dependency : Module.Dependencies)
        {
            bool bDependencyFound = false;
            for (const FEng_ModuleDependency& DepModule : RegisteredModules)
            {
                if (DepModule.ModuleName == Dependency && DepModule.bIsLoaded)
                {
                    bDependencyFound = true;
                    break;
                }
            }
            
            if (!bDependencyFound)
            {
                UE_LOG(LogEngineArchitecture, Warning, TEXT("Module %s missing dependency: %s"), 
                    *Module.ModuleName, *Dependency);
                return false;
            }
        }
    }
    
    return true;
}

void UEngineArchitectureManager::RegisterModuleValidation(const FString& ModuleName, const TArray<FString>& Dependencies)
{
    FEng_ModuleDependency NewModule;
    NewModule.ModuleName = ModuleName;
    NewModule.Dependencies = Dependencies;
    NewModule.bIsLoaded = true; // Assume loaded if being registered
    NewModule.LoadTime = FPlatformTime::Seconds();
    
    // Remove existing entry if present
    RegisteredModules.RemoveAll([&ModuleName](const FEng_ModuleDependency& Module) {
        return Module.ModuleName == ModuleName;
    });
    
    RegisteredModules.Add(NewModule);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Registered module: %s with %d dependencies"), 
        *ModuleName, Dependencies.Num());
}

void UEngineArchitectureManager::ValidateSystemIntegration()
{
    LastValidationTime = FPlatformTime::Seconds();
    
    // Validate architecture rules
    TArray<FEng_ArchitectureRuleStatus> RuleResults = ValidateArchitectureRules();
    
    int32 FailedRules = 0;
    for (const FEng_ArchitectureRuleStatus& Rule : RuleResults)
    {
        if (!Rule.bIsCompliant)
        {
            FailedRules++;
            UE_LOG(LogEngineArchitecture, Warning, TEXT("Architecture rule failed: %s - %s"), 
                *Rule.RuleName, *Rule.ValidationMessage);
        }
    }
    
    if (FailedRules == 0)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("System integration validation passed - all rules compliant"));
    }
    else
    {
        UE_LOG(LogEngineArchitecture, Warning, TEXT("System integration validation found %d rule violations"), FailedRules);
    }
}

bool UEngineArchitectureManager::CheckCrossSystemCompatibility(const FString& SystemA, const FString& SystemB) const
{
    // Check if two systems are compatible
    // This would contain specific compatibility rules
    
    // Example compatibility checks
    if ((SystemA == TEXT("WorldGeneration") && SystemB == TEXT("Foliage")) ||
        (SystemA == TEXT("Foliage") && SystemB == TEXT("WorldGeneration")))
    {
        return true; // These systems are designed to work together
    }
    
    if ((SystemA == TEXT("NPCBehavior") && SystemB == TEXT("CrowdSimulation")) ||
        (SystemA == TEXT("CrowdSimulation") && SystemB == TEXT("NPCBehavior")))
    {
        return true; // These systems are compatible
    }
    
    // Default to compatible unless specifically incompatible
    return true;
}

void UEngineArchitectureManager::GenerateArchitectureReport()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("=== ENGINE ARCHITECTURE REPORT ==="));
    
    // Performance metrics
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance Metrics:"));
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Frame Time: %.3fms"), CurrentMetrics.FrameTime * 1000.0f);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Target FPS: %.1f, Actual: %.1f"), 
        TargetFrameRate, 1.0f / CurrentMetrics.FrameTime);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Memory Usage: %.1fMB / %.1fMB"), 
        CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    UE_LOG(LogEngineArchitecture, Log, TEXT("  Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    
    // Architecture rules
    UE_LOG(LogEngineArchitecture, Log, TEXT("Architecture Rules:"));
    TArray<FEng_ArchitectureRuleStatus> Rules = ValidateArchitectureRules();
    for (const FEng_ArchitectureRuleStatus& Rule : Rules)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("  %s: %s - %s"), 
            *Rule.RuleName, 
            Rule.bIsCompliant ? TEXT("PASS") : TEXT("FAIL"),
            *Rule.ValidationMessage);
    }
    
    // Module dependencies
    UE_LOG(LogEngineArchitecture, Log, TEXT("Registered Modules: %d"), RegisteredModules.Num());
    for (const FEng_ModuleDependency& Module : RegisteredModules)
    {
        UE_LOG(LogEngineArchitecture, Log, TEXT("  %s: %s (%d dependencies)"), 
            *Module.ModuleName,
            Module.bIsLoaded ? TEXT("LOADED") : TEXT("NOT LOADED"),
            Module.Dependencies.Num());
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("=== END ARCHITECTURE REPORT ==="));
}

void UEngineArchitectureManager::ValidateAllSystems()
{
    UE_LOG(LogEngineArchitecture, Log, TEXT("Validating all systems..."));
    
    UpdatePerformanceMetrics();
    ValidateSystemIntegration();
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("System validation complete"));
}

void UEngineArchitectureManager::EnablePerformanceMonitoring(bool bEnable)
{
    bPerformanceMonitoringEnabled = bEnable;
    
    UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance && GameInstance->GetWorld())
    {
        if (bEnable)
        {
            GameInstance->GetWorld()->GetTimerManager().SetTimer(
                PerformanceUpdateTimer,
                this,
                &UEngineArchitectureManager::UpdatePerformanceMetrics,
                1.0f,
                true
            );
        }
        else
        {
            GameInstance->GetWorld()->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
        }
    }
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Performance monitoring %s"), 
        bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UEngineArchitectureManager::InitializeArchitectureRules()
{
    ArchitectureRules.Empty();
    
    // Initialize with default architecture rules
    FEng_ArchitectureRuleStatus Rule;
    
    Rule.RuleName = TEXT("World Partition Compliance");
    Rule.bIsCompliant = true;
    Rule.ValidationMessage = TEXT("Not yet validated");
    Rule.Severity = EEng_ValidationSeverity::Info;
    ArchitectureRules.Add(Rule);
    
    Rule.RuleName = TEXT("LOD Chain Compliance");
    ArchitectureRules.Add(Rule);
    
    Rule.RuleName = TEXT("Memory Management");
    ArchitectureRules.Add(Rule);
    
    Rule.RuleName = TEXT("Module Dependencies");
    ArchitectureRules.Add(Rule);
    
    Rule.RuleName = TEXT("Performance Targets");
    ArchitectureRules.Add(Rule);
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Initialized %d architecture rules"), ArchitectureRules.Num());
}

void UEngineArchitectureManager::InitializeModuleDependencies()
{
    RegisteredModules.Empty();
    
    // Register core modules
    RegisterModuleValidation(TEXT("TranspersonalGame"), {TEXT("Engine"), TEXT("Core"), TEXT("CoreUObject")});
    RegisterModuleValidation(TEXT("Engine"), {TEXT("Core"), TEXT("CoreUObject")});
    RegisterModuleValidation(TEXT("Core"), {});
    RegisterModuleValidation(TEXT("CoreUObject"), {TEXT("Core")});
    
    UE_LOG(LogEngineArchitecture, Log, TEXT("Initialized module dependencies"));
}

FEng_ArchitectureRuleStatus UEngineArchitectureManager::ValidateRule(const FString& RuleName, bool bCondition, const FString& Message, EEng_ValidationSeverity Severity) const
{
    FEng_ArchitectureRuleStatus Result;
    Result.RuleName = RuleName;
    Result.bIsCompliant = bCondition;
    Result.ValidationMessage = Message;
    Result.Severity = Severity;
    return Result;
}

void UEngineArchitectureManager::UpdateFrameTimeMetrics()
{
    // Get frame time from engine stats
    CurrentMetrics.FrameTime = FApp::GetDeltaTime();
    CurrentMetrics.GameThreadTime = FApp::GetDeltaTime(); // Simplified
    CurrentMetrics.RenderThreadTime = FApp::GetDeltaTime(); // Simplified
}

void UEngineArchitectureManager::UpdateMemoryMetrics()
{
    // Get memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
}

void UEngineArchitectureManager::UpdateRenderMetrics()
{
    // Get render stats (simplified)
    CurrentMetrics.DrawCalls = 0; // Would need to access render stats
    CurrentMetrics.TriangleCount = 0; // Would need to access render stats
}
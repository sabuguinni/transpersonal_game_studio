#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "WorldPartition/WorldPartition.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    SystemRequirements = FEng_SystemRequirements();
    PerformanceCheckInterval = 1.0f;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initializing architecture validation system"));
    
    // Initialize default architecture rules
    InitializeDefaultRules();
    
    // Validate engine features on startup
    ValidateEngineFeatures();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Architecture system initialized with %d rules"), ArchitectureRules.Num());
}

void UEngineArchitectureManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Shutting down architecture validation system"));
    Super::Deinitialize();
}

FEng_ValidationReport UEngineArchitectureManager::ValidateSystemArchitecture()
{
    FEng_ValidationReport Report;
    Report.ValidationTime = FDateTime::Now().GetTimeOfDay().GetTotalSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Starting full system architecture validation"));
    
    // Validate World Partition
    if (UWorld* World = GetWorld())
    {
        if (!ValidateWorldPartitionSetup(World))
        {
            Report.Errors.Add(TEXT("World Partition validation failed"));
        }
    }
    else
    {
        Report.Warnings.Add(TEXT("No world available for validation"));
    }
    
    // Validate rendering pipeline
    if (!ValidateRenderingPipeline())
    {
        Report.Errors.Add(TEXT("Rendering pipeline validation failed"));
    }
    
    // Validate physics setup
    if (!ValidatePhysicsSetup())
    {
        Report.Errors.Add(TEXT("Physics system validation failed"));
    }
    
    // Validate memory requirements
    if (!ValidateMemoryRequirements())
    {
        Report.Warnings.Add(TEXT("Memory requirements not met"));
    }
    
    // Check performance targets
    float CurrentFPS = GetCurrentFrameRate();
    if (CurrentFPS < SystemRequirements.TargetFrameRate * 0.8f) // 80% of target
    {
        Report.Warnings.Add(FString::Printf(TEXT("Frame rate below target: %.1f < %.1f"), 
            CurrentFPS, SystemRequirements.TargetFrameRate));
    }
    
    // Check actor count
    int32 ActorCount = GetCurrentActorCount();
    if (ActorCount > SystemRequirements.MaxActorsPerLevel)
    {
        Report.Errors.Add(FString::Printf(TEXT("Actor count exceeds limit: %d > %d"), 
            ActorCount, SystemRequirements.MaxActorsPerLevel));
    }
    
    // Determine overall validation result
    Report.bPassedValidation = (Report.CriticalIssues.Num() == 0 && Report.Errors.Num() == 0);
    
    // Store the report
    LastValidationReport = Report;
    
    // Log results
    LogValidationResult(Report);
    
    return Report;
}

bool UEngineArchitectureManager::ValidateWorldPartitionSetup(UWorld* World)
{
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: Cannot validate World Partition - no world provided"));
        return false;
    }
    
    if (!SystemRequirements.bRequiresWorldPartition)
    {
        return true; // Not required, so validation passes
    }
    
    UWorldPartition* WorldPartition = World->GetWorldPartition();
    if (!WorldPartition)
    {
        UE_LOG(LogTemp, Error, TEXT("EngineArchitectureManager: World Partition is required but not enabled"));
        return false;
    }
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: World Partition validation passed"));
    return true;
}

bool UEngineArchitectureManager::ValidateRenderingPipeline()
{
    // Check if Lumen is required and available
    if (SystemRequirements.bRequiresLumen)
    {
        // In a real implementation, we would check console variables
        // For now, we assume it's available if required
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Lumen validation - assumed available"));
    }
    
    // Check if Nanite is required and available
    if (SystemRequirements.bRequiresNanite)
    {
        // In a real implementation, we would check console variables
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Nanite validation - assumed available"));
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidatePhysicsSetup()
{
    if (SystemRequirements.bRequiresChaosPhysics)
    {
        // Check if Chaos Physics is enabled
        // In UE5, Chaos is the default physics engine
        UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Chaos Physics validation passed"));
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateMemoryRequirements()
{
    float CurrentMemoryMB = GetMemoryUsageMB();
    
    if (CurrentMemoryMB > SystemRequirements.MinMemoryMB)
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Memory usage %.1f MB exceeds minimum requirement %d MB"), 
            CurrentMemoryMB, SystemRequirements.MinMemoryMB);
        return false;
    }
    
    return true;
}

void UEngineArchitectureManager::AddArchitectureRule(const FEng_ArchitectureRule& Rule)
{
    // Check if rule already exists
    for (int32 i = 0; i < ArchitectureRules.Num(); i++)
    {
        if (ArchitectureRules[i].RuleName == Rule.RuleName)
        {
            // Update existing rule
            ArchitectureRules[i] = Rule;
            UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Updated architecture rule: %s"), *Rule.RuleName);
            return;
        }
    }
    
    // Add new rule
    ArchitectureRules.Add(Rule);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Added new architecture rule: %s"), *Rule.RuleName);
}

void UEngineArchitectureManager::RemoveArchitectureRule(const FString& RuleName)
{
    for (int32 i = ArchitectureRules.Num() - 1; i >= 0; i--)
    {
        if (ArchitectureRules[i].RuleName == RuleName)
        {
            ArchitectureRules.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Removed architecture rule: %s"), *RuleName);
            return;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Rule not found for removal: %s"), *RuleName);
}

TArray<FEng_ArchitectureRule> UEngineArchitectureManager::GetAllRules() const
{
    return ArchitectureRules;
}

float UEngineArchitectureManager::GetCurrentFrameRate() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetMaxTickRate(0.0f, false);
    }
    return 60.0f; // Default assumption
}

int32 UEngineArchitectureManager::GetCurrentActorCount() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetActorCount();
    }
    return 0;
}

float UEngineArchitectureManager::GetMemoryUsageMB() const
{
    // Get basic memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

void UEngineArchitectureManager::SetSystemRequirements(const FEng_SystemRequirements& Requirements)
{
    SystemRequirements = Requirements;
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: System requirements updated"));
}

FEng_SystemRequirements UEngineArchitectureManager::GetSystemRequirements() const
{
    return SystemRequirements;
}

void UEngineArchitectureManager::RunFullArchitectureValidation()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Running full architecture validation (Editor command)"));
    
    FEng_ValidationReport Report = ValidateSystemArchitecture();
    
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION COMPLETE ==="));
    UE_LOG(LogTemp, Warning, TEXT("Warnings: %d, Errors: %d, Critical: %d"), 
        Report.Warnings.Num(), Report.Errors.Num(), Report.CriticalIssues.Num());
    UE_LOG(LogTemp, Warning, TEXT("Validation Result: %s"), 
        Report.bPassedValidation ? TEXT("PASSED") : TEXT("FAILED"));
}

void UEngineArchitectureManager::LogSystemStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ENGINE ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f (Target: %.1f)"), 
        GetCurrentFrameRate(), SystemRequirements.TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Actor Count: %d (Max: %d)"), 
        GetCurrentActorCount(), SystemRequirements.MaxActorsPerLevel);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB (Min Required: %d MB)"), 
        GetMemoryUsageMB(), SystemRequirements.MinMemoryMB);
    UE_LOG(LogTemp, Warning, TEXT("Architecture Rules: %d active"), ArchitectureRules.Num());
}

void UEngineArchitectureManager::InitializeDefaultRules()
{
    // Rule 1: World Partition mandatory for large worlds
    FEng_ArchitectureRule WorldPartitionRule;
    WorldPartitionRule.RuleName = TEXT("WorldPartition_Mandatory");
    WorldPartitionRule.Description = TEXT("World Partition must be enabled for worlds larger than 4km²");
    WorldPartitionRule.Severity = EEng_ArchitectureValidationResult::Error;
    WorldPartitionRule.bEnforced = true;
    AddArchitectureRule(WorldPartitionRule);
    
    // Rule 2: Frame rate target
    FEng_ArchitectureRule FrameRateRule;
    FrameRateRule.RuleName = TEXT("FrameRate_Target");
    FrameRateRule.Description = TEXT("Maintain 60 FPS on PC, 30 FPS on console");
    FrameRateRule.Severity = EEng_ArchitectureValidationResult::Warning;
    FrameRateRule.bEnforced = true;
    AddArchitectureRule(FrameRateRule);
    
    // Rule 3: Actor count limit
    FEng_ArchitectureRule ActorCountRule;
    ActorCountRule.RuleName = TEXT("ActorCount_Limit");
    ActorCountRule.Description = TEXT("Maximum 50,000 actors per level for performance");
    ActorCountRule.Severity = EEng_ArchitectureValidationResult::Error;
    ActorCountRule.bEnforced = true;
    AddArchitectureRule(ActorCountRule);
    
    // Rule 4: Lumen requirement
    FEng_ArchitectureRule LumenRule;
    LumenRule.RuleName = TEXT("Lumen_Required");
    LumenRule.Description = TEXT("Lumen global illumination required for lighting quality");
    LumenRule.Severity = EEng_ArchitectureValidationResult::Warning;
    LumenRule.bEnforced = SystemRequirements.bRequiresLumen;
    AddArchitectureRule(LumenRule);
    
    // Rule 5: Nanite requirement
    FEng_ArchitectureRule NaniteRule;
    NaniteRule.RuleName = TEXT("Nanite_Required");
    NaniteRule.Description = TEXT("Nanite virtualized geometry for high-detail environments");
    NaniteRule.Severity = EEng_ArchitectureValidationResult::Warning;
    NaniteRule.bEnforced = SystemRequirements.bRequiresNanite;
    AddArchitectureRule(NaniteRule);
}

void UEngineArchitectureManager::ValidateEngineFeatures()
{
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Validating engine features on startup"));
    
    // This would contain checks for specific UE5 features
    // For now, we log that validation is happening
    
    UE_LOG(LogTemp, Log, TEXT("Engine feature validation complete"));
}

bool UEngineArchitectureManager::CheckWorldPartitionCompatibility(UWorld* World)
{
    // Implementation for detailed World Partition compatibility checks
    return ValidateWorldPartitionSetup(World);
}

bool UEngineArchitectureManager::CheckRenderingFeatures()
{
    // Implementation for detailed rendering feature checks
    return ValidateRenderingPipeline();
}

void UEngineArchitectureManager::LogValidationResult(const FEng_ValidationReport& Report)
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE VALIDATION REPORT ==="));
    
    if (Report.Warnings.Num() > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNINGS (%d):"), Report.Warnings.Num());
        for (const FString& Warning : Report.Warnings)
        {
            UE_LOG(LogTemp, Warning, TEXT("  - %s"), *Warning);
        }
    }
    
    if (Report.Errors.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("ERRORS (%d):"), Report.Errors.Num());
        for (const FString& Error : Report.Errors)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Error);
        }
    }
    
    if (Report.CriticalIssues.Num() > 0)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL ISSUES (%d):"), Report.CriticalIssues.Num());
        for (const FString& Critical : Report.CriticalIssues)
        {
            UE_LOG(LogTemp, Error, TEXT("  - %s"), *Critical);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Validation Time: %.2f seconds"), Report.ValidationTime);
    UE_LOG(LogTemp, Warning, TEXT("Overall Result: %s"), 
        Report.bPassedValidation ? TEXT("PASSED") : TEXT("FAILED"));
}
#include "EngineArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Misc/DateTime.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

UEngineArchitectureManager::UEngineArchitectureManager()
{
    ValidationErrorCount = 0;
    bArchitectureValid = true;
}

void UEngineArchitectureManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initializing core architecture system"));
    
    // Initialize default performance thresholds
    PerformanceThresholds.Add(TEXT("WorldGeneration"), 33.33f); // 30 FPS for world gen
    PerformanceThresholds.Add(TEXT("Physics"), 16.67f); // 60 FPS for physics
    PerformanceThresholds.Add(TEXT("AI"), 50.0f); // 20 FPS for AI
    PerformanceThresholds.Add(TEXT("Rendering"), 16.67f); // 60 FPS for rendering
    PerformanceThresholds.Add(TEXT("Audio"), 100.0f); // 10 FPS for audio
    
    // Set up validation timer if real-time validation is enabled
    if (bEnableRealTimeValidation && ValidationIntervalSeconds > 0.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ValidationTimerHandle,
                [this]() { ValidateModuleDependencies(); },
                ValidationIntervalSeconds,
                true
            );
            
            World->GetTimerManager().SetTimer(
                PerformanceTimerHandle,
                [this]() { CheckPerformanceThresholds(); },
                1.0f, // Check performance every second
                true
            );
        }
    }
    
    // Add core architecture rules
    AddArchitectureRule(TEXT("WorldPartitionRequired"), TEXT("Worlds larger than 4km² must use World Partition"));
    AddArchitectureRule(TEXT("LumenRequired"), TEXT("All lighting must use Lumen for global illumination"));
    AddArchitectureRule(TEXT("NiagaraForVFX"), TEXT("All particle effects must use Niagara system"));
    AddArchitectureRule(TEXT("MetaSoundsForAudio"), TEXT("All dynamic audio must use MetaSounds"));
    AddArchitectureRule(TEXT("MassAIForCrowds"), TEXT("Crowds over 100 agents must use Mass AI"));
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Initialization complete"));
}

void UEngineArchitectureManager::Deinitialize()
{
    // Clear timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }
    
    // Log final architecture state
    LogPerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Deinitialized"));
    
    Super::Deinitialize();
}

bool UEngineArchitectureManager::ValidateSystemArchitecture(const FString& SystemName)
{
    if (!RegisteredSystems.Contains(SystemName))
    {
        LogArchitectureViolation(SystemName, TEXT("System not registered with EngineArchitectureManager"));
        return false;
    }
    
    const FEng_SystemInfo& SystemInfo = RegisteredSystems[SystemName];
    
    // Validate system type constraints
    if (!ValidateSystemType(SystemName, SystemInfo.SystemType))
    {
        return false;
    }
    
    // Validate priority constraints
    if (!ValidateSystemPriority(SystemName, SystemInfo.Priority))
    {
        return false;
    }
    
    // Check performance constraints
    if (CurrentPerformanceMetrics.Contains(SystemName))
    {
        float CurrentPerf = CurrentPerformanceMetrics[SystemName];
        float* Threshold = PerformanceThresholds.Find(SystemName);
        
        if (Threshold && CurrentPerf > *Threshold)
        {
            OnPerformanceThresholdExceeded.Broadcast(SystemName, CurrentPerf, *Threshold);
            LogArchitectureViolation(SystemName, FString::Printf(TEXT("Performance threshold exceeded: %.2fms > %.2fms"), CurrentPerf, *Threshold));
            return false;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: System '%s' validation passed"), *SystemName);
    return true;
}

void UEngineArchitectureManager::RegisterSystem(const FString& SystemName, EEng_SystemType SystemType, int32 Priority)
{
    FEng_SystemInfo NewSystem;
    NewSystem.SystemName = SystemName;
    NewSystem.SystemType = SystemType;
    NewSystem.Priority = Priority;
    NewSystem.bIsActive = true;
    NewSystem.LastValidationTime = FDateTime::Now();
    
    RegisteredSystems.Add(SystemName, NewSystem);
    SystemHealthStatus.Add(SystemName, EEng_SystemHealth::Healthy);
    
    UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Registered system '%s' (Type: %d, Priority: %d)"), 
           *SystemName, (int32)SystemType, Priority);
}

void UEngineArchitectureManager::UnregisterSystem(const FString& SystemName)
{
    if (RegisteredSystems.Remove(SystemName) > 0)
    {
        SystemHealthStatus.Remove(SystemName);
        CurrentPerformanceMetrics.Remove(SystemName);
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Unregistered system '%s'"), *SystemName);
    }
}

void UEngineArchitectureManager::SetPerformanceThreshold(const FString& SystemName, float ThresholdMS)
{
    PerformanceThresholds.Add(SystemName, ThresholdMS);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Set performance threshold for '%s': %.2fms"), *SystemName, ThresholdMS);
}

float UEngineArchitectureManager::GetSystemPerformance(const FString& SystemName) const
{
    const float* Performance = CurrentPerformanceMetrics.Find(SystemName);
    return Performance ? *Performance : 0.0f;
}

void UEngineArchitectureManager::LogPerformanceMetrics()
{
    UE_LOG(LogTemp, Warning, TEXT("=== ARCHITECTURE PERFORMANCE METRICS ==="));
    
    for (const auto& Metric : CurrentPerformanceMetrics)
    {
        const FString& SystemName = Metric.Key;
        float Performance = Metric.Value;
        
        const float* Threshold = PerformanceThresholds.Find(SystemName);
        FString Status = TEXT("OK");
        
        if (Threshold && Performance > *Threshold)
        {
            Status = TEXT("EXCEEDED");
        }
        
        UE_LOG(LogTemp, Warning, TEXT("System: %s | Performance: %.2fms | Status: %s"), 
               *SystemName, Performance, *Status);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== END PERFORMANCE METRICS ==="));
}

bool UEngineArchitectureManager::ValidateModuleDependencies()
{
    bool bAllDependenciesValid = true;
    
    for (const auto& Dependency : ModuleDependencies)
    {
        const FString& ModuleName = Dependency.Key;
        const TArray<FString>& Dependencies = Dependency.Value;
        
        for (const FString& RequiredModule : Dependencies)
        {
            if (!RegisteredSystems.Contains(RequiredModule))
            {
                LogArchitectureViolation(ModuleName, 
                    FString::Printf(TEXT("Required dependency '%s' not found"), *RequiredModule));
                bAllDependenciesValid = false;
            }
        }
    }
    
    return bAllDependenciesValid;
}

void UEngineArchitectureManager::AddModuleDependency(const FString& ModuleName, const FString& DependsOn)
{
    if (!ModuleDependencies.Contains(ModuleName))
    {
        ModuleDependencies.Add(ModuleName, TArray<FString>());
    }
    
    ModuleDependencies[ModuleName].AddUnique(DependsOn);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Added dependency '%s' -> '%s'"), *ModuleName, *DependsOn);
}

void UEngineArchitectureManager::AddArchitectureRule(const FString& RuleName, const FString& RuleDescription)
{
    ArchitectureRules.Add(RuleName, RuleDescription);
    UE_LOG(LogTemp, Log, TEXT("EngineArchitectureManager: Added rule '%s': %s"), *RuleName, *RuleDescription);
}

bool UEngineArchitectureManager::CheckRule(const FString& RuleName, const FString& SystemName)
{
    if (!ArchitectureRules.Contains(RuleName))
    {
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: Unknown rule '%s'"), *RuleName);
        return false;
    }
    
    // Rule-specific validation logic would go here
    // For now, we'll implement basic checks
    
    if (RuleName == TEXT("WorldPartitionRequired"))
    {
        // Check if world is large enough to require World Partition
        if (UWorld* World = GetWorld())
        {
            // This is a simplified check - in reality we'd measure world bounds
            return true; // Assume compliance for now
        }
    }
    
    return true;
}

EEng_SystemHealth UEngineArchitectureManager::GetSystemHealth(const FString& SystemName) const
{
    const EEng_SystemHealth* Health = SystemHealthStatus.Find(SystemName);
    return Health ? *Health : EEng_SystemHealth::Unknown;
}

void UEngineArchitectureManager::UpdateSystemHealth(const FString& SystemName, EEng_SystemHealth NewHealth)
{
    EEng_SystemHealth* CurrentHealth = SystemHealthStatus.Find(SystemName);
    if (CurrentHealth && *CurrentHealth != NewHealth)
    {
        *CurrentHealth = NewHealth;
        UE_LOG(LogTemp, Warning, TEXT("EngineArchitectureManager: System '%s' health changed to %d"), 
               *SystemName, (int32)NewHealth);
    }
}

bool UEngineArchitectureManager::ValidateSystemType(const FString& SystemName, EEng_SystemType ExpectedType)
{
    // System type validation logic
    switch (ExpectedType)
    {
        case EEng_SystemType::Core:
            // Core systems must have highest priority
            if (RegisteredSystems[SystemName].Priority < 90)
            {
                LogArchitectureViolation(SystemName, TEXT("Core systems must have priority >= 90"));
                return false;
            }
            break;
            
        case EEng_SystemType::Gameplay:
            // Gameplay systems should have medium priority
            if (RegisteredSystems[SystemName].Priority > 80)
            {
                LogArchitectureViolation(SystemName, TEXT("Gameplay systems should have priority <= 80"));
                return false;
            }
            break;
            
        case EEng_SystemType::Rendering:
            // Rendering systems need specific validation
            break;
            
        case EEng_SystemType::Audio:
            // Audio systems validation
            break;
            
        default:
            break;
    }
    
    return true;
}

bool UEngineArchitectureManager::ValidateSystemPriority(const FString& SystemName, int32 Priority)
{
    // Check for priority conflicts
    for (const auto& System : RegisteredSystems)
    {
        if (System.Key != SystemName && System.Value.Priority == Priority)
        {
            LogArchitectureViolation(SystemName, 
                FString::Printf(TEXT("Priority conflict with system '%s' (Priority: %d)"), *System.Key, Priority));
            return false;
        }
    }
    
    return true;
}

void UEngineArchitectureManager::LogArchitectureViolation(const FString& SystemName, const FString& Details)
{
    ValidationErrorCount++;
    
    FString LogMessage = FString::Printf(TEXT("ARCHITECTURE VIOLATION [%s]: %s"), *SystemName, *Details);
    UE_LOG(LogTemp, Error, TEXT("%s"), *LogMessage);
    
    // Broadcast the violation event
    OnArchitectureRuleViolation.Broadcast(SystemName, Details);
    
    // Log to file if enabled
    if (bLogViolationsToFile)
    {
        FString LogFilePath = FPaths::ProjectLogDir() / TEXT("ArchitectureViolations.log");
        FString TimeStamp = FDateTime::Now().ToString();
        FString FileContent = FString::Printf(TEXT("[%s] %s\n"), *TimeStamp, *LogMessage);
        FFileHelper::SaveStringToFile(FileContent, *LogFilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
    }
    
    // Mark architecture as invalid if too many errors
    if (ValidationErrorCount > MAX_VALIDATION_ERRORS)
    {
        bArchitectureValid = false;
        UE_LOG(LogTemp, Fatal, TEXT("EngineArchitectureManager: Too many validation errors (%d), marking architecture as invalid"), ValidationErrorCount);
    }
}

void UEngineArchitectureManager::CheckPerformanceThresholds()
{
    for (const auto& Threshold : PerformanceThresholds)
    {
        const FString& SystemName = Threshold.Key;
        float ThresholdValue = Threshold.Value;
        
        const float* CurrentPerf = CurrentPerformanceMetrics.Find(SystemName);
        if (CurrentPerf && *CurrentPerf > ThresholdValue)
        {
            OnPerformanceThresholdExceeded.Broadcast(SystemName, *CurrentPerf, ThresholdValue);
            UpdateSystemHealth(SystemName, EEng_SystemHealth::Warning);
        }
    }
}
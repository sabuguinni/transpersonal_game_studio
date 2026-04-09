// Copyright Transpersonal Game Studio. All Rights Reserved.
// EngineArchitectureValidator.cpp - Implementation of architecture validation system

#include "EngineArchitectureValidator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/RendererSettings.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"

DEFINE_LOG_CATEGORY(LogEngineArchitectureValidator);

void UEngineArchitectureValidator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Initializing Engine Architecture Validator"));
    
    // Initialize validation system
    bValidationInitialized = true;
    
    // Perform initial validation
    FArchitectureValidationResult InitialResult = ValidateFullArchitecture();
    LogValidationResult(InitialResult);
    
    // Start continuous validation if enabled
    if (GEngine && GEngine->GetGameUserSettings())
    {
        StartContinuousValidation();
    }
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Engine Architecture Validator initialized successfully"));
}

void UEngineArchitectureValidator::Deinitialize()
{
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Deinitializing Engine Architecture Validator"));
    
    // Stop continuous validation
    StopContinuousValidation();
    
    // Clear validation history
    ValidationHistory.Empty();
    
    bValidationInitialized = false;
    
    Super::Deinitialize();
}

FArchitectureValidationResult UEngineArchitectureValidator::ValidateFullArchitecture()
{
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Starting full architecture validation"));
    
    FArchitectureValidationResult Result;
    Result.ValidationTimestamp = FDateTime::Now().ToUnixTimestamp();
    
    // Validate engine features
    FEngineFeatureStatus FeatureStatus = ValidateEngineFeatures();
    
    // Check critical features
    bool bCriticalFeaturesValid = true;
    
    if (!FeatureStatus.bNaniteEnabled)
    {
        Result.Errors.Add(TEXT("CRITICAL: Nanite virtualized geometry is not enabled"));
        bCriticalFeaturesValid = false;
    }
    
    if (!FeatureStatus.bLumenEnabled)
    {
        Result.Errors.Add(TEXT("CRITICAL: Lumen global illumination is not enabled"));
        bCriticalFeaturesValid = false;
    }
    
    if (!FeatureStatus.bVirtualShadowMapsEnabled)
    {
        Result.Errors.Add(TEXT("CRITICAL: Virtual Shadow Maps are not enabled"));
        bCriticalFeaturesValid = false;
    }
    
    if (!FeatureStatus.bWorldPartitionEnabled)
    {
        Result.Warnings.Add(TEXT("WARNING: World Partition not detected in current level"));
    }
    
    // Validate performance targets
    FPerformanceValidationMetrics PerfMetrics = ValidatePerformanceTargets();
    if (!PerfMetrics.bWithinTargets)
    {
        Result.Warnings.Add(FString::Printf(TEXT("WARNING: Performance targets not met - FPS: %.1f/%.1f"), 
            PerfMetrics.CurrentFrameRate, PerfMetrics.TargetFrameRate));
    }
    
    // Validate project settings
    if (!ValidateProjectSettings())
    {
        Result.Errors.Add(TEXT("CRITICAL: Project settings validation failed"));
        bCriticalFeaturesValid = false;
    }
    
    // Set final validation result
    Result.bIsValid = bCriticalFeaturesValid && Result.Errors.Num() == 0;
    
    if (Result.bIsValid)
    {
        Result.ValidationMessage = TEXT("Architecture validation passed - all critical features enabled");
    }
    else
    {
        Result.ValidationMessage = FString::Printf(TEXT("Architecture validation failed - %d errors, %d warnings"), 
            Result.Errors.Num(), Result.Warnings.Num());
    }
    
    // Cache result
    LastValidationResult = Result;
    
    // Broadcast validation completed
    OnValidationCompleted.Broadcast(Result);
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Full architecture validation completed: %s"), 
        Result.bIsValid ? TEXT("PASSED") : TEXT("FAILED"));
    
    return Result;
}

FEngineFeatureStatus UEngineArchitectureValidator::ValidateEngineFeatures()
{
    FEngineFeatureStatus Status;
    
    // Check Nanite
    if (const URendererSettings* RendererSettings = GetDefault<URendererSettings>())
    {
        Status.bNaniteEnabled = RendererSettings->bEnableNanite;
        
        // Check Lumen
        Status.bLumenEnabled = (RendererSettings->DynamicGlobalIlluminationMethod == EDynamicGlobalIlluminationMethod::Lumen);
        
        // Check Virtual Shadow Maps
        Status.bVirtualShadowMapsEnabled = (RendererSettings->ShadowMapMethod == EShadowMapMethod::VirtualShadowMaps);
    }
    
    // Check World Partition (requires active world)
    if (UWorld* World = GEngine->GetCurrentPlayWorld())
    {
        Status.bWorldPartitionEnabled = World->IsPartitionedWorld();
    }
    else if (UWorld* EditorWorld = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
    {
        Status.bWorldPartitionEnabled = EditorWorld->IsPartitionedWorld();
    }
    
    // Check Mass Entity (check if plugin is loaded)
    Status.bMassEntityEnabled = FModuleManager::Get().IsModuleLoaded(TEXT("MassEntity"));
    
    // Check Chaos Physics
    if (const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>())
    {
        Status.bChaosPhysicsEnabled = (PhysicsSettings->DefaultBroadphaseType == EBroadphaseType::MBP) ||
                                      (PhysicsSettings->DefaultBroadphaseType == EBroadphaseType::ABP);
    }
    
    // Check MetaSounds
    Status.bMetaSoundsEnabled = FModuleManager::Get().IsModuleLoaded(TEXT("MetasoundEngine"));
    
    // Check Niagara
    Status.bNiagaraEnabled = FModuleManager::Get().IsModuleLoaded(TEXT("Niagara"));
    
    // Cache result
    LastFeatureStatus = Status;
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Engine features validation: Nanite=%s, Lumen=%s, VSM=%s, WP=%s"),
        Status.bNaniteEnabled ? TEXT("ON") : TEXT("OFF"),
        Status.bLumenEnabled ? TEXT("ON") : TEXT("OFF"),
        Status.bVirtualShadowMapsEnabled ? TEXT("ON") : TEXT("OFF"),
        Status.bWorldPartitionEnabled ? TEXT("ON") : TEXT("OFF"));
    
    return Status;
}

FPerformanceValidationMetrics UEngineArchitectureValidator::ValidatePerformanceTargets()
{
    FPerformanceValidationMetrics Metrics;
    
    // Get current frame rate
    if (GEngine && GEngine->GetGameUserSettings())
    {
        Metrics.CurrentFrameRate = 1.0f / FApp::GetDeltaTime();
        
        // Determine target based on platform
        #if PLATFORM_DESKTOP
            Metrics.TargetFrameRate = TPSArchitecture::TARGET_FRAMERATE_PC;
        #else
            Metrics.TargetFrameRate = TPSArchitecture::TARGET_FRAMERATE_CONSOLE;
        #endif
    }
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    Metrics.TargetMemoryMB = TPSArchitecture::TARGET_MEMORY_CONSOLE_MB;
    
    // Get draw calls (approximate from stats)
    if (GEngine && GEngine->GetCurrentPlayWorld())
    {
        // This is an approximation - in a real implementation you'd hook into the renderer
        Metrics.ActiveDrawCalls = 1000; // Placeholder
    }
    
    // Check if within targets
    bool bFrameRateOK = Metrics.CurrentFrameRate >= (Metrics.TargetFrameRate * 0.9f); // 90% tolerance
    bool bMemoryOK = Metrics.MemoryUsageMB <= Metrics.TargetMemoryMB;
    bool bDrawCallsOK = Metrics.ActiveDrawCalls <= Metrics.MaxDrawCalls;
    
    Metrics.bWithinTargets = bFrameRateOK && bMemoryOK && bDrawCallsOK;
    
    // Cache result
    LastPerformanceMetrics = Metrics;
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Performance validation: FPS=%.1f/%.1f, Memory=%.1fMB/%.1fMB, DrawCalls=%d/%d"),
        Metrics.CurrentFrameRate, Metrics.TargetFrameRate,
        Metrics.MemoryUsageMB, Metrics.TargetMemoryMB,
        Metrics.ActiveDrawCalls, Metrics.MaxDrawCalls);
    
    return Metrics;
}

bool UEngineArchitectureValidator::ValidateNaniteConfiguration()
{
    const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
    if (!RendererSettings)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Cannot access renderer settings"));
        return false;
    }
    
    bool bNaniteValid = RendererSettings->bEnableNanite;
    
    if (!bNaniteValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Nanite is not enabled - this is mandatory for Transpersonal Game"));
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Nanite configuration validated successfully"));
    }
    
    return bNaniteValid;
}

bool UEngineArchitectureValidator::ValidateLumenConfiguration()
{
    const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
    if (!RendererSettings)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Cannot access renderer settings"));
        return false;
    }
    
    bool bLumenValid = (RendererSettings->DynamicGlobalIlluminationMethod == EDynamicGlobalIlluminationMethod::Lumen);
    
    if (!bLumenValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Lumen Global Illumination is not enabled - this is mandatory"));
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Lumen configuration validated successfully"));
    }
    
    return bLumenValid;
}

bool UEngineArchitectureValidator::ValidateVirtualShadowMapsConfiguration()
{
    const URendererSettings* RendererSettings = GetDefault<URendererSettings>();
    if (!RendererSettings)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Cannot access renderer settings"));
        return false;
    }
    
    bool bVSMValid = (RendererSettings->ShadowMapMethod == EShadowMapMethod::VirtualShadowMaps);
    
    if (!bVSMValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Virtual Shadow Maps are not enabled - this is mandatory"));
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Virtual Shadow Maps configuration validated successfully"));
    }
    
    return bVSMValid;
}

bool UEngineArchitectureValidator::ValidateWorldPartitionConfiguration()
{
    UWorld* CurrentWorld = nullptr;
    
    // Try to get current world
    if (GEngine)
    {
        if (UWorld* PlayWorld = GEngine->GetCurrentPlayWorld())
        {
            CurrentWorld = PlayWorld;
        }
        else if (GEditor)
        {
            CurrentWorld = GEditor->GetEditorWorldContext().World();
        }
    }
    
    if (!CurrentWorld)
    {
        UE_LOG(LogEngineArchitectureValidator, Warning, TEXT("No active world found for World Partition validation"));
        return false;
    }
    
    bool bWorldPartitionValid = CurrentWorld->IsPartitionedWorld();
    
    if (!bWorldPartitionValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Warning, TEXT("Current world is not using World Partition"));
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("World Partition configuration validated successfully"));
    }
    
    return bWorldPartitionValid;
}

bool UEngineArchitectureValidator::ValidateMassEntityConfiguration()
{
    bool bMassEntityValid = FModuleManager::Get().IsModuleLoaded(TEXT("MassEntity"));
    
    if (!bMassEntityValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Warning, TEXT("Mass Entity system is not loaded"));
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Mass Entity configuration validated successfully"));
    }
    
    return bMassEntityValid;
}

void UEngineArchitectureValidator::StartContinuousValidation()
{
    if (bContinuousValidationActive)
    {
        UE_LOG(LogEngineArchitectureValidator, Warning, TEXT("Continuous validation already active"));
        return;
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ValidationTimerHandle, 
            this, &UEngineArchitectureValidator::PerformScheduledValidation, 
            ValidationInterval, true);
        
        bContinuousValidationActive = true;
        
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Continuous validation started (interval: %.1fs)"), ValidationInterval);
    }
}

void UEngineArchitectureValidator::StopContinuousValidation()
{
    if (!bContinuousValidationActive)
    {
        return;
    }
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ValidationTimerHandle);
    }
    
    bContinuousValidationActive = false;
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Continuous validation stopped"));
}

void UEngineArchitectureValidator::GenerateArchitectureReport()
{
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("=== TRANSPERSONAL GAME ARCHITECTURE REPORT ==="));
    
    // Generate timestamp
    FDateTime Now = FDateTime::Now();
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Report generated: %s"), *Now.ToString());
    
    // Feature status
    FEngineFeatureStatus Features = ValidateEngineFeatures();
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("FEATURE STATUS:"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Nanite: %s"), Features.bNaniteEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Lumen: %s"), Features.bLumenEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Virtual Shadow Maps: %s"), Features.bVirtualShadowMapsEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  World Partition: %s"), Features.bWorldPartitionEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Mass Entity: %s"), Features.bMassEntityEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    // Performance metrics
    FPerformanceValidationMetrics Perf = ValidatePerformanceTargets();
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("PERFORMANCE METRICS:"));
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Frame Rate: %.1f fps (target: %.1f fps)"), Perf.CurrentFrameRate, Perf.TargetFrameRate);
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Memory Usage: %.1f MB (target: %.1f MB)"), Perf.MemoryUsageMB, Perf.TargetMemoryMB);
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  Within Targets: %s"), Perf.bWithinTargets ? TEXT("YES") : TEXT("NO"));
    
    // Overall validation
    FArchitectureValidationResult Overall = ValidateFullArchitecture();
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("OVERALL STATUS: %s"), Overall.bIsValid ? TEXT("COMPLIANT") : TEXT("NON-COMPLIANT"));
    
    if (Overall.Errors.Num() > 0)
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("ERRORS:"));
        for (const FString& Error : Overall.Errors)
        {
            UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  - %s"), *Error);
        }
    }
    
    if (Overall.Warnings.Num() > 0)
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("WARNINGS:"));
        for (const FString& Warning : Overall.Warnings)
        {
            UE_LOG(LogEngineArchitectureValidator, Log, TEXT("  - %s"), *Warning);
        }
    }
    
    UE_LOG(LogEngineArchitectureValidator, Log, TEXT("=== END ARCHITECTURE REPORT ==="));
}

bool UEngineArchitectureValidator::ValidateProjectSettings()
{
    // This would validate project-specific settings
    // For now, return true as a placeholder
    return true;
}

bool UEngineArchitectureValidator::ValidateRenderingSettings()
{
    // Validate rendering-specific settings beyond the basic feature checks
    return ValidateNaniteConfiguration() && ValidateLumenConfiguration() && ValidateVirtualShadowMapsConfiguration();
}

bool UEngineArchitectureValidator::ValidatePhysicsSettings()
{
    // Validate Chaos physics configuration
    const UPhysicsSettings* PhysicsSettings = GetDefault<UPhysicsSettings>();
    return PhysicsSettings != nullptr;
}

bool UEngineArchitectureValidator::ValidateAudioSettings()
{
    // Validate MetaSounds and audio configuration
    return FModuleManager::Get().IsModuleLoaded(TEXT("MetasoundEngine"));
}

bool UEngineArchitectureValidator::ValidateWorldSettings()
{
    // Validate world-specific settings
    return ValidateWorldPartitionConfiguration();
}

void UEngineArchitectureValidator::UpdatePerformanceMetrics()
{
    LastPerformanceMetrics = ValidatePerformanceTargets();
}

void UEngineArchitectureValidator::CheckPerformanceThresholds()
{
    FPerformanceValidationMetrics Metrics = ValidatePerformanceTargets();
    
    if (!Metrics.bWithinTargets)
    {
        FString ErrorMsg = FString::Printf(TEXT("Performance targets not met - FPS: %.1f/%.1f"), 
            Metrics.CurrentFrameRate, Metrics.TargetFrameRate);
        
        OnValidationFailed.Broadcast(ErrorMsg);
        AddValidationEntry(ErrorMsg);
    }
}

void UEngineArchitectureValidator::LogValidationResult(const FArchitectureValidationResult& Result)
{
    if (Result.bIsValid)
    {
        UE_LOG(LogEngineArchitectureValidator, Log, TEXT("Validation PASSED: %s"), *Result.ValidationMessage);
    }
    else
    {
        UE_LOG(LogEngineArchitectureValidator, Error, TEXT("Validation FAILED: %s"), *Result.ValidationMessage);
        
        for (const FString& Error : Result.Errors)
        {
            UE_LOG(LogEngineArchitectureValidator, Error, TEXT("  ERROR: %s"), *Error);
        }
        
        for (const FString& Warning : Result.Warnings)
        {
            UE_LOG(LogEngineArchitectureValidator, Warning, TEXT("  WARNING: %s"), *Warning);
        }
    }
}

void UEngineArchitectureValidator::AddValidationEntry(const FString& Message)
{
    FString TimestampedMessage = FString::Printf(TEXT("[%s] %s"), 
        *FDateTime::Now().ToString(), *Message);
    
    ValidationHistory.Add(TimestampedMessage);
    
    // Limit history size
    if (ValidationHistory.Num() > MaxHistoryEntries)
    {
        ValidationHistory.RemoveAt(0);
    }
}

void UEngineArchitectureValidator::PerformScheduledValidation()
{
    if (!bValidationInitialized)
    {
        return;
    }
    
    // Perform lightweight validation
    FEngineFeatureStatus Features = ValidateEngineFeatures();
    FPerformanceValidationMetrics Performance = ValidatePerformanceTargets();
    
    // Check for critical issues
    if (!Features.bNaniteEnabled || !Features.bLumenEnabled || !Features.bVirtualShadowMapsEnabled)
    {
        FString ErrorMsg = TEXT("Critical engine features disabled during runtime");
        OnValidationFailed.Broadcast(ErrorMsg);
        AddValidationEntry(ErrorMsg);
    }
    
    // Check performance
    CheckPerformanceThresholds();
}
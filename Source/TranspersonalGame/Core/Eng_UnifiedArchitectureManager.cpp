#include "Eng_UnifiedArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"

UEng_UnifiedArchitectureManager::UEng_UnifiedArchitectureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Initialize architecture state
    CurrentArchitectureState = EEng_ArchitectureState::Initializing;
    ArchitectureVersion = TEXT("2.0.0");
    
    // Performance monitoring
    bEnablePerformanceMonitoring = true;
    PerformanceCheckInterval = 1.0f;
    MaxAllowedFrameTime = 16.67f; // 60 FPS target
    
    // System validation
    bEnableSystemValidation = true;
    ValidationCheckInterval = 5.0f;
    
    // Initialize timers
    LastPerformanceCheck = 0.0f;
    LastValidationCheck = 0.0f;
    
    // Architecture compliance
    bEnforceArchitecturalCompliance = true;
    ComplianceLevel = EEng_ComplianceLevel::Strict;
}

void UEng_UnifiedArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize architecture systems
    InitializeArchitectureSystems();
    
    // Validate initial state
    ValidateArchitectureIntegrity();
    
    // Set state to active
    CurrentArchitectureState = EEng_ArchitectureState::Active;
    
    UE_LOG(LogTemp, Log, TEXT("Unified Architecture Manager initialized - Version: %s"), *ArchitectureVersion);
}

void UEng_UnifiedArchitectureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Performance monitoring
    if (bEnablePerformanceMonitoring && CurrentTime - LastPerformanceCheck >= PerformanceCheckInterval)
    {
        MonitorSystemPerformance();
        LastPerformanceCheck = CurrentTime;
    }
    
    // System validation
    if (bEnableSystemValidation && CurrentTime - LastValidationCheck >= ValidationCheckInterval)
    {
        ValidateSystemIntegrity();
        LastValidationCheck = CurrentTime;
    }
    
    // Update architecture state
    UpdateArchitectureState(DeltaTime);
}

void UEng_UnifiedArchitectureManager::InitializeArchitectureSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing architecture systems..."));
    
    // Initialize core systems registry
    CoreSystems.Empty();
    
    // Register essential systems
    RegisterCoreSystem(TEXT("PhysicsManager"), EEng_SystemPriority::Critical);
    RegisterCoreSystem(TEXT("BiomeManager"), EEng_SystemPriority::High);
    RegisterCoreSystem(TEXT("CharacterSystem"), EEng_SystemPriority::High);
    RegisterCoreSystem(TEXT("CombatSystem"), EEng_SystemPriority::Medium);
    RegisterCoreSystem(TEXT("AudioSystem"), EEng_SystemPriority::Medium);
    RegisterCoreSystem(TEXT("VFXSystem"), EEng_SystemPriority::Low);
    
    // Initialize performance metrics
    PerformanceMetrics.Empty();
    
    // Initialize validation results
    ValidationResults.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture systems initialized - %d core systems registered"), CoreSystems.Num());
}

void UEng_UnifiedArchitectureManager::RegisterCoreSystem(const FString& SystemName, EEng_SystemPriority Priority)
{
    FEng_CoreSystemInfo SystemInfo;
    SystemInfo.SystemName = SystemName;
    SystemInfo.Priority = Priority;
    SystemInfo.bIsActive = true;
    SystemInfo.LastUpdateTime = GetWorld()->GetTimeSeconds();
    SystemInfo.PerformanceRating = 1.0f;
    
    CoreSystems.Add(SystemName, SystemInfo);
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Registered core system: %s (Priority: %d)"), *SystemName, (int32)Priority);
}

void UEng_UnifiedArchitectureManager::ValidateArchitectureIntegrity()
{
    UE_LOG(LogTemp, Log, TEXT("Validating architecture integrity..."));
    
    int32 PassedChecks = 0;
    int32 TotalChecks = 0;
    
    // Validate core systems
    for (const auto& SystemPair : CoreSystems)
    {
        TotalChecks++;
        if (ValidateCoreSystem(SystemPair.Value))
        {
            PassedChecks++;
        }
    }
    
    // Validate system dependencies
    TotalChecks++;
    if (ValidateSystemDependencies())
    {
        PassedChecks++;
    }
    
    // Validate performance constraints
    TotalChecks++;
    if (ValidatePerformanceConstraints())
    {
        PassedChecks++;
    }
    
    // Calculate integrity score
    float IntegrityScore = TotalChecks > 0 ? (float)PassedChecks / (float)TotalChecks : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Architecture integrity validation complete - Score: %.2f (%d/%d checks passed)"), 
           IntegrityScore, PassedChecks, TotalChecks);
    
    // Update architecture state based on integrity
    if (IntegrityScore >= 0.9f)
    {
        CurrentArchitectureState = EEng_ArchitectureState::Active;
    }
    else if (IntegrityScore >= 0.7f)
    {
        CurrentArchitectureState = EEng_ArchitectureState::Warning;
    }
    else
    {
        CurrentArchitectureState = EEng_ArchitectureState::Error;
    }
}

bool UEng_UnifiedArchitectureManager::ValidateCoreSystem(const FEng_CoreSystemInfo& SystemInfo)
{
    // Check if system is responsive
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceUpdate = CurrentTime - SystemInfo.LastUpdateTime;
    
    if (TimeSinceUpdate > 10.0f) // System hasn't updated in 10 seconds
    {
        UE_LOG(LogTemp, Warning, TEXT("Core system '%s' appears unresponsive (%.2fs since last update)"), 
               *SystemInfo.SystemName, TimeSinceUpdate);
        return false;
    }
    
    // Check performance rating
    if (SystemInfo.PerformanceRating < 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core system '%s' has poor performance rating (%.2f)"), 
               *SystemInfo.SystemName, SystemInfo.PerformanceRating);
        return false;
    }
    
    return true;
}

bool UEng_UnifiedArchitectureManager::ValidateSystemDependencies()
{
    // Check critical dependencies
    bool bPhysicsActive = CoreSystems.Contains(TEXT("PhysicsManager")) && 
                         CoreSystems[TEXT("PhysicsManager")].bIsActive;
    
    bool bCharacterActive = CoreSystems.Contains(TEXT("CharacterSystem")) && 
                           CoreSystems[TEXT("CharacterSystem")].bIsActive;
    
    if (!bPhysicsActive)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical dependency missing: PhysicsManager"));
        return false;
    }
    
    if (!bCharacterActive)
    {
        UE_LOG(LogTemp, Error, TEXT("Critical dependency missing: CharacterSystem"));
        return false;
    }
    
    return true;
}

bool UEng_UnifiedArchitectureManager::ValidatePerformanceConstraints()
{
    // Check frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    
    if (CurrentFrameTime > MaxAllowedFrameTime)
    {
        UE_LOG(LogTemp, Warning, TEXT("Frame time constraint violated: %.2fms (max: %.2fms)"), 
               CurrentFrameTime, MaxAllowedFrameTime);
        return false;
    }
    
    return true;
}

void UEng_UnifiedArchitectureManager::MonitorSystemPerformance()
{
    // Monitor frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f;
    
    FEng_PerformanceMetric FrameTimeMetric;
    FrameTimeMetric.MetricName = TEXT("FrameTime");
    FrameTimeMetric.Value = CurrentFrameTime;
    FrameTimeMetric.Timestamp = GetWorld()->GetTimeSeconds();
    FrameTimeMetric.bIsWithinLimits = CurrentFrameTime <= MaxAllowedFrameTime;
    
    PerformanceMetrics.Add(FrameTimeMetric);
    
    // Limit history size
    if (PerformanceMetrics.Num() > 100)
    {
        PerformanceMetrics.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Performance monitoring - Frame time: %.2fms"), CurrentFrameTime);
}

void UEng_UnifiedArchitectureManager::ValidateSystemIntegrity()
{
    // Update system status
    for (auto& SystemPair : CoreSystems)
    {
        FEng_CoreSystemInfo& SystemInfo = SystemPair.Value;
        
        // Update performance rating based on recent metrics
        UpdateSystemPerformanceRating(SystemInfo);
        
        // Update last update time (simplified - in real implementation would check actual system)
        SystemInfo.LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("System integrity validation completed"));
}

void UEng_UnifiedArchitectureManager::UpdateSystemPerformanceRating(FEng_CoreSystemInfo& SystemInfo)
{
    // Simplified performance rating calculation
    // In a real implementation, this would analyze actual system metrics
    
    float BaseRating = 1.0f;
    
    // Check if system is critical and adjust rating
    if (SystemInfo.Priority == EEng_SystemPriority::Critical)
    {
        BaseRating = 0.95f; // Critical systems start with high rating
    }
    else if (SystemInfo.Priority == EEng_SystemPriority::High)
    {
        BaseRating = 0.85f;
    }
    else
    {
        BaseRating = 0.75f;
    }
    
    // Apply some variation based on current performance
    float PerformanceVariation = FMath::RandRange(-0.1f, 0.1f);
    SystemInfo.PerformanceRating = FMath::Clamp(BaseRating + PerformanceVariation, 0.0f, 1.0f);
}

void UEng_UnifiedArchitectureManager::UpdateArchitectureState(float DeltaTime)
{
    // State-specific updates
    switch (CurrentArchitectureState)
    {
        case EEng_ArchitectureState::Initializing:
            // Should not be in this state during tick
            CurrentArchitectureState = EEng_ArchitectureState::Active;
            break;
            
        case EEng_ArchitectureState::Active:
            // Normal operation - no special handling needed
            break;
            
        case EEng_ArchitectureState::Warning:
            // Log warning state
            UE_LOG(LogTemp, Warning, TEXT("Architecture in warning state - monitoring closely"));
            break;
            
        case EEng_ArchitectureState::Error:
            // Log error state
            UE_LOG(LogTemp, Error, TEXT("Architecture in error state - intervention required"));
            break;
            
        case EEng_ArchitectureState::Shutdown:
            // Prepare for shutdown
            break;
    }
}

FString UEng_UnifiedArchitectureManager::GetArchitectureStatusReport() const
{
    FString Report;
    Report += FString::Printf(TEXT("=== ARCHITECTURE STATUS REPORT ===\n"));
    Report += FString::Printf(TEXT("Version: %s\n"), *ArchitectureVersion);
    Report += FString::Printf(TEXT("State: %d\n"), (int32)CurrentArchitectureState);
    Report += FString::Printf(TEXT("Core Systems: %d\n"), CoreSystems.Num());
    Report += FString::Printf(TEXT("Performance Metrics: %d\n"), PerformanceMetrics.Num());
    Report += FString::Printf(TEXT("Compliance Level: %d\n"), (int32)ComplianceLevel);
    
    return Report;
}

void UEng_UnifiedArchitectureManager::SetComplianceLevel(EEng_ComplianceLevel NewLevel)
{
    ComplianceLevel = NewLevel;
    UE_LOG(LogTemp, Log, TEXT("Architecture compliance level set to: %d"), (int32)NewLevel);
}
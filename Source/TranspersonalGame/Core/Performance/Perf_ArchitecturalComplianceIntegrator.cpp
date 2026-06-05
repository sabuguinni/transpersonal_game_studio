#include "Perf_ArchitecturalComplianceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UPerf_ArchitecturalComplianceIntegrator::UPerf_ArchitecturalComplianceIntegrator()
{
    OptimizationState = EPerf_ComplianceOptimizationState::Optimal;
    bIsMonitoringActive = false;
    LastOptimizationTime = 0.0f;
    ComplianceSystem = nullptr;
    AverageComplianceCheckTime = 0.0f;
    AverageValidationTime = 0.0f;
    ComplianceCheckStartTime = 0.0;
    ValidationStartTime = 0.0;
    GovernanceIntegrationStartTime = 0.0;
}

void UPerf_ArchitecturalComplianceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Architectural Compliance Integrator initialized"));
    
    // Initialize performance metrics
    ResetPerformanceMetrics();
    
    // Set initial optimization state
    OptimizationState = EPerf_ComplianceOptimizationState::Monitoring;
    
    // Start monitoring after a brief delay to allow other systems to initialize
    if (UWorld* World = GetWorld())
    {
        FTimerHandle InitializationTimer;
        World->GetTimerManager().SetTimer(InitializationTimer, this, 
            &UPerf_ArchitecturalComplianceIntegrator::IntegrateWithComplianceSystem, 1.0f, false);
    }
}

void UPerf_ArchitecturalComplianceIntegrator::Deinitialize()
{
    StopCompliancePerformanceMonitoring();
    ComplianceSystem = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Architectural Compliance Integrator deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_ArchitecturalComplianceIntegrator::StartCompliancePerformanceMonitoring()
{
    if (bIsMonitoringActive)
    {
        return;
    }
    
    bIsMonitoringActive = true;
    OptimizationState = EPerf_ComplianceOptimizationState::Monitoring;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Started compliance performance monitoring"));
    
    // Start periodic performance updates
    if (UWorld* World = GetWorld())
    {
        FTimerHandle MonitoringTimer;
        World->GetTimerManager().SetTimer(MonitoringTimer, [this]()
        {
            if (bIsMonitoringActive)
            {
                UpdateCompliancePerformanceMetrics(0.1f); // Update every 100ms
            }
        }, 0.1f, true);
    }
}

void UPerf_ArchitecturalComplianceIntegrator::StopCompliancePerformanceMonitoring()
{
    bIsMonitoringActive = false;
    OptimizationState = EPerf_ComplianceOptimizationState::Disabled;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Stopped compliance performance monitoring"));
}

void UPerf_ArchitecturalComplianceIntegrator::UpdateCompliancePerformanceMetrics(float DeltaTime)
{
    if (!bIsMonitoringActive || !ComplianceSystem)
    {
        return;
    }
    
    // Update compliance check performance
    float CurrentComplianceTime = AnalyzeComplianceCheckPerformance();
    ComplianceMetrics.ComplianceCheckTime = CurrentComplianceTime;
    
    // Update governance integration performance
    float CurrentGovernanceTime = AnalyzeGovernanceIntegrationPerformance();
    ComplianceMetrics.GovernanceIntegrationTime = CurrentGovernanceTime;
    
    // Update performance validation metrics
    ComplianceMetrics.PerformanceValidationTime = ValidationStartTime > 0.0 ? 
        FPlatformTime::Seconds() - ValidationStartTime : 0.0f;
    
    // Update active checks count
    ComplianceMetrics.ActiveComplianceChecks = ComplianceCheckTimes.Num();
    
    // Calculate average compliance score
    ComplianceMetrics.AverageComplianceScore = GetCompliancePerformanceScore();
    
    // Update optimization state based on performance
    UpdateOptimizationState();
    
    // Perform adaptive optimization if enabled
    if (OptimizationSettings.bEnableAdaptiveCompliance)
    {
        PerformAdaptiveOptimization();
    }
    
    // Apply performance throttling if necessary
    if (OptimizationSettings.bEnablePerformanceThrottling && 
        OptimizationState == EPerf_ComplianceOptimizationState::Critical)
    {
        ApplyPerformanceThrottling();
    }
}

void UPerf_ArchitecturalComplianceIntegrator::IntegrateWithComplianceSystem()
{
    GovernanceIntegrationStartTime = FPlatformTime::Seconds();
    
    // Find the compliance system
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        ComplianceSystem = GameInstance->GetSubsystem<UCore_ArchitecturalCompliance>();
        
        if (ComplianceSystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Successfully integrated with Architectural Compliance system"));
            
            // Start performance monitoring
            StartCompliancePerformanceMonitoring();
            
            // Register performance callbacks with compliance system
            // This would integrate with the compliance system's performance reporting
            ComplianceMetrics.GovernanceIntegrationTime = FPlatformTime::Seconds() - GovernanceIntegrationStartTime;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Failed to find Architectural Compliance system"));
            OptimizationState = EPerf_ComplianceOptimizationState::Disabled;
        }
    }
}

void UPerf_ArchitecturalComplianceIntegrator::ValidateCompliancePerformance()
{
    if (!ComplianceSystem)
    {
        return;
    }
    
    ValidationStartTime = FPlatformTime::Seconds();
    
    // Validate compliance check performance
    bool bComplianceOptimal = ComplianceMetrics.ComplianceCheckTime <= OptimizationSettings.MaxComplianceCheckTime;
    bool bValidationOptimal = ComplianceMetrics.PerformanceValidationTime <= OptimizationSettings.MaxPerformanceValidationTime;
    bool bGovernanceOptimal = ComplianceMetrics.GovernanceIntegrationTime <= OptimizationSettings.MaxGovernanceIntegrationTime;
    
    if (!bComplianceOptimal)
    {
        ComplianceMetrics.PerformanceViolations++;
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Compliance check time violation: %f ms"), 
               ComplianceMetrics.ComplianceCheckTime * 1000.0f);
    }
    
    if (!bValidationOptimal)
    {
        ComplianceMetrics.PerformanceViolations++;
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Performance validation time violation: %f ms"), 
               ComplianceMetrics.PerformanceValidationTime * 1000.0f);
    }
    
    if (!bGovernanceOptimal)
    {
        ComplianceMetrics.PerformanceViolations++;
        UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Governance integration time violation: %f ms"), 
               ComplianceMetrics.GovernanceIntegrationTime * 1000.0f);
    }
    
    // Update validation time
    ComplianceMetrics.PerformanceValidationTime = FPlatformTime::Seconds() - ValidationStartTime;
}

void UPerf_ArchitecturalComplianceIntegrator::OptimizeComplianceOperations()
{
    if (!ComplianceSystem || OptimizationState == EPerf_ComplianceOptimizationState::Disabled)
    {
        return;
    }
    
    OptimizationState = EPerf_ComplianceOptimizationState::Optimizing;
    LastOptimizationTime = FPlatformTime::Seconds();
    
    // Optimize compliance check frequency
    if (ComplianceMetrics.ComplianceCheckTime > OptimizationSettings.MaxComplianceCheckTime)
    {
        // Reduce compliance check frequency
        int32 NewCheckLimit = FMath::Max(1, OptimizationSettings.MaxConcurrentComplianceChecks - 2);
        OptimizationSettings.MaxConcurrentComplianceChecks = NewCheckLimit;
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Reduced concurrent compliance checks to %d"), NewCheckLimit);
    }
    
    // Optimize performance validation
    if (ComplianceMetrics.PerformanceValidationTime > OptimizationSettings.MaxPerformanceValidationTime)
    {
        // Increase validation time threshold slightly
        OptimizationSettings.MaxPerformanceValidationTime = FMath::Min(5.0f, 
            OptimizationSettings.MaxPerformanceValidationTime * 1.1f);
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Adjusted validation time threshold to %f ms"), 
               OptimizationSettings.MaxPerformanceValidationTime * 1000.0f);
    }
    
    // Reset violation counter after optimization
    ComplianceMetrics.PerformanceViolations = FMath::Max(0, ComplianceMetrics.PerformanceViolations - 1);
    
    OptimizationState = EPerf_ComplianceOptimizationState::Monitoring;
}

float UPerf_ArchitecturalComplianceIntegrator::AnalyzeComplianceCheckPerformance()
{
    if (ComplianceCheckTimes.Num() == 0)
    {
        return 0.0f;
    }
    
    // Calculate average compliance check time
    float TotalTime = 0.0f;
    for (float CheckTime : ComplianceCheckTimes)
    {
        TotalTime += CheckTime;
    }
    
    AverageComplianceCheckTime = TotalTime / ComplianceCheckTimes.Num();
    
    // Keep only recent measurements (last 100)
    if (ComplianceCheckTimes.Num() > 100)
    {
        ComplianceCheckTimes.RemoveAt(0, ComplianceCheckTimes.Num() - 100);
    }
    
    return AverageComplianceCheckTime;
}

float UPerf_ArchitecturalComplianceIntegrator::AnalyzeGovernanceIntegrationPerformance()
{
    // Analyze governance integration performance
    if (ComplianceSystem)
    {
        // This would integrate with actual governance system performance metrics
        return ComplianceMetrics.GovernanceIntegrationTime;
    }
    
    return 0.0f;
}

bool UPerf_ArchitecturalComplianceIntegrator::IsCompliancePerformanceOptimal() const
{
    bool bComplianceOptimal = ComplianceMetrics.ComplianceCheckTime <= OPTIMAL_COMPLIANCE_TIME;
    bool bValidationOptimal = ComplianceMetrics.PerformanceValidationTime <= OptimizationSettings.MaxPerformanceValidationTime;
    bool bGovernanceOptimal = ComplianceMetrics.GovernanceIntegrationTime <= OptimizationSettings.MaxGovernanceIntegrationTime;
    bool bScoreOptimal = ComplianceMetrics.AverageComplianceScore >= OptimizationSettings.MinComplianceScore;
    
    return bComplianceOptimal && bValidationOptimal && bGovernanceOptimal && bScoreOptimal;
}

void UPerf_ArchitecturalComplianceIntegrator::SetComplianceOptimizationLevel(int32 Level)
{
    Level = FMath::Clamp(Level, 0, 5);
    
    switch (Level)
    {
        case 0: // Minimal optimization
            OptimizationSettings.MaxComplianceCheckTime = 5.0f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 20;
            break;
        case 1: // Low optimization
            OptimizationSettings.MaxComplianceCheckTime = 3.0f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 15;
            break;
        case 2: // Medium optimization
            OptimizationSettings.MaxComplianceCheckTime = 2.0f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 10;
            break;
        case 3: // High optimization
            OptimizationSettings.MaxComplianceCheckTime = 1.5f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 8;
            break;
        case 4: // Maximum optimization
            OptimizationSettings.MaxComplianceCheckTime = 1.0f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 5;
            break;
        case 5: // Ultra optimization
            OptimizationSettings.MaxComplianceCheckTime = 0.5f;
            OptimizationSettings.MaxConcurrentComplianceChecks = 3;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Set compliance optimization level to %d"), Level);
}

void UPerf_ArchitecturalComplianceIntegrator::EnableAdaptiveComplianceOptimization(bool bEnable)
{
    OptimizationSettings.bEnableAdaptiveCompliance = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Adaptive compliance optimization %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_ArchitecturalComplianceIntegrator::ThrottleComplianceOperations(float ThrottleAmount)
{
    ThrottleAmount = FMath::Clamp(ThrottleAmount, 0.1f, 1.0f);
    
    // Apply throttling to compliance operations
    OptimizationSettings.MaxConcurrentComplianceChecks = FMath::Max(1, 
        FMath::RoundToInt(OptimizationSettings.MaxConcurrentComplianceChecks * ThrottleAmount));
    
    OptimizationSettings.MaxComplianceCheckTime *= (2.0f - ThrottleAmount); // Increase allowed time
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Applied compliance throttling: %f"), ThrottleAmount);
}

float UPerf_ArchitecturalComplianceIntegrator::GetCompliancePerformanceScore() const
{
    if (!ComplianceSystem)
    {
        return 0.0f;
    }
    
    // Calculate performance score based on metrics
    float ComplianceScore = (ComplianceMetrics.ComplianceCheckTime <= OPTIMAL_COMPLIANCE_TIME) ? 100.0f : 
                           (ComplianceMetrics.ComplianceCheckTime <= WARNING_COMPLIANCE_TIME) ? 75.0f : 
                           (ComplianceMetrics.ComplianceCheckTime <= CRITICAL_COMPLIANCE_TIME) ? 50.0f : 25.0f;
    
    float ValidationScore = (ComplianceMetrics.PerformanceValidationTime <= OptimizationSettings.MaxPerformanceValidationTime) ? 100.0f : 50.0f;
    float GovernanceScore = (ComplianceMetrics.GovernanceIntegrationTime <= OptimizationSettings.MaxGovernanceIntegrationTime) ? 100.0f : 50.0f;
    
    // Weight the scores
    float TotalScore = (ComplianceScore * 0.5f) + (ValidationScore * 0.3f) + (GovernanceScore * 0.2f);
    
    return FMath::Clamp(TotalScore, 0.0f, 100.0f);
}

void UPerf_ArchitecturalComplianceIntegrator::UpdateOptimizationState()
{
    if (!bIsMonitoringActive)
    {
        OptimizationState = EPerf_ComplianceOptimizationState::Disabled;
        return;
    }
    
    EPerf_ComplianceOptimizationState NewState = OptimizationState;
    
    if (ComplianceMetrics.ComplianceCheckTime <= OPTIMAL_COMPLIANCE_TIME && 
        ComplianceMetrics.PerformanceViolations == 0)
    {
        NewState = EPerf_ComplianceOptimizationState::Optimal;
    }
    else if (ComplianceMetrics.ComplianceCheckTime <= WARNING_COMPLIANCE_TIME)
    {
        NewState = EPerf_ComplianceOptimizationState::Monitoring;
    }
    else if (ComplianceMetrics.ComplianceCheckTime <= CRITICAL_COMPLIANCE_TIME)
    {
        NewState = EPerf_ComplianceOptimizationState::Optimizing;
    }
    else
    {
        NewState = EPerf_ComplianceOptimizationState::Critical;
    }
    
    if (NewState != OptimizationState)
    {
        OptimizationState = NewState;
        OnComplianceOptimizationStateChanged.Broadcast(OptimizationState);
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimizer: Compliance optimization state changed to %d"), 
               static_cast<int32>(OptimizationState));
    }
}

void UPerf_ArchitecturalComplianceIntegrator::PerformAdaptiveOptimization()
{
    if (OptimizationState == EPerf_ComplianceOptimizationState::Optimal)
    {
        return;
    }
    
    // Adaptive optimization based on current performance
    if (ComplianceMetrics.ComplianceCheckTime > OptimizationSettings.MaxComplianceCheckTime)
    {
        OptimizeComplianceOperations();
    }
    
    // Adjust optimization settings based on performance trends
    if (ComplianceMetrics.PerformanceViolations > 5)
    {
        SetComplianceOptimizationLevel(4); // High optimization
    }
    else if (ComplianceMetrics.PerformanceViolations > 2)
    {
        SetComplianceOptimizationLevel(2); // Medium optimization
    }
}

void UPerf_ArchitecturalComplianceIntegrator::ApplyPerformanceThrottling()
{
    if (OptimizationState != EPerf_ComplianceOptimizationState::Critical)
    {
        return;
    }
    
    // Apply aggressive throttling in critical state
    float ThrottleAmount = 0.5f; // 50% throttling
    ThrottleComplianceOperations(ThrottleAmount);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimizer: Applied critical performance throttling"));
}

void UPerf_ArchitecturalComplianceIntegrator::ResetPerformanceMetrics()
{
    ComplianceMetrics = FPerf_CompliancePerformanceMetrics();
    ComplianceCheckTimes.Empty();
    PerformanceValidationTimes.Empty();
    AverageComplianceCheckTime = 0.0f;
    AverageValidationTime = 0.0f;
}
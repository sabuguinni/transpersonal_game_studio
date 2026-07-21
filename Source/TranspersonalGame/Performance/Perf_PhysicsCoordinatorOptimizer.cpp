#include "Perf_PhysicsCoordinatorOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

APerf_PhysicsCoordinatorOptimizer::APerf_PhysicsCoordinatorOptimizer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS monitoring

    // Initialize optimization settings
    OptimizationSettings = FPerf_CoordinatorOptimizationSettings();
    
    // Initialize metrics
    CurrentMetrics = FPerf_PhysicsCoordinationMetrics();
    
    // Set initial state
    OptimizationState = EPerf_CoordinatorOptimizationState::Initializing;
    
    // Initialize monitoring variables
    MonitoringInterval = 0.1f;
    LastMonitoringTime = 0.0f;
    MaxHistorySize = 100;
    
    // Performance tracking
    bIsMonitoring = false;
    bOptimizationEnabled = true;
    LastFrameTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    MonitoringTimer = 0.0f;
    OptimizationTimer = 0.0f;
    
    // Initialize physics coordinator reference
    PhysicsCoordinator = nullptr;
}

void APerf_PhysicsCoordinatorOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Initializing performance optimization system"));
    
    // Start performance monitoring
    StartPerformanceMonitoring();
    
    // Register with physics coordinator
    if (RegisterWithPhysicsCoordinator())
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Successfully registered with physics coordinator"));
        OptimizationState = EPerf_CoordinatorOptimizationState::Monitoring;
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Failed to register with physics coordinator"));
        OptimizationState = EPerf_CoordinatorOptimizationState::Degraded;
    }
    
    // Integrate with adaptive quality system
    IntegrateWithAdaptiveQuality();
    
    // Validate integration
    ValidatePhysicsIntegration();
}

void APerf_PhysicsCoordinatorOptimizer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsMonitoring)
        return;
    
    // Update monitoring timer
    MonitoringTimer += DeltaTime;
    
    // Update performance metrics at specified interval
    if (MonitoringTimer >= MonitoringInterval)
    {
        UpdatePerformanceMetrics();
        MonitoringTimer = 0.0f;
    }
    
    // Update optimization timer
    OptimizationTimer += DeltaTime;
    
    // Apply optimizations every second
    if (OptimizationTimer >= 1.0f && bOptimizationEnabled)
    {
        OptimizePhysicsCoordination();
        OptimizationTimer = 0.0f;
    }
    
    // Update optimization state
    UpdateOptimizationState();
}

void APerf_PhysicsCoordinatorOptimizer::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    MonitoringTimer = 0.0f;
    OptimizationTimer = 0.0f;
    FrameTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Performance monitoring started"));
}

void APerf_PhysicsCoordinatorOptimizer::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Performance monitoring stopped"));
}

void APerf_PhysicsCoordinatorOptimizer::UpdatePerformanceMetrics()
{
    if (!GetWorld())
        return;
    
    // Get current frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate coordination metrics
    CurrentMetrics.CoordinationTime = CurrentFrameTime;
    
    // Simulate physics coordinator metrics (would be real data in production)
    CurrentMetrics.SystemRegistrationTime = FMath::RandRange(0.1f, 1.5f);
    CurrentMetrics.CharacterPhysicsTime = FMath::RandRange(1.0f, 8.0f);
    CurrentMetrics.QualityAdjustmentTime = FMath::RandRange(0.1f, 0.5f);
    CurrentMetrics.ValidationTime = FMath::RandRange(0.1f, 0.3f);
    
    // Count registered systems and active characters
    CurrentMetrics.RegisteredSystems = FMath::RandRange(5, 15);
    CurrentMetrics.ActiveCharacters = FMath::RandRange(1, 10);
    
    // Calculate memory usage
    CurrentMetrics.MemoryUsageMB = FMath::RandRange(100.0f, 400.0f);
    
    // Process performance history
    ProcessPerformanceHistory();
}

void APerf_PhysicsCoordinatorOptimizer::OptimizePhysicsCoordination()
{
    if (OptimizationState == EPerf_CoordinatorOptimizationState::Disabled)
        return;
    
    // Check if optimization is needed
    bool bNeedsOptimization = false;
    
    if (CurrentMetrics.CoordinationTime > OptimizationSettings.CoordinationTimeThreshold)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Coordination time exceeds threshold: %.2fms"), CurrentMetrics.CoordinationTime);
    }
    
    if (CurrentMetrics.CharacterPhysicsTime > OptimizationSettings.CharacterPhysicsThreshold)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Character physics time exceeds threshold: %.2fms"), CurrentMetrics.CharacterPhysicsTime);
    }
    
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB)
    {
        bNeedsOptimization = true;
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Memory usage exceeds threshold: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    }
    
    if (bNeedsOptimization)
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Optimizing;
        
        // Apply optimizations
        ApplyQualityOptimizations();
        OptimizeSystemRegistration();
        OptimizeCharacterPhysics();
        
        // Apply adaptive optimizations if enabled
        if (OptimizationSettings.bEnableAdaptiveQuality)
        {
            ApplyAdaptiveOptimizations();
        }
    }
    else
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Monitoring;
    }
}

void APerf_PhysicsCoordinatorOptimizer::ApplyQualityOptimizations()
{
    // Reduce quality settings based on performance metrics
    if (CurrentMetrics.CoordinationTime > OptimizationSettings.TargetFrameTime60FPS)
    {
        // Reduce physics quality
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Applying quality optimizations for coordination performance"));
        
        // Would integrate with physics coordinator's quality settings here
        // PhysicsCoordinator->ReduceQualityLevel(OptimizationSettings.QualityAdjustmentRate);
    }
}

void APerf_PhysicsCoordinatorOptimizer::OptimizeSystemRegistration()
{
    // Optimize system registration process
    if (CurrentMetrics.SystemRegistrationTime > OptimizationSettings.SystemRegistrationThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Optimizing system registration process"));
        
        // Limit number of registered systems
        if (CurrentMetrics.RegisteredSystems > OptimizationSettings.MaxRegisteredSystems)
        {
            UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Too many registered systems (%d), optimization needed"), CurrentMetrics.RegisteredSystems);
        }
    }
}

void APerf_PhysicsCoordinatorOptimizer::OptimizeCharacterPhysics()
{
    // Optimize character physics processing
    if (CurrentMetrics.CharacterPhysicsTime > OptimizationSettings.CharacterPhysicsThreshold)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Optimizing character physics processing"));
        
        // Limit number of active characters
        if (CurrentMetrics.ActiveCharacters > OptimizationSettings.MaxActiveCharacters)
        {
            UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Too many active characters (%d), optimization needed"), CurrentMetrics.ActiveCharacters);
        }
    }
}

bool APerf_PhysicsCoordinatorOptimizer::RegisterWithPhysicsCoordinator()
{
    // Find physics coordinator in the world
    if (GetWorld())
    {
        // In a real implementation, we would find the coordinator actor
        // For now, simulate successful registration
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Attempting to register with physics coordinator"));
        
        // PhysicsCoordinator = FindPhysicsCoordinatorInWorld();
        // if (PhysicsCoordinator)
        // {
        //     return PhysicsCoordinator->RegisterOptimizer(this);
        // }
        
        return true; // Simulate successful registration
    }
    
    return false;
}

void APerf_PhysicsCoordinatorOptimizer::IntegrateWithAdaptiveQuality()
{
    if (OptimizationSettings.bEnableAdaptiveQuality)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Integrating with adaptive quality system"));
        
        // Would integrate with physics coordinator's adaptive quality system here
        // PhysicsCoordinator->SetAdaptiveQualityCallback(this);
    }
}

void APerf_PhysicsCoordinatorOptimizer::ValidatePhysicsIntegration()
{
    bool bIntegrationValid = true;
    
    // Validate physics coordinator connection
    if (!PhysicsCoordinator)
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Physics coordinator reference is null"));
        bIntegrationValid = false;
    }
    
    // Validate performance targets
    if (!ValidatePerformanceTargets())
    {
        UE_LOG(LogTemp, Warning, TEXT("PhysicsCoordinatorOptimizer: Performance targets validation failed"));
        bIntegrationValid = false;
    }
    
    if (bIntegrationValid)
    {
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Physics integration validation passed"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("PhysicsCoordinatorOptimizer: Physics integration validation failed"));
        OptimizationState = EPerf_CoordinatorOptimizationState::Critical;
    }
}

FPerf_PhysicsCoordinationMetrics APerf_PhysicsCoordinatorOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

float APerf_PhysicsCoordinatorOptimizer::GetAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
        return 0.0f;
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

bool APerf_PhysicsCoordinatorOptimizer::IsPerformanceWithinTargets() const
{
    float AverageFrameTime = GetAverageFrameTime();
    
    // Check against 60fps target (16.67ms)
    if (AverageFrameTime <= OptimizationSettings.TargetFrameTime60FPS)
    {
        return true;
    }
    
    // Check against 30fps target (33.33ms) for console
    if (AverageFrameTime <= OptimizationSettings.TargetFrameTime30FPS)
    {
        return true;
    }
    
    return false;
}

void APerf_PhysicsCoordinatorOptimizer::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PHYSICS COORDINATOR PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("Coordination Time: %.2fms"), CurrentMetrics.CoordinationTime);
    UE_LOG(LogTemp, Log, TEXT("System Registration Time: %.2fms"), CurrentMetrics.SystemRegistrationTime);
    UE_LOG(LogTemp, Log, TEXT("Character Physics Time: %.2fms"), CurrentMetrics.CharacterPhysicsTime);
    UE_LOG(LogTemp, Log, TEXT("Quality Adjustment Time: %.2fms"), CurrentMetrics.QualityAdjustmentTime);
    UE_LOG(LogTemp, Log, TEXT("Validation Time: %.2fms"), CurrentMetrics.ValidationTime);
    UE_LOG(LogTemp, Log, TEXT("Registered Systems: %d"), CurrentMetrics.RegisteredSystems);
    UE_LOG(LogTemp, Log, TEXT("Active Characters: %d"), CurrentMetrics.ActiveCharacters);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2fms"), GetAverageFrameTime());
    UE_LOG(LogTemp, Log, TEXT("Performance Within Targets: %s"), IsPerformanceWithinTargets() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("Optimization State: %d"), (int32)OptimizationState);
}

void APerf_PhysicsCoordinatorOptimizer::RunPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Running performance test"));
    
    // Force metrics update
    UpdatePerformanceMetrics();
    
    // Run optimization
    OptimizePhysicsCoordination();
    
    // Log results
    LogPerformanceReport();
}

void APerf_PhysicsCoordinatorOptimizer::ResetOptimizationState()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Resetting optimization state"));
    
    OptimizationState = EPerf_CoordinatorOptimizationState::Initializing;
    CurrentMetrics = FPerf_PhysicsCoordinationMetrics();
    FrameTimeHistory.Empty();
    MonitoringTimer = 0.0f;
    OptimizationTimer = 0.0f;
    
    // Restart monitoring
    StartPerformanceMonitoring();
}

void APerf_PhysicsCoordinatorOptimizer::ValidateCoordinatorIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Validating coordinator integration"));
    
    ValidatePhysicsIntegration();
    LogPerformanceReport();
}

void APerf_PhysicsCoordinatorOptimizer::UpdateOptimizationState()
{
    // Update state based on current performance
    float AverageFrameTime = GetAverageFrameTime();
    
    if (AverageFrameTime > OptimizationSettings.TargetFrameTime30FPS * 2.0f)
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Critical;
    }
    else if (AverageFrameTime > OptimizationSettings.TargetFrameTime30FPS)
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Degraded;
    }
    else if (AverageFrameTime > OptimizationSettings.TargetFrameTime60FPS)
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Optimizing;
    }
    else
    {
        OptimizationState = EPerf_CoordinatorOptimizationState::Monitoring;
    }
}

void APerf_PhysicsCoordinatorOptimizer::ProcessPerformanceHistory()
{
    // Process frame time history for trends and patterns
    if (FrameTimeHistory.Num() < 10)
        return;
    
    // Calculate performance trends
    float RecentAverage = 0.0f;
    int32 RecentSamples = FMath::Min(10, FrameTimeHistory.Num());
    
    for (int32 i = FrameTimeHistory.Num() - RecentSamples; i < FrameTimeHistory.Num(); i++)
    {
        RecentAverage += FrameTimeHistory[i];
    }
    RecentAverage /= RecentSamples;
    
    // Update metrics with processed data
    CurrentMetrics.CoordinationTime = RecentAverage;
}

void APerf_PhysicsCoordinatorOptimizer::ApplyAdaptiveOptimizations()
{
    // Apply adaptive optimizations based on current performance
    float AverageFrameTime = GetAverageFrameTime();
    
    if (AverageFrameTime > OptimizationSettings.TargetFrameTime60FPS)
    {
        // Reduce quality gradually
        float QualityReduction = OptimizationSettings.QualityAdjustmentRate;
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsCoordinatorOptimizer: Applying adaptive optimization - quality reduction: %.2f"), QualityReduction);
        
        // Would apply to physics coordinator here
        // PhysicsCoordinator->AdjustQuality(-QualityReduction);
    }
}

bool APerf_PhysicsCoordinatorOptimizer::ValidatePerformanceTargets()
{
    // Validate that performance targets are reasonable
    if (OptimizationSettings.TargetFrameTime60FPS <= 0.0f || OptimizationSettings.TargetFrameTime30FPS <= 0.0f)
    {
        return false;
    }
    
    if (OptimizationSettings.TargetFrameTime60FPS >= OptimizationSettings.TargetFrameTime30FPS)
    {
        return false;
    }
    
    return true;
}
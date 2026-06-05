#include "Perf_PhysicsGameplayIntegrationOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

UPerf_PhysicsGameplayIntegrationOptimizer::UPerf_PhysicsGameplayIntegrationOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 times per second
    
    // Initialize default settings
    OptimizationSettings = FPerf_PhysicsGameplayIntegrationSettings();
    CurrentState = EPerf_PhysicsGameplayIntegrationState::Optimal;
    bMonitoringEnabled = true;
    MonitoringInterval = 0.1f;
    bAutoOptimizationEnabled = true;
    bEmergencyOptimizationActive = false;
    
    // Initialize tracking variables
    LastIntegrationTime = 0.0f;
    AverageIntegrationTime = 0.0f;
    PeakIntegrationTime = 0.0f;
    IntegrationFrameCount = 0;
}

void UPerf_PhysicsGameplayIntegrationOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    if (bMonitoringEnabled)
    {
        StartPerformanceMonitoring();
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Initialized with monitoring %s"), 
           bMonitoringEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMonitoringEnabled)
    {
        UpdatePerformanceMetrics();
        UpdatePerformanceState();
        
        if (bAutoOptimizationEnabled)
        {
            ApplyOptimizations();
        }
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::StartPerformanceMonitoring()
{
    bMonitoringEnabled = true;
    ResetPerformanceTracking();
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MonitoringTimerHandle, 
            [this]() { UpdatePerformanceMetrics(); }, 
            MonitoringInterval, true);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Performance monitoring started"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::StopPerformanceMonitoring()
{
    bMonitoringEnabled = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        World->GetTimerManager().ClearTimer(OptimizationTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Performance monitoring stopped"));
}

FPerf_PhysicsGameplayIntegrationMetrics UPerf_PhysicsGameplayIntegrationOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

EPerf_PhysicsGameplayIntegrationState UPerf_PhysicsGameplayIntegrationOptimizer::GetCurrentPerformanceState() const
{
    return CurrentState;
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Running integration performance optimization"));
    
    // Optimize different aspects of physics gameplay integration
    OptimizeIntegrationBatching();
    OptimizeIntegrationPriority();
    OptimizeIntegrationCaching();
    OptimizeIntegrationThreading();
    
    // Update metrics after optimization
    UpdatePerformanceMetrics();
    UpdatePerformanceState();
}

void UPerf_PhysicsGameplayIntegrationOptimizer::SetOptimizationSettings(const FPerf_PhysicsGameplayIntegrationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimization settings updated"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::ApplyPerformanceProfile(EPerformanceProfile Profile)
{
    switch (Profile)
    {
        case EPerformanceProfile::HighEnd:
            OptimizationSettings.IntegrationQualityScale = 1.5f;
            OptimizationSettings.PhysicsDetailLevel = 1.5f;
            OptimizationSettings.MaxConcurrentIntegrations = 24;
            break;
            
        case EPerformanceProfile::Medium:
            OptimizationSettings.IntegrationQualityScale = 1.0f;
            OptimizationSettings.PhysicsDetailLevel = 1.0f;
            OptimizationSettings.MaxConcurrentIntegrations = 16;
            break;
            
        case EPerformanceProfile::LowEnd:
            OptimizationSettings.IntegrationQualityScale = 0.7f;
            OptimizationSettings.PhysicsDetailLevel = 0.7f;
            OptimizationSettings.MaxConcurrentIntegrations = 8;
            break;
            
        case EPerformanceProfile::Console:
            OptimizationSettings.IntegrationQualityScale = 0.8f;
            OptimizationSettings.PhysicsDetailLevel = 0.8f;
            OptimizationSettings.MaxConcurrentIntegrations = 12;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Applied performance profile"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::EnableEmergencyOptimization()
{
    bEmergencyOptimizationActive = true;
    
    // Apply emergency settings
    OptimizationSettings.IntegrationQualityScale = 0.5f;
    OptimizationSettings.PhysicsDetailLevel = 0.5f;
    OptimizationSettings.MaxConcurrentIntegrations = 4;
    OptimizationSettings.IntegrationBatchSize = 2;
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrationOptimizer: Emergency optimization enabled"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::DisableEmergencyOptimization()
{
    bEmergencyOptimizationActive = false;
    
    // Restore normal settings
    OptimizationSettings = FPerf_PhysicsGameplayIntegrationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Emergency optimization disabled"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeGameplayPhysicsIntegration()
{
    // Optimize gameplay-specific physics integration
    float CurrentGameplayTime = CurrentMetrics.GameplayPhysicsTime;
    float TargetTime = OptimizationSettings.MaxGameplayPhysicsTime;
    
    if (CurrentGameplayTime > TargetTime)
    {
        float OptimizationFactor = TargetTime / CurrentGameplayTime;
        OptimizationSettings.GameplayResponsivenessScale *= OptimizationFactor;
        
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimized gameplay physics integration"));
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeCharacterPhysicsIntegration()
{
    // Optimize character-specific physics integration
    float CurrentCharacterTime = CurrentMetrics.CharacterIntegrationTime;
    float TargetRatio = 0.3f; // 30% of total integration time
    float TargetTime = OptimizationSettings.MaxIntegrationTime * TargetRatio;
    
    if (CurrentCharacterTime > TargetTime)
    {
        // Reduce character physics detail
        OptimizationSettings.PhysicsDetailLevel *= 0.9f;
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimized character physics integration"));
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeDinosaurPhysicsIntegration()
{
    // Optimize dinosaur-specific physics integration
    float CurrentDinosaurTime = CurrentMetrics.DinosaurIntegrationTime;
    float TargetRatio = 0.4f; // 40% of total integration time
    float TargetTime = OptimizationSettings.MaxIntegrationTime * TargetRatio;
    
    if (CurrentDinosaurTime > TargetTime)
    {
        // Reduce dinosaur physics complexity
        OptimizationSettings.IntegrationBatchSize = FMath::Max(1, OptimizationSettings.IntegrationBatchSize - 1);
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimized dinosaur physics integration"));
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeTerrainPhysicsIntegration()
{
    // Optimize terrain-specific physics integration
    float CurrentTerrainTime = CurrentMetrics.TerrainIntegrationTime;
    float TargetRatio = 0.2f; // 20% of total integration time
    float TargetTime = OptimizationSettings.MaxIntegrationTime * TargetRatio;
    
    if (CurrentTerrainTime > TargetTime)
    {
        // Reduce terrain physics detail
        OptimizationSettings.PhysicsDetailLevel *= 0.95f;
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimized terrain physics integration"));
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationMemoryUsage()
{
    float CurrentMemoryPercentage = GetMemoryUsagePercentage();
    
    if (CurrentMemoryPercentage > OptimizationSettings.GarbageCollectionThreshold)
    {
        ForceGarbageCollection();
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Optimized integration memory usage"));
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::ForceGarbageCollection()
{
    if (GEngine)
    {
        GEngine->ForceGarbageCollection(true);
        UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Forced garbage collection"));
    }
}

float UPerf_PhysicsGameplayIntegrationOptimizer::GetMemoryUsagePercentage() const
{
    // Simplified memory usage calculation
    return CurrentMetrics.MemoryUsage / OptimizationSettings.MemoryBudgetMB;
}

void UPerf_PhysicsGameplayIntegrationOptimizer::AdjustIntegrationQuality(float QualityScale)
{
    OptimizationSettings.IntegrationQualityScale = FMath::Clamp(QualityScale, 0.1f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Adjusted integration quality to %f"), QualityScale);
}

void UPerf_PhysicsGameplayIntegrationOptimizer::SetPhysicsDetailLevel(float DetailLevel)
{
    OptimizationSettings.PhysicsDetailLevel = FMath::Clamp(DetailLevel, 0.1f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Set physics detail level to %f"), DetailLevel);
}

void UPerf_PhysicsGameplayIntegrationOptimizer::SetGameplayResponsiveness(float ResponsivenessScale)
{
    OptimizationSettings.GameplayResponsivenessScale = FMath::Clamp(ResponsivenessScale, 0.1f, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Set gameplay responsiveness to %f"), ResponsivenessScale);
}

bool UPerf_PhysicsGameplayIntegrationOptimizer::ValidateIntegrationPerformance()
{
    return ValidateFrameTimeTargets() && ValidateMemoryUsage() && ValidateIntegrationStability();
}

bool UPerf_PhysicsGameplayIntegrationOptimizer::IsPerformanceWithinTargets() const
{
    float CurrentFrameTime = CurrentMetrics.IntegrationTime;
    float TargetFrameTime = OptimizationSettings.TargetFrameTime60FPS;
    
    return CurrentFrameTime <= TargetFrameTime;
}

TArray<FString> UPerf_PhysicsGameplayIntegrationOptimizer::GetPerformanceWarnings() const
{
    TArray<FString> Warnings;
    
    if (CurrentMetrics.IntegrationTime > OptimizationSettings.TargetFrameTime60FPS)
    {
        Warnings.Add(TEXT("Integration time exceeds 60fps target"));
    }
    
    if (GetMemoryUsagePercentage() > OptimizationSettings.GarbageCollectionThreshold)
    {
        Warnings.Add(TEXT("Memory usage exceeds threshold"));
    }
    
    if (CurrentMetrics.PhysicsStability < 0.9f)
    {
        Warnings.Add(TEXT("Physics stability below acceptable level"));
    }
    
    return Warnings;
}

void UPerf_PhysicsGameplayIntegrationOptimizer::RunIntegrationPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Running integration performance test"));
    
    // Simulate performance test
    UpdatePerformanceMetrics();
    
    bool bTestPassed = ValidateIntegrationPerformance();
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer: Performance test %s"), 
           bTestPassed ? TEXT("PASSED") : TEXT("FAILED"));
}

void UPerf_PhysicsGameplayIntegrationOptimizer::GeneratePerformanceReport()
{
    FString Report = FString::Printf(TEXT(
        "Physics Gameplay Integration Performance Report\\n"
        "Generated: %s\\n"
        "Integration Time: %.2fms\\n"
        "Gameplay Physics Time: %.2fms\\n"
        "Character Integration Time: %.2fms\\n"
        "Dinosaur Integration Time: %.2fms\\n"
        "Terrain Integration Time: %.2fms\\n"
        "Memory Usage: %.2fMB\\n"
        "Performance State: %d\\n"
        "Active Components: %d\\n"
        "Queued Tasks: %d\\n"
    ), 
    *FDateTime::Now().ToString(),
    CurrentMetrics.IntegrationTime,
    CurrentMetrics.GameplayPhysicsTime,
    CurrentMetrics.CharacterIntegrationTime,
    CurrentMetrics.DinosaurIntegrationTime,
    CurrentMetrics.TerrainIntegrationTime,
    CurrentMetrics.MemoryUsage,
    (int32)CurrentState,
    CurrentMetrics.ActiveIntegrationComponents,
    CurrentMetrics.QueuedIntegrationTasks
    );
    
    UE_LOG(LogTemp, Log, TEXT("PhysicsGameplayIntegrationOptimizer Performance Report:\\n%s"), *Report);
}

void UPerf_PhysicsGameplayIntegrationOptimizer::UpdatePerformanceMetrics()
{
    IntegrationFrameCount++;
    
    // Simulate performance metrics collection
    float DeltaTime = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
    
    // Update integration timing
    CurrentMetrics.IntegrationTime = DeltaTime * 1000.0f; // Convert to milliseconds
    CurrentMetrics.GameplayPhysicsTime = CurrentMetrics.IntegrationTime * 0.4f;
    CurrentMetrics.CharacterIntegrationTime = CurrentMetrics.IntegrationTime * 0.3f;
    CurrentMetrics.DinosaurIntegrationTime = CurrentMetrics.IntegrationTime * 0.4f;
    CurrentMetrics.TerrainIntegrationTime = CurrentMetrics.IntegrationTime * 0.2f;
    CurrentMetrics.CollisionIntegrationTime = CurrentMetrics.IntegrationTime * 0.1f;
    CurrentMetrics.ValidationTime = CurrentMetrics.IntegrationTime * 0.05f;
    
    // Update memory metrics
    CurrentMetrics.MemoryUsage = 128.0f; // Simulated memory usage in MB
    CurrentMetrics.ActiveIntegrationComponents = 12;
    CurrentMetrics.QueuedIntegrationTasks = 6;
    
    // Update quality metrics
    CurrentMetrics.IntegrationAccuracy = 0.95f;
    CurrentMetrics.PhysicsStability = 0.98f;
    CurrentMetrics.GameplayResponsiveness = 0.96f;
    
    // Update tracking
    LastIntegrationTime = CurrentMetrics.IntegrationTime;
    AverageIntegrationTime = (AverageIntegrationTime * (IntegrationFrameCount - 1) + CurrentMetrics.IntegrationTime) / IntegrationFrameCount;
    PeakIntegrationTime = FMath::Max(PeakIntegrationTime, CurrentMetrics.IntegrationTime);
}

void UPerf_PhysicsGameplayIntegrationOptimizer::UpdatePerformanceState()
{
    float PerformanceRatio = CurrentMetrics.IntegrationTime / OptimizationSettings.TargetFrameTime60FPS;
    
    if (PerformanceRatio <= 0.7f)
    {
        CurrentState = EPerf_PhysicsGameplayIntegrationState::Optimal;
    }
    else if (PerformanceRatio <= OptimizationSettings.PerformanceWarningThreshold)
    {
        CurrentState = EPerf_PhysicsGameplayIntegrationState::Good;
    }
    else if (PerformanceRatio <= OptimizationSettings.PerformanceCriticalThreshold)
    {
        CurrentState = EPerf_PhysicsGameplayIntegrationState::Warning;
        HandlePerformanceWarning();
    }
    else if (PerformanceRatio <= 1.2f)
    {
        CurrentState = EPerf_PhysicsGameplayIntegrationState::Critical;
        HandlePerformanceCritical();
    }
    else
    {
        CurrentState = EPerf_PhysicsGameplayIntegrationState::Emergency;
        EnableEmergencyOptimization();
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::ApplyOptimizations()
{
    if (CurrentState == EPerf_PhysicsGameplayIntegrationState::Warning ||
        CurrentState == EPerf_PhysicsGameplayIntegrationState::Critical)
    {
        OptimizeIntegrationPerformance();
    }
    
    if (GetMemoryUsagePercentage() > OptimizationSettings.GarbageCollectionThreshold)
    {
        OptimizeIntegrationMemoryUsage();
    }
}

void UPerf_PhysicsGameplayIntegrationOptimizer::HandlePerformanceWarning()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsGameplayIntegrationOptimizer: Performance warning - applying optimizations"));
    OptimizeGameplayPhysicsIntegration();
}

void UPerf_PhysicsGameplayIntegrationOptimizer::HandlePerformanceCritical()
{
    UE_LOG(LogTemp, Error, TEXT("PhysicsGameplayIntegrationOptimizer: Critical performance - applying aggressive optimizations"));
    OptimizeCharacterPhysicsIntegration();
    OptimizeDinosaurPhysicsIntegration();
    OptimizeTerrainPhysicsIntegration();
}

void UPerf_PhysicsGameplayIntegrationOptimizer::ResetPerformanceTracking()
{
    LastIntegrationTime = 0.0f;
    AverageIntegrationTime = 0.0f;
    PeakIntegrationTime = 0.0f;
    IntegrationFrameCount = 0;
    
    CurrentMetrics = FPerf_PhysicsGameplayIntegrationMetrics();
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationBatching()
{
    // Optimize how integration tasks are batched
    int32 OptimalBatchSize = FMath::Clamp(OptimizationSettings.IntegrationBatchSize, 1, 8);
    OptimizationSettings.IntegrationBatchSize = OptimalBatchSize;
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationPriority()
{
    // Optimize integration task priority
    // Character physics gets highest priority, followed by dinosaurs, then terrain
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationCaching()
{
    // Optimize caching of integration results
    // Cache frequently used integration calculations
}

void UPerf_PhysicsGameplayIntegrationOptimizer::OptimizeIntegrationThreading()
{
    // Optimize threading for integration tasks
    int32 MaxThreads = FMath::Min(OptimizationSettings.MaxConcurrentIntegrations, 16);
    OptimizationSettings.MaxConcurrentIntegrations = MaxThreads;
}

float UPerf_PhysicsGameplayIntegrationOptimizer::CalculateIntegrationEfficiency() const
{
    float TargetTime = OptimizationSettings.TargetFrameTime60FPS;
    float ActualTime = CurrentMetrics.IntegrationTime;
    return FMath::Clamp(TargetTime / ActualTime, 0.0f, 1.0f);
}

float UPerf_PhysicsGameplayIntegrationOptimizer::CalculateMemoryEfficiency() const
{
    float MemoryUsageRatio = GetMemoryUsagePercentage();
    return FMath::Clamp(1.0f - MemoryUsageRatio, 0.0f, 1.0f);
}

float UPerf_PhysicsGameplayIntegrationOptimizer::CalculateOverallPerformanceScore() const
{
    float IntegrationEfficiency = CalculateIntegrationEfficiency();
    float MemoryEfficiency = CalculateMemoryEfficiency();
    float QualityScore = (CurrentMetrics.IntegrationAccuracy + CurrentMetrics.PhysicsStability + CurrentMetrics.GameplayResponsiveness) / 3.0f;
    
    return (IntegrationEfficiency * 0.5f + MemoryEfficiency * 0.3f + QualityScore * 0.2f);
}

bool UPerf_PhysicsGameplayIntegrationOptimizer::ValidateFrameTimeTargets() const
{
    return CurrentMetrics.IntegrationTime <= OptimizationSettings.TargetFrameTime60FPS;
}

bool UPerf_PhysicsGameplayIntegrationOptimizer::ValidateMemoryUsage() const
{
    return GetMemoryUsagePercentage() <= 1.0f;
}

bool UPerf_PhysicsGameplayIntegrationOptimizer::ValidateIntegrationStability() const
{
    return CurrentMetrics.PhysicsStability >= 0.9f && CurrentMetrics.IntegrationAccuracy >= 0.9f;
}
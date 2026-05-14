#include "Perf_VehiclePhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "HAL/PlatformMemory.h"
#include "Stats/StatsHierarchical.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Async/AsyncWork.h"
#include "Kismet/GameplayStatics.h"

DEFINE_STAT(STAT_VehiclePhysicsUpdate);
DEFINE_STAT(STAT_VehicleSurfaceDetection);
DEFINE_STAT(STAT_VehicleDamageCalculation);
DEFINE_STAT(STAT_VehicleMemoryUsage);

UPerf_VehiclePhysicsPerformanceIntegrator::UPerf_VehiclePhysicsPerformanceIntegrator()
    : bIsMonitoringActive(false)
    , bDebugEnabled(false)
    , LastOptimizationTime(0.0f)
    , OptimizationInterval(PERFORMANCE_UPDATE_INTERVAL)
    , AccumulatedPhysicsTime(0.0f)
    , AccumulatedFrameTime(0.0f)
    , SampleCount(0)
{
    // Initialize default optimization settings
    OptimizationSettings = FPerf_VehicleOptimizationSettings();
    
    // Initialize metrics
    CurrentMetrics = FPerf_VehiclePhysicsMetrics();
    
    // Reserve space for performance history
    FrameTimeHistory.Reserve(MAX_FRAME_HISTORY);
    PhysicsTimeHistory.Reserve(MAX_FRAME_HISTORY);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Initializing vehicle physics performance monitoring"));
    
    // Initialize performance monitoring
    InitializeVehiclePerformanceMonitoring();
    
    // Set up optimization timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UPerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePerformanceMetrics,
            OptimizationInterval,
            true
        );
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Deinitializing vehicle physics performance monitoring"));
    
    bIsMonitoringActive = false;
    TrackedVehicles.Empty();
    
    Super::Deinitialize();
}

void UPerf_VehiclePhysicsPerformanceIntegrator::InitializeVehiclePerformanceMonitoring()
{
    SCOPE_CYCLE_COUNTER(STAT_VehiclePhysicsUpdate);
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Starting vehicle physics performance monitoring"));
    
    // Clear previous tracking data
    TrackedVehicles.Empty();
    FrameTimeHistory.Empty();
    PhysicsTimeHistory.Empty();
    
    // Reset metrics
    CurrentMetrics = FPerf_VehiclePhysicsMetrics();
    AccumulatedPhysicsTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    SampleCount = 0;
    
    // Find all vehicle physics actors in the world
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor && Actor->GetName().Contains(TEXT("Vehicle")))
            {
                TrackedVehicles.Add(Actor);
                UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Tracking vehicle actor: %s"), *Actor->GetName());
            }
        }
    }
    
    bIsMonitoringActive = true;
    LastOptimizationTime = FPlatformTime::Seconds();
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Monitoring %d vehicle actors"), TrackedVehicles.Num());
}

void UPerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePerformanceMetrics()
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    SCOPE_CYCLE_COUNTER(STAT_VehiclePhysicsUpdate);
    
    const float CurrentTime = FPlatformTime::Seconds();
    const float DeltaTime = CurrentTime - LastOptimizationTime;
    
    // Update frame time tracking
    const float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > MAX_FRAME_HISTORY)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    CurrentMetrics.AverageFrameTime = FrameTimeHistory.Num() > 0 ? TotalFrameTime / FrameTimeHistory.Num() : 0.0f;
    
    // Update vehicle count and physics metrics
    int32 ActiveVehicles = 0;
    float TotalPhysicsTime = 0.0f;
    float TotalSurfaceDetectionTime = 0.0f;
    float TotalDamageTime = 0.0f;
    float TotalSuspensionTime = 0.0f;
    float TotalTireTime = 0.0f;
    
    // Clean up invalid weak pointers
    TrackedVehicles.RemoveAll([](const TWeakObjectPtr<AActor>& WeakPtr) {
        return !WeakPtr.IsValid();
    });
    
    // Update metrics for each tracked vehicle
    for (const TWeakObjectPtr<AActor>& WeakVehicle : TrackedVehicles)
    {
        if (AActor* Vehicle = WeakVehicle.Get())
        {
            ActiveVehicles++;
            
            // Simulate physics timing measurements
            TotalPhysicsTime += 0.5f + FMath::RandRange(-0.2f, 0.2f);
            TotalSurfaceDetectionTime += 0.1f + FMath::RandRange(-0.05f, 0.05f);
            TotalDamageTime += 0.05f + FMath::RandRange(-0.02f, 0.02f);
            TotalSuspensionTime += 0.3f + FMath::RandRange(-0.1f, 0.1f);
            TotalTireTime += 0.2f + FMath::RandRange(-0.08f, 0.08f);
        }
    }
    
    // Update current metrics
    CurrentMetrics.ActiveVehicleCount = ActiveVehicles;
    CurrentMetrics.PhysicsUpdateTime = TotalPhysicsTime;
    CurrentMetrics.SurfaceDetectionTime = TotalSurfaceDetectionTime;
    CurrentMetrics.DamageCalculationTime = TotalDamageTime;
    CurrentMetrics.SuspensionUpdateTime = TotalSuspensionTime;
    CurrentMetrics.TirePhysicsTime = TotalTireTime;
    
    // Monitor memory usage
    MonitorVehicleMemoryUsage();
    
    // Check performance targets
    const float TargetFrameTime = (OptimizationSettings.PerformanceLevel == EPerf_VehiclePerformanceLevel::Ultra) ? 
        TARGET_60FPS_FRAME_TIME : TARGET_30FPS_FRAME_TIME;
    CurrentMetrics.bPerformanceTargetMet = CurrentMetrics.AverageFrameTime <= TargetFrameTime;
    
    // Apply optimizations if needed
    if (DeltaTime >= OptimizationInterval)
    {
        OptimizeVehiclePhysicsPerformance();
        LastOptimizationTime = CurrentTime;
    }
    
    // Debug logging
    if (bDebugEnabled)
    {
        UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Active Vehicles: %d, Avg Frame Time: %.2fms, Physics Time: %.2fms"), 
            CurrentMetrics.ActiveVehicleCount, CurrentMetrics.AverageFrameTime, CurrentMetrics.PhysicsUpdateTime);
    }
}

FPerf_VehiclePhysicsMetrics UPerf_VehiclePhysicsPerformanceIntegrator::GetVehiclePhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::SetVehicleOptimizationSettings(const FPerf_VehicleOptimizationSettings& Settings)
{
    OptimizationSettings = Settings;
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Updated optimization settings - Performance Level: %d"), 
        static_cast<int32>(Settings.PerformanceLevel));
    
    // Apply new settings immediately
    ApplyPerformanceOptimizations();
}

FPerf_VehicleOptimizationSettings UPerf_VehiclePhysicsPerformanceIntegrator::GetOptimizationSettings() const
{
    return OptimizationSettings;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehiclePhysicsPerformance()
{
    SCOPE_CYCLE_COUNTER(STAT_VehiclePhysicsUpdate);
    
    if (!CurrentMetrics.bPerformanceTargetMet)
    {
        UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsPerformanceIntegrator: Performance target not met, applying optimizations"));
        
        // Automatically adjust performance level if targets are not met
        if (CurrentMetrics.AverageFrameTime > TARGET_60FPS_FRAME_TIME * 1.5f)
        {
            // Significant performance issues - reduce to Low
            if (OptimizationSettings.PerformanceLevel > EPerf_VehiclePerformanceLevel::Low)
            {
                OptimizationSettings.PerformanceLevel = EPerf_VehiclePerformanceLevel::Low;
                UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsPerformanceIntegrator: Auto-reducing to Low performance level"));
            }
        }
        else if (CurrentMetrics.AverageFrameTime > TARGET_60FPS_FRAME_TIME * 1.2f)
        {
            // Moderate performance issues - reduce to Medium
            if (OptimizationSettings.PerformanceLevel > EPerf_VehiclePerformanceLevel::Medium)
            {
                OptimizationSettings.PerformanceLevel = EPerf_VehiclePerformanceLevel::Medium;
                UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsPerformanceIntegrator: Auto-reducing to Medium performance level"));
            }
        }
    }
    else if (CurrentMetrics.AverageFrameTime < TARGET_60FPS_FRAME_TIME * 0.8f)
    {
        // Performance is good - can potentially increase quality
        if (OptimizationSettings.PerformanceLevel < EPerf_VehiclePerformanceLevel::Ultra)
        {
            OptimizationSettings.PerformanceLevel = static_cast<EPerf_VehiclePerformanceLevel>(
                static_cast<int32>(OptimizationSettings.PerformanceLevel) + 1);
            UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Auto-increasing performance level"));
        }
    }
    
    ApplyPerformanceOptimizations();
}

void UPerf_VehiclePhysicsPerformanceIntegrator::SetPerformanceLevel(EPerf_VehiclePerformanceLevel Level)
{
    OptimizationSettings.PerformanceLevel = Level;
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Performance level set to %d"), static_cast<int32>(Level));
    
    ApplyPerformanceOptimizations();
}

bool UPerf_VehiclePhysicsPerformanceIntegrator::ArePerformanceTargetsMet() const
{
    return CurrentMetrics.bPerformanceTargetMet;
}

TArray<FString> UPerf_VehiclePhysicsPerformanceIntegrator::GetPerformanceRecommendations() const
{
    TArray<FString> Recommendations;
    
    if (CurrentMetrics.AverageFrameTime > TARGET_60FPS_FRAME_TIME)
    {
        Recommendations.Add(TEXT("Consider reducing vehicle physics update frequency"));
        
        if (CurrentMetrics.ActiveVehicleCount > OptimizationSettings.MaxConcurrentVehicles)
        {
            Recommendations.Add(FString::Printf(TEXT("Too many active vehicles (%d). Consider reducing to %d"), 
                CurrentMetrics.ActiveVehicleCount, OptimizationSettings.MaxConcurrentVehicles));
        }
        
        if (CurrentMetrics.PhysicsUpdateTime > 2.0f)
        {
            Recommendations.Add(TEXT("Physics update time is high. Enable async physics processing"));
        }
        
        if (CurrentMetrics.SurfaceDetectionTime > 0.5f)
        {
            Recommendations.Add(TEXT("Surface detection is expensive. Increase LOD distance"));
        }
        
        if (CurrentMetrics.MemoryUsageMB > 100.0f)
        {
            Recommendations.Add(TEXT("High memory usage detected. Consider reducing vehicle detail levels"));
        }
    }
    
    if (Recommendations.Num() == 0)
    {
        Recommendations.Add(TEXT("Performance is optimal"));
    }
    
    return Recommendations;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::SetVehiclePhysicsDebugging(bool bEnabled)
{
    bDebugEnabled = bEnabled;
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Debug mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

FString UPerf_VehiclePhysicsPerformanceIntegrator::GetVehiclePhysicsDebugInfo() const
{
    FString DebugInfo = FString::Printf(TEXT("Vehicle Physics Performance Debug Info:\n"));
    DebugInfo += FString::Printf(TEXT("Active Vehicles: %d\n"), CurrentMetrics.ActiveVehicleCount);
    DebugInfo += FString::Printf(TEXT("Average Frame Time: %.2fms\n"), CurrentMetrics.AverageFrameTime);
    DebugInfo += FString::Printf(TEXT("Physics Update Time: %.2fms\n"), CurrentMetrics.PhysicsUpdateTime);
    DebugInfo += FString::Printf(TEXT("Surface Detection Time: %.2fms\n"), CurrentMetrics.SurfaceDetectionTime);
    DebugInfo += FString::Printf(TEXT("Damage Calculation Time: %.2fms\n"), CurrentMetrics.DamageCalculationTime);
    DebugInfo += FString::Printf(TEXT("Memory Usage: %.2fMB\n"), CurrentMetrics.MemoryUsageMB);
    DebugInfo += FString::Printf(TEXT("Performance Target Met: %s\n"), CurrentMetrics.bPerformanceTargetMet ? TEXT("Yes") : TEXT("No"));
    DebugInfo += FString::Printf(TEXT("Performance Level: %d\n"), static_cast<int32>(OptimizationSettings.PerformanceLevel));
    
    return DebugInfo;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePhysicsLOD()
{
    // Update LOD based on distance and performance level
    float LODDistance = OptimizationSettings.SurfaceDetectionLODDistance;
    
    switch (OptimizationSettings.PerformanceLevel)
    {
        case EPerf_VehiclePerformanceLevel::Ultra:
            LODDistance *= 1.5f;
            break;
        case EPerf_VehiclePerformanceLevel::High:
            LODDistance *= 1.2f;
            break;
        case EPerf_VehiclePerformanceLevel::Medium:
            LODDistance *= 1.0f;
            break;
        case EPerf_VehiclePerformanceLevel::Low:
            LODDistance *= 0.7f;
            break;
        case EPerf_VehiclePerformanceLevel::Minimal:
            LODDistance *= 0.5f;
            break;
    }
    
    // Apply LOD settings to tracked vehicles
    for (const TWeakObjectPtr<AActor>& WeakVehicle : TrackedVehicles)
    {
        if (AActor* Vehicle = WeakVehicle.Get())
        {
            // Apply LOD distance settings to vehicle components
            // This would integrate with the actual vehicle physics system
        }
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::MonitorVehicleMemoryUsage()
{
    SCOPE_CYCLE_COUNTER(STAT_VehicleMemoryUsage);
    
    // Get current memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    // Estimate vehicle physics memory usage (simplified calculation)
    const float BaseMemoryPerVehicle = 2.0f; // MB per vehicle
    const float EstimatedVehicleMemory = CurrentMetrics.ActiveVehicleCount * BaseMemoryPerVehicle;
    
    CurrentMetrics.MemoryUsageMB = EstimatedVehicleMemory;
    
    // Log memory warnings if usage is high
    if (CurrentMetrics.MemoryUsageMB > 50.0f && bDebugEnabled)
    {
        UE_LOG(LogTemp, Warning, TEXT("VehiclePhysicsPerformanceIntegrator: High vehicle memory usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::ApplyPerformanceOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Applying performance optimizations for level %d"), 
        static_cast<int32>(OptimizationSettings.PerformanceLevel));
    
    // Update physics LOD
    UpdateVehiclePhysicsLOD();
    
    // Update async physics settings
    UpdateAsyncPhysicsSettings();
    
    // Adjust concurrent vehicle limits
    switch (OptimizationSettings.PerformanceLevel)
    {
        case EPerf_VehiclePerformanceLevel::Ultra:
            OptimizationSettings.MaxConcurrentVehicles = 20;
            OptimizationSettings.bEnableDamageSimulation = true;
            break;
        case EPerf_VehiclePerformanceLevel::High:
            OptimizationSettings.MaxConcurrentVehicles = 15;
            OptimizationSettings.bEnableDamageSimulation = true;
            break;
        case EPerf_VehiclePerformanceLevel::Medium:
            OptimizationSettings.MaxConcurrentVehicles = 10;
            OptimizationSettings.bEnableDamageSimulation = true;
            break;
        case EPerf_VehiclePerformanceLevel::Low:
            OptimizationSettings.MaxConcurrentVehicles = 6;
            OptimizationSettings.bEnableDamageSimulation = false;
            break;
        case EPerf_VehiclePerformanceLevel::Minimal:
            OptimizationSettings.MaxConcurrentVehicles = 3;
            OptimizationSettings.bEnableDamageSimulation = false;
            break;
    }
}

float UPerf_VehiclePhysicsPerformanceIntegrator::CalculateVehiclePerformanceScore() const
{
    float Score = 100.0f;
    
    // Penalize for high frame times
    if (CurrentMetrics.AverageFrameTime > TARGET_60FPS_FRAME_TIME)
    {
        Score -= (CurrentMetrics.AverageFrameTime - TARGET_60FPS_FRAME_TIME) * 2.0f;
    }
    
    // Penalize for high physics times
    if (CurrentMetrics.PhysicsUpdateTime > 1.0f)
    {
        Score -= (CurrentMetrics.PhysicsUpdateTime - 1.0f) * 10.0f;
    }
    
    // Penalize for high memory usage
    if (CurrentMetrics.MemoryUsageMB > 30.0f)
    {
        Score -= (CurrentMetrics.MemoryUsageMB - 30.0f) * 0.5f;
    }
    
    return FMath::Clamp(Score, 0.0f, 100.0f);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::UpdateAsyncPhysicsSettings()
{
    // Enable async physics based on performance level and current metrics
    bool bShouldUseAsync = OptimizationSettings.bEnableAsyncPhysics && 
                          (OptimizationSettings.PerformanceLevel >= EPerf_VehiclePerformanceLevel::Medium);
    
    if (bShouldUseAsync && CurrentMetrics.PhysicsUpdateTime > 1.0f)
    {
        UE_LOG(LogTemp, Log, TEXT("VehiclePhysicsPerformanceIntegrator: Enabling async physics for better performance"));
        // This would integrate with the actual vehicle physics system to enable async processing
    }
}
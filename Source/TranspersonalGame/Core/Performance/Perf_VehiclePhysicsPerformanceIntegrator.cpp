#include "Perf_VehiclePhysicsPerformanceIntegrator.h"
#include "../Core_VehiclePhysicsSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UPerf_VehiclePhysicsPerformanceIntegrator::UPerf_VehiclePhysicsPerformanceIntegrator()
{
    CurrentOptimizationLevel = EPerf_VehiclePhysicsOptimizationLevel::High;
    bIsMonitoringActive = false;
    bAdaptiveOptimizationEnabled = true;
    
    // Performance Thresholds
    TargetVehiclePhysicsFrameTime = 2.0f; // 2ms target for vehicle physics
    MaxVehicleSimulationCost = 5.0f; // 5ms max simulation cost
    MaxActiveVehicles = 20; // Maximum 20 active vehicles
    
    // Internal State
    LastFrameTime = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    bOptimizationInProgress = false;
    LastOptimizationTime = 0.0f;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Integrator initialized"));
    
    // Start monitoring automatically
    StartVehiclePhysicsMonitoring();
}

void UPerf_VehiclePhysicsPerformanceIntegrator::Deinitialize()
{
    StopVehiclePhysicsMonitoring();
    RegisteredVehicleSystems.Empty();
    
    Super::Deinitialize();
}

void UPerf_VehiclePhysicsPerformanceIntegrator::StartVehiclePhysicsMonitoring()
{
    if (bIsMonitoringActive)
    {
        return;
    }
    
    bIsMonitoringActive = true;
    
    if (UWorld* World = GetWorld())
    {
        // Start monitoring timer (60 FPS monitoring)
        World->GetTimerManager().SetTimer(
            MonitoringTimerHandle,
            this,
            &UPerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePhysicsMetrics,
            1.0f / 60.0f,
            true
        );
        
        // Start optimization timer (10 FPS optimization checks)
        World->GetTimerManager().SetTimer(
            OptimizationTimerHandle,
            this,
            &UPerf_VehiclePhysicsPerformanceIntegrator::CheckAdaptiveOptimization,
            0.1f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance monitoring started"));
}

void UPerf_VehiclePhysicsPerformanceIntegrator::StopVehiclePhysicsMonitoring()
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    bIsMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MonitoringTimerHandle);
        World->GetTimerManager().ClearTimer(OptimizationTimerHandle);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance monitoring stopped"));
}

FPerf_VehiclePhysicsMetrics UPerf_VehiclePhysicsPerformanceIntegrator::GetVehiclePhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_VehiclePhysicsPerformanceIntegrator::SetVehiclePhysicsOptimizationLevel(EPerf_VehiclePhysicsOptimizationLevel Level)
{
    if (CurrentOptimizationLevel == Level)
    {
        return;
    }
    
    CurrentOptimizationLevel = Level;
    ApplyVehiclePhysicsOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics optimization level set to: %d"), (int32)Level);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehiclePhysicsPerformance()
{
    if (bOptimizationInProgress)
    {
        return;
    }
    
    bOptimizationInProgress = true;
    
    // Apply optimizations based on current level
    ApplyVehiclePhysicsOptimizations();
    
    // Update optimization state
    CurrentMetrics.bVehiclePhysicsOptimizationActive = true;
    LastOptimizationTime = GetWorld()->GetTimeSeconds();
    
    bOptimizationInProgress = false;
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics performance optimization completed"));
}

void UPerf_VehiclePhysicsPerformanceIntegrator::EnableAdaptiveVehiclePhysicsOptimization(bool bEnable)
{
    bAdaptiveOptimizationEnabled = bEnable;
    
    UE_LOG(LogTemp, Log, TEXT("Adaptive Vehicle Physics optimization: %s"), 
           bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_VehiclePhysicsPerformanceIntegrator::RegisterVehiclePhysicsSystem(ACore_VehiclePhysicsSystem* VehicleSystem)
{
    if (!VehicleSystem || RegisteredVehicleSystems.Contains(VehicleSystem))
    {
        return;
    }
    
    RegisteredVehicleSystems.Add(VehicleSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics System registered: %s"), 
           *VehicleSystem->GetName());
}

void UPerf_VehiclePhysicsPerformanceIntegrator::UnregisterVehiclePhysicsSystem(ACore_VehiclePhysicsSystem* VehicleSystem)
{
    if (!VehicleSystem)
    {
        return;
    }
    
    RegisteredVehicleSystems.Remove(VehicleSystem);
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics System unregistered: %s"), 
           *VehicleSystem->GetName());
}

void UPerf_VehiclePhysicsPerformanceIntegrator::AnalyzeVehiclePhysicsPerformance()
{
    // Comprehensive performance analysis
    float TotalVehiclePhysicsCost = CurrentMetrics.VehicleCollisionCost + 
                                   CurrentMetrics.VehicleWheelPhysicsCost + 
                                   CurrentMetrics.VehicleEngineSimulationCost + 
                                   CurrentMetrics.VehicleSuspensionCost;
    
    // Performance assessment
    bool bPerformanceOptimal = (CurrentMetrics.VehiclePhysicsFrameTime <= TargetVehiclePhysicsFrameTime) &&
                              (TotalVehiclePhysicsCost <= MaxVehicleSimulationCost) &&
                              (CurrentMetrics.ActiveVehicleCount <= MaxActiveVehicles);
    
    // Log analysis results
    UE_LOG(LogTemp, Warning, TEXT("=== Vehicle Physics Performance Analysis ==="));
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms (Target: %.2fms)"), 
           CurrentMetrics.VehiclePhysicsFrameTime, TargetVehiclePhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Total Physics Cost: %.2fms (Max: %.2fms)"), 
           TotalVehiclePhysicsCost, MaxVehicleSimulationCost);
    UE_LOG(LogTemp, Warning, TEXT("Active Vehicles: %d (Max: %d)"), 
           CurrentMetrics.ActiveVehicleCount, MaxActiveVehicles);
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimal: %s"), 
           bPerformanceOptimal ? TEXT("YES") : TEXT("NO"));
    
    // Recommend optimizations if needed
    if (!bPerformanceOptimal)
    {
        if (CurrentMetrics.VehiclePhysicsFrameTime > TargetVehiclePhysicsFrameTime)
        {
            UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: Reduce vehicle physics quality"));
        }
        if (CurrentMetrics.ActiveVehicleCount > MaxActiveVehicles)
        {
            UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: Implement vehicle culling"));
        }
    }
}

bool UPerf_VehiclePhysicsPerformanceIntegrator::IsVehiclePhysicsPerformanceOptimal() const
{
    return (CurrentMetrics.VehiclePhysicsFrameTime <= TargetVehiclePhysicsFrameTime) &&
           (CurrentMetrics.VehicleSimulationCost <= MaxVehicleSimulationCost) &&
           (CurrentMetrics.ActiveVehicleCount <= MaxActiveVehicles);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::RunVehiclePhysicsPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Vehicle Physics Performance Test Started ==="));
    
    // Simulate performance test
    float TestStartTime = GetWorld()->GetTimeSeconds();
    
    // Test vehicle physics systems
    for (ACore_VehiclePhysicsSystem* VehicleSystem : RegisteredVehicleSystems)
    {
        if (VehicleSystem)
        {
            UE_LOG(LogTemp, Log, TEXT("Testing Vehicle System: %s"), *VehicleSystem->GetName());
            // Vehicle system would have test methods here
        }
    }
    
    float TestDuration = GetWorld()->GetTimeSeconds() - TestStartTime;
    
    UE_LOG(LogTemp, Warning, TEXT("Vehicle Physics Performance Test completed in %.2fms"), 
           TestDuration * 1000.0f);
    
    // Run analysis after test
    AnalyzeVehiclePhysicsPerformance();
}

void UPerf_VehiclePhysicsPerformanceIntegrator::LogVehiclePhysicsPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Vehicle Physics Performance Report ==="));
    UE_LOG(LogTemp, Warning, TEXT("Timestamp: %s"), *FDateTime::Now().ToString());
    UE_LOG(LogTemp, Warning, TEXT("Optimization Level: %d"), (int32)CurrentOptimizationLevel);
    UE_LOG(LogTemp, Warning, TEXT("Monitoring Active: %s"), bIsMonitoringActive ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Adaptive Optimization: %s"), bAdaptiveOptimizationEnabled ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Registered Vehicle Systems: %d"), RegisteredVehicleSystems.Num());
    
    // Current metrics
    UE_LOG(LogTemp, Warning, TEXT("--- Current Metrics ---"));
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2fms"), CurrentMetrics.VehiclePhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Simulation Cost: %.2fms"), CurrentMetrics.VehicleSimulationCost);
    UE_LOG(LogTemp, Warning, TEXT("Active Vehicles: %d"), CurrentMetrics.ActiveVehicleCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.VehiclePhysicsMemoryUsage);
    UE_LOG(LogTemp, Warning, TEXT("Quality Scale: %.2f"), CurrentMetrics.VehiclePhysicsQualityScale);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePhysicsMetrics()
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    // Update frame time tracking
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float DeltaTime = CurrentTime - LastFrameTime;
    LastFrameTime = CurrentTime;
    
    AccumulatedFrameTime += DeltaTime;
    FrameCount++;
    
    // Calculate average frame time over last second
    if (AccumulatedFrameTime >= 1.0f)
    {
        CurrentMetrics.VehiclePhysicsFrameTime = (AccumulatedFrameTime / FrameCount) * 1000.0f; // Convert to ms
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
    }
    
    // Update active vehicle count
    CurrentMetrics.ActiveVehicleCount = RegisteredVehicleSystems.Num();
    
    // Simulate physics costs (would be real measurements in production)
    CurrentMetrics.VehicleCollisionCost = FMath::RandRange(0.5f, 2.0f);
    CurrentMetrics.VehicleWheelPhysicsCost = FMath::RandRange(0.3f, 1.5f);
    CurrentMetrics.VehicleEngineSimulationCost = FMath::RandRange(0.2f, 1.0f);
    CurrentMetrics.VehicleSuspensionCost = FMath::RandRange(0.1f, 0.8f);
    
    CurrentMetrics.VehicleSimulationCost = CurrentMetrics.VehicleCollisionCost + 
                                          CurrentMetrics.VehicleWheelPhysicsCost + 
                                          CurrentMetrics.VehicleEngineSimulationCost + 
                                          CurrentMetrics.VehicleSuspensionCost;
    
    // Update memory usage (simulated)
    CurrentMetrics.VehiclePhysicsMemoryUsage = CurrentMetrics.ActiveVehicleCount * 2.5f; // 2.5MB per vehicle
    
    // Update quality scale based on optimization level
    switch (CurrentOptimizationLevel)
    {
        case EPerf_VehiclePhysicsOptimizationLevel::Ultra:
            CurrentMetrics.VehiclePhysicsQualityScale = 1.0f;
            break;
        case EPerf_VehiclePhysicsOptimizationLevel::High:
            CurrentMetrics.VehiclePhysicsQualityScale = 0.85f;
            break;
        case EPerf_VehiclePhysicsOptimizationLevel::Medium:
            CurrentMetrics.VehiclePhysicsQualityScale = 0.7f;
            break;
        case EPerf_VehiclePhysicsOptimizationLevel::Low:
            CurrentMetrics.VehiclePhysicsQualityScale = 0.5f;
            break;
        case EPerf_VehiclePhysicsOptimizationLevel::Minimal:
            CurrentMetrics.VehiclePhysicsQualityScale = 0.3f;
            break;
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::ApplyVehiclePhysicsOptimizations()
{
    // Apply optimizations based on current level
    OptimizeVehicleCollisionSettings();
    OptimizeVehicleWheelPhysics();
    OptimizeVehicleEngineSimulation();
    OptimizeVehicleSuspensionSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics optimizations applied for level: %d"), 
           (int32)CurrentOptimizationLevel);
}

void UPerf_VehiclePhysicsPerformanceIntegrator::CheckAdaptiveOptimization()
{
    if (!bAdaptiveOptimizationEnabled || bOptimizationInProgress)
    {
        return;
    }
    
    // Check if optimization is needed
    bool bNeedsOptimization = (CurrentMetrics.VehiclePhysicsFrameTime > TargetVehiclePhysicsFrameTime * 1.2f) ||
                             (CurrentMetrics.VehicleSimulationCost > MaxVehicleSimulationCost * 1.1f);
    
    if (bNeedsOptimization)
    {
        // Automatically reduce optimization level
        EPerf_VehiclePhysicsOptimizationLevel NewLevel = CurrentOptimizationLevel;
        
        switch (CurrentOptimizationLevel)
        {
            case EPerf_VehiclePhysicsOptimizationLevel::Ultra:
                NewLevel = EPerf_VehiclePhysicsOptimizationLevel::High;
                break;
            case EPerf_VehiclePhysicsOptimizationLevel::High:
                NewLevel = EPerf_VehiclePhysicsOptimizationLevel::Medium;
                break;
            case EPerf_VehiclePhysicsOptimizationLevel::Medium:
                NewLevel = EPerf_VehiclePhysicsOptimizationLevel::Low;
                break;
            case EPerf_VehiclePhysicsOptimizationLevel::Low:
                NewLevel = EPerf_VehiclePhysicsOptimizationLevel::Minimal;
                break;
            case EPerf_VehiclePhysicsOptimizationLevel::Minimal:
                // Already at minimum
                break;
        }
        
        if (NewLevel != CurrentOptimizationLevel)
        {
            SetVehiclePhysicsOptimizationLevel(NewLevel);
            UE_LOG(LogTemp, Warning, TEXT("Adaptive optimization: Reduced to level %d"), (int32)NewLevel);
        }
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehicleCollisionSettings()
{
    // Optimize collision settings based on optimization level
    for (ACore_VehiclePhysicsSystem* VehicleSystem : RegisteredVehicleSystems)
    {
        if (VehicleSystem)
        {
            // Vehicle system would have collision optimization methods
            UE_LOG(LogTemp, Verbose, TEXT("Optimizing collision for vehicle: %s"), *VehicleSystem->GetName());
        }
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehicleWheelPhysics()
{
    // Optimize wheel physics based on optimization level
    for (ACore_VehiclePhysicsSystem* VehicleSystem : RegisteredVehicleSystems)
    {
        if (VehicleSystem)
        {
            // Vehicle system would have wheel physics optimization methods
            UE_LOG(LogTemp, Verbose, TEXT("Optimizing wheel physics for vehicle: %s"), *VehicleSystem->GetName());
        }
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehicleEngineSimulation()
{
    // Optimize engine simulation based on optimization level
    for (ACore_VehiclePhysicsSystem* VehicleSystem : RegisteredVehicleSystems)
    {
        if (VehicleSystem)
        {
            // Vehicle system would have engine simulation optimization methods
            UE_LOG(LogTemp, Verbose, TEXT("Optimizing engine simulation for vehicle: %s"), *VehicleSystem->GetName());
        }
    }
}

void UPerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehicleSuspensionSettings()
{
    // Optimize suspension settings based on optimization level
    for (ACore_VehiclePhysicsSystem* VehicleSystem : RegisteredVehicleSystems)
    {
        if (VehicleSystem)
        {
            // Vehicle system would have suspension optimization methods
            UE_LOG(LogTemp, Verbose, TEXT("Optimizing suspension for vehicle: %s"), *VehicleSystem->GetName());
        }
    }
}
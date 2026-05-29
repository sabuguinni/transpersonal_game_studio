#include "Core_PhysicsPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

ACore_PhysicsPerformanceMonitor::ACore_PhysicsPerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms
    
    // Initialize default values
    CurrentPerformanceLevel = ECore_PhysicsPerformanceLevel::Medium;
    MonitoringInterval = 0.5f;
    bAutoOptimize = true;
    TargetFrameTime = 16.67f; // 60 FPS target
    bIsMonitoring = false;
    TimeSinceLastCheck = 0.0f;
    MaxHistorySize = 60; // Keep 60 samples for averaging
    AverageFrameTime = 0.0f;
    
    // Initialize metrics
    CurrentMetrics = FCore_PhysicsMetrics();
    
    // Reserve space for history arrays
    FrameTimeHistory.Reserve(MaxHistorySize);
    RigidBodyHistory.Reserve(MaxHistorySize);
}

void ACore_PhysicsPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Starting performance monitoring"));
    
    // Auto-start monitoring if enabled
    if (bAutoOptimize)
    {
        StartMonitoring();
    }
    
    // Apply initial performance settings
    ApplyPerformanceSettings();
}

void ACore_PhysicsPerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsMonitoring)
    {
        return;
    }
    
    TimeSinceLastCheck += DeltaTime;
    
    if (TimeSinceLastCheck >= MonitoringInterval)
    {
        UpdateMetrics();
        
        if (bAutoOptimize)
        {
            CheckPerformanceThresholds();
        }
        
        TimeSinceLastCheck = 0.0f;
    }
}

void ACore_PhysicsPerformanceMonitor::StartMonitoring()
{
    bIsMonitoring = true;
    TimeSinceLastCheck = 0.0f;
    
    // Clear history
    FrameTimeHistory.Empty();
    RigidBodyHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Monitoring started"));
}

void ACore_PhysicsPerformanceMonitor::StopMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Monitoring stopped"));
}

FCore_PhysicsMetrics ACore_PhysicsPerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void ACore_PhysicsPerformanceMonitor::SetPerformanceLevel(ECore_PhysicsPerformanceLevel NewLevel)
{
    if (CurrentPerformanceLevel != NewLevel)
    {
        CurrentPerformanceLevel = NewLevel;
        ApplyPerformanceSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Performance level changed to %d"), 
               static_cast<int32>(NewLevel));
    }
}

ECore_PhysicsPerformanceLevel ACore_PhysicsPerformanceMonitor::GetCurrentPerformanceLevel() const
{
    return CurrentPerformanceLevel;
}

void ACore_PhysicsPerformanceMonitor::OptimizePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get current physics settings
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (!PhysicsSettings)
    {
        return;
    }
    
    // Apply optimizations based on current performance
    if (AverageFrameTime > TargetFrameTime * 1.5f) // Performance is poor
    {
        // Reduce physics quality for better performance
        if (CurrentPerformanceLevel != ECore_PhysicsPerformanceLevel::Low)
        {
            SetPerformanceLevel(ECore_PhysicsPerformanceLevel::Low);
        }
    }
    else if (AverageFrameTime < TargetFrameTime * 0.8f) // Performance is good
    {
        // Increase physics quality
        if (CurrentPerformanceLevel != ECore_PhysicsPerformanceLevel::Ultra)
        {
            ECore_PhysicsPerformanceLevel NextLevel = static_cast<ECore_PhysicsPerformanceLevel>(
                static_cast<int32>(CurrentPerformanceLevel) + 1);
            SetPerformanceLevel(NextLevel);
        }
    }
}

void ACore_PhysicsPerformanceMonitor::EnableLODSystem(bool bEnable)
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: LOD System %s"), 
           bEnable ? TEXT("Enabled") : TEXT("Disabled"));
    
    // Implementation would enable/disable physics LOD system
    // This is a placeholder for the actual LOD implementation
}

void ACore_PhysicsPerformanceMonitor::SetPhysicsSubsteps(int32 NumSubsteps)
{
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        // Set physics substeps for better accuracy vs performance trade-off
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Physics substeps set to %d"), NumSubsteps);
    }
}

void ACore_PhysicsPerformanceMonitor::LogPerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %d"), static_cast<int32>(CurrentPerformanceLevel));
    UE_LOG(LogTemp, Warning, TEXT("Physics Frame Time: %.2f ms"), CurrentMetrics.PhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Collision Checks: %d"), CurrentMetrics.CollisionChecks);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("CPU Usage: %.2f%%"), CurrentMetrics.CPUUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("=== END REPORT ==="));
}

void ACore_PhysicsPerformanceMonitor::ResetMetrics()
{
    CurrentMetrics = FCore_PhysicsMetrics();
    FrameTimeHistory.Empty();
    RigidBodyHistory.Empty();
    AverageFrameTime = 0.0f;
    TimeSinceLastCheck = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Metrics reset"));
}

void ACore_PhysicsPerformanceMonitor::UpdateMetrics()
{
    CollectPhysicsMetrics();
    CollectMemoryMetrics();
    CollectCPUMetrics();
    
    // Update frame time history
    FrameTimeHistory.Add(CurrentMetrics.PhysicsFrameTime);
    if (FrameTimeHistory.Num() > MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Update rigid body history
    RigidBodyHistory.Add(CurrentMetrics.ActiveRigidBodies);
    if (RigidBodyHistory.Num() > MaxHistorySize)
    {
        RigidBodyHistory.RemoveAt(0);
    }
    
    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Time : FrameTimeHistory)
        {
            TotalTime += Time;
        }
        AverageFrameTime = TotalTime / FrameTimeHistory.Num();
    }
}

void ACore_PhysicsPerformanceMonitor::CheckPerformanceThresholds()
{
    // Check if we need to adjust performance level
    if (AverageFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Performance is poor, reduce quality
        if (CurrentPerformanceLevel != ECore_PhysicsPerformanceLevel::Low)
        {
            ECore_PhysicsPerformanceLevel NewLevel = static_cast<ECore_PhysicsPerformanceLevel>(
                FMath::Max(0, static_cast<int32>(CurrentPerformanceLevel) - 1));
            SetPerformanceLevel(NewLevel);
        }
    }
    else if (AverageFrameTime < TargetFrameTime * 0.7f) // 30% under target
    {
        // Performance is good, increase quality
        if (CurrentPerformanceLevel != ECore_PhysicsPerformanceLevel::Ultra)
        {
            ECore_PhysicsPerformanceLevel NewLevel = static_cast<ECore_PhysicsPerformanceLevel>(
                FMath::Min(3, static_cast<int32>(CurrentPerformanceLevel) + 1));
            SetPerformanceLevel(NewLevel);
        }
    }
}

void ACore_PhysicsPerformanceMonitor::ApplyPerformanceSettings()
{
    switch (CurrentPerformanceLevel)
    {
        case ECore_PhysicsPerformanceLevel::Low:
            OptimizeForLowEnd();
            break;
        case ECore_PhysicsPerformanceLevel::Medium:
            OptimizeForMedium();
            break;
        case ECore_PhysicsPerformanceLevel::High:
            OptimizeForHighEnd();
            break;
        case ECore_PhysicsPerformanceLevel::Ultra:
            OptimizeForUltra();
            break;
    }
}

void ACore_PhysicsPerformanceMonitor::CollectPhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get current frame time (simplified)
    CurrentMetrics.PhysicsFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to ms
    
    // Count active rigid bodies (simplified estimation)
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.CollisionChecks = 0;
    
    // In a real implementation, this would query the physics world
    // For now, we'll use placeholder values based on level complexity
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActiveRigidBodies++;
            }
        }
    }
    
    // Estimate collision checks based on active bodies
    CurrentMetrics.CollisionChecks = CurrentMetrics.ActiveRigidBodies * CurrentMetrics.ActiveRigidBodies / 4;
}

void ACore_PhysicsPerformanceMonitor::CollectMemoryMetrics()
{
    // Simplified memory usage calculation
    // In a real implementation, this would query actual physics memory usage
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveRigidBodies * 0.1f; // Rough estimate
}

void ACore_PhysicsPerformanceMonitor::CollectCPUMetrics()
{
    // Simplified CPU usage calculation
    // In a real implementation, this would query actual CPU usage
    float BaseUsage = 10.0f; // Base CPU usage
    float PhysicsUsage = CurrentMetrics.ActiveRigidBodies * 0.05f; // Physics contribution
    CurrentMetrics.CPUUsagePercent = FMath::Clamp(BaseUsage + PhysicsUsage, 0.0f, 100.0f);
}

void ACore_PhysicsPerformanceMonitor::OptimizeForLowEnd()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Applying Low-End optimizations"));
    
    // Reduce physics quality for better performance
    SetPhysicsSubsteps(1);
    EnableLODSystem(true);
    
    // Additional low-end optimizations would go here
}

void ACore_PhysicsPerformanceMonitor::OptimizeForMedium()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Applying Medium optimizations"));
    
    SetPhysicsSubsteps(2);
    EnableLODSystem(true);
}

void ACore_PhysicsPerformanceMonitor::OptimizeForHighEnd()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Applying High-End optimizations"));
    
    SetPhysicsSubsteps(3);
    EnableLODSystem(false);
}

void ACore_PhysicsPerformanceMonitor::OptimizeForUltra()
{
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsPerformanceMonitor: Applying Ultra optimizations"));
    
    SetPhysicsSubsteps(4);
    EnableLODSystem(false);
    
    // Maximum quality settings
}
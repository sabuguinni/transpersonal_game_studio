#include "Perf_VehiclePhysicsOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/StatsHierarchical.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"

// Console variables for runtime performance tuning
TAutoConsoleVariable<float> UPerf_VehiclePhysicsOptimizer::CVarVehicleMaxDistance(
    TEXT("perf.VehicleMaxDistance"),
    5000.0f,
    TEXT("Maximum distance for vehicle physics simulation"),
    ECVF_Default
);

TAutoConsoleVariable<int32> UPerf_VehiclePhysicsOptimizer::CVarMaxActiveVehicles(
    TEXT("perf.MaxActiveVehicles"),
    10,
    TEXT("Maximum number of active vehicles with full physics"),
    ECVF_Default
);

TAutoConsoleVariable<float> UPerf_VehiclePhysicsOptimizer::CVarVehiclePhysicsRate(
    TEXT("perf.VehiclePhysicsRate"),
    60.0f,
    TEXT("Vehicle physics update rate in Hz"),
    ECVF_Default
);

UPerf_VehiclePhysicsOptimizer::UPerf_VehiclePhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize performance targets
    TargetFrameTime = 16.67f; // 60 FPS
    MaxVehiclePhysicsTime = 5.0f; // 5ms budget for vehicle physics
    MaxMemoryUsageMB = 100.0f;
    
    // Initialize optimization settings
    OptimizationSettings.MaxVehicleDistance = 5000.0f;
    OptimizationSettings.MaxActiveVehicles = 10;
    OptimizationSettings.PhysicsUpdateRate = 60.0f;
    OptimizationSettings.LODDistanceThreshold = 2000.0f;
    OptimizationSettings.bEnableDistanceCulling = true;
    OptimizationSettings.bEnableLODOptimization = true;
}

void UPerf_VehiclePhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizer: BeginPlay"));
    
    // Start performance monitoring automatically
    StartPerformanceMonitoring();
    
    // Initialize vehicle tracking
    UpdateVehicleDistances();
    
    // Apply initial optimizations
    OptimizeVehiclePhysics();
}

void UPerf_VehiclePhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMonitoringActive)
    {
        MonitoringDuration += DeltaTime;
        UpdatePerformanceMetrics(DeltaTime);
        
        // Run optimization every 2 seconds
        if (MonitoringDuration - LastOptimizationTime >= 2.0f)
        {
            OptimizeVehiclePhysics();
            LastOptimizationTime = MonitoringDuration;
        }
    }
}

void UPerf_VehiclePhysicsOptimizer::StartPerformanceMonitoring()
{
    bMonitoringActive = true;
    MonitoringDuration = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizer: Performance monitoring started"));
}

void UPerf_VehiclePhysicsOptimizer::StopPerformanceMonitoring()
{
    bMonitoringActive = false;
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizer: Performance monitoring stopped"));
}

void UPerf_VehiclePhysicsOptimizer::UpdatePerformanceMetrics(float DeltaTime)
{
    // Update frame time tracking
    AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameCount++;
    
    if (FrameCount >= 60) // Update metrics every 60 frames
    {
        CurrentMetrics.AverageFrameTime = AccumulatedFrameTime / FrameCount;
        AccumulatedFrameTime = 0.0f;
        FrameCount = 0;
        
        // Update vehicle count
        CurrentMetrics.ActiveVehicleCount = TrackedVehicles.Num();
        
        // Estimate vehicle physics time (simplified calculation)
        CurrentMetrics.VehiclePhysicsTime = CurrentMetrics.ActiveVehicleCount * 0.5f; // 0.5ms per vehicle estimate
        
        // Check if performance target is met
        CurrentMetrics.bPerformanceTargetMet = 
            (CurrentMetrics.AverageFrameTime <= TargetFrameTime) &&
            (CurrentMetrics.VehiclePhysicsTime <= MaxVehiclePhysicsTime);
        
        // Memory usage estimation (simplified)
        CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActiveVehicleCount * 2.0f; // 2MB per vehicle estimate
        
        // CPU usage estimation
        CurrentMetrics.CPUUsagePercent = (CurrentMetrics.VehiclePhysicsTime / TargetFrameTime) * 100.0f;
    }
}

void UPerf_VehiclePhysicsOptimizer::OptimizeVehiclePhysics()
{
    SCOPE_CYCLE_COUNTER(STAT_VehiclePhysicsOptimization);
    
    // Update vehicle tracking
    UpdateVehicleDistances();
    
    // Apply distance-based optimizations
    if (OptimizationSettings.bEnableDistanceCulling)
    {
        OptimizeVehiclesByDistance();
    }
    
    // Apply LOD optimizations
    if (OptimizationSettings.bEnableLODOptimization)
    {
        UpdateVehicleLOD();
    }
    
    // Apply performance-based optimizations
    if (!CurrentMetrics.bPerformanceTargetMet)
    {
        OptimizeVehiclesByPerformance();
    }
    
    // Update physics rate if needed
    ManageVehiclePhysicsRate();
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Vehicle Physics Optimization: %d vehicles optimized"), TrackedVehicles.Num());
}

void UPerf_VehiclePhysicsOptimizer::UpdateVehicleDistances()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Clear previous tracking
    TrackedVehicles.Empty();
    VehicleDistances.Empty();
    
    // Find all vehicle actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("Vehicle")))
        {
            float Distance = CalculateDistanceToPlayer(Actor);
            
            TrackedVehicles.Add(Actor);
            VehicleDistances.Add(Distance);
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Vehicle Distance Update: Tracking %d vehicles"), TrackedVehicles.Num());
}

void UPerf_VehiclePhysicsOptimizer::OptimizeVehiclesByDistance()
{
    for (int32 i = 0; i < TrackedVehicles.Num(); i++)
    {
        AActor* Vehicle = TrackedVehicles[i];
        float Distance = VehicleDistances[i];
        
        if (Distance > OptimizationSettings.MaxVehicleDistance)
        {
            // Disable physics for very distant vehicles
            SetVehicleOptimizationLevel(Vehicle, 0);
        }
        else if (Distance > OptimizationSettings.LODDistanceThreshold)
        {
            // Reduce physics quality for distant vehicles
            SetVehicleOptimizationLevel(Vehicle, 1);
        }
        else
        {
            // Full physics for nearby vehicles
            SetVehicleOptimizationLevel(Vehicle, 2);
        }
    }
}

void UPerf_VehiclePhysicsOptimizer::OptimizeVehiclesByPerformance()
{
    // If performance is poor, reduce active vehicle count
    if (CurrentMetrics.ActiveVehicleCount > OptimizationSettings.MaxActiveVehicles)
    {
        // Sort vehicles by distance and deactivate furthest ones
        TArray<TPair<float, AActor*>> SortedVehicles;
        
        for (int32 i = 0; i < TrackedVehicles.Num(); i++)
        {
            SortedVehicles.Add(TPair<float, AActor*>(VehicleDistances[i], TrackedVehicles[i]));
        }
        
        // Sort by distance (furthest first)
        SortedVehicles.Sort([](const TPair<float, AActor*>& A, const TPair<float, AActor*>& B)
        {
            return A.Key > B.Key;
        });
        
        // Deactivate excess vehicles
        int32 VehiclesToDeactivate = CurrentMetrics.ActiveVehicleCount - OptimizationSettings.MaxActiveVehicles;
        for (int32 i = 0; i < VehiclesToDeactivate && i < SortedVehicles.Num(); i++)
        {
            SetVehicleOptimizationLevel(SortedVehicles[i].Value, 0);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance Optimization: Deactivated %d vehicles"), VehiclesToDeactivate);
    }
}

void UPerf_VehiclePhysicsOptimizer::UpdateVehicleLOD()
{
    for (int32 i = 0; i < TrackedVehicles.Num(); i++)
    {
        AActor* Vehicle = TrackedVehicles[i];
        float Distance = VehicleDistances[i];
        
        // Apply LOD based on distance
        if (Distance < 500.0f)
        {
            // High detail LOD
            SetVehicleOptimizationLevel(Vehicle, 3);
        }
        else if (Distance < 1500.0f)
        {
            // Medium detail LOD
            SetVehicleOptimizationLevel(Vehicle, 2);
        }
        else if (Distance < OptimizationSettings.LODDistanceThreshold)
        {
            // Low detail LOD
            SetVehicleOptimizationLevel(Vehicle, 1);
        }
        else
        {
            // Minimal/No LOD
            SetVehicleOptimizationLevel(Vehicle, 0);
        }
    }
}

void UPerf_VehiclePhysicsOptimizer::ManageVehiclePhysicsRate()
{
    // Adjust physics update rate based on performance
    float TargetRate = OptimizationSettings.PhysicsUpdateRate;
    
    if (!CurrentMetrics.bPerformanceTargetMet)
    {
        // Reduce physics rate if performance is poor
        TargetRate *= 0.75f; // 25% reduction
        TargetRate = FMath::Max(TargetRate, 30.0f); // Minimum 30 Hz
    }
    
    // Apply the rate (this would typically involve setting physics substep settings)
    UE_LOG(LogTemp, VeryVerbose, TEXT("Vehicle Physics Rate: %.1f Hz"), TargetRate);
}

float UPerf_VehiclePhysicsOptimizer::CalculateDistanceToPlayer(AActor* Vehicle) const
{
    UWorld* World = GetWorld();
    if (!World || !Vehicle) return 99999.0f;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return 99999.0f;
    
    return FVector::Dist(Vehicle->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_VehiclePhysicsOptimizer::SetVehicleOptimizationLevel(AActor* Vehicle, int32 Level)
{
    if (!Vehicle) return;
    
    // This would typically involve setting properties on the vehicle's physics component
    // For now, we'll just log the optimization level
    UE_LOG(LogTemp, VeryVerbose, TEXT("Vehicle %s optimization level set to %d"), 
           *Vehicle->GetName(), Level);
    
    // In a real implementation, this might involve:
    // - Setting physics simulation quality
    // - Adjusting collision complexity
    // - Modifying update rates
    // - Enabling/disabling certain physics features
}

bool UPerf_VehiclePhysicsOptimizer::ShouldCullVehicle(AActor* Vehicle) const
{
    if (!Vehicle) return true;
    
    float Distance = CalculateDistanceToPlayer(Vehicle);
    return Distance > OptimizationSettings.MaxVehicleDistance;
}

void UPerf_VehiclePhysicsOptimizer::SetOptimizationSettings(const FPerf_VehicleOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizer: Settings updated"));
}

void UPerf_VehiclePhysicsOptimizer::EnableVehicleLOD(bool bEnable)
{
    OptimizationSettings.bEnableLODOptimization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Vehicle LOD Optimization: %s"), bEnable ? TEXT("Enabled") : TEXT("Disabled"));
}

void UPerf_VehiclePhysicsOptimizer::SetMaxActiveVehicles(int32 MaxCount)
{
    OptimizationSettings.MaxActiveVehicles = FMath::Max(1, MaxCount);
    UE_LOG(LogTemp, Log, TEXT("Max Active Vehicles set to: %d"), OptimizationSettings.MaxActiveVehicles);
}

void UPerf_VehiclePhysicsOptimizer::CullDistantVehicles()
{
    int32 CulledCount = 0;
    
    for (AActor* Vehicle : TrackedVehicles)
    {
        if (ShouldCullVehicle(Vehicle))
        {
            SetVehicleOptimizationLevel(Vehicle, 0);
            CulledCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Distance Culling: %d vehicles culled"), CulledCount);
}

void UPerf_VehiclePhysicsOptimizer::ScaleVehicleQuality(float QualityScale)
{
    QualityScale = FMath::Clamp(QualityScale, 0.1f, 2.0f);
    
    // Apply quality scaling to all tracked vehicles
    for (AActor* Vehicle : TrackedVehicles)
    {
        // This would typically involve scaling various quality parameters
        UE_LOG(LogTemp, VeryVerbose, TEXT("Vehicle %s quality scaled to %.2f"), 
               *Vehicle->GetName(), QualityScale);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Quality scaled to: %.2f"), QualityScale);
}

void UPerf_VehiclePhysicsOptimizer::SetVehiclePhysicsUpdateRate(float UpdateRate)
{
    OptimizationSettings.PhysicsUpdateRate = FMath::Clamp(UpdateRate, 10.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Update Rate set to: %.1f Hz"), OptimizationSettings.PhysicsUpdateRate);
}

void UPerf_VehiclePhysicsOptimizer::RunPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("=== VEHICLE PHYSICS PERFORMANCE TEST ==="));
    
    // Update tracking and run optimization
    UpdateVehicleDistances();
    OptimizeVehiclePhysics();
    
    // Log current metrics
    LogCurrentPerformance();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE TEST COMPLETE ==="));
}

void UPerf_VehiclePhysicsOptimizer::LogCurrentPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Current Performance Metrics:"));
    UE_LOG(LogTemp, Warning, TEXT("  Frame Time: %.2f ms (Target: %.2f ms)"), 
           CurrentMetrics.AverageFrameTime, TargetFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("  Vehicle Physics Time: %.2f ms (Budget: %.2f ms)"), 
           CurrentMetrics.VehiclePhysicsTime, MaxVehiclePhysicsTime);
    UE_LOG(LogTemp, Warning, TEXT("  Active Vehicles: %d (Max: %d)"), 
           CurrentMetrics.ActiveVehicleCount, OptimizationSettings.MaxActiveVehicles);
    UE_LOG(LogTemp, Warning, TEXT("  Memory Usage: %.1f MB (Max: %.1f MB)"), 
           CurrentMetrics.MemoryUsageMB, MaxMemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("  CPU Usage: %.1f%%"), CurrentMetrics.CPUUsagePercent);
    UE_LOG(LogTemp, Warning, TEXT("  Performance Target Met: %s"), 
           CurrentMetrics.bPerformanceTargetMet ? TEXT("YES") : TEXT("NO"));
}

void UPerf_VehiclePhysicsOptimizer::ResetOptimizations()
{
    // Reset all vehicles to full quality
    for (AActor* Vehicle : TrackedVehicles)
    {
        SetVehicleOptimizationLevel(Vehicle, 3);
    }
    
    // Reset settings to defaults
    OptimizationSettings = FPerf_VehicleOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizations Reset"));
}
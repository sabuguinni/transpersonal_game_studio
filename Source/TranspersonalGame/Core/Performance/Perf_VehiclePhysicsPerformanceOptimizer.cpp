#include "Perf_VehiclePhysicsPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UPerf_VehiclePhysicsPerformanceOptimizer::UPerf_VehiclePhysicsPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    OptimizationSettings = FPerf_VehicleOptimizationSettings();
    CurrentOptimizationLevel = EPerf_VehicleOptimizationLevel::High;
    bEnableRealTimeOptimization = true;
    MetricsUpdateInterval = 0.5f;
    
    // Initialize runtime state
    bIsMonitoring = false;
    bIsProfiling = false;
    LastUpdateTime = 0.0f;
    FrameCounter = 0;
    
    // Initialize metrics
    CurrentMetrics = FPerf_VehiclePhysicsMetrics();
    MetricsHistory.Reserve(100); // Store last 100 metric samples
}

void UPerf_VehiclePhysicsPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Optimizer initialized"));
    
    // Start monitoring by default
    StartPerformanceMonitoring();
    
    // Set up metrics update timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            MetricsUpdateTimer,
            this,
            &UPerf_VehiclePhysicsPerformanceOptimizer::UpdatePerformanceMetrics,
            MetricsUpdateInterval,
            true
        );
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        FrameCounter++;
        LastUpdateTime += DeltaTime;
        
        // Update vehicle metrics
        UpdateVehicleMetrics();
        
        // Apply real-time optimizations if enabled
        if (bEnableRealTimeOptimization)
        {
            OptimizeVehiclePhysics();
        }
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    FrameCounter = 0;
    LastUpdateTime = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Monitoring started"));
}

void UPerf_VehiclePhysicsPerformanceOptimizer::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Monitoring stopped"));
}

FPerf_VehiclePhysicsMetrics UPerf_VehiclePhysicsPerformanceOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_VehiclePhysicsPerformanceOptimizer::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
        return;
    
    // Calculate current metrics
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    CurrentMetrics.PhysicsUpdateTime = GetPhysicsUpdateTime();
    CurrentMetrics.ActiveVehicleCount = GetAllVehicleActors().Num();
    CurrentMetrics.MemoryUsageMB = GetMemoryUsage();
    CurrentMetrics.CPUUsagePercent = GetCPUUsage();
    CurrentMetrics.bIsPerformanceOptimal = IsPerformanceOptimal();
    
    // Calculate detailed physics metrics
    CalculatePhysicsPerformance();
    
    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > 100)
    {
        MetricsHistory.RemoveAt(0);
    }
    
    // Log performance data if profiling
    if (bIsProfiling)
    {
        LogPerformanceData();
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::SetOptimizationLevel(EPerf_VehicleOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    // Adjust settings based on optimization level
    switch (Level)
    {
        case EPerf_VehicleOptimizationLevel::Ultra:
            OptimizationSettings.MaxPhysicsUpdateRate = 120.0f;
            OptimizationSettings.LODDistanceThreshold = 5000.0f;
            OptimizationSettings.MaxActiveVehicles = 20;
            OptimizationSettings.bEnableAdaptiveLOD = false;
            OptimizationSettings.bEnablePhysicsCulling = false;
            break;
            
        case EPerf_VehicleOptimizationLevel::High:
            OptimizationSettings.MaxPhysicsUpdateRate = 90.0f;
            OptimizationSettings.LODDistanceThreshold = 3000.0f;
            OptimizationSettings.MaxActiveVehicles = 15;
            OptimizationSettings.bEnableAdaptiveLOD = true;
            OptimizationSettings.bEnablePhysicsCulling = true;
            break;
            
        case EPerf_VehicleOptimizationLevel::Medium:
            OptimizationSettings.MaxPhysicsUpdateRate = 60.0f;
            OptimizationSettings.LODDistanceThreshold = 2000.0f;
            OptimizationSettings.MaxActiveVehicles = 10;
            OptimizationSettings.bEnableAdaptiveLOD = true;
            OptimizationSettings.bEnablePhysicsCulling = true;
            break;
            
        case EPerf_VehicleOptimizationLevel::Low:
            OptimizationSettings.MaxPhysicsUpdateRate = 30.0f;
            OptimizationSettings.LODDistanceThreshold = 1000.0f;
            OptimizationSettings.MaxActiveVehicles = 5;
            OptimizationSettings.bEnableAdaptiveLOD = true;
            OptimizationSettings.bEnablePhysicsCulling = true;
            break;
            
        case EPerf_VehicleOptimizationLevel::Minimal:
            OptimizationSettings.MaxPhysicsUpdateRate = 15.0f;
            OptimizationSettings.LODDistanceThreshold = 500.0f;
            OptimizationSettings.MaxActiveVehicles = 3;
            OptimizationSettings.bEnableAdaptiveLOD = true;
            OptimizationSettings.bEnablePhysicsCulling = true;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimization Level set to: %d"), (int32)Level);
}

void UPerf_VehiclePhysicsPerformanceOptimizer::ApplyOptimizationSettings(const FPerf_VehicleOptimizationSettings& Settings)
{
    OptimizationSettings = Settings;
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimization Settings applied"));
}

void UPerf_VehiclePhysicsPerformanceOptimizer::OptimizeVehiclePhysics()
{
    if (!bIsMonitoring)
        return;
    
    // Apply different optimization strategies
    if (OptimizationSettings.bEnableAdaptiveLOD)
    {
        UpdateAdaptiveLOD();
    }
    
    if (OptimizationSettings.bEnablePhysicsCulling)
    {
        UpdatePhysicsCulling();
    }
    
    // Adjust physics update rate based on performance
    if (CurrentMetrics.AverageFrameTime > (1000.0f / OptimizationSettings.TargetFrameRate))
    {
        AdjustPhysicsUpdateRate(CurrentMetrics.AverageFrameTime);
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::ResetOptimizations()
{
    // Reset to default high-quality settings
    SetOptimizationLevel(EPerf_VehicleOptimizationLevel::Ultra);
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Optimizations reset to default"));
}

void UPerf_VehiclePhysicsPerformanceOptimizer::UpdateAdaptiveLOD()
{
    TArray<AActor*> VehicleActors = GetAllVehicleActors();
    
    for (AActor* Vehicle : VehicleActors)
    {
        if (!Vehicle)
            continue;
        
        // Calculate distance to player
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (!PlayerPawn)
            continue;
        
        float Distance = FVector::Dist(Vehicle->GetActorLocation(), PlayerPawn->GetActorLocation());
        
        // Apply LOD based on distance
        if (Distance > OptimizationSettings.LODDistanceThreshold * 2.0f)
        {
            // Very far - minimal detail
            Vehicle->SetActorTickEnabled(false);
        }
        else if (Distance > OptimizationSettings.LODDistanceThreshold)
        {
            // Far - reduced detail
            Vehicle->SetActorTickEnabled(true);
            Vehicle->SetActorTickInterval(0.1f); // 10 FPS
        }
        else
        {
            // Near - full detail
            Vehicle->SetActorTickEnabled(true);
            Vehicle->SetActorTickInterval(0.0f); // Full FPS
        }
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::UpdatePhysicsCulling()
{
    TArray<AActor*> VehicleActors = GetAllVehicleActors();
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    for (AActor* Vehicle : VehicleActors)
    {
        if (!Vehicle)
            continue;
        
        float Distance = FVector::Dist(Vehicle->GetActorLocation(), PlayerPawn->GetActorLocation());
        
        // Disable physics for very distant vehicles
        if (Distance > OptimizationSettings.CullingDistance)
        {
            UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Vehicle->GetRootComponent());
            if (RootPrimitive)
            {
                RootPrimitive->SetSimulatePhysics(false);
            }
        }
        else
        {
            UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Vehicle->GetRootComponent());
            if (RootPrimitive)
            {
                RootPrimitive->SetSimulatePhysics(true);
            }
        }
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::AdjustPhysicsUpdateRate(float TargetFrameTime)
{
    // Reduce physics update rate if performance is poor
    float CurrentFPS = 1000.0f / TargetFrameTime;
    float TargetFPS = OptimizationSettings.TargetFrameRate;
    
    if (CurrentFPS < TargetFPS * 0.8f) // If FPS is 20% below target
    {
        // Reduce physics update rate
        OptimizationSettings.MaxPhysicsUpdateRate = FMath::Max(15.0f, OptimizationSettings.MaxPhysicsUpdateRate * 0.9f);
        UE_LOG(LogTemp, Warning, TEXT("Reducing vehicle physics update rate to: %f"), OptimizationSettings.MaxPhysicsUpdateRate);
    }
    else if (CurrentFPS > TargetFPS * 1.1f) // If FPS is 10% above target
    {
        // Increase physics update rate
        OptimizationSettings.MaxPhysicsUpdateRate = FMath::Min(120.0f, OptimizationSettings.MaxPhysicsUpdateRate * 1.05f);
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::StartProfiling()
{
    bIsProfiling = true;
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Profiling started"));
}

void UPerf_VehiclePhysicsPerformanceOptimizer::StopProfiling()
{
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Profiling stopped"));
}

void UPerf_VehiclePhysicsPerformanceOptimizer::SavePerformanceReport()
{
    TArray<FString> Report = GetPerformanceReport();
    
    FString ReportText = FString::Join(Report, TEXT("\n"));
    FString FileName = FString::Printf(TEXT("VehiclePhysicsPerformanceReport_%s.txt"), 
        *FDateTime::Now().ToString());
    
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports") / FileName;
    
    if (FFileHelper::SaveStringToFile(ReportText, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance Report saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to save Vehicle Physics Performance Report"));
    }
}

TArray<FString> UPerf_VehiclePhysicsPerformanceOptimizer::GetPerformanceReport() const
{
    TArray<FString> Report;
    
    Report.Add(TEXT("=== VEHICLE PHYSICS PERFORMANCE REPORT ==="));
    Report.Add(FString::Printf(TEXT("Generated: %s"), *FDateTime::Now().ToString()));
    Report.Add(TEXT(""));
    
    Report.Add(TEXT("Current Metrics:"));
    Report.Add(FString::Printf(TEXT("  Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime));
    Report.Add(FString::Printf(TEXT("  Physics Update Time: %.2f ms"), CurrentMetrics.PhysicsUpdateTime));
    Report.Add(FString::Printf(TEXT("  Suspension Calculation Time: %.2f ms"), CurrentMetrics.SuspensionCalculationTime));
    Report.Add(FString::Printf(TEXT("  Tire Physics Time: %.2f ms"), CurrentMetrics.TirePhysicsTime));
    Report.Add(FString::Printf(TEXT("  Collision Detection Time: %.2f ms"), CurrentMetrics.CollisionDetectionTime));
    Report.Add(FString::Printf(TEXT("  Active Vehicle Count: %d"), CurrentMetrics.ActiveVehicleCount));
    Report.Add(FString::Printf(TEXT("  Physics Body Count: %d"), CurrentMetrics.PhysicsBodyCount));
    Report.Add(FString::Printf(TEXT("  Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB));
    Report.Add(FString::Printf(TEXT("  CPU Usage: %.2f%%"), CurrentMetrics.CPUUsagePercent));
    Report.Add(FString::Printf(TEXT("  Performance Optimal: %s"), CurrentMetrics.bIsPerformanceOptimal ? TEXT("Yes") : TEXT("No")));
    Report.Add(TEXT(""));
    
    Report.Add(TEXT("Optimization Settings:"));
    Report.Add(FString::Printf(TEXT("  Optimization Level: %d"), (int32)CurrentOptimizationLevel));
    Report.Add(FString::Printf(TEXT("  Max Physics Update Rate: %.2f Hz"), OptimizationSettings.MaxPhysicsUpdateRate));
    Report.Add(FString::Printf(TEXT("  LOD Distance Threshold: %.2f"), OptimizationSettings.LODDistanceThreshold));
    Report.Add(FString::Printf(TEXT("  Max Active Vehicles: %d"), OptimizationSettings.MaxActiveVehicles));
    Report.Add(FString::Printf(TEXT("  Adaptive LOD Enabled: %s"), OptimizationSettings.bEnableAdaptiveLOD ? TEXT("Yes") : TEXT("No")));
    Report.Add(FString::Printf(TEXT("  Physics Culling Enabled: %s"), OptimizationSettings.bEnablePhysicsCulling ? TEXT("Yes") : TEXT("No")));
    Report.Add(FString::Printf(TEXT("  Culling Distance: %.2f"), OptimizationSettings.CullingDistance));
    Report.Add(FString::Printf(TEXT("  Async Physics Enabled: %s"), OptimizationSettings.bEnableAsyncPhysics ? TEXT("Yes") : TEXT("No")));
    Report.Add(FString::Printf(TEXT("  Target Frame Rate: %.2f FPS"), OptimizationSettings.TargetFrameRate));
    
    return Report;
}

void UPerf_VehiclePhysicsPerformanceOptimizer::UpdateVehicleMetrics()
{
    // Update basic metrics
    CurrentMetrics.ActiveVehicleCount = GetAllVehicleActors().Num();
    
    // Count physics bodies
    CurrentMetrics.PhysicsBodyCount = 0;
    TArray<AActor*> VehicleActors = GetAllVehicleActors();
    for (AActor* Vehicle : VehicleActors)
    {
        if (Vehicle)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            for (UPrimitiveComponent* Primitive : PrimitiveComponents)
            {
                if (Primitive && Primitive->IsSimulatingPhysics())
                {
                    CurrentMetrics.PhysicsBodyCount++;
                }
            }
        }
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::CalculatePhysicsPerformance()
{
    // Simulate physics timing calculations
    // In a real implementation, these would be measured using actual profiling
    CurrentMetrics.SuspensionCalculationTime = CurrentMetrics.PhysicsUpdateTime * 0.3f;
    CurrentMetrics.TirePhysicsTime = CurrentMetrics.PhysicsUpdateTime * 0.4f;
    CurrentMetrics.CollisionDetectionTime = CurrentMetrics.PhysicsUpdateTime * 0.3f;
}

void UPerf_VehiclePhysicsPerformanceOptimizer::ApplyLODOptimizations()
{
    UpdateAdaptiveLOD();
}

void UPerf_VehiclePhysicsPerformanceOptimizer::ApplyCullingOptimizations()
{
    UpdatePhysicsCulling();
}

void UPerf_VehiclePhysicsPerformanceOptimizer::ApplyQualityOptimizations()
{
    // Apply quality-based optimizations based on current performance
    if (!CurrentMetrics.bIsPerformanceOptimal)
    {
        // Reduce quality settings
        if (CurrentOptimizationLevel != EPerf_VehicleOptimizationLevel::Minimal)
        {
            EPerf_VehicleOptimizationLevel NewLevel = static_cast<EPerf_VehicleOptimizationLevel>(
                static_cast<int32>(CurrentOptimizationLevel) + 1);
            SetOptimizationLevel(NewLevel);
        }
    }
}

void UPerf_VehiclePhysicsPerformanceOptimizer::LogPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Performance - Frame: %.2f ms, Physics: %.2f ms, Vehicles: %d"), 
        CurrentMetrics.AverageFrameTime, CurrentMetrics.PhysicsUpdateTime, CurrentMetrics.ActiveVehicleCount);
}

TArray<AActor*> UPerf_VehiclePhysicsPerformanceOptimizer::GetAllVehicleActors() const
{
    TArray<AActor*> VehicleActors;
    
    if (GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Vehicle")))
            {
                VehicleActors.Add(Actor);
            }
        }
    }
    
    return VehicleActors;
}

float UPerf_VehiclePhysicsPerformanceOptimizer::CalculateAverageFrameTime() const
{
    if (FrameCounter > 0 && LastUpdateTime > 0.0f)
    {
        return (LastUpdateTime / FrameCounter) * 1000.0f; // Convert to milliseconds
    }
    return 0.0f;
}

float UPerf_VehiclePhysicsPerformanceOptimizer::GetPhysicsUpdateTime() const
{
    // Simulate physics update time based on active vehicles
    return CurrentMetrics.ActiveVehicleCount * 0.5f + FMath::RandRange(0.1f, 2.0f);
}

float UPerf_VehiclePhysicsPerformanceOptimizer::GetMemoryUsage() const
{
    // Simulate memory usage calculation
    return CurrentMetrics.ActiveVehicleCount * 5.0f + CurrentMetrics.PhysicsBodyCount * 0.5f;
}

float UPerf_VehiclePhysicsPerformanceOptimizer::GetCPUUsage() const
{
    // Simulate CPU usage based on performance metrics
    float BaseUsage = CurrentMetrics.AverageFrameTime * 0.1f;
    float PhysicsUsage = CurrentMetrics.PhysicsUpdateTime * 0.2f;
    return FMath::Clamp(BaseUsage + PhysicsUsage, 0.0f, 100.0f);
}

bool UPerf_VehiclePhysicsPerformanceOptimizer::IsPerformanceOptimal() const
{
    float TargetFrameTime = 1000.0f / OptimizationSettings.TargetFrameRate;
    return CurrentMetrics.AverageFrameTime <= TargetFrameTime * 1.1f; // 10% tolerance
}
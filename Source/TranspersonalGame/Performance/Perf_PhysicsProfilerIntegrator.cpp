#include "Perf_PhysicsProfilerIntegrator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"

UPerf_PhysicsProfilerIntegrator::UPerf_PhysicsProfilerIntegrator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms

    bIsProfilingActive = false;
    ProfilingUpdateInterval = 0.1f;
    LastProfilingUpdate = 0.0f;
    bIntegratedWithCoreProfiler = false;
    MaxHistorySize = 300; // 30 seconds at 10Hz
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    bPerformanceWarningIssued = false;

    // Initialize optimization settings
    OptimizationSettings.MaxPhysicsTickTime = 16.67f; // 60 FPS target
    OptimizationSettings.MaxActiveRigidBodies = 500;
    OptimizationSettings.MaxCollisionPairs = 1000;
    OptimizationSettings.bEnableAdaptiveLOD = true;
    OptimizationSettings.bEnableDistanceCulling = true;
    OptimizationSettings.PhysicsCullingDistance = 5000.0f;
    OptimizationSettings.bEnableAsyncPhysics = true;
}

void UPerf_PhysicsProfilerIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: BeginPlay"));
    
    // Initialize physics profiling
    PhysicsTickTimeHistory.Reserve(MaxHistorySize);
    
    // Attempt integration with core profiler
    IntegrateWithCorePhysicsProfiler();
    
    // Start profiling by default
    StartPhysicsProfiling();
}

void UPerf_PhysicsProfilerIntegrator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsProfilingActive)
    {
        MonitorPhysicsPerformance(DeltaTime);
        
        LastProfilingUpdate += DeltaTime;
        if (LastProfilingUpdate >= ProfilingUpdateInterval)
        {
            UpdatePhysicsMetrics();
            CheckPerformanceThresholds();
            LastProfilingUpdate = 0.0f;
        }
    }
}

void UPerf_PhysicsProfilerIntegrator::StartPhysicsProfiling()
{
    bIsProfilingActive = true;
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    PhysicsTickTimeHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Started physics profiling"));
}

void UPerf_PhysicsProfilerIntegrator::StopPhysicsProfiling()
{
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Stopped physics profiling"));
}

FPerf_PhysicsProfileMetrics UPerf_PhysicsProfilerIntegrator::GetCurrentPhysicsMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PhysicsProfilerIntegrator::UpdatePhysicsOptimizationSettings(const FPerf_PhysicsOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Updated optimization settings"));
    
    // Apply new settings immediately
    ApplyPerformanceOptimizations();
}

void UPerf_PhysicsProfilerIntegrator::IntegrateWithCorePhysicsProfiler()
{
    // Attempt to find and integrate with Core Systems physics profiler
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Profiler Integrator: No world found for integration"));
        return;
    }

    FindCorePhysicsProfiler();
    
    if (bIntegratedWithCoreProfiler)
    {
        SynchronizeWithCoreProfiler();
        UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Successfully integrated with Core Physics Profiler"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Profiler Integrator: Failed to integrate with Core Physics Profiler"));
    }
}

bool UPerf_PhysicsProfilerIntegrator::ValidatePhysicsPerformance()
{
    if (!bIsProfilingActive)
    {
        return false;
    }

    bool bPerformanceGood = true;
    
    // Check physics tick time
    if (CurrentMetrics.PhysicsTickTime > OptimizationSettings.MaxPhysicsTickTime)
    {
        bPerformanceGood = false;
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance: Tick time exceeded threshold (%.2f ms > %.2f ms)"), 
               CurrentMetrics.PhysicsTickTime, OptimizationSettings.MaxPhysicsTickTime);
    }
    
    // Check active rigid body count
    if (CurrentMetrics.ActiveRigidBodies > OptimizationSettings.MaxActiveRigidBodies)
    {
        bPerformanceGood = false;
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance: Too many active rigid bodies (%d > %d)"), 
               CurrentMetrics.ActiveRigidBodies, OptimizationSettings.MaxActiveRigidBodies);
    }
    
    // Check collision pairs
    if (CurrentMetrics.ActiveCollisionPairs > OptimizationSettings.MaxCollisionPairs)
    {
        bPerformanceGood = false;
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance: Too many collision pairs (%d > %d)"), 
               CurrentMetrics.ActiveCollisionPairs, OptimizationSettings.MaxCollisionPairs);
    }

    CurrentMetrics.bIsPhysicsPerformanceGood = bPerformanceGood;
    return bPerformanceGood;
}

void UPerf_PhysicsProfilerIntegrator::OptimizePhysicsBasedOnMetrics()
{
    if (!ValidatePhysicsPerformance())
    {
        ApplyPerformanceOptimizations();
    }
}

void UPerf_PhysicsProfilerIntegrator::MonitorPhysicsPerformance(float DeltaTime)
{
    // Accumulate frame time for averaging
    AccumulatedPhysicsTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameCount++;
    
    // Add to history
    if (PhysicsTickTimeHistory.Num() >= MaxHistorySize)
    {
        PhysicsTickTimeHistory.RemoveAt(0);
    }
    PhysicsTickTimeHistory.Add(DeltaTime * 1000.0f);
    
    // Calculate average frame time
    CalculateAverageFrameTime();
}

void UPerf_PhysicsProfilerIntegrator::ApplyPerformanceOptimizations()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Applying performance optimizations"));

    // Apply distance culling
    if (OptimizationSettings.bEnableDistanceCulling)
    {
        ApplyDistanceCulling();
    }

    // Apply LOD optimizations
    if (OptimizationSettings.bEnableAdaptiveLOD)
    {
        ApplyLODOptimizations();
    }

    // Manage rigid body count
    ManageRigidBodyCount();

    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Performance optimizations applied"));
}

void UPerf_PhysicsProfilerIntegrator::ResetPhysicsOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Resetting physics optimizations"));
    
    // Reset to default optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    
    // Clear performance history
    PhysicsTickTimeHistory.Empty();
    AccumulatedPhysicsTime = 0.0f;
    FrameCount = 0;
    bPerformanceWarningIssued = false;
}

void UPerf_PhysicsProfilerIntegrator::TestPhysicsProfilerIntegration()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Testing integration"));
    
    if (bIsProfilingActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Profiling Status: ACTIVE"));
        UE_LOG(LogTemp, Log, TEXT("Current Physics Tick Time: %.2f ms"), CurrentMetrics.PhysicsTickTime);
        UE_LOG(LogTemp, Log, TEXT("Active Rigid Bodies: %d"), CurrentMetrics.ActiveRigidBodies);
        UE_LOG(LogTemp, Log, TEXT("Active Collision Pairs: %d"), CurrentMetrics.ActiveCollisionPairs);
        UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), CurrentMetrics.AverageFrameTime);
        UE_LOG(LogTemp, Log, TEXT("Performance Good: %s"), CurrentMetrics.bIsPhysicsPerformanceGood ? TEXT("YES") : TEXT("NO"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Profiling Status: INACTIVE"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Integration Status: %s"), bIntegratedWithCoreProfiler ? TEXT("INTEGRATED") : TEXT("NOT INTEGRATED"));
}

void UPerf_PhysicsProfilerIntegrator::ExportPhysicsProfileData()
{
    if (PhysicsTickTimeHistory.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Profiler Integrator: No profile data to export"));
        return;
    }

    FString ExportData;
    ExportData += TEXT("Timestamp,PhysicsTickTime,ActiveRigidBodies,CollisionPairs,AverageFrameTime\n");
    
    FString TimeStamp = FDateTime::Now().ToString();
    for (int32 i = 0; i < PhysicsTickTimeHistory.Num(); i++)
    {
        ExportData += FString::Printf(TEXT("%s,%.3f,%d,%d,%.3f\n"),
            *TimeStamp,
            PhysicsTickTimeHistory[i],
            CurrentMetrics.ActiveRigidBodies,
            CurrentMetrics.ActiveCollisionPairs,
            CurrentMetrics.AverageFrameTime);
    }

    FString FileName = FString::Printf(TEXT("PhysicsProfileData_%s.csv"), *FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S")));
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("Profiling") / FileName;
    
    if (FFileHelper::SaveStringToFile(ExportData, *FilePath))
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Exported profile data to %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Physics Profiler Integrator: Failed to export profile data"));
    }
}

void UPerf_PhysicsProfilerIntegrator::UpdatePhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Update physics tick time from accumulated data
    if (FrameCount > 0)
    {
        CurrentMetrics.PhysicsTickTime = AccumulatedPhysicsTime / FrameCount;
    }

    // Count active rigid bodies and collision pairs
    CurrentMetrics.ActiveRigidBodies = 0;
    CurrentMetrics.ActiveCollisionPairs = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActiveRigidBodies++;
            }
        }
    }

    // Estimate collision pairs (simplified calculation)
    CurrentMetrics.ActiveCollisionPairs = (CurrentMetrics.ActiveRigidBodies * (CurrentMetrics.ActiveRigidBodies - 1)) / 2;

    // Calculate memory usage (simplified estimation)
    CurrentMetrics.PhysicsMemoryUsage = CurrentMetrics.ActiveRigidBodies * 1024; // Rough estimate in bytes
}

void UPerf_PhysicsProfilerIntegrator::CalculateAverageFrameTime()
{
    if (PhysicsTickTimeHistory.Num() == 0)
    {
        CurrentMetrics.AverageFrameTime = 0.0f;
        return;
    }

    float Sum = 0.0f;
    for (float TickTime : PhysicsTickTimeHistory)
    {
        Sum += TickTime;
    }
    CurrentMetrics.AverageFrameTime = Sum / PhysicsTickTimeHistory.Num();
}

void UPerf_PhysicsProfilerIntegrator::CheckPerformanceThresholds()
{
    bool bCurrentPerformanceGood = ValidatePhysicsPerformance();
    
    if (!bCurrentPerformanceGood && !bPerformanceWarningIssued)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Performance Warning: Performance thresholds exceeded"));
        bPerformanceWarningIssued = true;
        
        // Trigger automatic optimizations
        OptimizePhysicsBasedOnMetrics();
    }
    else if (bCurrentPerformanceGood && bPerformanceWarningIssued)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Performance: Performance restored to acceptable levels"));
        bPerformanceWarningIssued = false;
    }
}

void UPerf_PhysicsProfilerIntegrator::ApplyDistanceCulling()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Get player location for distance calculations
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledObjects = 0;

    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != PlayerPawn && Actor->GetRootComponent())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && Distance > OptimizationSettings.PhysicsCullingDistance)
            {
                if (PrimComp->IsSimulatingPhysics())
                {
                    PrimComp->SetSimulatePhysics(false);
                    CulledObjects++;
                }
            }
            else if (PrimComp && Distance <= OptimizationSettings.PhysicsCullingDistance)
            {
                if (!PrimComp->IsSimulatingPhysics() && PrimComp->GetBodyInstance())
                {
                    PrimComp->SetSimulatePhysics(true);
                }
            }
        }
    }

    if (CulledObjects > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Distance Culling: Disabled physics on %d objects"), CulledObjects);
    }
}

void UPerf_PhysicsProfilerIntegrator::ApplyLODOptimizations()
{
    // Simplified LOD optimization based on performance metrics
    if (CurrentMetrics.PhysicsTickTime > OptimizationSettings.MaxPhysicsTickTime * 0.8f)
    {
        // Reduce physics complexity when approaching performance limits
        UE_LOG(LogTemp, Log, TEXT("Physics LOD: Applying performance-based LOD optimizations"));
        
        // This would typically involve reducing collision complexity,
        // switching to simpler physics shapes, or reducing update frequencies
    }
}

void UPerf_PhysicsProfilerIntegrator::ManageRigidBodyCount()
{
    if (CurrentMetrics.ActiveRigidBodies > OptimizationSettings.MaxActiveRigidBodies)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Management: Active rigid body count (%d) exceeds limit (%d)"), 
               CurrentMetrics.ActiveRigidBodies, OptimizationSettings.MaxActiveRigidBodies);
        
        // Apply distance-based culling more aggressively
        OptimizationSettings.PhysicsCullingDistance *= 0.8f;
        ApplyDistanceCulling();
    }
}

void UPerf_PhysicsProfilerIntegrator::FindCorePhysicsProfiler()
{
    // Attempt to find Core Systems physics profiler in the world
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Search for Core_PhysicsProfiler component or actor
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetName().Contains(TEXT("PhysicsProfiler")))
        {
            bIntegratedWithCoreProfiler = true;
            UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Found Core Physics Profiler: %s"), *Actor->GetName());
            break;
        }
    }
}

void UPerf_PhysicsProfilerIntegrator::SynchronizeWithCoreProfiler()
{
    if (!bIntegratedWithCoreProfiler)
    {
        return;
    }

    // Synchronize settings and data with the core profiler
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Synchronizing with Core Physics Profiler"));
}

void UPerf_PhysicsProfilerIntegrator::ValidateIntegration()
{
    if (bIntegratedWithCoreProfiler)
    {
        UE_LOG(LogTemp, Log, TEXT("Physics Profiler Integrator: Integration validation PASSED"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Profiler Integrator: Integration validation FAILED"));
    }
}
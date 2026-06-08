#include "Core_PhysicsProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "DrawDebugHelpers.h"

UCore_PhysicsProfiler::UCore_PhysicsProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    CurrentFrameTime = 0.0f;
    PhysicsStepTime = 0.0f;
    ActiveRigidBodies = 0;
    SleepingRigidBodies = 0;
    CollisionPairs = 0;
    MemoryUsageMB = 0.0f;
    
    // Configuration defaults
    bEnableProfiling = true;
    ProfilingInterval = 0.1f;
    bLogToConsole = false;
    bDisplayOnScreen = true;
    
    // Performance thresholds
    WarningFrameTime = 16.67f; // 60 FPS threshold
    CriticalFrameTime = 33.33f; // 30 FPS threshold
    MaxActiveRigidBodies = 1000;
    
    LastProfilingTime = 0.0f;
    FrameTimeHistory.Reserve(100);
    RigidBodyCountHistory.Reserve(100);
}

void UCore_PhysicsProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableProfiling)
    {
        StartProfiling();
    }
}

void UCore_PhysicsProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableProfiling)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastProfilingTime >= ProfilingInterval)
    {
        UpdateMetrics();
        AnalyzePerformance();
        
        if (bDisplayOnScreen)
        {
            DisplayMetricsOnScreen();
        }
        
        if (bLogToConsole)
        {
            LogCurrentMetrics();
        }
        
        LastProfilingTime = CurrentTime;
    }
}

void UCore_PhysicsProfiler::StartProfiling()
{
    bEnableProfiling = true;
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler: Started profiling"));
}

void UCore_PhysicsProfiler::StopProfiling()
{
    bEnableProfiling = false;
    
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler: Stopped profiling"));
}

void UCore_PhysicsProfiler::ResetMetrics()
{
    CurrentFrameTime = 0.0f;
    PhysicsStepTime = 0.0f;
    ActiveRigidBodies = 0;
    SleepingRigidBodies = 0;
    CollisionPairs = 0;
    MemoryUsageMB = 0.0f;
    
    FrameTimeHistory.Empty();
    RigidBodyCountHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler: Metrics reset"));
}

FString UCore_PhysicsProfiler::GetPerformanceReport()
{
    FString Report;
    Report += FString::Printf(TEXT("=== PHYSICS PERFORMANCE REPORT ===\n"));
    Report += FString::Printf(TEXT("Frame Time: %.2f ms\n"), CurrentFrameTime);
    Report += FString::Printf(TEXT("Physics Step Time: %.2f ms\n"), PhysicsStepTime);
    Report += FString::Printf(TEXT("Active Rigid Bodies: %d\n"), ActiveRigidBodies);
    Report += FString::Printf(TEXT("Sleeping Rigid Bodies: %d\n"), SleepingRigidBodies);
    Report += FString::Printf(TEXT("Collision Pairs: %d\n"), CollisionPairs);
    Report += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), MemoryUsageMB);
    
    // Performance status
    if (CurrentFrameTime > CriticalFrameTime)
    {
        Report += TEXT("Status: CRITICAL - Performance below 30 FPS\n");
    }
    else if (CurrentFrameTime > WarningFrameTime)
    {
        Report += TEXT("Status: WARNING - Performance below 60 FPS\n");
    }
    else
    {
        Report += TEXT("Status: GOOD - Performance within limits\n");
    }
    
    return Report;
}

bool UCore_PhysicsProfiler::IsPerformanceWithinLimits()
{
    return (CurrentFrameTime <= WarningFrameTime && ActiveRigidBodies <= MaxActiveRigidBodies);
}

void UCore_PhysicsProfiler::LogCurrentMetrics()
{
    UE_LOG(LogTemp, Log, TEXT("Physics Profiler - Frame: %.2fms, Bodies: %d/%d, Collisions: %d"), 
           CurrentFrameTime, ActiveRigidBodies, SleepingRigidBodies, CollisionPairs);
}

void UCore_PhysicsProfiler::ProfilePhysicsStep()
{
    // Profile the physics simulation step
    double StartTime = FPlatformTime::Seconds();
    
    // Physics step happens automatically in UE5, we measure the delta
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    PhysicsStepTime = DeltaTime * 1000.0f; // Convert to milliseconds
    
    double EndTime = FPlatformTime::Seconds();
    float ProfileTime = (EndTime - StartTime) * 1000.0f;
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Physics Step Profile: %.3f ms"), ProfileTime);
}

void UCore_PhysicsProfiler::ProfileCollisionDetection()
{
    // Profile collision detection by counting collision events
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count active collision pairs (approximation)
    CollisionPairs = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsCollisionEnabled())
            {
                CollisionPairs++;
            }
        }
    }
}

void UCore_PhysicsProfiler::ProfileConstraintSolving()
{
    // Profile physics constraint solving
    // This is handled internally by Chaos Physics
    // We can monitor the overall physics performance impact
    
    UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        float MaxSubstepDelta = PhysicsSettings->MaxSubstepDeltaTime;
        int32 MaxSubsteps = PhysicsSettings->MaxSubsteps;
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Constraint Solver Settings - MaxSubstepDelta: %.4f, MaxSubsteps: %d"), 
               MaxSubstepDelta, MaxSubsteps);
    }
}

TArray<FString> UCore_PhysicsProfiler::GetOptimizationSuggestions()
{
    TArray<FString> Suggestions;
    
    if (CurrentFrameTime > CriticalFrameTime)
    {
        Suggestions.Add(TEXT("CRITICAL: Reduce active rigid bodies count"));
        Suggestions.Add(TEXT("CRITICAL: Implement physics LOD system"));
        Suggestions.Add(TEXT("CRITICAL: Increase physics culling distance"));
    }
    else if (CurrentFrameTime > WarningFrameTime)
    {
        Suggestions.Add(TEXT("WARNING: Consider physics optimization"));
        Suggestions.Add(TEXT("WARNING: Monitor rigid body count"));
    }
    
    if (ActiveRigidBodies > MaxActiveRigidBodies)
    {
        Suggestions.Add(TEXT("Too many active rigid bodies - implement culling"));
    }
    
    if (CollisionPairs > 500)
    {
        Suggestions.Add(TEXT("High collision pair count - optimize collision channels"));
    }
    
    if (MemoryUsageMB > 100.0f)
    {
        Suggestions.Add(TEXT("High physics memory usage - consider mesh simplification"));
    }
    
    return Suggestions;
}

void UCore_PhysicsProfiler::ApplyAutoOptimizations()
{
    // Apply automatic optimizations based on current performance
    if (CurrentFrameTime > CriticalFrameTime)
    {
        // Emergency optimizations
        UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
        if (PhysicsSettings)
        {
            // Reduce physics accuracy for better performance
            PhysicsSettings->MaxSubsteps = FMath::Max(1, PhysicsSettings->MaxSubsteps - 1);
            UE_LOG(LogTemp, Warning, TEXT("Auto-optimization: Reduced MaxSubsteps to %d"), PhysicsSettings->MaxSubsteps);
        }
    }
}

void UCore_PhysicsProfiler::UpdateMetrics()
{
    // Update frame time
    CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f;
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > 100)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Count rigid bodies
    ActiveRigidBodies = 0;
    SleepingRigidBodies = 0;
    
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        ActiveRigidBodies++;
                    }
                    else
                    {
                        SleepingRigidBodies++;
                    }
                }
            }
        }
    }
    
    RigidBodyCountHistory.Add(ActiveRigidBodies);
    if (RigidBodyCountHistory.Num() > 100)
    {
        RigidBodyCountHistory.RemoveAt(0);
    }
    
    // Profile collision detection
    ProfileCollisionDetection();
    
    // Estimate memory usage (simplified)
    MemoryUsageMB = (ActiveRigidBodies * 0.1f) + (CollisionPairs * 0.05f);
}

void UCore_PhysicsProfiler::AnalyzePerformance()
{
    // Analyze performance trends
    if (FrameTimeHistory.Num() > 10)
    {
        float AverageFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= FrameTimeHistory.Num();
        
        // Check for performance degradation
        float RecentAverage = 0.0f;
        int32 RecentSamples = FMath::Min(10, FrameTimeHistory.Num());
        for (int32 i = FrameTimeHistory.Num() - RecentSamples; i < FrameTimeHistory.Num(); i++)
        {
            RecentAverage += FrameTimeHistory[i];
        }
        RecentAverage /= RecentSamples;
        
        if (RecentAverage > AverageFrameTime * 1.2f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics Profiler: Performance degradation detected"));
        }
    }
}

void UCore_PhysicsProfiler::DisplayMetricsOnScreen()
{
    if (GEngine)
    {
        FString DisplayText = FString::Printf(
            TEXT("Physics Profiler:\nFrame: %.1fms | Bodies: %d | Collisions: %d | Memory: %.1fMB"),
            CurrentFrameTime, ActiveRigidBodies, CollisionPairs, MemoryUsageMB
        );
        
        FColor DisplayColor = FColor::Green;
        if (CurrentFrameTime > CriticalFrameTime)
        {
            DisplayColor = FColor::Red;
        }
        else if (CurrentFrameTime > WarningFrameTime)
        {
            DisplayColor = FColor::Yellow;
        }
        
        GEngine->AddOnScreenDebugMessage(-1, ProfilingInterval + 0.1f, DisplayColor, DisplayText);
    }
}
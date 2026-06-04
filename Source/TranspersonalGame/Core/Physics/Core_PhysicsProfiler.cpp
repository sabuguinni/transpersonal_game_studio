#include "Core_PhysicsProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

DEFINE_LOG_CATEGORY(LogPhysicsProfiler);

UCore_PhysicsProfiler::UCore_PhysicsProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS profiling
    
    // Initialize profiling data
    bIsProfilingActive = false;
    ProfileFrameCount = 0;
    TotalPhysicsTime = 0.0f;
    MaxPhysicsTime = 0.0f;
    MinPhysicsTime = FLT_MAX;
    
    // Performance thresholds
    PerformanceThresholds.PhysicsTimeWarning = 8.33f; // 8.33ms = 120fps limit
    PerformanceThresholds.PhysicsTimeCritical = 16.67f; // 16.67ms = 60fps limit
    PerformanceThresholds.RigidBodyCountWarning = 500;
    PerformanceThresholds.RigidBodyCountCritical = 1000;
    PerformanceThresholds.CollisionCountWarning = 100;
    PerformanceThresholds.CollisionCountCritical = 200;
    
    // Initialize metrics
    CurrentMetrics.PhysicsTime = 0.0f;
    CurrentMetrics.RigidBodyCount = 0;
    CurrentMetrics.CollisionCount = 0;
    CurrentMetrics.ConstraintCount = 0;
    CurrentMetrics.MemoryUsage = 0;
    CurrentMetrics.CPUUsage = 0.0f;
    
    // Profiling configuration
    ProfilingConfig.bEnableDetailedProfiling = true;
    ProfilingConfig.bLogToFile = true;
    ProfilingConfig.bDisplayOnScreen = false;
    ProfilingConfig.ProfilingDuration = 60.0f; // 60 seconds
    ProfilingConfig.SampleRate = 0.1f; // 10 samples per second
}

void UCore_PhysicsProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics Profiler initialized"));
    
    // Start profiling if auto-start is enabled
    if (ProfilingConfig.bAutoStartProfiling)
    {
        StartProfiling();
    }
}

void UCore_PhysicsProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProfilingActive)
    {
        UpdatePhysicsMetrics();
        
        ProfileFrameCount++;
        ProfilingElapsedTime += DeltaTime;
        
        // Check if profiling duration exceeded
        if (ProfilingElapsedTime >= ProfilingConfig.ProfilingDuration)
        {
            StopProfiling();
        }
        
        // Display on-screen if enabled
        if (ProfilingConfig.bDisplayOnScreen)
        {
            DisplayMetricsOnScreen();
        }
    }
}

void UCore_PhysicsProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("Profiling already active"));
        return;
    }
    
    bIsProfilingActive = true;
    ProfileFrameCount = 0;
    ProfilingElapsedTime = 0.0f;
    TotalPhysicsTime = 0.0f;
    MaxPhysicsTime = 0.0f;
    MinPhysicsTime = FLT_MAX;
    
    // Clear previous data
    PhysicsTimeHistory.Empty();
    RigidBodyCountHistory.Empty();
    CollisionCountHistory.Empty();
    
    ProfilingStartTime = FDateTime::Now();
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics profiling started"));
    
    // Broadcast start event
    OnProfilingStarted.Broadcast();
}

void UCore_PhysicsProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("Profiling not active"));
        return;
    }
    
    bIsProfilingActive = false;
    
    // Calculate final statistics
    if (ProfileFrameCount > 0)
    {
        float AveragePhysicsTime = TotalPhysicsTime / ProfileFrameCount;
        
        UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics Profiling Results:"));
        UE_LOG(LogPhysicsProfiler, Log, TEXT("  Duration: %.2f seconds"), ProfilingElapsedTime);
        UE_LOG(LogPhysicsProfiler, Log, TEXT("  Frames: %d"), ProfileFrameCount);
        UE_LOG(LogPhysicsProfiler, Log, TEXT("  Average Physics Time: %.2f ms"), AveragePhysicsTime);
        UE_LOG(LogPhysicsProfiler, Log, TEXT("  Max Physics Time: %.2f ms"), MaxPhysicsTime);
        UE_LOG(LogPhysicsProfiler, Log, TEXT("  Min Physics Time: %.2f ms"), MinPhysicsTime);
    }
    
    // Save profiling data if enabled
    if (ProfilingConfig.bLogToFile)
    {
        SaveProfilingDataToFile();
    }
    
    // Broadcast stop event
    OnProfilingStopped.Broadcast();
}

void UCore_PhysicsProfiler::UpdatePhysicsMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get physics scene
    FPhysScene* PhysScene = World->GetPhysicsScene();
    if (!PhysScene)
    {
        return;
    }
    
    // Update physics timing
    float CurrentPhysicsTime = GetPhysicsTime();
    CurrentMetrics.PhysicsTime = CurrentPhysicsTime;
    
    // Update statistics
    TotalPhysicsTime += CurrentPhysicsTime;
    MaxPhysicsTime = FMath::Max(MaxPhysicsTime, CurrentPhysicsTime);
    MinPhysicsTime = FMath::Min(MinPhysicsTime, CurrentPhysicsTime);
    
    // Store history
    PhysicsTimeHistory.Add(CurrentPhysicsTime);
    if (PhysicsTimeHistory.Num() > 1000) // Keep last 1000 samples
    {
        PhysicsTimeHistory.RemoveAt(0);
    }
    
    // Update rigid body count
    CurrentMetrics.RigidBodyCount = GetRigidBodyCount();
    RigidBodyCountHistory.Add(CurrentMetrics.RigidBodyCount);
    if (RigidBodyCountHistory.Num() > 1000)
    {
        RigidBodyCountHistory.RemoveAt(0);
    }
    
    // Update collision count
    CurrentMetrics.CollisionCount = GetCollisionCount();
    CollisionCountHistory.Add(CurrentMetrics.CollisionCount);
    if (CollisionCountHistory.Num() > 1000)
    {
        CollisionCountHistory.RemoveAt(0);
    }
    
    // Update other metrics
    CurrentMetrics.ConstraintCount = GetConstraintCount();
    CurrentMetrics.MemoryUsage = GetPhysicsMemoryUsage();
    CurrentMetrics.CPUUsage = GetPhysicsCPUUsage();
    
    // Check performance thresholds
    CheckPerformanceThresholds();
}

float UCore_PhysicsProfiler::GetPhysicsTime()
{
    // Get physics timing from stats system
    // This is a simplified implementation - in real scenario would use proper stats
    return FMath::RandRange(2.0f, 12.0f); // Simulated physics time in milliseconds
}

int32 UCore_PhysicsProfiler::GetRigidBodyCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    
    // Count all actors with physics bodies
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

int32 UCore_PhysicsProfiler::GetCollisionCount()
{
    // This would typically track collision events
    // For now, return a simulated value
    return FMath::RandRange(10, 50);
}

int32 UCore_PhysicsProfiler::GetConstraintCount()
{
    // Count physics constraints in the world
    // Simplified implementation
    return FMath::RandRange(0, 20);
}

int32 UCore_PhysicsProfiler::GetPhysicsMemoryUsage()
{
    // Get physics memory usage in KB
    // This would use proper memory tracking in real implementation
    return FMath::RandRange(1024, 8192); // 1-8 MB
}

float UCore_PhysicsProfiler::GetPhysicsCPUUsage()
{
    // Get physics CPU usage percentage
    // Simplified implementation
    return FMath::RandRange(5.0f, 25.0f);
}

void UCore_PhysicsProfiler::CheckPerformanceThresholds()
{
    // Check physics time thresholds
    if (CurrentMetrics.PhysicsTime > PerformanceThresholds.PhysicsTimeCritical)
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("CRITICAL: Physics time %.2f ms exceeds critical threshold %.2f ms"), 
               CurrentMetrics.PhysicsTime, PerformanceThresholds.PhysicsTimeCritical);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Critical, TEXT("Physics Time"));
    }
    else if (CurrentMetrics.PhysicsTime > PerformanceThresholds.PhysicsTimeWarning)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("WARNING: Physics time %.2f ms exceeds warning threshold %.2f ms"), 
               CurrentMetrics.PhysicsTime, PerformanceThresholds.PhysicsTimeWarning);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Warning, TEXT("Physics Time"));
    }
    
    // Check rigid body count thresholds
    if (CurrentMetrics.RigidBodyCount > PerformanceThresholds.RigidBodyCountCritical)
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("CRITICAL: Rigid body count %d exceeds critical threshold %d"), 
               CurrentMetrics.RigidBodyCount, PerformanceThresholds.RigidBodyCountCritical);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Critical, TEXT("Rigid Body Count"));
    }
    else if (CurrentMetrics.RigidBodyCount > PerformanceThresholds.RigidBodyCountWarning)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("WARNING: Rigid body count %d exceeds warning threshold %d"), 
               CurrentMetrics.RigidBodyCount, PerformanceThresholds.RigidBodyCountWarning);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Warning, TEXT("Rigid Body Count"));
    }
    
    // Check collision count thresholds
    if (CurrentMetrics.CollisionCount > PerformanceThresholds.CollisionCountCritical)
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("CRITICAL: Collision count %d exceeds critical threshold %d"), 
               CurrentMetrics.CollisionCount, PerformanceThresholds.CollisionCountCritical);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Critical, TEXT("Collision Count"));
    }
    else if (CurrentMetrics.CollisionCount > PerformanceThresholds.CollisionCountWarning)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("WARNING: Collision count %d exceeds warning threshold %d"), 
               CurrentMetrics.CollisionCount, PerformanceThresholds.CollisionCountWarning);
        OnPerformanceThresholdExceeded.Broadcast(ECore_PerformanceLevel::Warning, TEXT("Collision Count"));
    }
}

void UCore_PhysicsProfiler::DisplayMetricsOnScreen()
{
    if (GEngine)
    {
        FString DisplayText = FString::Printf(
            TEXT("Physics Profiler\n")
            TEXT("Physics Time: %.2f ms\n")
            TEXT("Rigid Bodies: %d\n")
            TEXT("Collisions: %d\n")
            TEXT("Constraints: %d\n")
            TEXT("Memory: %d KB\n")
            TEXT("CPU: %.1f%%"),
            CurrentMetrics.PhysicsTime,
            CurrentMetrics.RigidBodyCount,
            CurrentMetrics.CollisionCount,
            CurrentMetrics.ConstraintCount,
            CurrentMetrics.MemoryUsage,
            CurrentMetrics.CPUUsage
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, DisplayText);
    }
}

void UCore_PhysicsProfiler::SaveProfilingDataToFile()
{
    FString FileName = FString::Printf(TEXT("PhysicsProfile_%s.csv"), 
                                      *ProfilingStartTime.ToString(TEXT("%Y%m%d_%H%M%S")));
    
    FString FilePath = FPaths::ProjectLogDir() / FileName;
    
    FString CSVContent = TEXT("Frame,PhysicsTime,RigidBodies,Collisions\n");
    
    int32 MaxSamples = FMath::Min({PhysicsTimeHistory.Num(), RigidBodyCountHistory.Num(), CollisionCountHistory.Num()});
    
    for (int32 i = 0; i < MaxSamples; i++)
    {
        CSVContent += FString::Printf(TEXT("%d,%.2f,%d,%d\n"),
                                     i,
                                     PhysicsTimeHistory[i],
                                     RigidBodyCountHistory[i],
                                     CollisionCountHistory[i]);
    }
    
    if (FFileHelper::SaveStringToFile(CSVContent, *FilePath))
    {
        UE_LOG(LogPhysicsProfiler, Log, TEXT("Profiling data saved to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("Failed to save profiling data to: %s"), *FilePath);
    }
}

FCore_PhysicsMetrics UCore_PhysicsProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

FCore_PerformanceThresholds UCore_PhysicsProfiler::GetPerformanceThresholds() const
{
    return PerformanceThresholds;
}

void UCore_PhysicsProfiler::SetPerformanceThresholds(const FCore_PerformanceThresholds& NewThresholds)
{
    PerformanceThresholds = NewThresholds;
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Performance thresholds updated"));
}

bool UCore_PhysicsProfiler::IsProfilingActive() const
{
    return bIsProfilingActive;
}

float UCore_PhysicsProfiler::GetAveragePhysicsTime() const
{
    if (ProfileFrameCount > 0)
    {
        return TotalPhysicsTime / ProfileFrameCount;
    }
    return 0.0f;
}

float UCore_PhysicsProfiler::GetMaxPhysicsTime() const
{
    return MaxPhysicsTime;
}

float UCore_PhysicsProfiler::GetMinPhysicsTime() const
{
    return MinPhysicsTime == FLT_MAX ? 0.0f : MinPhysicsTime;
}

TArray<float> UCore_PhysicsProfiler::GetPhysicsTimeHistory() const
{
    return PhysicsTimeHistory;
}

void UCore_PhysicsProfiler::SetProfilingConfig(const FCore_ProfilingConfig& NewConfig)
{
    ProfilingConfig = NewConfig;
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Profiling configuration updated"));
}

FCore_ProfilingConfig UCore_PhysicsProfiler::GetProfilingConfig() const
{
    return ProfilingConfig;
}

void UCore_PhysicsProfiler::ResetProfiling()
{
    if (bIsProfilingActive)
    {
        StopProfiling();
    }
    
    ProfileFrameCount = 0;
    TotalPhysicsTime = 0.0f;
    MaxPhysicsTime = 0.0f;
    MinPhysicsTime = FLT_MAX;
    ProfilingElapsedTime = 0.0f;
    
    PhysicsTimeHistory.Empty();
    RigidBodyCountHistory.Empty();
    CollisionCountHistory.Empty();
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics profiler reset"));
}
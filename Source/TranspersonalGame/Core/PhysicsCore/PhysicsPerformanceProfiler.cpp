// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "PhysicsPerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

DEFINE_LOG_CATEGORY(LogPhysicsProfiler);

UPhysicsPerformanceProfiler::UPhysicsPerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    // Set reasonable defaults
    ProfilingFrequency = 1.0f;
    bPerformanceWarningsEnabled = true;
    MaxSimulationTimeMs = 16.67f; // 60 FPS target
    MaxMemoryUsageMB = 100.0f;
    MaxHistorySamples = 60;
}

void UPhysicsPerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics Performance Profiler initialized"));
    
    // Start profiling by default
    StartProfiling();
}

void UPhysicsPerformanceProfiler::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopProfiling();
    Super::EndPlay(EndPlayReason);
}

void UPhysicsPerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsProfiling)
    {
        // Update metrics at specified frequency
        double CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - LastProfilingTime >= ProfilingFrequency)
        {
            UpdatePerformanceMetrics();
            CheckPerformanceWarnings();
            LastProfilingTime = CurrentTime;
        }
    }
}

void UPhysicsPerformanceProfiler::StartProfiling()
{
    if (!bIsProfiling)
    {
        bIsProfiling = true;
        LastProfilingTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics performance profiling started"));
        
        // Clear existing data
        ResetStatistics();
    }
}

void UPhysicsPerformanceProfiler::StopProfiling()
{
    if (bIsProfiling)
    {
        bIsProfiling = false;
        
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(ProfilingTimerHandle);
        }
        
        UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics performance profiling stopped"));
    }
}

FPhysicsPerformanceData UPhysicsPerformanceProfiler::GetCurrentPerformanceData() const
{
    return CurrentPerformanceData;
}

FPhysicsPerformanceData UPhysicsPerformanceProfiler::GetAveragePerformanceData() const
{
    if (PerformanceHistory.Num() == 0)
    {
        return FPhysicsPerformanceData();
    }
    
    FPhysicsPerformanceData AverageData;
    
    for (const FPhysicsPerformanceData& Data : PerformanceHistory)
    {
        AverageData.ActiveRigidBodies += Data.ActiveRigidBodies;
        AverageData.ActiveCollisionPairs += Data.ActiveCollisionPairs;
        AverageData.SimulationTimeMs += Data.SimulationTimeMs;
        AverageData.SubstepCount += Data.SubstepCount;
        AverageData.MemoryUsageMB += Data.MemoryUsageMB;
        AverageData.PhysicsFPSImpact += Data.PhysicsFPSImpact;
        AverageData.SleepingBodies += Data.SleepingBodies;
        AverageData.KinematicBodies += Data.KinematicBodies;
    }
    
    int32 SampleCount = PerformanceHistory.Num();
    AverageData.ActiveRigidBodies /= SampleCount;
    AverageData.ActiveCollisionPairs /= SampleCount;
    AverageData.SimulationTimeMs /= SampleCount;
    AverageData.SubstepCount /= SampleCount;
    AverageData.MemoryUsageMB /= SampleCount;
    AverageData.PhysicsFPSImpact /= SampleCount;
    AverageData.SleepingBodies /= SampleCount;
    AverageData.KinematicBodies /= SampleCount;
    
    return AverageData;
}

void UPhysicsPerformanceProfiler::ResetStatistics()
{
    CurrentPerformanceData = FPhysicsPerformanceData();
    PerformanceHistory.Empty();
    LastProfilingTime = 0.0;
    
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Physics performance statistics reset"));
}

void UPhysicsPerformanceProfiler::SetProfilingFrequency(float FrequencySeconds)
{
    ProfilingFrequency = FMath::Clamp(FrequencySeconds, 0.1f, 5.0f);
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Profiling frequency set to %.2f seconds"), ProfilingFrequency);
}

void UPhysicsPerformanceProfiler::SetPerformanceWarningsEnabled(bool bEnabled)
{
    bPerformanceWarningsEnabled = bEnabled;
    UE_LOG(LogPhysicsProfiler, Log, TEXT("Performance warnings %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

float UPhysicsPerformanceProfiler::GetPhysicsMemoryUsage() const
{
    return CalculatePhysicsMemoryUsage();
}

TArray<FString> UPhysicsPerformanceProfiler::GetPerformanceBottlenecks() const
{
    TArray<FString> Bottlenecks;
    
    const FPhysicsPerformanceData& Data = CurrentPerformanceData;
    
    // Check simulation time
    if (Data.SimulationTimeMs > MaxSimulationTimeMs)
    {
        Bottlenecks.Add(FString::Printf(TEXT("High simulation time: %.2f ms (target: %.2f ms)"), 
            Data.SimulationTimeMs, MaxSimulationTimeMs));
    }
    
    // Check memory usage
    if (Data.MemoryUsageMB > MaxMemoryUsageMB)
    {
        Bottlenecks.Add(FString::Printf(TEXT("High memory usage: %.2f MB (limit: %.2f MB)"), 
            Data.MemoryUsageMB, MaxMemoryUsageMB));
    }
    
    // Check active body count
    if (Data.ActiveRigidBodies > 1000)
    {
        Bottlenecks.Add(FString::Printf(TEXT("High active body count: %d bodies"), Data.ActiveRigidBodies));
    }
    
    // Check collision pairs
    if (Data.ActiveCollisionPairs > 5000)
    {
        Bottlenecks.Add(FString::Printf(TEXT("High collision pair count: %d pairs"), Data.ActiveCollisionPairs));
    }
    
    // Check substep count
    if (Data.SubstepCount > 4)
    {
        Bottlenecks.Add(FString::Printf(TEXT("High substep count: %d substeps"), Data.SubstepCount));
    }
    
    return Bottlenecks;
}

bool UPhysicsPerformanceProfiler::ExportPerformanceData(const FString& FilePath) const
{
    if (PerformanceHistory.Num() == 0)
    {
        UE_LOG(LogPhysicsProfiler, Warning, TEXT("No performance data to export"));
        return false;
    }
    
    FString CSVContent;
    CSVContent += TEXT("Timestamp,ActiveRigidBodies,ActiveCollisionPairs,SimulationTimeMs,SubstepCount,MemoryUsageMB,PhysicsFPSImpact,SleepingBodies,KinematicBodies\n");
    
    for (int32 i = 0; i < PerformanceHistory.Num(); ++i)
    {
        const FPhysicsPerformanceData& Data = PerformanceHistory[i];
        CSVContent += FString::Printf(TEXT("%d,%d,%d,%.3f,%d,%.3f,%.3f,%d,%d\n"),
            i,
            Data.ActiveRigidBodies,
            Data.ActiveCollisionPairs,
            Data.SimulationTimeMs,
            Data.SubstepCount,
            Data.MemoryUsageMB,
            Data.PhysicsFPSImpact,
            Data.SleepingBodies,
            Data.KinematicBodies
        );
    }
    
    bool bSuccess = FFileHelper::SaveStringToFile(CSVContent, *FilePath);
    
    if (bSuccess)
    {
        UE_LOG(LogPhysicsProfiler, Log, TEXT("Performance data exported to: %s"), *FilePath);
    }
    else
    {
        UE_LOG(LogPhysicsProfiler, Error, TEXT("Failed to export performance data to: %s"), *FilePath);
    }
    
    return bSuccess;
}

void UPhysicsPerformanceProfiler::UpdatePerformanceMetrics()
{
    // Gather Chaos physics data
    GatherChaosPhysicsData();
    
    // Calculate memory usage
    CurrentPerformanceData.MemoryUsageMB = CalculatePhysicsMemoryUsage();
    
    // Calculate FPS impact (simplified)
    CurrentPerformanceData.PhysicsFPSImpact = CurrentPerformanceData.SimulationTimeMs / 16.67f * 100.0f;
    
    // Add to history
    PerformanceHistory.Add(CurrentPerformanceData);
    
    // Maintain history size
    if (PerformanceHistory.Num() > MaxHistorySamples)
    {
        PerformanceHistory.RemoveAt(0);
    }
    
    // Log periodic updates
    UE_LOG(LogPhysicsProfiler, VeryVerbose, TEXT("Physics Metrics - Bodies: %d, Pairs: %d, Time: %.2f ms, Memory: %.2f MB"),
        CurrentPerformanceData.ActiveRigidBodies,
        CurrentPerformanceData.ActiveCollisionPairs,
        CurrentPerformanceData.SimulationTimeMs,
        CurrentPerformanceData.MemoryUsageMB);
}

void UPhysicsPerformanceProfiler::CheckPerformanceWarnings()
{
    if (!bPerformanceWarningsEnabled)
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastWarningTime < WarningCooldownTime)
    {
        return; // Still in cooldown
    }
    
    TArray<FString> Bottlenecks = GetPerformanceBottlenecks();
    if (Bottlenecks.Num() > 0)
    {
        for (const FString& Bottleneck : Bottlenecks)
        {
            UE_LOG(LogPhysicsProfiler, Warning, TEXT("Physics Performance Warning: %s"), *Bottleneck);
        }
        
        LastWarningTime = CurrentTime;
    }
}

void UPhysicsPerformanceProfiler::GatherChaosPhysicsData()
{
    UWorld* World = GetWorld();
    if (!World || !World->GetPhysicsScene())
    {
        return;
    }
    
    // Get physics scene
    FPhysScene* PhysScene = World->GetPhysicsScene();
    if (!PhysScene)
    {
        return;
    }
    
    // Reset counters
    CurrentPerformanceData.ActiveRigidBodies = 0;
    CurrentPerformanceData.ActiveCollisionPairs = 0;
    CurrentPerformanceData.SleepingBodies = 0;
    CurrentPerformanceData.KinematicBodies = 0;
    
    // Count physics actors in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        // Check for physics components
        UPrimitiveComponent* PrimComp = Actor->GetRootComponent() ? 
            Cast<UPrimitiveComponent>(Actor->GetRootComponent()) : nullptr;
        
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            CurrentPerformanceData.ActiveRigidBodies++;
            
            // Check if sleeping
            if (PrimComp->IsAnyRigidBodySleeping())
            {
                CurrentPerformanceData.SleepingBodies++;
            }
        }
        else if (PrimComp && PrimComp->GetBodyInstance())
        {
            CurrentPerformanceData.KinematicBodies++;
        }
    }
    
    // Estimate collision pairs (simplified calculation)
    CurrentPerformanceData.ActiveCollisionPairs = 
        (CurrentPerformanceData.ActiveRigidBodies * (CurrentPerformanceData.ActiveRigidBodies - 1)) / 2;
    
    // Get simulation timing from stats (if available)
    CurrentPerformanceData.SimulationTimeMs = 0.0f; // Would need access to Chaos internal timing
    CurrentPerformanceData.SubstepCount = 1; // Default, would need Chaos internal data
    
    // Use UPhysicsSettings to get some configuration data
    const UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get();
    if (PhysicsSettings)
    {
        CurrentPerformanceData.SubstepCount = FMath::Max(1, PhysicsSettings->MaxSubsteps);
    }
}

float UPhysicsPerformanceProfiler::CalculatePhysicsMemoryUsage() const
{
    // Simplified memory calculation
    // In a real implementation, this would query Chaos memory allocators
    
    float EstimatedMemoryMB = 0.0f;
    
    // Base memory per rigid body (estimated)
    EstimatedMemoryMB += CurrentPerformanceData.ActiveRigidBodies * 0.001f; // ~1KB per body
    
    // Memory for collision pairs
    EstimatedMemoryMB += CurrentPerformanceData.ActiveCollisionPairs * 0.0001f; // ~100 bytes per pair
    
    // Base physics system overhead
    EstimatedMemoryMB += 10.0f; // 10MB base overhead
    
    return EstimatedMemoryMB;
}
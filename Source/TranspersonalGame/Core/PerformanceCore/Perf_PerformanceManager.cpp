#include "Perf_PerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"
#include "Stats/StatsData.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"

UPerf_PerformanceManager::UPerf_PerformanceManager()
{
    bIsMonitoring = false;
    bAutomaticOptimization = false;
    LastUpdateTime = 0.0f;
    UpdateInterval = 1.0f; // Update every second
    MaxFrameHistorySize = 60; // Keep 60 frames of history
    
    // Initialize default performance settings
    PerformanceSettings = FPerf_PerformanceSettings();
}

void UPerf_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Performance Manager initialized"));
    
    // Detect hardware capabilities on startup
    DetectHardwareCapabilities();
    
    // Start monitoring by default
    StartPerformanceMonitoring();
}

void UPerf_PerformanceManager::Deinitialize()
{
    StopPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Manager deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_PerformanceManager::StartPerformanceMonitoring()
{
    if (bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = true;
    LastUpdateTime = FPlatformTime::Seconds();
    
    // Clear frame history
    FrameTimeHistory.Empty();
    
    // Start periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            PerformanceUpdateTimer,
            this,
            &UPerf_PerformanceManager::UpdatePerformanceMetrics,
            UpdateInterval,
            true // Loop
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_PerformanceManager::StopPerformanceMonitoring()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    bIsMonitoring = false;
    
    // Stop periodic updates
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics UPerf_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

float UPerf_PerformanceManager::GetCurrentFPS() const
{
    return CurrentMetrics.CurrentFPS;
}

bool UPerf_PerformanceManager::IsPerformanceAcceptable() const
{
    return CurrentMetrics.CurrentFPS >= PerformanceSettings.MinimumFPS;
}

void UPerf_PerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    PerformanceSettings.PerformanceLevel = NewLevel;
    
    // Apply settings based on performance level
    switch (NewLevel)
    {
        case EPerf_PerformanceLevel::Low:
            PerformanceSettings.TargetFPS = 30.0f;
            PerformanceSettings.MinimumFPS = 20.0f;
            PerformanceSettings.MaxViewDistance = 5000.0f;
            PerformanceSettings.MaxPhysicsActors = 50;
            break;
            
        case EPerf_PerformanceLevel::Medium:
            PerformanceSettings.TargetFPS = 45.0f;
            PerformanceSettings.MinimumFPS = 30.0f;
            PerformanceSettings.MaxViewDistance = 7500.0f;
            PerformanceSettings.MaxPhysicsActors = 75;
            break;
            
        case EPerf_PerformanceLevel::High:
            PerformanceSettings.TargetFPS = 60.0f;
            PerformanceSettings.MinimumFPS = 45.0f;
            PerformanceSettings.MaxViewDistance = 10000.0f;
            PerformanceSettings.MaxPhysicsActors = 100;
            break;
            
        case EPerf_PerformanceLevel::Ultra:
            PerformanceSettings.TargetFPS = 120.0f;
            PerformanceSettings.MinimumFPS = 60.0f;
            PerformanceSettings.MaxViewDistance = 15000.0f;
            PerformanceSettings.MaxPhysicsActors = 150;
            break;
    }
    
    ApplyPerformanceSettings(PerformanceSettings);
    
    UE_LOG(LogTemp, Log, TEXT("Performance level set to: %d"), (int32)NewLevel);
}

EPerf_PerformanceLevel UPerf_PerformanceManager::GetPerformanceLevel() const
{
    return PerformanceSettings.PerformanceLevel;
}

void UPerf_PerformanceManager::SetTargetFPS(float NewTargetFPS)
{
    PerformanceSettings.TargetFPS = FMath::Max(NewTargetFPS, 15.0f); // Minimum 15 FPS
    UE_LOG(LogTemp, Log, TEXT("Target FPS set to: %.1f"), PerformanceSettings.TargetFPS);
}

float UPerf_PerformanceManager::GetTargetFPS() const
{
    return PerformanceSettings.TargetFPS;
}

void UPerf_PerformanceManager::EnableAutomaticOptimization(bool bEnable)
{
    bAutomaticOptimization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Automatic optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceManager::OptimizeForCurrentHardware()
{
    // Detect current performance and adjust settings accordingly
    float CurrentFPS = GetCurrentFPS();
    
    if (CurrentFPS < PerformanceSettings.MinimumFPS)
    {
        // Performance is poor, reduce quality
        if (PerformanceSettings.PerformanceLevel != EPerf_PerformanceLevel::Low)
        {
            EPerf_PerformanceLevel NewLevel = static_cast<EPerf_PerformanceLevel>(
                FMath::Max(0, static_cast<int32>(PerformanceSettings.PerformanceLevel) - 1)
            );
            SetPerformanceLevel(NewLevel);
            UE_LOG(LogTemp, Warning, TEXT("Performance degraded, reducing quality level"));
        }
    }
    else if (CurrentFPS > PerformanceSettings.TargetFPS * 1.2f)
    {
        // Performance is excellent, can increase quality
        if (PerformanceSettings.PerformanceLevel != EPerf_PerformanceLevel::Ultra)
        {
            EPerf_PerformanceLevel NewLevel = static_cast<EPerf_PerformanceLevel>(
                FMath::Min(3, static_cast<int32>(PerformanceSettings.PerformanceLevel) + 1)
            );
            SetPerformanceLevel(NewLevel);
            UE_LOG(LogTemp, Log, TEXT("Performance excellent, increasing quality level"));
        }
    }
}

void UPerf_PerformanceManager::ApplyPerformanceSettings(const FPerf_PerformanceSettings& Settings)
{
    PerformanceSettings = Settings;
    
    // Apply console variable settings
    ExecuteConsoleCommand(FString::Printf(TEXT("t.MaxFPS %f"), Settings.TargetFPS));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.ViewDistanceScale %f"), 
        Settings.MaxViewDistance / 10000.0f)); // Normalize to scale
    
    // Apply LOD settings based on performance level
    switch (Settings.PerformanceLevel)
    {
        case EPerf_PerformanceLevel::Low:
            ExecuteConsoleCommand(TEXT("r.StaticMeshLODBias 2"));
            ExecuteConsoleCommand(TEXT("r.SkeletalMeshLODBias 2"));
            ExecuteConsoleCommand(TEXT("r.ParticleLODBias 2"));
            break;
            
        case EPerf_PerformanceLevel::Medium:
            ExecuteConsoleCommand(TEXT("r.StaticMeshLODBias 1"));
            ExecuteConsoleCommand(TEXT("r.SkeletalMeshLODBias 1"));
            ExecuteConsoleCommand(TEXT("r.ParticleLODBias 1"));
            break;
            
        case EPerf_PerformanceLevel::High:
        case EPerf_PerformanceLevel::Ultra:
            ExecuteConsoleCommand(TEXT("r.StaticMeshLODBias 0"));
            ExecuteConsoleCommand(TEXT("r.SkeletalMeshLODBias 0"));
            ExecuteConsoleCommand(TEXT("r.ParticleLODBias 0"));
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance settings applied"));
}

void UPerf_PerformanceManager::ForceGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Log, TEXT("Forced garbage collection"));
}

float UPerf_PerformanceManager::GetUsedMemoryMB() const
{
    return CurrentMetrics.UsedMemoryMB;
}

void UPerf_PerformanceManager::OptimizeActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count physics actors and disable some if over limit
    int32 PhysicsActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsActorCount++;
                
                if (PhysicsActorCount > PerformanceSettings.MaxPhysicsActors)
                {
                    // Disable physics on excess actors
                    PrimComp->SetSimulatePhysics(false);
                    UE_LOG(LogTemp, Warning, TEXT("Disabled physics on actor to maintain performance"));
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Actor optimization complete. Physics actors: %d"), PhysicsActorCount);
}

int32 UPerf_PerformanceManager::GetActiveActorCount() const
{
    return CurrentMetrics.ActiveActorCount;
}

void UPerf_PerformanceManager::SetMaxPhysicsActors(int32 MaxActors)
{
    PerformanceSettings.MaxPhysicsActors = FMath::Max(MaxActors, 10); // Minimum 10 physics actors
    UE_LOG(LogTemp, Log, TEXT("Max physics actors set to: %d"), PerformanceSettings.MaxPhysicsActors);
}

void UPerf_PerformanceManager::UpdateLODLevels()
{
    // Force LOD update based on current performance
    float PerformanceRatio = CurrentMetrics.CurrentFPS / PerformanceSettings.TargetFPS;
    
    if (PerformanceRatio < 0.8f)
    {
        // Performance is low, increase LOD bias
        SetGlobalLODBias(2.0f);
    }
    else if (PerformanceRatio > 1.2f)
    {
        // Performance is good, decrease LOD bias
        SetGlobalLODBias(0.0f);
    }
    else
    {
        // Performance is acceptable, use medium LOD bias
        SetGlobalLODBias(1.0f);
    }
}

void UPerf_PerformanceManager::SetGlobalLODBias(float LODBias)
{
    ExecuteConsoleCommand(FString::Printf(TEXT("r.StaticMeshLODBias %f"), LODBias));
    ExecuteConsoleCommand(FString::Printf(TEXT("r.SkeletalMeshLODBias %f"), LODBias));
    UE_LOG(LogTemp, Log, TEXT("Global LOD bias set to: %.1f"), LODBias);
}

void UPerf_PerformanceManager::ShowPerformanceStats()
{
    ExecuteConsoleCommand(TEXT("stat fps"));
    ExecuteConsoleCommand(TEXT("stat unit"));
    ExecuteConsoleCommand(TEXT("stat memory"));
    UE_LOG(LogTemp, Log, TEXT("Performance stats displayed"));
}

void UPerf_PerformanceManager::HidePerformanceStats()
{
    ExecuteConsoleCommand(TEXT("stat none"));
    UE_LOG(LogTemp, Log, TEXT("Performance stats hidden"));
}

void UPerf_PerformanceManager::DumpPerformanceReport()
{
    FString ReportPath = FPaths::ProjectLogDir() / TEXT("PerformanceReport.txt");
    FString Report;
    
    Report += FString::Printf(TEXT("Performance Report - %s\n"), *FDateTime::Now().ToString());
    Report += TEXT("=====================================\n");
    Report += FString::Printf(TEXT("Current FPS: %.2f\n"), CurrentMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average Frame Time: %.2f ms\n"), CurrentMetrics.AverageFrameTime);
    Report += FString::Printf(TEXT("Game Thread Time: %.2f ms\n"), CurrentMetrics.GameThreadTime);
    Report += FString::Printf(TEXT("Render Thread Time: %.2f ms\n"), CurrentMetrics.RenderThreadTime);
    Report += FString::Printf(TEXT("GPU Time: %.2f ms\n"), CurrentMetrics.GPUTime);
    Report += FString::Printf(TEXT("Draw Calls: %d\n"), CurrentMetrics.DrawCalls);
    Report += FString::Printf(TEXT("Triangles: %d\n"), CurrentMetrics.Triangles);
    Report += FString::Printf(TEXT("Used Memory: %.2f MB\n"), CurrentMetrics.UsedMemoryMB);
    Report += FString::Printf(TEXT("Active Actors: %d\n"), CurrentMetrics.ActiveActorCount);
    Report += FString::Printf(TEXT("Physics Actors: %d\n"), CurrentMetrics.PhysicsActorCount);
    Report += FString::Printf(TEXT("Performance Level: %d\n"), (int32)PerformanceSettings.PerformanceLevel);
    Report += FString::Printf(TEXT("Target FPS: %.1f\n"), PerformanceSettings.TargetFPS);
    
    FFileHelper::SaveStringToFile(Report, *ReportPath);
    UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *ReportPath);
}

void UPerf_PerformanceManager::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring)
    {
        return;
    }
    
    float CurrentTime = FPlatformTime::Seconds();
    float DeltaTime = CurrentTime - LastUpdateTime;
    LastUpdateTime = CurrentTime;
    
    // Update frame time history
    UpdateFrameTimeHistory(DeltaTime);
    
    // Calculate current FPS
    CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    
    // Get memory usage
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.UsedMemoryMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Count actors
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.ActiveActorCount = 0;
        CurrentMetrics.PhysicsActorCount = 0;
        
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor)
            {
                CurrentMetrics.ActiveActorCount++;
                
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentMetrics.PhysicsActorCount++;
                    }
                }
            }
        }
    }
    
    // Check performance thresholds
    CheckPerformanceThresholds();
    
    // Apply automatic optimizations if enabled
    if (bAutomaticOptimization)
    {
        ApplyAutomaticOptimizations();
    }
}

void UPerf_PerformanceManager::CheckPerformanceThresholds()
{
    if (CurrentMetrics.CurrentFPS < PerformanceSettings.MinimumFPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below minimum threshold: %.1f FPS"), 
            CurrentMetrics.CurrentFPS);
    }
    
    if (CurrentMetrics.UsedMemoryMB > 4000.0f) // 4GB threshold
    {
        UE_LOG(LogTemp, Warning, TEXT("High memory usage: %.1f MB"), CurrentMetrics.UsedMemoryMB);
    }
}

void UPerf_PerformanceManager::ApplyAutomaticOptimizations()
{
    // Only optimize if performance is consistently poor
    if (FrameTimeHistory.Num() >= 10)
    {
        float AverageFPS = 1.0f / CalculateAverageFrameTime();
        
        if (AverageFPS < PerformanceSettings.MinimumFPS * 0.9f)
        {
            // Apply optimizations
            OptimizeActorCount();
            UpdateLODLevels();
            
            // Force garbage collection if memory is high
            if (CurrentMetrics.UsedMemoryMB > 3000.0f)
            {
                ForceGarbageCollection();
            }
        }
    }
}

void UPerf_PerformanceManager::ExecuteConsoleCommand(const FString& Command)
{
    if (UWorld* World = GetWorld())
    {
        GEngine->Exec(World, *Command);
    }
}

float UPerf_PerformanceManager::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
    {
        return 0.0f;
    }
    
    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }
    
    return Sum / FrameTimeHistory.Num();
}

void UPerf_PerformanceManager::UpdateFrameTimeHistory(float DeltaTime)
{
    FrameTimeHistory.Add(DeltaTime);
    
    // Keep history size limited
    if (FrameTimeHistory.Num() > MaxFrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

void UPerf_PerformanceManager::DetectHardwareCapabilities()
{
    // Basic hardware detection - in a real implementation this would be more sophisticated
    UE_LOG(LogTemp, Log, TEXT("Detecting hardware capabilities..."));
    
    // For now, assume medium performance as default
    SetPerformanceLevel(EPerf_PerformanceLevel::Medium);
    
    UE_LOG(LogTemp, Log, TEXT("Hardware detection complete - set to Medium performance"));
}
#include "Eng_PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Actor.h"
#include "Engine/Level.h"
#include "TimerManager.h"

void UEng_PerformanceProfiler::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bIsProfilingActive = false;
    TargetFrameRate = 60.0f;
    MetricsHistory.Reserve(1000);
    
    UE_LOG(LogTemp, Log, TEXT("Engine Performance Profiler initialized"));
}

void UEng_PerformanceProfiler::Deinitialize()
{
    StopProfiling();
    Super::Deinitialize();
}

FEng_PerformanceMetrics UEng_PerformanceProfiler::GetCurrentMetrics()
{
    FEng_PerformanceMetrics Metrics;
    
    if (UWorld* World = GetWorld())
    {
        // Frame rate
        if (GEngine && GEngine->GetGameViewport())
        {
            Metrics.FrameRate = 1.0f / World->GetDeltaSeconds();
        }
        
        // Actor counts
        if (ULevel* Level = World->GetCurrentLevel())
        {
            Metrics.ActorCount = Level->Actors.Num();
            Metrics.DinosaurCount = CountActorsByType(TEXT("Dinosaur"));
            Metrics.PropsCount = CountActorsByType(TEXT("StaticMesh"));
        }
        
        // Memory usage (approximate)
        FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
        Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
        
        // GPU and thread times (simplified)
        Metrics.GPUTimeMS = FPlatformTime::ToMilliseconds(GRenderThreadTime);
        Metrics.GameThreadTimeMS = FPlatformTime::ToMilliseconds(GGameThreadTime);
        Metrics.RenderThreadTimeMS = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    }
    
    return Metrics;
}

EEng_PerformanceLevel UEng_PerformanceProfiler::GetPerformanceLevel()
{
    FEng_PerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    
    // Critical thresholds
    if (CurrentMetrics.FrameRate < 20.0f || CurrentMetrics.ActorCount > 20000 || CurrentMetrics.MemoryUsageMB > 6000.0f)
    {
        return EEng_PerformanceLevel::Critical;
    }
    
    // Warning thresholds
    if (CurrentMetrics.FrameRate < 40.0f || CurrentMetrics.ActorCount > 15000 || CurrentMetrics.MemoryUsageMB > 4000.0f)
    {
        return EEng_PerformanceLevel::Warning;
    }
    
    // Good thresholds
    if (CurrentMetrics.FrameRate < 55.0f || CurrentMetrics.ActorCount > 8000 || CurrentMetrics.MemoryUsageMB > 2000.0f)
    {
        return EEng_PerformanceLevel::Good;
    }
    
    return EEng_PerformanceLevel::Excellent;
}

void UEng_PerformanceProfiler::StartProfiling()
{
    if (!bIsProfilingActive)
    {
        bIsProfilingActive = true;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                ProfilingTimerHandle,
                this,
                &UEng_PerformanceProfiler::UpdateMetrics,
                1.0f,
                true
            );
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
    }
}

void UEng_PerformanceProfiler::StopProfiling()
{
    if (bIsProfilingActive)
    {
        bIsProfilingActive = false;
        
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ProfilingTimerHandle);
        }
        
        UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
    }
}

void UEng_PerformanceProfiler::SetTargetFrameRate(float NewTargetFPS)
{
    TargetFrameRate = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Target frame rate set to %.1f FPS"), TargetFrameRate);
}

void UEng_PerformanceProfiler::LogPerformanceReport()
{
    FEng_PerformanceMetrics Metrics = GetCurrentMetrics();
    EEng_PerformanceLevel Level = GetPerformanceLevel();
    
    FString LevelString;
    switch (Level)
    {
        case EEng_PerformanceLevel::Excellent: LevelString = TEXT("EXCELLENT"); break;
        case EEng_PerformanceLevel::Good: LevelString = TEXT("GOOD"); break;
        case EEng_PerformanceLevel::Warning: LevelString = TEXT("WARNING"); break;
        case EEng_PerformanceLevel::Critical: LevelString = TEXT("CRITICAL"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Level: %s"), *LevelString);
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.1f FPS (Target: %.1f)"), Metrics.FrameRate, TargetFrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Memory: %.1f MB"), Metrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Actors: %d (Dinos: %d, Props: %d)"), Metrics.ActorCount, Metrics.DinosaurCount, Metrics.PropsCount);
    UE_LOG(LogTemp, Warning, TEXT("GPU Time: %.2f ms"), Metrics.GPUTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("Game Thread: %.2f ms"), Metrics.GameThreadTimeMS);
    UE_LOG(LogTemp, Warning, TEXT("========================"));
}

void UEng_PerformanceProfiler::EnforceActorLimits()
{
    FEng_PerformanceMetrics Metrics = GetCurrentMetrics();
    
    if (Metrics.ActorCount > 20000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Actor count (%d) exceeds limit (20000). Cleaning up..."), Metrics.ActorCount);
        CleanupExcessActors();
    }
    else if (Metrics.DinosaurCount > 150)
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Dinosaur count (%d) exceeds recommended limit (150)"), Metrics.DinosaurCount);
    }
}

void UEng_PerformanceProfiler::UpdateMetrics()
{
    if (!bIsProfilingActive) return;
    
    FEng_PerformanceMetrics CurrentMetrics = GetCurrentMetrics();
    MetricsHistory.Add(CurrentMetrics);
    
    // Keep only last 100 entries
    if (MetricsHistory.Num() > 100)
    {
        MetricsHistory.RemoveAt(0);
    }
    
    CheckPerformanceThresholds();
}

void UEng_PerformanceProfiler::CheckPerformanceThresholds()
{
    EEng_PerformanceLevel Level = GetPerformanceLevel();
    
    if (Level == EEng_PerformanceLevel::Critical)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL PERFORMANCE DETECTED - Enforcing limits"));
        EnforceActorLimits();
    }
    else if (Level == EEng_PerformanceLevel::Warning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance warning detected"));
    }
}

void UEng_PerformanceProfiler::CleanupExcessActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    // Sort by distance from origin, remove furthest first
    AllActors.Sort([](const AActor& A, const AActor& B) {
        return A.GetActorLocation().Size() > B.GetActorLocation().Size();
    });
    
    int32 ActorsToRemove = FMath::Max(0, AllActors.Num() - 18000);
    int32 RemovedCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (RemovedCount >= ActorsToRemove) break;
        
        // Don't remove essential actors
        if (Actor->IsA<APawn>() || Actor->IsA<APlayerStart>() || Actor->IsA<ALight>())
        {
            continue;
        }
        
        Actor->Destroy();
        RemovedCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Cleanup complete: Removed %d excess actors"), RemovedCount);
}

int32 UEng_PerformanceProfiler::CountActorsByType(const FString& TypeName)
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && Actor->GetClass()->GetName().Contains(TypeName))
        {
            Count++;
        }
    }
    
    return Count;
}
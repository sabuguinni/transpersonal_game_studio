#include "Eng_PerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "TimerManager.h"

void UEng_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Initialized"));
    
    // Start metrics collection
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MetricsUpdateTimer, 
            FTimerDelegate::CreateUObject(this, &UEng_PerformanceManager::UpdateMetrics),
            PerformanceCheckInterval, true);
    }
    
    CurrentPerformanceLevel = EEng_PerformanceLevel::Medium;
    CurrentMetrics = FEng_PerformanceMetrics();
}

void UEng_PerformanceManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MetricsUpdateTimer);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Deinitialized"));
    Super::Deinitialize();
}

FEng_PerformanceMetrics UEng_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UEng_PerformanceManager::SetPerformanceLevel(EEng_PerformanceLevel Level)
{
    CurrentPerformanceLevel = Level;
    ApplyPerformanceOptimizations();
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Set level to %d"), (int32)Level);
}

bool UEng_PerformanceManager::IsPerformanceCritical() const
{
    return CurrentMetrics.bIsPerformanceCritical;
}

void UEng_PerformanceManager::OptimizeLevel()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Count actors and apply optimizations
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorCount = AllActors.Num();
    
    // Auto-adjust performance level based on actor count
    if (ActorCount > 5000)
    {
        SetPerformanceLevel(EEng_PerformanceLevel::Low);
    }
    else if (ActorCount > 2000)
    {
        SetPerformanceLevel(EEng_PerformanceLevel::Medium);
    }
    else
    {
        SetPerformanceLevel(EEng_PerformanceLevel::High);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Optimized level with %d actors"), ActorCount);
}

void UEng_PerformanceManager::RunPerformanceTest()
{
    UpdateMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE TEST ==="));
    UE_LOG(LogTemp, Warning, TEXT("Frame Rate: %.2f FPS"), CurrentMetrics.FrameRate);
    UE_LOG(LogTemp, Warning, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Actor Count: %d"), CurrentMetrics.ActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Performance Critical: %s"), 
           CurrentMetrics.bIsPerformanceCritical ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %d"), (int32)CurrentPerformanceLevel);
}

void UEng_PerformanceManager::UpdateMetrics()
{
    // Update frame timing
    float CurrentTime = FPlatformTime::Seconds();
    if (LastFrameTime > 0.0f)
    {
        CurrentMetrics.FrameTime = (CurrentTime - LastFrameTime) * 1000.0f; // Convert to ms
        CurrentMetrics.FrameRate = 1.0f / (CurrentTime - LastFrameTime);
    }
    LastFrameTime = CurrentTime;
    
    // Update actor count
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.ActorCount = AllActors.Num();
    }
    
    // Estimate memory usage (simplified)
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // Update draw calls (simplified estimation)
    CurrentMetrics.DrawCalls = FMath::Max(1, CurrentMetrics.ActorCount / 10);
    
    CheckPerformanceThresholds();
}

void UEng_PerformanceManager::CheckPerformanceThresholds()
{
    // Check if performance is critical
    bool bWasCritical = CurrentMetrics.bIsPerformanceCritical;
    
    CurrentMetrics.bIsPerformanceCritical = 
        (CurrentMetrics.FrameTime > CriticalFrameTimeThreshold) ||
        (CurrentMetrics.ActorCount > 8000) ||
        (CurrentMetrics.MemoryUsageMB > 4096.0f);
    
    // If performance became critical, auto-optimize
    if (CurrentMetrics.bIsPerformanceCritical && !bWasCritical)
    {
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Performance critical - auto-optimizing"));
        OptimizeLevel();
    }
}

void UEng_PerformanceManager::ApplyPerformanceOptimizations()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Apply console commands based on performance level
    switch (CurrentPerformanceLevel)
    {
        case EEng_PerformanceLevel::Low:
            GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.5"));
            GEngine->Exec(World, TEXT("r.ShadowQuality 1"));
            GEngine->Exec(World, TEXT("r.PostProcessQuality 1"));
            break;
            
        case EEng_PerformanceLevel::Medium:
            GEngine->Exec(World, TEXT("r.ViewDistanceScale 0.75"));
            GEngine->Exec(World, TEXT("r.ShadowQuality 2"));
            GEngine->Exec(World, TEXT("r.PostProcessQuality 2"));
            break;
            
        case EEng_PerformanceLevel::High:
            GEngine->Exec(World, TEXT("r.ViewDistanceScale 1.0"));
            GEngine->Exec(World, TEXT("r.ShadowQuality 3"));
            GEngine->Exec(World, TEXT("r.PostProcessQuality 3"));
            break;
    }
}
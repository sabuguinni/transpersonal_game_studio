#include "Perf_OptimizationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/Console.h"

APerf_OptimizationManager::APerf_OptimizationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    SetActorTickEnabled(true);
    
    // Initialize default settings
    OptimizationSettings.TargetFrameRate = 60.0f;
    OptimizationSettings.MinFrameRate = 30.0f;
    OptimizationSettings.MaxPhysicsActors = 500;
    OptimizationSettings.ViewDistanceScale = 1.0f;
    OptimizationSettings.ShadowResolution = 2048;
    OptimizationSettings.bEnableAutomaticOptimization = true;
    
    MetricsUpdateInterval = 1.0f;
    bEnableDebugDisplay = true;
    
    FrameRateHistory.Reserve(MaxHistorySize);
}

void APerf_OptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Optimization Manager started"));
    
    // Apply initial optimization settings
    ApplyOptimizationSettings();
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
}

void APerf_OptimizationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastMetricsUpdate += DeltaTime;
    
    if (LastMetricsUpdate >= MetricsUpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (OptimizationSettings.bEnableAutomaticOptimization)
        {
            ApplyAutomaticOptimizations();
        }
        
        if (bEnableDebugDisplay)
        {
            DisplayDebugInfo();
        }
        
        LastMetricsUpdate = 0.0f;
    }
}

void APerf_OptimizationManager::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate frame rate metrics
    CalculateFrameRateMetrics();
    
    // Count physics actors
    CountPhysicsActors();
    
    // Update memory metrics
    UpdateMemoryMetrics();
    
    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.TotalActorCount = AllActors.Num();
}

void APerf_OptimizationManager::CalculateFrameRateMetrics()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        CurrentMetrics.CurrentFPS = CurrentFPS;
        
        // Add to history
        FrameRateHistory.Add(CurrentFPS);
        if (FrameRateHistory.Num() > MaxHistorySize)
        {
            FrameRateHistory.RemoveAt(0);
        }
        
        // Calculate average
        if (FrameRateHistory.Num() > 0)
        {
            float Total = 0.0f;
            for (float FPS : FrameRateHistory)
            {
                Total += FPS;
            }
            CurrentMetrics.AverageFPS = Total / FrameRateHistory.Num();
        }
    }
}

void APerf_OptimizationManager::CountPhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    int32 PhysicsCount = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsCount++;
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsActors = PhysicsCount;
}

void APerf_OptimizationManager::UpdateMemoryMetrics()
{
    // Get memory stats
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    CurrentMetrics.MemoryUsageMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    
    // GPU memory is harder to get, approximate
    CurrentMetrics.GPUMemoryUsageMB = CurrentMetrics.MemoryUsageMB * 0.3f; // Rough estimate
}

void APerf_OptimizationManager::ApplyOptimizationSettings()
{
    if (!GetWorld())
    {
        return;
    }
    
    UConsole* Console = GEngine->GetGameViewport()->ViewportConsole;
    if (Console)
    {
        // Apply view distance scaling
        FString ViewDistanceCmd = FString::Printf(TEXT("r.ViewDistanceScale %f"), OptimizationSettings.ViewDistanceScale);
        Console->ConsoleCommand(*ViewDistanceCmd);
        
        // Apply shadow resolution
        FString ShadowCmd = FString::Printf(TEXT("r.Shadow.MaxResolution %d"), OptimizationSettings.ShadowResolution);
        Console->ConsoleCommand(*ShadowCmd);
        
        // Apply frame rate target
        FString FrameRateCmd = FString::Printf(TEXT("t.MaxFPS %f"), OptimizationSettings.TargetFrameRate);
        Console->ConsoleCommand(*FrameRateCmd);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Applied optimization settings: ViewDistance=%.2f, ShadowRes=%d, TargetFPS=%.1f"),
        OptimizationSettings.ViewDistanceScale, OptimizationSettings.ShadowResolution, OptimizationSettings.TargetFrameRate);
}

void APerf_OptimizationManager::SetTargetFrameRate(float NewTargetFPS)
{
    OptimizationSettings.TargetFrameRate = FMath::Clamp(NewTargetFPS, 30.0f, 120.0f);
    ApplyOptimizationSettings();
}

void APerf_OptimizationManager::EnableAutomaticOptimization(bool bEnable)
{
    OptimizationSettings.bEnableAutomaticOptimization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Automatic optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void APerf_OptimizationManager::OptimizePhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxPhysicsActors)
    {
        int32 ActorsToDisable = CurrentMetrics.ActivePhysicsActors - OptimizationSettings.MaxPhysicsActors;
        int32 DisabledCount = 0;
        
        for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr && DisabledCount < ActorsToDisable; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    // Disable physics for distant or less important actors
                    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
                    float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                    
                    if (Distance > 5000.0f) // 50 meters
                    {
                        PrimComp->SetSimulatePhysics(false);
                        DisabledCount++;
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Optimized physics: disabled %d actors"), DisabledCount);
    }
}

void APerf_OptimizationManager::OptimizeRenderingSettings()
{
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.MinFrameRate)
    {
        // Reduce quality settings
        OptimizationSettings.ViewDistanceScale = FMath::Max(0.5f, OptimizationSettings.ViewDistanceScale - 0.1f);
        OptimizationSettings.ShadowResolution = FMath::Max(512, OptimizationSettings.ShadowResolution - 256);
        
        ApplyOptimizationSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Applied emergency optimization: ViewDistance=%.2f, ShadowRes=%d"),
            OptimizationSettings.ViewDistanceScale, OptimizationSettings.ShadowResolution);
    }
}

void APerf_OptimizationManager::ResetToDefaultSettings()
{
    OptimizationSettings.TargetFrameRate = 60.0f;
    OptimizationSettings.MinFrameRate = 30.0f;
    OptimizationSettings.MaxPhysicsActors = 500;
    OptimizationSettings.ViewDistanceScale = 1.0f;
    OptimizationSettings.ShadowResolution = 2048;
    OptimizationSettings.bEnableAutomaticOptimization = true;
    
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Reset to default optimization settings"));
}

void APerf_OptimizationManager::ApplyAutomaticOptimizations()
{
    // Check if frame rate is below minimum
    if (CurrentMetrics.CurrentFPS < OptimizationSettings.MinFrameRate)
    {
        OptimizeRenderingSettings();
        OptimizePhysicsActors();
    }
    
    // Check if too many physics actors
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxPhysicsActors)
    {
        OptimizePhysicsActors();
    }
}

void APerf_OptimizationManager::DisplayDebugInfo()
{
    if (GEngine)
    {
        FString DebugText = FString::Printf(
            TEXT("Performance Monitor | FPS: %.1f (Avg: %.1f) | Physics Actors: %d | Total Actors: %d | Memory: %.1f MB"),
            CurrentMetrics.CurrentFPS,
            CurrentMetrics.AverageFPS,
            CurrentMetrics.ActivePhysicsActors,
            CurrentMetrics.TotalActorCount,
            CurrentMetrics.MemoryUsageMB
        );
        
        GEngine->AddOnScreenDebugMessage(-1, 0.1f, FColor::Green, DebugText);
    }
}
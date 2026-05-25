#include "Perf_PerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"

UPerf_PerformanceManager::UPerf_PerformanceManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize platform targets for PC High-End
    PlatformTargets.TargetFPS = 60.0f;
    PlatformTargets.MaxFrameTime = 16.67f;
    PlatformTargets.MaxActors = 10000;
    PlatformTargets.MaxDrawCalls = 2000;
    PlatformTargets.MaxMemoryMB = 8192.0f;
    
    FrameTimeHistory.Reserve(MaxFrameHistorySize);
}

void UPerf_PerformanceManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoOptimize)
    {
        StartPerformanceMonitoring();
    }
    
    ApplyPlatformSettings();
    ApplyPerformanceLevelSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Manager initialized for platform: %d"), (int32)CurrentPlatform);
}

void UPerf_PerformanceManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bIsMonitoring)
    {
        LastMonitorTime += DeltaTime;
        if (LastMonitorTime >= MonitoringInterval)
        {
            UpdatePerformanceMetrics();
            CheckPerformanceThresholds();
            LastMonitorTime = 0.0f;
        }
    }
    
    // Auto-optimization with cooldown
    if (bAutoOptimize && (GetWorld()->GetTimeSeconds() - LastOptimizationTime) > OptimizationCooldown)
    {
        if (!IsPerformanceTargetMet())
        {
            AutoOptimizePerformance();
            LastOptimizationTime = GetWorld()->GetTimeSeconds();
        }
    }
}

void UPerf_PerformanceManager::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    FrameTimeHistory.Empty();
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring started"));
}

void UPerf_PerformanceManager::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Log, TEXT("Performance monitoring stopped"));
}

FPerf_PerformanceMetrics UPerf_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool UPerf_PerformanceManager::IsPerformanceTargetMet() const
{
    return CurrentMetrics.CurrentFPS >= (PlatformTargets.TargetFPS * 0.9f) && 
           CurrentMetrics.AverageFrameTime <= (PlatformTargets.MaxFrameTime * 1.1f);
}

void UPerf_PerformanceManager::SetPlatformType(EPerf_PlatformType NewPlatform)
{
    CurrentPlatform = NewPlatform;
    ApplyPlatformSettings();
    UE_LOG(LogTemp, Log, TEXT("Platform type changed to: %d"), (int32)NewPlatform);
}

void UPerf_PerformanceManager::SetPerformanceLevel(EPerf_PerformanceLevel NewLevel)
{
    CurrentPerformanceLevel = NewLevel;
    ApplyPerformanceLevelSettings();
    UE_LOG(LogTemp, Log, TEXT("Performance level changed to: %d"), (int32)NewLevel);
}

void UPerf_PerformanceManager::AutoOptimizePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimizing performance - FPS: %.1f, Target: %.1f"), 
           CurrentMetrics.CurrentFPS, PlatformTargets.TargetFPS);
    
    // Progressive optimization based on performance deficit
    float performanceRatio = CurrentMetrics.CurrentFPS / PlatformTargets.TargetFPS;
    
    if (performanceRatio < 0.7f)
    {
        // Severe performance issues - aggressive optimization
        OptimizeRenderingSettings();
        OptimizePhysicsSettings();
        OptimizeActorLOD();
        CullDistantActors();
    }
    else if (performanceRatio < 0.9f)
    {
        // Moderate performance issues - targeted optimization
        OptimizeActorLOD();
        AdjustLODDistances();
    }
}

void UPerf_PerformanceManager::OptimizeActorLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    // Optimize actors in chunks to spread load
    OptimizeActorsInRadius(PlayerLocation, 5000.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Actor LOD optimization completed"));
}

void UPerf_PerformanceManager::OptimizePhysicsSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Reduce physics simulation complexity based on performance level
    switch (CurrentPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Low:
        case EPerf_PerformanceLevel::Potato:
            // Disable physics on distant objects
            for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
            {
                AActor* Actor = *ActorItr;
                if (Actor && Actor->GetRootComponent())
                {
                    UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                    if (PrimComp && PrimComp->IsSimulatingPhysics())
                    {
                        FVector ActorLocation = Actor->GetActorLocation();
                        FVector PlayerLocation = FVector::ZeroVector;
                        if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
                        {
                            PlayerLocation = PlayerPawn->GetActorLocation();
                        }
                        
                        float Distance = FVector::Dist(ActorLocation, PlayerLocation);
                        if (Distance > 2000.0f)
                        {
                            PrimComp->SetSimulatePhysics(false);
                        }
                    }
                }
            }
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization completed"));
}

void UPerf_PerformanceManager::OptimizeRenderingSettings()
{
    // Apply rendering optimizations based on performance level
    switch (CurrentPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Low:
            // Reduce shadow quality
            if (GEngine && GEngine->GetGameUserSettings())
            {
                GEngine->GetGameUserSettings()->SetShadowQuality(1);
                GEngine->GetGameUserSettings()->SetTextureQuality(1);
                GEngine->GetGameUserSettings()->SetEffectsQuality(1);
            }
            break;
        case EPerf_PerformanceLevel::Potato:
            // Minimal rendering quality
            if (GEngine && GEngine->GetGameUserSettings())
            {
                GEngine->GetGameUserSettings()->SetShadowQuality(0);
                GEngine->GetGameUserSettings()->SetTextureQuality(0);
                GEngine->GetGameUserSettings()->SetEffectsQuality(0);
                GEngine->GetGameUserSettings()->SetPostProcessingQuality(0);
            }
            break;
        default:
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Rendering optimization completed"));
}

void UPerf_PerformanceManager::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Calculate FPS and frame time
    float DeltaTime = World->GetDeltaSeconds();
    if (DeltaTime > 0.0f)
    {
        CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
        CurrentMetrics.AverageFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Update frame time history
        FrameTimeHistory.Add(CurrentMetrics.AverageFrameTime);
        if (FrameTimeHistory.Num() > MaxFrameHistorySize)
        {
            FrameTimeHistory.RemoveAt(0);
        }
        
        // Calculate average frame time
        if (FrameTimeHistory.Num() > 0)
        {
            float TotalFrameTime = 0.0f;
            for (float FrameTime : FrameTimeHistory)
            {
                TotalFrameTime += FrameTime;
            }
            CurrentMetrics.AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
        }
    }
    
    // Count actors
    CurrentMetrics.ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        CurrentMetrics.ActorCount++;
    }
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
    
    // Estimate draw calls (simplified based on visible actors)
    CurrentMetrics.DrawCalls = CurrentMetrics.ActorCount / 5; // Rough estimate
}

void UPerf_PerformanceManager::CheckPerformanceThresholds()
{
    bool bPreviousWarning = bPerformanceWarning;
    
    bPerformanceWarning = !IsPerformanceTargetMet();
    
    if (bPerformanceWarning && !bPreviousWarning)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance warning: FPS %.1f below target %.1f"), 
               CurrentMetrics.CurrentFPS, PlatformTargets.TargetFPS);
    }
    else if (!bPerformanceWarning && bPreviousWarning)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance recovered: FPS %.1f"), CurrentMetrics.CurrentFPS);
    }
}

void UPerf_PerformanceManager::ApplyPlatformSettings()
{
    switch (CurrentPlatform)
    {
        case EPerf_PlatformType::PC_HighEnd:
            PlatformTargets.TargetFPS = 60.0f;
            PlatformTargets.MaxFrameTime = 16.67f;
            PlatformTargets.MaxActors = 15000;
            PlatformTargets.MaxDrawCalls = 3000;
            PlatformTargets.MaxMemoryMB = 16384.0f;
            break;
        case EPerf_PlatformType::PC_MidRange:
            PlatformTargets.TargetFPS = 60.0f;
            PlatformTargets.MaxFrameTime = 16.67f;
            PlatformTargets.MaxActors = 10000;
            PlatformTargets.MaxDrawCalls = 2000;
            PlatformTargets.MaxMemoryMB = 8192.0f;
            break;
        case EPerf_PlatformType::PC_LowEnd:
            PlatformTargets.TargetFPS = 30.0f;
            PlatformTargets.MaxFrameTime = 33.33f;
            PlatformTargets.MaxActors = 5000;
            PlatformTargets.MaxDrawCalls = 1000;
            PlatformTargets.MaxMemoryMB = 4096.0f;
            break;
        case EPerf_PlatformType::Console_Next:
            PlatformTargets.TargetFPS = 60.0f;
            PlatformTargets.MaxFrameTime = 16.67f;
            PlatformTargets.MaxActors = 12000;
            PlatformTargets.MaxDrawCalls = 2500;
            PlatformTargets.MaxMemoryMB = 12288.0f;
            break;
        case EPerf_PlatformType::Console_Current:
            PlatformTargets.TargetFPS = 30.0f;
            PlatformTargets.MaxFrameTime = 33.33f;
            PlatformTargets.MaxActors = 8000;
            PlatformTargets.MaxDrawCalls = 1500;
            PlatformTargets.MaxMemoryMB = 6144.0f;
            break;
        case EPerf_PlatformType::Mobile:
            PlatformTargets.TargetFPS = 30.0f;
            PlatformTargets.MaxFrameTime = 33.33f;
            PlatformTargets.MaxActors = 2000;
            PlatformTargets.MaxDrawCalls = 500;
            PlatformTargets.MaxMemoryMB = 2048.0f;
            break;
    }
}

void UPerf_PerformanceManager::ApplyPerformanceLevelSettings()
{
    switch (CurrentPerformanceLevel)
    {
        case EPerf_PerformanceLevel::Ultra:
            MonitoringInterval = 0.5f;
            bEnableDetailedProfiling = true;
            break;
        case EPerf_PerformanceLevel::High:
            MonitoringInterval = 1.0f;
            bEnableDetailedProfiling = true;
            break;
        case EPerf_PerformanceLevel::Medium:
            MonitoringInterval = 1.0f;
            bEnableDetailedProfiling = false;
            break;
        case EPerf_PerformanceLevel::Low:
            MonitoringInterval = 2.0f;
            bEnableDetailedProfiling = false;
            break;
        case EPerf_PerformanceLevel::Potato:
            MonitoringInterval = 5.0f;
            bEnableDetailedProfiling = false;
            break;
    }
}

void UPerf_PerformanceManager::OptimizeActorsInRadius(const FVector& Center, float Radius)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 OptimizedCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), Center);
        
        // Apply LOD based on distance
        if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            if (Distance > 3000.0f)
            {
                MeshComp->SetForcedLodModel(3); // Lowest LOD
            }
            else if (Distance > 1500.0f)
            {
                MeshComp->SetForcedLodModel(2);
            }
            else if (Distance > 500.0f)
            {
                MeshComp->SetForcedLodModel(1);
            }
            else
            {
                MeshComp->SetForcedLodModel(0); // Highest LOD
            }
            OptimizedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d actors in radius %.1f"), OptimizedCount, Radius);
}

void UPerf_PerformanceManager::CullDistantActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    FVector PlayerLocation = FVector::ZeroVector;
    if (APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn())
    {
        PlayerLocation = PlayerPawn->GetActorLocation();
    }
    
    float CullDistance = 10000.0f; // 10km
    int32 CulledCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor->IsA<APawn>()) continue; // Don't cull pawns
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        if (Distance > CullDistance)
        {
            Actor->SetActorHiddenInGame(true);
            CulledCount++;
        }
        else
        {
            Actor->SetActorHiddenInGame(false);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Culled %d distant actors"), CulledCount);
}

void UPerf_PerformanceManager::AdjustLODDistances()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Adjust LOD distances based on current performance
    float PerformanceRatio = CurrentMetrics.CurrentFPS / PlatformTargets.TargetFPS;
    float LODMultiplier = FMath::Clamp(PerformanceRatio, 0.5f, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Adjusting LOD distances with multiplier: %.2f"), LODMultiplier);
}
#include "Perf_PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Default performance targets
    TargetFPS_PC = 60.0f;
    TargetFPS_Console = 30.0f;
    MaxMemoryUsageMB = 8192.0f; // 8GB
    MaxActorCount = 10000;

    // Initialize profiling state
    bIsProfiling = false;
    CurrentProfilingLevel = EPerf_ProfilingLevel::Basic;
    FrameHistorySize = 60; // 6 seconds of history at 10Hz
    ProfilingTimer = 0.0f;
    ProfilingInterval = 1.0f; // Log every second
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(FrameHistorySize);
    
    // Start profiling automatically
    StartProfiling();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized"));
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bIsProfiling)
        return;

    // Update performance metrics
    UpdateMetrics(DeltaTime);

    // Periodic logging
    ProfilingTimer += DeltaTime;
    if (ProfilingTimer >= ProfilingInterval)
    {
        ProfilingTimer = 0.0f;
        
        if (CurrentProfilingLevel >= EPerf_ProfilingLevel::Basic)
        {
            LogPerformanceReport();
        }

        // Auto-optimize if performance is poor
        if (!IsPerformanceAcceptable())
        {
            OptimizePerformance();
        }
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfiling = true;
    FrameTimeHistory.Empty();
    ProfilingTimer = 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_PerformanceProfiler::SetProfilingLevel(EPerf_ProfilingLevel NewLevel)
{
    CurrentProfilingLevel = NewLevel;
    UE_LOG(LogTemp, Log, TEXT("Profiling level set to: %d"), (int32)NewLevel);
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: %.1f | Frame Time: %.2fms"), CurrentMetrics.CurrentFPS, CurrentMetrics.AverageFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Memory: %.1fMB | Actors: %d"), CurrentMetrics.MemoryUsageMB, CurrentMetrics.ActorCount);
    UE_LOG(LogTemp, Log, TEXT("CPU Time: %.2fms | GPU Time: %.2fms"), CurrentMetrics.CPUTime, CurrentMetrics.GPUTime);
    
    if (!IsPerformanceAcceptable())
    {
        UE_LOG(LogTemp, Warning, TEXT("PERFORMANCE WARNING: Below target thresholds"));
    }
}

void UPerf_PerformanceProfiler::OptimizeActorLOD(AActor* Actor, float Distance)
{
    if (!Actor)
        return;

    // Get static mesh components
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);

    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        if (MeshComp && MeshComp->GetStaticMesh())
        {
            // Set LOD based on distance
            int32 ForcedLOD = -1; // Auto LOD
            
            if (Distance > 5000.0f)
            {
                ForcedLOD = 3; // Lowest detail
            }
            else if (Distance > 2000.0f)
            {
                ForcedLOD = 2;
            }
            else if (Distance > 1000.0f)
            {
                ForcedLOD = 1;
            }
            
            MeshComp->SetForcedLodModel(ForcedLOD);
        }
    }

    // Get skeletal mesh components (for dinosaurs)
    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);

    for (USkeletalMeshComponent* SkelMeshComp : SkeletalMeshes)
    {
        if (SkelMeshComp && SkelMeshComp->GetSkeletalMeshAsset())
        {
            // Reduce animation update rate for distant actors
            if (Distance > 3000.0f)
            {
                SkelMeshComp->SetComponentTickEnabled(false);
            }
            else if (Distance > 1500.0f)
            {
                SkelMeshComp->SetComponentTickInterval(0.1f); // 10 FPS animation
            }
            else
            {
                SkelMeshComp->SetComponentTickInterval(0.0f); // Full rate
                SkelMeshComp->SetComponentTickEnabled(true);
            }
        }
    }
}

void UPerf_PerformanceProfiler::CullDistantActors(float CullDistance)
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
        return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == PlayerPawn)
            continue;

        float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
        
        if (Distance > CullDistance)
        {
            // Hide distant actors
            Actor->SetActorHiddenInGame(true);
            Actor->SetActorTickEnabled(false);
        }
        else
        {
            // Show nearby actors
            Actor->SetActorHiddenInGame(false);
            Actor->SetActorTickEnabled(true);
            
            // Apply LOD optimization
            OptimizeActorLOD(Actor, Distance);
        }
    }
}

bool UPerf_PerformanceProfiler::IsPerformanceAcceptable() const
{
    // Check FPS target (PC vs Console detection would be more complex in real implementation)
    float TargetFPS = TargetFPS_PC; // Assume PC for now
    
    bool FPSAcceptable = CurrentMetrics.CurrentFPS >= (TargetFPS * 0.9f); // 90% of target
    bool MemoryAcceptable = CurrentMetrics.MemoryUsageMB <= MaxMemoryUsageMB;
    bool ActorCountAcceptable = CurrentMetrics.ActorCount <= MaxActorCount;
    
    return FPSAcceptable && MemoryAcceptable && ActorCountAcceptable;
}

void UPerf_PerformanceProfiler::UpdateMetrics(float DeltaTime)
{
    // Update frame time history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > FrameHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }

    // Calculate current FPS
    CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime() * 1000.0f; // Convert to milliseconds

    // Collect metrics based on profiling level
    switch (CurrentProfilingLevel)
    {
        case EPerf_ProfilingLevel::Basic:
            CollectBasicMetrics();
            break;
        case EPerf_ProfilingLevel::Detailed:
            CollectDetailedMetrics();
            break;
        case EPerf_ProfilingLevel::Advanced:
            CollectAdvancedMetrics();
            break;
        default:
            break;
    }
}

void UPerf_PerformanceProfiler::CollectBasicMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
        return;

    // Count actors
    CurrentMetrics.ActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        CurrentMetrics.ActorCount++;
    }

    // Basic memory estimation (simplified)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActorCount * 0.1f; // Rough estimate
}

void UPerf_PerformanceProfiler::CollectDetailedMetrics()
{
    CollectBasicMetrics();
    
    // More detailed memory calculation would require platform-specific code
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.ActorCount * 0.15f; // Slightly more detailed estimate
    
    // Estimate draw calls (simplified)
    CurrentMetrics.DrawCalls = CurrentMetrics.ActorCount / 2; // Rough estimate
}

void UPerf_PerformanceProfiler::CollectAdvancedMetrics()
{
    CollectDetailedMetrics();
    
    // Advanced metrics would require engine stats integration
    CurrentMetrics.CPUTime = CurrentMetrics.AverageFrameTime * 0.7f; // Estimate 70% CPU
    CurrentMetrics.GPUTime = CurrentMetrics.AverageFrameTime * 0.3f; // Estimate 30% GPU
}

float UPerf_PerformanceProfiler::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0)
        return 0.0f;

    float Sum = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Sum += FrameTime;
    }

    return Sum / FrameTimeHistory.Num();
}

void UPerf_PerformanceProfiler::OptimizePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization triggered due to poor performance"));
    
    // Cull distant actors aggressively
    CullDistantActors(5000.0f);
    
    // Reduce tick rates for non-critical actors
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && !Actor->IsA<APawn>()) // Don't affect player or important pawns
            {
                Actor->SetActorTickInterval(0.1f); // Reduce to 10 FPS
            }
        }
    }
}
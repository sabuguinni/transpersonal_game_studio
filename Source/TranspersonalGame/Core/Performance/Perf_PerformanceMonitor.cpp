#include "Perf_PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"

UPerf_PerformanceMonitor::UPerf_PerformanceMonitor()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    bMonitoringActive = false;
    MonitoringStartTime = 0.0f;
    TotalFrameTime = 0.0f;
    FrameCount = 0;
    
    AverageFrameRate = 0.0f;
    MinFrameRateRecorded = 999.0f;
    MaxFrameRateRecorded = 0.0f;
    bIsPerformanceCritical = false;
}

void UPerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    StartPerformanceMonitoring();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Started monitoring for actor %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UPerf_PerformanceMonitor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bMonitoringActive)
    {
        UpdateFrameData(DeltaTime);
        CheckPerformanceThresholds();
        
        // Apply optimizations if performance is critical
        if (bIsPerformanceCritical)
        {
            ApplyPerformanceOptimizations();
        }
    }
}

void UPerf_PerformanceMonitor::StartPerformanceMonitoring()
{
    bMonitoringActive = true;
    MonitoringStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    ResetPerformanceData();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Monitoring started"));
}

void UPerf_PerformanceMonitor::StopPerformanceMonitoring()
{
    bMonitoringActive = false;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Monitoring stopped. Average FPS: %.2f"), AverageFrameRate);
}

void UPerf_PerformanceMonitor::ResetPerformanceData()
{
    FrameHistory.Empty();
    TotalFrameTime = 0.0f;
    FrameCount = 0;
    AverageFrameRate = 0.0f;
    MinFrameRateRecorded = 999.0f;
    MaxFrameRateRecorded = 0.0f;
    bIsPerformanceCritical = false;
    
    CurrentFrameData = FPerf_FrameData();
}

void UPerf_PerformanceMonitor::UpdateFrameData(float DeltaTime)
{
    if (DeltaTime <= 0.0f) return;
    
    // Update current frame data
    CurrentFrameData.DeltaTime = DeltaTime;
    CurrentFrameData.FrameRate = 1.0f / DeltaTime;
    CurrentFrameData.ActorCount = GetActiveActorCount();
    CurrentFrameData.MemoryUsageMB = GetMemoryUsage();
    CurrentFrameData.GameThreadTime = DeltaTime * 1000.0f; // Convert to ms
    CurrentFrameData.RenderThreadTime = DeltaTime * 1000.0f; // Simplified
    
    // Add to history
    FrameHistory.Add(CurrentFrameData);
    if (FrameHistory.Num() > MaxFrameHistorySize)
    {
        FrameHistory.RemoveAt(0);
    }
    
    // Update statistics
    TotalFrameTime += DeltaTime;
    FrameCount++;
    
    if (FrameCount > 0)
    {
        AverageFrameRate = FrameCount / TotalFrameTime;
    }
    
    // Update min/max frame rates
    if (CurrentFrameData.FrameRate < MinFrameRateRecorded)
    {
        MinFrameRateRecorded = CurrentFrameData.FrameRate;
    }
    if (CurrentFrameData.FrameRate > MaxFrameRateRecorded)
    {
        MaxFrameRateRecorded = CurrentFrameData.FrameRate;
    }
}

void UPerf_PerformanceMonitor::CheckPerformanceThresholds()
{
    bool bWasCritical = bIsPerformanceCritical;
    
    // Check if performance is below minimum threshold
    bIsPerformanceCritical = (CurrentFrameData.FrameRate < OptimizationSettings.MinFrameRate) ||
                            (CurrentFrameData.ActorCount > OptimizationSettings.MaxActorsPerFrame);
    
    // Log performance state changes
    if (bIsPerformanceCritical && !bWasCritical)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Monitor: Performance critical! FPS: %.2f, Actors: %d"), 
               CurrentFrameData.FrameRate, CurrentFrameData.ActorCount);
    }
    else if (!bIsPerformanceCritical && bWasCritical)
    {
        UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Performance recovered. FPS: %.2f"), 
               CurrentFrameData.FrameRate);
    }
}

void UPerf_PerformanceMonitor::ApplyPerformanceOptimizations()
{
    if (OptimizationSettings.bEnableAutoLOD)
    {
        OptimizeLODSettings();
    }
    
    if (OptimizationSettings.bEnableOcclusion)
    {
        OptimizeActorCulling();
    }
}

void UPerf_PerformanceMonitor::OptimizeLODSettings()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Get all actors in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == GetOwner()) continue;
        
        // Optimize static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp)
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation());
                
                // Apply LOD based on distance
                if (Distance > OptimizationSettings.CullingDistance * 0.8f)
                {
                    MeshComp->SetForcedLodModel(3); // Lowest LOD
                }
                else if (Distance > OptimizationSettings.CullingDistance * 0.5f)
                {
                    MeshComp->SetForcedLodModel(2); // Medium LOD
                }
                else if (Distance > OptimizationSettings.CullingDistance * 0.2f)
                {
                    MeshComp->SetForcedLodModel(1); // High LOD
                }
                else
                {
                    MeshComp->SetForcedLodModel(0); // Highest LOD
                }
            }
        }
    }
}

void UPerf_PerformanceMonitor::OptimizeActorCulling()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Cull actors beyond culling distance
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor || Actor == Owner) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), OwnerLocation);
        
        if (Distance > OptimizationSettings.CullingDistance)
        {
            Actor->SetActorHiddenInGame(true);
        }
        else
        {
            Actor->SetActorHiddenInGame(false);
        }
    }
}

float UPerf_PerformanceMonitor::GetMemoryUsage() const
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return MemStats.UsedPhysical / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerf_PerformanceMonitor::GetActiveActorCount() const
{
    UWorld* World = GetWorld();
    if (!World) return 0;
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && !Actor->IsHidden())
        {
            Count++;
        }
    }
    
    return Count;
}

FPerf_FrameData UPerf_PerformanceMonitor::GetCurrentPerformanceData() const
{
    return CurrentFrameData;
}

float UPerf_PerformanceMonitor::GetAverageFrameRate() const
{
    return AverageFrameRate;
}

bool UPerf_PerformanceMonitor::IsPerformanceCritical() const
{
    return bIsPerformanceCritical;
}

void UPerf_PerformanceMonitor::SetOptimizationSettings(const FPerf_OptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Updated optimization settings. Target FPS: %.2f"), 
           OptimizationSettings.TargetFrameRate);
}

void UPerf_PerformanceMonitor::RunPerformanceTest()
{
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Running performance test..."));
    
    ResetPerformanceData();
    
    // Force a brief monitoring period
    float TestDuration = 5.0f; // 5 seconds
    MonitoringStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Monitor: Test started. Duration: %.2f seconds"), TestDuration);
}
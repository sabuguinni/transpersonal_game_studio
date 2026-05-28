#include "Perf_PerformanceProfiler.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    ProfilerMode = EPerf_ProfilerMode::Basic;
    UpdateInterval = 1.0f;
    bEnableDetailedProfiling = false;
    bLogPerformanceWarnings = true;
    FPSWarningThreshold = 30.0f;
    MemoryWarningThresholdMB = 2048.0f;
    
    bIsProfilingActive = false;
    TimeSinceLastUpdate = 0.0f;
    TotalFPSSum = 0.0f;
    FPSHistoryCount = 0;
    
    // Initialize FPS history array
    FPSHistory.Reserve(60); // Store 60 seconds of FPS data
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    if (ProfilerMode != EPerf_ProfilerMode::Disabled)
    {
        StartProfiling();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized - Mode: %d"), (int32)ProfilerMode);
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfilingActive || ProfilerMode == EPerf_ProfilerMode::Disabled)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableDetailedProfiling)
        {
            CollectActorPerformanceData();
        }
        
        CheckPerformanceThresholds();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    if (bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = true;
    ResetMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    if (!bIsProfilingActive)
    {
        return;
    }
    
    bIsProfilingActive = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    ActorPerformanceData.Empty();
    FPSHistory.Empty();
    TotalFPSSum = 0.0f;
    FPSHistoryCount = 0;
    TimeSinceLastUpdate = 0.0f;
}

FPerf_PerformanceMetrics UPerf_PerformanceProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

TArray<FPerf_ActorPerformanceData> UPerf_PerformanceProfiler::GetActorPerformanceData() const
{
    return ActorPerformanceData;
}

void UPerf_PerformanceProfiler::SetProfilerMode(EPerf_ProfilerMode NewMode)
{
    ProfilerMode = NewMode;
    
    if (NewMode == EPerf_ProfilerMode::Disabled)
    {
        StopProfiling();
    }
    else if (!bIsProfilingActive)
    {
        StartProfiling();
    }
}

bool UPerf_PerformanceProfiler::IsPerformanceAcceptable() const
{
    return CurrentMetrics.CurrentFPS >= FPSWarningThreshold && 
           CurrentMetrics.MemoryUsageMB <= MemoryWarningThresholdMB;
}

void UPerf_PerformanceProfiler::OptimizePerformance()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Get all actors for optimization
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 OptimizedActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        // Optimize static mesh components
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        
        for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
        {
            if (MeshComp && MeshComp->GetStaticMesh())
            {
                // Enable LOD if not already enabled
                if (!MeshComp->bOverrideLightMapRes)
                {
                    MeshComp->bOverrideLightMapRes = true;
                    MeshComp->OverriddenLightMapRes = 32; // Lower resolution for performance
                    OptimizedActors++;
                }
            }
        }
        
        // Optimize skeletal mesh components
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        
        for (USkeletalMeshComponent* SkeletalComp : SkeletalMeshComponents)
        {
            if (SkeletalComp)
            {
                // Reduce update rate for distant actors
                float DistanceToPlayer = 10000.0f; // Default large distance
                
                if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetPawn())
                {
                    FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
                    DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                }
                
                if (DistanceToPlayer > 5000.0f)
                {
                    SkeletalComp->SetComponentTickInterval(0.2f); // Reduce tick rate for distant actors
                    OptimizedActors++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Performance optimization completed - %d actors optimized"), OptimizedActors);
}

void UPerf_PerformanceProfiler::RunPerformanceTest()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE TEST STARTED ==="));
    
    // Test 1: Actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), AllActors.Num());
    
    // Test 2: Memory usage estimation
    SIZE_T MemoryUsage = GEngine->GetWorldContextFromWorld(GetWorld())->World()->GetOutermost()->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
    float MemoryMB = MemoryUsage / (1024.0f * 1024.0f);
    UE_LOG(LogTemp, Warning, TEXT("Estimated Memory Usage: %.2f MB"), MemoryMB);
    
    // Test 3: Mesh complexity
    int32 TotalTriangles = 0;
    int32 StaticMeshCount = 0;
    int32 SkeletalMeshCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        TArray<UStaticMeshComponent*> StaticMeshComponents;
        Actor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
        StaticMeshCount += StaticMeshComponents.Num();
        
        TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
        Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
        SkeletalMeshCount += SkeletalMeshComponents.Num();
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Static Mesh Components: %d"), StaticMeshCount);
    UE_LOG(LogTemp, Warning, TEXT("Skeletal Mesh Components: %d"), SkeletalMeshCount);
    
    // Test 4: Performance recommendations
    if (AllActors.Num() > 10000)
    {
        UE_LOG(LogTemp, Error, TEXT("WARNING: High actor count may impact performance"));
    }
    
    if (MemoryMB > 2048.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("WARNING: High memory usage detected"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE TEST COMPLETED ==="));
}

void UPerf_PerformanceProfiler::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentMetrics.CurrentFPS = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    // Update FPS history
    UpdateFPSHistory(CurrentMetrics.CurrentFPS);
    
    // Calculate average FPS
    CurrentMetrics.AverageFPS = CalculateAverageFPS();
    
    // Update min/max FPS
    if (FPSHistoryCount == 1)
    {
        CurrentMetrics.MinFPS = CurrentMetrics.CurrentFPS;
        CurrentMetrics.MaxFPS = CurrentMetrics.CurrentFPS;
    }
    else
    {
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, CurrentMetrics.CurrentFPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, CurrentMetrics.CurrentFPS);
    }
    
    // Update actor count
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActorCount = AllActors.Num();
    
    // Estimate memory usage
    SIZE_T MemoryUsage = GEngine->GetWorldContextFromWorld(GetWorld())->World()->GetOutermost()->GetResourceSizeBytes(EResourceSizeMode::EstimatedTotal);
    CurrentMetrics.MemoryUsageMB = MemoryUsage / (1024.0f * 1024.0f);
    
    // Estimate draw calls (simplified)
    CurrentMetrics.DrawCalls = CurrentMetrics.ActorCount / 2; // Rough estimation
}

void UPerf_PerformanceProfiler::CollectActorPerformanceData()
{
    if (!GetWorld())
    {
        return;
    }
    
    ActorPerformanceData.Empty();
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (!Actor)
        {
            continue;
        }
        
        FPerf_ActorPerformanceData ActorData;
        ActorData.ActorName = Actor->GetName();
        
        // Check if actor has mesh components (performance critical)
        TArray<UMeshComponent*> MeshComponents;
        Actor->GetComponents<UMeshComponent>(MeshComponents);
        
        if (MeshComponents.Num() > 0)
        {
            ActorData.bIsPerformanceCritical = true;
            ActorData.TriangleCount = MeshComponents.Num() * 1000; // Rough estimation
        }
        
        // Estimate render time based on distance and complexity
        if (GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetPawn())
        {
            FVector PlayerLocation = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
            float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
            ActorData.RenderTime = (Distance < 1000.0f) ? 0.5f : 0.1f; // Simplified estimation
        }
        
        ActorPerformanceData.Add(ActorData);
        
        // Limit data collection to prevent performance impact
        if (ActorPerformanceData.Num() >= 100)
        {
            break;
        }
    }
}

void UPerf_PerformanceProfiler::CheckPerformanceThresholds()
{
    if (!bLogPerformanceWarnings)
    {
        return;
    }
    
    if (CurrentMetrics.CurrentFPS < FPSWarningThreshold)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Low FPS detected: %.1f (threshold: %.1f)"), 
                                            CurrentMetrics.CurrentFPS, FPSWarningThreshold));
    }
    
    if (CurrentMetrics.MemoryUsageMB > MemoryWarningThresholdMB)
    {
        LogPerformanceWarning(FString::Printf(TEXT("High memory usage: %.1f MB (threshold: %.1f MB)"), 
                                            CurrentMetrics.MemoryUsageMB, MemoryWarningThresholdMB));
    }
    
    if (CurrentMetrics.ActorCount > 15000)
    {
        LogPerformanceWarning(FString::Printf(TEXT("High actor count: %d"), CurrentMetrics.ActorCount));
    }
}

void UPerf_PerformanceProfiler::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("[PERFORMANCE WARNING] %s"), *Warning);
}

float UPerf_PerformanceProfiler::CalculateAverageFPS() const
{
    if (FPSHistoryCount == 0)
    {
        return 0.0f;
    }
    
    return TotalFPSSum / FPSHistoryCount;
}

void UPerf_PerformanceProfiler::UpdateFPSHistory(float NewFPS)
{
    // Add to running total
    TotalFPSSum += NewFPS;
    FPSHistoryCount++;
    
    // Add to history array
    FPSHistory.Add(NewFPS);
    
    // Keep only last 60 entries (60 seconds of data)
    if (FPSHistory.Num() > 60)
    {
        float RemovedFPS = FPSHistory[0];
        FPSHistory.RemoveAt(0);
        TotalFPSSum -= RemovedFPS;
        FPSHistoryCount--;
    }
}
#include "Perf_PerformanceAnalyzer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "RHI.h"

UPerf_PerformanceAnalyzer::UPerf_PerformanceAnalyzer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    bIsAnalyzing = false;
    AnalysisTimer = 0.0f;
    AnalysisInterval = 1.0f; // Analyze every second
    
    // Set default performance thresholds
    TargetFPS = 60.0f;
    MinAcceptableFPS = 30.0f;
    MaxMemoryUsageMB = 8192.0f; // 8GB
    MaxDrawCalls = 2000;
    
    // Initialize FPS history array
    FPSHistory.Reserve(60); // Store last 60 FPS samples
}

void UPerf_PerformanceAnalyzer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Analyzer initialized"));
    StartPerformanceAnalysis();
}

void UPerf_PerformanceAnalyzer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsAnalyzing)
        return;
    
    AnalysisTimer += DeltaTime;
    
    // Update current FPS
    CurrentMetrics.CurrentFPS = 1.0f / DeltaTime;
    UpdateFPSHistory(CurrentMetrics.CurrentFPS);
    
    // Perform full analysis at intervals
    if (AnalysisTimer >= AnalysisInterval)
    {
        AnalysisTimer = 0.0f;
        
        // Update all performance metrics
        CurrentMetrics.MemoryUsageMB = GetMemoryUsage();
        CurrentMetrics.DrawCalls = GetDrawCallCount();
        CurrentMetrics.GPUTime = GetGPUTime();
        CurrentMetrics.CPUTime = GetCPUTime();
        
        CalculateAverageMetrics();
        AnalyzeActorPerformance();
    }
}

FPerf_PerformanceMetrics UPerf_PerformanceAnalyzer::GetCurrentPerformanceMetrics()
{
    return CurrentMetrics;
}

TArray<FPerf_ActorPerformanceData> UPerf_PerformanceAnalyzer::GetActorPerformanceData()
{
    TArray<FPerf_ActorPerformanceData> ActorData;
    
    UWorld* World = GetWorld();
    if (!World)
        return ActorData;
    
    // Get player location for distance calculations
    APawn* PlayerPawn = World->GetFirstPlayerController()->GetPawn();
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    // Analyze all actors in the world
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor || Actor == PlayerPawn)
            continue;
        
        FPerf_ActorPerformanceData Data;
        Data.ActorName = Actor->GetName();
        Data.ActorClass = Actor->GetClass()->GetName();
        Data.DistanceToPlayer = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        
        // Check if actor has mesh components
        UStaticMeshComponent* StaticMeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        USkeletalMeshComponent* SkeletalMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>();
        
        if (StaticMeshComp && StaticMeshComp->GetStaticMesh())
        {
            Data.TriangleCount = StaticMeshComp->GetStaticMesh()->GetNumTriangles(0);
            Data.bIsVisible = StaticMeshComp->IsVisible();
        }
        else if (SkeletalMeshComp && SkeletalMeshComp->GetSkeletalMeshAsset())
        {
            // Estimate triangle count for skeletal mesh
            Data.TriangleCount = 1000; // Placeholder - actual implementation would need LOD data
            Data.bIsVisible = SkeletalMeshComp->IsVisible();
        }
        
        ActorData.Add(Data);
    }
    
    return ActorData;
}

void UPerf_PerformanceAnalyzer::StartPerformanceAnalysis()
{
    bIsAnalyzing = true;
    AnalysisTimer = 0.0f;
    ResetPerformanceMetrics();
    
    UE_LOG(LogTemp, Log, TEXT("Performance analysis started"));
}

void UPerf_PerformanceAnalyzer::StopPerformanceAnalysis()
{
    bIsAnalyzing = false;
    UE_LOG(LogTemp, Log, TEXT("Performance analysis stopped"));
}

void UPerf_PerformanceAnalyzer::ResetPerformanceMetrics()
{
    CurrentMetrics = FPerf_PerformanceMetrics();
    FPSHistory.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("Performance metrics reset"));
}

TArray<FString> UPerf_PerformanceAnalyzer::GetPerformanceOptimizationSuggestions()
{
    TArray<FString> Suggestions;
    
    // FPS-based suggestions
    if (CurrentMetrics.AverageFPS < MinAcceptableFPS)
    {
        Suggestions.Add(TEXT("FPS below minimum threshold - consider reducing quality settings"));
        Suggestions.Add(TEXT("Reduce shadow quality and distance"));
        Suggestions.Add(TEXT("Lower texture quality"));
        Suggestions.Add(TEXT("Disable expensive post-processing effects"));
    }
    else if (CurrentMetrics.AverageFPS < TargetFPS * 0.8f)
    {
        Suggestions.Add(TEXT("FPS below target - minor optimizations recommended"));
        Suggestions.Add(TEXT("Consider reducing view distance"));
        Suggestions.Add(TEXT("Optimize LOD settings"));
    }
    
    // Memory-based suggestions
    if (CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB * 0.8f)
    {
        Suggestions.Add(TEXT("High memory usage detected"));
        Suggestions.Add(TEXT("Consider texture streaming optimizations"));
        Suggestions.Add(TEXT("Reduce texture pool size"));
    }
    
    // Draw call suggestions
    if (CurrentMetrics.DrawCalls > MaxDrawCalls)
    {
        Suggestions.Add(TEXT("High draw call count - consider mesh instancing"));
        Suggestions.Add(TEXT("Merge similar materials"));
        Suggestions.Add(TEXT("Use LOD groups for distant objects"));
    }
    
    // GPU time suggestions
    if (CurrentMetrics.GPUTime > 16.0f) // More than 16ms for 60fps
    {
        Suggestions.Add(TEXT("GPU bottleneck detected"));
        Suggestions.Add(TEXT("Reduce lighting complexity"));
        Suggestions.Add(TEXT("Optimize shaders"));
    }
    
    if (Suggestions.Num() == 0)
    {
        Suggestions.Add(TEXT("Performance is within acceptable parameters"));
    }
    
    return Suggestions;
}

bool UPerf_PerformanceAnalyzer::ShouldReduceQuality()
{
    return CurrentMetrics.AverageFPS < MinAcceptableFPS || 
           CurrentMetrics.MemoryUsageMB > MaxMemoryUsageMB ||
           CurrentMetrics.DrawCalls > MaxDrawCalls;
}

bool UPerf_PerformanceAnalyzer::ShouldIncreaseQuality()
{
    return CurrentMetrics.AverageFPS > TargetFPS * 1.2f && 
           CurrentMetrics.MemoryUsageMB < MaxMemoryUsageMB * 0.6f &&
           CurrentMetrics.DrawCalls < MaxDrawCalls * 0.7f;
}

void UPerf_PerformanceAnalyzer::UpdateFPSHistory(float CurrentFPS)
{
    FPSHistory.Add(CurrentFPS);
    
    // Keep only the last 60 samples
    if (FPSHistory.Num() > 60)
    {
        FPSHistory.RemoveAt(0);
    }
}

void UPerf_PerformanceAnalyzer::CalculateAverageMetrics()
{
    if (FPSHistory.Num() == 0)
        return;
    
    float TotalFPS = 0.0f;
    CurrentMetrics.MinFPS = FPSHistory[0];
    CurrentMetrics.MaxFPS = FPSHistory[0];
    
    for (float FPS : FPSHistory)
    {
        TotalFPS += FPS;
        CurrentMetrics.MinFPS = FMath::Min(CurrentMetrics.MinFPS, FPS);
        CurrentMetrics.MaxFPS = FMath::Max(CurrentMetrics.MaxFPS, FPS);
    }
    
    CurrentMetrics.AverageFPS = TotalFPS / FPSHistory.Num();
}

void UPerf_PerformanceAnalyzer::AnalyzeActorPerformance()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    int32 TotalActors = 0;
    int32 VisibleActors = 0;
    int32 TotalTriangles = 0;
    
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
            continue;
        
        TotalActors++;
        
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (MeshComp && MeshComp->IsVisible())
        {
            VisibleActors++;
            if (MeshComp->GetStaticMesh())
            {
                TotalTriangles += MeshComp->GetStaticMesh()->GetNumTriangles(0);
            }
        }
    }
    
    CurrentMetrics.TriangleCount = TotalTriangles;
    
    UE_LOG(LogTemp, Log, TEXT("Performance Analysis - Actors: %d, Visible: %d, Triangles: %d"), 
           TotalActors, VisibleActors, TotalTriangles);
}

float UPerf_PerformanceAnalyzer::GetMemoryUsage()
{
    // Get memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return static_cast<float>(MemStats.UsedPhysical) / (1024.0f * 1024.0f); // Convert to MB
}

int32 UPerf_PerformanceAnalyzer::GetDrawCallCount()
{
    // This is a simplified implementation
    // In a real scenario, you'd need to access render thread statistics
    return 500; // Placeholder value
}

float UPerf_PerformanceAnalyzer::GetGPUTime()
{
    // Simplified GPU time estimation
    // Real implementation would require render thread integration
    return CurrentMetrics.CurrentFPS > 0 ? (1000.0f / CurrentMetrics.CurrentFPS) * 0.7f : 0.0f;
}

float UPerf_PerformanceAnalyzer::GetCPUTime()
{
    // Simplified CPU time estimation
    return CurrentMetrics.CurrentFPS > 0 ? (1000.0f / CurrentMetrics.CurrentFPS) * 0.3f : 0.0f;
}
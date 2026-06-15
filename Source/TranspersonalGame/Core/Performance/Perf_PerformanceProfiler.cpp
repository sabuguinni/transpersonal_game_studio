#include "Perf_PerformanceProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformMemory.h"
#include "Stats/Stats.h"
#include "RenderingThread.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/LightComponent.h"
#include "Particles/ParticleSystemComponent.h"

UPerf_PerformanceProfiler::UPerf_PerformanceProfiler()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    TargetFrameRate = 60.0f;
    ProfilingUpdateRate = 0.1f;
    bAutoOptimizationEnabled = false;
    bLogPerformanceWarnings = true;
    StatsSampleSize = 100;
    
    // Default performance budgets
    MaxDrawCalls = 2000;
    MaxTriangles = 1000000;
    MaxPhysicalMemoryMB = 4096.0f;
    MaxVirtualMemoryMB = 8192.0f;
    
    bIsProfiling = false;
    ProfilingTimer = 0.0f;
    TotalFrameTime = 0.0f;
    MinFrameTime = FLT_MAX;
    MaxFrameTime = 0.0f;
    FrameCount = 0;
    PeakPhysicalMemory = 0.0f;
    PeakVirtualMemory = 0.0f;
}

void UPerf_PerformanceProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Start profiling automatically
    StartProfiling();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Profiler initialized - Target FPS: %.1f"), TargetFrameRate);
}

void UPerf_PerformanceProfiler::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsProfiling)
        return;
    
    ProfilingTimer += DeltaTime;
    
    if (ProfilingTimer >= ProfilingUpdateRate)
    {
        UpdateFrameStats(DeltaTime);
        UpdateMemoryStats();
        UpdateRenderStats();
        CheckPerformanceThresholds();
        
        if (bAutoOptimizationEnabled)
        {
            ApplyAutoOptimizations();
        }
        
        ProfilingTimer = 0.0f;
    }
}

void UPerf_PerformanceProfiler::StartProfiling()
{
    bIsProfiling = true;
    ResetStats();
    UE_LOG(LogTemp, Log, TEXT("Performance profiling started"));
}

void UPerf_PerformanceProfiler::StopProfiling()
{
    bIsProfiling = false;
    UE_LOG(LogTemp, Log, TEXT("Performance profiling stopped"));
}

void UPerf_PerformanceProfiler::ResetStats()
{
    FrameTimeHistory.Empty();
    TotalFrameTime = 0.0f;
    MinFrameTime = FLT_MAX;
    MaxFrameTime = 0.0f;
    FrameCount = 0;
    PeakPhysicalMemory = 0.0f;
    PeakVirtualMemory = 0.0f;
    
    CachedFrameStats = FPerf_FrameStats();
    CachedMemoryStats = FPerf_MemoryStats();
    CachedRenderStats = FPerf_RenderStats();
}

FPerf_FrameStats UPerf_PerformanceProfiler::GetFrameStats() const
{
    return CachedFrameStats;
}

FPerf_MemoryStats UPerf_PerformanceProfiler::GetMemoryStats() const
{
    return CachedMemoryStats;
}

FPerf_RenderStats UPerf_PerformanceProfiler::GetRenderStats() const
{
    return CachedRenderStats;
}

void UPerf_PerformanceProfiler::UpdateFrameStats(float DeltaTime)
{
    if (DeltaTime <= 0.0f)
        return;
    
    float CurrentFPS = 1.0f / DeltaTime;
    
    // Update frame time history
    FrameTimeHistory.Add(DeltaTime);
    if (FrameTimeHistory.Num() > StatsSampleSize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    TotalFrameTime += DeltaTime;
    FrameCount++;
    
    MinFrameTime = FMath::Min(MinFrameTime, DeltaTime);
    MaxFrameTime = FMath::Max(MaxFrameTime, DeltaTime);
    
    // Calculate average FPS
    float AverageFPS = FrameCount > 0 ? FrameCount / TotalFrameTime : 0.0f;
    
    // Update cached stats
    CachedFrameStats.CurrentFPS = CurrentFPS;
    CachedFrameStats.AverageFPS = AverageFPS;
    CachedFrameStats.MinFPS = MaxFrameTime > 0.0f ? 1.0f / MaxFrameTime : 0.0f;
    CachedFrameStats.MaxFPS = MinFrameTime < FLT_MAX ? 1.0f / MinFrameTime : 0.0f;
    CachedFrameStats.FrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    CachedFrameStats.GameThreadTime = DeltaTime * 1000.0f; // Simplified
    CachedFrameStats.RenderThreadTime = DeltaTime * 1000.0f; // Simplified
    CachedFrameStats.GPUTime = DeltaTime * 1000.0f; // Simplified
}

void UPerf_PerformanceProfiler::UpdateMemoryStats()
{
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    
    float UsedPhysicalMB = MemoryStats.UsedPhysical / (1024.0f * 1024.0f);
    float UsedVirtualMB = MemoryStats.UsedVirtual / (1024.0f * 1024.0f);
    
    PeakPhysicalMemory = FMath::Max(PeakPhysicalMemory, UsedPhysicalMB);
    PeakVirtualMemory = FMath::Max(PeakVirtualMemory, UsedVirtualMB);
    
    CachedMemoryStats.UsedPhysicalMemoryMB = UsedPhysicalMB;
    CachedMemoryStats.UsedVirtualMemoryMB = UsedVirtualMB;
    CachedMemoryStats.PeakUsedPhysicalMemoryMB = PeakPhysicalMemory;
    CachedMemoryStats.PeakUsedVirtualMemoryMB = PeakVirtualMemory;
    CachedMemoryStats.TexturePoolSizeMB = 512; // Estimated
    CachedMemoryStats.StreamingPoolSizeMB = 256; // Estimated
}

void UPerf_PerformanceProfiler::UpdateRenderStats()
{
    if (!GetWorld())
        return;
    
    int32 StaticMeshCount = 0;
    int32 SkeletalMeshCount = 0;
    int32 LightCount = 0;
    int32 ParticleCount = 0;
    
    for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (!Actor)
            continue;
        
        if (Actor->IsA<AStaticMeshActor>())
        {
            StaticMeshCount++;
        }
        
        if (USkeletalMeshComponent* SkelMesh = Actor->FindComponentByClass<USkeletalMeshComponent>())
        {
            SkeletalMeshCount++;
        }
        
        if (ULightComponent* Light = Actor->FindComponentByClass<ULightComponent>())
        {
            LightCount++;
        }
        
        if (UParticleSystemComponent* Particles = Actor->FindComponentByClass<UParticleSystemComponent>())
        {
            ParticleCount++;
        }
    }
    
    CachedRenderStats.StaticMeshes = StaticMeshCount;
    CachedRenderStats.SkeletalMeshes = SkeletalMeshCount;
    CachedRenderStats.Lights = LightCount;
    CachedRenderStats.Particles = ParticleCount;
    CachedRenderStats.DrawCalls = StaticMeshCount + SkeletalMeshCount + ParticleCount; // Estimated
    CachedRenderStats.Triangles = StaticMeshCount * 1000 + SkeletalMeshCount * 5000; // Estimated
    CachedRenderStats.Shadows = LightCount; // Simplified
}

void UPerf_PerformanceProfiler::CheckPerformanceThresholds()
{
    if (!bLogPerformanceWarnings)
        return;
    
    // Check frame rate
    if (CachedFrameStats.CurrentFPS < TargetFrameRate * 0.8f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: FPS below target (%.1f < %.1f)"), 
               CachedFrameStats.CurrentFPS, TargetFrameRate);
    }
    
    // Check memory usage
    if (CachedMemoryStats.UsedPhysicalMemoryMB > MaxPhysicalMemoryMB * 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: High memory usage (%.1f MB)"), 
               CachedMemoryStats.UsedPhysicalMemoryMB);
    }
    
    // Check draw calls
    if (CachedRenderStats.DrawCalls > MaxDrawCalls * 0.9f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: High draw call count (%d)"), 
               CachedRenderStats.DrawCalls);
    }
}

void UPerf_PerformanceProfiler::ApplyAutoOptimizations()
{
    // Simple auto-optimization: trigger GC if memory is high
    if (CachedMemoryStats.UsedPhysicalMemoryMB > MaxPhysicalMemoryMB * 0.8f)
    {
        TriggerGarbageCollection();
    }
}

void UPerf_PerformanceProfiler::LogPerformanceReport()
{
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Log, TEXT("FPS: Current=%.1f, Average=%.1f, Min=%.1f, Max=%.1f"), 
           CachedFrameStats.CurrentFPS, CachedFrameStats.AverageFPS, 
           CachedFrameStats.MinFPS, CachedFrameStats.MaxFPS);
    UE_LOG(LogTemp, Log, TEXT("Memory: Physical=%.1fMB, Virtual=%.1fMB"), 
           CachedMemoryStats.UsedPhysicalMemoryMB, CachedMemoryStats.UsedVirtualMemoryMB);
    UE_LOG(LogTemp, Log, TEXT("Rendering: DrawCalls=%d, Triangles=%d, StaticMeshes=%d"), 
           CachedRenderStats.DrawCalls, CachedRenderStats.Triangles, CachedRenderStats.StaticMeshes);
    UE_LOG(LogTemp, Log, TEXT("Performance Score: %.1f"), GetPerformanceScore());
}

bool UPerf_PerformanceProfiler::IsTargetFrameRateAchieved() const
{
    return CachedFrameStats.CurrentFPS >= TargetFrameRate * 0.95f;
}

void UPerf_PerformanceProfiler::SetTargetFrameRate(float NewTargetFPS)
{
    TargetFrameRate = FMath::Clamp(NewTargetFPS, 15.0f, 120.0f);
    UE_LOG(LogTemp, Log, TEXT("Target frame rate set to %.1f FPS"), TargetFrameRate);
}

float UPerf_PerformanceProfiler::GetPerformanceScore() const
{
    float FPSScore = FMath::Clamp(CachedFrameStats.CurrentFPS / TargetFrameRate, 0.0f, 1.0f);
    float MemoryScore = FMath::Clamp(1.0f - (CachedMemoryStats.UsedPhysicalMemoryMB / MaxPhysicalMemoryMB), 0.0f, 1.0f);
    float RenderScore = FMath::Clamp(1.0f - (float(CachedRenderStats.DrawCalls) / float(MaxDrawCalls)), 0.0f, 1.0f);
    
    return (FPSScore * 0.5f + MemoryScore * 0.3f + RenderScore * 0.2f) * 100.0f;
}

TArray<FString> UPerf_PerformanceProfiler::GetOptimizationSuggestions() const
{
    TArray<FString> Suggestions;
    
    if (CachedFrameStats.CurrentFPS < TargetFrameRate * 0.8f)
    {
        Suggestions.Add(TEXT("Consider reducing draw calls or mesh complexity"));
        Suggestions.Add(TEXT("Enable LOD system for distant objects"));
    }
    
    if (CachedMemoryStats.UsedPhysicalMemoryMB > MaxPhysicalMemoryMB * 0.8f)
    {
        Suggestions.Add(TEXT("Reduce texture resolution or enable streaming"));
        Suggestions.Add(TEXT("Consider garbage collection"));
    }
    
    if (CachedRenderStats.DrawCalls > MaxDrawCalls * 0.8f)
    {
        Suggestions.Add(TEXT("Batch similar objects together"));
        Suggestions.Add(TEXT("Use instanced rendering for repeated objects"));
    }
    
    if (Suggestions.Num() == 0)
    {
        Suggestions.Add(TEXT("Performance is within acceptable limits"));
    }
    
    return Suggestions;
}

void UPerf_PerformanceProfiler::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("Auto-optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PerformanceProfiler::TriggerGarbageCollection()
{
    GEngine->ForceGarbageCollection(true);
    UE_LOG(LogTemp, Log, TEXT("Garbage collection triggered"));
}

void UPerf_PerformanceProfiler::SetRenderBudget(int32 NewMaxDrawCalls, int32 NewMaxTriangles)
{
    MaxDrawCalls = FMath::Max(NewMaxDrawCalls, 100);
    MaxTriangles = FMath::Max(NewMaxTriangles, 10000);
    UE_LOG(LogTemp, Log, TEXT("Render budget set: DrawCalls=%d, Triangles=%d"), MaxDrawCalls, MaxTriangles);
}

void UPerf_PerformanceProfiler::SetMemoryBudget(float NewMaxPhysicalMemoryMB, float NewMaxVirtualMemoryMB)
{
    MaxPhysicalMemoryMB = FMath::Max(NewMaxPhysicalMemoryMB, 512.0f);
    MaxVirtualMemoryMB = FMath::Max(NewMaxVirtualMemoryMB, 1024.0f);
    UE_LOG(LogTemp, Log, TEXT("Memory budget set: Physical=%.1fMB, Virtual=%.1fMB"), 
           MaxPhysicalMemoryMB, MaxVirtualMemoryMB);
}

bool UPerf_PerformanceProfiler::IsWithinRenderBudget() const
{
    return CachedRenderStats.DrawCalls <= MaxDrawCalls && CachedRenderStats.Triangles <= MaxTriangles;
}

bool UPerf_PerformanceProfiler::IsWithinMemoryBudget() const
{
    return CachedMemoryStats.UsedPhysicalMemoryMB <= MaxPhysicalMemoryMB && 
           CachedMemoryStats.UsedVirtualMemoryMB <= MaxVirtualMemoryMB;
}
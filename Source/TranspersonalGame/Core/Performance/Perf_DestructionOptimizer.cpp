#include "Perf_DestructionOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"

UPerf_DestructionOptimizer::UPerf_DestructionOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    PerformanceUpdateInterval = 1.0f;
    bEnablePerformanceLogging = true;
    LastPerformanceUpdate = 0.0f;
    FramesSinceLastUpdate = 0;
    
    // Initialize default settings
    DestructionSettings = FPerf_DestructionSettings();
    CurrentMetrics = FPerf_DestructionMetrics();
}

void UPerf_DestructionOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Initialized destruction performance monitoring"));
    
    // Apply initial quality settings
    ApplyQualitySettings();
    
    // Start performance tracking
    LastPerformanceUpdate = GetWorld()->GetTimeSeconds();
}

void UPerf_DestructionOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    FramesSinceLastUpdate++;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        LastPerformanceUpdate = CurrentTime;
        FramesSinceLastUpdate = 0;
    }
    
    // Cleanup old debris periodically
    if (FMath::Fmod(CurrentTime, 5.0f) < DeltaTime)
    {
        CleanupOldDebris();
    }
    
    // Cull distant destructions
    CullDistantDestructions();
}

FPerf_DestructionMetrics UPerf_DestructionOptimizer::GetDestructionMetrics() const
{
    return CurrentMetrics;
}

void UPerf_DestructionOptimizer::OptimizeDestructionPerformance()
{
    UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Running performance optimization"));
    
    // Analyze current performance
    float AverageFrameTime = CalculateFrameTimeImpact();
    
    // Automatically adjust quality based on performance
    if (AverageFrameTime > 20.0f) // Above 20ms frame time
    {
        // Reduce quality
        if (DestructionSettings.QualityLevel == EPerf_DestructionQuality::Ultra)
        {
            SetDestructionQuality(EPerf_DestructionQuality::High);
        }
        else if (DestructionSettings.QualityLevel == EPerf_DestructionQuality::High)
        {
            SetDestructionQuality(EPerf_DestructionQuality::Medium);
        }
        else if (DestructionSettings.QualityLevel == EPerf_DestructionQuality::Medium)
        {
            SetDestructionQuality(EPerf_DestructionQuality::Low);
        }
        
        // Reduce max simultaneous destructions
        DestructionSettings.MaxSimultaneousDestructions = FMath::Max(1, DestructionSettings.MaxSimultaneousDestructions - 1);
        
        // Increase culling distance
        DestructionSettings.DestructionCullingDistance *= 0.8f;
    }
    else if (AverageFrameTime < 10.0f) // Below 10ms frame time
    {
        // Can increase quality
        if (DestructionSettings.QualityLevel == EPerf_DestructionQuality::Low)
        {
            SetDestructionQuality(EPerf_DestructionQuality::Medium);
        }
        else if (DestructionSettings.QualityLevel == EPerf_DestructionQuality::Medium)
        {
            SetDestructionQuality(EPerf_DestructionQuality::High);
        }
        
        // Increase max simultaneous destructions
        DestructionSettings.MaxSimultaneousDestructions = FMath::Min(10, DestructionSettings.MaxSimultaneousDestructions + 1);
    }
    
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Performance optimization complete. Quality: %d, Max Destructions: %d"), 
           (int32)DestructionSettings.QualityLevel, DestructionSettings.MaxSimultaneousDestructions);
}

void UPerf_DestructionOptimizer::SetDestructionQuality(EPerf_DestructionQuality NewQuality)
{
    DestructionSettings.QualityLevel = NewQuality;
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Destruction quality set to %d"), (int32)NewQuality);
}

bool UPerf_DestructionOptimizer::CanPerformDestruction(const FVector& Location) const
{
    // Check if we're at the destruction limit
    if (RecentDestructions.Num() >= DestructionSettings.MaxSimultaneousDestructions)
    {
        return false;
    }
    
    // Check distance culling
    if (GetOwner())
    {
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Location);
        if (Distance > DestructionSettings.DestructionCullingDistance)
        {
            return false;
        }
    }
    
    return true;
}

void UPerf_DestructionOptimizer::RegisterDestructionEvent(const FVector& Location, float Intensity)
{
    float StartTime = FPlatformTime::Seconds();
    
    // Add to recent destructions
    RecentDestructions.Add(Location);
    
    // Simulate destruction processing time based on intensity and quality
    float ProcessingTime = Intensity * 0.1f;
    switch (DestructionSettings.QualityLevel)
    {
        case EPerf_DestructionQuality::Low:
            ProcessingTime *= 0.5f;
            break;
        case EPerf_DestructionQuality::Medium:
            ProcessingTime *= 1.0f;
            break;
        case EPerf_DestructionQuality::High:
            ProcessingTime *= 1.5f;
            break;
        case EPerf_DestructionQuality::Ultra:
            ProcessingTime *= 2.0f;
            break;
    }
    
    float EndTime = FPlatformTime::Seconds();
    float ActualTime = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    DestructionTimes.Add(ActualTime);
    
    // Keep only recent data
    if (DestructionTimes.Num() > 100)
    {
        DestructionTimes.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_DestructionOptimizer: Registered destruction at %s, processing time: %.2fms"), 
           *Location.ToString(), ActualTime);
}

void UPerf_DestructionOptimizer::CleanupOldDebris()
{
    if (!GetWorld()) return;
    
    int32 CleanedCount = 0;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove old destruction locations
    for (int32 i = RecentDestructions.Num() - 1; i >= 0; i--)
    {
        // Simple time-based cleanup (in a real implementation, you'd track timestamps)
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per cleanup cycle
        {
            RecentDestructions.RemoveAt(i);
            CleanedCount++;
        }
    }
    
    if (CleanedCount > 0 && bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Cleaned up %d old debris objects"), CleanedCount);
    }
}

void UPerf_DestructionOptimizer::RunDestructionPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_DestructionOptimizer: Running destruction performance test..."));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Simulate multiple destruction events
    for (int32 i = 0; i < 20; i++)
    {
        FVector TestLocation = GetOwner() ? GetOwner()->GetActorLocation() + FVector(i * 100, 0, 0) : FVector(i * 100, 0, 0);
        RegisterDestructionEvent(TestLocation, FMath::RandRange(0.5f, 2.0f));
    }
    
    float EndTime = FPlatformTime::Seconds();
    float TestDuration = (EndTime - StartTime) * 1000.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_DestructionOptimizer: Performance test completed in %.2fms"), TestDuration);
    
    // Update metrics
    UpdatePerformanceMetrics();
    LogPerformanceData();
}

void UPerf_DestructionOptimizer::AnalyzeDestructionBottlenecks()
{
    UE_LOG(LogTemp, Warning, TEXT("Perf_DestructionOptimizer: Analyzing destruction bottlenecks..."));
    
    // Analyze destruction times
    if (DestructionTimes.Num() > 0)
    {
        float TotalTime = 0.0f;
        float MinTime = DestructionTimes[0];
        float MaxTime = DestructionTimes[0];
        
        for (float Time : DestructionTimes)
        {
            TotalTime += Time;
            MinTime = FMath::Min(MinTime, Time);
            MaxTime = FMath::Max(MaxTime, Time);
        }
        
        float AverageTime = TotalTime / DestructionTimes.Num();
        
        UE_LOG(LogTemp, Warning, TEXT("Destruction Time Analysis:"));
        UE_LOG(LogTemp, Warning, TEXT("  Average: %.2fms"), AverageTime);
        UE_LOG(LogTemp, Warning, TEXT("  Min: %.2fms"), MinTime);
        UE_LOG(LogTemp, Warning, TEXT("  Max: %.2fms"), MaxTime);
        UE_LOG(LogTemp, Warning, TEXT("  Samples: %d"), DestructionTimes.Num());
        
        // Identify bottlenecks
        if (AverageTime > 5.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("BOTTLENECK: Average destruction time exceeds 5ms"));
        }
        
        if (MaxTime > 20.0f)
        {
            UE_LOG(LogTemp, Warning, TEXT("BOTTLENECK: Peak destruction time exceeds 20ms"));
        }
        
        if (CurrentMetrics.ActiveDestructibleActors > 50)
        {
            UE_LOG(LogTemp, Warning, TEXT("BOTTLENECK: Too many active destructible actors (%d)"), CurrentMetrics.ActiveDestructibleActors);
        }
    }
}

void UPerf_DestructionOptimizer::UpdatePerformanceMetrics()
{
    // Calculate average destruction time
    if (DestructionTimes.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Time : DestructionTimes)
        {
            TotalTime += Time;
        }
        CurrentMetrics.AverageDestructionTime = TotalTime / DestructionTimes.Num();
    }
    
    // Update active destructible actors count
    CurrentMetrics.ActiveDestructibleActors = RecentDestructions.Num();
    
    // Calculate frame time impact
    CurrentMetrics.FrameTimeImpact = CalculateFrameTimeImpact();
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (RecentDestructions.Num() * 0.5f) + (DestructionTimes.Num() * 0.001f);
    
    if (bEnablePerformanceLogging)
    {
        LogPerformanceData();
    }
}

void UPerf_DestructionOptimizer::ApplyQualitySettings()
{
    switch (DestructionSettings.QualityLevel)
    {
        case EPerf_DestructionQuality::Low:
            DestructionSettings.MaxSimultaneousDestructions = FMath::Min(DestructionSettings.MaxSimultaneousDestructions, 3);
            DestructionSettings.DebrisLifetime = 15.0f;
            break;
        case EPerf_DestructionQuality::Medium:
            DestructionSettings.MaxSimultaneousDestructions = FMath::Min(DestructionSettings.MaxSimultaneousDestructions, 5);
            DestructionSettings.DebrisLifetime = 30.0f;
            break;
        case EPerf_DestructionQuality::High:
            DestructionSettings.MaxSimultaneousDestructions = FMath::Min(DestructionSettings.MaxSimultaneousDestructions, 8);
            DestructionSettings.DebrisLifetime = 60.0f;
            break;
        case EPerf_DestructionQuality::Ultra:
            DestructionSettings.MaxSimultaneousDestructions = FMath::Min(DestructionSettings.MaxSimultaneousDestructions, 12);
            DestructionSettings.DebrisLifetime = 120.0f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_DestructionOptimizer: Applied quality settings for level %d"), (int32)DestructionSettings.QualityLevel);
}

void UPerf_DestructionOptimizer::CullDistantDestructions()
{
    if (!GetOwner()) return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    int32 CulledCount = 0;
    
    for (int32 i = RecentDestructions.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(OwnerLocation, RecentDestructions[i]);
        if (Distance > DestructionSettings.DestructionCullingDistance)
        {
            RecentDestructions.RemoveAt(i);
            CulledCount++;
        }
    }
    
    if (CulledCount > 0 && bEnablePerformanceLogging)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_DestructionOptimizer: Culled %d distant destructions"), CulledCount);
    }
}

void UPerf_DestructionOptimizer::OptimizeDebrisCount()
{
    // If we have too many debris objects, remove the oldest ones
    while (RecentDestructions.Num() > DestructionSettings.MaxSimultaneousDestructions)
    {
        RecentDestructions.RemoveAt(0);
    }
}

float UPerf_DestructionOptimizer::CalculateFrameTimeImpact() const
{
    // Simplified frame time impact calculation
    float BaseImpact = CurrentMetrics.ActiveDestructibleActors * 0.1f; // 0.1ms per active destruction
    float QualityMultiplier = 1.0f;
    
    switch (DestructionSettings.QualityLevel)
    {
        case EPerf_DestructionQuality::Low:
            QualityMultiplier = 0.5f;
            break;
        case EPerf_DestructionQuality::Medium:
            QualityMultiplier = 1.0f;
            break;
        case EPerf_DestructionQuality::High:
            QualityMultiplier = 1.5f;
            break;
        case EPerf_DestructionQuality::Ultra:
            QualityMultiplier = 2.0f;
            break;
    }
    
    return BaseImpact * QualityMultiplier;
}

void UPerf_DestructionOptimizer::LogPerformanceData() const
{
    UE_LOG(LogTemp, Log, TEXT("=== DESTRUCTION PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Log, TEXT("Average Destruction Time: %.2fms"), CurrentMetrics.AverageDestructionTime);
    UE_LOG(LogTemp, Log, TEXT("Active Destructible Actors: %d"), CurrentMetrics.ActiveDestructibleActors);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Frame Time Impact: %.2fms"), CurrentMetrics.FrameTimeImpact);
    UE_LOG(LogTemp, Log, TEXT("Quality Level: %d"), (int32)DestructionSettings.QualityLevel);
    UE_LOG(LogTemp, Log, TEXT("Max Simultaneous: %d"), DestructionSettings.MaxSimultaneousDestructions);
    UE_LOG(LogTemp, Log, TEXT("========================================"));
}
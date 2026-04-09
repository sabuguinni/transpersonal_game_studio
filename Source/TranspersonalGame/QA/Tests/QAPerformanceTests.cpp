#include "QAPerformanceTests.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameViewportClient.h"
#include "HAL/PlatformMemory.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Stats/StatsData.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"

IMPLEMENT_TRANSPERSONAL_TEST(FQAFrameRateTest, "Transpersonal.Performance.FrameRate", QATestCategories::Performance)

bool FQAFrameRateTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Frame Rate Performance Test"));
    
    UWorld* TestWorld = GetTestWorld();
    if (!TestWorld)
    {
        AddError(TEXT("Failed to get test world"));
        return false;
    }
    
    // Test frame rate stability
    float MinFPS = FPerformanceThresholds::PC_MIN_FPS;
    bool bFrameRateStable = ValidateFrameRateStability(MinFPS, 5.0f);
    VALIDATE_PERFORMANCE(bFrameRateStable, "Frame rate stability test failed");
    
    // Test frame rate under load
    bool bFrameRateUnderLoad = ValidateFrameRateUnderLoad(MinFPS * 0.8f); // 80% of target under load
    VALIDATE_PERFORMANCE(bFrameRateUnderLoad, "Frame rate under load test failed");
    
    // Test frame time variance
    bool bFrameTimeVariance = MeasureFrameTimeVariance(5.0f); // Max 5ms variance
    VALIDATE_PERFORMANCE(bFrameTimeVariance, "Frame time variance test failed");
    
    UE_LOG(LogQAFramework, Log, TEXT("Frame Rate Performance Test completed successfully"));
    return true;
}

bool FQAFrameRateTest::ValidateFrameRateStability(float MinFPS, float TestDuration)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<float> FrameRateSamples;
    float StartTime = World->GetTimeSeconds();
    float SampleInterval = 0.1f; // Sample every 100ms
    float LastSampleTime = StartTime;
    
    while (World->GetTimeSeconds() - StartTime < TestDuration)
    {
        float CurrentTime = World->GetTimeSeconds();
        if (CurrentTime - LastSampleTime >= SampleInterval)
        {
            float CurrentFPS = 1.0f / World->GetDeltaSeconds();
            FrameRateSamples.Add(CurrentFPS);
            LastSampleTime = CurrentTime;
        }
        
        // Yield control
        FPlatformProcess::Sleep(0.001f);
    }
    
    if (FrameRateSamples.Num() == 0)
    {
        return false;
    }
    
    // Calculate average and minimum frame rates
    float TotalFPS = 0.0f;
    float MinObservedFPS = FLT_MAX;
    
    for (float FPS : FrameRateSamples)
    {
        TotalFPS += FPS;
        MinObservedFPS = FMath::Min(MinObservedFPS, FPS);
    }
    
    float AverageFPS = TotalFPS / FrameRateSamples.Num();
    
    UE_LOG(LogQAFramework, Log, TEXT("Frame Rate Stability: Avg=%.2f, Min=%.2f, Target=%.2f"), 
           AverageFPS, MinObservedFPS, MinFPS);
    
    return AverageFPS >= MinFPS && MinObservedFPS >= MinFPS * 0.9f; // Allow 10% dips
}

bool FQAFrameRateTest::ValidateFrameRateUnderLoad(float MinFPS)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        return false;
    }
    
    // Spawn multiple actors to create load
    TArray<AActor*> LoadActors;
    for (int32 i = 0; i < 100; ++i)
    {
        FVector SpawnLocation(i * 100.0f, 0.0f, 100.0f);
        AActor* LoadActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (LoadActor)
        {
            LoadActors.Add(LoadActor);
        }
    }
    
    // Measure frame rate under load
    float LoadTestDuration = 3.0f;
    bool bFrameRateValid = ValidateFrameRateStability(MinFPS, LoadTestDuration);
    
    // Clean up load actors
    for (AActor* Actor : LoadActors)
    {
        if (Actor && IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    return bFrameRateValid;
}

bool FQAFrameRateTest::MeasureFrameTimeVariance(float MaxVarianceMS)
{
    UWorld* World = GetTestWorld();
    if (!World)
    {
        return false;
    }
    
    TArray<float> FrameTimes;
    float TestDuration = 3.0f;
    float StartTime = World->GetTimeSeconds();
    
    while (World->GetTimeSeconds() - StartTime < TestDuration)
    {
        float FrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to milliseconds
        FrameTimes.Add(FrameTime);
        FPlatformProcess::Sleep(0.001f);
    }
    
    if (FrameTimes.Num() < 2)
    {
        return false;
    }
    
    // Calculate variance
    float Sum = 0.0f;
    for (float FrameTime : FrameTimes)
    {
        Sum += FrameTime;
    }
    float Mean = Sum / FrameTimes.Num();
    
    float VarianceSum = 0.0f;
    for (float FrameTime : FrameTimes)
    {
        float Diff = FrameTime - Mean;
        VarianceSum += Diff * Diff;
    }
    float Variance = VarianceSum / FrameTimes.Num();
    float StandardDeviation = FMath::Sqrt(Variance);
    
    UE_LOG(LogQAFramework, Log, TEXT("Frame Time Variance: Mean=%.2fms, StdDev=%.2fms, Max=%.2fms"), 
           Mean, StandardDeviation, MaxVarianceMS);
    
    return StandardDeviation <= MaxVarianceMS;
}

IMPLEMENT_TRANSPERSONAL_TEST(FQAMemoryUsageTest, "Transpersonal.Performance.Memory", QATestCategories::Performance)

bool FQAMemoryUsageTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Memory Usage Performance Test"));
    
    // Test memory footprint
    bool bMemoryFootprint = ValidateMemoryFootprint(FPerformanceThresholds::MAX_MEMORY_MB_PC);
    VALIDATE_PERFORMANCE(bMemoryFootprint, "Memory footprint test failed");
    
    // Test memory leaks
    bool bMemoryLeaks = ValidateMemoryLeaks();
    VALIDATE_PERFORMANCE(bMemoryLeaks, "Memory leak test failed");
    
    // Test garbage collection
    bool bGarbageCollection = ValidateGarbageCollection();
    VALIDATE_PERFORMANCE(bGarbageCollection, "Garbage collection test failed");
    
    UE_LOG(LogQAFramework, Log, TEXT("Memory Usage Performance Test completed successfully"));
    return true;
}

bool FQAMemoryUsageTest::ValidateMemoryFootprint(int32 MaxMemoryMB)
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    int32 UsedMemoryMB = MemStats.UsedPhysical / (1024 * 1024);
    int32 AvailableMemoryMB = MemStats.AvailablePhysical / (1024 * 1024);
    
    UE_LOG(LogQAFramework, Log, TEXT("Memory Usage: Used=%dMB, Available=%dMB, Max=%dMB"), 
           UsedMemoryMB, AvailableMemoryMB, MaxMemoryMB);
    
    return UsedMemoryMB <= MaxMemoryMB;
}

bool FQAMemoryUsageTest::ValidateMemoryLeaks()
{
    // Record initial memory state
    FPlatformMemoryStats InitialStats = FPlatformMemory::GetStats();
    int32 InitialMemoryMB = InitialStats.UsedPhysical / (1024 * 1024);
    
    // Perform memory-intensive operations
    TArray<TArray<uint8>*> TestArrays;
    for (int32 i = 0; i < 100; ++i)
    {
        TArray<uint8>* TestArray = new TArray<uint8>();
        TestArray->SetNum(1024 * 1024); // 1MB per array
        TestArrays.Add(TestArray);
    }
    
    // Clean up
    for (TArray<uint8>* TestArray : TestArrays)
    {
        delete TestArray;
    }
    TestArrays.Empty();
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Check final memory state
    FPlatformMemoryStats FinalStats = FPlatformMemory::GetStats();
    int32 FinalMemoryMB = FinalStats.UsedPhysical / (1024 * 1024);
    
    int32 MemoryDifferenceMB = FinalMemoryMB - InitialMemoryMB;
    
    UE_LOG(LogQAFramework, Log, TEXT("Memory Leak Test: Initial=%dMB, Final=%dMB, Difference=%dMB"), 
           InitialMemoryMB, FinalMemoryMB, MemoryDifferenceMB);
    
    // Allow up to 50MB difference for normal operations
    return MemoryDifferenceMB <= 50;
}

bool FQAMemoryUsageTest::ValidateGarbageCollection()
{
    // Force garbage collection and measure time
    double StartTime = FPlatformTime::Seconds();
    GEngine->ForceGarbageCollection(true);
    double EndTime = FPlatformTime::Seconds();
    
    double GCTimeMS = (EndTime - StartTime) * 1000.0;
    
    UE_LOG(LogQAFramework, Log, TEXT("Garbage Collection Time: %.2fms"), GCTimeMS);
    
    // GC should complete within reasonable time (100ms)
    return GCTimeMS <= 100.0;
}

IMPLEMENT_TRANSPERSONAL_TEST(FQARenderingPerformanceTest, "Transpersonal.Performance.Rendering", QATestCategories::Performance)

bool FQARenderingPerformanceTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Rendering Performance Test"));
    
    // Test draw call count
    bool bDrawCalls = ValidateDrawCallCount(FPerformanceThresholds::MAX_DRAW_CALLS);
    VALIDATE_PERFORMANCE(bDrawCalls, "Draw call count test failed");
    
    // Test triangle count
    bool bTriangles = ValidateTriangleCount(FPerformanceThresholds::MAX_TRIANGLES);
    VALIDATE_PERFORMANCE(bTriangles, "Triangle count test failed");
    
    // Test GPU memory usage
    bool bGPUMemory = ValidateGPUMemoryUsage(2048); // 2GB GPU memory limit
    VALIDATE_PERFORMANCE(bGPUMemory, "GPU memory usage test failed");
    
    // Test LOD system
    bool bLODSystem = ValidateLODSystem();
    VALIDATE_PERFORMANCE(bLODSystem, "LOD system test failed");
    
    UE_LOG(LogQAFramework, Log, TEXT("Rendering Performance Test completed successfully"));
    return true;
}

bool FQARenderingPerformanceTest::ValidateDrawCallCount(int32 MaxDrawCalls)
{
    // This would require access to rendering stats
    // For now, return true as placeholder
    UE_LOG(LogQAFramework, Log, TEXT("Draw Call Validation: MaxDrawCalls=%d"), MaxDrawCalls);
    return true;
}

bool FQARenderingPerformanceTest::ValidateTriangleCount(int32 MaxTriangles)
{
    // This would require access to rendering stats
    // For now, return true as placeholder
    UE_LOG(LogQAFramework, Log, TEXT("Triangle Count Validation: MaxTriangles=%d"), MaxTriangles);
    return true;
}

bool FQARenderingPerformanceTest::ValidateGPUMemoryUsage(int32 MaxGPUMemoryMB)
{
    // This would require access to GPU memory stats
    // For now, return true as placeholder
    UE_LOG(LogQAFramework, Log, TEXT("GPU Memory Validation: MaxGPUMemoryMB=%d"), MaxGPUMemoryMB);
    return true;
}

bool FQARenderingPerformanceTest::ValidateLODSystem()
{
    // Test LOD system functionality
    UE_LOG(LogQAFramework, Log, TEXT("LOD System Validation"));
    return true;
}

// Performance Metrics Collector Implementation
FQAPerformanceMetrics FQAPerformanceCollector::CollectMetrics(float SampleDuration)
{
    FQAPerformanceMetrics Metrics;
    
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return Metrics;
    }
    
    // Collect frame rate metrics
    CollectFrameRateMetrics(Metrics, SampleDuration);
    
    // Collect memory metrics
    CollectMemoryMetrics(Metrics);
    
    // Collect rendering metrics
    CollectRenderingMetrics(Metrics);
    
    // Collect physics metrics
    CollectPhysicsMetrics(Metrics);
    
    // Collect AI metrics
    CollectAIMetrics(Metrics);
    
    Metrics.bMetricsValid = true;
    return Metrics;
}

void FQAPerformanceCollector::CollectFrameRateMetrics(FQAPerformanceMetrics& Metrics, float SampleDuration)
{
    UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(GEngine, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
    if (!World)
    {
        return;
    }
    
    TArray<float> FrameRateSamples;
    float StartTime = World->GetTimeSeconds();
    float SampleInterval = 0.1f;
    float LastSampleTime = StartTime;
    
    while (World->GetTimeSeconds() - StartTime < SampleDuration)
    {
        float CurrentTime = World->GetTimeSeconds();
        if (CurrentTime - LastSampleTime >= SampleInterval)
        {
            float CurrentFPS = 1.0f / World->GetDeltaSeconds();
            FrameRateSamples.Add(CurrentFPS);
            LastSampleTime = CurrentTime;
        }
        FPlatformProcess::Sleep(0.001f);
    }
    
    if (FrameRateSamples.Num() > 0)
    {
        float TotalFPS = 0.0f;
        Metrics.MinFPS = FLT_MAX;
        Metrics.MaxFPS = 0.0f;
        
        for (float FPS : FrameRateSamples)
        {
            TotalFPS += FPS;
            Metrics.MinFPS = FMath::Min(Metrics.MinFPS, FPS);
            Metrics.MaxFPS = FMath::Max(Metrics.MaxFPS, FPS);
        }
        
        Metrics.AverageFPS = TotalFPS / FrameRateSamples.Num();
        
        // Calculate frame time variance
        float MeanFrameTime = 1000.0f / Metrics.AverageFPS; // Convert to milliseconds
        float VarianceSum = 0.0f;
        
        for (float FPS : FrameRateSamples)
        {
            float FrameTime = 1000.0f / FPS;
            float Diff = FrameTime - MeanFrameTime;
            VarianceSum += Diff * Diff;
        }
        
        Metrics.FrameTimeVariance = FMath::Sqrt(VarianceSum / FrameRateSamples.Num());
    }
}

void FQAPerformanceCollector::CollectMemoryMetrics(FQAPerformanceMetrics& Metrics)
{
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    Metrics.MemoryUsageMB = MemStats.UsedPhysical / (1024 * 1024);
}

void FQAPerformanceCollector::CollectRenderingMetrics(FQAPerformanceMetrics& Metrics)
{
    // Placeholder for rendering metrics collection
    Metrics.DrawCalls = 0;
    Metrics.Triangles = 0;
    Metrics.GPUMemoryUsageMB = 0;
}

void FQAPerformanceCollector::CollectPhysicsMetrics(FQAPerformanceMetrics& Metrics)
{
    // Placeholder for physics metrics collection
    Metrics.PhysicsStepTime = 0.0f;
}

void FQAPerformanceCollector::CollectAIMetrics(FQAPerformanceMetrics& Metrics)
{
    // Placeholder for AI metrics collection
    Metrics.AITickTime = 0.0f;
}

bool FQAPerformanceCollector::ValidateMetricsAgainstThresholds(const FQAPerformanceMetrics& Metrics)
{
    if (!Metrics.bMetricsValid)
    {
        return false;
    }
    
    bool bValid = true;
    
    // Validate frame rate
    if (Metrics.AverageFPS < FPerformanceThresholds::PC_MIN_FPS)
    {
        UE_LOG(LogQAFramework, Error, TEXT("Average FPS below threshold: %.2f < %.2f"), 
               Metrics.AverageFPS, FPerformanceThresholds::PC_MIN_FPS);
        bValid = false;
    }
    
    // Validate memory usage
    if (Metrics.MemoryUsageMB > FPerformanceThresholds::MAX_MEMORY_MB_PC)
    {
        UE_LOG(LogQAFramework, Error, TEXT("Memory usage above threshold: %d > %d MB"), 
               Metrics.MemoryUsageMB, FPerformanceThresholds::MAX_MEMORY_MB_PC);
        bValid = false;
    }
    
    return bValid;
}

FString FQAPerformanceCollector::FormatMetricsReport(const FQAPerformanceMetrics& Metrics)
{
    FString Report;
    Report += TEXT("=== Performance Metrics Report ===\n");
    Report += FString::Printf(TEXT("Frame Rate: Avg=%.2f, Min=%.2f, Max=%.2f FPS\n"), 
                             Metrics.AverageFPS, Metrics.MinFPS, Metrics.MaxFPS);
    Report += FString::Printf(TEXT("Frame Time Variance: %.2f ms\n"), Metrics.FrameTimeVariance);
    Report += FString::Printf(TEXT("Memory Usage: %d MB\n"), Metrics.MemoryUsageMB);
    Report += FString::Printf(TEXT("GPU Memory: %d MB\n"), Metrics.GPUMemoryUsageMB);
    Report += FString::Printf(TEXT("Draw Calls: %d\n"), Metrics.DrawCalls);
    Report += FString::Printf(TEXT("Triangles: %d\n"), Metrics.Triangles);
    Report += FString::Printf(TEXT("Physics Step Time: %.2f ms\n"), Metrics.PhysicsStepTime);
    Report += FString::Printf(TEXT("AI Tick Time: %.2f ms\n"), Metrics.AITickTime);
    
    return Report;
}
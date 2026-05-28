#include "Perf_PerformanceTestSuite.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Stats/Stats.h"
#include "Engine/GameViewportClient.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "AIController.h"
#include "Engine/LevelStreaming.h"

UPerf_PerformanceTestSuite::UPerf_PerformanceTestSuite()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Default test thresholds
    TargetFrameRate = 60.0f;
    MaxMemoryUsageMB = 4096.0f;
    MaxPhysicsObjects = 1000;
    bAutoRunOnBeginPlay = false;
}

void UPerf_PerformanceTestSuite::BeginPlay()
{
    Super::BeginPlay();
    
    if (bAutoRunOnBeginPlay)
    {
        // Delay test execution to allow world to fully load
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UPerf_PerformanceTestSuite::RunAllTests, 2.0f, false);
    }
}

void UPerf_PerformanceTestSuite::RunAllTests()
{
    UE_LOG(LogTemp, Warning, TEXT("Performance Test Suite: Starting comprehensive performance tests..."));
    
    TestResults.Empty();
    
    // Run all test categories
    TestResults.Add(RunFrameRateTest());
    TestResults.Add(RunMemoryTest());
    TestResults.Add(RunPhysicsTest());
    TestResults.Add(RunRenderingTest());
    TestResults.Add(RunAITest());
    TestResults.Add(RunStreamingTest());
    
    // Log summary
    float OverallScore = GetOverallScore();
    bool bAllPassed = AllTestsPassed();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Test Suite: Complete. Overall Score: %.2f, All Tests Passed: %s"), 
           OverallScore, bAllPassed ? TEXT("YES") : TEXT("NO"));
    
    // Output detailed results
    for (const FPerf_TestResult& Result : TestResults)
    {
        LogTestResult(Result);
    }
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunFrameRateTest(float Duration)
{
    UE_LOG(LogTemp, Log, TEXT("Running Frame Rate Test for %.1f seconds..."), Duration);
    
    float AverageFrameRate = MeasureAverageFrameRate(Duration);
    bool bPassed = AverageFrameRate >= TargetFrameRate * 0.9f; // 90% of target
    
    FString Details = FString::Printf(TEXT("Average FPS: %.1f, Target: %.1f"), AverageFrameRate, TargetFrameRate);
    
    return CreateTestResult(EPerf_TestType::FrameRate, AverageFrameRate, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunMemoryTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running Memory Usage Test..."));
    
    float MemoryUsageMB = GetCurrentMemoryUsageMB();
    bool bPassed = MemoryUsageMB <= MaxMemoryUsageMB;
    
    FString Details = FString::Printf(TEXT("Memory Usage: %.1f MB, Max: %.1f MB"), MemoryUsageMB, MaxMemoryUsageMB);
    
    float Score = FMath::Max(0.0f, 100.0f - (MemoryUsageMB / MaxMemoryUsageMB * 100.0f));
    
    return CreateTestResult(EPerf_TestType::Memory, Score, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunPhysicsTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running Physics Performance Test..."));
    
    int32 PhysicsObjectCount = CountPhysicsObjects();
    bool bPassed = PhysicsObjectCount <= MaxPhysicsObjects;
    
    FString Details = FString::Printf(TEXT("Physics Objects: %d, Max: %d"), PhysicsObjectCount, MaxPhysicsObjects);
    
    float Score = FMath::Max(0.0f, 100.0f - (float(PhysicsObjectCount) / float(MaxPhysicsObjects) * 100.0f));
    
    return CreateTestResult(EPerf_TestType::Physics, Score, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunRenderingTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running Rendering Performance Test..."));
    
    float RenderTime = MeasureRenderTime();
    bool bPassed = RenderTime <= 16.67f; // 60 FPS threshold
    
    FString Details = FString::Printf(TEXT("Render Time: %.2f ms, Target: <=16.67 ms"), RenderTime);
    
    float Score = FMath::Max(0.0f, 100.0f - (RenderTime / 16.67f * 100.0f));
    
    return CreateTestResult(EPerf_TestType::Rendering, Score, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunAITest()
{
    UE_LOG(LogTemp, Log, TEXT("Running AI Performance Test..."));
    
    float AIProcessingTime = MeasureAIProcessingTime();
    bool bPassed = AIProcessingTime <= 5.0f; // 5ms threshold
    
    FString Details = FString::Printf(TEXT("AI Processing: %.2f ms, Target: <=5.0 ms"), AIProcessingTime);
    
    float Score = FMath::Max(0.0f, 100.0f - (AIProcessingTime / 5.0f * 100.0f));
    
    return CreateTestResult(EPerf_TestType::AI, Score, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::RunStreamingTest()
{
    UE_LOG(LogTemp, Log, TEXT("Running Streaming Performance Test..."));
    
    float StreamingPerf = MeasureStreamingPerformance();
    bool bPassed = StreamingPerf >= 50.0f; // Arbitrary good score
    
    FString Details = FString::Printf(TEXT("Streaming Score: %.1f, Target: >=50.0"), StreamingPerf);
    
    return CreateTestResult(EPerf_TestType::Streaming, StreamingPerf, bPassed, Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::GetTestResult(EPerf_TestType TestType) const
{
    for (const FPerf_TestResult& Result : TestResults)
    {
        if (Result.TestType == TestType)
        {
            return Result;
        }
    }
    
    // Return empty result if not found
    return FPerf_TestResult();
}

bool UPerf_PerformanceTestSuite::AllTestsPassed() const
{
    for (const FPerf_TestResult& Result : TestResults)
    {
        if (!Result.bPassed)
        {
            return false;
        }
    }
    return TestResults.Num() > 0;
}

float UPerf_PerformanceTestSuite::GetOverallScore() const
{
    if (TestResults.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalScore = 0.0f;
    for (const FPerf_TestResult& Result : TestResults)
    {
        TotalScore += Result.Score;
    }
    
    return TotalScore / float(TestResults.Num());
}

float UPerf_PerformanceTestSuite::MeasureAverageFrameRate(float Duration)
{
    if (!GEngine || !GetWorld())
    {
        return 0.0f;
    }
    
    // Simple frame rate measurement
    float StartTime = GetWorld()->GetTimeSeconds();
    int32 FrameCount = 0;
    
    while (GetWorld()->GetTimeSeconds() - StartTime < Duration)
    {
        FrameCount++;
        // This is a simplified approach - in reality you'd need to properly measure frame times
    }
    
    return float(FrameCount) / Duration;
}

float UPerf_PerformanceTestSuite::GetCurrentMemoryUsageMB()
{
    // Get memory stats from engine
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    return float(MemStats.UsedPhysical) / (1024.0f * 1024.0f);
}

int32 UPerf_PerformanceTestSuite::CountPhysicsObjects()
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

float UPerf_PerformanceTestSuite::MeasureRenderTime()
{
    // Simplified render time measurement
    // In a real implementation, you'd use GPU timing queries
    return FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
}

float UPerf_PerformanceTestSuite::MeasureAIProcessingTime()
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    // Count AI controllers and estimate processing time
    int32 AICount = 0;
    for (TActorIterator<AAIController> AIItr(GetWorld()); AIItr; ++AIItr)
    {
        AICount++;
    }
    
    // Simplified estimation: 0.1ms per AI controller
    return float(AICount) * 0.1f;
}

float UPerf_PerformanceTestSuite::MeasureStreamingPerformance()
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    // Check level streaming status
    const TArray<ULevelStreaming*>& StreamingLevels = GetWorld()->GetStreamingLevels();
    int32 LoadedLevels = 0;
    int32 TotalLevels = StreamingLevels.Num();
    
    for (ULevelStreaming* StreamingLevel : StreamingLevels)
    {
        if (StreamingLevel && StreamingLevel->IsLevelLoaded())
        {
            LoadedLevels++;
        }
    }
    
    // Return percentage of loaded levels as performance metric
    return TotalLevels > 0 ? (float(LoadedLevels) / float(TotalLevels) * 100.0f) : 100.0f;
}

void UPerf_PerformanceTestSuite::LogTestResult(const FPerf_TestResult& Result)
{
    FString TestTypeName;
    switch (Result.TestType)
    {
        case EPerf_TestType::FrameRate: TestTypeName = TEXT("FrameRate"); break;
        case EPerf_TestType::Memory: TestTypeName = TEXT("Memory"); break;
        case EPerf_TestType::Physics: TestTypeName = TEXT("Physics"); break;
        case EPerf_TestType::Rendering: TestTypeName = TEXT("Rendering"); break;
        case EPerf_TestType::AI: TestTypeName = TEXT("AI"); break;
        case EPerf_TestType::Streaming: TestTypeName = TEXT("Streaming"); break;
        default: TestTypeName = TEXT("Unknown"); break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Test %s: Score=%.1f, Passed=%s, Details=%s"), 
           *TestTypeName, Result.Score, Result.bPassed ? TEXT("YES") : TEXT("NO"), *Result.Details);
}

FPerf_TestResult UPerf_PerformanceTestSuite::CreateTestResult(EPerf_TestType TestType, float Score, bool bPassed, const FString& Details)
{
    FPerf_TestResult Result;
    Result.TestType = TestType;
    Result.Score = Score;
    Result.bPassed = bPassed;
    Result.Details = Details;
    return Result;
}
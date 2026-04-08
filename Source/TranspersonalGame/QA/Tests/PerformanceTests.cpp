#include "QA/QATestFramework.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

/**
 * Performance Tests for Transpersonal Game
 * 
 * These tests validate that the game meets performance requirements:
 * - 60 FPS on PC
 * - 30 FPS on Console
 * - Memory usage within limits
 * - Rendering performance targets
 */

/**
 * Test frame rate performance under normal gameplay conditions
 */
IMPLEMENT_TRANSPERSONAL_TEST(FFrameRateTest, "Transpersonal.Performance.FrameRate.Normal", QATestCategories::Performance)

bool FFrameRateTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Frame Rate Performance Test"));
    
    // Get current world
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/TestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load test level"));
        return false;
    }
    
    // Wait for level to fully load
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(2.0f));
    
    // Test PC frame rate target
    float TargetFPS = FPerformanceThresholds::PC_MIN_FPS;
    
    #if PLATFORM_CONSOLE
        TargetFPS = FPerformanceThresholds::CONSOLE_MIN_FPS;
    #endif
    
    bool bFrameRateValid = ValidateFrameRate(TargetFPS, 10.0f);
    
    VALIDATE_PERFORMANCE(bFrameRateValid, "Frame rate below target");
    
    UE_LOG(LogQAFramework, Log, TEXT("Frame Rate Performance Test completed successfully"));
    return true;
}

/**
 * Test frame rate under stress conditions (many dinosaurs)
 */
IMPLEMENT_TRANSPERSONAL_TEST(FFrameRateStressTest, "Transpersonal.Performance.FrameRate.Stress", QATestCategories::Performance)

bool FFrameRateStressTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Frame Rate Stress Test"));
    
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/StressTestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load stress test level"));
        return false;
    }
    
    // Spawn multiple dinosaurs for stress testing
    // This would be implemented when dinosaur classes are available
    
    // Test frame rate under stress
    float StressTargetFPS = FPerformanceThresholds::PC_MIN_FPS * 0.8f; // Allow 20% drop under stress
    
    #if PLATFORM_CONSOLE
        StressTargetFPS = FPerformanceThresholds::CONSOLE_MIN_FPS * 0.8f;
    #endif
    
    bool bStressFrameRateValid = ValidateFrameRate(StressTargetFPS, 15.0f);
    
    VALIDATE_PERFORMANCE(bStressFrameRateValid, "Frame rate under stress below acceptable threshold");
    
    UE_LOG(LogQAFramework, Log, TEXT("Frame Rate Stress Test completed successfully"));
    return true;
}

/**
 * Test memory usage during normal gameplay
 */
IMPLEMENT_TRANSPERSONAL_TEST(FMemoryUsageTest, "Transpersonal.Performance.Memory.Normal", QATestCategories::Performance)

bool FMemoryUsageTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Memory Usage Test"));
    
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/TestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load test level"));
        return false;
    }
    
    // Wait for level streaming and asset loading
    ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(5.0f));
    
    int32 MaxMemoryMB = FPerformanceThresholds::MAX_MEMORY_MB_PC;
    
    #if PLATFORM_CONSOLE
        MaxMemoryMB = FPerformanceThresholds::MAX_MEMORY_MB_CONSOLE;
    #endif
    
    bool bMemoryValid = ValidateMemoryUsage(MaxMemoryMB);
    
    VALIDATE_PERFORMANCE(bMemoryValid, "Memory usage exceeds platform limits");
    
    UE_LOG(LogQAFramework, Log, TEXT("Memory Usage Test completed successfully"));
    return true;
}

/**
 * Test rendering performance metrics
 */
IMPLEMENT_TRANSPERSONAL_TEST(FRenderingPerformanceTest, "Transpersonal.Performance.Rendering", QATestCategories::Performance)

bool FRenderingPerformanceTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Rendering Performance Test"));
    
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/RenderingTestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load rendering test level"));
        return false;
    }
    
    // Test rendering performance
    bool bRenderingValid = ValidateRenderingPerformance();
    
    VALIDATE_PERFORMANCE(bRenderingValid, "Rendering performance below acceptable levels");
    
    UE_LOG(LogQAFramework, Log, TEXT("Rendering Performance Test completed successfully"));
    return true;
}

/**
 * Test physics performance with multiple objects
 */
IMPLEMENT_TRANSPERSONAL_TEST(FPhysicsPerformanceTest, "Transpersonal.Performance.Physics", QATestCategories::Performance)

bool FPhysicsPerformanceTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting Physics Performance Test"));
    
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/PhysicsTestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load physics test level"));
        return false;
    }
    
    // Test physics performance
    bool bPhysicsValid = ValidatePhysicsPerformance();
    
    VALIDATE_PERFORMANCE(bPhysicsValid, "Physics performance below acceptable levels");
    
    UE_LOG(LogQAFramework, Log, TEXT("Physics Performance Test completed successfully"));
    return true;
}

/**
 * Test AI performance with multiple dinosaurs
 */
IMPLEMENT_TRANSPERSONAL_TEST(FAIPerformanceTest, "Transpersonal.Performance.AI", QATestCategories::Performance)

bool FAIPerformanceTest::RunTest(const FString& Parameters)
{
    UE_LOG(LogQAFramework, Log, TEXT("Starting AI Performance Test"));
    
    UWorld* World = AutomationOpenMap(TEXT("/Game/Maps/AITestLevel"));
    if (!World)
    {
        AddError(TEXT("Failed to load AI test level"));
        return false;
    }
    
    // Test AI performance
    bool bAIValid = ValidateAIPerformance();
    
    VALIDATE_PERFORMANCE(bAIValid, "AI performance below acceptable levels");
    
    UE_LOG(LogQAFramework, Log, TEXT("AI Performance Test completed successfully"));
    return true;
}
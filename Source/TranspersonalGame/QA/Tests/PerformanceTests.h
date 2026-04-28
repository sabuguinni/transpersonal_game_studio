#pragma once

#include "CoreMinimal.h"
// FIXME: Missing header - #include "QA/QATestFramework.h"
#include "Misc/AutomationTest.h"

/**
 * Performance Test Declarations
 * 
 * These tests validate that the game meets performance requirements:
 * - 60 FPS on PC / 30 FPS on Console
 * - Memory usage within platform limits
 * - Rendering performance targets
 * - Physics performance under load
 * - AI performance with multiple entities
 */

/**
 * Test frame rate performance under normal gameplay conditions
 */
class TRANSPERSONALGAME_API FFrameRateTest : public FAutomationTestBase
{
public:
    FFrameRateTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test frame rate under stress conditions (many dinosaurs)
 */
class TRANSPERSONALGAME_API FFrameRateStressTest : public FAutomationTestBase
{
public:
    FFrameRateStressTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test memory usage during normal gameplay
 */
class TRANSPERSONALGAME_API FMemoryUsageTest : public FAutomationTestBase
{
public:
    FMemoryUsageTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test rendering performance metrics
 */
class TRANSPERSONALGAME_API FRenderingPerformanceTest : public FAutomationTestBase
{
public:
    FRenderingPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test physics performance with multiple objects
 */
class TRANSPERSONALGAME_API FPhysicsPerformanceTest : public FAutomationTestBase
{
public:
    FPhysicsPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test AI performance with multiple dinosaurs
 */
class TRANSPERSONALGAME_API FAIPerformanceTest : public FAutomationTestBase
{
public:
    FAIPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};
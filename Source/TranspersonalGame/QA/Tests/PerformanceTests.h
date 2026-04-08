#pragma once

#include "CoreMinimal.h"
#include "QA/QATestFramework.h"
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
class TRANSPERSONALGAME_API FFrameRateTest : public FTranspersonalQATestBase
{
public:
    FFrameRateTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test frame rate under stress conditions (many dinosaurs)
 */
class TRANSPERSONALGAME_API FFrameRateStressTest : public FTranspersonalQATestBase
{
public:
    FFrameRateStressTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test memory usage during normal gameplay
 */
class TRANSPERSONALGAME_API FMemoryUsageTest : public FTranspersonalQATestBase
{
public:
    FMemoryUsageTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test rendering performance metrics
 */
class TRANSPERSONALGAME_API FRenderingPerformanceTest : public FTranspersonalQATestBase
{
public:
    FRenderingPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test physics performance with multiple objects
 */
class TRANSPERSONALGAME_API FPhysicsPerformanceTest : public FTranspersonalQATestBase
{
public:
    FPhysicsPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test AI performance with multiple dinosaurs
 */
class TRANSPERSONALGAME_API FAIPerformanceTest : public FTranspersonalQATestBase
{
public:
    FAIPerformanceTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};
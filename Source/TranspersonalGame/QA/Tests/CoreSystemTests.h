#pragma once

#include "CoreMinimal.h"
#include "QA/QATestFramework.h"
#include "Misc/AutomationTest.h"

/**
 * Core System Test Declarations
 * 
 * These tests validate fundamental game systems:
 * - World initialization and setup
 * - Physics systems functionality
 * - Collision detection accuracy
 * - Terrain generation performance
 * - Level streaming efficiency
 */

/**
 * Test world initialization and basic setup
 */
class TRANSPERSONALGAME_API FWorldInitializationTest : public FTranspersonalQATestBase
{
public:
    FWorldInitializationTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test physics system functionality
 */
class TRANSPERSONALGAME_API FPhysicsSystemTest : public FTranspersonalQATestBase
{
public:
    FPhysicsSystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test collision detection systems
 */
class TRANSPERSONALGAME_API FCollisionDetectionTest : public FTranspersonalQATestBase
{
public:
    FCollisionDetectionTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test terrain generation system
 */
class TRANSPERSONALGAME_API FTerrainGenerationTest : public FTranspersonalQATestBase
{
public:
    FTerrainGenerationTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test vegetation placement system
 */
class TRANSPERSONALGAME_API FVegetationPlacementTest : public FTranspersonalQATestBase
{
public:
    FVegetationPlacementTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test water systems
 */
class TRANSPERSONALGAME_API FWaterSystemTest : public FTranspersonalQATestBase
{
public:
    FWaterSystemTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test level streaming functionality
 */
class TRANSPERSONALGAME_API FLevelStreamingTest : public FTranspersonalQATestBase
{
public:
    FLevelStreamingTest(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};
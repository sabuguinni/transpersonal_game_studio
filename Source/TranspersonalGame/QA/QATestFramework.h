#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"

/**
 * QA Test Framework for Transpersonal Game Studio
 * 
 * This framework provides comprehensive testing capabilities for:
 * - Core Systems (Physics, Collision, Performance)
 * - AI Behavior (Dinosaur AI, NPC Behavior)
 * - World Generation (PCG, Environment)
 * - Player Systems (Survival, Crafting, Combat)
 * - Performance Validation (60fps PC / 30fps Console)
 * 
 * Based on Epic Games Automation Framework with custom extensions
 * for survival game mechanics and dinosaur behavior validation.
 */

DECLARE_LOG_CATEGORY_EXTERN(LogQAFramework, Log, All);

/**
 * Base class for all Transpersonal Game QA tests
 * Provides common functionality and validation methods
 */
class TRANSPERSONALGAME_API FTranspersonalQATestBase : public FAutomationTestBase
{
public:
    FTranspersonalQATestBase(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

protected:
    // Common test utilities
    bool ValidateFrameRate(float MinFPS, float TestDuration = 5.0f);
    bool ValidateMemoryUsage(int32 MaxMemoryMB);
    bool ValidateWorldState(UWorld* World);
    bool ValidateDinosaurBehavior(class ADinosaurBase* Dinosaur);
    bool ValidatePlayerSurvival(class ATranspersonalPlayerCharacter* Player);
    
    // Performance validation
    bool ValidateRenderingPerformance();
    bool ValidatePhysicsPerformance();
    bool ValidateAIPerformance();
    
    // World validation
    bool ValidateTerrainGeneration();
    bool ValidateVegetationPlacement();
    bool ValidateWaterSystems();
    
    // Gameplay validation
    bool ValidateCraftingSystem();
    bool ValidateCombatSystem();
    bool ValidateDomesticationSystem();
};

/**
 * Test categories for organized test execution
 */
namespace QATestCategories
{
    static const FString Core = TEXT("Transpersonal.Core");
    static const FString Physics = TEXT("Transpersonal.Physics");
    static const FString AI = TEXT("Transpersonal.AI");
    static const FString World = TEXT("Transpersonal.World");
    static const FString Performance = TEXT("Transpersonal.Performance");
    static const FString Gameplay = TEXT("Transpersonal.Gameplay");
    static const FString Survival = TEXT("Transpersonal.Survival");
    static const FString Combat = TEXT("Transpersonal.Combat");
    static const FString Dinosaurs = TEXT("Transpersonal.Dinosaurs");
}

/**
 * Performance thresholds for different platforms
 */
struct FPerformanceThresholds
{
    static constexpr float PC_MIN_FPS = 60.0f;
    static constexpr float CONSOLE_MIN_FPS = 30.0f;
    static constexpr int32 MAX_MEMORY_MB_PC = 8192;
    static constexpr int32 MAX_MEMORY_MB_CONSOLE = 4096;
    static constexpr float MAX_FRAME_TIME_MS = 16.67f; // 60fps
    static constexpr int32 MAX_DRAW_CALLS = 2000;
    static constexpr int32 MAX_TRIANGLES = 2000000;
};

/**
 * Macros for creating Transpersonal-specific tests
 */
#define IMPLEMENT_TRANSPERSONAL_TEST(TestClass, TestName, TestCategory) \
    IMPLEMENT_SIMPLE_AUTOMATION_TEST(TestClass, TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

#define IMPLEMENT_TRANSPERSONAL_COMPLEX_TEST(TestClass, TestName, TestCategory) \
    IMPLEMENT_COMPLEX_AUTOMATION_TEST(TestClass, TestName, EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

/**
 * Test result validation macros
 */
#define VALIDATE_PERFORMANCE(Condition, Message) \
    if (!(Condition)) \
    { \
        AddError(FString::Printf(TEXT("Performance Validation Failed: %s"), TEXT(Message))); \
        return false; \
    }

#define VALIDATE_GAMEPLAY(Condition, Message) \
    if (!(Condition)) \
    { \
        AddError(FString::Printf(TEXT("Gameplay Validation Failed: %s"), TEXT(Message))); \
        return false; \
    }

#define VALIDATE_AI_BEHAVIOR(Condition, Message) \
    if (!(Condition)) \
    { \
        AddError(FString::Printf(TEXT("AI Behavior Validation Failed: %s"), TEXT(Message))); \
        return false; \
    }
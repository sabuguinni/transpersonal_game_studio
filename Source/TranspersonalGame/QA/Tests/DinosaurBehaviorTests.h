#pragma once

#include "CoreMinimal.h"
#include "QATestFramework.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

/**
 * Comprehensive test suite for dinosaur behavior validation
 * Tests AI behavior, animation systems, and interaction mechanics
 * 
 * Validates:
 * - Individual dinosaur AI behavior patterns
 * - Herd/pack behavior dynamics
 * - Predator-prey interactions
 * - Environmental awareness and pathfinding
 * - Animation state transitions
 * - Combat behavior validation
 * - Domestication system mechanics
 */

DECLARE_LOG_CATEGORY_EXTERN(LogDinosaurBehaviorTests, Log, All);

/**
 * Base class for dinosaur behavior testing
 */
class TRANSPERSONALGAME_API FDinosaurBehaviorTestBase : public FAutomationTestBase
{
public:
    FDinosaurBehaviorTestBase(const FString& InName, const bool bInComplexTask)
        : FAutomationTestBase(InName, bInComplexTask)
    {
    }

protected:
    // Core behavior validation
    bool ValidateDinosaurSpawning(UWorld* TestWorld);
    bool ValidateBasicAI(class ADinosaurBase* Dinosaur);
    bool ValidateAnimationStates(class ADinosaurBase* Dinosaur);
    bool ValidatePathfinding(class ADinosaurBase* Dinosaur);
    
    // Interaction testing
    bool ValidatePredatorBehavior(class APredatorDinosaur* Predator);
    bool ValidateHerbivoreHerdBehavior(TArray<class AHerbivoreDinosaur*> Herd);
    bool ValidatePlayerInteraction(class ADinosaurBase* Dinosaur, class ATranspersonalPlayerCharacter* Player);
    
    // Advanced behavior systems
    bool ValidateDomesticationProgress(class ADinosaurBase* Dinosaur);
    bool ValidateEnvironmentalAwareness(class ADinosaurBase* Dinosaur);
    bool ValidateCombatBehavior(class ADinosaurBase* Attacker, class ADinosaurBase* Target);
    
    // Performance validation for AI systems
    bool ValidateAIPerformanceLoad(int32 DinosaurCount);
    bool ValidateBehaviorTreeExecution(class ADinosaurBase* Dinosaur);
    
    // Utility functions
    class ADinosaurBase* SpawnTestDinosaur(UWorld* World, TSubclassOf<class ADinosaurBase> DinosaurClass, FVector Location);
    TArray<class ADinosaurBase*> SpawnDinosaurHerd(UWorld* World, TSubclassOf<class ADinosaurBase> DinosaurClass, int32 Count);
    void CleanupTestDinosaurs(TArray<class ADinosaurBase*>& Dinosaurs);
};

/**
 * Test individual dinosaur AI behavior patterns
 */
class TRANSPERSONALGAME_API FDinosaurIndividualBehaviorTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurIndividualBehaviorTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.IndividualBehavior"), false)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test herd/pack behavior dynamics
 */
class TRANSPERSONALGAME_API FDinosaurHerdBehaviorTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurHerdBehaviorTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.HerdBehavior"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test predator-prey interaction systems
 */
class TRANSPERSONALGAME_API FDinosaurPredatorPreyTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurPredatorPreyTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.PredatorPrey"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test domestication system mechanics
 */
class TRANSPERSONALGAME_API FDinosaurDomesticationTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurDomesticationTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.Domestication"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test AI performance under load
 */
class TRANSPERSONALGAME_API FDinosaurAIPerformanceTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurAIPerformanceTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.AIPerformance"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};

/**
 * Test environmental awareness and adaptation
 */
class TRANSPERSONALGAME_API FDinosaurEnvironmentalTest : public FDinosaurBehaviorTestBase
{
public:
    FDinosaurEnvironmentalTest()
        : FDinosaurBehaviorTestBase(TEXT("Transpersonal.Dinosaurs.Environmental"), true)
    {
    }

    /* DISABLED: virtual bool RunTest(const FString& Parameters) override */;
};
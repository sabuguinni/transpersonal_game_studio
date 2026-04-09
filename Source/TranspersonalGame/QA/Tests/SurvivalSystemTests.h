#pragma once

#include "CoreMinimal.h"
#include "QATestFramework.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/AutomationTest.h"

/**
 * Comprehensive test suite for survival system validation
 * Tests all core survival mechanics and player progression systems
 * 
 * Validates:
 * - Hunger, thirst, and health systems
 * - Crafting and building mechanics
 * - Resource gathering and management
 * - Weather and environmental effects
 * - Day/night cycle impacts
 * - Equipment durability and repair
 * - Base building and security
 * - Food preservation and cooking
 */

DECLARE_LOG_CATEGORY_EXTERN(LogSurvivalSystemTests, Log, All);

/**
 * Base class for survival system testing
 */
class TRANSPERSONALGAME_API FSurvivalSystemTestBase : public FTranspersonalQATestBase
{
public:
    FSurvivalSystemTestBase(const FString& InName, const bool bInComplexTask)
        : FTranspersonalQATestBase(InName, bInComplexTask)
    {
    }

protected:
    // Core survival validation
    bool ValidateHungerSystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateThirstSystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateHealthSystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateStaminaSystem(class ATranspersonalPlayerCharacter* Player);
    
    // Crafting system validation
    bool ValidateCraftingInterface(class ATranspersonalPlayerCharacter* Player);
    bool ValidateRecipeSystem();
    bool ValidateResourceRequirements();
    bool ValidateCraftingQueue();
    
    // Building system validation
    bool ValidateBuildingPlacement(class ATranspersonalPlayerCharacter* Player);
    bool ValidateStructuralIntegrity();
    bool ValidateBaseSecurity();
    bool ValidateBuildingMaterials();
    
    // Resource management validation
    bool ValidateInventorySystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateResourceGathering(class ATranspersonalPlayerCharacter* Player);
    bool ValidateStorageContainers();
    bool ValidateItemDurability();
    
    // Environmental system validation
    bool ValidateWeatherEffects(class ATranspersonalPlayerCharacter* Player);
    bool ValidateDayNightCycle(class ATranspersonalPlayerCharacter* Player);
    bool ValidateTemperatureSystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateSeasonalChanges();
    
    // Food and cooking validation
    bool ValidateFoodSystem(class ATranspersonalPlayerCharacter* Player);
    bool ValidateCookingMechanics(class ATranspersonalPlayerCharacter* Player);
    bool ValidateFoodPreservation();
    bool ValidateNutritionalValues();
    
    // Utility functions
    class ATranspersonalPlayerCharacter* SpawnTestPlayer(UWorld* World, FVector Location);
    void SetupSurvivalTestEnvironment(UWorld* World);
    void CleanupSurvivalTest(class ATranspersonalPlayerCharacter* Player);
    bool SimulateSurvivalConditions(class ATranspersonalPlayerCharacter* Player, float Duration);
};

/**
 * Test basic survival needs (hunger, thirst, health)
 */
class TRANSPERSONALGAME_API FSurvivalBasicNeedsTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalBasicNeedsTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.BasicNeeds"), false)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test crafting system mechanics
 */
class TRANSPERSONALGAME_API FSurvivalCraftingTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalCraftingTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Crafting"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test building and construction systems
 */
class TRANSPERSONALGAME_API FSurvivalBuildingTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalBuildingTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Building"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test resource gathering and management
 */
class TRANSPERSONALGAME_API FSurvivalResourceTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalResourceTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Resources"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test environmental effects on survival
 */
class TRANSPERSONALGAME_API FSurvivalEnvironmentalTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalEnvironmentalTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Environmental"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test food and cooking systems
 */
class TRANSPERSONALGAME_API FSurvivalFoodTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalFoodTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Food"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test survival system performance under stress
 */
class TRANSPERSONALGAME_API FSurvivalPerformanceTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalPerformanceTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Performance"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};

/**
 * Test survival progression and advancement
 */
class TRANSPERSONALGAME_API FSurvivalProgressionTest : public FSurvivalSystemTestBase
{
public:
    FSurvivalProgressionTest()
        : FSurvivalSystemTestBase(TEXT("Transpersonal.Survival.Progression"), true)
    {
    }

    virtual bool RunTest(const FString& Parameters) override;
};